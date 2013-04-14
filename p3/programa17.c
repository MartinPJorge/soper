#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>


#define NUM_HILOS 2
#define CLAVE_SEMAFORO 2000
#define NUM_SEMAFOROS 1
#define TIME_SLEEP 1e6

char **mensajes_globales;


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


void *funcion_hilo(void *id)
{
    int tid;
    static int cont = 0;
    int semID = semget(CLAVE_SEMAFORO, 1, SHM_R | SHM_W);

    if(semID == -1)
    	perror("semget");

    tid = *((int *)id);

    //Inicio de la region critica
    down(semID,0);
    usleep(TIME_SLEEP);
    printf("[Hilo %d]: %s (cont=%d)\n", tid, mensajes_globales[tid], ++cont);
    up(semID,0);
    //Fin de la region critica
    
    pthread_exit(NULL);
}

int main()
{
    char *mensajes_locales[NUM_HILOS] = { "Mensaje A", "Mensaje B" };
    int i, id[NUM_HILOS];
    pthread_t tid;
    int semID;

    semID = semget(CLAVE_SEMAFORO, 1, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    semctl(semID, 0, SETVAL, 1);
    mensajes_globales = mensajes_locales;

    for (i=0;i<NUM_HILOS;i++)
    {
        id[i] = i;
        pthread_create(&tid, NULL, funcion_hilo, (void *)(id + i));

        pthread_join(tid, NULL);
    }

    semctl(semID, 0, IPC_RMID, NULL);

    pthread_exit(NULL);
}
