#include "Socket.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h> 

ServerSocket::ServerSocket(unsigned short int port){
	hasError = false;

	isConnected = false;

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

	clientSocket = accept(sockfd, (struct sockaddr*) &client, &clientSize); // new socket number
	if(clientSocket == -1){
		hasError = true; //Problem with client connecting
		return;
	}

	isConnected = true;

	// Close the listening socket
	close(sockfd);
}

ServerSocket::~ServerSocket(){
	if(!hasError){
		close(clientSocket);
	}
}

int ServerSocket::receive(char *buffer, int bufferSize){
	if(hasError || !isConnected){
		return -1;
	}

	// Clear buffer
	memset(buffer, 0, bufferSize);

	// Wait for a message
	int bytesRecv = recv(clientSocket, buffer, bufferSize, 0);

	if(bytesRecv == -1 || bytesRecv == 0){ // Connection error OR the client disconnected
		isConnected = false;
	}

	return bytesRecv;
}

int ServerSocket::send(char *buffer, int bufferSize){
	if(hasError || !isConnected){
		return -1;
	}

	if (bufferSize == 1)
		return 	0;
	

	// calls send from global namespace
	return ::send(clientSocket, buffer, bufferSize, 0);
}

ClientSocket::ClientSocket(unsigned short int port, char* serverName){
	
	struct sockaddr_in serv_addr;//server address	
    struct hostent *server;
	
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1) {
        hasError = true;// Can't create socket
		return;
	}

	server = gethostbyname(serverName);	
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons((uint16_t) port); // htons changes byte order

	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);//copies length bytes from s1 to s2. 

	if (connect(serverSocket,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) == -1){
		//connect function is called by the client to establish a connection to the server.
		hasError = true;
		return;
	}

	isConnected=  true;
}

ClientSocket::~ClientSocket(){
	if(!hasError){
		close(serverSocket);
	}
}

int ClientSocket::receive(char *buffer, int bufferSize){
	if(hasError || !isConnected){
		return -1;
	}

	// Clear buffer
	memset(buffer, 0, bufferSize);

	// Wait for a message
	int bytesRecv = recv(serverSocket, buffer, bufferSize, 0);

	if(bytesRecv == -1 || bytesRecv == 0){ // Connection error OR the client disconnected
		isConnected = false;
	}

	return bytesRecv;
}

int ClientSocket::send(char *buffer, int bufferSize){
	if(hasError || !isConnected){
		return -1;
	}

	if (bufferSize == 1)
		return 	0;
	

	// calls send from global namespace
	return ::send(serverSocket, buffer, bufferSize, 0);
}