#include "../HeaderFiles/Server.hpp"
#include <typeinfo>

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

void Server::checkArgs(int argc, char **argv) {
    port = "6667";

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
                    password = (argv[2] ? argv[2] : "");
                }
            }
    }
}

Server::Server(int argc, char **argv) {
    checkArgs(argc, argv);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port.c_str(), &hints, &res) != 0) {
        std::cerr << "getaddrinfo error" << std::endl;
        exit(1);
    }

    serverSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (serverSocket < 0) {
        std::cerr << "Socket error" << std::endl;
        exit(1);
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(serverSocket, res->ai_addr, res->ai_addrlen) < 0) {
        std::cerr << "Bind error" << std::endl;
        exit(1);
    }

    freeaddrinfo(res);

    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Listen error" << std::endl;
        exit(1);
    }

    std::cout << "Server started on port " << port << std::endl;
    std::cout << "Server started with the password " << "<" << password << ">" << std::endl;
}

Server::~Server(){
    close(serverSocket); 
}

int Server::getServerSocket() {
    return (serverSocket);
}

void sendHellGate(int client_socket, std::string name) {
    std::string hellGate = 
        ":localhost 001 " + name + "\n"
        "🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥\r\n"
        "    🔥      "RED"    Welcome to       🔥\n"
        "    🔥      "RED"      Hell!          🔥\n"
        "    🔥                           🔥\n"
        "    🔥   👿 "RED" Beware of the       🔥\n"
        "    🔥   "RED"Darkness and Flames!    🔥\n"
        "    🔥                           🔥\n"
        "🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥"RESET"\r\n";

    // Send the hellGate message to the client
    send(client_socket, hellGate.c_str(), hellGate.length(), 0);
}

void Server::acceptConnection() {
    int newClientSocket = accept(serverSocket, NULL, NULL);
    if (newClientSocket < 0) {
        std::cerr << "Accept error" << std::endl;
        return;
    }

    clients.push_back(Client(newClientSocket));  // Add client to the server's client list
    sendHellGate(newClientSocket, "mohcine");
}

bool Server::setUpClient(std::string message, Client &client) {
    // if (!client.isAuthenticated() && !password.empty()) {
    //     std::string passRequest = ":localhost NOTICE * :Please provide your password using the PASS command.\r\n";
    //     send(client.getSocket(), passRequest.c_str(), passRequest.size(), 0);

    //     if (message.find("PASS") != std::string::npos) {
    //         std::string pass = message.substr(5);  // Extract password
    //         if (validatePassword(pass, password)) {
    //             client.authenticate();
    //             return (true);
    //         } else {
    //             std::string incorrectPassMsg = ":localhost NOTICE * :Incorrect password. Try again.\r\n";
    //             send(client.getSocket(), incorrectPassMsg.c_str(), incorrectPassMsg.size(), 0);
    //         }
    //     }
    // }

    // // If PASS succeeded, proceed to request NICK

    if (!client.isAuthenticated()) {
        std::string nickRequest = ":localhost NOTICE * :Please provide your nickname using the NICK command.\r\n";
        send(client.getSocket(), nickRequest.c_str(), nickRequest.size(), 0);

        if (message.find("NICK") != std::string::npos) {
            std::string nick = message.substr(5);  // Extract nickname
            std::cout << nick << std::endl;
            client.setNickName(nick);
            client.authenticate();
            std::string nickName = ":localhost NOTICE * :your nickname is settled\r\n";
            send(client.getSocket(), nickName.c_str(), nickName.size(), 0);
            return (true);
        } else {
            std::string incorrectNickMsg = ":localhost NOTICE * :Please provide a valid NICK command.\r\n";
            send(client.getSocket(), incorrectNickMsg.c_str(), incorrectNickMsg.size(), 0);
        }
    }
    return (false);
    // // If NICK succeeded, proceed to request USER
    // retry = 3;
    // authenticated = false;

    // while (retry > 0 && !authenticated) {
    //     std::string userRequest = ":localhost NOTICE * :Please provide your user information using the USER command.\r\n";
    //     send(client.getSocket(), userRequest.c_str(), userRequest.size(), 0);

    //     if (message.find("USER") != std::string::npos) {
    //         std::string user = message.substr(5);  // Extract user information
    //         client.setUserName(user);
    //         authenticated = true;
    //     } else {
    //         std::string incorrectUserMsg = ":localhost NOTICE * :Please provide a valid USER command.\r\n";
    //         send(client.getSocket(), incorrectUserMsg.c_str(), incorrectUserMsg.size(), 0);
    //         retry--;
    //     }
    // }

    // if (!authenticated) {
    //     std::string disconnectMessage = ":localhost NOTICE * :Too many incorrect attempts. Disconnecting.\r\n";
    //     send(client.getSocket(), disconnectMessage.c_str(), disconnectMessage.size(), 0);
    //     close(client.getSocket());  // Disconnect after failed attempts
    //     return;
    // }

    // // If everything is correct, send a welcome message
    // std::string welcomeMessage = ":localhost 001 " + client.getNickName() + " :Welcome to the IRC server\r\n";
    // send(client.getSocket(), welcomeMessage.c_str(), welcomeMessage.size(), 0);

    std::cout << "Client setup completed successfully for " << client.getNickName() << std::endl;
}


void sendCapResponse(int clientSocket) {
    std::string capResponse = ":localhost CAP * LS :\r\n"; // sending Empty CAP List (do not support any advanced features)

    send(clientSocket, capResponse.c_str(), capResponse.size(), 0);
    std::cout << "Sent CAP LS response to client" << std::endl;
}

void Server::handleClientMessage(Client &client, const std::string &message) {
    if (message.find("CAP LS 302") != std::string::npos)
        sendCapResponse(client.getSocket());
    else if (message.find("PING") == 0) {
        std::string pingToken = message.substr(5);
        std::string pongResponse = "PONG " + pingToken + "\r\n";

        // Send PONG response back to the client (to stay connectes without reseting the connection // the irssi client wil restart if no pong send recived)
        send(client.getSocket(), pongResponse.c_str(), pongResponse.size(), 0);
        std::cout << message;
        std::cout << pongResponse;
    } else {
        std::cout << "Received from client: " << message << std::endl;
        if (setUpClient(message, client)) {
            std::cout << "Client setup completed successfully for " << client.getNickName() << std::endl;
        }
    }
}

void Server::processClienstMessage(fd_set readfds) {
    char buffer[BUFFER_SIZE];

    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ) {
        if (FD_ISSET(it->getSocket(), &readfds)) {
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t bytesReceived = recv(it->getSocket(), buffer, BUFFER_SIZE, 0);

            if (bytesReceived <= 0) {
                if (bytesReceived == 0)
                    std::cout << "Client disconnected" << std::endl;
                else
                    perror("recv error");
                close(it->getSocket());
                it = clients.erase(it);
            } else {
                std::string clientMessage(buffer);
                handleClientMessage(*it, clientMessage);
                ++it;
            }
        } else
            ++it;
    }
}

void Server::start() {
    int  maxFd;
    fd_set readfds;

    while (true) {
        FD_ZERO(&readfds);

        FD_SET(getServerSocket(), &readfds);
        maxFd = getServerSocket();

        for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
            if (it->getSocket() > 0)
                FD_SET(it->getSocket(), &readfds); // make the client socket a memeber of fd_set 
            if (it->getSocket() > maxFd)
                maxFd = it->getSocket();
        }

        if (select(maxFd + 1, &readfds, NULL, NULL, NULL) < 0) {
                perror("select error");
                exit(EXIT_FAILURE);
        }

        if (FD_ISSET(getServerSocket(), &readfds))
            acceptConnection(); // create clients
        processClienstMessage(readfds);
    }
}

bool Server::validatePassword(const std::string &clientPassword, const std::string &expectedPassword) {
    return expectedPassword.empty() || clientPassword == expectedPassword; // if there's no password or password matched return turue 
}

// int Server::process_client_message(int clientfd)
// {
//     std::array<char, READ_BUFFER_SIZE> buffer;
//     ssize_t ret = recv(clientfd, buffer.data(), buffer.size(), 0);
//     if (ret <= 0){
//         //                         if (ret == 0) { 
//         //     LOG_INFO("Client closed connection");  // Graceful close
//         // } else {
//         //     LOG_ERROR("Error occurred on socket: " << strerror(errno)); 
//         // }
//         LOG_ERROR("To remove client");
//         FD_CLR(clientfd, &(this->all_objs));
//         close(clientfd);
//     }
//     std::string tmp;
//     tmp.reserve(ret);
//     for (ssize_t s = 0; s < ret; s++) {
//     tmp.push_back(buffer.data()[s]);
//     }
//     LOG_INFO("This content is: " << tmp << "the size is: " << ret);
//     std::pair<bool, std::string> result = this->clients[clientfd]->setBuffer(tmp); // check if I recieved the \r\n : 
//     if (result.first) // message is finished!
//     {
//         // handel message by client -> 
//         std::vector<std::string> CommandParsed = tools.CommandParser(result.second);
//         for (size_t i = 0; i < CommandParsed[0].size(); ++i) {
//             CommandParsed[0][i] = tolower(CommandParsed[0][i]); 
//         }
//         if ((CommandParsed[0] != "nick" && CommandParsed[0] != "pass" && CommandParsed[0] != "user") && (!this->clients[clientfd]->getAuthStatus() || !this->clients[clientfd]->getNickName().empty()
//                 || !this->clients[clientfd]->getNickName().empty())) {
//                 LOG_INFO("HE HAS NO AUTH");
//                 return 1;
//         }
//         if (CommandParsed[0] == "join")
//         {
//             // parse join args :

//             // check channel exists => if not create it =>
//                     /*
//                         - Channel name rules:
//                             Channels names are strings (beginning with a '&' or '#' character) of
//                             length up to 200 characters.  Apart from the the requirement that the
//                             first character being either '&' or '#'; the only restriction on a
//                             channel name is that it may not contain any spaces (' '), a control G
//                             (^G or ASCII 7), or a comma (',' which is used as a list item
//                             separator by the protocol).
//                         - when creating the channel : the channel is created and the creating user becomes a
//                             channel operator.
//                     */
//             // if the channel exists :
//             /*
//                whether or not your
//                 request to JOIN that channel is honoured depends on the current modes
//                 of the channel. For example, if the channel is invite-only, (+i),
//                 then you may only join if invited.  As part of the protocol, a user
//                 may be a part of several channels at once, but a limit of ten (10)
//                 channels is recommended as being ample for both experienced and
//                 novice users.
//             */
             
//             // this->channels
//         }
//         if (CommandParsed[0] == "nick")
//         {
//             if (!this->clients[clientfd]->getAuthStatus())
//             {
//                 LOG_ERROR("YOU NEED A PASS FIRST");
//                 return 1;
//             }
//             //   nickname   =  ( letter / special ) *8( letter / digit / special / "-" ) => total 9 chars
//         }
//         else if (CommandParsed[0] == "pass")
//         {

//         }
//         else if (CommandParsed[0] == "user")
//         {
//             if (!this->clients[clientfd]->getAuthStatus())
//             {
//                 LOG_ERROR("YOU NEED A PASS FIRST");
//                 return 1;
//             }
//         }
//         else if (CommandParsed[0] == "mod")
//         {
//             //    Parameters: <channel> {[+|-]|o|p|s|i|t|n|b|v} [<limit>] [<user>]
//             //                [<ban mask>]

//             //    The MODE command is provided so that channel operators may change the
//             //    characteristics of `their' channel.  It is also required that servers
//             //    be able to change channel modes so that channel operators may be
//             //    created.

//             //    The various modes available for channels are as follows:

//             //            o - give/take channel operator privileges;
//             //            p - private channel flag;
//             //            s - secret channel flag;
//             //            i - invite-only channel flag;
//             //            t - topic settable by channel operator only flag;
//             //            n - no messages to channel from clients on the outside;
//             //            m - moderated channel;
//             //            l - set the user limit to channel;
//                 //         b - set a ban mask to keep users out;
//                 //         v - give/take the ability to speak on a moderated channel;
//                 //         k - set a channel key (password).

//                 //         When using the 'o' and 'b' options, a restriction on a total of three
//                 //         per mode command has been imposed.  That is, any combination of 'o'

//         } else if (CommandParsed[0] == "topic") 
//         {
//             // Parameters: <channel> [<topic>]

//             // The TOPIC message is used to change or view the topic of a channel.
//             // The topic for channel <channel> is returned if there is no <topic>
//             // given.  If the <topic> parameter is present, the topic for that
//             // channel will be changed, if the channel modes permit this action.
//         }
//         // send message by client..
//     }
// }

// int Server::broadcast_message(std::string message, Channel* channel)
// {

// }
