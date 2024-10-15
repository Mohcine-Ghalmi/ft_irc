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

#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"

#define READ_BUFFER_SIZE 1024 * 32
#define BUFFER_SIZE 1024

#define LOG_INFO(custom_string)  \
    std::cout << GREEN << "INFO: " << RESET << custom_string<< std::endl; 

#define LOG_ERROR(custom_string)  \
    std::cout << RED << "ERROR: " << RESET << custom_string<< std::endl;

#define LOG_MSG(custom_string)  \
    std::cout << custom_string<< std::endl;

#define MAX_CLIENTS 10

class Client {
    private:
        int clientSocket;
        std::string nickname;
        std::string username;
        std::string buffer;
        bool authenticated;

    public:
        Client(int socket);
        std::string getNickName();
        std::string getUserName();
        void setNickName(const std::string &nick);
        void setUserName(const std::string &user);
        void appendToBuffer(const std::string &data);
        std::string getBuffer();
        bool isAuthenticated();
        void authenticate();
        int getSocket();
        ~Client();
};
