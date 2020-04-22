#include "Socket.hpp"
#include <iostream>
#include <string>

#define PORT 54000

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
		std::cout << "Received: " << std::string(buffer, 0, bytesRecv) << std::endl;

		// Resend message
		server.send(buffer, bytesRecv + 1);
	}

	return 0;
}
