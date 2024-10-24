#include "../HeaderFiles/Replies.hpp"
#include <sstream>

std::string Replies::ERR_NEEDMOREPARAMS(int clientSocket) {
    std::stringstream ss;
    ss << clientSocket;
    return ss.str() + " PASS :Not enough parameters\r\n";
}

std::string Replies::RPL_WELCOME(std::string nickName, std::string userName) {
    return  ":localhost 001 " + nickName + " :Welcome to the Hell! Network, " + nickName \
            + "[!" + userName + "@localhost]\r\n";
}

std::string Replies::RPL_YOURHOST(const std::string &serverName, const std::string &nickName) {
    std::stringstream ss;
    ss << ":" << serverName << " 002 " << nickName
       << " :Your host is " << serverName << ", running version 1.0\r\n";
    return ss.str();
}
