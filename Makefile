CC = gcc
INCLUDES = -I.
DEPS = portscan.h
OBJ = portscan.c

all: portscan

portscan.o: portscan.c
	$(CC) -Wall -Wextra -c $(INCLUDES) portscan.c

portscan: portscan.o
	$(CC) -o portscan portscan.o

clean:
	rm -f *.o *~ portscan