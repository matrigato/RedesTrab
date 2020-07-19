#include "Chat.hpp"
#include "Socket.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <iostream>
#include <thread> 
#include <poll.h>

ChatRoom :: ChatRoom(unsigned short int port){
	//starting users
	users = (UserData*)malloc(sizeof(UserData) * 20);
	for(int i = 0; i < 20; i++){
		users[i] = UserData();
		users[i].isConnected = false;
	}

	//starting values
	waitingSocket = -1;
	bzero(waitingIp,50);
	isMainServer = true;

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

	whatsMyName();
	std::cout << "\n\rSERVER_LOG: A sala de chat foi aberta; Esperando por usuarios..." << std::endl;
	
}

ChatRoom :: ChatRoom(){
	
	users = (UserData*)malloc(sizeof(UserData) * 20);
	for(int i = 0; i < 20; i++){
		users[i] = UserData();
		users[i].isConnected = false;
	}
	sockfd = -1;
	waitingSocket = -1;
	bzero(waitingIp,50);
	bzero(roomName,50);
	isMainServer = false;
}

void ChatRoom::acceptC(){

	if(!isMainServer){
		std:: cout << "\n\rSERVER_LOG: ERRO; apenas o servidor principal pode aceitar novos clients" << std:: endl;
		return ;
	}

	if(hasSocket())
		return;

	if(userNum == -1){//closing room; No more users in the room.
		std:: cout << "\n\rSERVER_LOG: todos os usuarios sairam; Concluindo Processo" << std:: endl;
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
	int  * newConnectionSocket =  (int*)malloc(sizeof(int));

	std:: cout << "\n\rSERVER_LOG: ERRNO_B_ACCEPT: " << errno << std:: endl;
	*newConnectionSocket = accept(sockfd, (struct sockaddr*) &client, &clientSize); // new socket number
	std:: cout << "\n\rSERVER_LOG: ERRNO_A_ACCEPT: " << errno << std:: endl;
	

	//gera o ip do client na forma de string
	char ipStr[50];
	inet_ntop(AF_INET, (struct sockaddr*)&client.sin_addr, ipStr, 50);
	strcpy(waitingIp, ipStr);

	if(*newConnectionSocket == -1){
		free(newConnectionSocket);
		//Problem with client connecting
		return ;
	}

	if(userNum >=0  && userNum < 20){//max number of users in the same room
		waitingSocket = *newConnectionSocket;
		//create a new thread and add it to thread vector

	}
	free(newConnectionSocket);
	return ;
}

void ChatRoom:: whatsMyName(){
	if(isMainServer){
		char name[99];
		if(gethostname(name,99)!= 0){
			std::cout << "I don't know my name."<< std::endl;
			return;
		}
		std::cout << "My name is "<< name << std::endl;
	}
	else{
		std::cout << "My name is "<< roomName << std::endl;
	}
}

void ChatRoom :: addNewUser(){
	if(!hasSocket())
		return;

	if(!isMainServer){
		std:: cout << "\n\rSERVER_LOG: ERRO; apenas o servidor principal pode aceitar novos clients" << std:: endl;
		return ;
	}

	if (userNum >= 0 && userNum < 20)
	{
		int newSocket = waitingSocket;
		waitingSocket = -1;//removes the waitng socket
		if(userNum  == 0)
			admSocket = newSocket;
		UserData newUser(newSocket);//create the new user and starts to listen to it
		//seting base name
		strcpy(newUser.userName,"user");
		char num[] = "$$";   
		sprintf(num, "%d", userNum);

		strcat(newUser.userName,num);

		//seting ip
		strcpy(newUser.userIp, waitingIp);
		bzero(waitingIp,50);

		//new user id
		int newId = -1;

		//put the user in the vector
		for (size_t i = 0; i < 20; i++)
		{
			if (!users[i].isConnected){
				users[i] = newUser;
				newId = i;
				break;
			}
		}
		userNum++;//update the user num

		std:: cout << "\n\rSERVER_LOG: ERRNO_ADD: " << errno << std:: endl;
		
		//start to listen the user with the new id
		listenUser(newId ,newSocket);
		
	}else{
		waitingSocket = -1;
	}
}

void ChatRoom :: addUserFromServer(UserData newUser, int sock){

	if(userNum != -1 && userNum < 20){
		int pos;
		for(int i = 0; i < 20; i++){
			if(!users[i].isConnected){
				users[i] = newUser;
				userNum++;
				pos = i;
			}
		}

		//inicia a troca de mensagens com o user
		listenUser(pos,sock);
	}
}

bool ChatRoom::hasSocket(){
	std::lock_guard<std::mutex> locker(connectionMu);
	return waitingSocket != -1 ?true : false;
}

void ChatRoom :: removeUser(int userSocket){
	
	//block other remotions of the same user and block messages from being send while running
	std::lock_guard<std::mutex> locker(roomMu);
	close(userSocket);
	for(int i = 0; i < 20; i++){ 
		
		if(users[i].verifySocket(userSocket)){
			//prepare user left message
			char buffer[4096];
			strcpy(buffer,users[i].userName); // nomes de no maximo 14
			strcat(buffer," saiu da sala.\n");
			
			users[i].isConnected = false;

			sendMToAll(buffer);
			std::cout << "\n\rSERVER_LOG: " << buffer <<std::endl;
			userNum--;
			if(userNum == 0){
				userNum = -1;
				closeRoom();
				return;
			}
			
			if (users[i].verifySocket(admSocket))
			{
				for (size_t j = 0; j < 20; j++)
				{
					if(users[j].isConnected){
						admSocket = users[j].connectedSocket;
						char buffer[4096] = "SERVER: Você é o novo adm da sala";
						users[j].sendNewM(buffer,4096);
						return;	
					}
				}
				
			}
			return;
		}
	}
}

// send a message to all the users, server messages only, 
void ChatRoom :: sendMToAll(char * message){
	
	if (userNum > 0)
	{
		for (size_t i = 0; i < 20; i++)
		{
			if (users[i].isConnected)
				users[i].sendNewM(message, 4096);//send the message to the user
		}

		std::cout << "\n\rSERVER_LOG: : Mensagem do sistema enviada."<< std::endl;	
	}
	
}

//send a message to all the users, but not the user that make the request, don't use mutex becase the server message have bigger priority
void ChatRoom :: sendUserM(int userSocket, char * message){	
	//block other messages while running this
	std::lock_guard<std::mutex> locker(roomMu);
	for (size_t i = 0; i < 20; i++)
	{
		if(!users[i].verifySocket(userSocket) && users[i].isConnected)
			users[i].sendNewM(message, strlen(message));
	}
	std::cout << "\n\rSERVER_LOG: Mensagem de um usuario enviada."<< std::endl;
}

void ChatRoom :: listenUser(int id, int socket){
	char buffer[4096];
	struct pollfd fds[1];
	fds[0].fd = socket;
	fds[0].events = 0;
	fds[0].events |= POLLIN; 

	while(true){
		if(poll(fds,1,3000) != 0){

			bzero(buffer, 4096);

			std:: cout << "\n\rSERVER_LOG: ERRNO_POLL: " << errno << std:: endl;
			int bytesRecv = users[id].receive(buffer, 4096);

			if(bytesRecv == -1){
				std::cerr << "\n\rSERVER_LOG: There was a connection issue with an user" << std::endl;
				break;
			}
			else if(bytesRecv == 0 || strcmp(buffer,"/quit")==0){
				
				std::cout << "\n\rSERVER_LOG: One user disconnected" << std::endl;
				break;

			}
			else if(buffer[0] == '/'){ //um comando esta sendo chamado
				commands(buffer, id);
			}
			else{
				std:: cout << "\n\rSERVER_LOG: ERRNO_SEND: " << errno << std:: endl;
				// Display mesage
				std:: cout << "\n\rSERVER_LOG: Nova mensagem de " << users[id].userName <<";  "
				<< "\nReceived: " << std::string(buffer, 0, bytesRecv) << std::endl;
	            
				if(users[id].canTalk){
					char message[4096];// userName (14) + ": "(2) + buffer(4096)
					strcpy(message,users[id].userName);
					strcat(message,": ");
					strcat(message,buffer);

					sendMToAll(message);
				}
				else{
					strcpy(buffer, "SERVER: O adm mutou você.");
					users[id].sendNewM(buffer, 4096);
				}
				
			}
			bzero(buffer,4096);
		}
	}
	
	users[id].closeSocket();
	removeUser(socket);
}

void ChatRoom:: closeRoom(){
	close(sockfd);
	free(users);
}

void ChatRoom :: commands(char * buffer, int id){
	if(strcmp(buffer, "/ping") == 0){
		//send /pong in to user
		strcpy(buffer,"Server: pong");
		users[id].sendNewM(buffer, 4096);
		std:: cout << "\n\rSERVER_LOG: Ping request de " << users[id].userName << std::endl;

	}
	else if(strncmp(buffer,"/nickname ",10)==0){
		//change the user nick name
		std:: cout << "\n\rSERVER_LOG: Change Nickname request de " << users[id].userName << std::endl;

		//verify if there is a name
		if(strlen(buffer) >  13){
				//change name
				int size = strlen(buffer) - 10;
			if(size > 50)
				size = 50;

			for(int i = 0; i < size; i++)
				users[id].userName[i] = buffer[i + 10];
			users[id].userName[size] = '\0';
			
			strcpy(buffer, "\n\rSERVER_LOG: Sucesso no processo de alterar o nome do usuario.");
			//user feedback	
			users[id].sendNewM(buffer,4096);
		}
		else{
			
			//user feedback	
			users[id].sendNewM(buffer,4096);
		}
	}
	else if(strncmp(buffer,"/kick ",6) == 0){
		if(!users[id].verifySocket(admSocket)){
			//o usuario não pode usar o comando
			strcpy(buffer, "Server: comando invalido.");
			return;
		}
		
		if(strlen(buffer) >  9){
			char name[50];
			
			int sizeb = strlen(buffer) - 6;
			if(sizeb > 50)
				sizeb = 50;

			for (int i = 0; i < sizeb; i++)
			{
				name[i] = buffer[6 + i];
			}
			name[sizeb] = '\0';
			std:: cout << "\n\rSERVER_LOG: Kick request de " << users[id].userName << " para "<< name << std::endl;
			int otherId = getUserByName(name);
			if(otherId != -1){
				removeUser(users[otherId].connectedSocket);
				strcpy(buffer,"\n\rSERVER_LOG: Sucesso no processo de Kick");	
			}
			else{
				strcpy(buffer,"\n\rSERVER_LOG: Falha no processo de kick; Verifique se o nome do usuario esta correto");
			}
			//user feedback	
			users[id].sendNewM(buffer,4096);

		}
		else{
			strcpy(buffer,"\n\rSERVER_LOG: Falha no processo de kick; Verifique se o nome do usuario esta correto");
			//user feedback	
			users[id].sendNewM(buffer,4096);
		}
	}
	else if(strncmp(buffer,"/mute ",6) == 0){
		if(!users[id].verifySocket(admSocket)){
			//o usuario não pode usar o comando
			strcpy(buffer, "Server: comando invalido.");
			return;
		}
		
		if(strlen(buffer) >  9){
			
			int size = strlen(buffer) - 6;
			
			char name[50];

			if(size > 50)
				size = 50;

			for (int i = 0; i < size; i++)
			{
				name[i] = buffer[6 + i];
			}

			name[size] = '\0';

			std:: cout << "\n\rSERVER_LOG: Mute request de " << users[id].userName << " para " << name<< std::endl;
			
			//mute the client with the search name
			int otherId = getUserByName(name);
			if(otherId != -1){
				users[otherId].canTalk = false;	
				strcpy(buffer,"\n\rSERVER_LOG: Sucesso no processo de Mute");
			}
			else{
				strcpy(buffer,"\n\rSERVER_LOG: Falha no processo de Mute; Verifique se o nome do usuario esta correto");
			}

			//user feedback	
			users[id].sendNewM(buffer,4096);
		}
		else{
			strcpy(buffer,"\n\rSERVER_LOG: Falha no processo de Mute; Verifique se o nome do usuario esta correto");
			//user feedback	
			users[id].sendNewM(buffer,4096);
		}

	}
	else if(strncmp(buffer,"/unmute ",8) == 0){
		if(!users[id].verifySocket(admSocket)){
			//o usuario não pode usar o comando
			strcpy(buffer, "Server: comando invalido.");
			return;
		}

		if(strlen(buffer) >  11){
			char name[50];
			int size = strlen(buffer) - 8;
			
			if(size > 50)
				size = 50;
			
			for (int i = 0; i < size; i++)
			{
				name[i] = buffer[8 + i];
			}
			name[size] = '\0';

			std:: cout << "\n\rSERVER_LOG: UnMute request de " << users[id].userName << " para " << name<< std::endl;
			//mute the client with the searched name
			int otherId = getUserByName(name);
			if(otherId != -1){
				users[otherId].canTalk = true;
				strcpy(buffer,"\n\rSERVER_LOG: Sucesso no processo de UnMute");
			}
			else
				strcpy(buffer,"\n\rSERVER_LOG: Falha no processo de UnMute; Verifique se o nome do usuario esta correto");
			
			//user feedback	
			users[id].sendNewM(buffer,4096);
		}
		else{
			strcpy(buffer,"\n\rSERVER_LOG: Falha no processo de UnMute; Verifique se o nome do usuario esta correto");
			//user feedback	
			users[id].sendNewM(buffer,4096);
		}
	}
	else if(strncmp(buffer,"/whois ",7) == 0){
		if(!users[id].verifySocket(admSocket)){
			//o usuario não pode usar o comando
			strcpy(buffer, "Server: comando invalido.");
			return;
		}
		else{
			if(strlen(buffer) >  10)
			{
				char name[50];
				int size = strlen(buffer) - 7;
				
				if(size > 50)
				size = 50;

				for (int i = 0; i < size; i++)
				{
					name[i] = buffer[7 + i];
				}
				name[size] = '\0';

				std:: cout << "\n\rSERVER_LOG: whois request de " << users[id].userName << " para " << name<< std::endl;
				//mute the client with the searched name
				int otherId = getUserByName(name);
				if(otherId != -1){

					strcpy(buffer,"\n\rSERVER_LOG: O ip de ");
					strcat(buffer, users[otherId].userName);
					strcat(buffer," é ");
					strcat(buffer,users[otherId].userIp);
					strcat(buffer, ".\n");
				}
				else{
					strcpy(buffer,"\n\rSERVER_LOG: Falha no processo de Whois; Verifique se o nome do usuario esta correto");
				}

				//user feedback	
				users[id].sendNewM(buffer,4096);
			}
			else{
				strcpy(buffer,"\n\rSERVER_LOG: Falha no processo de Whois; Verifique se o nome do usuario esta correto");
				//user feedback	
				users[id].sendNewM(buffer,4096);
			}
		}

	}
	else if(strcmp(buffer,"/help") == 0){
		strcpy(buffer,"\nComandos:\n");
		strcat(buffer,"\t/ping: o servidor retorna pong.\n");
		strcat(buffer,"\t/nickname NAME: muda o nome do usuario no servidor para o nome indicado no campo NAME.\n");
		strcat(buffer,"\t/kick NAME: remove do servidor o usuario de nome NAME (adm apenas).\n");
		strcat(buffer,"\t/mute NAME: remove a capacidade de falar do usuario de nome NAME (adm apenas).\n");
		strcat(buffer,"\t/unmute NAME: retorna a capacidade de falar do usuario de nome NAME (adm apenas).\n");
		strcat(buffer,"\t/whois NAME: retorna o ip do usuario de nome NAME (adm apenas).\n");
		strcat(buffer,"\t/users: mostra os usuarios na sala.\n");

		users[id].sendNewM(buffer, 4096);
	}
	else if(strcmp(buffer,"/users")==0){
		strcpy(buffer,"\nUSERS:");

		for(int i = 0; i < 20; i++){
			if(users[i].isConnected){
				strcat(buffer,"\n\t");
				strcat(buffer,users[i].userName);
			}
		}

		strcat(buffer,"\n");
		users[id].sendNewM(buffer, 4096);
	}
	else{
		strcpy(buffer,"Server: Comando invalido, tente usar /help para ver os comandos.");
		users[id].sendNewM(buffer, 4096);
	}
	
}

int  ChatRoom :: getUserByName(char* name){
	for(int i = 0; i < 20; i++){
		if(users[i].isConnected && (strcmp(users[i].userName, name) == 0)){
			return i;
		}
	}
	return -1;
}

UserData :: UserData(int newSocket){
	isConnected = true;
	hasError = false;
	connectedSocket = newSocket;
	canTalk = true;
	bzero(userIp, 50);
}

void UserData :: setSocket(int socket){
	connectedSocket = socket;
}

UserData::UserData(){
	isConnected = false;
	hasError = false;
	connectedSocket = -1;
}	

UserData::UserData(const UserData &x){
	for(int i = 0; i < 14; i++){
		userName[i] = x.userName[i];
	}

	strcpy(userIp, x.userIp);
	hasError = x.hasError;
	isConnected = x.isConnected;
	connectedSocket = x.connectedSocket;
}

//send a message and if necessary resend it;
void UserData :: sendNewM(char * buffer, int bSize){
	for (size_t i = 0; i < 5; i++)
	{
		if(send(buffer, bSize) > -1)
			return;
		std::cout << "\n\rSERVER_LOG: ERRNO: "<< errno << std:: endl; 
		
	}
	std::cout << "\n\rSERVER_LOG: Problemas em se conectar com " << userName << std::endl;
	hasError = true;//will remove this user and stop the connection
}

bool UserData :: verifySocket(int otherSocket){
	return connectedSocket == otherSocket ? true : false;
}

void UserData::operator=(const UserData &x){
	
	for(int i = 0; i < 50; i++){
		userName[i] = x.userName[i];
	}

	strcpy(userIp, x.userIp);
	hasError = x.hasError;
	isConnected = x.isConnected;
	connectedSocket = x.connectedSocket;
}

void ChatRoom :: operator=(const ChatRoom &x){
	for (size_t i = 0; i < 50; i++)
	{
		roomName[i] = x.roomName[i];
	}

	userNum = x.userNum;
	for (size_t i = 0; i < 20; i++)
	{
		users[i] = x.users[i];
	}
	
	sockfd = x.sockfd;
	admSocket = x.admSocket;
	isMainServer = x.isMainServer;
}