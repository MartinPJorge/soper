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

void iniColaMsg();
void delColaMsg();
int procesarEnvio();
void *funcion_hilo(void *arg);
void *funcion_hilo_respuesta(void *arg);
int comprobarComando(pthread_t* tid_command, pthread_t* tid_resp, int numCaballos);
int mirarMensajesCaballo(int indexArray, int *bloqueado, int semEscritura, int numCaballos);
int tiradaComando(int tipoTirada);
