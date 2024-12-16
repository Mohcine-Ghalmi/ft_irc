#pragma once

#include <string>
#include <ctime>

class Replies {
    public:
        std::string RPL_WELCOME(std::string nickName, std::string userName);
        std::string RPL_YOURHOST(const std::string &serverName, const std::string &nickName);
        std::string RPL_CREATED(const std::string &serverName, const std::string &nickName);
        std::string RPL_MYINFO(const std::string &serverName, const std::string &nickName);
        std::string RPL_ISUPPORT(const std::string& serverName);


        std::string ERR_PASSWDMISMATCH(const std::string &clientNick);
        std::string ERR_NEEDMOREPARAMS(int clientSocket);
};
