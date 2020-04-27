#include "Socket.hpp"
#include <iostream>
#include <string>
#include <string.h>

#define PORT 54000

int main(){
    char serverName[] = "vini-pc";
    char initMessage[] = "client is tring to say something to the server\0";
	ClientSocket client(PORT, serverName);
	char buffer[4096];

    if(client.send(initMessage, sizeof(initMessage)) == -1)
        std::cout << "erro" << std::endl;

	while(true){
		//receive message
		int bytesRecv = client.receive(buffer, 4096);

		if(bytesRecv == -1){
			std::cerr << "There was a connection issue" << std::endl;
			break;
		}
		if(bytesRecv == 0){
			std::cout << "The server disconnected" << std::endl;
			break;
		}

		// Display message
		std::cout << "Received form Server: " << std::string(buffer, 0, bytesRecv) << std::endl;
        buffer[0] = '\0';

		std::cin.getline(buffer,4096);
		// Resend message
		client.send(buffer, strlen(buffer)+1);
        buffer[0] = '\0';
	}

	return 0;
}
