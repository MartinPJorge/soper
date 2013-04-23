#include "sistema.h"

/*
 * Descripcion: se encarga de obtener los identificadores de los semaforos, y decide 
 * si el proceso que llama a la funcion es el principal o un caballo.
 *
 * Retorno:
 *    - 1 : el proceso es el principal
 *    - 0 : el proceso es un caballo
 */
int iniSemaforos(int numCaballos, int *semEntrada, int *semLectura, int *semEscritura,
        int *semPadre, int *semLeido, int *semHijos, int *semPrincipio);



/*
 * Descripcion: se encarga de devolver la direccion de memoria dentro 
 * del proceso, en la que se aloja la memoria compartida.
 *
 * Retorono:
 *    - el puntero a la direccion de memoria compartida
 *    - NULL en caso de error
 */
int *iniMemCompartida(int numCaballos, int *semPrincipio, int *shm);
