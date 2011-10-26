CC=gcc
CFLAGS=-Wall -pedantic -ansi
OBJS=p2p.o sfhash.o
OUT=p2p

$(OUT): main.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

all: p2p

clean:
	rm -f *.o $(OUT)

.c.o:
	$(CC) $(CFLAGS) -c $*.c
