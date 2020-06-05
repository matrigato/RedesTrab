#include "Socket.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <iostream>
#include <thread>
#include <vector>
#include <poll.h>


// Socket (Abstract class)

// Definition is needed for a pure virtual destructor
Socket::~Socket(){
	closeSocket();
}

int Socket::receive(char *buffer, int bufferSize){
	
	if(hasError || !isConnected){
		return -1;
	}
	
	// Clear buffer
	memset(buffer, 0, bufferSize);

	
	int bytesRecv = recv(connectedSocket, buffer, bufferSize, 0);
	
	if(bytesRecv == -1 || bytesRecv == 0){ // Connection error OR the client disconnected

		isConnected = false;
	}

	return bytesRecv;
}

int Socket::send(char *buffer, int bufferSize){
	if(hasError || !isConnected){
		return -1;
	}

	if (bufferSize <= 1){
		return 	0;
	}

	struct pollfd fds[1];
	fds[0].fd = connectedSocket;
	fds[0].events = 0;
	fds[0].events |= POLLOUT; 
	if (poll(fds,1, 10000) <= 0)
	{
		return 0;
	}

	// calls send from global namespace
	
	return ::send(connectedSocket, buffer, bufferSize,0);
}

void Socket::closeSocket(){
	std::lock_guard<std::mutex> locker(mu);
	if(!isConnected || hasError)
		return;
	close(connectedSocket);
	isConnected = false;
	hasError = true;
}

void Socket::sendM(){
	char buffer[4096];
	bool verifyEOF = false;
	while (std::cin.getline(buffer,4096))
	{
		verifyEOF = true;
		// Resend message
		if(send(buffer, strlen(buffer) + 1) == -1)
			break;

		//quit command
		if (strcmp(buffer,"/quit")==0)
			break;
		

		bzero(buffer, 4096);
		verifyEOF = false;
	}
	
	if (!verifyEOF)
	{
		strcpy(buffer,"/quit");
		send(buffer, strlen(buffer));
	}

	closeSocket();
}

void Socket::readM(){
	char buffer[4096];

	while (true)
	{
		int bytesRecv = receive(buffer, 4096);

		if(bytesRecv == -1){
			std::cerr << "\n\rThere was a connection issue" << std::endl;
			break;
		}
		if(bytesRecv == 0 || strcmp(buffer,"/quit")==0){
			std::cout << "\n\rThe other party disconnected" << std::endl;
			break;
		}

		// Display message
		std::cout << "\n\rReceived: " << std::string(buffer, 0, bytesRecv) << std::endl;
		bzero(buffer, 4096);
	}
	closeSocket();
}

// ServerSocket

ServerSocket::ServerSocket(unsigned short int port){
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

	// accept a call
	struct sockaddr_in client;
	socklen_t clientSize = sizeof(client);

	connectedSocket = accept(sockfd, (struct sockaddr*) &client, &clientSize); // new socket number
	if(connectedSocket == -1){
		hasError = true; //Problem with client connecting
		return;
	}

	isConnected = true;

	// Close the listening socket
	close(sockfd);
}

void ServerSocket::whatsMyName(){
	char name[99];

	if(gethostname(name,99)!= 0){
		std::cout << "I don't know my name."<< std::endl;
		return;
	}
	std::cout << "My name is "<< name << std::endl;
}

void ServerSocket::readM(){
	char buffer[4096];

	while (true)
	{
		int bytesRecv = receive(buffer, 4096);

		if(bytesRecv == -1){
			std::cerr << "\n\rThere was a connection issue." << std::endl;
			std::cout << "\tPress enter to close."<<std::endl;
			break;
		}
		if(bytesRecv == 0 || strcmp(buffer,"/quit")==0){
			std::cout << "\n\rThe Client disconnected." << std::endl;
			if( strcmp(buffer,"/quit")==0)
				std::cout << "\tPress enter to close."<<std::endl;
			break;
		}

		// Display message
		std::cout << "\n\rReceived from client: " << std::string(buffer, 0, bytesRecv) << std::endl;
		bzero(buffer, 4096);
	}
	closeSocket();
		
}

// ClientSocket

ClientSocket::ClientSocket(unsigned short int port, char* serverName){

	struct sockaddr_in serv_addr;//server address
	struct hostent *server;
	
	hasError = false;
	connectedSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (connectedSocket == -1) {
		hasError = true;// Can't create socket
		std::cout << "ERRO: falha em criar um socket" << std::endl;
		return;
	}

	server = gethostbyname(serverName);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons((uint16_t) port); // htons changes byte order

	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);//copies length bytes from s1 to s2.

	if (connect(connectedSocket,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) == -1){
		//connect function is called by the client to establish a connection to the server.
		hasError = true;
		std::cout << "ERRO: falha em se conectar ao server" << std::endl;
		return;
	}
	
	isConnected=  true;
}

void ClientSocket::readM(){
	char buffer[4096];

	while(true){
		int bytesRecv = receive(buffer, 4096);

		if(bytesRecv == -1){
			std::cerr << "\n\rThere was a connection issue." << std::endl;
			std::cout << "\tPress enter to close."<<std::endl;
			break;
		}
		if(bytesRecv == 0 || strcmp(buffer,"/quit")==0 ){
			std::cout << "\n\rDisconnected from server." << std::endl;
			if(strcmp(buffer,"/quit")==0)
				std::cout << "\tPress enter to close."<<std::endl;
			break;
		}

		// Display message
		std::cout << "\n\r" << std::string(buffer, 0, bytesRecv) << std::endl;
		bzero(buffer, 4096);
	}
	closeSocket();
}
