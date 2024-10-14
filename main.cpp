#include <iostream>
#include <string>
#include <cctype>
#include <climits>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <array>
#include <unistd.h>

typedef struct addrinfo info;

#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"

#define READ_BUFFER_SIZE 1024 * 32
#define BUFFER_SIZE 1024

#define LOG_INFO(custom_string)  \
    std::cout << GREEN << "INFO: " << RESET << custom_string<< std::endl; 

#define LOG_ERROR(custom_string)  \
    std::cout << RED << "ERROR: " << RESET << custom_string<< std::endl;

#define LOG_MSG(custom_string)  \
    std::cout << custom_string<< std::endl;

#define MAX_CLIENTS 10

bool isall_objsdigits(std::string str) 
{
    for (size_t i = 0; i < str.length(); i++)
        if (!std::isdigit((unsigned char)str[i]))
            return false;
    return true;
}

std::string checking_input(int argc, char **argv, std::string &pass) {
    std::string port = "6667";

    if (argc == 1) {
        LOG_MSG(BLUE "NOTE:" RESET "The programe will run with the default server");
    }
    else {
        if (argc > 3)
            LOG_MSG( BLUE "NOTE:" RESET  " We will only be accepting the first two arguments. " 
                      << "Any additional arguments will be ignored.")
        if (argc == 2) 
            LOG_MSG(BLUE "WARNING" RESET " The program is running without a password.");
        if (!isall_objsdigits(argv[1])) {LOG_ERROR("The programe will run with the default Port");}
        else {
            char *endptr;
            long int result = strtol(argv[1], &endptr, 10);

                if (*endptr != '\0' || result == LONG_MIN || result == LONG_MAX) {
                    LOG_MSG(BLUE "NOTE:" RESET "The programe will run with the default Port");
                }
                else {
                    port = argv[1];
                    pass = (argv[2] ? argv[2] : "");
                }
            }
    }

    return (port);
}

void sendCapResponse(int clientSocket) {
    std::string capResponse = ":localhost CAP * LS :\r\n"; // sending Empty CAP List (do not support any advanced features)

    send(clientSocket, capResponse.c_str(), capResponse.size(), 0);
    std::cout << "Sent CAP LS response to client" << std::endl;
}

void handleClientMessage(int clientSocket, const std::string &message) {
    if (message.find("CAP LS 302") != std::string::npos)
        sendCapResponse(clientSocket);
    else
        std::cout << "Received from client: " << message << std::endl;
}
    
int main(int argc, char **argv) {
    int serverSocket, newClientSocket, maxFd, activity;
    struct addrinfo *serverAddr;
    struct addrinfo serverInfo;
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    std::string pass;
    std::string port;

    port = checking_input(argc, argv, pass);

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    serverInfo.ai_family = AF_INET;
    serverInfo.ai_socktype = SOCK_STREAM;
    serverInfo.ai_protocol = IPPROTO_TCP;
    serverInfo.ai_flags = AI_PASSIVE;

    if (getaddrinfo((const char *)"localhost", port.c_str(), &serverInfo, &serverAddr) != 0) { 
        LOG_ERROR("getaddrinfo : " << strerror(errno));
        exit(1);
    }

    if (bind(serverSocket, serverAddr->ai_addr, serverAddr->ai_addrlen) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << port << std::endl;

    int clientSockets[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++)
        clientSockets[i] = 0;

    while (true) {
        FD_ZERO(&readfds);

        FD_SET(serverSocket, &readfds);
        maxFd = serverSocket;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clientSockets[i] > 0)
                FD_SET(clientSockets[i], &readfds);
            if (clientSockets[i] > maxFd)
                maxFd = clientSockets[i];
        }

        activity = select(maxFd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            continue;
        }

        if (FD_ISSET(serverSocket, &readfds)) {
            if ((newClientSocket = accept(serverSocket, NULL, NULL)) < 0) {
                perror("accept error");
                continue;
            }

            std::string welcomeMessage = ":localhost 001 irssi_user :Welcome to the simple IRC server\r\n";
            send(newClientSocket, welcomeMessage.c_str(), welcomeMessage.size(), 0);
            std::cout << "New client connected, welcome message sent" << std::endl;

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clientSockets[i] == 0) {
                    clientSockets[i] = newClientSocket;
                    std::cout << "Added new client to socket list at index " << i << std::endl;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int clientSocket = clientSockets[i];

            if (FD_ISSET(clientSocket, &readfds)) {
                memset(buffer, 0, BUFFER_SIZE);
                ssize_t bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);

                if (bytesReceived <= 0) {
                    if (bytesReceived == 0)
                        std::cout << "Client disconnected" << std::endl;
                    else
                        perror("recv error");
                    
                    close(clientSocket);
                    clientSockets[i] = 0;
                } else {
                    std::string clientMessage(buffer);
                    handleClientMessage(clientSocket, clientMessage);
                }
            }
        }
    }

    close(serverSocket);
    return 0;
}
