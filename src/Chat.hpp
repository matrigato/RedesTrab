#ifndef REDESTRAB_CHAT_SYST_HPP
#define REDESTRAB_CHAT_SYST_HPP
#include "Socket.hpp"
#include <vector> 
#include <mutex>

class UserData : public Socket{
	public:
		char userName[14];
		UserData(int newSocket);
		bool verifySocket(int otherSocket);
		long getIp();
		void sendNewM(char * buffer, int bSize);
	private:
		long ip; 
};

class ChatRoom{
	public:
		int userNum = 0;
		void sendMToAll(char * message);//message from server to all
		void sendUserM(int userSocket, char * message); //message from user to all the other users
		void whatsMyName();
		void addNewUser(int newSocket);
		void removeUser(int userSocket);
		void listenUser(UserData user, int socket);// listen to one user
		ChatRoom(unsigned short int port);
	
	private:
		std:: vector<UserData> userVector;
		bool hasError;
		bool isConnected;
		std::mutex roomMu;
};


#endif