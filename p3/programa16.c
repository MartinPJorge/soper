#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_HILOS 2

char **mensajes_globales;

void *funcion_hilo(void *id)
{
    int tid;
    static int cont = 0;

    tid = *((int *)id);
    printf("[Hilo %d]: %s (cont=%d)\n", tid, mensajes_globales[tid], ++cont);

    return NULL;
}

int main()
{
    char *mensajes_locales[NUM_HILOS] = { "Mensaje A", "Mensaje B" };
    int i, id[NUM_HILOS];
    pthread_t tid;

    mensajes_globales = mensajes_locales;

    for (i=0;i<NUM_HILOS;i++)
    {
        id[i] = i;
        pthread_create(&tid, NULL, funcion_hilo, (void *)(id + i));
    }

    pthread_exit(NULL);
}
