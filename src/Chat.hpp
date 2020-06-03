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
		bool verifySocket(int otherSocket);
		void sendNewM(char * buffer, int bSize);
		long ip; //NOTA: talvez tenha que mudar para char; não faz nada ainda
};

class ChatRoom{
	public:
		int userNum = 0;
		void sendMToAll(char * mesage);//mesage from server to all
		void sendUserM(int userSocket, char * mesage); //mesage from user to all the other users
		void whatsMyName();
		void addNewUser(int newSocket);
		void removeUser(int userSocket);
		void listenUser(UserData user, int socket);// listen to one user
		ChatRoom(unsigned short int port);
	
	private:
		std:: vector<UserData> userVector;
		std:: vector<std::thread> threadVector;
		std::mutex roomMu;
};


#endif