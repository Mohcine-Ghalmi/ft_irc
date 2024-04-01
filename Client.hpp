#pragma once
#include <string>

class Client
{
    private:
        int socket_fd;
        std::string nickname;
        std::string username;
        std::string realname;
        bool is_operator;
        std::string buffer;
    public:
        Client();
        std::string getNickName();
        std::string getUserName();
        std::string getRealName();
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
std::string Client::getRealName()
{
    return this->realname;
}