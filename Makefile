# BF Interpreter Makefile
CC=gcc
PROG=bf
OBJS=$(patsubst %, build/%, main.o bf.o)
LIBOBJS=$(patsubst %, lib/build/%, stack.o)

default: all
all: $(PROG)

$(PROG): build $(OBJS) $(LIBOBJS)
	$(CC) -o $@ $(filter %.o, $^)

build:
	mkdir -p $@ lib/$@

build/main.o: src/main.c src/bf.h
	$(CC) -o $@ -c $<

build/bf.o: src/bf.c src/bf.h lib/src/stack.h
	$(CC) -o $@ -c $<

lib/build/stack.o: lib/src/stack.c lib/src/stack.h
	$(CC) -o $@ -c $<

clean:
	$(RM) $(PROG) $(OBJS) $(LIBOBJS)

.PHONY: default all build clean
