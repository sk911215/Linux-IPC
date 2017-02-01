CC = gcc
CFLAGS = -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement

SOURCES1 = posixmsg_server.c
OBJECTS1 = $(SOURCES1:.c=.o)
PROGS1 = posixmsg_server

SOURCES2 = posixmsg_client.c
OBJECTS2 = $(SOURCES2:.c=.o)
PROGS2 = posixmsg_client

all: $(SOURCES1) $(PROGS1) $(SOURCES2) $(PROGS2)

$(PROGS1): $(OBJECTS1)
	$(CC) $(CFLAGS) -g -o $(PROGS1) $(OBJECTS1) -lrt

posixmsg_server.o: $(SOURCES1)
	$(CC) -c $(CFLAGS) $(SOURCES1)



$(PROGS2): $(OBJECTS2)
	$(CC) $(CFLAGS) -g -o $(PROGS2) $(OBJECTS2) -lrt

posixmsg_client.o: $(SOURCES2)
	$(CC) -c $(CFLAGS) $(SOURCES2)


clean:
	rm -f *.o *.c~ $(PROGS1) $(PROGS2)
