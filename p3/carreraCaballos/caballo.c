#include "caballo.h"


int enCarrera; /* Sirve para saber si el caballo ha entrado */
int *compartido; /* puntero al bloque de memoria compartida */
int indiceLugar; /* Sirve para saber la posicion que ocupa en la mem. compartida */
int semEntradaG; /* semID de semEntrada */
int semHijosG; /* semID de semHijos */
int semLeidoG; /* semID de semLeido */
int semEscrituraG; /* semId de semEscritura */
int numCaballosG; /* numero max. de caballos */
int semPadreG;
int semLecturaG;

/*
 * Descripcion: se encarga de registrar al nuevo caballo
 * en la memoria compartida, asignandole su puesto, posicion 
 * y pid.
 *
 * Retorno:
 *     - posicion que ocupa el caballo en el array de memoria compartida.
 *     - -1: si no existe hueco libre (no se deberia de dar)
 */
int registrarse(int **shared, int numCaballos, int semLectura) {

    int i, ultimo = -1, indexUltimo = -1;
    int llegaTarde = 0;

    /* Buscamos cual es el ultimo puesto */
    for (i = 0; i < numCaballos; i++)
        if (puesto(*shared, i, numCaballos) >= ultimo) {

            indexUltimo = i;
            ultimo = puesto(*shared, i, numCaballos);
        }

    /* Buscamos una posicion libre */
    for (i = 0; i < numCaballos; i++)
        if (pid(*shared, i, numCaballos) == -1)
            break;


    /* Miramos si se incorpora tarde. */
    downSimetrico(semLectura, 0);
    llegaTarde = (round(*shared, numCaballos) == 0) ? 0 : 1;
    upSimetrico(semLectura, 0);

    /* Asignamos los valores que tendra. */
    pid(*shared, i, numCaballos) = getpid();
    puesto(*shared, i, numCaballos) = ultimo + 1;
    if ((llegaTarde == 1) && (ultimo != -1))
        posicion(*shared, i, numCaballos) = posicion(*shared, indexUltimo, numCaballos) - 10;
    else
        posicion(*shared, i, numCaballos) = 0;
    estado(*shared, i, numCaballos) = ANTES_BARRERA;



    return i;
}

/*
 * Descripcion: se encarga de tirar el dado sabiendo el puesto.
 *
 * Retorno:
 *     - tirada del dado/s.
 */
int tirar(int indexArray, int *shared, int numCaballos) {

    int resultado = 0;
    int puestoActual = puesto(shared, indexArray, numCaballos);

    /* Tirada ganadora */
    if (puestoActual == 1)
        resultado = (rand() % 7) + 1;

    /* Tirada remontadora */
    else if (puestoActual == numCaballos) {

        resultado += (rand() % 6) + 1;
        resultado += (rand() % 6) + 1;
    }

    /* Tirada normal */
    else
        resultado = (rand() % 6) + 1;

    return resultado;
}

/*
 * Descripcion: se encarga de actualizar el puesto y la
 * posicion del caballo 'indexArray', y tambien del resto
 * de caballos.
 */
void actualizarPosicion(int **shared, int numCaballos, int tirada,
        int indexArray) {

    int i, delanteSuya = 0, nuevaPos;

    /* Nueva posicion */
    posicion(*shared, indexArray, numCaballos) += tirada;
    nuevaPos = posicion(*shared, indexArray, numCaballos);

    /* Buscamos quienes estan por delante */
    for (i = 0; i < numCaballos; i++)
        if (posicion(*shared, i, numCaballos) > nuevaPos)
            delanteSuya++;

    /* Nuevo puesto */
    puesto(*shared, indexArray, numCaballos) = delanteSuya + 1;


    /* Actualiza los puestos por detras */
    for (i = 0; i < numCaballos; i++)
        if ((puesto(*shared, i, numCaballos) >= puesto(*shared, indexArray, numCaballos)))
            puesto(*shared, i, numCaballos) += 1;

    /* Notificamos que ya hemos tirado */
    estado(*shared, indexArray, numCaballos) = TIRADO;

    return;
}

/*
 * Descripcion: a esta funcion se le llama cuando un caballo 
 * recibe una interrupcion antes de la barrera. Esto se debe 
 * a que puede que haya procesos metidos en esta que esten es- 
 * perando su llegada, y por tanto hay que sacarlos de dicho 
 * bloqueo.
 */
void sacarDeBarrera() {

    int i, enBarrera = 0;


    /* Vamos sacando los que se quedan en la barrera */
    downSimetrico(semLecturaG, 0);
    for (i = 0; i < numCaballosG; i++)
        if (estado(compartido, i, numCaballosG) == EN_BARRERA) {
            enBarrera++;
        }

    /* Si este caballo (el que recibe SIGINT) era el unico que faltaba 
     * por entrar en la barrera, entonces tiene que despertar al resto. */
    if (enBarrera == (caballosCorriendo(compartido, numCaballosG) - 1))
        up(semLeidoG, 0);

    counter(compartido, numCaballosG) += 1;
    upSimetrico(semLecturaG, 0);

    return;
}

/*
 * Descripcion: se encarga de gestionar una interrupcion.
 */
void sigInt(int signal) {

    int i, miPuesto;


    /* Adelantamos un puesto los que ivan por detras */
    downSimetrico(semLecturaG, 0);
    miPuesto = puesto(compartido, indiceLugar, numCaballosG);
    for (i = 0; i < numCaballosG; i++)
        if (puesto(compartido, i, numCaballosG) > miPuesto)
            puesto(compartido, i, numCaballosG) -= 1;
    upSimetrico(semLecturaG, 0);


    /* Lo marcamos como caido. */
    downSimetrico(semEscrituraG, 0);
    puesto(compartido, indiceLugar, numCaballosG) = -1;
    pid(compartido, indiceLugar, numCaballosG) = -1;
    posicion(compartido, indiceLugar, numCaballosG) = -1;
    upSimetrico(semEscrituraG, 0);



    /* Si no estaba en la carrera, salimos. */
    if (enCarrera == 0) {

        shmdt(compartido);
        exit(0);
    }


    downSimetrico(semEscrituraG, 0);
    if (estado(compartido, indiceLugar, numCaballosG) == ANTES_BARRERA)
        sacarDeBarrera();


    /* Le dejamos sin estado */
    estado(compartido, indiceLugar, numCaballosG) = -1;
    upSimetrico(semEscrituraG, 0);


    if(shmdt(compartido)) { /* Nos desenganchamos de la memoria compartida. */
        perror("shmdt");
        exit(1);
    }


    /* Levantamos los semaforos necesarios */
    up(semHijosG, indiceLugar);
    up(semEntradaG, 0);
    up(semPadreG, indiceLugar);


    exit(1);
}

/*
 * Descripcion: se encarga de realizar la ejecucion de un proceso
 * caballo.
 */
void ejecutaCaballo(int numCaballos, int longitud, int semEntrada,
        int semLectura, int semEscritura, int semPadre,
        int semLeido, int semHijos, int **shared) {

    int indexArray, tirada;
    int bloqueado = 0;
    int tipoTirada = 0;

    /* Proporcionamos el bloque de mem. compartida */
    compartido = *shared;
    semEntradaG = semEntrada;
    semHijosG = semHijos;
    semLeidoG = semLeido;
    semPadreG = semPadre;
    semEscrituraG = semEscritura;
    semLecturaG = semLectura;
    numCaballosG = numCaballos;

    srand(getpid());
    enCarrera = 0; /* Todavia no esta en carrera */

    if (signal(SIGINT, sigInt) == SIG_ERR) {

        puts("Error en la captura");
        exit(1);
    }


    down(semEntrada, 0);
    enCarrera = 1; /* Ya ha entrado en la carrera */



    /* Registramos al caballo. */
    downSimetrico(semEscritura, 0);
    indexArray = registrarse(shared, numCaballos, semLectura);
    indiceLugar = indexArray;
    upSimetrico(semEscritura, 0);

    printf("Entra caballo %d\n", getpid());//fflush(NULL);

    while (1) {

        downSimetrico(semEscritura, 0);
        estado(*shared, indexArray, numCaballos) = ANTES_BARRERA;
        upSimetrico(semEscritura, 0);


        down(semHijos, indexArray);

        /* Miramos si tenemos algún mensaje para nosotros */
        tipoTirada = mirarMensajesCaballo(indexArray, &bloqueado, semEscritura, numCaballos);

        /* Miramos si han matado este caballo */
        if(tipoTirada == 3)
            sigInt(SIGINT);


        tirada = 0;
        if (bloqueado > 0)
            --bloqueado;

        if (bloqueado == 0) {
            /* Realizamos la tirada correspondiente */
            downSimetrico(semLectura, 0);
            if (tipoTirada != 0)
                tirada = tiradaComando(tipoTirada);
            else
                tirada = tirar(indexArray, *shared, numCaballos);
            upSimetrico(semLectura, 0);
        }

        /* SINCRONIZACION */
        barrera(shared, semLeido, semEscritura, numCaballos, indexArray);
        afterBarrera(shared, semEscritura, numCaballos, indexArray);


        /* Actualizamos las posiciones */
        downSimetrico(semEscritura, 0);
        actualizarPosicion(shared, numCaballos, tirada, indexArray);
        upSimetrico(semEscritura, 0);


        /* Subimos el semaforo que le corresponde en el array del padre */
        up(semPadre, indexArray);
    }

    return;
}