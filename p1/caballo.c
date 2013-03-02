#include "tipos.h"
#include "caballo.h"

extern int sigAlarma;
extern int caida;
extern int stopEverything;


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
 * 		ejecutarCaballo
 * descripcion:
 * 		funcion encargada de la ejecucion del padre
 *
 * parametros:
 *              - sonTemp: mascara temporal del caballo para 'sigsuspend()'
 * 		- tubPadre: array con las tuberias de comunicacion del padre al hijo
 * 		- tubHijo: array con las tuberias de comunicacion del hijo al padre
 * 		- nProc: numero de procesos hijos
 * 		- PIDHijo: array con los PID de los procesos hijos
 *              - posHijo: posicion del caballo en el array 'PIDHijo'
 */
void ejecutarCaballo(sigset_t *sonTemp, int **tubPadre, int **tubHijo,
        int nProc, int *PIDHijo, int posHijo) {
    
    char buffer[TAM];
    int tipoTirada, resulDado;
    
    srand(getpid());

    /* El hijo tira siempre hasta que el padre le manda parar. */
    while(1) {

            sigsuspend(sonTemp);

            if(caida)
                    retirarCaballo(tubPadre, tubHijo, PIDHijo, nProc);

            read(tubPadre[posHijo][0], buffer, TAM);
            tipoTirada = atoi(buffer);

            resulDado = tirarDado(tipoTirada);
            snprintf(buffer, TAM, "%d", resulDado);
            write (tubHijo[posHijo][1], buffer, strlen(buffer) + 1);

            kill(getppid(), SIGUSR1);
    }

}
