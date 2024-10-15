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
#include "HeaderFiles/Server.hpp"
#include "HeaderFiles/Client.hpp"

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
    int  newClientSocket, maxFd, activity;
    fd_set readfds;
    Server _server(argc, argv);
    char buffer[BUFFER_SIZE];

    int clientSockets[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++)
        clientSockets[i] = 0;

    while (true) {
        FD_ZERO(&readfds);

        FD_SET(_server.getServerSocket(), &readfds);
        maxFd = _server.getServerSocket();

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

        if (FD_ISSET(_server.getServerSocket(), &readfds)) {
            if ((newClientSocket = accept(_server.getServerSocket(), NULL, NULL)) < 0) {
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

    // close(serverSocket);
    return 0;
}
