#pragma once
#include <string>
#include <vector>
#include "Client.hpp"
#include "Server.hpp"

class Channel
{
    private:
        std::string name;
        std::string topic;
        std::vector<Client&> clients;
        std::string modes;
        std::string key;
    public:
        Channel(/* args */);
        int add_client(Client* client);
        int remove_client(Client* client);
        int set_topic(std::string topic);
        int set_mode(std::string mode);
        bool has_mode(char mode_char);
        ~Channel();
};

// Channel::Channel(/* args */)
// {
// }

// Channel::~Channel()
// {
// }
