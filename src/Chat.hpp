#ifndef REDESTRAB_CHAT_SYST_HPP
#define REDESTRAB_CHAT_SYST_HPP
#include "Socket.hpp"
#include <vector> 
#include <mutex>
#include <thread>

class UserData : public Socket{
	public:	
		char userName[50];
		bool canTalk = true;
		UserData(int newSocket);
		UserData(); // Only when going to be overwritten
		UserData(const UserData &x);
		bool verifySocket(int otherSocket);
		void setSocket(int socket);
		void sendNewM(char * buffer, int bSize);
		char userIp[50];
		void operator =(const UserData &x);
};

class ChatRoom{
	public:
		int userNum = 0;
		void sendMToAll(char * mesage);//mesage from server to all
		void sendUserM(int userSocket, char * mesage); //mesage from user to all the other users
		void whatsMyName();
		void addNewUser();
		void addUserFromServer(UserData newUser,int sock);
		void removeUser(int userSocket);
		void listenUser(int id, int socket);// listen to one user
		ChatRoom(unsigned short int port);//serverChatRoom
		ChatRoom();//defalt ChatRoom
		ChatRoom(const ChatRoom &x);
		void acceptC();
		bool hasSocket();
		void closeRoom();
		int getUserByName(char* name);
		char roomName[50];
		void operator=(const ChatRoom &x);
		int sockfd;
		int admSocket;
		bool isMainServer;
		UserData * users;
	private:
		void commands(char * buffer, int id);
		std::mutex roomMu;
		std::mutex connectionMu;
		int waitingSocket;
		char waitingIp[50];
};

#endif