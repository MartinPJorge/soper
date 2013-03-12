#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define CLAVE_SEMAFORO 2000
#define CLAVE_MEMORIA 1000
#define SEM_MUTEX 0
#define NUM_SEMAFOROS 1



/* Variables globales. */
int capturaSignal = 0;


void capturaSIGINT(int sennal) {

    printf("He capturado SIGINT\n");
    capturaSignal = 1;

    return;
}


int down(int id, int num_sem){
    struct sembuf sops;
    int retSemOp;

    capturaSignal = 0; /* La senal no ha saltado todavia. */

    if(signal(SIGINT, capturaSIGINT) == SIG_ERR) {

        puts("Error en la captura.\n");
        exit(1);
    }

    sops.sem_num = (short) num_sem;
    sops.sem_op = -1;
    sops.sem_flg = 0 | SEM_UNDO;
    retSemOp = semop(id, &sops, NUM_SEMAFOROS);

    /* Si sale de la espera por una senal. */
    if(capturaSignal == 1)
        return down(id, num_sem);

    return retSemOp;
}


int up(int id, int num_sem){
    struct sembuf sops;
    sops.sem_num = (short) num_sem;
    sops.sem_op = 1;
    sops.sem_flg = 0 | SEM_UNDO;
    return semop(id, &sops, NUM_SEMAFOROS);
}



int main() {
	
	int semid;
	int val = 1, PID;

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


    PID = fork();


    printf("Proceso: %d ---- Estado: %d\n", getpid(), semctl(semid, 0, GETVAL));

    /* Regulamos el acceso al entero de la memoria compartida. */
    if(down(semid, SEM_MUTEX) == -1) {

        semctl(semid, 0, IPC_RMID);
        perror("semop");
        exit(-1);
    }

    /*-----------------
	 *	REGION CRITICA
	 *-----------------*/

	/* Paramos el padre para que el hijo espere al semaforo. */
	if(PID != 0) {

		sleep(3);
		printf("Numero de semaforos esperando acceso a region critica: %d\n", semctl(semid, 0, GETNCNT));
	}

    if(up(semid, SEM_MUTEX) == -1) {

        semctl(semid, 0, IPC_RMID);
        perror("semop");
        exit(-1);
    }

    if(PID != 0)
    	wait(NULL);


    /* Liberamos la memoria del semaforo. */
    if(PID != 0) {

    	semctl(semid, 0, IPC_RMID);
    	exit(0);
    }

	return 0;
}
