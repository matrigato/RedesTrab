#ifndef REDESTRAB_MAIN_SERVER_SYST_HPP
#define REDESTRAB_MAIN_SERVER_SYST_HPP
#include "Socket.hpp"
#include "Chat.hpp"
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

class MainServer{
    public:
        
        MainServer(unsigned short int port);
        void acceptC();
        void listenUser(int id, int sock);
        void closeServer();
        void whatsMyName();
        void setUserToWaiting(UserData user, int sock);
        int chatNum = 0;
        int waitingUserNum = 0;
        int getRoomByName(char * name);
        int newRoom(char * name);
        void removeWaitingUser(int id);
        void sendChatRooms(int id);
        void verifyServer();
        void startUser();
        bool isOpen;
        int tempUser = -1;
        ChatRoom rooms[5];
    private:
        int sockfd;
        UserData * waitingUsers;
        char tempIp[50];

};


#endif