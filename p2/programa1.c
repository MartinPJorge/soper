#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define CLAVE_MEMORIA 2555

int main()
{
    int shm;

    shm = shmget(CLAVE_MEMORIA, sizeof(int), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);


    if (shm == -1)
    {
    	perror("shmget");

    	if(EEXIST == errno)
    		printf("Error, el segmento reservado ya existia.\n");
        
        exit(-1);
    }

    

    exit(0);
}
