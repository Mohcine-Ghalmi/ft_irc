#include "../HeaderFiles/Server.hpp"
#include <typeinfo>
#include <sstream>


#define LOG_INFO(custom_string)  \
    std::cout  << "INFO: " << custom_string<< std::endl; 
 
#define LOG_ERROR(custom_string)  \
    std::cout << "ERROR: " << custom_string<< std::endl;

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
        LOG_MSG(BLUE "NOTE:" "The programe will run with the default server");
    }
    else {
        if (argc > 3)
            LOG_MSG( BLUE "NOTE:"  " We will only be accepting the first two arguments. " 
                      << "Any additional arguments will be ign.")
        if (argc == 2) 
            LOG_MSG(BLUE "WARNING" " The program is running without a password.");
        if (!isall_objsdigits(argv[1])) {LOG_ERROR("The programe will run with the default Port");}
        else {
            char *endptr;
            long int result = strtol(argv[1], &endptr, 10);

                if (*endptr != '\0' || result == LONG_MIN || result == LONG_MAX) {
                    LOG_MSG(BLUE "NOTE:" "The programe will run with the default Port");
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
        "    🔥          Welcome to       🔥\n"
        "    🔥            Hell!          🔥\n"
        "    🔥                           🔥\n"
        "    🔥   👿  Beware of the       🔥\n"
        "    🔥   Darkness and Flames!    🔥\n"
        "    🔥                           🔥\n"
        "🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥\r\n";

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
    // sendHellGate(newClientSocket, "newUser");
}

std::vector<std::string> splitMessages(const std::string &buffer) {
    std::vector<std::string> lines;
    std::istringstream stream(buffer);
    std::string line;
    
    while (std::getline(stream, line))
        lines.push_back(line);
    
    return lines;
}

// void Server::disconnectClient(Client &client) {
//     std::cout << "Disconnecting client " << client.getNickName() << "..." << std::endl;
//     close(client.getSocket()); // Close the socket
//     // Remove the client from the list of connected clients
//     clients.erase(std::remove_if(clients.begin(), clients.end(), [&](const Client& c) {
//         return c.getSocket() == client.getSocket();
//     }), clients.end());
//     std::cout << "Client disconnected." << std::endl;
// }

bool Server::setUpClient(Client &client) {
    std::string msg = client.getBuffer();  // Get the log of messages

    std::vector<std::string> messages = splitMessages(client.getBuffer());
    std::string pass, nick;

    // Iterate through each line of the buffer
    for (const std::string &message : messages) {
        // Check if the line starts with "PASS"
        if (message.find("PASS") == 0) {
            pass = message.substr(5);  // Extract the password (characters after "PASS ")
            pass = pass.substr(0, pass.length() - 1); // deleting the new line 
            if (pass != password) {
                std::cout <<  "invalide password" <<  std::endl;
                return (0);
            } else
                std::cout << "valide password" << std::endl;

        }
        // Check if the line starts with "NICK"
        else if (message.find("NICK") == 0) {
            nick = message.substr(5);  // Extract the nickname (characters after "NICK ")
            client.setNickName(nick);
            sendHellGate(client.getSocket(), nick);
        }
    }

    return !pass.empty() && !nick.empty();
}

void sendCapResponse(int clientSocket) {
    std::string capResponse = ":localhost CAP * LS :\r\n"; // sending Empty CAP List (do not support any advanced features)

    send(clientSocket, capResponse.c_str(), capResponse.size(), 0);
    std::cout << "Sent CAP LS response to client"  << std::endl;
}

void ping(std::string message, int ClientSocket) {
    std::string pingToken = message.substr(5);
    std::string pongResponse = "PONG " + pingToken + "\r\n";

    // Send PONG response back to the client (to stay connectes withouting the connection the irssi client wil restart if no pong send recived)
    send(ClientSocket, pongResponse.c_str(), pongResponse.size(), 0);
    std::cout << message;
    std::cout << pongResponse;
}

void Server::handleClientMessage(Client &client, const std::string &message) {
    client.appendToBuffer(message);
    if (message.find("CAP LS 302") != std::string::npos)
        sendCapResponse(client.getSocket());
    else if (message.find("PING") == 0) {
       ping(message, client.getSocket());
    } else {
        std::cout << "Received from client: "  << message << std::endl;
        if (setUpClient(client)) {
            std::cout << "Client setup completed successfully for " << client.getNickName() << std::endl;
            client.clearBuffer();
        }
    }
}

void Server::processClienstMessage(fd_set readfds) {
    char buffer[BUFFER_SIZE];

    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ) {
        memset(buffer, 0, BUFFER_SIZE);
        if (FD_ISSET(it->getSocket(), &readfds)) {
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

void Server::shutdownServer() {
    std::cout << "server shutdown ... !" << std::endl;

}

// int Server::process_client_message(int clientfd)
// {
//     std::array<char, READ_BUFFER_SIZE> buffer;
//     ssize_t ret = recv(clientfd, buffer.data(), buffer.size(), 0);
//     if (ret <= 0){
//         //                         if (ret == 0) { 
//         //     LOG_INFO("Client closed connection");  // Graceful close
//         // } else {
//         //     LOG_ERROR("Error occu on socket: " << strerror(errno)); 
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
//                 request to JOIN that channel is hono depends on the current modes
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
//             //    characteristics of `their' channel.  It is also requ that servers
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
