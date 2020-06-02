#ifndef REDESTRAB_CHAT_SYST_HPP
#define REDESTRAB_CHAT_SYST_HPP
#include "Socket.hpp"
#include <vector> 
#include <mutex>

class UserData : public Socket{
	public:
		char userName[50];
		UserData(int newSocket);
		bool verifySocket(int otherSocket);
		long getIp();
	private:
		long ip; 
};

class ChatRoom{
	public:
		int userNum = 0;
		void sendMToAll();
		void sendMToUser(char * message);
		void sendUserM(int userSocket, char * message);
		void whatsMyName();
		void addNewUser(int newSocket);
		void removeUser(int userSocket);
		ChatRoom(unsigned short int port);
	private:
		std:: vector<UserData> userVector;
		bool hasError;
		bool isConnected;
		std::mutex roomMu;
};


#endif