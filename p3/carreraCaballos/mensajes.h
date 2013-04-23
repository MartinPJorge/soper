#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>
#include <string.h>
#include "sistema.h"

#define TIPO_MSG_PADRE 99

typedef struct msgbuf {
    long type;
    char msg[20];

} MSGBUF;


/*
 * Descripcion: inicializa la cola de mensajes que establece la 
 * comunicacion entre los caballos y el proceso ppal.
 */
void iniColaMsg();


/*
 * Descripcion: elimina la cola de mensajes que establece la 
 * comunicacion entre los caballos y el proceso ppal.
 */
void delColaMsg();


/*
 * Descripcion: lee el comando recibido y lo procesa para rellenar una estructura mensaje con
 * el comando a realizar y con el tipo del número de caballo al que va dirigido. Por último,
 * envía el mensaje creado.
 */
int procesarEnvio();


/*
 * Descripcion: se encarga de recoger un comando introducido por un usuario y copiarlo en
 * la variable global.
 */
void *funcion_hilo(void *arg);


/*
 * Descripcion: se encarga de recibir un mensaje enviado al padre e imprimirlo por pantalla.
 */
void *funcion_hilo_respuesta(void *arg);


/*
 * Descripcion: se encarga de comprobar si el usuario ha introducido un comando (el hilo 
 * lector de comandos habrá terminado). Si lo ha hecho, lo cogemos, lo procesamos, 
 * y se lo enviamos al caballo correspondiente. A continuación creamos un hilo que espere
 * la respuesta de dicho caballo, y creamos otro hilo que espere un nuevo comando.
 * El hilo lector de respuestas no será desenganchado (detached), por lo que no habrá
 * que recoger ningún retorno y liberará sus recursos cuando termine.
 *
 * Retorno:
 * 	- Retorna lo que nos devuelva la función procesarEnvio.
 */
int comprobarComando(pthread_t* tid_command, pthread_t* tid_resp, int numCaballos);


/*
 * Descripcion: cada caballo comprueba si existe algún mensaje cuyo tipo coincida con su número en el array
 * de caballos. Si lo hay, lo lee, lo interpreta y envía al padre un mensaje para avisarle de que lo ha leído.
 *
 * Retorno:
 * 	- 0 : fuera de la función, el caballo hará la tirada correspondiente.
 *	- 1 : fuera de la función, el caballo hará una tirada mitad en lugar de la correspondiente.
 *	- 2 : fuera de la función, el caballo hará una tirada doble en lugar de la correspondiente.
 *  - 3 : fuera de la función, el caballo invocará a sigInt
 */
int mirarMensajesCaballo(int indexArray, int *bloqueado, int semEscritura, int numCaballos);


/*
 * Descripcion: se encarga de realizar una tirada concreta, ordenada
 * mediante un comando.
 *
 * Retorno:
 *	- Si tipoTirada es 1, devolvemos una tirada mitad.
 *	- Si tipoTirada es 2, devolvemos una tirada doble.
 */
int tiradaComando(int tipoTirada);
