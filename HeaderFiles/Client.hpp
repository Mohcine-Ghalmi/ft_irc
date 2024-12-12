#pragma once
#include <string>
#include <iostream>
#include <string>
#include <cctype>
#include <climits>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <array>
#include <unistd.h>
#include "Replies.hpp"
#include <vector>
#include <map>

#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"

#define READ_BUFFER_SIZE 1024 * 32
#define BUFFER_SIZE 1024

#define MAX_CLIENTS 10

#define LOG_INFO(custom_string)  \
    std::cout  << "INFO: " << custom_string << RESET << std::endl;

#define LOG_ERROR(custom_string)  \
    std::cout << "ERROR: " << custom_string<< std::endl;

#define LOG_MSG(custom_string)  \
    std::cout << custom_string<< std::endl;

#define LOG_CLIENT(label, custom_string) \
    std::cout << GREEN "Client "<< label <<" <= " RESET << custom_string << std::endl;

#define LOG_SERVER(custom_string) \
    std::cout << BLUE "Server => " RESET << custom_string << std::endl;

class Replies;

class Client{
    private:
        int clientSocket;
        std::string nickName;
        std::string userName;
        std::string hostName;
        std::string realName;
        bool authenticated;
        std::string buffer;
        std::string password;
        Replies replies;

    public:

        Client(int socket);
        Client(const Client &client);
        ~Client();

        int getSocket();
        std::string getBuffer();
        std::string getNickName();
        std::string getUserName();
        std::string getHostname();
        bool isAuthenticated();

        std::string getPassword();
        void setNickName(const std::string &nick);
        void setUserName(const std::string &user);
        void setPassword(std::string &password);
        void setHostName(const std::string &host);
        void setRealName(const std::string &real);
        void appendToBuffer(const std::string &data);
        void authenticate();

        void    clearBuffer();
        void sendReply(int replyNumber, Client &client);
        void ERR_NICKNAMEINUSE(Client &client, const std::string &newNick);
        void ERR_NOSUCHNICK(Client &client, const std::string &targetNick);
        void ERR_ERRONEUSNICKNAME(Client &client, const std::string &invalidNick);
        void ERR_NEEDMOREPARAMS(Client &client, std::string cmd);
        void ERR_NOSUCHCHANNEL(Client &client, const std::string &channelName);
        void RPL_INVITE(Client &client, const std::string &invitedUser, const std::string &channelName);
        void ERR_INVITEONLYCHAN(Client &client, const std::string &channelName);
        void ERR_USERONCHANNEL(Client &client, const std::string &nick, const std::string &channelName);
        void ERR_CHANOPRIVSNEEDED(Client &client, const std::string &channelName);
        // void RPL_INVITESENTTO(Client &client, const std::string &channelName,std::string &userInvited);
        void RPL_INVITESENTTO(Client &client, const std::string &channelName, const std::string &userInvited);
        void RPL_KICKED(Client &client, const std::string &channelName, std::string &reason);
        void ERR_USERNOTINCHANNEL(Client &client,  const std::string &targetNick, const std::string &channelName);
        void RPL_CANTKICKSELF(Client &client, const std::string &channelName);
        // void RPL_TOPIC(Client &client,const std::string &setterName ,const std::string &topic, const std::string &channelName);
        void RPL_TOPIC(Client &client, const std::string &channelName, const std::string &topic);
        void ERR_BADCHANNELKEY(Client &client, const std::string &channelName);
        void RPL_NEWOPERATOR(Client &newOperator, Client &oldOperator, const std::string &channelName, bool remove, std::map<std::string, Client> &members);
        // void RPL_ALREADYOPERATOR(Client &client, const std::string &channelName, const std::string &newOperator, const bool &isOperator);
        void RPL_ALREADYOPERATOR(Client &client, const std::string &channelName, const std::string &newOperator, const bool &isOperator);
        void RPL_PUBLICCHANNEL(Client &client, const std::string &channelName, const bool &inviteOnly);
        void RPL_NAMREPLY(Client &operatorClient, const std::string &channelName,
                                std::map<std::string, Client> &members,
                                std::map<std::string, Client> &operators);
        void broadcastModeChange(const std::string &setterNick, const std::string &mode, const std::string &targetNick,  std::map<std::string, Client> &members,const std::string &channelName);
        void ERR_UNKNOWNMODE(Client &client, const std::string &channelName, char modechar);
};
