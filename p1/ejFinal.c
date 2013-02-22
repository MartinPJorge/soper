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
#define TAM 16
#define T_ESPERA 5

typedef enum {T_NORMAL = 1, T_REMONTADORA, T_GANADORA} Tirada;
typedef enum {TIRANDO = 0, ESPERANDO, CAIDO} HorseStatus;
typedef enum {TRUE, FALSE} BOOL;

int sigAlarma = 0;

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

				kill(PIDHijo[i], SIGUSR2);
			}

			else if(posiciones[i] == min) {

				snprintf(buffer, TAM, "%d", T_REMONTADORA);			/* Escribir tirada en pipe, y mandar señal. */
				write (tubPadre[i][1], buffer, strlen(buffer) + 1);

				kill(PIDHijo[i], SIGUSR2);
			}

			else {

				snprintf(buffer, TAM, "%d", T_NORMAL);			/* Escribir tirada en pipe, y mandar señal. */
				write (tubPadre[i][1], buffer, strlen (buffer)+1);

				kill(PIDHijo[i], SIGUSR2);
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
void makePipes(int **tubPadre, int **tubHijo, int nProc){


	int i;

	tubPadre = (int**) malloc (nProc*sizeof(int*));
	if(!tubPadre){
		printf("Error en la reserva de memoria.\n");
		exit (1);
	}

	tubHijo = (int**) malloc (nProc*sizeof(int*));
	if(!tubHijo){
		printf("Error en la reserva de memoria.\n");
		exit (1);
	}

	for(i=0; i<nProc; ++i){

		tubPadre[i]=(int*) malloc (2*sizeof(int));
		if(!tubPadre[i]){
			printf("Error en la reserva de memoria.\n");
			exit (1);
		}

		tubHijo[i]=(int*) malloc (2*sizeof(int));
		if(!tubHijo[i]){
			printf("Error en la reserva de memoria.\n");
			exit (1);
		}
	}

	/* Creamos las tuberias para los procesos hijos.
	   Ponemos los canales de lectura para que no sean bloqueantes. */
	for(i = 0; i < nProc; i++){

		if(pipe (tubPadre[i]) == -1) {

			printf("No se ha creado bien la tuberia %d.\n", i);
			exit(1);
		}
		fcntl(tubPadre[i][0], F_SETFL, O_NONBLOCK);

		if(pipe (tubHijo[i]) == -1) {

			printf("No se ha creado bien la tuberia %d.\n", i);
			exit(1);
		}
		fcntl(tubHijo[i][0], F_SETFL, O_NONBLOCK);
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
 */
void gestionarRespuestas(int *posiciones, int *estadoCaballos, int **tubHijo,
				int nProc) {

	int i, lectura;
	char buffer[TAM];

	for (i = 0; i < nProc; i++) {
		
		if(estadoCaballos[i] == TIRANDO) {

			lectura = read(tubHijo[i][0], buffer, TAM + 1);

			if (lectura != -1) {
				
				posiciones[i] += atoi(buffer);
				estadoCaballos[i] = ESPERANDO;
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
		if(estadoCaballos == TIRANDO) {

			printf("El caballo %d se ha caido\n", i+1);

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
			printf("%d\t", posiciones[i]);
		else if(posiciones[i] < meta)
			printf("-\t");

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
	int nbytes, **tubPadre=NULL, **tubHijo=NULL, i, j, flag = 0;
	int *posiciones, *estadoCaballos;
	BOOL finCarrera = FALSE, todosResponden = FALSE;
	pid_t *PIDHijo;
	int nProc=0, longitud=0;
	sigset_t mask, temp, oldMask;

	if(argc!=3){
		printf("Numero incorrecto de argumentos.\n");
		exit (1);
	}

	/* Creamos las mascaras */
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_SETMASK, &mask, &oldMask);

	sigfillset(&temp);
	sigdelset(&temp, SIGUSR1);
	sigdelset(&temp, SIGALRM);


	/* Determinamos la gestion de señales. */
	if(signal(SIGUSR1, capturaUser1) == SIG_ERR) {

		printf("Error en la captura.\n");
		exit(1);
	}


	nProc = atoi(argv[1]);
	longitud = atoi(argv[2]);

	makePipes(tubPadre, tubHijo, nProc);

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

	parent_closeNonUsedPipes(tubPadre, tubHijo, nProc);
		

	/* Hasta aqui ya tenemos creados todos los procesos hijos y sus tuberías */
	/*----------------------------------------------------------------------------------------------------------*/
	

	/* Ejecucion del padre. */	
	if(flag != 1) {

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

			while((todosResponden == FALSE) && !sigAlarma && (finCarrera == FALSE)) {

				sigsuspend(&temp);

				if(sigAlarma)	/* Cuando llega la alarma no leemos las tuberias. */
					break;

				gestionarRespuestas(posiciones, estadoCaballos, tubHijo, nProc);
				todosResponden = finDeTiradas(estadoCaballos, nProc);
				finCarrera = comprobarFinCarrera(estadoCaballos, posiciones, nProc, longitud);
			}

			if(sigAlarma)
				buscarCaidas(estadoCaballos, tubHijo, tubPadre, PIDHijo, nProc);

			mostrarPosiciones(estadoCaballos, posiciones, nProc, longitud);
		}

		/* Liberamos memoria del padre */
		free(posiciones);
		free(estadoCaballos);
	}
	else {

		/* Cerramos el resto de tuberias. */
		for(j = 0; j < nProc; j++) {

			if(j != i) {
				close(tubHijo[j][0]);
				close(tubHijo[j][1]);
				close(tubPadre[j][0]);
				close(tubPadre[j][1]);
			}
		}

		close(tubHijo[i][0]);
		close(tubPadre[i][1]);

		while ((nbytes = read(tubPadre[i][0], buffer, TAM)) > 0)
			fprintf (stdout, "Texto leido por el hijo: %s", buffer);
		close (tubPadre[i][0]);

		snprintf(buffer, TAM, "Datos devueltos a traves de la tuberia por el proceso PID = %d\n", getpid());
		write (tubHijo[i][1] ,buffer, strlen(buffer) + 1);
		close (tubHijo[i][1]);
	}


	exit(0);
}

