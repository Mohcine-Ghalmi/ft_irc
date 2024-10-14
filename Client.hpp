#pragma once
#include <string>

class Client
{
    private:
        int socket_fd;
        std::string nickname;
        std::string username;
        bool is_auth;
        bool is_operator;
        std::string buffer;
    public:
        Client();
        std::string getNickName();
        std::string getUserName();
        bool getAuthStatus();
        int send_message(std::string message);
        int join_channel(std::string channel_name);
        int leave_channel(std::string channel_name);
        int set_nickname(std::string nickname);
        std::pair<bool, std::string> setBuffer(std::string tmp);
        ~Client();
};

Client::Client(/* args */)
{
}

Client::~Client()
{
}


std::string Client::getNickName()
{
    return this->nickname;
}
std::string Client::getUserName()
{
    return this->username;
}

bool Client::getAuthStatus()
{
    return this->is_auth;
}
