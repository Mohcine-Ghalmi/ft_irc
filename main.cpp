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
    
int main(int argc, char **argv) {
    Server _server(argc, argv);
    _server.start();
    
    return 0;
}
