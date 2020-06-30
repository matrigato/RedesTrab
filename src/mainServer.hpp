#ifndef REDESTRAB_MAIN_SERVER_SYST_HPP
#define REDESTRAB_MAIN_SERVER_SYST_HPP
#include "Socket.hpp"
#include "Chat.hpp"
#include <vector> 
#include <mutex>
#include <thread>

class MainServer{
    public:
        UserData * waitingUsers;
        ChatRoom * rooms;
        MainServer(unsigned short int port);
        void acceptC();
        void listenUser(UserData user);
        void closeServer();
        void whatsMyName();
        int chatNum = 0;
        int waitingUserNum = 0;
    private:
        int sockfd;
        bool isOpen;
        
};







#endif