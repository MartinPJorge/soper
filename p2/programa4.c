#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define CLAVE_MEMORIA 1555

int main() {

    int status;
    int shm;
    int *dir = NULL;

    shm = shmget(CLAVE_MEMORIA, sizeof(int), IPC_CREAT | IPC_EXCL| SHM_R | SHM_W);
    if (shm == -1) {
        printf("Error en la obtencion del shmid.\n");
        return 1;
    }

    switch(fork()) {

        case -1:
            printf("Error con fork.\n");
            break;

        case 0:
            dir = shmat(shm, NULL, 0);
            shmdt(dir);
            shmctl(shm, IPC_RMID, NULL);
            break;

        default:
            dir = shmat(shm, NULL, 0);
            wait(&status);

            printf("El hijo ha hecho 'shmctl':\n");
            system("ipcs -m");
            fflush(stdout);

            shmdt(dir);

            printf("El padre ha desenganchado de la memoria compartida:\n");
            system("ipcs -m");
            fflush(stdout);
    }

    return 0;
}