#include "ini.h"
/*
#include "principal.h"
*/
#include "caballo.h"


int main(int argc, char const **argv) {

    int numCaballos = -1, longitud = -1;
    int semEntrada, semLectura, semEscritura, semPadre, semLeido, semHijos, semPrincipio;
    int esPadre, shm;
    int *shared = NULL;

    /* Numero de parametros incorrecto. */
    if (argc != 3) {

        printf("Numero de parametros incorrecto.\n");
        exit(1);
    }

    /* Obtenemos los parametros */
    numCaballos = atoi(argv[1]);
    longitud = atoi(argv[2]);


    /* INICIALIZACION */
    esPadre = iniSemaforos(numCaballos, &semEntrada, &semLectura, &semEscritura,
            &semPadre, &semLeido, &semHijos, &semPrincipio);
    shared = iniMemCompartida(numCaballos, &semPrincipio, &shm);
    iniColaMsg();


    if (esPadre)
        ejecutaPrincipal(numCaballos, longitud, semEntrada,
            semLectura, semEscritura, semPadre,
            semLeido, semHijos, semPrincipio,
            shared, shm);
    else
        ejecutaCaballo(numCaballos, longitud, semEntrada,
            semLectura, semEscritura, semPadre,
            semLeido, semHijos, &shared);


    return 0;
}



