#include "Socket.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <mutex>

#define PORT 54000

int main(){
	char serverName[] = "vini-pc";
	char initMessage[] = "client is trying to say something to the server\0";
	ClientSocket client(PORT, serverName);

	if(client.send(initMessage, sizeof(initMessage)) == -1)
		std::cout << "erro" << std::endl;

	std:: thread t1(&ClientSocket::readM, &client);
	std:: thread t2(&ClientSocket::sendM, &client);

	t1.join();
	t2.join();

	return 0;
}
