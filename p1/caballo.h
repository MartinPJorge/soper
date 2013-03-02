/* 
 * File:   caballo.h
 * Author: Jorge
 *
 * Created on 26 de febrero de 2013, 22:00
 */

#ifndef CABALLO_H
#define	CABALLO_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "tipos.h"
    
    

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
void son_closeNonUsedPipes(int **tubPadre, int **tubHijo, int nProc, int index);



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
int tirarDado(int tipoTirada);


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
void retirarCaballo(int **tubPadre, int **tubHijo, int *PIDHijo, int nProc);


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
        int nProc, int *PIDHijo, int posHijo);


#ifdef	__cplusplus
}
#endif

#endif	/* CABALLO_H */

