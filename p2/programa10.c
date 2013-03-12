#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

#define CLAVE_SEMAFORO 2000
/* Completar las definiciones de constantes */
#define CLAVE_MEMORIA 1000
#define SEM_MUTEX 0
#define NUM_SEMAFOROS 1

int down(int id, int num_sem){
    struct sembuf sops;
    sops.sem_num=(short)num_sem;
    sops.sem_op=-1;
    sops.sem_flg=0;
    return semop(id, &sops, NUM_SEMAFOROS);
}


int up(int id, int num_sem){
    struct sembuf sops;
    sops.sem_num=(short)num_sem;
    sops.sem_op=1;
    sops.sem_flg=0;
    return semop(id, &sops, NUM_SEMAFOROS);
}


int main()
{
    unsigned short val = 1;
    int semid, shm;
    int *dir = NULL;

    /* Creamos los semaforos. */
    semid = semget(CLAVE_SEMAFORO, NUM_SEMAFOROS, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    if ((semid == -1) && (errno == EEXIST))
        semid = semget(CLAVE_SEMAFORO, NUM_SEMAFOROS, SHM_R | SHM_W);
    if (semid == -1) {

        perror("semget");
        exit(-1);
    }


    /* Inicializamos SEM_MUTEX a 1 */
    if(semctl(semid, SEM_MUTEX, SETVAL, val) == -1) {

        perror("semctl");
        semctl(semid, 0, IPC_RMID);
        exit(errno);
    }


    /* Crear una zona de memoria compartida para guardar un entero y obtener el puntero para acceder a ella */
    shm = shmget(CLAVE_MEMORIA, sizeof(int), IPC_CREAT | IPC_EXCL| SHM_R | SHM_W);
    if(shm == -1) {

        if(EEXIST == errno){

            shm = shmget(CLAVE_MEMORIA, sizeof(int), SHM_R | SHM_W);
            if(shm==-1){

                perror("shmget");
                exit(-1);
            }
        } else {

            perror("shmget");
            semctl(semid, 0, IPC_RMID);
            exit(-1);
        }
    }

    /* Obtenemos la direccion de memoria. */
    dir = shmat(shm, NULL, 0);
    if(*dir == -1) {

        perror("shmat");
        semctl(semid, 0, IPC_RMID);
        shmctl(shm, IPC_RMID, NULL);
        exit(-1);
    }


    /* Regulamos el acceso al entero de la memoria compartida. */
    if(down(semid, SEM_MUTEX)==-1) {

        semctl(semid, 0, IPC_RMID);
        shmctl(shm, IPC_RMID, NULL);
        perror("semop");
        exit(-1);
    }

    (*dir) = 2;

    if(up(semid, SEM_MUTEX)==-1) {

        semctl(semid, 0, IPC_RMID);
        shmctl(shm, IPC_RMID, NULL);
        perror("semop");
        exit(-1);
    }


    /* Liberar la memoria compartida y los semaforos */
    if(shmdt(dir) == -1) {

        semctl(semid, 0, IPC_RMID);
        shmctl(shm, IPC_RMID, NULL);
        perror("shmdt");
        exit(-1);
    }
    if(shmctl(shm, IPC_RMID, NULL) == -1) {

        semctl(semid, 0, IPC_RMID);
        perror("shmctl");
        exit(-1);
    }

    if(semctl(semid, 0, IPC_RMID, 0) == -1) { 

        perror("semctl");
        exit(-1);
    }

    exit(0);
}