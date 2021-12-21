CC=gcc
CFLAGS=-ansi
SOURCES=./lib/utils.c  ./lib/connection.c ./lib/server.c ./lib/client.c  


run: 	build
		./server/main.c
		./client/main.c

build: 	my
		server

server: $(SOURCES)
        $(CC) -o server ./server/main.c $(SOURCES) $(CFLAGS)

client: $(SOURCES)
        $(CC) -o client ./client/main.c $(SOURCES) $(CFLAGS)


