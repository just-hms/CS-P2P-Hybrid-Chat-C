CC=gcc
SOURCES=./src/lib/utils.c  ./src/lib/endpoint.c ./src/lib/io.c
CFLAGS=-ansi -g -Wall -D_GNU_SOURCE

build_all: 		build_server build_client

build_server: 	$(SOURCES)
				$(CC) -o ./serv ./src/server.c $(SOURCES) $(CFLAGS)

build_client: 	$(SOURCES)
				$(CC) -o ./dev ./src/client.c $(SOURCES) $(CFLAGS)
