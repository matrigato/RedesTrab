#include "Socket.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <mutex>

#define PORT 54000

int main(){
	ServerSocket server(PORT);
	server.whatsMyName();

	std::thread t1(&ServerSocket::readM,&server);
	std::thread t2(&ServerSocket::sendM,&server);

	t1.join();
	t2.join();

	return 0;
}
