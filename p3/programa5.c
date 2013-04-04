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

typedef struct msgbuf
{
    long type;
    char msg[SIZE];
} MSGBUF;

int main()
{
    int i = 0;
    int queue;
    int result;
    struct msgbuf buffer;

    queue = msgget(CLAVE_MENSAJES, IPC_CREAT|SHM_R|SHM_W);
    if (queue == -1)
    {
        perror("msgget");
        exit(-1);
    }

    while (1)
    {
        result = msgrcv(queue, &buffer, SIZE, 0, 0);
        if (result == -1)
        {
            perror("msgrcv");
            exit(-1);
        }
        printf ("Mensaje %d dice: %s\n", i++, buffer.msg);
    }

    exit(0);
}
