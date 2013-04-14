#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>
#include <string.h>


#define NUM_HILOS 2
#define CLAVE_SEMAFORO 2000
#define NUM_SEMAFOROS 1
#define TIME_SLEEP 1e6
#define TAM 50

char **mensajes_globales;

int down(int id, int num_sem) {
    struct sembuf sops;
    sops.sem_num = (short) num_sem;
    sops.sem_op = -1;
    sops.sem_flg = 0 | SEM_UNDO;
    return semop(id, &sops, NUM_SEMAFOROS);
}

int up(int id, int num_sem) {
    struct sembuf sops;
    sops.sem_num = (short) num_sem;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    return semop(id, &sops, NUM_SEMAFOROS);
}

void *funcion_hilo(void *id) {
    int tid;
    static int cont = 0;
    char saludo[TAM];
    unsigned short val;
    int semID = semget(CLAVE_SEMAFORO, 1, SHM_R | SHM_W);
    if (semID == -1)
        perror("semget");
    
    // La cancelación del hilo pasará a ser asíncrona.
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    tid = *((int *) id);
    val = semctl(semID, 0, GETVAL, &val);
    
    down(semID, 0);
    usleep(TIME_SLEEP);
    printf("[Hilo %d]: %s (cont=%d)\n", tid, mensajes_globales[tid], ++cont);
    up(semID, 0);

    sprintf(saludo, "%s%u", "Hola, soy el hilo con TID = ", (unsigned int) pthread_self());

    pthread_exit((void*) saludo);
}

int main() {
    char *mensajes_locales[NUM_HILOS] = {"Mensaje A", "Mensaje B"};
    int i, id[NUM_HILOS];
    pthread_t tid;
    int semID;
    void* retorno;
    unsigned short val=-1;
    
    
    semID = semget(CLAVE_SEMAFORO, 1, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    if (semID == -1 && errno == EEXIST) {
        semID = semget(CLAVE_SEMAFORO, 1, SHM_R | SHM_W);
        if (semID == -1) {
            printf("Error en semget\n");
            exit(1);
        }
    }
    
    //Inicializamos el semáforo a 1
    if(semctl(semID, 0, SETVAL, 1) == -1) {
        perror("semctl");
        exit(errno);
    }
    
    mensajes_globales = mensajes_locales;

    
    for (i = 0; i < NUM_HILOS; i++) {
        id[i] = i;
        pthread_create(&tid, NULL, funcion_hilo, (void *) (id + i));

        if (pthread_cancel(tid) == 0) {
            if (pthread_join(tid, &retorno) != 0) {
                exit(1);
            }
            if (retorno == PTHREAD_CANCELED) {
                printf("El hilo %d ha sido cancelado.\n", id[i]);
            } else {
                printf("\t%s\n", (char*) (retorno));
            }
        } 
        
    }
    
    
    semctl(semID, 0, IPC_RMID, NULL);


    pthread_exit(NULL);
}

