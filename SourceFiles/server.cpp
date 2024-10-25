#include "../HeaderFiles/Server.hpp"
#include <typeinfo>
#include <sstream>

bool isall_objsdigits(std::string str) 
{
    for (size_t i = 0; i < str.length(); i++)
        if (!std::isdigit((unsigned char)str[i]))
            return false;
    return true;
}

void Server::checkArgs(int argc, char **argv) {
    port = "6667";

    // if (argc != 3) {
    //     std::cerr << RED "Usage: ./a.out <port> <password>" RESET << std::endl;
    //     exit(EXIT_FAILURE);
    // }

    if (argc == 1) {
        LOG_MSG(BLUE "NOTE:" "The programe will run with the default server");
    }
    else {
        if (argc > 3)
            LOG_MSG( BLUE "NOTE:"  " We will only be accepting the first two arguments. " 
                      << "Any additional arguments will be ignored.")
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

    LOG_SERVER("Server started on port " << port);
    LOG_SERVER("Server started with the password " << "<" << password << ">");
}

Server::~Server(){
    close(serverSocket); 
}

int Server::getServerSocket() {
    return (serverSocket);
}

void sendHellGate(int client_socket) {
    std::string hellGate = 
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

    clients.push_back(Client(newClientSocket)); // Add client to the server's client list
}

std::vector<std::string> splitMessages(const std::string &buffer) {
    std::vector<std::string> lines;
    std::istringstream stream(buffer);
    std::string line;
    
    while (std::getline(stream, line))
        lines.push_back(line);
    
    return lines;
}

bool Server::processPassCommand(Client &client, const std::string &message) {
    if (message.find("PASS") == 0 && client.getPassword().empty()) {
        if (message.length() <= 5) {
            LOG_SERVER("Error: PASS command provided without a value.");
            // client.sendReply(462, client);
            return true;
        }

        std::string pass = message.substr(5);
        client.setPassword(pass);
        if (!pass.empty() && (pass[pass.length() - 1] == '\r' || pass[pass.length() - 1] == '\n'))
            pass.erase(pass.length() - 1);

        if (pass != password) {
            LOG_SERVER("Invalid password");
            client.clearBuffer();
            return false;
        }

        client.setPassword(pass);
        LOG_SERVER("Valid password");
        return true;
    }
    return false;
}

bool Server::processNickCommand(Client &client, const std::string &message) {
    if (message.find("NICK") == 0) {
        if (message.length() <= 5) {
            LOG_SERVER("Error: NICK command provided without a value.");
            return true;
        }

        client.setNickName(message.substr(5));
        client.sendReply(001, client);// set you're nickname
        LOG_SERVER("Client NICK setup");
        return true;
    }
    return false;
}
// /quote USER myusername myhostname localhost :MyCustomRealName
bool Server::processUserCommand(Client &client, const std::string &message) {
    if (message.find("USER") == 0) {
        std::stringstream ss(message.substr(5));
        std::string username, hostname, realname;
        
        ss >> username >> hostname;  // Extract username and hostname

        size_t realnamePos = message.find(':');
        if (realnamePos != std::string::npos) {
            realname = message.substr(realnamePos + 1);
            if (!realname.empty() && (realname[realname.length() - 1] == '\r' || realname[realname.length() - 1] == '\n'))
                realname.erase(realname.length() - 1);
        }

        if (username.empty() || hostname.empty() || realname.empty()) {
            LOG_SERVER("Error: USER command provided without a complete value.");
            return true;
        }

        client.setUserName(username);
        client.setHostName(hostname);
        client.setRealName(realname);
        client.sendReply(002, client);// set you're host
        LOG_SERVER("Client USER setup: username=" + username + ", hostname=" + hostname + ", realname=" + realname);
        return true;
    }
    return false;
}

bool Server::setUpClient(Client &client) {
    std::vector<std::string> messages = splitMessages(client.getBuffer());

    if (client.isAuthenticated())
        return false;

    for (std::vector<std::string>::size_type i = 0; i < messages.size(); ) {
        const std::string &message = messages[i];

        if (processPassCommand(client, message)) {
            messages.erase(messages.begin() + i);
            continue;
        }else if (processNickCommand(client, message)) {
            messages.erase(messages.begin() + i);
            continue;
        }else if (processUserCommand(client, message)) {
            messages.erase(messages.begin() + i);
            continue;
        } 
        else
            ++i;
    }

    if (!client.isAuthenticated() && (client.getPassword().empty() || client.getNickName().empty() || client.getUserName().empty())) {
        LOG_SERVER("Error: PASS, NICK, and USER must all be provided.");
        return false;
    }

    client.authenticate();
    return true;
}

void Server::updateNickUser(Client &client) {
    std::vector<std::string> messages = splitMessages(client.getBuffer());

    for (std::vector<std::string>::size_type i = 0; i < messages.size(); ) {
        const std::string &message = messages[i];

        if (processNickCommand(client, message)) {
            messages.erase(messages.begin() + i);
            continue;
        } else if (processUserCommand(client, message)) {
            messages.erase(messages.begin() + i);
            continue;
        } else
            ++i;
    }
}

void sendCapResponse(int clientSocket) {
    std::string capResponse = ":localhost CAP * LS :\r\n"; // sending Empty CAP List (do not support any advanced features)

    send(clientSocket, capResponse.c_str(), capResponse.size(), 0);
    LOG_SERVER("Sent CAP LS response to client");
}

void ping(std::string message, int ClientSocket) {
    std::string pingToken = message.substr(5);
    std::string pongResponse = "PONG " + pingToken + "\r\n";

    // Send PONG response back to the client (to stay connecte without it the connection to
    // irssi client will restart if no pong send)
    send(ClientSocket, pongResponse.c_str(), pongResponse.size(), 0);
    LOG_CLIENT(message);
    LOG_SERVER(pongResponse);
}

void Server::handleClientMessage(Client &client, const std::string &message) {
    client.appendToBuffer(message);
    std::cout << "Received from client: "  << message ;
    if (message.find("CAP LS 302") != std::string::npos)
        sendCapResponse(client.getSocket());
    else if (message.find("PING") == 0) {
       ping(message, client.getSocket());
    } else {
        if (setUpClient(client)) {
            sendHellGate(client.getSocket());
            LOG_SERVER("Client setup completed successfully for " << client.getNickName());
            client.clearBuffer();
        } else 
            updateNickUser(client);
    }
}

void Server::processClienstMessage(fd_set readfds) {
    char buffer[BUFFER_SIZE];

    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ) {
        memset(buffer, 0, BUFFER_SIZE);
        if (FD_ISSET(it->getSocket(), &readfds)) {
            ssize_t bytesReceived = recv(it->getSocket(), buffer, BUFFER_SIZE, 0);

            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {LOG_SERVER("Client disconnected ");}
                else
                    perror("recv error");
                close(it->getSocket());
                it = clients.erase(it);
                continue;
            } else {
                std::string clientMessage(buffer);
                handleClientMessage(*it, clientMessage);
                if (it->getPassword() != password && !it->getPassword().empty()) {
                    close(it->getSocket());
                    it = clients.erase(it);
                    LOG_SERVER("client out");
                    continue;
                }
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
