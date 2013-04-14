#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_HILOS 5
#define TIME_SLEEP 1e6

void *imprimir_saludo(void *id)
{
    int tid;

    tid = *((int *)id);
    usleep(TIME_SLEEP);
    printf("Hilo %d - %lu\n", tid, pthread_self()); fflush(stdout);
    pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
    int i, id[NUM_HILOS];
    pthread_t hilos[NUM_HILOS];

    for (i=0;i<NUM_HILOS;i++)
    {
        printf("Creando hilo %d\n", i); fflush(stdout);
        id[i] = i;
        
        if(pthread_create(hilos + i, NULL, imprimir_saludo, (void *) (id + i)) != 0) {
            perror("pthread_create");
            exit(1);
        }
    }
    pthread_exit(NULL);
}
