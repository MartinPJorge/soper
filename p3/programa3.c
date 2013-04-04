#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLAVE_MENSAJES 1555

typedef struct msgbuf
{
    long type;
    char msg[20];
} MSGBUF;

int main()
{
    int queue;
    int result;
    struct msgbuf buffer;

    queue = msgget(CLAVE_MENSAJES, IPC_CREAT|SHM_R|SHM_W);
    if (queue == -1)
    {
        perror("msgget");
        exit(-1);
    }

    buffer.type = 1;
    strcpy(buffer.msg, "Hola Mundo!");
    result = msgsnd(queue, &buffer, strlen(buffer.msg)+1, 0);
    if (result == -1)
    {
        perror("msgsnd");
        exit(-1);
    }

    /* Eliminamos la cola de mensajes. */
    if(msgctl(queue, IPC_RMID, NULL) == -1) {

        perror("msgctl");
        exit(-1);
    }

    exit(0);
}