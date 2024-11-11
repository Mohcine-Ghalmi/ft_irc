#pragma once 

#include <string>
#include <ctime>

class Replies {
    public:
        std::string RPL_WELCOME(std::string nickName, std::string userName);                    // 001
        std::string RPL_YOURHOST(const std::string &serverName, const std::string &nickName);   // 002
        std::string RPL_CREATED(const std::string &serverName, const std::string &nickName);    // 003
        std::string RPL_MYINFO(const std::string &serverName, const std::string &nickName);     // 004
        std::string RPL_ISUPPORT(const std::string& serverName);                                // 005

        
        std::string ERR_PASSWDMISMATCH(const std::string &clientNick);                            // 464 
        std::string ERR_NEEDMOREPARAMS(int clientSocket);                                         // 461
};