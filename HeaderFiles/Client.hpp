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
};
