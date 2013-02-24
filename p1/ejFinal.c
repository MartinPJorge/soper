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
#define TIEMPO_ESPERA 5


typedef enum {T_NORMAL = 1, T_REMONTADORA, T_GANADORA} Tirada;
typedef enum {TIRANDO = 0, ESPERANDO, CAIDO} HorseStatus;
typedef enum {TRUE, FALSE} BOOL;

int sigAlarma = 0;

/* 
 * funcion:
 * 		tirada
 * descripcion: 
 * 		se encarga de reconocer las posiciones de los caballos, para escribir en las
 * 		tuberias correspondientes el tipo de tirada que les corresponde, para despues
 * 		mandarles una señal de comienzo
 *
 * parametros:
 * 		- posiciones: array de las posiciones de los caballos
 * 		- PIDHijo: array con los PID de los caballos
 * 		- tubPadre: array con las tuberias de comunicacion del padre al hijo
 * 		- tubHijo: array con las tuberias de comunicacion del hijo al padre
 * 		- nProc: numero de caballos
 *		- estadoCaballos: array que lleva el control de los estados de los caballos
 */
void tirada(int *posiciones, pid_t *PIDHijo, int **tubPadre,
		int **tubHijo, int nProc, int *estadoCaballos){

	int min = posiciones[0], max = 0, i;
	char buffer[TAM];

	/* Buscamos el primero y el ultimo. */
	for(i = 0; i < nProc; i++) {

		if(estadoCaballos[i] != CAIDO) {

			if(posiciones[i] < min) {

				min = posiciones[i];
			}

			else if(posiciones[i] > max) {

				max = posiciones[i];
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

		if(i == index) {

			close(tubPadre[index][1]);
			close(tubHijo[index][0]);
		}
		else {

			close(tubPadre[i][0]);
			close(tubPadre[i][1]);
			close(tubHijo[i][0]);
			close(tubHijo[i][1]);
			
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
		else //if(posiciones[i] < meta)
			printf("-\t");

	return;
}


/*
 * funcion:
 * 		capturaSIGUSR

 * descripcion:
 * 		funcion encargada de que hacer tras la captura de SIGUSR1
 */
void capturaSIGUSR(int sennal) {

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


/*
 * funcion:
 * 		aleat_num
 * descripcion:
 * 		funcion que genera un numero aleatorio entre los dos introducidos
 */
int aleat_num(int inf, int sup) {
   return inf+(int)((sup-inf+1.0)*rand()/(RAND_MAX+1.0));
}


/*
 * funcion:
 * 		realizarTirada
 * descripcion:
 * 		funcion que genera, a partir del tipo de tirada, una puntuacion aleatoria correspondiente
 */
int realizarTirada(tipoTirada){
	switch(tipoTirada){
		case T_NORMAL:
			return aleat_num(1,6);
			break;
		
		case T_REMONTADORA:
			return aleat_num(1,6)+aleat_num(1,6);
			break;

		case T_GANADORA:
			return aleat_num(1,7);
			break;

		default:
			return -1;
	}
}


int main (int argc, char *argv [], char *env []) {
	
	char buffer[TAM];
	int nbytes, **tubPadre=NULL, **tubHijo=NULL, i, j, tipoTirada=0, puntos=0, flag = 0;
	int *posiciones, *estadoCaballos;
	BOOL finCarrera = FALSE, todosResponden = FALSE;
	pid_t *PIDHijo;
	int nProc=0, longitud=0;
	sigset_t mask, temp, oldMask, childrenMask;

	if(argc!=3){
		printf("Numero incorrecto de argumentos.\n");
		exit (1);
	}

	srand(time(NULL));

	/* Creamos las mascaras */
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_SETMASK, &mask, &oldMask);	// mask bloqueará SIGUSR1

	sigfillset(&temp);
	sigdelset(&temp, SIGUSR1);	// temp bloqueará todas las señales excepto SIGUSR1...
	sigdelset(&temp, SIGALRM);	// ...y SIGALRM


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

	son_closeNonUsedPipes(tubPadre, tubHijo, nProc, i);
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

		if(signal(SIGUSR1, capturaSIGUSR) == SIG_ERR) {
			printf("Error en la captura.\n");
			exit(1);
		}

		while(finCarrera == FALSE) {

			ponerEnModoTirando(estadoCaballos, nProc);
			tirada(posiciones, PIDHijo, tubPadre, tubHijo, nProc, estadoCaballos);
			
			sigAlarma = 0;
			alarm(TIEMPO_ESPERA);
			
			while((todosResponden == FALSE) && !sigAlarma /*&& (finCarrera == FALSE)*/) {

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

		for(i=0; i<nProc; ++i){
			if(posiciones[i]>=longitud){
				printf("Ha ganado el Caballo%d.\n", i); //Cambiar para empates
			}
		}
		/* Liberamos memoria del padre */
		free(posiciones);
		free(estadoCaballos);
		free(PIDHijo);

		for(i = 0; i < nProc; i++) {
			close(tubPadre[i][1]);
			free(tubPadre[i]);
			close(tubHijo[i][0]);
			free(tubHijo[i]);
		}
		free(tubPadre);
		free(tubHijo);
	}
	else {
		sigfillset(&childrenMask);
		sigdelset(&childrenMask, SIGUSR2);	// childrenMask bloqueará todas las señales excepto SIGUSR2

		if(signal(SIGUSR2, capturaSIGUSR) == SIG_ERR) {
			printf("Error en la captura.\n");
			exit(1);
		}

		while(1){
			sigsuspend(childrenMask);
			
			if(read(tubPadre[i][0], buffer, TAM)<=0){
				printf("Datos mal enviados.\n");
				return -1;
			}
			tipoTirada=atoi(buffer);
			
			puntos=realizarTirada(tipoTirada);
			if(puntos==-1){
				printf("Tirada fallida!\n");
			}

			snprintf(buffer, TAM, "%d", puntos);			/* Escribir tirada en pipe, y mandar señal. */
			write (tubHijo[i][1], buffer, strlen (buffer)+1);

			kill(getppid(), SIGUSR1);
	
		}
	}


	exit(0);
}

