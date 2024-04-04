#pragma once

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
#include <sstream>

typedef struct addrinfo info;

///

typedef struct addrinfo info;

#define READ_BUFFER_SIZE 1024 * 32


#define LOG_INFO(custom_string)  \
    std::cout << "INFO: " << custom_string << std::endl; 

#define LOG_ERROR(custom_string)  \
    std::cout << "ERROR: " << custom_string << std::endl;


///


class Helper
{
    // private:
    //     /* data */
    public:
        Helper(/* args */);
        bool isall_objsdigits(std::string str);
        std::vector<std::string> CommandParser(std::string line);
        ~Helper();
};

Helper::Helper(/* args */)
{
}

Helper::~Helper()
{
}

bool Helper::isall_objsdigits(std::string str) 
{
    for (int i = 0; i < str.length(); i++)
        if (!std::isdigit((unsigned char)str[i]))
            return false;
    return true;
}



std::vector<std::string> Helper::CommandParser(std::string line)
{
    std::size_t start = 0;
    std::vector<std::string> args;
    args.reserve(3);


    std::stringstream ss(line);
    std::string token;

    std::getline(ss, token, ' ');
    args.push_back(token);
    while (std::getline(ss, token, ' ')) {
        if (token.front() == ':') { 
            args.push_back(token.substr(1));
            break;
        } else {
            args.push_back(token);
        }
    }

    return args;
}
