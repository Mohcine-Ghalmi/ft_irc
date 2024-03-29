#include <iostream>
#include <string>
#include <cctype>
#include <climits>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
typedef struct addrinfo infos;

bool isalldigits(std::string str) 
{
    for (int i = 0; i < str.length(); i++)
        if (!std::isdigit((unsigned char)str[i]))
            return false;
    return true;
}

int main (int argc, char **argv)
{
    // params parsing
    std::string port = "8080";

    if (argc == 1) {
        std::cout << "Run default server" << std::endl;
    }
    else {
        if (argc != 2 ) {
            std::cout << "avoiding the args and taking only the first!" << std::endl;
        }
        if (!isalldigits(argv[1]))
            std::cout << "error! runing with the default port" << std::endl;
        else {
            char *endptr;
            long int result = strtol(argv[1], &endptr, 10);
                if (*endptr != '\0' || result == LONG_MIN || result == LONG_MAX)
                    std::cout << "Run default server" << std::endl;
                else 
                    port = argv[1];
            }
    }

    // server setuping
    int backlog = 128;
    infos* addr;
    infos hints;

    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    int ireturn = ::getaddrinfo((const char *)"localhost", port.c_str(), &hints, &addr);
    if (ireturn != 0) {
        std::cerr << "getaddrinfo : " << strerror(errno) << '\n';
        exit(1);
    }
    int fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    int enable = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) ==
        -1) {
        std::cerr << "setsockopt : " << strerror(errno) << '\n';
        exit(1);
    }
    if (bind(fd, addr->ai_addr, addr->ai_addrlen) == -1) {
        std::cerr << "bind : " << strerror(errno) << '\n';
        exit(1);
    }
    if (listen(fd, backlog) == -1) {
        std::cerr << "listen : " << strerror(errno) << '\n';
        exit(1);
    }
    freeaddrinfo(addr); 

    // event loop

    while (01) {
        // select()
        // pool()
        // kqueue()
    }   
}