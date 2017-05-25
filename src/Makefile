all: clean libsalloc.a app

libsalloc.a: 	salloc.c
	gcc -Wall -c salloc.c -lpthread
	ar -cvq libsalloc.a salloc.o
	ranlib libsalloc.a

app:
	gcc -Wall -o app ../supplied/app.c  -L. -lsalloc -lpthread


clean: 
	rm -fr *.o *.a *~ a.out app