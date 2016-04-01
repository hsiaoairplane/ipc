
CC = gcc
CFLAGS += -I. -I/usr/include/libnl3
CFLAGS += -s
CFLAGS += -Os
LDFLAGS += -L.

all:
	gcc client.c -o client
	gcc server_1.c -o server-1
	gcc server_2.c -o server-2
	gcc server_3.c -o server-3
	gcc -I/usr/include/libnl3 -L/usr/lib/libnl-3.so prow_client.c -lnl-3 -o prow_client
	gcc -I/usr/include/libnl3 -L/usr/lib/libnl-3.so prow_server.c -lnl-3 -o prow_server
#	gcc prow_server.c -I/usr/include/libnl3 -o prow_server -L/usr/lib/libnl-3.so

clean:
	rm client server-1 server-2 server-3 prow_client prow_server

