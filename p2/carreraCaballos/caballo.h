#include "sistema.h"


/*
 * Descripcion: se encarga de registrar al nuevo caballo
 * en la memoria compartida, asignandole su puesto, posicion 
 * y pid.
 *
 * Retorno:
 *     - posicion que ocupa el caballo en el array de memoria compartida.
 *     - -1: si no existe hueco libre (no se deberia de dar)
 */
int registrarse(int **shared, int numCaballos, int semLectura);


/*
 * Descripcion: se encarga de tirar el dado sabiendo el puesto.
 *
 * Retorno:
 *     - tirada del dado/s.
 */
int tirar(int indexArray, int *shared, int numCaballos);


/*
 * Descripcion: se encarga de actualizar el puesto y la
 * posicion del caballo 'indexArray', y tambien del resto
 * de caballos.
 */
void actualizarPosicion(int **shared, int numCaballos, int tirada,
						int indexArray);


/*
 * Descripcion: a esta funcion se le llama cuando un caballo 
 * recibe una interrupcion antes de la barrera. Esto se debe 
 * a que puede que haya procesos metidos en esta que esten es- 
 * perando su llegada, y por tanto hay que sacarlos de dicho 
 * bloqueo.
 */
void sacarDeBarrera();


/*
 * Descripcion: se encarga de gestionar una interrupcion.
 */
void sigInt(int signal);


/*
 * Descripcion: se encarga de realizar la ejecucion de un proceso
 * caballo.
 */
void ejecutaCaballo(int numCaballos, int longitud, int semEntrada,
					int semLectura, int semEscritura, int semPadre,
					int semLeido, int semHijos, int **shared);
