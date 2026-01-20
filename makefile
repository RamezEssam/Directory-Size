CC      = gcc
CFLAGS  = -Wall -Wextra -O3

main: main.c
	$(CC) $(CFLAGS) main.c -o dirsize