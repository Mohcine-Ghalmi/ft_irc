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
    public:
        std::string port;
        int serverSocket;
        std::string password;
        std::vector<Client> clients;  // Store clients connected to the server
        int clientSockets[MAX_CLIENTS];

    public:
        Server(int argc, char **argv);
        void checkArgs(int argc, char **argv);
        void start();
        int getServerSocket();
        void acceptConnection();
        void handleClientMessage(int clientSocket, const std::string &message);
        void removeClient(int clientSocket);
        bool validatePassword(const std::string &clientPassword, const std::string &expectedPassword);
        // void processClientMessage(int clientSocket);
        ~Server();
};

