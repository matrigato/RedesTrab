#ifndef REDESTRAB_SOCKET_HPP
#define REDESTRAB_SOCKET_HPP

#include <mutex>

class ServerSocket{
	private:
		bool hasError;
		bool isConnected;
		int clientSocket;
		std::mutex mu;
	public:
		ServerSocket(unsigned short int port);
		~ServerSocket();
		int receive(char *buffer, int bufferSize);
		int send(char *buffer, int bufferSize);
		void closeSocket();
		void wathsMyName();
		void sendM();
		void readM();
};

class ClientSocket{
	private:
		bool hasError;
		bool isConnected;
		int serverSocket;
		std::mutex mu;
	public:
	ClientSocket(unsigned short int port, char* serverName);
	~ClientSocket();
	int receive(char *buffer, int bufferSize);
	int send(char *buffer, int bufferSize);
	void closeSocket();
	void sendM();
	void readM();
};

#endif
