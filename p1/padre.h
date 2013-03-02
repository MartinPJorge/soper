/* 
 * File:   padre.h
 * Author: Jorge
 *
 * Created on 26 de febrero de 2013, 21:32
 */

#ifndef PADRE_H
#define	PADRE_H

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
		int **tubHijo, int nProc, int *estadoCaballos);


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
void parent_closeNonUsedPipes(int **tubPadre, int **tubHijo, int nProc);


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
				int nProc, int meta);


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
BOOL finDeTiradas(int *estadoCaballos, int nProc);


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
			int meta);


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
			pid_t *PIDHijo, int nProc);


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
void ponerEnModoTirando(int *estadoCaballos, int nProc);


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
void mostrarPosiciones(int *estadoCaballos, int *posiciones, int nProc, int meta);


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
void mostrarGanador(int *posiciones, int nProc);


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
void sacarCaballos(int *PIDHijo, int nProc, int *estadoCaballos);


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
void liberarPipes(int **tubPadre, int **tubHijo, int nProc);


/*
 * funcion:
 * 		abortProgram
 * descripcion:
 * 		funcion que se llamará cuando el programa reciba un SIGINT y liberará la memoria reservada
 */
void abortProgram(int *PIDHijo, int nProc, int *estadoCaballos);


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
                sigset_t *temp, int longitud);
    

#ifdef	__cplusplus
}
#endif

#endif	/* PADRE_H */

