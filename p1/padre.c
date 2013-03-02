#include "padre.h"


extern int sigAlarma;
extern int caida;
extern int stopEverything; 

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
	char buffer[TAM];

	/* Buscamos el primero y el ultimo. */
	for(i = 0; i < nProc; i++) {

		if(estadoCaballos[i] != CAIDO) {

			if(posiciones[i] < min)
				min = posiciones[i];

			else if(posiciones[i] > max)
				max = posiciones[i];

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
			printf("-\t\t");

	printf("\n");

	return;
}


/*
 * funcion:
 * 		mostrarGanador
 * descripcion:
 * 		funcion encargada de determinar el caballo ganador de la carrera
 *
 * parametros:
 * 		- posiciones: array con las posiciones de los caballos
 * 		- nProc: numero de procesos hijos
 */
void mostrarGanador(int *posiciones, int nProc) {

	int i, max=0, indexMax=0;

	for (i = 0; i < nProc; i++) {
		
		if(max<=posiciones[i]){
			indexMax=i;
			max=posiciones[i];
		}
	}

	printf("\nEl ganador es el Caballo%d.\n", indexMax);
	 
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
 * 		liberarPipes
 * descripcion:
 * 		funcion encargada de liberar la memoria de los pipes para el padre
 *
 * parametros:
 * 		- tubPadre: array con las tuberias de comunicacion del padre al hijo
 * 		- tubHijo: array con las tuberias de comunicacion del hijo al padre
 * 		- nProc: numero de procesos hijos
 */
void liberarPipes(int **tubPadre, int **tubHijo, int nProc) {

	int i;


	for (i = 0; i < nProc; i++) {
		
		free(tubPadre[i]);
		free(tubHijo[i]);
	}

	free(tubPadre);
	free(tubHijo);

	return;
}

/*
 * funcion:
 * 		abortProgram
 * descripcion:
 * 		funcion que se llamará cuando el programa reciba un SIGINT y liberará la memoria reservada
 */
void abortProgram(int *PIDHijo, int nProc, int *estadoCaballos){
	int i;
	int status;
	for(i=0; i<nProc; ++i){
		if(estadoCaballos[i] != CAIDO)
			kill(PIDHijo[i], SIGILL);		
		if(waitpid(PIDHijo[i], &status, WNOHANG)==-1){
			printf("El proceso %d no ha terminado correctamente.\n", PIDHijo[i]);
		}
	}
}


/*
 * funcion:
 * 		ejecutarPadre
 * descripcion:
 * 		funcion encargada de la ejecucion del padre
 *
 * parametros:
 * 		- tubPadre: array con las tuberias de comunicacion del padre al hijo
 * 		- tubHijo: array con las tuberias de comunicacion del hijo al padre
 * 		- nProc: numero de procesos hijos
 *              - PIDHijo: array con los PID de los procesos hijos
 *              - temp: mascara temporal para el 'sigsuspend()'
 *              - longitud: longitud de la carrera
 */
void ejecutarPadre(int **tubPadre, int **tubHijo, int nProc, int *PIDHijo, 
                sigset_t *temp, int longitud) {

	int i, *posiciones, *estadoCaballos;
	int finCarrera = FALSE;
        BOOL todosResponden = FALSE;


	parent_closeNonUsedPipes(tubPadre, tubHijo, nProc);

	for(i = 0; i < nProc; i++)
		printf("Caballo%d\t", i);
	printf("\n");

	posiciones = (int *) calloc(nProc, sizeof(int));
	estadoCaballos = (int *) calloc(nProc, sizeof(int));


	while(finCarrera == FALSE && !stopEverything) {

		ponerEnModoTirando(estadoCaballos, nProc);
		tirada(posiciones, PIDHijo, tubPadre, tubHijo, nProc, estadoCaballos);
		

		alarm(T_ESPERA);
		sigAlarma = 0;
		todosResponden = FALSE;

		while((todosResponden == FALSE) && !sigAlarma && !stopEverything) {

			sigsuspend(temp);


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

	if(!stopEverything){
		mostrarGanador(posiciones, nProc);
		sacarCaballos(PIDHijo, nProc, estadoCaballos);
	}else{
		printf("\nDeteniendo programa...\n");
		abortProgram(PIDHijo, nProc, estadoCaballos);
	}

	/* Liberamos memoria del padre */
	liberarPipes(tubPadre, tubHijo, nProc);
	free(posiciones);
	free(estadoCaballos);
	free(PIDHijo);
}
