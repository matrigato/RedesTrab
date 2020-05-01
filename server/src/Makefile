run_client:
	./client

run_server:
	./server

all: testeCli_o Socket_o teste_o main_o
	g++ Socket.o testeCli.o -g -std=c++11 -lpthread -Wall -o client
	g++ Socket.o test.o -g -std=c++11 -lpthread -Wall -o server
	g++ Socket.o main.o -g -pthread -std=c++11 -lpthread -Wall -o main

main_o: main.cpp
	g++ -c main.cpp -g -pthread -std=c++11 -lpthread

testeCli_o: testeCli.cpp
	g++ -c testeCli.cpp -g -pthread -std=c++11 -lpthread

teste_o: test.cpp
	g++ -c test.cpp -g -pthread -std=c++11 -lpthread

Socket_o: Socket.cpp
	g++ -c Socket.cpp -g -pthread -std=c++11 -lpthread