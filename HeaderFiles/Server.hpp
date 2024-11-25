#pragma once
#include <string>
#include <vector>
#include <map>
#include <vector>
#include "Client.hpp"
#include <iostream>
#include "Replies.hpp"
#include "../HeaderFiles/Channel.hpp"
#include <sstream>
class Channel;
class Server {
    private:
        std::string port;
        int serverSocket;
        std::string password;
        std::vector<Client> clients;  // Store clients connected to the server
    public:
        std::map<std::string, Channel> channels;
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
        bool processJoinCommand(Client &client, const std::string &message);
        //
        bool processModeCommand(Client &operatorClient, const std::string &message);
        bool processINVITECommand(Client &operatorClient, const std::string &message);
        bool processPartCommand(Client &client, const std::string &message);
        bool processKICKCommand(Client &operatorClient, const std::string &message);
        bool processTOPICcommand(Client &operatorClient, const std::string &message);
        //
        bool checkInvitesToChannel(Client &operatorClient, Channel *channel, std::string &channelName, Client *userInvited);

        Channel* getChannel(const std::string &channelName);
        Channel* createChannel(const std::string &channelName);
        bool joinChannel(Client &client, const std::string &channelName,const std::string &key); // JOIN
        bool leaveChannel(Client &client, const std::string &channelName); // PART
        void sendMessageToChannel(Client &sender, const std::string &channelName, const std::string &messageText);

        // helpers
        bool proccessCommandHelper(std::string cmd, std::string dif);
};

void    ft_setInviteOnly(Channel *channel, Client &operatorClient, char mode);
void    ft_removeAddOperator(Client &operatorClient, Client *newOperator, Channel *channel, const char &adding);
