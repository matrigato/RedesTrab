#include "Socket.hpp"
#include "Chat.hpp"
#include "mainServer.hpp"
#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <signal.h>
#define PORT 54005


void sigintHandler(int sig_num){
	signal(SIGINT,sigintHandler);
	printf("para fechar o programa utilize o comando /quit\n");
}

void chat_server_routine(){
	ChatRoom chat(PORT);
	std::vector<std::thread> threadVector;
	
	while(chat.userNum >= 0){
		if(chat.userNum <= 20){
			chat.acceptC();
			
			if(chat.hasSocket())
				threadVector.push_back(std::thread(&ChatRoom::addNewUser, &chat));
		}
	}

	for (size_t i = 0; i < threadVector.size(); i++)
	{
		threadVector[i].join();
	}
}

void main_server_routine(){

	MainServer server(PORT);
	std::vector<std::thread> threadVector;
	
	while (server.isOpen)// parametro, n tem a ver com o ternario
	{
		server.acceptC();
		
		if(server.tempUser != -1)
			threadVector.push_back(std::thread(&MainServer::startUser, &server));
		
		//server.verifyServer();//verifica se ainda temos usuarios no servidor
	}
	
	for (size_t i = 0; i < threadVector.size(); i++)
	{
		threadVector[i].join();
	}

}

void server_rotine(){
	ServerSocket server(PORT);
	std::thread t1(&ServerSocket::readM,&server);
	std::thread t2(&ServerSocket::sendM,&server);
	t1.join();
	t2.join();
	
}

void client_rotine(){
	//recives user name
	bool allowed;
	char userName[50];
	do{
		allowed = true;
		std::cout << "User Name: ";
		bzero(userName,50);
		std::cin >> userName;
		std::cin.ignore();
		userName[49] = '\0';

		for(int i = 0; userName[i] != '\0'; i++){
			if(!((userName[i] >= 'a' && userName[i] <= 'z') || (userName[i] >= 'A' && userName[i] <= 'Z'))){
				std::cout << "Apenas caracteres 'a' ... 'z' | 'A' ... 'Z' são permitidos. Digite novamente." << std::endl;
				allowed = false;
				break;
			}
		}
	}while(!allowed);

	//recives server name
	std::cout << "Server Name: ";
	char serverName[99];
	bzero(serverName,99);
	std::cin >> serverName;
	std::cin.ignore();
	
	char initMessage[99] = "/start \0";
	strcat(initMessage,userName);
	ClientSocket client(PORT, serverName);
	
	std::cout << "tentando se conectar com o servidor " << serverName << "...";
	if(client.send(initMessage, sizeof(initMessage)) == -1){
		std::cout << "ERRO" << std::endl;
		return;
	}
	std::cout << "SUCESSO" << std::endl;

	std:: thread t1(&ClientSocket::readM, &client);
	std:: thread t2(&ClientSocket::sendM, &client);

	t1.join();
	t2.join();
}

void menu(){
	std::cout << "Você deseja: " << std::endl;
	std::cout << "\t/host - Entrar como um usuario host." << std::endl;
	std::cout << "\t/setroom - Iniciar um servidor de uma ChatRoom." << std::endl;
	std::cout << "\t/connect - Entrar como Client em uma chatRoom ou se comunicar com um usuario host." << std::endl;
	std::cout << "\t/openserver - Criar um servidor com multiplas ChatRooms" << std::endl;
	std::cout << "\t/quit - Sair." << std::endl;
}

int main(){

	std::string select;
	bool valid;

	signal(SIGINT,sigintHandler);// ignore ctrl + C 

	do{
		menu();
		std::cin >> select;
		//ignore \n
		std::cin.ignore();

		valid = true;

		if(select.compare("/connect") == 0){
			client_rotine();
		}else if(select.compare("/host") == 0){
			server_rotine();
		}else if(select.compare("/setroom") == 0){
			chat_server_routine();
		}
		else if (select.compare("/openserver") == 0)
		{
			main_server_routine();
		}else if(select.compare("/quit") == 0){
			std::cout << "Saindo..." << std::endl;
		}else{
			std::cout << "Entrada invalida" << std::endl;
			valid = false;
		}
	}while(!valid);
	return 0;
}
