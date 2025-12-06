CC = gcc
CFLAGS = -O3

mini-shell: mini-shell.c 
	$(CC) $(CFLAGS) -o mini-shell mini-shell.c

all: mini-shell

clean:
	rm -f mini-shell
