#include "../HeaderFiles/Client.hpp"

Client::Client(int socket) : clientSocket(socket), authenticated(false) {}

std::string Client::getNickName() {
    return nickname;
}

std::string Client::getUserName() {
    return username;
}

void Client::setNickName(const std::string &nick) {
    nickname = nick;
}

void Client::setUserName(const std::string &user) {
    username = user;
}

void Client::appendToBuffer(const std::string &data) {
    buffer += data;
}

std::string Client::getBuffer() {
    return buffer;
}

bool Client::isAuthenticated() {
    return authenticated;
}

void Client::authenticate() {
    authenticated = true;
}

int Client::getSocket() {
    return clientSocket;
}

Client::~Client() {
    // close(clientSocket);
}
