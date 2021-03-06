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
    for (size_t i = 0; i < 5; i++)
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

//imprime na tela o nome do servidor
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

    if(tempUser != -1)//user waiting
    {
        return;
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
	
    //verifica se teve algum problema no accept e continua o processo   
    if(newConnectionSocket != -1){
        
        //gera o ip do client na forma de string
        char ipStr[50];
        inet_ntop(AF_INET, (struct sockaddr*)&client.sin_addr, ipStr, 50);

        strcpy(tempIp,ipStr);

        tempUser= newConnectionSocket;
    }
}

//adiciona o novo usuario e se prepara para receber/enviar mensagens 
void MainServer :: setUserToWaiting(UserData user, int sock){
    int pos;
    for(int i = 0; i < 20; i++){
        if(!waitingUsers[i].isConnected){
            waitingUsers[i] = user;
            pos = i;
            break;
        }
    }

    listenUser(pos,sock);
}

void MainServer :: listenUser(int id, int sock){
    char buffer[4096];
	struct pollfd fds[1];
	fds[0].fd = sock;
	fds[0].events = 0;
	fds[0].events |= POLLIN;

    //first message
    strcpy(buffer,"Para se conectar utilize o comando /join seguido do nome da sala que gostaria de entrar.");
    waitingUsers[id].sendNewM(buffer,4096);
    sendChatRooms(id);
    //recive user name
    int Recv = waitingUsers[id].receive(buffer, 4096);
    if(strncmp(buffer,"/start ",7)==0){
        for (size_t i = 0; i < 50; i++)
            waitingUsers[id].userName[i] = buffer[i+7];
    }
    
    while (true)
    {
        if(poll(fds,1,3000) != 0){
            bzero(buffer,4096);

            int bytesRecv = waitingUsers[id].receive(buffer, 4096);

            if(bytesRecv == -1){
				std::cerr << "\n\rSERVER_LOG: There was a connection issue with an user" << std::endl;
                
                //remove o usuario da fila de espera
                waitingUsers[id].closeSocket();
                removeWaitingUser(id);
				break;
			}
			else if(bytesRecv == 0 || strcmp(buffer,"/quit")==0){
				
				std::cout << "\n\rSERVER_LOG: One user disconnected" << std::endl;
                
                //remove o usuario da fila de espera
                waitingUsers[id].closeSocket();
                removeWaitingUser(id);
				break;
			}
            else if(strncmp(buffer, "/join ",6) == 0){
                
                int size = strlen(buffer);
                if (size > 6)
                {
                    //get the room name
                    char roomName[50];
                    int i;
                    for (i = 0; i < size - 6; i++)
                    {
                        roomName[i] = buffer[i + 6];
                    }
                    roomName[i] = '\0';

                    //verify allowed name
                    bool allowed = true;
                    if(roomName[0] != '&' && roomName[0] != '#'){
                        strcpy(buffer,"O nome do canal deve começar com # ou &.");
                        allowed = false;
                    }
                    for(i = 1; roomName[i] != '\0'; i++){
                        if(!allowed) break;
                        switch(roomName[i]){
                            case ' ':
                                strcpy(buffer,"Espaços não são permitidos no nome do canal.");
                                allowed = false;
                                break;
                            case ',':
                                strcpy(buffer,"Vírgulas não são permitidas no nome do canal.");
                                allowed = false;
                                break;
                            case 7:
                                strcpy(buffer,"Control G não é permitido no nome do canal.");
                                allowed = false;
                                break;
                        }
                    }

                    if(allowed){
                        //verify repeated name
                        int roomId = getRoomByName(roomName);
                        if(roomId != -1){                        
                            
                            std::cout << "\n\rSERVER_LOG: um usuario esta entrando na sala "<< rooms[roomId].roomName << std::endl;
                            //adiciona o usuario na sala
                            UserData user = waitingUsers[id];

                            //remove o usuario da fila de espera
                            removeWaitingUser(id);
                            if(rooms[roomId].userNum > -1 && rooms[roomId].userNum < 20 ){
                                rooms[roomId].addUserFromServer(user, sock);
                                break;
                            }

                        }
                        else{

                            roomId = newRoom(roomName);
                            if(roomId != -1){
                                
                                std::cout << "\n\rSERVER_LOG: um usuario criou a sala "<< rooms[roomId].roomName << std::endl;
                                //adiciona o usuario na sala
                                UserData user = waitingUsers[id];

                                //remove o usuario da fila de espera
                                removeWaitingUser(id);
                                rooms[roomId].addUserFromServer(user, sock);
                                break;
                            }
                        }
                    }
                    else{
                        waitingUsers[id].sendNewM(buffer,4096); // Mensagem sobre caracteres invalidos
                    }

                    strcpy(buffer, "\n\rSERVER_LOG: ERRO; não foi possivel se connectar/criar a sala desejada, tente novamente.");
                    waitingUsers[id].sendNewM(buffer,4096);
                }
            }

            strcpy(buffer,"Para se conectar utilize o comando /join seguido do nome da sala que gostaria de entrar.");
            waitingUsers[id].sendNewM(buffer,4096);
            sendChatRooms(id);
        }
    }
    
}

//procura uma sala com nome name, retorna o id da sala ou -1
int MainServer :: getRoomByName(char * name){
    if(chatNum <= 0)
        return -1;
    
    for (int  i = 0; i < 5; i++)
    {
        if(strcmp(name, rooms[i].roomName) == 0 && rooms[i].userNum > -1)
            return i;
    }

    return -1;
}

//cria uma nova sala com o nome name, retorna o id da sala ou -1
int MainServer :: newRoom(char * name){
    //cant creat a new room
    if(chatNum >= 20)
        return -1;

    //create a new room
    for (int  i = 0; i < 20; i++)
    {
        if(rooms[i].userNum == -1){
            rooms[i] = ChatRoom();
            strcpy(rooms[i].roomName, name);
            chatNum++;
            return i;
        }
    }

    //error
    return -1;
}

//remove o usuario na posicao id
void MainServer :: removeWaitingUser(int id){
    
    //remove o usuario anterior e fica pronto para receber um novo
    waitingUsers[id] = UserData();
	waitingUsers[id].isConnected = false;
    waitingUserNum--;
    
    verifyServer();

}

//manda para o usuario indicado pelo id as sala que estão sendo utilizadas no momento
void MainServer :: sendChatRooms(int id){
    
    char buffer[4096];
    bzero(buffer, 4096);

    if(chatNum > 0){
        strcpy(buffer,"\nSalas abertas:");
        for (int i = 0; i < 5; i++)
        {
            if(rooms[i].userNum >= 0){

                strcat(buffer,"\n\t");
                strcat(buffer,rooms[i].roomName);
            }
        }

        strcat(buffer,"\n");
        
        //send to user
        waitingUsers[id].sendNewM(buffer,4096);
    }
    else if( chatNum == 0){
        strcpy(buffer, "\nNão temos nenhuma sala no momento; você pode criar a sua própria sala utilizando o comando /join");
        waitingUsers[id].sendNewM(buffer,4096);
    }
}

void MainServer :: verifyServer(){
   
    if(!isOpen)
        return;

    int num = 0;

    for(int i = 0; i < 20; i++){
        if(rooms[i].userNum >= 0)
            num++;
    }
    
    chatNum = num;

    if(num <= 0 && waitingUserNum <= 0){
        closeServer();
    }

}

void MainServer :: closeServer(){
    if(isOpen){
        isOpen = false;
        waitingUserNum = -1;
        chatNum = -1;
        close(sockfd);
    }
}

void MainServer :: startUser(){

    if(tempUser != -1){
        UserData newUser(tempUser);
        int sock = tempUser;
        strcpy(newUser.userIp,tempIp);
//        printf("\nIP: %s\n", newUser.userIp);
        strcpy(tempIp, "");
        tempUser = -1;
        waitingUserNum++;
        setUserToWaiting(newUser,sock);
    }

}