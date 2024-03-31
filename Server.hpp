#pragma once
#include <string>
#include <vector>
#include <map>
#include <Channel.hpp>
#include <Helper.hpp>

class Server
{
private:
    std::string  port = "8080";
    int socket_fd;
    std::string password;
    // std::vector<Client&> clients;
    std::map<int, Client*> clients;
    std::map<std::string, Channel*> channels;
    Helper tools;
    int fds_higher;
    fd_set  all_objs;
    /* data */
public:
    Server(int argc, char **argv);
    // start()
    Client *accept_connections();
    int handle_io();
    int process_client_message(int clientfd);
    int broadcast_message(std::string message, Channel* channel);
    ~Server();
};


