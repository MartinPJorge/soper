/* Practica 1 de Sistemas Operativos - ejercicio final
 * 
 * autores:
 * 		Ivan Marquez Pardo
 * 		Jorge Martin Perez
 */


#include "procesos.h"
#include "tipos.h"
#include "padre.h"
#include "caballo.h"
#include "procesos.h"
#include "tuberia.h"


int sigAlarma = 0;
int caida = 0;
int stopEverything = 0; 


int main (int argc, char *argv [], char *env []) {
	
	int  **tubPadre=NULL, **tubHijo=NULL, esHijo = 0;
	int posHijo;
	pid_t *PIDHijo;
	int nProc=0, longitud=0;
	sigset_t mask, temp, sonTemp, oldMask;

        
        /* Control de errores */
	if(argc!=3){
		printf("Numero incorrecto de argumentos.\n");
		exit (1);
	}
        nProc = atoi(argv[1]);
	longitud = atoi(argv[2]);

        
        
	crearMascaras(&mask, &temp, &sonTemp, &oldMask);
	gestionarSignals();  /* Llamamos a los distintos 'signal()' */
	makePipes(&tubPadre, &tubHijo, nProc);

	PIDHijo = (pid_t*) malloc (nProc*sizeof(pid_t));
	if(!PIDHijo){
		printf("Error en la reserva de memoria.\n");
		exit (1);
	}
        
        /* Creamos los procesos de los caballos */
	posHijo = crearCaballos(nProc, tubPadre, tubHijo, PIDHijo, &esHijo);

        
	/* Ejecucion del padre. */	
	if(esHijo != 1)
            ejecutarPadre(tubPadre, tubHijo, nProc, PIDHijo, &temp, longitud);

	/* Procesos hijos (caballos) */
	else
            ejecutarCaballo(&sonTemp, tubPadre, tubHijo, nProc, PIDHijo, 
                    posHijo);


	exit(0);
}

