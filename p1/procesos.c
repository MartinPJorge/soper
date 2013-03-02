#include "procesos.h"

extern int sigAlarma;
extern int caida;
extern int stopEverything;


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


/*
 * funcion:
 * 		capturaInt
 * descripcion:
 * 		funcion encargada de modificar la variable global encargada de detener todo el programa
 */
void capturaInt(int sennal) {

	stopEverything = 1;  /* Indicamos que ha saltado la alarma. */

	return;
}



/*
 * funcion:
 * 		crearMascaras
 * descripcion:
 * 		funcion que se encarga de crear las mascaras del programa
 *
 * parametros:
 * 		- mask: mascara de la carrera
 * 		- temp: mascara temporal para el sigsuspend del padre
 * 		- sonTemp: mascara temporal para el sigsuspend de los caballos
 * 		- oldMask: mascara vieja
 */
void crearMascaras(sigset_t *mask, sigset_t *temp, sigset_t *sonTemp,
			sigset_t *oldMask) {


	sigemptyset(mask);		// mask bloqueará SIGUSR1, SIGILL Y SIGALRM
	sigaddset(mask, SIGUSR1);
	sigaddset(mask, SIGILL);
	sigaddset(mask, SIGALRM);
	sigprocmask(SIG_SETMASK, mask, oldMask);

	sigfillset(temp);		// temp bloqueará todas las señales excepto SIGUSR1, SIGILL, SIGALRM y SIGINT
	sigdelset(temp, SIGUSR1);
	sigdelset(temp, SIGILL);
	sigdelset(temp, SIGALRM);
	sigdelset(temp, SIGINT);

	sigfillset(sonTemp);	// sonTemp bloqueará SIGILL y SIGUSR1
	sigdelset(sonTemp, SIGUSR1);
	sigdelset(sonTemp, SIGILL);

	return;
}


/*
 * funcion:
 * 		gestionarSignals
 * descripcion:
 * 		funcion que se encarga de hacer las llamadas a 'signal'
 *
 * parametros:
 * 		- mask: mascara de la carrera
 * 		- temp: mascara temporal para el sigsuspend del padre
 * 		- sonTemp: mascara temporal para el sigsuspend de los caballos
 * 		- oldMask: mascara vieja
 */
void gestionarSignals() {

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

	if(signal(SIGINT, capturaInt) == SIG_ERR) {

		printf("Error en la captura.\n");
		exit(1);
	}

	return;
}


/*
 * funcion:
 * 		crearCaballos
 * descripcion:
 * 		funcion que se encarga de crear los procesos de caballo
 *
 * parametros:
 * 		- nProc: numero de procesos hijos
 * 		- tubPadre: array con las tuberias de comunicacion del padre al hijo
 * 		- tubHijo: array con las tuberias de comunicacion del hijo al padre
 * 		- PIDHijo: array con los PID de los procesos hijos
 * 		- esHijo: en esta variable se guardara si el proceso que
 * 			sale de la funcion es un caballo o no
 *
 * retorno:
 * 		- i: posicion que ocupa el caballo en el array de
 * 			'PIDHijo'
 */
int crearCaballos(int nProc, int **tubPadre, int **tubHijo, 
		int *PIDHijo, int *esHijo) {

	int i;

	/* Creamos los procesos hijos. */
	for (i = 0; i < nProc; i++) {

		PIDHijo[i] = fork();

		switch(PIDHijo[i]) {

			case 0:
				*esHijo = 1;
				son_closeNonUsedPipes(tubPadre, tubHijo, nProc, i);
				break;

			case -1:
				printf("Error al crear proceso.\n");
				*esHijo = 1;
				break;

			default: 
				break;
		}

		if(*esHijo == 1)
			break;
	}

	return i;
}

