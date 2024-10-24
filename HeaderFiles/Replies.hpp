#pragma once 

#include <string>


class Replies {
    public:
        std::string RPL_WELCOME(std::string nickName, std::string userName);
        std::string RPL_YOURHOST(const std::string &serverName, const std::string &nickName);

        std::string ERR_NEEDMOREPARAMS(int clientSocket);
};

