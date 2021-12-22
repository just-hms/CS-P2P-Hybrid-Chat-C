CC=gcc
SOURCES=./lib/utils.c ./lib/connection.c  
CFLAGS=-ansi 

# -Wall

build: 			build_server build_client

run:			build
				./server/server

build_server: 	$(SOURCES)
				$(CC) -o ./server/server ./server/main.c $(SOURCES) $(CFLAGS)

build_client: 	$(SOURCES)
				$(CC) -o ./client/client ./client/main.c $(SOURCES) $(CFLAGS)


