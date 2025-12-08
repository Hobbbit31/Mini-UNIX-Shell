CC = gcc
CFLAGS = -O3

mini-shell: mini-shell.c include/tokenizer.c include/execute.c include/parser.c include/parser.h include/execute.h  include/history.h
	$(CC) $(CFLAGS) -o mini-shell mini-shell.c include/tokenizer.c include/execute.c include/parser.c include/history.c

all: mini-shell

clean:
	rm -f mini-shell
