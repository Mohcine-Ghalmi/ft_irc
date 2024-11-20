#include "../HeaderFiles/Client.hpp"
#include <sstream>

Client::Client(int socket) : clientSocket(socket),  authenticated(false), password("") {}

Client::Client(const Client &client) : clientSocket(client.clientSocket), nickName(client.nickName), userName(client.userName), hostName(client.hostName), realName(client.realName), authenticated(client.authenticated), buffer(client.buffer), password(client.password), replies(client.replies) { }

Client::~Client() { /**close(clientSocket);**/}
void Client::clearBuffer() { buffer.clear();}

std::string Client::getNickName() { return nickName; }
std::string Client::getUserName() { return userName; }
std::string Client::getBuffer() { return buffer;}
int Client::getSocket() { return clientSocket;}
std::string Client::getPassword() { return password; }
std::string Client::getHostname() {return hostName; }
bool Client::isAuthenticated() { return authenticated;}


void Client::setPassword(std::string &password) { this->password = password; }
void Client::setNickName(const std::string &nick) { nickName = nick;}
void Client::setUserName(const std::string &user) { userName = user; }
void Client::setHostName(const std::string &host) { hostName = host; }
void Client::setRealName(const std::string &real) { realName = real; }
void Client::appendToBuffer(const std::string &data) { buffer += data;}
void Client::authenticate() { authenticated = true;}


void Client::sendReply(int replyNumber, Client &client) {
    std::string message;

    switch (replyNumber) {
        case 001:
            message = replies.RPL_WELCOME(client.getNickName(), client.getUserName());
            break;
        case 002:
            message = replies.RPL_YOURHOST(client.getHostname(), client.getNickName());
            break;
        case 003:
            message = replies.RPL_CREATED(client.getHostname(), client.getNickName());
        case 004:
            message = replies.RPL_MYINFO(client.getHostname(), client.getNickName());
            break;
        case 005:
            message = replies.RPL_ISUPPORT(client.getHostname());
            break;
        case 464:
            message = replies.ERR_PASSWDMISMATCH(client.getNickName());
            break;
        default:
            break;
    }
    send(this->getSocket(), message.c_str(), message.length(), 0);
}

void Client::ERR_NICKNAMEINUSE(Client &client, const std::string &newNick) {
    std::stringstream ss;

    ss << ":" << client.getHostname() << " 433 " << client.getNickName() << " " << newNick <<" :Nickname is already in use\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::ERR_NOSUCHNICK(Client &client,  const std::string &targetNick) {
    std::stringstream ss;

    ss << ":" << client.getNickName() << " 401 " << client.getNickName()  << " " << targetNick << " :No such nick/channel\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::ERR_NOSUCHNICKINCHANNEL(Client &client, const std::string &targetNick, const std::string &channelName) {
    std::stringstream ss;

    ss << ":" << " NOTICE " << channelName
       << " :" << targetNick << " is not in this channel.\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}


void Client::ERR_ERRONEUSNICKNAME(Client &client, const std::string &invalidNick) {
    std::stringstream ss;

    ss <<  ":" << client.getHostname() << " 432 " << client.getNickName() << " " << invalidNick << " :Erroneous nickname\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::ERR_NEEDMOREPARAMS(Client &client, std::string cmd) {
    std::stringstream ss;

    ss <<  ":" << client.getHostname() << " 461 " << client.getNickName() << " " << cmd << " :Not enough parameters\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::ERR_NOSUCHCHANNEL(Client &client, const std::string &channelName) {
    std::stringstream ss;

    ss << ":" << client.getHostname()
       << " 403 " << client.getNickName()
       << " " << channelName
       << " :No such channel\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::RPL_INVITE(Client &client, const std::string &invitedUser, const std::string &channelName) {
    std::stringstream ss;

    ss << ":" << client.getHostname()
       << " INVITE " << invitedUser
       << " :" << channelName << "\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}



void Client::ERR_INVITEONLYCHAN(Client &client, const std::string &channel) {
    std::stringstream ss;

    ss << ":" << client.getHostname()
       << " 473 " << client.getNickName()
       << " " << channel
       << " :Cannot join channel (+i)\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}


void Client::ERR_USERONCHANNEL(Client &client, const std::string &nick, const std::string &channelName)
{
    std::stringstream ss;

    ss << ":" << client.getHostname()
        << " 443 " << channelName
        << " " << nick
        << " :is already on channel\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}


void Client::ERR_CHANOPRIVSNEEDED(Client &client, const std::string &channelName) {
    std::stringstream ss;

    ss << ":" << client.getHostname()
        << " 443 " << channelName
        << " :You're not channel operator\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::RPL_KICKED(Client &client, const std::string &channelName, const std::string &operatorName) {
    std::stringstream ss;
    std::stringstream ss2;

    ss << ":" << client.getHostname()
        << " KICK " << channelName
        << " :" << client.getNickName() << "\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
    ss2 << "You were kicked from " << channelName
        << " by " << operatorName << "\r\n";
    send(client.getSocket(), ss2.str().c_str(), ss2.str().length(), 0);
}

void Client::RPL_INVITESENTTO(Client &client, const std::string &channelName, std::string &userInvited) {
    std::stringstream ss;

    ss << ":" << client.getHostname()
        << " 701 " << userInvited
        << " :You Where Invited To "
        << channelName << "\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

// void Client::MODE_NOTIFY(const std::string &channelName, const std::string &modeChange, const std::string &target, const std::vector<Client *> &channelClients) {
//     std::stringstream ss;

//     ss << ":" << getNickname()
//        << " MODE " << channelName
//        << " " << modeChange;

//     if (!target.empty()) {
//         ss << " " << target;
//     }
//     ss << "\r\n";

//     // Send the message to all clients in the channel
//     for (std::vector<Client *>::const_iterator it = channelClients.begin(); it != channelClients.end(); ++it) {
//         send((*it)->getSocket(), ss.str().c_str(), ss.str().length(), 0);
//     }
// }
