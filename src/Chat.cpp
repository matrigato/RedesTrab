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

	whatsMyName();

	// create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		std:: cout << "\n\rSERVER_LOG: SOCKET_ERROR" << std:: endl;
		return;
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
	if(listen(sockfd, SOMAXCONN) == -1){
		 // Can't listen
		std:: cout << "\n\rSERVER_LOG: LISTEN_ERROR" << std:: endl;
		return;
	}

	std::cout << "\n\rSERVER_LOG: A sala de chat foi aberta; Esperando por usuarios..." << std::endl;

}

int ChatRoom::accept(UserData *user, int *connection, int *flag){
	*flag = 0;

	if(userNum == -1){//closing room; No more users in the room.
		std:: cout << "\n\rSERVER_LOG: todos os usuarios sairam; Concluindo Processo" << std:: endl;
		return 1;
	}

	struct sockaddr_in client;
	socklen_t clientSize = sizeof(client);
	int newConnectionSocket;
	newConnectionSocket = ::accept(sockfd, (struct sockaddr*) &client, &clientSize); // new socket number
	
	if(newConnectionSocket == -1){
		//Problem with client connecting
		return 1;
	}

	if(userNum >=0  && userNum < 20){//max number of users in the same room
		addNewUser(newConnectionSocket);
		//create a new thread and add it to thread vector
		*user = userVector[userNum-1];
		*connection = newConnectionSocket;
	}else{
		close(newConnectionSocket);
		*flag = 1;
	}

	return 0;
}

void ChatRoom::destroy(){
	for (size_t i = 0; i < threadVector.size(); i++)
	{
		if(threadVector[i].joinable())
			threadVector[i].join();
	}
}

/*void ChatRoom::newThread(std::thread t){
	threadVector.push_back(t);
}*/

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
		UserData newUser(newSocket);//create the new user and starts to listen to it
		
		//seting name
		strcpy(newUser.userName,"user");
		char* num = "$$";   
		strcat(newUser.userName,num);
		sprintf(num, "%d", userNum);
		
		//put the user in the vector
		userVector.push_back(newUser);
		userNum++;//update the user num
	}else{
		close(newSocket);
	}
}



void ChatRoom :: removeUser(int userSocket){
	
	//block other remotions of the same user and while running
	std::lock_guard<std::mutex> locker(roomMu);
	for(int i = 0; i < (int) userVector.size(); i++){ 
		
		if(userVector[i].verifySocket(userSocket)){
			//prepare user left message
			char buffer[4096];
			strcpy(buffer,userVector[i].userName); // nomes de no maximo 14
			strcat(buffer," saiu da sala.\n");
			
			userVector.erase(userVector.begin()+i);
			
			sendMToAll(buffer);
			std::cout << "\n\rSERVER_LOG: " << buffer <<std::endl;
			return;
		}
	}
}

// send a message to all the users, server messages only
void ChatRoom :: sendMToAll(char * message){
	
	if (userNum > 0)
	{
		int bytesSend = 0;
		for (size_t i = 0; i < userVector.size(); i++)
		{
			userVector[i].sendNewM(message, 4096);//send the message to the user
		}

		std::cout << "\n\rSERVER_LOG: : Mensagem do sistema enviada."<< std::endl;	
	}
	
}

//send a message to all the users, but not the user that make the request
void ChatRoom :: sendUserM(int userSocket, char * message){	
	//block other messages while running this
	std::lock_guard<std::mutex> locker(roomMu);
	for (size_t i = 0; i < userVector.size(); i++)
	{
		if(!userVector[i].verifySocket(userSocket))
			userVector[i].sendNewM(message, strlen(message));
	}
	std::cout << "\n\rSERVER_LOG: Mensagem de um usuario enviada."<< std::endl;
}

void ChatRoom :: listenUser(UserData user, int socket){
	char buffer[4096];//4096
	
	while(true){
		int bytesRecv = user.receive(buffer, 4096);

		if(bytesRecv == -1){
			std::cerr << "\n\rSERVER_LOG: There was a connection issue with an user" << std::endl;
			break;
		}
		else if(bytesRecv == 0 || strcmp(buffer,"/quit")==0){
			
			std::cout << "\n\rSERVER_LOG: One user disconnected" << std::endl;
			break;

		}else if(strcmp(buffer, "/ping") == 0){
			//send /pong in to user
			strcpy(buffer,"Server: /pong");
			user.sendNewM(buffer, 4096);
			std:: cout << "\n\rSERVER_LOG: Ping request de " << user.userName << std::endl;

		}else if(strncmp(buffer,"/nickname",9)==0){
			//change the user nick name
			std:: cout << "\n\rSERVER_LOG: Change Nickname request de " << user.userName << std::endl;
			
			//verify if there is a name
			if(strlen(buffer) > 9 ){
				//change name
			}
		}
		else{
			// Display mesage
			std:: cout << "\n\rSERVER_LOG: Nova mensagem de " << user.userName <<";  "
			<< "\nReceived: " << std::string(buffer, 0, bytesRecv) << std::endl;
			bzero(buffer, 4096);
			
			char message[4101];// userName (14) + :(1) + buffer(4016)
			strcpy(message,user.userName);
			strcat(message,":");
			strcat(message,buffer);

			sendUserM(socket, message);//send the message to the other users
		}
	}
	
	user.closeSocket();
	removeUser(socket);
}

UserData :: UserData(int newSocket){
	isConnected = true;
	hasError = false;
	connectedSocket = newSocket;
}

UserData :: UserData(){
	
}

//send a message and if necessary resend it;
void UserData :: sendNewM(char * buffer, int bSize){
	for (size_t i = 0; i < 5; i++)
	{
		if(send(buffer, bSize) != -1)
			return;
	}
	std::cout << "\n\rSERVER_LOG: Problemas em se conectar com " << userName << std::endl;
	hasError = true;//will remove this user aand stop the connection
}

bool UserData :: verifySocket(int otherSocket){
	return connectedSocket == otherSocket ? true : false;
}

void UserData::operator=(UserData &x){
	for(int i = 0; i < 14; i++){
		userName[i] = x.userName[i];
	}

	ip = x.ip;
	hasError = x.hasError;
	isConnected = x.isConnected;
	connectedSocket = x.connectedSocket;
}