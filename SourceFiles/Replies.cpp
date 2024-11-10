#include "../HeaderFiles/Replies.hpp"
#include <sstream>
#include <iostream>

std::string Replies::ERR_NEEDMOREPARAMS(int clientSocket) {
    std::stringstream ss;
    ss << clientSocket;
    return ss.str() + " PASS :Not enough parameters\r\n";
}

std::string Replies::RPL_WELCOME(std::string nickName, std::string userName) {
    std::stringstream ss;
    ss << ":localhost 001 " << nickName << " :Welcome to the Hell! Network, " << nickName
         << "[!" << userName << "@localhost]\r\n";
    return ss.str();
}

std::string Replies::RPL_YOURHOST(const std::string &serverName, const std::string &nickName) {
    std::stringstream ss;
    ss << ":" << serverName << " 002 " << nickName
       << " :Your host is " << serverName << ", running version 1.0\r\n";
    return ss.str();
}

std::string Replies::RPL_CREATED(const std::string &serverName, const std::string &nickName) {
    std::stringstream ss;

    time_t now = time(0);
    struct tm *timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, 80, "%c", timeinfo); // Format date/time

    ss << ":" << serverName << " 003 " << nickName
       << " :This server was created " << buffer << "\r\n";
    return ss.str();
}

std::string Replies::RPL_MYINFO(const std::string &serverName, const std::string &nickName) {
    std::stringstream ss;

    std::string userModes = "iwso";      // Example user modes
    std::string channelModes = "mtov";   // Example channel modes

    ss << ":" << serverName << " 004 " << nickName << " " << serverName << " 1.0 " << userModes << " " << channelModes << "\r\n";
    return ss.str();
}

std::string Replies::RPL_ISUPPORT(const std::string& serverName) {
    std::stringstream response;

    response << ":" << serverName << " 005 * " // Server name and numeric reply
             << "CHANTYPES=#& "               // Supported channel prefixes
             << "PREFIX=(ov)@+ "              // Operator/voice prefixes
             << "MODES=4 "                    // Max modes that can be set in a single command
             << "CHANLIMIT=#&:10 "            // Max number of channels a user can join
             << "NICKLEN=9 "                  // Max nickname length
             << "TOPICLEN=390 "               // Max topic length
             << "KICKLEN=390 "                // Max kick reason length
             << "NETWORK=MyNetwork "          // Network name
             << "CASEMAPPING=ascii "          // Case mapping rules
             << "CHARSET=utf-8 "              // Character encoding
             << "CHANNELLEN=50 "              // Max length for a channel name
             << "SAFELIST "                   // Indicates support for the /LIST command without flooding
             << ":are supported by this server";

    return response.str();
}

std::string Replies::ERR_NICKNAMEINUSE(const std::string &serverName, const std::string &nickname) {
    std::stringstream ss;

    ss << ":" << serverName << " 433 " << nickname << " :Nickname is already in use\r\n";
    return ss.str();
}

std::string Replies::ERR_PASSWDMISMATCH(const std::string &clientNick) {
    std::stringstream ss;

    ss <<  ":" << clientNick << " 464 " << clientNick << " :Password incorrect\r\n";
    return ss.str();
}
