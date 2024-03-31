#include <Server.hpp>


Server::Server(int argc, char **argv)
{
    if (argc == 1) {
        std::cerr << "Run default server" << std::endl;
    }
    else {
        if (argc != 2 ) {
            std::cerr << "avoiding the args and taking only the first!" << std::endl;
        }
        if (!tools.isall_objsdigits(argv[1]))
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
    info* addr;
    info hints;

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
}

Server::~Server()
{
}

Client *Server::accept_connections()
{
    LOG_INFO("- Server with reading request -");
    int new_client_fd = accept(this->socket_fd, NULL, NULL);
    if (new_client_fd < 0)
    {
        LOG_INFO("- This client is out! -");
        return NULL;
    }
    FD_SET(new_client_fd, &(this->all_objs));
    if (new_client_fd > this->fds_higher)
        this->fds_higher = new_client_fd;
    Client *newClient = new Client();

    clients[new_client_fd] = newClient;

    return newClient;
}

int Server::handle_io()
{
    fd_set read_objs;
    fd_set write_objs;
    fd_set expect_objs;

    FD_ZERO(&(this->all_objs));
    FD_ZERO(&read_objs);
    FD_ZERO(&write_objs);
    FD_ZERO(&expect_objs);

    this->fds_higher = this->socket_fd;

    FD_SET(this->socket_fd, &(this->all_objs));

    int select_return;
    int handled_events_nb;
    while (01) {
        handled_events_nb = 0;
        read_objs = this->all_objs;
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0; 
        FD_ZERO(&expect_objs);
        FD_ZERO(&write_objs);
        // write_objs = this->all_objs;
        expect_objs = this->all_objs;
        select_return = select(this->fds_higher + 1, &read_objs, &write_objs, &expect_objs, &timeout);
        if (select_return < 0)
        {
            std::cerr << "well selected failed asidi" << std::endl;
            exit (1);
        }
        else if (select_return != 0) {
            for (int i = this->socket_fd; i <= this->fds_higher; i++)
            {
                if (FD_ISSET(i, &read_objs))
                {
                    LOG_INFO("- This file discreptor is ready to read => -" << i);
                    handled_events_nb++;
                    // accepting a client who joined us
                    if (i == this->socket_fd) {
                        this->accept_connections();
                        continue;
                    }
                    process_client_message(i);
                }
                else if (FD_ISSET(i, &write_objs))
                {
                    handled_events_nb++;
                    if (i == this->socket_fd) {
                        LOG_INFO("- Server with writing request ? comment -");
                    }
                    LOG_INFO("This file discreptor is ready to write => " << i);
                }
                else if (FD_ISSET(i, &expect_objs))
                {
                    if (i == this->socket_fd) {
                        LOG_INFO("server with exeption! - we closed today");
                        exit (1);
                    }
                    handled_events_nb++;
                    LOG_ERROR("This file discreptor has exeption => " << i);
                    FD_CLR(i, &(this->all_objs));
                    close(i);
                }
                if (handled_events_nb == select_return)
                    break;
            }
        }
    }
}

int Server::process_client_message(int clientfd)
{
    std::array<char, READ_BUFFER_SIZE> buffer;
    ssize_t ret = recv(clientfd, buffer.data(), buffer.size(), 0);
    if (ret <= 0){
        //                         if (ret == 0) { 
        //     LOG_INFO("Client closed connection");  // Graceful close
        // } else {
        //     LOG_ERROR("Error occurred on socket: " << strerror(errno)); 
        // }
        LOG_ERROR("To remove client");
        FD_CLR(clientfd, &(this->all_objs));
        close(clientfd);
    }
    std::string tmp;
    tmp.reserve(ret);
    for (ssize_t s = 0; s < ret; s++) {
    tmp.push_back(buffer.data()[s]);
    }
    LOG_INFO("This content is: " << tmp << "the size is: " << ret);
    std::pair<bool, std::string> result = this->clients[clientfd]->setBuffer(tmp);
    if (result.first) // message is finished!
    {
        // handel message by client ->
        for (size_t i = 0; i < result.second.size(); ++i) {
            result.second[i] = tolower(result.second[i]); 
        }
        std::vector<std::string> CommandParsed = tools.CommandParser(result.second);
        if (CommandParsed[0] == "join")
        {
            // join channel
        }
        else if (CommandParsed[0] == "nick")
        {

        }
        else if (CommandParsed[0] == "pass")
        {

        }
        else if (CommandParsed[0] == "user")
        {

        }
        else if (CommandParsed[0] == "join")
        {

        }
        else if (CommandParsed[0] == "join")
        {

        }
        // send message by client..
    }
}

int Server::broadcast_message(std::string message, Channel* channel)
{

}
