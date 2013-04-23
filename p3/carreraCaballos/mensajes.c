#include "mensajes.h"

int* compartido;
char msg_buffer[TAM_MSG_BUFF];
int colaMensajes;

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


    if (strlen(instruccion) >= 3) {

        if (!strcmp(instruccion, "Mostrar\n")) {
            return 1;
        }

        p = strtok(instruccion, " ");
        if (p == NULL) {
            printf("Comando no válido.\n");
            printf("Aq1\n");
            return 0;
        }
        comando = strdup(p);


        p = strtok(NULL, " ");
        if (p == NULL) {
            
            printf("Comando no válido.\n");
            printf("Aq2\n");
            return 0;
        }
        cab = strdup(p);

        printf("%s\n", cab);
        fflush(NULL);

        if ((p = strtok(NULL, " "))!=NULL)
            n = strdup(p);

        buffer.type = atoi(cab) + 1;

        
        /*  ------ IVAN ------
        strcpy(buffer.msg, comando);
        if (n != NULL) {
            sprintf(buffer.msg, "%s %d", comando, atoi(n));
        }*/

        if(n == NULL)
            sprintf(buffer.msg, "%s %d", comando, atoi(cab));
        else
            sprintf(buffer.msg, "%s %d %d", comando, atoi(cab), atoi(n));


        if (msgsnd(colaMensajes, &buffer, strlen(buffer.msg) + 1, 0) == -1) {
            perror("msgsnd");
            exit(1);
        }

        


    } else {
        printf("Comando no válido.\n");
        printf("Aq3\n");
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
    //gets(tmp);
    //scanf("%s", tmp);
    //scanf ("%[^\n]%*c", tmp);
    //scanf("%[^\n]s",tmp);
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
    printf("mensaje recibido\n");
    if (ret == -1 && errno == ENOMSG) {
        return 0;
    } else if (ret == -1) {
        perror("msgrcv");
        exit(1);
    }

    printf("%s\n", buffer.msg);
    //printf("Salgo del printf()\n");

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
    //printf("El kill devuelve: %d\n", retKill);

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
 */
int mirarMensajesCaballo(int indexArray, int *bloqueado, int semEscritura, int numCaballos) {
    struct msgbuf buffer;
    struct msgbuf respuesta;
    char *p;
    int pos, ret;

    ssize_t msgret = msgrcv(colaMensajes, &buffer, 20, indexArray + 1, IPC_NOWAIT | MSG_NOERROR);
    //printf("El caballo recibe mensaje de tipo: %d\n", indexArray + 1);

    if (msgret == -1 && errno == ENOMSG) {
        return 0;
    } else if (msgret == -1) {
        perror("msgrcv");
        printf("Tam. buffer: %d\n", (int)strlen(buffer.msg));
        exit(1);
    }

    //printf("El caballo recibe mensaje: %s\n", buffer.msg);

    p = strtok(buffer.msg, " ");
    if (p == NULL) {
        return 0;
    }

    /* Interpretamos el mensaje recibido */
    if (!strcmp(p, "parar")) {
        *bloqueado = -1;printf("Seccion0\n");
        ret = 0;
    } else if (!strcmp(p, "cont")) {
        *bloqueado = 0;printf("Seccion1\n");
        ret = 0;
    } else if (!strcmp(p, "doble")) {
        ret = 2;printf("Seccion3\n");
    } else if (!strcmp(p, "mitad")) {
        ret = 1;printf("Seccion4\n");
    } else if (!strcmp(p, "matar")) {
        kill(pid(compartido, indexArray, numCaballos), SIGINT);printf("Seccion5\n");
    } else if (!strcmp(p, "mover")) {
        p = strtok(buffer.msg, " ");printf("Seccion6\n");
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
    } else if (!strcmp(p, "bloquear")) {
        p = strtok(buffer.msg, " ");printf("Seccion7\n");
        if (p == NULL) {
            return 0;
        }
        *bloqueado = atoi(p);
        ret = 0;
    } else {
printf("Seccion8\n");
        /*Comando inválido, enviamos mensaje informativo*/
        strcpy(respuesta.msg, "Comando no valido.\n");
        respuesta.type = (long) TIPO_MSG_PADRE;
        if (msgsnd(colaMensajes, &respuesta, strlen(respuesta.msg) + 1, 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
        return 0;
    }

    printf("Seccion9\n");

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