#include "principal.h"

int primerTurno;
int numCaballosG;
int *compartido;
int semLecturaG;
int semPadreG;

int interrupcion;

char mensajes_globales[TAM_MSG_BUFF];

/*
 * Descripcion: muestra la cabecera.
 */
void mostrarCabecera(int numCaballos) {
    int i;

    for (i = 0; i < numCaballos; i++)
        printf("Caballo%d\t", i);
    printf("\n");
    //fflush(NULL);

    return;
}

/*
 * Descripcion: imprime por pantalla las posiciones si mostrar==1.
 *
 * Retorno:
 *      -> -1 : no hay ganador
 *      -> en caso de que haya ganador, devuelve su numero
 */
int mostrarResultados(int *shared, int numCaballos, int longitud,
        int semLectura, int mostrar) {

    int i, ganador = -1;
    if (mostrar == 1) {
        /* Imprimimos las posiciones */
        for (i = 0; i < numCaballos; i++)
            if (posicion(shared, i, numCaballos) == -1)
                printf("--\t");
            else
                printf("%d\t", posicion(shared, i, numCaballos));

        printf("\n");
        //fflush(NULL);
    }

    /* Vemos si hay algun ganador */
    for (i = 0; i < numCaballos; i++)
        if (posicion(shared, i, numCaballos) >= longitud)
            ganador = i;



    return ganador;
}

/*
 * Descripcion: sirve para contemplar el caso en el que algun
 * caballo se ha quedado esperando despues de la barrera, mien-
 * tras que el resto estan esperando el nuevo turno.
 *
 * Retorno:
 *      -> 1 : se da la situacion
 *      -> 0 : no se da la situacion
 */
int colgadosTrasBarrera() {

    int i;
    int despues = -1, antesBarrera = -1;
    int estado;


    downSimetrico(semLecturaG, 0);

    /* Buscamos los que esten tirando */
    for (i = 0; i < numCaballosG; i++)
        if (pid(compartido, i, numCaballosG) != -1) {

            estado = estado(compartido, i, numCaballosG);
            if ((estado == DESPUES_BARRERA) || (estado == TIRADO))
                despues = 1;

            else if (estado(compartido, i, numCaballosG) == ANTES_BARRERA)
                antesBarrera = 1;
        }


    /* Si no estamos en la situacion */
    if ((despues == -1) || (antesBarrera == -1))
        return 0;



    /* A los caballos que tardan, los interrumpimos */
    for (i = 0; i < numCaballosG; i++)
        if (pid(compartido, i, numCaballosG) != -1) {

            estado = estado(compartido, i, numCaballosG);
            if ((estado == DESPUES_BARRERA) || (estado == TIRADO))
                kill(pid(compartido, i, numCaballosG), SIGINT);
        }


    upSimetrico(semLecturaG, 0);

    return 1;
}

/*
 * Descripcion: gestiona las caidas de caballos en la carrera.
 * En caso de que haya muchos caballos bloqueados en el mismo
 * estado, la funcion retira al primero que se encuentre.
 *
 * Retorno:
 *      -> 1 : se ha dado la situacion de colgados tras barrera
 *      -> 0 : no se da la situacion de colgados tras barrera
 */
int comprobarCaidas() {

    int i, max = ANTES_BARRERA;



    /* Si algun caballo se ha colgado tras tirar */
    if (colgadosTrasBarrera() == 1)
        return 1;



    downSimetrico(semLecturaG, 0);

    /* Miramos cual es el estado mas avanzado. */
    for (i = 0; i < numCaballosG; i++)
        if (pid(compartido, i, numCaballosG) != -1)
            if (estado(compartido, i, numCaballosG) > max)
                max = estado(compartido, i, numCaballosG);


    /* Interrumpimos a los caballos que se retrasan */
    for (i = 0; i < numCaballosG; i++)
        if (pid(compartido, i, numCaballosG) != -1)
            if (estado(compartido, i, numCaballosG) < max)
                kill(pid(compartido, i, numCaballosG), SIGINT);


    /* En caso de que se queden todos retrasados por culpa de uno,
     * sacamos al primero de los que esten en ese estado. */
    for (i = 0; i < numCaballosG; i++)
        if (pid(compartido, i, numCaballosG) != -1)
            if (estado(compartido, i, numCaballosG) == max) {

                kill(pid(compartido, i, numCaballosG), SIGINT);
                break;
            }



    upSimetrico(semLecturaG, 0);

    return 0;
}

/*
 * Descripcion: gestiona la llegada de una alarma.
 */
void sigAlarma(int signal) {

    /* Si es la primera espera, salimos. */
    if (primerTurno == 1) {

        primerTurno = 0;
        return;
    }

    comprobarCaidas();

    return;
}

/*
 * Descripcion: gestiona una senal de interrupcion.
 */
void sigInterrupt(int signal) {

    interrupcion = 1;

    return;
}

/*
 * Descripcion: inserta un elemento en un array, y en caso de
 * que no entre reserva nueva memoria, y lo introduce.
 *
 * Retorno:
 *      -> el nuevo tamano del array
 */
int insertar(int **array, int elemento, int size) {

    int i;
    int newSize = size;

    /* Intentamos meter el elemento */
    for (i = 0; i < size; i++)
        if ((*array)[i] == 0)
            (*array)[i] = elemento;

    /* No hay hueco libre */
    if (i == size) {

        newSize = 2 * size;
        *array = realloc(*array, newSize);
        (*array)[size] = elemento;
    }

    return newSize;
}

/*
 * Descripcion: busca un elemento en un array.
 *
 * Retorno:
 *      -> 1 : se ha encontrado
 *      -> 0 : no se ha encontrado
 */
int buscar(int *array, int elemento, int size) {

    int i;

    for (i = 0; i < size; i++)
        if (array[i] == elemento)
            return 1;

    return 0;
}

/*
 * Descripcion: se encarga de retirar todos los caballos, tanto 
 * los que estan en la carrera, como los que estan esperando.
 */
void retirarCaballos(int *shared, int numCaballos, int semLectura,
        int semEscritura, int shm) {

    int i, sizeIntPIDs = numCaballos;
    int *intPIDs = (int *) calloc(numCaballos, sizeof (int));
    int encontrado = 0;
    struct shmid_ds infoSHM;

    shmctl(shm, IPC_STAT, &infoSHM);

    while (infoSHM.shm_nattch > 1) {
        for (i = 0; i < numCaballos; i++) {

            downSimetrico(semLectura, 0);
            if (pid(shared, i, numCaballos) != -1) {

                encontrado = buscar(intPIDs, pid(shared, i, numCaballos), sizeIntPIDs);

                if (encontrado == 0) {
                    sizeIntPIDs = insertar(&intPIDs, pid(shared, i, numCaballos), sizeIntPIDs);
                    kill(pid(shared, i, numCaballos), SIGINT);
                }
            }
            upSimetrico(semLectura, 0);
        }

        shmctl(shm, IPC_STAT, &infoSHM);
    }


    free(intPIDs);

    return;
}

/*
 * Descripcion: se encarga de gestionar el cierre del simulador.
 */
void finSimulador(int semEntrada, int semLectura, int semEscritura,
        int semPadre, int semLeido, int semHijos,
        int semPrincipio, int *shared, int shm,
        int numCaballos) {

    /* Retiramos todos los caballos */
    retirarCaballos(shared, numCaballos, semLectura, semEscritura, shm);



    /* Eliminamos memoria compartida */
    shmdt(shared);
    shmctl(shm, IPC_RMID, NULL);

    /* Eliminamos los semaforos */
    semctl(semEntrada, 0, IPC_RMID);
    semctl(semLectura, 0, IPC_RMID);
    semctl(semEscritura, 0, IPC_RMID);
    semctl(semPadre, 0, IPC_RMID);
    semctl(semLeido, 0, IPC_RMID);
    semctl(semHijos, 0, IPC_RMID);
    semctl(semPrincipio, 0, IPC_RMID);
    
    delColaMsg();
    
    exit(0);
}

/*
 * Descripcion: revisa los semaforos padre en busca de
 * alguno mayor que 0, para bajarlo a 0.
 */
void revisarSemPadre(int semPadre, int numCaballos) {

    int i, j;

    /* Si algun semaforo no esta a 0, lo bajamos a 0 */
    for (i = 0; i < numCaballos; i++)
        if (semctl(semPadre, i, GETVAL) > 0)
            for (j = 0; j < semctl(semPadre, i, GETVAL); j++)
                down(semPadre, i);

    return;
}

/*
 * Descripcion: se encarga de realizar la ejecucion del  proceso
 * principal.
 */
void ejecutaPrincipal(int numCaballos, int longitud, int semEntrada,
        int semLectura, int semEscritura, int semPadre,
        int semLeido, int semHijos, int semPrincipio,
        int *shared, int shm) {

    int ganador, mostrar = 0;
    numCaballosG = numCaballos;
    compartido = shared;
    semPadreG = semPadre;
    semLecturaG = semLectura;
    interrupcion = 0;
    pthread_t tid, tid_resp;

    primerTurno = 1;
    if (signal(SIGALRM, sigAlarma) == SIG_ERR) {

        puts("Error en la captura");
        exit(1);
    }
    if (signal(SIGINT, sigInterrupt) == SIG_ERR) {

        puts("Error en la captura");
        exit(1);
    }


    /* Esperamos a que entren caballos. */
    alarm(7);
    pause();

    /* Tras la entrada, ponemos los semPadre a 0 */
    downTodos(shared, semPadre, numCaballos, semLectura);


    mostrarCabecera(numCaballos);
    //fflush(NULL);


    /*Creamos el hilo lector de comandos*/
    pthread_create(&tid, NULL, funcion_hilo, compartido);

    while (1) {

        sleep(1);
        alarm(5); /* Ponemos la alarma */

        mostrar = comprobarComando(&tid, &tid_resp, numCaballos);

        /* Mostrar resultados */
        downSimetrico(semLectura, 0);
        ganador = mostrarResultados(shared, numCaballos, longitud, semLectura, mostrar);
        upSimetrico(semLectura, 0);
        

        //fflush(NULL);

        if ((ganador != -1) || interrupcion) {

            /* Desactivamos la alarma */
            alarm(0);

            if (ganador != -1)
                printf("\nEl caballo %d ha ganado.\n", ganador);

            finSimulador(semEntrada, semLectura, semEscritura,
                    semPadre, semLeido, semHijos,
                    semPrincipio, shared, shm, numCaballos);
        }

        //printf("El padre sale del bloqueo\n");

        upTodos(shared, semHijos, numCaballos);
        downTodos(shared, semPadre, numCaballos, semLectura);
        revisarSemPadre(semPadre, numCaballos);



        /* Incrementamos #rondas */
        downSimetrico(semEscritura, 0);
        round(shared, numCaballos) += 1;
        upSimetrico(semEscritura, 0);
    }

    return;
}
