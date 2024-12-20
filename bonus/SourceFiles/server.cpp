#include "../HeaderFiles/Server.hpp"
#include <typeinfo>

bool Server::proccessCommandHelper(std::string cmd, std::string dif) {
    unsigned long pos = cmd.find(" ");
    if (pos == std::string::npos) return false;
    return cmd.substr(0, pos) == dif;
}

void Server::checkArgs(int argc, char **argv) {
    if (argc != 3) {
        LOG_ERROR(RED "Invalid number of arguments. Usage: ./server <port> <password>");
        exit(EXIT_FAILURE);
    }

    try {
        int portValue = std::stoi(argv[1]);
        if (portValue < 1024 || portValue > 65535) {
            LOG_ERROR(RED "Invalid port \"" << argv[1] << "\". Please specify a port number between 1024 and 65535.");
            exit(EXIT_FAILURE);
        }
        port = std::to_string(portValue);
    } catch (const std::invalid_argument&) {
        LOG_ERROR(RED "Invalid port \"" << argv[1] << "\" provided. Please specify a numeric port.");
        exit(EXIT_FAILURE);
    }

    password = argv[2];
    if (password.length() < 4) {
        LOG_ERROR(RED "Password is too short. Please provide a password with at least 4 characters.");
        exit(EXIT_FAILURE);
    }
}

Server::Server(int argc, char **argv) {
    checkArgs(argc, argv);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo((const char *)"localhost", port.c_str(), &hints, &res) != 0) {
        std::cerr << "getaddrinfo error" << std::endl;
        exit(1);
    }

    serverSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (serverSocket < 0) {
        std::cerr << "Socket error" << std::endl;
        exit(1);
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(serverSocket, res->ai_addr, res->ai_addrlen) < 0) {
        std::cerr << "Bind error" << std::endl;
        exit(1);
    }

    freeaddrinfo(res);

    if (listen(serverSocket, SOMAXCONN) < 0) {
        std::cerr << "Listen error" << std::endl;
        exit(1);
    }

    LOG_SERVER("Server started on port " << port);
    LOG_SERVER("Server started with the password " << "<" << password << ">");
}

Server::~Server(){
    close(serverSocket);
}

int Server::getServerSocket() {
    return (serverSocket);
}

void Server::acceptConnection() {
    int newClientSocket = accept(serverSocket, NULL, NULL);
    if (newClientSocket < 0) {
        std::cerr << "Accept error" << std::endl;
        return;
    }

    clients.push_back(Client(newClientSocket)); // Add client to the server's client list
}

std::vector<std::string> splitMessages(const std::string &buffer) {
    std::vector<std::string> lines;
    std::istringstream stream(buffer);
    std::string line;

    while (std::getline(stream, line))
        lines.push_back(line);

    return lines;
}

void removeCarriageReturn(std::string &str) {
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
}

void ping(std::string message, int ClientSocket) {
    std::string pingToken = message.substr(5);
    std::string pongResponse = "PONG " + pingToken + "\r\n";

    send(ClientSocket, pongResponse.c_str(), pongResponse.size(), 0);
    LOG_CLIENT(ClientSocket, message);
    LOG_SERVER(pongResponse);
}

Client* Server::getClientByNick(const std::string &targetNick) {
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
        if (it->getNickName() == targetNick)
            return &(*it);
    return NULL;
}

std::vector<std::string> splitByDelimiter(const std::string &input, const std::string &delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end;
    while ((end = input.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(input.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(input.substr(start));
    return tokens;
}

void Server::sendUnknownCommandReply(Client &client,const std::string &command) {
    std::string serverName = "localhost";
    std::string commandCarriage = command;
    removeCarriageReturn(commandCarriage);
    std::string reply = ":" + serverName + " 421 " + client.getNickName() + " " + commandCarriage + " Unknown command\r\n";
    send(client.getSocket(), reply.c_str(), reply.length(), 0);
}

void Server::handleClientMessage(Client &client, const std::string &message, int &flag) {
    client.appendToBuffer(message);
    if (hasNewline(client.getBuffer())) {
        LOG_CLIENT(client.getSocket(), client.getBuffer());
        if (client.getBuffer().find("CAP LS 302") != std::string::npos)
            sendCapResponse(client.getSocket());
        else if (client.getBuffer().find("PING") == 0)
            ping(client.getBuffer(), client.getSocket());
        else {
            if (setUpClient(client, flag)) {
                sendHellGate(client.getSocket());
                LOG_SERVER("Client setup completed successfully for " << client.getNickName());
            } else if (client.isAuthenticated()) {
                if (processPrivMsgCommand(client, client.getBuffer())) {
                    LOG_INFO("message sent");}
                else if (processJoinCommand(client, client.getBuffer())) {
                    LOG_INFO("Joining done");
                }
                else if (processModeCommand(client, client.getBuffer())) {
                    LOG_INFO("Mode Set");
                }
                else if (processINVITECommand(client, client.getBuffer())) {
                    LOG_INFO("Invite sent");
                } else if (processPartCommand(client, client.getBuffer())) {
                    LOG_INFO("Part sent");
                } else if (processKICKCommand(client, client.getBuffer())) {
                    LOG_INFO("Part sent");
                }
                else if (processTOPICcommand(client, client.getBuffer())) {
                    LOG_INFO("Topic set");
                }
            }
        }
        client.clearBuffer();
    }
}

bool hasCarriageReturn(const std::string &input) {
    return input.find('\r') != std::string::npos;
}

void Server::removeUserFromChannels(const std::string &nickName) {
    std::map<std::string, Channel>::iterator it = channels.begin();
    while (it != channels.end()) {
        Channel &channel = it->second;
        if (channel.isMember(getClientByNick(nickName)))
            if (processPartCommand(*getClientByNick(nickName), "PART " + channel.getName())) {
                it = channels.begin();
                continue;
            }
        // if (channel.getInvites().count(nickName)) {
        //     channel.removeInvitedUser(getClientByNick(nickName));
        //     LOG_INFO("Removed invite for " + nickName + " from channel " + channel.getName());
        // }
        ++it;
    }
}

void Server::processClienstMessage(fd_set readfds) {
    char buffer[BUFFER_SIZE];
    int flag = 0;

    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ) {
        memset(buffer, 0, BUFFER_SIZE);
        if (FD_ISSET(it->getSocket(), &readfds)) {
            ssize_t bytesReceived = recv(it->getSocket(), buffer, BUFFER_SIZE, 0);
            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {LOG_INFO(RED "Client disconnected ");}
                else
                {
                    LOG_ERROR("recv error");
                    // break;
                }
                removeUserFromChannels(it->getNickName());
                close(it->getSocket());
                clients.erase(it);
                it = clients.begin();
                continue;
            } else {
                std::string clientMessage(buffer);
                handleClientMessage(*it, clientMessage, flag);
                if ((it->getPassword() != password && !it->getPassword().empty())) {
                    close(it->getSocket());
                    it = clients.erase(it);
                    LOG_INFO(RED "client out");
                    continue;
                }
                ++it;
            }
        } else
            ++it;
    }
}

void Server::start() {
    int  maxFd;
    fd_set readfds;

    while (true) {
        FD_ZERO(&readfds);

        FD_SET(getServerSocket(), &readfds);
        maxFd = getServerSocket();

        for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
            if (it->getSocket() > 0)
                FD_SET(it->getSocket(), &readfds); // make the client socket a memeber of fd_set
            if (it->getSocket() > maxFd)
                maxFd = it->getSocket();
        }

        if (select(maxFd + 1, &readfds, NULL, NULL, NULL) < 0) {
                perror("select error");
                exit(EXIT_FAILURE);
        }

        if (FD_ISSET(getServerSocket(), &readfds))
            acceptConnection();
        processClienstMessage(readfds);
    }
}
