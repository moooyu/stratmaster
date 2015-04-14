# STRATMASTER
# Makefile for runtime demo programs

CC = gcc

INCLUDES = -pthread
CFLAGS = -g -Wall
LDLIBS  =

# HelloWorld order only demo
.PHONY: demorder
demorder: demorder.o
	$(CC) $(CFLAGS) demorder.o -o demorder $(INCLUDES) $(LDLIBS)		

demorder.o : demorder.c
	$(CC) -g -c demorder.c


# Algorithm demo
.PHONY: demalgo
demalgo: demalgo.o
	$(CC) $(CFLAGS) demalgo.o -o demalgo $(INCLUDES) $(LDLIBS)		

demalgo.o : demalgo.c
	$(CC) -g -c demalgo.c

.PHONY: all
all:	demorder demalgo

.PHONY: clean
clean:
	rm -f *.o *~ a.out core demorder demalgo
