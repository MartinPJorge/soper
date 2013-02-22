#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define TAM 16



typedef enum { T_NORMAL=1, T_REMONTADORA, T_GANADORA} Tirada;


int tirada(int *posiciones, pid_t *PIDHijo, int *tubPadre, int *tubHijo, int nProc){

	int min = 0, max = 0, i;
	int posMin = 0, posMax = 0;
	char buffer[TAM]

	/* Buscamos el primero y el ultimo. */
	for(i = 0; i < nProc; i++) {

		if(posiciones[i] < min) {

			min = posiciones[i];
			posMin = i;
		}

		else if(posiciones[i] > max) {

			max = posiciones[i];
			posMax = i;
		}
	}



	for (i = 0; i < nProc; i++) {

		if(posiciones[i] == max) {

			snprintf(buffer, TAM, "%d", T_GANADORA);			/* Escribir tirada en pipe, y mandar señal. */
			write (tubPadre[i][1], buffer, strlen (buffer)+1);

			kill(PIDHijo[i], SIGUSR2);
		}

		else if(posiciones[i] == min) {

			snprintf(buffer, TAM, "%d", T_REMONTADORA);			/* Escribir tirada en pipe, y mandar señal. */
			write (tubPadre[i][1], buffer, strlen (buffer)+1);

			kill(PIDHijo[i], SIGUSR2);
		}

		else {

			snprintf(buffer, TAM, "%d", T_NORMAL);			/* Escribir tirada en pipe, y mandar señal. */
			write (tubPadre[i][1], buffer, strlen (buffer)+1);

			kill(PIDHijo[i], SIGUSR2);
		}
	}
	
	return;
}

void makePipes(int **tubPadre, int **tubHijo, int nProc){

	tubPadre = (int**) malloc (nProc*sizeof(int*));
	if(!tubPadre){
		printf("Error en la reserva de memoria.\n");
		exit (1);
	}

	tubHijo = (int**) malloc (nProc*sizeof(int*));
	if(!tubHijo){
		printf("Error en la reserva de memoria.\n");
		exit (1);
	}

	for(i=0; i<nProc; ++i){

		tubPadre[i]=(int*) malloc (2*sizeof(int));
		if(!tubPadre[i]){
			printf("Error en la reserva de memoria.\n");
			exit (1);
		}

		tubHijo[i]=(int*) malloc (2*sizeof(int));
		if(!tubHijo[i]){
			printf("Error en la reserva de memoria.\n");
			exit (1);
		}
	}

	/* Creamos las tuberias para los procesos hijos. */
	for(i = 0; i < N_PROC; i++){

		if(pipe (tubPadre[i]) == -1) {

			printf("No se ha creado bien la tuberia %d.\n", i);
			exit(1);
		}

		if(pipe (tubHijo[i]) == -1) {

			printf("No se ha creado bien la tuberia %d.\n", i);
			exit(1);
		}
	}

	return;
}


int main (int argc, char *argv [], char *env []) {
	
	char buffer[TAM];
	int nbytes, *tubPadre=NULL, *tubHijo=NULL, i, j, flag = 0;
	int *posiciones;
	int finCarrera = 0;
	pid_t *PIDHijo;
	int nProc=0, longitud=0;
	sigset_t mask, temp, oldMask;

	if(argc!=3){
		printf("Numero incorrecto de argumentos.\n");
		exit (1);
	}

	/* Creamos las mascaras */
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_SETMASK, &mask, &oldMask);

	sigfillset(&temp);
	sigdelset(&temp, SIGUSR1);



	nProc=argv[1];
	longitud=argv[2];

	makePipes(tubPadre, tubHijo, nProc);

	PIDHijo = (pid_t*) malloc (nProc*sizeof(pid_t));
	if(!PIDHijo){
		printf("Error en la reserva de memoria.\n");
		exit (1);
	}

	/* Creamos los procesos hijos. */
	for (i = 0; i < nProc; i++) {

		PIDHijo[i] = fork();

		switch(PIDHijo[i]) {

			case 0:
				flag = 1;
				break;

			case -1:
				printf("Error al crear proceso.\n");
				flag = 1;
				break;

			default: 
				break;
		}

		if(flag == 1)
			break;
	}
	

	/*Hasta aqui ya tenemos creados todos los procesos hijos y sus tuberías*/
	/*----------------------------------------------------------------------------------------------------------*/
	

	posiciones = (int*) calloc (nProc*sizeof(int));
	if(!posiciones){
		printf("Error en la reserva de memoria.\n");
		exit (1);
	}
	if(flag != 1) {


		while(!finCarrera) {

			tirada();

			while(!tope && !sigAlarma) {

				sigsuspend();
				/* Lectura de pipes. */
			}

			/* Comprobar si se cae alguno. */
			/* Comprobar fin de carrera. */
		}


		/*for(i = 0; i < N_PROC; i++) {

			 Cerramos extremos que no se usan 
			close (tubPadre[i][0]);
			close (tubHijo[i][1]);

			strcpy (buffer, "Datos enviados a traves de la tuberia\n");
			write (tubPadre[i][1], buffer, strlen (buffer)+1);
			close (tubPadre[i][1]);
			while ((nbytes = read(tubHijo[i][0], buffer, TAM)) > 0)
				fprintf (stdout, "Texto leido por el padre: %s", buffer);
			close(tubHijo[i][0]);
		}*/
	}
	else {

		/* Cerramos el resto de tuberias. */
		for(j = 0; j < N_PROC; j++) {

			if(j != i) {
				close(tubHijo[j][0]);
				close(tubHijo[j][1]);
				close(tubPadre[j][0]);
				close(tubPadre[j][1]);
			}
		}

		close(tubHijo[i][0]);
		close(tubPadre[i][1]);

		while ((nbytes = read(tubPadre[i][0], buffer, TAM)) > 0)
			fprintf (stdout, "Texto leido por el hijo: %s", buffer);
		close (tubPadre[i][0]);

		snprintf(buffer, TAM, "Datos devueltos a traves de la tuberia por el proceso PID = %d\n", getpid());
		write (tubHijo[i][1] ,buffer, strlen(buffer) + 1);
		close (tubHijo[i][1]);
	}
	exit(0);
}

