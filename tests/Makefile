CC = gcc
CFLAGS = -Wall -Wextra -Os -march=native -flto

all: process process2 process3 process4

process: process.c Makefile
	$(CC) $(CFLAGS) process.c -lm -o process

process2: process2.c Makefile
	$(CC) $(CFLAGS) process2.c -lm -o process2

process3: process3.c Makefile
	$(CC) $(CFLAGS) process3.c -o process3

process4: process4.c Makefile
	$(CC) $(CFLAGS) process4.c -o process4
