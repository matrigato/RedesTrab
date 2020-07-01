#ifndef REDESTRAB_MAIN_SERVER_SYST_HPP
#define REDESTRAB_MAIN_SERVER_SYST_HPP
#include "Socket.hpp"
#include "Chat.hpp"
#include <vector> 
#include <mutex>
#include <thread>

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
    private:
        int sockfd;
        bool isOpen;
        UserData * waitingUsers;
        ChatRoom * rooms;
        
};


#endif