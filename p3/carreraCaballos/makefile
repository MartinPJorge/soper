DEPS = main.c ini.c ini.h sistema.c sistema.h caballo.c caballo.h principal.c principal.h mensajes.c mensajes.h
CC = gcc -pthread
FLAGS =  -c -Wall

.PHONY:	clean

all:	ejFinal

clean:
	@rm -f *.o core ejFinal


ejFinal:	main.o ini.o caballo.o principal.o sistema.o mensajes.o
	$(CC) $^ -o ejFinal



###################
# Generar objetos
###################
main.o:	$(DEPS)
	$(CC) $(FLAGS) main.c

ini.o: $(DEPS)
	$(CC) $(FLAGS) ini.c

caballo.o: $(DEPS)
	$(CC) $(FLAGS) caballo.c

principal.o: $(DEPS)
	$(CC) $(FLAGS) principal.c

sistema.o: $(DEPS)
	$(CC) $(FLAGS) sistema.c

mensajes.o: $(DEPS)
	$(CC) $(FLAGS) mensajes.c
