# BF Interpreter Makefile
CC = gcc

default: all
all: bf

bf: build/bf.o build/stack.o
	$(CC) -o bf build/bf.o build/stack.o

build/bf.o: src/bf.c src/bf.h src/stack.h
	$(CC) -o build/bf.o -c src/bf.c

build/stack.o: src/stack.c src/stack.h
	$(CC) -o build/stack.o -c src/stack.c

clean:
	$(RM) build/*.o
