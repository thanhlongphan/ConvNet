CC=gcc
NETSRCS=conv.c iplimage.c conv_def.c pool.c relu.c netcreat.c netpass.c netfile.c net_errno.c
CFLAGS=-Wall -g -lpng12 -lm 
NETOBJS=$(NETSRCS:.c=.o)

all: train test #main

#main: $(NETOBJS) main.o
#	$(CC) $(CFLAGS) -o main main.o $(NETOBJS)

train: $(NETOBJS) train.o
	$(CC) $(CFLAGS) -o train train.o $(NETOBJS)

test: $(NETOBJS) test.o
	$(CC) $(CFLAGS) -o test test.o $(NETOBJS)

#main.o:main.c
#	$(CC) $(CFLAGS) -c main.c

train.o:train.c
	$(CC) $(CFLAGS) -c train.c

test.o:test.c
	$(CC) $(CFLAGS) -c test.c

conv.o:conv.c
	$(CC) $(CFLAGS) -c conv.c

relu.o:relu.c
	$(CC) $(CFLAGS) -c relu.c

pool.o:pool.c
	$(CC) $(CFLAGS) -c pool.c

conv_def.o:conv_def.c
	$(CC) $(CFLAGS) -c conv_def.c

iplimage.o:iplimage.c
	$(CC) $(CFLAGS) -c iplimage.c

netpass.o:netpass.c
	$(CC) $(CFLAGS) -c netpass.c

netcreat.o:netcreat.c
	$(CC) $(CFLAGS) -c netcreat.c

netfile.o:netfile.c
	$(CC) $(CFLAGS) -c netfile.c 

net_errno.o:net_errno.c
	$(CC) $(CFLAGS) -c net_errno.c

clean:
	rm train test *.o #main
