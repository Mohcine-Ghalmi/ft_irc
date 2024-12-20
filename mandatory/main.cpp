#include "HeaderFiles/Server.hpp"
#include "HeaderFiles/Client.hpp"

int main(int argc, char **argv) {
    Server _server(argc, argv);  // Initialize server object
    _server.start();            // Start the server

    return 0;
}
