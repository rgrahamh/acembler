CC=gcc
CFLAGS= -Wall -g

Acembler: acembler.c
	$(CC) $(CFLAGS) -o acembler acembler.c
