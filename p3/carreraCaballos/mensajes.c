#include "mensajes.h"

int* compartido;
char msg_buffer[TAM_MSG_BUFF];
int colaMensajes;


/*
 * Descripcion: inicializa la cola de mensajes que establece la 
 * comunicacion entre los caballos y el proceso ppal.
 */
void iniColaMsg(){
    colaMensajes = msgget(CLAVE_MENSAJES, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    if (colaMensajes == -1) {
        colaMensajes = msgget(CLAVE_MENSAJES, SHM_R | SHM_W);
        if (colaMensajes == -1) {
            perror("msgget");
            exit(1);
        }
    }
    return;
}


/*
 * Descripcion: elimina la cola de mensajes que establece la 
 * comunicacion entre los caballos y el proceso ppal.
 */
void delColaMsg(){
    if(msgctl(colaMensajes, IPC_RMID, NULL)==-1){
        perror("msgctl");
        exit(1);
    }
}
/*
 * Descripcion: lee el comando recibido y lo procesa para rellenar una estructura mensaje con
 * el comando a realizar y con el tipo del número de caballo al que va dirigido. Por último,
 * envía el mensaje creado.
 */
int procesarEnvio() {

    char *p, *instruccion = msg_buffer;
    char *comando, *cab = NULL, *n = NULL;
    struct msgbuf buffer;

    /* Todo comando tiene al menos 3 caracteres */
    if (strlen(instruccion) >= 3) {

        if (!strcmp(instruccion, "mostrar\n")) {
            return 1;
        }

        /* Obtenemos el comando */
        p = strtok(instruccion, " ");
        if (p == NULL) {
            printf("Comando no válido.\n");
            return 0;
        }
        comando = strdup(p);

        /* Obtenemos el caballo */
        p = strtok(NULL, " ");
        if (p == NULL) {
            
            printf("Comando no válido.\n");
            return 0;
        }
        cab = strdup(p);


        /* Obtenemos el campo n */
        if ((p = strtok(NULL, " "))!=NULL)
            n = strdup(p);


        /* Enviamos el mensaje */
        buffer.type = atoi(cab) + 1;
        strcpy(buffer.msg, comando);
        if (n != NULL) {
            sprintf(buffer.msg, "%s %d", comando, atoi(n));
        }
        if (msgsnd(colaMensajes, &buffer, strlen(buffer.msg) + 1, 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
    }

    else {
        printf("Comando no válido.\n");
    }
    return 0;
}

/*
 * Descripcion: se encarga de recoger un comando introducido por un usuario y copiarlo en
 * la variable global.
 */
void *funcion_hilo(void *arg) {
    char tmp[TAM_MSG_BUFF] = "";
    compartido = (int*) arg;

    fflush(NULL);
    if(fgets(tmp, TAM_MSG_BUFF, stdin) == NULL) {
        perror("ferror");
    }

    fflush(NULL);

    strcpy(msg_buffer, tmp);

    pthread_exit(NULL);
}

/*
 * Descripcion: se encarga de recibir un mensaje enviado al padre e imprimirlo por pantalla.
 */
void *funcion_hilo_respuesta(void *arg) {
    struct msgbuf buffer;
    
    ssize_t ret = msgrcv(colaMensajes, &buffer, 20, TIPO_MSG_PADRE, 0);
    if (ret == -1 && errno == ENOMSG) {
        return 0;
    } else if (ret == -1) {
        perror("msgrcv");
        exit(1);
    }

    printf("%s\n", buffer.msg);

    pthread_exit(NULL);
}

/*
 * Descripcion: se encarga de comprobar si el usuario ha introducido un comando (el hilo 
 * lector de comandos habrá terminado). Si lo ha hecho, lo cogemos, lo procesamos, 
 * y se lo enviamos al caballo correspondiente. A continuación creamos un hilo que espere
 * la respuesta de dicho caballo, y creamos otro hilo que espere un nuevo comando.
 * El hilo lector de respuestas no será desenganchado (detached), por lo que no habrá
 * que recoger ningún retorno y liberará sus recursos cuando termine.
 *
 * Retorno:
 * 	- Retorna lo que nos devuelva la función procesarEnvio.
 */
int comprobarComando(pthread_t* tid_command, pthread_t* tid_resp, int numCaballos) {
    int ret = 0;

    int retKill = pthread_kill(*tid_command, 0);

    if (retKill != 0) {
        pthread_join(*tid_command, NULL);
        ret = procesarEnvio();

        pthread_create(tid_resp, NULL, funcion_hilo_respuesta, &numCaballos);
        pthread_detach(*tid_resp);
        pthread_create(tid_command, NULL, funcion_hilo, compartido);
    }

    return ret;
}

/*
 * Descripcion: cada caballo comprueba si existe algún mensaje cuyo tipo coincida con su número en el array
 * de caballos. Si lo hay, lo lee, lo interpreta y envía al padre un mensaje para avisarle de que lo ha leído.
 *
 * Retorno:
 * 	- 0 : fuera de la función, el caballo hará la tirada correspondiente.
 *	- 1 : fuera de la función, el caballo hará una tirada mitad en lugar de la correspondiente.
 *	- 2 : fuera de la función, el caballo hará una tirada doble en lugar de la correspondiente.
 *  - 3 : fuera de la función, el caballo invocará a sigInt
 */
int mirarMensajesCaballo(int indexArray, int *bloqueado, int semEscritura, int numCaballos) {
    struct msgbuf buffer;
    struct msgbuf respuesta;
    char *p;
    int pos, ret;

    ssize_t msgret = msgrcv(colaMensajes, &buffer, 20, indexArray + 1, IPC_NOWAIT | MSG_NOERROR);

    if (msgret == -1 && errno == ENOMSG) {
        return 0;
    } else if (msgret == -1) {
        perror("msgrcv");
        exit(1);
    }


    p = strtok(buffer.msg, " ");
    if (p == NULL) {
        return 0;
    }

    /* Interpretamos el mensaje recibido */
    if (!strcmp(p, "parar")) {
        *bloqueado = -1;
        ret = 0;
    } else if (!strcmp(p, "cont")) {
        *bloqueado = 0;
        ret = 0;
    } else if (!strcmp(p, "doble")) {
        ret = 2;
    } else if (!strcmp(p, "mitad")) {
        ret = 1;
    } else if (!strcmp(p, "matar")) {
        //kill(pid(compartido, indexArray, numCaballos), SIGINT);
        ret = 3;
    } else if (!strcmp(p, "mover")) {
        p = strtok(NULL, " ");
        if (p == NULL) {
            return 0;
        }
        pos = atoi(p);
        if (pos >= 0) {
            downSimetrico(semEscritura, 0);
            posicion(compartido, indexArray, numCaballos) = pos;
            upSimetrico(semEscritura, 0);
        }
        ret = 0;
    } 

    else if (!strcmp(p, "bloquear")) {
        p = strtok(NULL, " ");
        if (p == NULL) {
            return 0;
        }
        *bloqueado = atoi(p);
        ret = 0;
    } 

    else {
        /*Comando inválido, enviamos mensaje informativo*/
        strcpy(respuesta.msg, "Comando no valido.\n");
        respuesta.type = (long) TIPO_MSG_PADRE;
        if (msgsnd(colaMensajes, &respuesta, strlen(respuesta.msg) + 1, 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
        return 0;
    }


    /* Avisamos al padre de que hemos recibido el mensaje correctamente */
    strcpy(respuesta.msg, "Ok!\n");
    respuesta.type = (long) TIPO_MSG_PADRE;
    if (msgsnd(colaMensajes, &respuesta, strlen(respuesta.msg) + 1, 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
    return ret;
}

/*
 * Descripcion: se encarga de realizar una tirada concreta, ordenada
 * mediante un comando.
 *
 * Retorno:
 *	- Si tipoTirada es 1, devolvemos una tirada mitad.
 *	- Si tipoTirada es 2, devolvemos una tirada doble.
 */
int tiradaComando(int tipoTirada) {
    int tirada = 0;
    /* Tiradas especiales de comando */
    if (tipoTirada == 1) {
        /* Tirada mitad */
        tirada = (rand() % 3) + 1;
    } else {
        /* Tirada doble */
        tirada = (rand() % 6) + 1;
        tirada += (rand() % 6) + 1;
    }
    return tirada;
}
