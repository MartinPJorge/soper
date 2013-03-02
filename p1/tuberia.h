/* 
 * File:   tuberia.h
 * Author: Jorge
 *
 * Created on 26 de febrero de 2013, 23:13
 */

#ifndef TUBERIA_H
#define	TUBERIA_H

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
void makePipes(int ***tubPadre, int ***tubHijo, int nProc);


#ifdef	__cplusplus
}
#endif

#endif	/* TUBERIA_H */

