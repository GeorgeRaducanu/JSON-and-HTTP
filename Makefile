CC=g++
CFLAGS=-I.

client: client.cpp requests.cpp helpers.cpp buffer.cpp
	$(CC) -o client client.cpp requests.cpp helpers.cpp buffer.cpp -g -Wall -w

run: client
	./client

clean:
	rm -f *.o client
