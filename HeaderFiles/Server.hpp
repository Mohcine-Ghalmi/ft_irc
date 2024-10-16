#pragma once
#include <string>
#include <vector>
#include <map>
#include <vector>
#include "Client.hpp"
#include <iostream>
// #include <Channel.hpp>
// #include <Helper.hpp>

class Server {
    private:
        std::string port;
        int serverSocket;
        std::string password;
        std::vector<Client> clients;  // Store clients connected to the server
    public:
        Server(int argc, char **argv);
        void checkArgs(int argc, char **argv);
        void start();
        void shutdownServer();
        int getServerSocket();
        void acceptConnection();
        void handleClientMessage(Client &client, const std::string &message); // for parsing and command handling 
        bool validatePassword(const std::string &clientPassword, const std::string &expectedPassword);
        bool setUpClient(std::string message, Client &client);
        void processClienstMessage(fd_set readfds);// for getting message for client and send it to handleClientMessage
        ~Server();
};

