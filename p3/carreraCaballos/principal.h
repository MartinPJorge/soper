#include "mensajes.h"

/*
 * Descripcion: muestra la cabecera.
 */
void mostrarCabecera(int numCaballos);


/*
 * Descripcion: imprime por pantalla las posiciones.
 */
int mostrarResultados(int *shared, int numCaballos, int longitud,
        int semLectura, int mostrar);


/*
 * Descripcion: sirve para contemplar el caso en el que algun
 * caballo se ha quedado esperando despues de la barrera, mien-
 * tras que el resto estan esperando el nuevo turno.
 *
 * Retorno:
 *      -> 1 : se da la situacion
 *      -> 0 : no se da la situacion
 */
int colgadosTrasBarrera();


/*
 * Descripcion: gestiona las caidas de caballos en la carrera.
 * En caso de que haya muchos caballos bloqueados en el mismo
 * estado, la funcion retira al primero que se encuentre.
 *
 * Retorno:
 *      -> 1 : se ha dado la situacion de colgados tras barrera
 *      -> 0 : no se da la situacion de colgados tras barrera
 */
int comprobarCaidas();


/*
 * Descripcion: gestiona la llegada de una alarma.
 */
void sigAlarma(int signal);


/*
 * Descripcion: gestiona una senal de interrupcion.
 */
void sigInterrupt(int signal);


/*
 * Descripcion: inserta un elemento en un array, y en caso de
 * que no entre reserva nueva memoria, y lo introduce.
 *
 * Retorno:
 *      -> el nuevo tamano del array
 */
int insertar(int **array, int elemento, int size);


/*
 * Descripcion: busca un elemento en un array.
 *
 * Retorno:
 *      -> 1 : se ha encontrado
 *      -> 0 : no se ha encontrado
 */
int buscar(int *array, int elemento, int size);


/*
 * Descripcion: se encarga de retirar todos los caballos, tanto 
 * los que estan en la carrera, como los que estan esperando.
 */
void retirarCaballos(int *shared, int numCaballos, int semLectura,
        int semEscritura, int shm);


/*
 * Descripcion: se encarga de gestionar el cierre del simulador.
 */
void finSimulador(int semEntrada, int semLectura, int semEscritura,
        int semPadre, int semLeido, int semHijos,
        int semPrincipio, int *shared, int shm,
        int numCaballos);


/*
 * Descripcion: revisa los semaforos padre en busca de
 * alguno mayor que 0, para bajarlo a 0.
 */
void revisarSemPadre(int semPadre, int numCaballos);


/*
 * Descripcion: se encarga de realizar la ejecucion del  proceso
 * principal.
 */
void ejecutaPrincipal(int numCaballos, int longitud, int semEntrada,
        int semLectura, int semEscritura, int semPadre,
        int semLeido, int semHijos, int semPrincipio,
        int *shared, int shm);
