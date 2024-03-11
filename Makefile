CC=gcc
CFLAGS=-Wall -pedantic -g
OBJECTS=ctar.o utar.o

all: ctar utar
ctar: ctar.c
	$(CC) $(CFLAGS) ctar.c -o ctar -lm
utar: utar.c
	$(CC) $(CFLAGS) utar.c -o utar

.PHONY: clean
clean:
	rm ctar utar
