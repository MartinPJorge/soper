#include "tuberia.h"

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
