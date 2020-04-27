#include "Socket.hpp"
#include <iostream>
#include <string>
#include <string.h>

#define PORT 54000

void server_rotine(){
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
		buffer[0] = '\0';
		std::cin.getline(buffer,4096);
		// Resend message
		server.send(buffer, strlen(buffer) + 1);
		buffer[0] = '\0';
	}
}

void client_rotine(){
	char serverName[] = "vini-pc";//colocar aqui o nome do computar para identificar ele como servidor
    char initMessage[] = "client is tring to say something to the server\0";
	ClientSocket client(PORT, serverName);
	char buffer[4096];

    if(client.send(initMessage, sizeof(initMessage)) == -1)
        std::cout << "erro na conexão" << std::endl;

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
}

void menu(){
	std::cout << "Você deseja: " << std::endl;
	std::cout << "\t1 Entrar como Server." << std::endl;
	std::cout << "\t2 Entrar como Client." << std::endl;
	std::cout << "\t3 Sair." << std::endl;	
}

int main(){

	menu();
	int select;
	std:: cin >> select;
	switch (select)
	{
		case 1:
			server_rotine();
			break;
		case 2:
			client_rotine();
			break;
		case 3:
			std::cout << "saindo...";
			break;	
		return 0;
	}
}