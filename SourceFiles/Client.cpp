#include "../HeaderFiles/Client.hpp"

Client::Client(int socket) : clientSocket(socket),  authenticated(false), password("") {}
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
    case 462:
        message = replies.ERR_NEEDMOREPARAMS(client.getSocket());
        break;
    
    default:
        break;
    }
    send(this->getSocket(), message.c_str(), message.length(), 0);
}
