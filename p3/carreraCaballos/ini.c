#include "ini.h"

/*
 * Descripcion: se encarga de obtener los identificadores de los semaforos, y decide 
 * si el proceso que llama a la funcion es el principal o un caballo.
 *
 * Retorno:
 *    - 1 : el proceso es el principal
 *    - 0 : el proceso es un caballo
 */
int iniSemaforos(int numCaballos, int *semEntrada, int *semLectura, int *semEscritura,
        int *semPadre, int *semLeido, int *semHijos, int *semPrincipio) {

    int numCaballosLeer = (numCaballos == 1) ? 1 : (numCaballos / 2);
    int esPadre, i;

    *semEntrada = semget(CLAVE_SEM_ENTRADA, 1, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);


    /* Creamos el semaforo de espera a la inicializacion */
    *semPrincipio = semget(CLAVE_SEM_PPIO, 1, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    if (*semPrincipio == -1)
        *semPrincipio = semget(CLAVE_SEM_PPIO, 1, SHM_R | SHM_W);
    else
        semctl(*semPrincipio, 0, SETVAL, 0);



    /* Hijo */
    if (*semEntrada == -1) {

        /* Si entramos antes que el padre, nos bloqueamos. */
        down(*semPrincipio, 0);
        up(*semPrincipio, 0);


        *semEntrada = semget(CLAVE_SEM_ENTRADA, 1, SHM_R | SHM_W);
        if(*semEntrada == -1) {
            perror("semget");
            exit(1);
        }

        *semLectura = semget(CLAVE_SEM_LECTURA, 1, SHM_R | SHM_W);
        if(*semLectura == -1) {
            perror("semget");
            exit(1);
        }

        *semEscritura = semget(CLAVE_SEM_ESCRITURA, 1, SHM_R | SHM_W);
        if(*semEscritura == -1) {
            perror("semget");
            exit(1);
        }

        *semPadre = semget(CLAVE_SEM_PADRE, numCaballos, SHM_R | SHM_W);
        if(*semEscritura == -1) {
            perror("semget");
            exit(1);
        }

        *semLeido = semget(CLAVE_SEM_LEIDO, numCaballos, SHM_R | SHM_W);
        if(*semLeido == -1) {
            perror("semget");
            exit(1);
        }

        *semHijos = semget(CLAVE_SEM_HIJOS, numCaballos, SHM_R | SHM_W);
        if(*semHijos == -1) {
            perror("semget");
            exit(1);
        }

        esPadre = 0;
    }
    
    /* Padre */
    else {


        *semLectura = semget(CLAVE_SEM_LECTURA, 1, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
        if(*semLectura == -1) {
            perror("semget");
            exit(1);
        }

        *semEscritura = semget(CLAVE_SEM_ESCRITURA, 1, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
        if(*semEscritura == -1) {
            perror("semget");
            exit(1);
        }

        *semPadre = semget(CLAVE_SEM_PADRE, numCaballos, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
        if(*semPadre == -1) {
            perror("semget");
            exit(1);
        }

        *semLeido = semget(CLAVE_SEM_LEIDO, numCaballos, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
        if(*semLeido == -1) {
            perror("semget");
            exit(1);
        }

        *semHijos = semget(CLAVE_SEM_HIJOS, numCaballos, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
        if(*semHijos == -1) {
            perror("semget");
            exit(1);
        }


        /* Inicializamos los semaforos */
        semctl(*semEntrada, 0, SETVAL, numCaballos);
        semctl(*semLectura, 0, SETVAL, numCaballosLeer);
        semctl(*semEscritura, 0, SETVAL, 1);
        semctl(*semPadre, 0, SETALL, 1);
        semctl(*semLeido, 0, SETALL, 0);
        semctl(*semHijos, 0, SETALL, 0);

        for (i = 0; i < numCaballos; i++) {

            semctl(*semHijos, i, SETVAL, 0);
            semctl(*semPadre, i, SETVAL, 1);
            semctl(*semLeido, i, SETVAL, 0);
        }


        esPadre = 1;
    }

    return esPadre;
}

/*
 * Descripcion: se encarga de devolver la direccion de memoria dentro 
 * del proceso, en la que se aloja la memoria compartida.
 *
 * Retorono:
 *    - el puntero a la direccion de memoria compartida
 *    - NULL en caso de error
 */
int *iniMemCompartida(int numCaballos, int *semPrincipio, int *shm) {

    int *shMem, i;
    int numeroEnteros = (4 * numCaballos) + 2;
    int shmID = shmget(CLAVE_MEMORIA, numeroEnteros * sizeof (int), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);

    /* Hijo */
    if (shmID == -1) {
        shmID = shmget(CLAVE_MEMORIA, numeroEnteros * sizeof (int), SHM_R | SHM_W);
        if (shmID == -1) {
            perror("shmget");
            exit(1);
        }
        shMem = shmat(shmID, NULL, 0);
        if (shMem == NULL) {
            perror("shmat");
            exit(1);
        }
    }
    
    /* Padre */
    else {
        shMem = shmat(shmID, NULL, 0);
        if (shMem == NULL) {
            perror("shmat");
            exit(1);
        }

        for (i = 0; i < 4 * numCaballos; i++)
            shMem[i] = -1;

        counter(shMem, numCaballos) = 0; /* Contador 'leido' */
        round(shMem, numCaballos) = 0; /* Contador de rondas */


        /* Sacamos del bloqueo a los hijos que entren antes que el padre */
        up(*semPrincipio, 0);
    }

    *shm = shmID;

    return shMem;
}

