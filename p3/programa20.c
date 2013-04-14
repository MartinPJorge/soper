#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


#define NUM_HILOS 1

void captura1(int signal){
    printf("Soy el Hilo Principal, con TID = %u; Yo he capturado SIGINT.\n", (unsigned short)pthread_self());
    return;
}

void captura2(int signal){
    printf("Soy el Hilo Secundario, con TID = %u; Yo he capturado SIGINT.\n", (unsigned short)pthread_self());
    return;
}

void *funcion_hilo(void *id) {
    static int cont = 0;

    if (signal (SIGINT, captura2) == SIG_ERR) {   
        puts ("Error en la captura");
        exit (1); 
    }

    pause();

    printf("Soy el Hilo Secundario y acabo de salir del pause.\n");

    pthread_exit(NULL);
}

int main() {
    pthread_t tid;

    if (signal (SIGINT, captura1) == SIG_ERR) {
        puts ("Error en la captura");
        exit (1); 
    }

    pthread_create(&tid, NULL, funcion_hilo, NULL);

    pause();

    printf("Soy el Hilo Principal y acabo de salir del pause.\n");

    pthread_join(tid, NULL);

    pthread_exit(NULL);
}

