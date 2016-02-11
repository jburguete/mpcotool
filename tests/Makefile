CC = gcc
CFLAGS = -Wall -Os -march=native -flto

all: process process2 process3

process: process.c Makefile
	$(CC) $(CFLAGS) process.c -lm -o process

process2: process2.c Makefile
	$(CC) $(CFLAGS) process2.c -lm -o process2

process3: process3.c Makefile
	$(CC) $(CFLAGS) process3.c -o process3
