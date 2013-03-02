/* 
 * File:   tipos.h
 * Author: Jorge
 *
 * Created on 26 de febrero de 2013, 21:24
 */

#ifndef TIPOS_H
#define	TIPOS_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum {T_NORMAL = 1, T_REMONTADORA, T_GANADORA} Tirada;
typedef enum {TIRANDO = 0, ESPERANDO, CAIDO} HorseStatus;
typedef enum {TRUE, FALSE} BOOL;

#define TAM 60
#define T_ESPERA 5


#ifdef	__cplusplus
}
#endif

#endif	/* TIPOS_H */

