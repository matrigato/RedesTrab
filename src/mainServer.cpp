#include "Chat.hpp"
#include "Socket.hpp"
#include "mainServer.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <iostream>
#include <thread>
#include <vector> 
#include <poll.h>

MainServer :: MainServer (unsigned short int port){
    
    isOpen = false;
    
    //prepara os usuarios em espera
    waitingUsers = (UserData*)malloc(sizeof(UserData) * 20);
	for(int i = 0; i < 20; i++){
		waitingUsers[i] = UserData();
		waitingUsers[i].isConnected = false;
	}

    //prepara as salas
    rooms = (ChatRoom*)malloc(sizeof(UserData) * 20);
    for (size_t i = 0; i < 20; i++)
    {
        rooms[i] = ChatRoom();
        rooms[i].userNum = -1;
    }


    // create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		std:: cout << "\n\rSERVER_LOG: SOCKET_ERROR" << std:: endl;
		return;
	}

	int opt = 1;

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    }

    // Bind socket to IP / port
	struct sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons((uint16_t) port); // htons changes byte order
	//inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
	hint.sin_addr.s_addr = INADDR_ANY; // or inet_addr(ipv4 string)

	if(bind(sockfd, (struct sockaddr*) &hint, sizeof(hint)) == -1){
		// Can't bind
		std:: cout << "\n\rSERVER_LOG: BIND_ERROR" << std:: endl;
		return;
	}

	// Mark the socket for listening in
	if(listen(sockfd, 25) == -1){
		 // Can't listen
		std:: cout << "\n\rSERVER_LOG: LISTEN_ERROR" << std:: endl;
		return;
	}

    std:: cout << "\n\rSERVER_LOG: Servidor aberto." << std:: endl;
    whatsMyName();

    isOpen = true;
}

void MainServer :: whatsMyName(){
    char name[99];
	if(gethostname(name,99)!= 0){
		std::cout << "I don't know my name."<< std::endl;
		return;
	}
	std::cout << "My name is "<< name << std::endl;
}

void MainServer :: acceptC(){

    if(!isOpen){
        std:: cout << "\n\rSERVER_LOG: fechando server." << std:: endl;
		return ;
    }

	struct pollfd fds[1];
	fds[0].fd = sockfd;
	fds[0].events = 0;
	fds[0].events |= POLLIN;
	
	if(poll(fds,1,3000) == 0) // no one is trying to connect
		return;

    struct sockaddr_in client;
	socklen_t clientSize = sizeof(client);
	int  newConnectionSocket ;

	std:: cout << "\n\rSERVER_LOG: ERRNO_B_ACCEPT: " << errno << std:: endl;
	newConnectionSocket = accept(sockfd, (struct sockaddr*) &client, &clientSize); // new socket number
	std:: cout << "\n\rSERVER_LOG: ERRNO_A_ACCEPT: " << errno << std:: endl;
	

	//gera o ip do client na forma de string
	char ipStr[50];
	inet_ntop(AF_INET, (struct sockaddr*)&client.sin_addr, ipStr, 50);
    if(newConnectionSocket != -1){
        UserData newUser(newConnectionSocket);
        strcpy(newUser.userIp,ipStr);
        //chamar listen
    }
}
