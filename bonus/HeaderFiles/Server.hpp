#pragma once
#include <string>
#include <vector>
#include "Client.hpp"
#include <iostream>
#include "Replies.hpp"
#include "../HeaderFiles/Channel.hpp"
#include <sstream>
#include <fstream>
#include <curl/curl.h>

class Channel;
class Server {
    private:
        std::string port;
        int serverSocket;
        std::string password;
        std::vector<Client> clients;
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
        void handleClientMessage(Client &client, const std::string &message);
        void processClienstMessage(fd_set readfds);


        bool processPassCommand(Client &client, const std::string &message);
        bool processNickCommand(Client &client, const std::string &message);
        bool processUserCommand(Client &client, const std::string &message);
        bool isNickTaken(std::string &nick);
        bool processPrivMsgCommand(Client &sender, const std::string &message);
        bool processJoinCommand(Client &client, const std::string &message);
        bool processBotcommand(Client &client, const std::string &message);
        //
        bool processModeCommand(Client &operatorClient, const std::string &message);
        bool processINVITECommand(Client &operatorClient, const std::string &message);
        bool processPartCommand(Client &client, const std::string &message);
        bool processKICKCommand(Client &operatorClient, const std::string &message);
        bool processTOPICcommand(Client &operatorClient, const std::string &message);

        bool checkInvitesToChannel(Client &operatorClient, Channel *channel, std::string &channelName, Client *userInvited);

        Channel* getChannel(const std::string &channelName);
        Channel* createChannel(const std::string &channelName);
        bool joinChannel(Client &client, const std::string &channelName,const std::string &key);
        bool leaveChannel(Client &client, const std::string &channelName);
        bool sendMessageToChannel(Client &sender, const std::string &channelName, const std::string &messageText);
        void removeUserFromChannels(const std::string &nickName);

        bool proccessCommandHelper(std::string cmd, std::string dif);

        void RPL_BOTCALLED(Client &client, const std::string &channelName, std::stringstream &Weather, bool isClient);
        void ERR_BOTCALLED(Client &client, const std::string &channelName,const std::string &Weather, bool isClient);
        void sendErrCannotSendToChan(Client &client, const std::string &channelName);
        void sendUnknownCommandReply(Client &client, const std::string &command);
};

void    ft_setInviteOnly(Channel *channel, Client &operatorClient, char mode);
void    ft_removeAddOperator(Client &operatorClient, Client *newOperator, Channel *channel, const char &adding);
void    removeCarriageReturn(std::string &str);
void    sendHellGate(int client_socket);
bool    isValidNick(const std::string &nickname);
void    sendCapResponse(int clientSocket);
std::vector<std::string> splitMessages(const std::string &buffer);
std::vector<std::string> splitByDelimiter(const std::string &input, const std::string &delimiter);
bool    hasNewline(const std::string& message);
void    sendInviteReply(Client operatorClient,Channel &channel, const std::string &invited);
void    sendkickRepleyToChannel(Client operatorClient,Channel &channel, const std::string &kickedUser);
void    sendTopicRepleyToChannel(Client operatorClient,Channel &channel, const std::string &topic);
