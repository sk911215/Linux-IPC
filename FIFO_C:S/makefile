CC = gcc
CFLAGS = -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement
all: fifo


fifo: fifo_client fifo_server

fifo_client: fifo_client.o
	gcc fifo_client.o -o fifo_client -g $(CFLAGS)

fifo_client.o: fifo_client.c
	gcc -c fifo_client.c $(CFLAGS)

fifo_server: fifo_server.o
	gcc fifo_server.o -o fifo_server -g $(CFLAGS)

fifo_server.o: fifo_server.c
	gcc -c fifo_server.c $(CFLAGS)


clean:
	rm -rf *.o *.c~ $(PROGS)
	rm -rf fifo_client fifo_server
	