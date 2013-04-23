#include "sistema.h"

/*
 * Descripcion: se encarga de bajar una posicion
 * el semaforo que ocupa la posicion 'num_sem' 
 * dentro del array de semaforos con 'id' dada.
 *
 * Retorno:
 *     - el valor devuelto por 'semop()'
 */
int down(int id, int num_sem) {

    struct sembuf sops;
    sops.sem_num = (u_short) num_sem;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    return semop(id, &sops, 1);
}

/*
 * Descripcion: se encarga de bajar una posicion
 * el semaforo que ocupa la posicion 'num_sem' 
 * dentro del array de semaforos con 'id' dada.
 * La funcion tiene la particularidad de poner 
 * a bandera 'SEM_UNDO' en la operacion realizada.
 *
 * Retorno:
 *     - el valor devuelto por 'semop()'
 */
int downSimetrico(int id, int num_sem) {

    struct sembuf sops;
    sops.sem_num = (u_short) num_sem;
    sops.sem_op = -1;
    sops.sem_flg = 0 | SEM_UNDO;

    return semop(id, &sops, 1);
}

/*
 * Descripcion: se encarga de bajar todos
 * los semaforos correspondientes a los ca-
 * ballos disponibles.
 *
 * Retorno:
 *     - el valor devuelto por 'semop()'
 */
int downTodos(int *shared, int id, int numCaballos,
        int semLectura) {

    int arrDisponibles[numCaballos];
    int i, retorno;
    struct sembuf *sops = (struct sembuf *) calloc(numCaballos, sizeof (struct sembuf));



    /* Ponemos op = -1 para los disponibles, 0 para el resto */
    downSimetrico(semLectura, 0);
    for (i = 0; i < numCaballos; i++) {

        if (pid(shared, i, numCaballos) == -1)
            arrDisponibles[i] = 0; //-1 * semctl(id, i, GETVAL);
        else
            arrDisponibles[i] = -1;
    }
    upSimetrico(semLectura, 0);


    /* Especificamos las operaciones */
    for (i = 0; i < numCaballos; i++) {

        sops[i].sem_flg = (arrDisponibles[i] == 0) ? IPC_NOWAIT : 0;
        sops[i].sem_num = i;
        sops[i].sem_op = arrDisponibles[i];
    }


    /* Realizamos las operaciones */
    retorno = semop(id, sops, numCaballos);
    if (retorno == -1)
        for (i = 0; i < numCaballos; i++)
            retorno = semop(id, &sops[i], 1);

    free(sops);

    return retorno;
}

/*
 * Descripcion: se encarga de subir una posicion
 * el semaforo que ocupa la posicion 'num_sem' 
 * dentro del array de semaforos con 'id' dada.
 *
 * Retorno:
 *     - el valor devuelto por 'semop()'
 */
int up(int id, int num_sem) {

    struct sembuf sops;
    sops.sem_num = (u_short) num_sem;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    return semop(id, &sops, 1);
}

/*
 * Descripcion: se encarga de subir una posicion
 * el semaforo que ocupa la posicion 'num_sem' 
 * dentro del array de semaforos con 'id' dada.
 * La funcion tiene la particularidad de poner 
 * a bandera 'SEM_UNDO' en la operacion realizada.
 *
 * Retorno:
 *     - el valor devuelto por 'semop()'
 */
int upSimetrico(int id, int num_sem) {

    struct sembuf sops;
    sops.sem_num = (u_short) num_sem;
    sops.sem_op = 1;
    sops.sem_flg = 0 | SEM_UNDO;

    int retorno = semop(id, &sops, 1);


    return retorno;
}

/*
 * Descripcion: se encarga de subir todos
 * los semaforos correspondientes a los ca-
 * ballos disponibles.
 *
 * Retorno:
 *     - el valor devuelto por 'semop()'
 */
int upTodos(int *shared, int id, int numCaballos) {

    int arrDisponibles[numCaballos];
    int i, retorno;
    struct sembuf *sops = (struct sembuf *) calloc(numCaballos, sizeof (struct sembuf));

    /* Ponemos op = -1 para los disponibles, 0 para el resto */
    for (i = 0; i < numCaballos; i++)
        arrDisponibles[i] = (pid(shared, i, numCaballos) != -1) ? 1 : 0;


    /* Especificamos las operaciones */
    for (i = 0; i < numCaballos; i++) {
        sops[i].sem_flg = (arrDisponibles[i] == 0) ? IPC_NOWAIT : 0;
        sops[i].sem_num = i;
        sops[i].sem_op = arrDisponibles[i];
    }

    /* Ejecutamos las operaciones */
    retorno = semop(id, sops, numCaballos);
    if (retorno == -1) /* Si el SO no permite arrays tan largos, vamos 1 a 1 */
        for (i = 0; i < numCaballos; i++)
            retorno = semop(id, &sops[i], 1);

    free(sops);

    return retorno;
}

/*
 * Descripcion: se encarga de contar los caballos
 * que estan corriendo en este momento.
 *
 * Retorno:
 *     - numero de caballos corriendo.
 */
int caballosCorriendo(int *shared, int numCaballos) {

    int i, count = 0;

    for (i = 0; i < numCaballos; i++)
        if (pid(shared, i, numCaballos) != -1)
            count++;

    return count;
}

/*
 * Descripcion: implementacion del patron de sincronizacion 
 * bajo el nombre de 'barrera', que se describe en la version 
 * 2.1.5 del libro 'The Little Book of Semaphores' - p.41
 *
 * Explicacion: el patron se encarga de que no pase ningun caballo 
 * hasta que cada uno de ellos haya pasado por esta funcion.
 */
void barrera(int **shared, int semLeido, int semEscritura,
        int numCaballos, int indexArray) {

    /* Notificamos la entrada en la barrera */
    downSimetrico(semEscritura, 0);
    estado(*shared, indexArray, numCaballos) = EN_BARRERA;
    counter(*shared, numCaballos) += 1;
    upSimetrico(semEscritura, 0);


    if (counter(*shared, numCaballos) == caballosCorriendo(*shared, numCaballos))
        up(semLeido, 0);



    down(semLeido, 0);
    up(semLeido, 0);

    return;
}

/*
 * Descripcion: se encarga de dejar la memoria compartida tal y 
 * como estaba antes de llamar a la funcion 'barrera()', para asi 
 * poder volver a invocar a dicha funcion en sucesivas iteraciones.
 */
void afterBarrera(int **shared, int semEscritura, int numCaballos,
        int indexArray) {


    downSimetrico(semEscritura, 0);
    counter(*shared, numCaballos) = 0;
    estado(*shared, indexArray, numCaballos) = DESPUES_BARRERA;
    upSimetrico(semEscritura, 0);

    return;
}
