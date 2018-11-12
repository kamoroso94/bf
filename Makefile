# BF Interpreter Makefile
CC = gcc

default: all
all: bf

bf: build/main.o build/bf.o build/stack.o
	$(CC) -o build/bf build/main.o build/bf.o build/stack.o

build/main.o: src/main.c src/bf.h
	$(CC) -o build/main.o -c src/main.c

build/bf.o: src/bf.c src/bf.h src/stack.h
	$(CC) -o build/bf.o -c src/bf.c

build/stack.o: src/stack.c src/stack.h
	$(CC) -o build/stack.o -c src/stack.c

clean:
	$(RM) build/*.o
