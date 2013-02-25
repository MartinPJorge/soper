/* Practica 1 de Sistemas Operativos - ejercicio final
 * 
 * autores:
 * 		Ivan Marquez Pardo
 * 		Jorge Martin Perez
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#define TAM 60
#define T_ESPERA 5

typedef enum {T_NORMAL = 1, T_REMONTADORA, T_GANADORA} Tirada;
typedef enum {TIRANDO = 0, ESPERANDO, CAIDO} HorseStatus;
typedef enum {TRUE, FALSE} BOOL;

int sigAlarma = 0;
int caida = 0;

/* 
 * funcion:
 * 		tirada
 * descripcion: 
 * 		se encarga de reconocer las posiciones de los caballos, para escribir en las
 * 		tunerias correspondientes el tipo de tirada que les corresponde, para despues
 * 		mandarles una señal de comienzo
 *
 * parametros:
 * 		- posiciones: array de las posiciones de los caballos
 * 		- PIDHijo: array con los PID de los caballos
 * 		- tubPadre: array con las tuberias de comunicacion del padre al hijo
 * 		- tubHijo: array con las tuberias de comunicacion del hijo al padre
 * 		- nProc: numero de caballos
 */
void tirada(int *posiciones, pid_t *PIDHijo, int **tubPadre,
		int **tubHijo, int nProc, int *estadoCaballos){

	int min = posiciones[0], max = 0, i;
	int posMin = 0, posMax = 0;
	char buffer[TAM];

	/* Buscamos el primero y el ultimo. */
	for(i = 0; i < nProc; i++) {

		if(estadoCaballos[i] != CAIDO) {

			if(posiciones[i] < min) {

				min = posiciones[i];
				posMin = i;
			}

			else if(posiciones[i] > max) {

				max = posiciones[i];
				posMax = i;
			}
		}
	}


	for (i = 0; i < nProc; i++) {

		if(estadoCaballos[i] != CAIDO) {

			if(posiciones[i] == max) {

				snprintf(buffer, TAM, "%d", T_GANADORA);			/* Escribir tirada en pipe, y mandar señal. */
				write (tubPadre[i][1], buffer, strlen(buffer) + 1);

				fflush(NULL);

				kill(PIDHijo[i], SIGUSR1);
			}

			else if(posiciones[i] == min) {

				snprintf(buffer, TAM, "%d", T_REMONTADORA);			/* Escribir tirada en pipe, y mandar señal. */
				write (tubPadre[i][1], buffer, strlen(buffer) + 1);

				kill(PIDHijo[i], SIGUSR1);
			}

			else {

				snprintf(buffer, TAM, "%d", T_NORMAL);			/* Escribir tirada en pipe, y mandar señal. */
				write (tubPadre[i][1], buffer, strlen (buffer)+1);

				kill(PIDHijo[i], SIGUSR1);
			}
		}
	}

	
	return;
}


/* 
 * funcion:
 * 		makePipes
 * descripcion: 
 * 		se encarga de crear dos arrays de pipes que permiten la comunicacion
 * 		bidireccional entre el proceso padre y los caballos
 *
 * parametros:
 * 		- tubPadre: array con las tuberias de comunicacion del padre al hijo
 * 		- tubHijo: array con las tuberias de comunicacion del hijo al padre
 * 		- nProc: numero de caballos
 */
void makePipes(int ***tubPadre, int ***tubHijo, int nProc){


	int i;

	*tubPadre = (int**) malloc (nProc * sizeof(int*));
	if(!(*tubPadre)){
		printf("Error en la reserva de memoria.\n");
		exit (1);
	}

	*tubHijo = (int**) malloc (nProc * sizeof(int*));
	if(!(*tubHijo)){
		printf("Error en la reserva de memoria.\n");
		exit (1);
	}

	for(i=0; i<nProc; i++){

		(*tubPadre)[i]=(int*) malloc (2 * sizeof(int));
		if(!(*tubPadre)[i]){
			printf("Error en la reserva de memoria.\n");
			exit (1);
		}

		(*tubHijo)[i]=(int*) malloc (2 * sizeof(int));
		if(!(*tubHijo)[i]){
			printf("Error en la reserva de memoria.\n");
			exit (1);
		}
	}



	/* Creamos las tuberias para los procesos hijos.
	   Ponemos los canales de lectura para que no sean bloqueantes. */
	for(i = 0; i < nProc; i++){

		if(pipe ((*tubPadre)[i]) == -1) {

			printf("No se ha creado bien la tuberia %d.\n", i);
			exit(1);
		}
		fcntl((*tubPadre)[i][0], F_SETFL, O_NONBLOCK);

		if(pipe ((*tubHijo)[i]) == -1) {

			printf("No se ha creado bien la tuberia %d.\n", i);
			exit(1);
		}
		fcntl((*tubHijo)[i][0], F_SETFL, O_NONBLOCK);
	}

	return;
}


/*
 * funcion:
 * 		son_closeNonUsedPipes
 * descripcion:
 * 		se encarga de cerrar las tuberias que no utiliza el proceso hijo que se
 * 		encuentra en la posicion 'index', dentro de un array de PID de
 * 		procesos.
 *
 * parametros:
 * 		- tubPadre: array con las tuberias de comunicacion del padre al hijo
 * 		- tubHijo: array con las tuberias de comunicacion del hijo al padre
 * 		- nProc: numero de procesos hijos
 * 		- index: posicion que ocupa el proceso que llama a esta funcion,
 * 			dentro de un array de PID de procesos
 */
void son_closeNonUsedPipes(int **tubPadre, int **tubHijo, int nProc, int index) {

	int i;

	for(i = 0; i < nProc; i++) {

		if(i != index) {

			close(tubPadre[i][0]);
			close(tubPadre[i][1]);
			close(tubHijo[i][0]);
			close(tubHijo[i][1]);
		}
		else {

			close(tubPadre[index][1]);
			close(tubHijo[index][0]);
		}
	}

	return;
}


/*
 * function:
 * 		parent_closeNonUsedPipes
 * descripcion:
 * 		se encarga de cerrar las tuberias que no utiliza el proceso padre
 *
 * parametros:
 * 		- tubPadre: array con las tuberias de comunicacion del padre al hijo
 * 		- tubHijo: array con las tuberias de comunicacion del hijo al padre
 * 		- nProc: numero de procesos hijos
 */
void parent_closeNonUsedPipes(int **tubPadre, int **tubHijo, int nProc) {

	int i;

	for(i = 0; i < nProc; i++) {

		close(tubPadre[i][0]);
		close(tubHijo[i][1]);
	}

	return;
}

/*
 * funcion:
 * 		gestionarRespuestas
 * descripcion:
 * 		se encarga de recorrer las tuberias en las que los caballos van
 * 		dejando la informacion de la tirada realizada, para actualizar
 * 		las posiciones de estos, asi como su estado.
 *
 * 	parametros:
 * 		- posiciones: array con las posiciones de los caballos
 * 		- estadoCaballos: array con el estado de cada caballo
 * 		- tubHijo: array con las tuberias de comunicacion del hijo al padre
 * 		- nProc: numero de procesos hijos
 * 		- meta: longitud de la carrera
 */
void gestionarRespuestas(int *posiciones, int *estadoCaballos, int **tubHijo,
				int nProc, int meta) {

	int i, lectura;
	char buffer[TAM];

	for (i = 0; i < nProc; i++) {
		
		if(estadoCaballos[i] == TIRANDO) {

			lectura = read(tubHijo[i][0], buffer, TAM + 1);

			if (lectura != -1) {
				
				posiciones[i] += atoi(buffer);
				estadoCaballos[i] = ESPERANDO;

				if(posiciones[i] >= meta)
					return;
			}
		}
	}

	return;
}


/*
 * funcion: finDeTiradas
 * descripcion:
 * 		se encarga de decir si todos los caballos
 * 		disponibles han terminado de tirar.
 *
 * parametros:
 * 		- estadoCaballos: array con el estado de cada caballo
 * 		- nProc: numero de procesos hijos
 *
 * retorno:
 * 		- TRUE: todos los caballos disponibles han tirado
 * 		- FALSE: hay algun caballo disponible por tirar
 */
BOOL finDeTiradas(int *estadoCaballos, int nProc) {

	int i;

	for(i = 0; i < nProc; i++)
		if(estadoCaballos[i] == TIRANDO)
			return FALSE;

	return TRUE;
}


/*
 * funcion:
 * 		comprobarFinCarrera
 * descripcion:
 * 		se encarga de determinar si un caballo a llegado a meta
 *
 * parametros:
 * 		- estadoCaballos: array con el estado de cada caballo
 * 		- posiciones: array con las posiciones de los caballos
 * 		- nProc: numero de procesos hijos
 * 		- meta: longitud de la carrera
 *
 * retorno:
 * 		- TRUE: la carrera ha acabado
 * 		- FALSE: la carrera no ha acabado
 */
BOOL comprobarFinCarrera(int *estadoCaballos, int *posiciones, int nProc,
			int meta) {

	int i;

	for(i = 0; i < nProc; i ++)
		if(estadoCaballos[i] != CAIDO)
			if(posiciones[i] >= meta)
				return TRUE;

	return FALSE;
}


/*
 * funcion:
 * 		buscarCaidas
 * descripcion:
 * 		se encarga de mirar los procesos que no han podido realizar
 * 		la tirada, una vez ha llegado la alarma.
 *
 * parametros:
 * 		- estadoCaballos: array con el estado de cada caballo
 * 		- tubPadre: array con las tuberias de comunicacion del padre al hijo
 * 		- tubHijo: array con las tuberias de comunicacion del hijo al padre
 * 		- PIDHijo: array con los PID de los procesos hijos
 * 		- nProc: numero de procesos hijos
 */
void buscarCaidas(int *estadoCaballos, int **tubHijo, int **tubPadre,
			pid_t *PIDHijo, int nProc) {

	int i;

	/* Los caballos que nos encontremos sin terminar, los
	 * matamos enviandoles 'SIGILL' para que su proceso
	 * asociado se encargue de liberar recursos. */
	for(i = 0; i < nProc; i++)
		if(estadoCaballos[i] == TIRANDO) {

			estadoCaballos[i] = CAIDO;

			close(tubPadre[i][1]);
			close(tubHijo[i][0]);

			kill(PIDHijo[i], SIGILL);
		}

	return;
}


/*
 * funcion:
 * 		ponerEnModoTirando
 * descripcion:
 * 		antes de llamar a la funcion 'tirada', se debe llamar
 * 		a esta funcion para poner cada caballo en estado de
 * 		'TIRANDO'.
 *
 * parametros:
 * 		- estadoCaballos: array con el estado de cada caballo
 * 		- nProc: numero de procesos hijos
 */
void ponerEnModoTirando(int *estadoCaballos, int nProc) {

	int i;

	for(i = 0; i < nProc; i++)
		if(estadoCaballos[i] != CAIDO)
			estadoCaballos[i] = TIRANDO;

	return;
}


/*
 * funcion:
 * 		mostrarPosiciones
 * descripcion:
 * 		muestra por pantalla el estado de la carrera
 *
 * parametros:
 * 		- estadoCaballos: array con el estado de cada caballo
 * 		- nProc: numero de procesos hijos
 */
void mostrarPosiciones(int *estadoCaballos, int *posiciones, int nProc, int meta) {

	int i;

	for(i = 0; i < nProc; i++)
		if(estadoCaballos[i] != CAIDO)
			printf("%d\t\t", posiciones[i]);
		else if(posiciones[i] < meta)
			printf("-\t");

	printf("\n");

	return;
}


/*
 * funcion:
 * 		tirarDado
 * descripcion:
 * 		simula el tipo de tirada de un caballo
 *
 * parametros:
 * 		- tipoTirada: indica el tipo de tirada mediante
 * 				la enumeracion de tipo 'Tirada'
 *
 * retorno:
 * 		el resultado de la tirada
 */
int tirarDado(int tipoTirada) {

	int resultado;

	switch(tipoTirada) {

		case T_NORMAL:

			resultado = (rand() % 6) + 1;
			break;

		case T_GANADORA:

			resultado = (rand() % 7) + 1;
			break;

		default:

			resultado = (rand() % 6) + 1;
			resultado += (rand() % 6) + 1;
			break;
	}

	return resultado;
}


/*
 * funcion:
 * 		retirarCaballo
 * descripcion:
 * 		funcion encargada de liberar la memoria usada por un caballo,
 * 		y de la terminacion del proceso
 *
 * parametros:
 * 		- tubPadre: array con las tuberias de comunicacion del padre al hijo
 * 		- tubHijo: array con las tuberias de comunicacion del hijo al padre
 * 		- PIDHijo: array con los PID de los procesos hijos
 * 		- nProc: numero de procesos hijos
 */
void retirarCaballo(int **tubPadre, int **tubHijo, int *PIDHijo, int nProc) {

	int i;

	free(PIDHijo);

	for (i = 0; i < nProc; i++) {
		
		free(tubPadre[i]);
		free(tubHijo[i]);
	}

	free(tubPadre);
	free(tubHijo);

	exit(1);

	return;
}


/*
 * funcion:
 * 		sacarCaballos
 * descripcion:
 * 		funcion encargada de mandar a los caballos la senal de finalizar
 *
 * parametros:
 * 		- PIDHijo: array con los PID de los procesos hijos
 * 		- nProc: numero de procesos hijos
 * 		- estadoCaballos: array con el estado de cada caballo
 */
void sacarCaballos(int *PIDHijo, int nProc, int *estadoCaballos) {

	int i;

	for(i = 0; i < nProc; i++)
		if(estadoCaballos[i] != CAIDO)
			kill(PIDHijo[i], SIGILL);

	return;
}


/*
 * funcion:
 * 		capturaUser1
 * descripcion:
 * 		funcion encargada de que hacer tras la captura de SIGUSR1
 */
void capturaUser1(int sennal) {

	return;
}


/*
 * funcion:
 * 		capturaCaida
 * descripcion:
 * 		funcion encargada de que hacer tras la captura de la sennal
 * 		que indica la caida de un caballo.
 */
void capturaCaida(int sennal) {

	caida = 1;
	
	return;
}

/*
 * funcion:
 * 		capturaAlarma
 * descripcion:
 * 		funcion encargada de que hacer tras la captura de SIGUSR1
 */
void capturaAlarma(int sennal) {

	sigAlarma = 1;  /* Indicamos que ha saltado la alarma. */

	return;
}





int main (int argc, char *argv [], char *env []) {
	
	char buffer[TAM];
	int  **tubPadre=NULL, **tubHijo=NULL, i, flag = 0;
	int *posiciones, *estadoCaballos;
	int tipoTirada, resulDado;
	BOOL finCarrera = FALSE, todosResponden = FALSE;
	pid_t *PIDHijo;
	int nProc=0, longitud=0;
	sigset_t mask, temp, sonTemp, oldMask;

	if(argc!=3){
		printf("Numero incorrecto de argumentos.\n");
		exit (1);
	}

	/* Creamos las mascaras */
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGILL);
	sigaddset(&mask, SIGALRM);
	sigprocmask(SIG_SETMASK, &mask, &oldMask);

	sigfillset(&temp);
	sigdelset(&temp, SIGUSR1);
	sigdelset(&temp, SIGILL);
	sigdelset(&temp, SIGALRM);

	sigfillset(&sonTemp);
	sigdelset(&sonTemp, SIGUSR1);
	sigdelset(&sonTemp, SIGILL);




	/* Determinamos la gestion de señales. */
	if(signal(SIGUSR1, capturaUser1) == SIG_ERR) {

		printf("Error en la captura.\n");
		exit(1);
	}

	if(signal(SIGILL, capturaCaida) == SIG_ERR) {

		printf("Error en la captura.\n");
		exit(1);
	}

	if(signal(SIGALRM, capturaAlarma) == SIG_ERR) {

		printf("Error en la captura.\n");
		exit(1);
	}



	nProc = atoi(argv[1]);
	longitud = atoi(argv[2]);

	makePipes(&tubPadre, &tubHijo, nProc);

	PIDHijo = (pid_t*) malloc (nProc*sizeof(pid_t));
	if(!PIDHijo){
		printf("Error en la reserva de memoria.\n");
		exit (1);
	}




	/* Creamos los procesos hijos. */
	for (i = 0; i < nProc; i++) {

		PIDHijo[i] = fork();

		switch(PIDHijo[i]) {

			case 0:
				flag = 1;
				son_closeNonUsedPipes(tubPadre, tubHijo, nProc, i);
				break;

			case -1:
				printf("Error al crear proceso.\n");
				flag = 1;
				break;

			default: 
				break;
		}

		if(flag == 1)
			break;
	}
	
		

	/* Hasta aqui ya tenemos creados todos los procesos hijos y sus tuberías */
	/*----------------------------------------------------------------------------------------------------------*/
	

	/* Ejecucion del padre. */	
	if(flag != 1) {

		parent_closeNonUsedPipes(tubPadre, tubHijo, nProc);

		for(i = 0; i < nProc; i++)
			printf("Caballo%d\t", i);
		printf("\n");

		posiciones = (int *) calloc(nProc, sizeof(int));
		estadoCaballos = (int *) calloc(nProc, sizeof(int));


		while(finCarrera == FALSE) {

			ponerEnModoTirando(estadoCaballos, nProc);
			tirada(posiciones, PIDHijo, tubPadre, tubHijo, nProc, estadoCaballos);

			

			alarm(T_ESPERA);
			sigAlarma = 0;
			todosResponden = FALSE;

			while((todosResponden == FALSE) && !sigAlarma && (finCarrera == FALSE)) {

				sigsuspend(&temp);


				if(sigAlarma)	/* Cuando llega la alarma no leemos las tuberias. */
					break;

				gestionarRespuestas(posiciones, estadoCaballos, tubHijo, nProc, longitud);
				todosResponden = finDeTiradas(estadoCaballos, nProc);
				finCarrera = comprobarFinCarrera(estadoCaballos, posiciones, nProc, longitud);
			}

			if(sigAlarma)
				buscarCaidas(estadoCaballos, tubHijo, tubPadre, PIDHijo, nProc);

			mostrarPosiciones(estadoCaballos, posiciones, nProc, longitud);
		}

		sacarCaballos(PIDHijo, nProc, estadoCaballos);

		/* Liberamos memoria del padre */
		free(posiciones);
		free(estadoCaballos);
	}

	/* Procesos hijos (caballos) */
	else {

		srand(getpid());

		/* El hijo tira siempre hasta que el padre le manda parar. */
		while(1) {

			sigsuspend(&sonTemp);

			if(caida)
				retirarCaballo(tubPadre, tubHijo, PIDHijo, nProc);

			read(tubPadre[i][0], buffer, TAM);
			tipoTirada = atoi(buffer);

			resulDado = tirarDado(tipoTirada);
			snprintf(buffer, TAM, "%d", resulDado);
			write (tubHijo[i][1], buffer, strlen(buffer) + 1);

			kill(getppid(), SIGUSR1);
		}

	}


	exit(0);
}

