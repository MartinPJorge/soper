/* 
 * File:   procesos.h
 * Author: Jorge
 *
 * Created on 26 de febrero de 2013, 23:07
 */

#ifndef PROCESOS_H
#define	PROCESOS_H

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
 * 		capturaUser1
 * descripcion:
 * 		funcion encargada de que hacer tras la captura de SIGUSR1
 */
void capturaUser1(int sennal);


/*
 * funcion:
 * 		capturaCaida
 * descripcion:
 * 		funcion encargada de que hacer tras la captura de la sennal
 * 		que indica la caida de un caballo.
 */
void capturaCaida(int sennal);


/*
 * funcion:
 * 		capturaAlarma
 * descripcion:
 * 		funcion encargada de que hacer tras la captura de SIGUSR1
 */
void capturaAlarma(int sennal);


/*
 * funcion:
 * 		capturaInt
 * descripcion:
 * 		funcion encargada de modificar la variable global encargada de detener todo el programa
 */
void capturaInt(int sennal);


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
			sigset_t *oldMask);


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
void gestionarSignals();


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
		int *PIDHijo, int *esHijo);


#ifdef	__cplusplus
}
#endif

#endif	/* PROCESOS_H */

