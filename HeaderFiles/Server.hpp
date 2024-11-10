#pragma once
#include <string>
#include <vector>
#include <map>
#include <vector>
#include "Client.hpp"
#include <iostream>
#include "Replies.hpp"

class Server {
    private:
        std::string port;
        int serverSocket;
        std::string password;
        std::vector<Client> clients;  // Store clients connected to the server
    public:
        Server(int argc, char **argv);
        void start();
        ~Server();
        
        int getServerSocket();
        Client* getClientByNick(const std::string &targetNick);
        
        bool setUpClient(Client &client);
        
        void checkArgs(int argc, char **argv);
        void acceptConnection();
        void handleClientMessage(Client &client, const std::string &message); // for parsing and command handling 
        void processClienstMessage(fd_set readfds);// for getting message for client and send it to handleClientMessage
        
        
        bool processPassCommand(Client &client, const std::string &message);
        bool processNickCommand(Client &client, const std::string &message);
        bool processUserCommand(Client &client, const std::string &message);
        void updateNickUser(Client &client);
        bool isNickTaken(std::string &nick);
        bool processPrivMsgCommand(Client &sender, const std::string &message);
};

