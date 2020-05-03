#include "Socket.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include <thread>

#define PORT 54002

void server_rotine(){
	ServerSocket server(PORT);

	std::thread t1(&ServerSocket::readM,&server);
	std::thread t2(&ServerSocket::sendM,&server);

	t1.join();
	t2.join();
}

void client_rotine(){

	std::cout << "Server Name: ";
	char serverName[99];
	std::cin >> serverName;
	std::cin.ignore();
	char initMessage[] = "client is trying to say something to the server";
	ClientSocket client(PORT, serverName);

	if(client.send(initMessage, sizeof(initMessage)) == -1)
		std::cout << "erro" << std::endl;

	std:: thread t1(&ClientSocket::readM, &client);
	std:: thread t2(&ClientSocket::sendM, &client);

	t1.join();
	t2.join();
}

void menu(){
	std::cout << "Você deseja: " << std::endl;
	std::cout << "\t1 Entrar como Server." << std::endl;
	std::cout << "\t2 Entrar como Client." << std::endl;
	std::cout << "\t3 Sair." << std::endl;
}

int main(){

	int select;

	do{
		menu();
		std::cin >> select;
		//ignore \n
		std::cin.ignore();

		switch (select)
		{
			case 1:
				server_rotine();
				break;
			case 2:
				client_rotine();
				break;
			case 3:
				std::cout << "Saindo..." << std::endl;
				break;
			default:
				std::cout << "Entrada invalida" << std::endl;
				break;
		}
	}while(select > 3 || select < 1);
	return 0;
}
