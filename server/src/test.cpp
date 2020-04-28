#include "Socket.hpp"
#include <iostream>
#include <string>
#include <string.h>


#define PORT 54001

int main(){
	ServerSocket server(PORT);

	char buffer[4096];

	while(true){
		//receive message
		int bytesRecv = server.receive(buffer, 4096);

		if(bytesRecv == -1){
			std::cerr << "There was a connection issue" << std::endl;
			break;
		}
		if(bytesRecv == 0){
			std::cout << "The client disconnected" << std::endl;
			break;
		}

		// Display message
		std::cout << "Received from client: " << std::string(buffer, 0, bytesRecv) << std::endl;
		bzero(buffer, 4096);
		std::cin.getline(buffer,4096);
		//quit command
		if (strcmp(buffer,"/quit")==0)
        {
            server.closeSocket();
            return 0;
        }
		// Resend message
		server.send(buffer, strlen(buffer) + 1);
		bzero(buffer, 4096);
	}

	return 0;
}
