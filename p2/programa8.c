#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

#define CLAVE_SEMAFORO 2000

int down(int id, int num_sem){
    struct sembuf sops;
    sops.sem_num=(short)num_sem;
    sops.sem_op=-1;
    sops.sem_flg=0;
    return semop(id, &sops, 1);
}

int main(){
    return(0);
}