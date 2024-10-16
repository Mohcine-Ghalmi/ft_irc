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

void *shutdownListener(void *serverInstance); 

int main(int argc, char **argv) {
    Server _server(argc, argv);  // Initialize server object
    _server.start();             // Start the server

    return 0;
}