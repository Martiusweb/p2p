CC=gcc
CFLAGS=-Wall -pedantic -ansi -g
LIBS=
OBJS=p2p.o sfhash.o
OUT=p2p

$(OUT): main.c $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

all: $(OUT)

clean:
	rm -f *.o $(OUT)

.c.o:
	$(CC) $(CFLAGS) -c $*.c
