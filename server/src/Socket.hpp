#ifndef REDESTRAB_SOCKET_HPP
#define REDESTRAB_SOCKET_HPP

class ServerSocket{
	private:
		bool hasError;
		bool isConnected;
		int clientSocket;
	public:
		ServerSocket(unsigned short int port);
		~ServerSocket();
		int receive(char *buffer, int bufferSize);
		int send(char *buffer, int bufferSize);
		void closeSocket();
};

class ClientSocket{
	private:
		bool hasError;
		bool isConnected;
		int serverSocket;
	
	public:
	ClientSocket(unsigned short int port, char* serverName);
	~ClientSocket();
	int receive(char *buffer, int bufferSize);
	int send(char *buffer, int bufferSize);
	void closeSocket();
};

#endif
