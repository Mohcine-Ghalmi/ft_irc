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

bool isall_objsdigits(std::string str) 
{
    for (size_t i = 0; i < str.length(); i++)
        if (!std::isdigit((unsigned char)str[i]))
            return false;
    return true;
}

std::string checking_input(int argc, char **argv, std::string &pass) {
    std::string port = "8080";

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

int connectClient(int serverSocket) {
    int newClientSocket = accept(serverSocket, NULL, NULL);

    if (newClientSocket < 0) {
        LOG_ERROR("accept : " << strerror(errno));
        return (newClientSocket);
    }

    std::string welcomeMessage = ":localhost 001 irssi_user :Welcome to the simple IRC server\r\n";
    send(newClientSocket, welcomeMessage.c_str(), welcomeMessage.size(), 0);

    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        ssize_t bytesReceived = recv(newClientSocket, buffer, BUFFER_SIZE, 0);

        if (bytesReceived <= 0) {
            if (bytesReceived == 0) {LOG_INFO("Client disconnected.");}
            else {LOG_ERROR("Error receiving message from client.");}
            close(newClientSocket);
            break;
        }

        std::string clientMessage(buffer);

        if (clientMessage.find("CAP LS 302") != std::string::npos) { // client asking for you're server Capabilities
            std::string capResponse = ":localhost CAP * LS :\r\n"; // sending Empty CAP List (do not support any advanced features)

            send(newClientSocket, capResponse.c_str(), capResponse.size(), 0);
            std::cout << "Sent CAP LS response to client" << std::endl;
        } else 
            std::cout << "Received from client: " << clientMessage << std::endl;

    }
    return (newClientSocket);
}

int main (int argc, char **argv) {
    // params parsing
    std::string pass;
    std::string port = checking_input(argc, argv, pass);
    LOG_INFO("Port is " << port);
    LOG_INFO("Password is " << "<" << (pass != "" ? pass : "No Password Used") << ">");
    

    // server setuping
    info* addr;
    info hints;

    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    
    if (getaddrinfo((const char *)"localhost", port.c_str(), &hints, &addr) != 0) { 
        LOG_ERROR("getaddrinfo : " << strerror(errno));
        exit(1);
    }

    int serverSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    int enable = 1;

    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) ==
        -1) {
            LOG_ERROR("setsockopt : " << strerror(errno));
        exit(1);
    }
    if (bind(serverSocket, addr->ai_addr, addr->ai_addrlen) == -1) {
        LOG_ERROR("bind : " << strerror(errno));
        exit(1);
    }

    freeaddrinfo(addr); 
    
    if (listen(serverSocket, 128) == -1) {
        LOG_ERROR("listen : " << strerror(errno));
        exit(1);
    }
    
    // event loop    
    
    // fd_set all_objs;
    // fd_set read_objs;
    // fd_set write_objs;
    // fd_set expect_objs;

    // FD_ZERO(&all_objs);
    // FD_ZERO(&read_objs);
    // FD_ZERO(&expect_objs);
    // FD_ZERO(&write_objs);

    // int fds_higher = fd;

    // FD_SET(fd, &all_objs);

    // int select_return;
    // int handled_events_nb;
    // while (01) {
    //     handled_events_nb = 0;
    //     read_objs = all_objs;
    //     timeval timeout;
    //     timeout.tv_sec = 1;  // Example: 1-second timeout
    //     timeout.tv_usec = 0; 
    //     FD_ZERO(&expect_objs);
    //     FD_ZERO(&write_objs);
    //     write_objs = all_objs;
    //     expect_objs = all_objs;
    //     select_return = select(fds_higher + 1, &read_objs, &write_objs, &expect_objs, &timeout);
    //     if (select_return < 0)
    //     {
    //         std::cerr << "well selected failed asidi" << std::endl;
    //         exit (1);
    //     }
    //     else if (select_return != 0) {
    //         for (int i = fd; i <= fds_higher; i++)
    //         {
    //             if (FD_ISSET(i, &read_objs))
    //             {
    //                 LOG_INFO("- This file discreptor is ready to read => -" << i);
    //                 handled_events_nb++;
    //                 // accepting a client who joined us
    //                 if (i == fd) {
    //                     LOG_INFO("- Server with reading request -");
                            // int newClientSocket = accept(serverSocket, NULL, NULL);
                            // if (newClientSocket < 0)
                            // {
                            //     LOG_INFO("- This client is out! -");
                            //     // continue;
                            // }
    //                     FD_SET(new_client_fd, &all_objs);
    //                     if (new_client_fd > fds_higher)
    //                         fds_higher = new_client_fd;
    //                     continue;
    //                 }
                    // std::array<char, READ_BUFFER_SIZE> buffer;
                    // ssize_t ret = recv(i, buffer.data(), buffer.size(), 0);
    //                 if (ret <= 0){
    //                         if (ret == 0) { 
    //                     //     LOG_INFO("Client closed connection");  // Graceful close
    //                     // } else {
    //                     //     LOG_ERROR("Error occurred on socket: " << strerror(errno)); 
    //                     // }
    //                     LOG_ERROR("To remove client");
    //                     FD_CLR(i, &all_objs);
    //                     close(i);
    //                 }
    //                 std::string tmp;
    //                 tmp.reserve(ret);
    //                 for (ssize_t s = 0; s < ret; s++) {
    //                     tmp.push_back(buffer.data()[s]);
    //                 }
    //                 LOG_INFO("This content is: " << tmp << "the size is: " << ret);
    //             }
    //             else if (FD_ISSET(i, &write_objs))
    //             {
    //                 handled_events_nb++;
    //                 if (i == fd) {
    //                     LOG_INFO("- Server with writing request ? comment -");
    //                 }
    //                 LOG_INFO("This file discreptor is ready to write => " << i);
    //             }
    //             else if (FD_ISSET(i, &expect_objs))
    //             {
    //                 if (i == fd) {
    //                     LOG_INFO("server with exeption! - we closed today");
    //                     exit (1);
    //                 }
    //                 handled_events_nb++;
    //                 LOG_ERROR("This file discreptor has exeption => " << i);
    //                 FD_CLR(i, &all_objs);
    //                 close(i);
    //             }
    //             if (handled_events_nb == select_return)
    //                 break;
    //         }
    //     }
    // }

    return (0);
}