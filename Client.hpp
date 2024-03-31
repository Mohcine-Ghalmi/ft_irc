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
