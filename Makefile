# specify the wireless interfaces that are capable of monitor mode
W1   := wlx503eaa3d660d
W2   := wlx5ca6e6a31052
CHAN := 1

# great set of commands: https://www.itdojo.com/courses-linux/linuxwifi/

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

start:
	sudo airmon-ng check kill
	sudo airmon-ng start ${W1}
	sudo airmon-ng start ${W2}
	iw dev

stop:
	airmon-ng stop ${W1}
	sudo airmon-ng stop ${W2}

perms:
	sudo setcap cap_net_raw=ep ${PROG}
	getcap ${PROG}

s:
	sudo ./${SEND} ${W1}

r:
	sudo ./${RECV} ${W2}

git:
	git add *
	git commit -am"updated"
	git push origin main

info:
	iw dev

chan:
	sudo iw dev ${W1} set channel ${CHAN}
	sudo iw dev ${W2} set channel ${CHAN}
	sudo iw dev
