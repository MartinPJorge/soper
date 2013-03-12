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
    unsigned short valores[NUM_SEMAFOROS] = {1,2,3,4,5,6,7,8,9,10};
    unsigned short recogeVal[NUM_SEMAFOROS];


    /* Obtenemos el semid de los semaforos creados. */
    semid = semget(CLAVE_SEMAFORO, NUM_SEMAFOROS, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    if ((semid == -1) && (errno == EEXIST))
        semid = semget(CLAVE_SEMAFORO, NUM_SEMAFOROS, SHM_R | SHM_W);
    if (semid == -1)
    {
        perror("semget");
        exit(errno);
    }

    system("ipcs -s");

    /* Asignamos los valores iniciales a los de array proporcionado. */
    if(semctl(semid, 0, SETALL, valores) == -1) {
        perror("semctl");
        exit(errno);
    }
        

    /* Obtenemos los valores de todos los semaforos. */
    if(semctl(semid, 0, GETALL, recogeVal) == -1) {
        perror("semctl");
        exit(errno);
    }

    for(i = 0; i < NUM_SEMAFOROS; i++)
        printf("%d\t", recogeVal[i]);
    printf("\n");


    /* Borramos los semaforos. */
    if(semctl(semid, 0, IPC_RMID) == -1){
        perror("semctl");
        exit(errno);
    }
    
    system("ipcs -s");

    exit(0);
}
