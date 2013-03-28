#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>


#define CLAVE_SEM_ENTRADA 2000
#define CLAVE_SEM_LECTURA 2001
#define CLAVE_SEM_HIJOS 2002
#define CLAVE_SEM_PADRE 2003
#define CLAVE_SEM_ESCRITURA 2004
#define CLAVE_SEM_LEIDO 2005
#define CLAVE_SEM_PPIO 2006
#define CLAVE_MEMORIA 1000
#define SEM_MUTEX 0


#define ANTES_BARRERA 2
#define EN_BARRERA 3
#define DESPUES_BARRERA 4
#define TIRADO 5


/* A(punteroSHM, posicion, #caballos) */
#define posicion(shm,pos,numCaballos) ((shm)[pos])
#define puesto(shm,pos,numCaballos) (((shm)[(pos == 0) ? (numCaballos) : (numCaballos+pos)]))
#define pid(shm,pos,numCaballos) (((shm)[(pos == 0) ? (2*numCaballos) : ((2*numCaballos)+pos)]))
#define estado(shm,pos,numCaballos) (((shm)[(pos == 0) ? (3*numCaballos) : ((3*numCaballos)+pos)]))
#define counter(shm,numCaballos) ((shm)[4*numCaballos])
#define round(shm,numCaballos) ((shm)[(4*numCaballos) + 1])



/*
 * Descripcion: se encarga de bajar una posicion
 * el semaforo que ocupa la posicion 'num_sem' 
 * dentro del array de semaforos con 'id' dada.
 *
 * Retorno:
 *     - el valor devuelto por 'semop()'
 */
int down(int id, int num_sem);


/*
 * Descripcion: se encarga de bajar una posicion
 * el semaforo que ocupa la posicion 'num_sem' 
 * dentro del array de semaforos con 'id' dada.
 * La funcion tiene la particularidad de poner 
 * a bandera 'SEM_UNDO' en la operacion realizada.
 *
 * Retorno:
 *     - el valor devuelto por 'semop()'
 */
int downSimetrico(int id, int num_sem);


/*
 * Descripcion: se encarga de bajar todos
 * los semaforos correspondientes a los ca-
 * ballos disponibles.
 *
 * Retorno:
 *     - el valor devuelto por 'semop()'
 */
int downTodos(int *shared, int id, int numCaballos, 
			int semLectura);


/*
 * Descripcion: se encarga de subir una posicion
 * el semaforo que ocupa la posicion 'num_sem' 
 * dentro del array de semaforos con 'id' dada.
 *
 * Retorno:
 *     - el valor devuelto por 'semop()'
 */
int up(int id, int num_sem);


/*
 * Descripcion: se encarga de subir una posicion
 * el semaforo que ocupa la posicion 'num_sem' 
 * dentro del array de semaforos con 'id' dada.
 * La funcion tiene la particularidad de poner 
 * a bandera 'SEM_UNDO' en la operacion realizada.
 *
 * Retorno:
 *     - el valor devuelto por 'semop()'
 */
int upSimetrico(int id, int num_sem);


/*
 * Descripcion: se encarga de subir todos
 * los semaforos correspondientes a los ca-
 * ballos disponibles.
 *
 * Retorno:
 *     - el valor devuelto por 'semop()'
 */
int upTodos(int *shared, int id, int numCaballos);


/*
 * Descripcion: se encarga de contar los caballos
 * que estan corriendo en este momento.
 *
 * Retorno:
 *     - numero de caballos corriendo.
 */
int caballosCorriendo(int *shared, int numCaballos);


/*
 * Descripcion: implementacion del patron de sincronizacion 
 * bajo el nombre de 'barrera', que se describe en la version 
 * 2.1.5 del libro 'The Little Book of Semaphores' - p.41
 *
 * Explicacion: el patron se encarga de que no pase ningun caballo 
 * hasta que cada uno de ellos haya pasado por esta funcion.
 */
void barrera(int **shared, int semLeido, int semEscritura, 
			int numCaballos, int indexArray);


/*
 * Descripcion: se encarga de dejar la memoria compartida tal y 
 * como estaba antes de llamar a la funcion 'barrera()', para asi 
 * poder volver a invocar a dicha funcion en sucesivas iteraciones.
 */
void afterBarrera(int **shared, int semEscritura, int numCaballos,
					int indexArray);


