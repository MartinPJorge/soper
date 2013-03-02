
CC = gcc
FLAGS = -c -Wall
DEPS_MAIN = tipos.h caballo.h caballo.c padre.h padre.c procesos.h procesos.c tuberia.h tuberia.c

.PHONY:	clean

all:	ejFinal

clean:
	@rm -f *.o core ejFinal




ejFinal:	main.o caballo.o padre.o procesos.o tuberia.o
	@$(CC) $^ -o ejFinal



###################
# Generar objetos
###################
main.o:	$(DEPS_MAIN)
	@$(CC) $(FLAGS) main.c

caballo.o:	tipos.h caballo.h caballo.c
	@$(CC) $(FLAGS) caballo.c

padre.o:	tipos.h padre.h padre.c
	@$(CC) $(FLAGS) padre.c

procesos.o:	tipos.h procesos.h procesos.c
	@$(CC) $(FLAGS) procesos.c

tuberia.o:	tipos.h tuberia.h tuberia.c
	@$(CC) $(FLAGS) tuberia.c