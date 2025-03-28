SEND = send
RECV = recv

.PHONY: all clean

all: $(RECV) $(SEND)
#all: $(SEND)

CC = gcc
CFLAGS = -Wall -O2

$(SEND): send.c
	$(CC) $(CFLAGS) -o $(SEND) send.c

$(RECV): recv.c
	$(CC) $(CFLAGS) -o $(RECV) recv.c

clean:
	-rm -f $(RECV) $(SEND)
	-rm -f *~
	-rm *.o
