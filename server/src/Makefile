all: testeCli_o Socket_o teste_o main_o
	g++ Socket.o testeCli.o -g -std=c++11 -lpthread -Wall -o client
	g++ Socket.o test.o -g -std=c++11 -lpthread -Wall -o server
	g++ Socket.o main.o -g -pthread -std=c++11 -lpthread -Wall -o main

main_o: main.cpp Socket.hpp
	g++ -c main.cpp -g -pthread -std=c++11 -lpthread -Wall -Wextra

testeCli_o: testeCli.cpp Socket.hpp
	g++ -c testeCli.cpp -g -pthread -std=c++11 -lpthread -Wall -Wextra

teste_o: test.cpp Socket.hpp
	g++ -c test.cpp -g -pthread -std=c++11 -lpthread -Wall -Wextra

Socket_o: Socket.cpp Socket.hpp
	g++ -c Socket.cpp -g -pthread -std=c++11 -lpthread -Wall -Wextra

run_client: client
	./client

run_server: server
	./server

clean:
	rm -rf *.o

rmproper: clean
	rm -rf main client server
