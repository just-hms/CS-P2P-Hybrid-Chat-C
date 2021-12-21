CC=gcc
CFLAGS=-ansi -Wall
SOURCES=./lib/utils.c ./lib/connection.c  

build: 			build_server build_client

build_server: 	$(SOURCES)
				$(CC) -o ./server/server ./server/main.c $(SOURCES) $(CFLAGS)

build_client: 	$(SOURCES)
				$(CC) -o ./client/client ./client/main.c $(SOURCES) $(CFLAGS)


