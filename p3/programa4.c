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
#define LOOP 2

typedef struct msgbuf
{
    long type;
    char msg[SIZE];
} MSGBUF;

int main()
{
    int i;
    int queue;
    int result;
    struct msgbuf buffer;
    struct msqid_ds buf;
    int loop;

    queue = msgget(CLAVE_MENSAJES, IPC_CREAT|SHM_R|SHM_W);
    if (queue == -1)
    {
        perror("msgget");
        exit(-1);
    }

    buffer.type = 1;
    strncpy(buffer.msg, "Hola Mundo!", SIZE);

    /* Obtenemos el maximo de bytes, y hacemos que loop ervase el tope */
    msgctl(queue, IPC_STAT, &buf);
    printf("Numero max.: %d\n", (int)buf.msg_qbytes);
    loop = ((int)(buf.msg_qbytes / strlen(buffer.msg)+1)) + 1;

    for (i=0;i < loop;i++)
    {
        result = msgsnd(queue, &buffer, strlen(buffer.msg)+1, 0);
        if (result == -1)
        {
            perror("msgsnd");
            exit(-1);
        }
    }

    exit(0);
}
