#include "Chat.hpp"
#include "Socket.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <thread>
#include <vector> 


ChatRoom :: ChatRoom(unsigned short int port){
	hasError = false;

	isConnected = false;

	whatsMyName();

	// create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		hasError = true; // Can't create socket
		return;
	}

	// Bind socket to IP / port
	struct sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons((uint16_t) port); // htons changes byte order
	//inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
	hint.sin_addr.s_addr = INADDR_ANY; // or inet_addr(ipv4 string)

	if(bind(sockfd, (struct sockaddr*) &hint, sizeof(hint)) == -1){
		hasError = true; // Can't bind
		return;
	}

	// Mark the socket for listening in
	if(listen(sockfd, SOMAXCONN) == -1){
		hasError = true; // Can't listen
		return;
	}

	std::cout << "A sala de chat foi aberta; Esperando por usuarios..." << std::endl;

	while (true)
	{
		if(userNum == -1)//closing room; No more users in the room.
			return;

		struct sockaddr_in client;
		socklen_t clientSize = sizeof(client);
		int newConnectionSocket;
		newConnectionSocket = accept(sockfd, (struct sockaddr*) &client, &clientSize); // new socket number
		
		if(newConnectionSocket == -1){
			hasError = true; //Problem with client connecting
			return;
		}

		if(userNum >=0  && userNum < 20){//max number of users in the same room
			addNewUser(newConnectionSocket);
		}else{
			close(newConnectionSocket);
		}
	} 
}

void ChatRoom:: whatsMyName(){
	char name[99];

	if(gethostname(name,99)!= 0){
		std::cout << "I don't know my name."<< std::endl;
		return;
	}
	std::cout << "My name is "<< name << std::endl;
}

void ChatRoom :: addNewUser(int newSocket){
	if (userNum >= 0 && userNum < 20)
	{
		UserData newUser(newSocket);//create the new user and starts to listem to it
		userVector.push_back(newUser);
		userNum++;//update the user num
	}else{
		close(newSocket);
	}
}



void ChatRoom :: removeUser(int userSocket){
	//colocar mutex aqui
	for(int i = 0; i < userVector.size(); i++){ 
		
		if(userVector[i].verifySocket(userSocket)){
			//prepare user left message
			char buffer[4096];
			strcpy(buffer,userVector[i].userName); // nomes de no maximo 14
			strcat(buffer," saiu da sala.\n");
			
			userVector.erase(i);
			
			sendMToAll(buffer);
		}
	}
}

// send a message to all the users, server messages only
void ChatRoom :: sendMToAll(char * message){
	
	//colocar mutex aqui para travar a função de ser acessada multiplas vezes, First In First Out 
	int bytesSend = 0;
	for (size_t i = 0; i < userVector.size(); i++)
	{
		userVector[i].sendNewM(message, 4096);//send the message to the user
	}

	std::cout << "LOG: Mensagem do sistema enviada."<< std::endl;
}

//send a message to all the users, but not the user that make the request
void ChatRoom :: sendUserM(int userSocket, char * message){
	//colocar mutex aqui para travar a função de ser acessada multiplas vezes, First In First Out, o mesmo que a função sendMToAll

	for (size_t i = 0; i < userVector.size(); i++)
	{
		if(!userVector[i].verifySocket(userSocket))
			userVector[i].sendNewM(message, 4096);
	}
	std::cout << "LOG: Mensagem de um usuario enviada."<< std::endl;
}

void ChatRoom :: listenUser(UserData user, int socket){
	char buffer[4096];//4096
	std::thread t1;

	while(true){
		int bytesRecv = user.receive(buffer, 4096);

		if(bytesRecv == -1){
			std::cerr << "\n\rThere was a connection issue with an user" << std::endl;
			//t1(); //menssagem para todos
			break;
		}
		else if(bytesRecv == 0 || strcmp(buffer,"/quit")==0){
			std::cout << "\n\rThe one user disconnected" << std::endl;
			//t1(); //menssagem para todos
			break;
		}
		else{
			// Display message
			std::cout << "\n\rReceived: " << std::string(buffer, 0, bytesRecv) << std::endl;
			bzero(buffer, 4096);
			//t1(); //menssagem para todos menos eu
		}
		t1.join();
	}

	t1.join();
}

UserData :: UserData(int newSocket){}

//send a messagem and if necessary resend it;
void UserData :: sendNewM(char * buffer, int bSize){
	for (size_t i = 0; i < 5; i++)
	{
		if(send(buffer, bSize) != -1)
			return;
	}

	std::cout << "LOG: Problemas em se conectar com " << userName << std::endl;
}