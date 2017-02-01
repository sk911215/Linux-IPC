CC = gcc
CFLAGS = -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement
all: myword_freq

myword_freq: myword_freq.o
	gcc myword_freq.o -o myword_freq -g $(CFLAGS)

myword_freq.o: myword_freq.c
	gcc -c myword_freq.c $(CFLAGS)


clean:
	rm -rf *.o *.c~ $(PROGS)
	rm -rf myword_freq
	