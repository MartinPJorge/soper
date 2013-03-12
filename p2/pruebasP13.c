#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define CLAVE_SEMAFORO 2000
/* Completar las definiciones de constantes */
#define CLAVE_MEMORIA 1000
#define SEM_MUTEX 0
#define NUM_SEMAFOROS 1


/* Variables globales */
int capturaSignal = 0;


void capturaSIGINT(int sennal) {
    printf("He capturado SIGINT\n");

    capturaSignal = 1;

    return;
}


int down(int id, int num_sem){
    struct sembuf sops;
    int retSemOp;


    capturaSignal = 0;

    if(signal(SIGINT, capturaSIGINT) == SIG_ERR) {

        puts("Error en la captura.\n");
        exit(1);
    }

    sops.sem_num = (short) num_sem;
    sops.sem_op = -1;
    sops.sem_flg = 0;
    retSemOp = semop(id, &sops, NUM_SEMAFOROS);

    if(capturaSignal == 1)
        return down(id, num_sem);

    return retSemOp;
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
    int PID;

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


    /* Creamos el hijo. */
    PID = fork();

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

    /*if(up(semid, SEM_MUTEX)==-1) {

        semctl(semid, 0, IPC_RMID);
        shmctl(shm, IPC_RMID, NULL);
        perror("semop");
        exit(-1);
    }*/

    wait(NULL);


    /* Liberar la memoria compartida y los semaforos */
    if(shmdt(dir) == -1) {

        semctl(semid, 0, IPC_RMID);
        shmctl(shm, IPC_RMID, NULL);
        perror("shmdt");
        exit(-1);
    }


    /* El padre libera la memoria de semaforos, y la mem. compartida. */
    if(PID != -1) {

        if(shmctl(shm, IPC_RMID, NULL) == -1) {

            semctl(semid, 0, IPC_RMID);
            perror("shmctl");
            exit(-1);
        }

        if(semctl(semid, 0, IPC_RMID, 0) == -1) { 

            perror("semctl");
            exit(-1);
        }
    }
    
    exit(0);
}