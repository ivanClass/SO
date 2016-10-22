CFLAGS = -Wall -g

all : mytar.o mytar_routines.o mytar.h
	gcc $(CFLAGS) -o mytar mytar.o mytar_routines.o

clean:
	-rm *.o mytar
