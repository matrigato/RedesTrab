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
	}
}

void ChatRoom :: removeUser(int userSocket){
	
	for(int i = 0; i < userVector.size(); i++){ 
		
		if(userVector[i].verifySocket(userSocket)){
			userVector.erase(i);
		}
	}
}
UserData :: UserData(int newSocket){}