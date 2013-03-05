#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define CLAVE_MEMORIA 2555

int main()
{
    int * dir = NULL;
    int numero = 2;
    int shm;

    shm = shmget(CLAVE_MEMORIA, sizeof(int), IPC_CREAT | SHM_R | SHM_W);
    if (shm == -1)
    {
        perror("shmget");
        exit(-1);
    }

    dir = (int *) shmat (shm, NULL, 0);
    (*dir) = numero;

    shmdt(dir);
    system("ipcs -m");
    shmctl(shm, IPC_RMID, NULL);
    system("ipcs -m");


    exit(0);
}
