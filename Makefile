CC=gcc
CFLAGS= -Wall

Acembler: acembler.c
	$(CC) $(CFLAGS) -o acembler acembler.c
