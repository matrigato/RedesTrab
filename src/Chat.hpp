#ifndef REDESTRAB_CHAT_SYST_HPP
#define REDESTRAB_CHAT_SYST_HPP
#include "Socket.hpp"
#include <vector> 
#include <mutex>
#include <thread>

class UserData : public Socket{
	public:
		char userName[14];
		UserData(int newSocket);
		UserData(); // Only when going to be overwritten
		UserData(const UserData &x);
		bool verifySocket(int otherSocket);
		void sendNewM(char * buffer, int bSize);
		long ip;
		void operator=(const UserData &x);
};

class ChatRoom{
	public:
		int userNum = 0;
		void sendMToAll(char * mesage);//mesage from server to all
		void sendUserM(int userSocket, char * mesage); //mesage from user to all the other users
		void whatsMyName();
		void addNewUser();
		void removeUser(int userSocket);
		void listenUser(UserData user, int socket);// listen to one user
		ChatRoom(unsigned short int port);
		void acceptC();
		bool hasSocket();
		void closeRoom();
	
	private:
		std:: vector<UserData> userVector;
		std::mutex roomMu;
		std::mutex connectionMu;
		int sockfd;
		int waitingSocket;
};


#endif