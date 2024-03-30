#include <iostream>
#include <string>
#include <cctype>
#include <climits>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
typedef struct addrinfo infos;

#define LOG_INFO(custom_string)  \
    std::cout << "INFO: " << custom_string << std::endl; 

#define LOG_ERROR(custom_string)  \
    std::cout << "ERROR: " << custom_string << std::endl;

// #define LOG_wa(custom_string)  \
//     std::cout << "INFO: " << custom_string << std::endl; 

bool isall_objsdigits(std::string str) 
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
        std::cerr << "Run default server" << std::endl;
    }
    else {
        if (argc != 2 ) {
            std::cerr << "avoiding the args and taking only the first!" << std::endl;
        }
        if (!isall_objsdigits(argv[1]))
            std::cerr << "error! runing with the default port" << std::endl;
        else {
            char *endptr;
            long int result = strtol(argv[1], &endptr, 10);
                if (*endptr != '\0' || result == LONG_MIN || result == LONG_MAX)
                    std::cerr << "Run default server" << std::endl;
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

    fd_set all_objs;
    fd_set read_objs;
    fd_set write_objs;
    fd_set expect_objs;

        //    void FD_CLR(int fd, fd_set *set);
    //    int  FD_ISSET(int fd, fd_set *set);
    //    void FD_SET(int fd, fd_set *set);
    //    void FD_ZERO(fd_set *set);
    FD_ZERO(&all_objs);
    FD_ZERO(&read_objs);
    FD_ZERO(&expect_objs);
    FD_ZERO(&write_objs);

    int fds_higher = fd;

    FD_SET(fd, &all_objs);

int select_return;
int handled_events_nb;
    while (01) {
        handled_events_nb = 0;
        read_objs = all_objs;
        write_objs = all_objs;
        expect_objs = all_objs;
        select_return = select(fds_higher + 1, &read_objs, &write_objs, &expect_objs, NULL);
        if (select_return < 0)
        {
            std::cerr << "well selected failed asidi" << std::endl;
            exit (1);
        }
        else {
            for (int i = fd; i <= fds_higher; i++)
            {
                if (FD_ISSET(i, &read_objs))
                {
                    handled_events_nb++;
                    if (i == fd) {
                        LOG_INFO("server with reading request");
                    }
                    LOG_INFO("this file discreptor is ready to read => " << i);
                }
                else if (FD_ISSET(i, &write_objs))
                {
                    handled_events_nb++;
                    if (i == fd) {
                        LOG_INFO("server with writing request ? comment");
                    }
                    LOG_INFO("this file discreptor is write to read => " << i);
                }
                else if (FD_ISSET(i, &expect_objs))
                {
                    if (i == fd) {
                        LOG_INFO("server with exeption! - we closed today");
                        exit (1);
                    }
                    handled_events_nb++;
                    LOG_ERROR("this file discreptor has exeption => " << i);
                }
                if (handled_events_nb == select_return)
                    break;
            }
        }
    }
}