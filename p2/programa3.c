#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define CLAVE_MEMORIA 1555

int main()
{
    int * dir = NULL;
    int numero = 2;
    int shm, ret;

    /* Obtenemos el shmid */
    shm = shmget(CLAVE_MEMORIA, sizeof(int), IPC_CREAT | SHM_R | SHM_W);
    if (shm == -1) {

        switch(errno) {

            case EACCES:
                printf("Error, el proceso no tiene permiso para acceder al segmento de memoria compartida solicitada.\n");
                exit(-1);
                break;

            case EINVAL:
                printf("Error, el segmento de memoria compartida solicitada ya existe con un tamano menor al pedido.\n");
                exit(-1);
                break;

            case ENOMEM:
                printf("Error, no queda suficiente memoria para crear el segmento compartido solicitado.\n");
                exit(-1);
                break;

            case ENOSPC:
                printf("Error, el sistema ha alcanzado el limite de identificadores de memoria compartida.\n");
                exit(-1);
                break;

            default:
                printf("Error.\n");
                exit(-1);
                break;
        }
    }

    /* Nos enganchamos al segmento de memoria compartida con id 'shm' */
    dir = (int *) shmat (shm, NULL, 0);
    if(*dir == -1) {

        switch(errno) {

            case EACCES:
                printf("Error, el proceso no tiene permiso para acceder al segmento de memoria compartida solicitada.\n");
                break;

            case EINVAL:
                printf("Error, no se ha especificado un identificador de memoria correcto, o la direccion proporcionada para mapear el segmento es una direccion ilegal.\n");
                break;

            case ENOMEM:
                printf("Error, el proceso no dispone de suficiente memoria para mapear el segmento de memoria compartido.\n");
                break;

            case EMFILE:
                printf("Error, el sistema ha alcanzado el limite de segmentos de memoria compartida.\n");
                break;

            default:
                printf("Error.\n");
                break;
        }

        shmctl(shm, IPC_RMID, NULL);
        exit(-1);
    }


    (*dir) = numero;

    /* Desenganchamos el proceso de la memoria compartida. */
    ret = shmdt(dir);
    if(ret == -1) {

        if(errno == EINVAL)
            printf("Error, la direccion especificada para que el proceso se desenganche, no es el comienzo de un bloque de memoria compartida que tenga mapeado el proceso.\n");
        else
            printf("Error.\n");

        shmctl(shm, IPC_RMID, NULL);
        exit(-1);
    }

    /* Mostramos por terminal los segmentos de memoria compartida, y eliminamos el bloque compartido. */
    system("ipcs -m");
    ret = shmctl(shm, IPC_RMID, NULL);
    if(ret == -1) {

        switch(errno) {

            case EINVAL:
                printf("Error, el id del espacio de memoria compartido no es valido, o el comando especificado para la funcion 'shmctl()' no es valido.\n");
                break;

            case EPERM:
                printf("Error, la llamada para borrar el segmento de memoria no se puede realizar debido a que no se ejecuta como super-usuario.\n");
                break;

            default:
                printf("Error.\n");
                break;
        }

        exit(-1);
    }
    system("ipcs -m");


    exit(0);
}
