#include <Server.hpp>


Server::Server(int argc, char **argv)
{
    if (argc == 1) {
        std::cerr << "Run default server" << std::endl;
    }
    else {
        if (argc != 3 ) {
            std::cerr << "args error!" << std::endl;
            exit (1);
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
        this->password = argv[2];
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
    std::pair<bool, std::string> result = this->clients[clientfd]->setBuffer(tmp); // check if I recieved the \r\n : 
    if (result.first) // message is finished!
    {
        // handel message by client -> 
        std::vector<std::string> CommandParsed = tools.CommandParser(result.second);
        for (size_t i = 0; i < CommandParsed[0].size(); ++i) {
            CommandParsed[0][i] = tolower(CommandParsed[0][i]); 
        }
        if ((CommandParsed[0] != "nick" && CommandParsed[0] != "pass" && CommandParsed[0] != "user") && (!this->clients[clientfd]->getAuthStatus() || !this->clients[clientfd]->getNickName().empty()
                || !this->clients[clientfd]->getNickName().empty())) {
                LOG_INFO("HE HAS NO AUTH");
                return 1;
        }
        if (CommandParsed[0] == "join")
        {
            // parse join args :

            // check channel exists => if not create it =>
                    /*
                        - Channel name rules:
                            Channels names are strings (beginning with a '&' or '#' character) of
                            length up to 200 characters.  Apart from the the requirement that the
                            first character being either '&' or '#'; the only restriction on a
                            channel name is that it may not contain any spaces (' '), a control G
                            (^G or ASCII 7), or a comma (',' which is used as a list item
                            separator by the protocol).
                        - when creating the channel : the channel is created and the creating user becomes a
                            channel operator.
                    */
            // if the channel exists :
            /*
               whether or not your
                request to JOIN that channel is honoured depends on the current modes
                of the channel. For example, if the channel is invite-only, (+i),
                then you may only join if invited.  As part of the protocol, a user
                may be a part of several channels at once, but a limit of ten (10)
                channels is recommended as being ample for both experienced and
                novice users.
            */
             
            // this->channels
        }
        if (CommandParsed[0] == "nick")
        {
            if (!this->clients[clientfd]->getAuthStatus())
            {
                LOG_ERROR("YOU NEED A PASS FIRST");
                return 1;
            }
            //   nickname   =  ( letter / special ) *8( letter / digit / special / "-" ) => total 9 chars
        }
        else if (CommandParsed[0] == "pass")
        {

        }
        else if (CommandParsed[0] == "user")
        {
            if (!this->clients[clientfd]->getAuthStatus())
            {
                LOG_ERROR("YOU NEED A PASS FIRST");
                return 1;
            }
        }
        else if (CommandParsed[0] == "mod")
        {
            //    Parameters: <channel> {[+|-]|o|p|s|i|t|n|b|v} [<limit>] [<user>]
            //                [<ban mask>]

            //    The MODE command is provided so that channel operators may change the
            //    characteristics of `their' channel.  It is also required that servers
            //    be able to change channel modes so that channel operators may be
            //    created.

            //    The various modes available for channels are as follows:

            //            o - give/take channel operator privileges;
            //            p - private channel flag;
            //            s - secret channel flag;
            //            i - invite-only channel flag;
            //            t - topic settable by channel operator only flag;
            //            n - no messages to channel from clients on the outside;
            //            m - moderated channel;
            //            l - set the user limit to channel;
                //         b - set a ban mask to keep users out;
                //         v - give/take the ability to speak on a moderated channel;
                //         k - set a channel key (password).

                //         When using the 'o' and 'b' options, a restriction on a total of three
                //         per mode command has been imposed.  That is, any combination of 'o'

        } else if (CommandParsed[0] == "topic") 
        {
            // Parameters: <channel> [<topic>]

            // The TOPIC message is used to change or view the topic of a channel.
            // The topic for channel <channel> is returned if there is no <topic>
            // given.  If the <topic> parameter is present, the topic for that
            // channel will be changed, if the channel modes permit this action.
        }
        // send message by client..
    }
}

int Server::broadcast_message(std::string message, Channel* channel)
{

}
