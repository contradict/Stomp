CFLAGS=-g3

all: test_publisher

rfd900x.o: rfd900x.c
	gcc -c $(CFLAGS) rfd900x.c

telem.o: telem.c messages.h
	gcc -c $(CFLAGS) telem.c

test_publisher.o: test_publisher.c messages.h
	gcc -c $(CFLAGS) test_publisher.c

test_publisher: test_publisher.o telem.o rfd900x.o
	gcc $(LDFLAGS) -o test_publisher test_publisher.o telem.o rfd900x.o

clean:
	rm *.o 
