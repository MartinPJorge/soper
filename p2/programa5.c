#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

#define NUM_SEMAFOROS 10
#define CLAVE_SEMAFORO 2000

int main()
{
    int semid;
    int i;
    int valores[NUM_SEMAFOROS] = {1,2,3,4,5,6,7,8,9,10};


    semid = semget(CLAVE_SEMAFORO, NUM_SEMAFOROS, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    if ((semid == -1) && (errno == EEXIST))
        semid = semget(CLAVE_SEMAFORO, NUM_SEMAFOROS, SHM_R | SHM_W);
    if (semid == -1)
    {
        perror("semget");
        exit(errno);
    }

    if(semctl(semid, 2143, SETALL, valores) == -1)
        printf("Error\n");

    exit(0);
}
