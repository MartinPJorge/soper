#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLAVE_MENSAJES 1555

#define SIZE 1000
#define LOOP 5

typedef struct msgbuf
{
    long type;
    char msg[SIZE+1];
} MSGBUF;

int main(int argc, char *argv[])
{
    int i;
    int queue;
    int result;
    int type;
    struct msgbuf buffer;
    buffer.msg[SIZE]=0;

    if (argc <= 1)
        type = 1;
    else
        type = atoi(argv[1]);

    queue = msgget(CLAVE_MENSAJES, IPC_CREAT|SHM_R|SHM_W);
    if (queue == -1)
    {
        perror("msgget");
        exit(-1);
    }

    buffer.type = type;
    for (i=0;i<LOOP;i++)
    {
        snprintf(buffer.msg, SIZE, "Hola Mundo! Tipo %d, mensaje %d", type, i);
        result = msgsnd(queue, &buffer, strlen(buffer.msg)+1, 0);
        if (result == -1)
        {
            perror("msgsnd");
            exit(-1);
        }
    }
    exit(0);
}
