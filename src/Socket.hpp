#ifndef REDESTRAB_SOCKET_HPP
#define REDESTRAB_SOCKET_HPP
#include <vector> 
#include <mutex>

// Abstract class
class Socket{

	protected:
		bool hasError;
		std::mutex mu;
		std::mutex mu2;
	public:
		int connectedSocket;
		bool isConnected;
		virtual ~Socket() = 0; // Makes class abstract, but doesn't require definition for destructor.
		int receive(char *buffer, int bufferSize);
		int send(char *buffer, int bufferSize);
		void closeSocket();
		void sendM();
		virtual void readM();
};

class ServerSocket : public Socket{
	public:
		ServerSocket(unsigned short int port);
		void whatsMyName();
		void readM();
};

class ClientSocket : public Socket{
	public:
		ClientSocket(unsigned short int port, char* serverName);
		void readM();
};


#endif
