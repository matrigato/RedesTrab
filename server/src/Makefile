run_client:
	./client

run_server:
	./server

all: testeCli_o Socket_o teste_o main_o
	g++ Socket.o testeCli.o -g -Wall -o client
	g++ Socket.o test.o -g -Wall -o server
	g++ Socket.o main.o -g -Wall -o main

main_o: main.cpp
	g++ -c main.cpp -g

testeCli_o: testeCli.cpp
	g++ -c testeCli.cpp -g

teste_o: test.cpp
	g++ -c test.cpp -g

Socket_o: Socket.cpp
	g++ -c Socket.cpp -g