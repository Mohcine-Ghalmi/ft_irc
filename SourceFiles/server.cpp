#include "../HeaderFiles/Server.hpp"
#include <typeinfo>
#include <sstream>

bool Server::proccessCommandHelper(std::string cmd, std::string dif) {
    unsigned long pos = cmd.find(" ");
    if (pos == std::string::npos) return false;
    return cmd.substr(0, pos) == dif;
}

void Server::checkArgs(int argc, char **argv) {
    if (argc != 3) {
        LOG_ERROR(RED "Invalid number of arguments. Usage: ./server <port> <password>");
        exit(EXIT_FAILURE);
    }

    try {
        int portValue = std::stoi(argv[1]);
        if (portValue < 1024 || portValue > 65535) {
            LOG_ERROR(RED "Invalid port \"" << argv[1] << "\". Please specify a port number between 1024 and 65535.");
            exit(EXIT_FAILURE);
        }
        port = std::to_string(portValue);
    } catch (const std::invalid_argument&) {
        LOG_ERROR(RED "Invalid port \"" << argv[1] << "\" provided. Please specify a numeric port.");
        exit(EXIT_FAILURE);
    }

    password = argv[2];
    if (password.length() < 4) {
        LOG_ERROR(RED "Password is too short. Please provide a password with at least 4 characters.");
        exit(EXIT_FAILURE);
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
    // if (message.find("PASS") == 0 && client.getPassword().empty()) {
    if (this->proccessCommandHelper(message, "PASS") && client.getPassword().empty()) {
        if (message.length() <= 5) {
            LOG_SERVER("Error: PASS command provided without a value.");
            client.ERR_NEEDMOREPARAMS(client, "PASS");
            return true;
        }

        std::string pass = message.substr(5);
        client.setPassword(pass);
        if (!pass.empty() && (pass[pass.length() - 1] == '\r' || pass[pass.length() - 1] == '\n'))
            pass.erase(pass.length() - 1);

        if (pass != password) {
            client.sendReply(464, client); // we will not use this because pass is the first thing we check so we don't have enought data 
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

bool Server::isNickTaken(std::string &nick) {
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
        if (it->getNickName() == nick)
            return true; // Nickname is already taken
    return false;
}

void removeCarriageReturn(std::string &str) {
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
}

bool isValidNick(const std::string &nickname) {
    if (nickname.empty() || nickname[0] == '$' || nickname[0] == ':')
        return false;

    for (size_t i = 0; i < nickname.length(); i++)
        if (nickname[i] == ' ' || nickname[i] == ',' || nickname[i] == '*' || nickname[i] == '?' || nickname[i] == '!' || nickname[i] == '@')
            return false;

    if (nickname.find('.') != std::string::npos)
        return false;

    return true;
}

bool Server::processNickCommand(Client &client, const std::string &message) {
    // if (message.find("NICK") == 0) {
    if (this->proccessCommandHelper(message, "NICK")) {
        if (message.length() <= 5) {
            LOG_SERVER("Error: NICK command provided without a value.");
            client.ERR_NEEDMOREPARAMS(client, "NICK");
            return true;
        }

        std::string newNick = message.substr(5);
        removeCarriageReturn(newNick);

        if (!isValidNick(newNick)) {
            LOG_SERVER("Error: Invalid nickname.");
            client.ERR_ERRONEUSNICKNAME(client, newNick);
            return false;
        }

        if (isNickTaken(newNick)) {
            LOG_SERVER("Error: Nickname already taken.");
            if (client.isAuthenticated())
                client.ERR_NICKNAMEINUSE(client, newNick);
            return false;
        }

        client.setNickName(newNick);
        LOG_SERVER("Client NICK setup");
        return true;
    }
    return false;
}
// FOR irssi /quote USER myusername myhostname localhost :MyCustomRealName
bool Server::processUserCommand(Client &client, const std::string &message) {
    // if (message.find("USER") == 0) {
    if (this->proccessCommandHelper(message, "USER")) {
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
    client.sendReply(001, client);// set you're nickname
    client.sendReply(002, client);// set you're host
    client.authenticate();
    return true;
}

void Server::updateNickUser(Client &client) {
    std::vector<std::string> messages = splitMessages(client.getBuffer());

    for (std::vector<std::string>::size_type i = 0; i < messages.size(); ) {
        const std::string &message = messages[i];

        if (processNickCommand(client, message)) {
            messages.erase(messages.begin() + i);
            client.sendReply(001, client);
            // may add nick name updated reply 
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
    LOG_CLIENT(ClientSocket, message);
    LOG_SERVER(pongResponse);
}

Client* Server::getClientByNick(const std::string &targetNick) {
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
        if (it->getNickName() == targetNick)
            return &(*it);
    return NULL;
}

void Server::sendMessageToChannel(Client &sender, const std::string &channelName, const std::string &messageText) {
    // Find the channel by name
    Channel* channel = getChannel(channelName);
    if (!channel) {
        LOG_SERVER("channel doesn't exist");
        // sender.sendReply(ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }

    if (!channel->isMember(&sender)) {
        LOG_SERVER("client is not in teh channel");
        // sender.sendReply(ERR_CANNOTSENDTOCHAN, channelName + " :Cannot send to channel");
        return;
    }
    std::string formattedMessage = ":" + sender.getNickName() + " PRIVMSG " + channelName + " :" + messageText + "\r\n";

    for (std::set<Client*>::iterator it = channel->getMembers().begin(); it != channel->getMembers().end(); ++it) {
        Client* targetClient = *it;
        if (targetClient != &sender)
            send(targetClient->getSocket(), formattedMessage.c_str(), formattedMessage.length(), 0);
    }
}


bool Server::processPrivMsgCommand(Client &sender, const std::string &message) {
    if (!this->proccessCommandHelper(message, "PRIVMSG")) 
        return false;

    std::istringstream ss(message.substr(8));
    std::string targetList, messageText;
    ss >> targetList;
    getline(ss, messageText);

    if (!messageText.empty() && messageText[0] == ' ')
        messageText = messageText.substr(1);

    std::stringstream targetStream(targetList);
    std::string targetNick;
    while (std::getline(targetStream, targetNick, ',')) {
        removeCarriageReturn(messageText);
        if (targetNick[0] == '#') {
            Channel *targetChannel = getChannel(targetNick);
            if (targetChannel)
                sendMessageToChannel(sender, targetNick, messageText);  // Sends to all members of the channel
        } 
        else {
            Client *targetClient = getClientByNick(targetNick);
            if (targetClient) {
                std::string formattedMessage = ":" + sender.getNickName() + " PRIVMSG " + targetNick + " :" + messageText + "\r\n";
                send(targetClient->getSocket(), formattedMessage.c_str(), formattedMessage.length(), 0);
            } else 
                sender.ERR_NOSUCHNICK(sender, targetNick);
        }
    }

    LOG_SERVER("Message sent successfully to " + targetList);
    return true;
}


bool hasNewline(const std::string& message) {
    return message.find("\n") != std::string::npos;
}

void Server::handleClientMessage(Client &client, const std::string &message) {
    client.appendToBuffer(message);
    if (hasNewline(message)) {
        LOG_CLIENT(client.getSocket(), message);
        if (message.find("CAP LS 302") != std::string::npos)
            sendCapResponse(client.getSocket());
        else if (message.find("PING") == 0)
        // else if ("PING\r\n" == message)
            ping(message, client.getSocket());
        else {
            if (setUpClient(client)) {
                sendHellGate(client.getSocket());
                LOG_SERVER("Client setup completed successfully for " << client.getNickName());
            } else if (client.isAuthenticated()) {
                updateNickUser(client);
                if (processPrivMsgCommand(client, message))
                    LOG_INFO("message sent");
                processJoinCommand(client, message);
            }
            client.clearBuffer();
        }
    } 
}

//cmnt this code will work tadaaa magic 
Channel* Server::createChannel(const std::string &channelName) {
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    // Create the channel if it doesn't exist
    if (it == channels.end())
        channels[channelName] = Channel(channelName);
    return &channels[channelName];
}

Channel* Server::getChannel(const std::string &channelName) {
    std::map<std::string, Channel>::iterator itStart = channels.begin();
    std::map<std::string, Channel>::iterator itEnd = channels.end();

    while (itStart != itEnd) {
        std::cout << itStart->first.length() << std::endl;
        itStart++;
    }
    
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it != channels.end())
        return &it->second;
    return NULL;
}

bool Server::joinChannel(Client &client, const std::string &channelName) {
    Channel* channel = createChannel(channelName);  // Create channel if it doesn't exist
    if (channel) {
        channel->addMember(&client);
        //client.sendReply(331, client);  // Send a welcome message or similar notification
        LOG_SERVER("client joind " << channel->getName() << " client number " << channel->getMembers().size());
        return true;
    }
    return false;
}

bool Server::processJoinCommand(Client &client, const std::string &message) {
    // if (message.find("JOIN") == 0) {
    if (this->proccessCommandHelper(message, "JOIN")) {
        std::string channelName = message.substr(5);  // Assuming "JOIN #channel" format
        if (channelName.empty()) {
            client.ERR_NEEDMOREPARAMS(client, "JOIN");  // Send ERR_NEEDMOREPARAMS if no channel specified
            return false;
        }

        removeCarriageReturn(channelName);
        if (joinChannel(client, channelName))
            //client.sendReply(331, client);  // Send RPL_NOTOPIC or similar welcome
        return true;
    }
    return false;
}

void Server::processClienstMessage(fd_set readfds) {
    char buffer[BUFFER_SIZE];

    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ) {
        memset(buffer, 0, BUFFER_SIZE);
        if (FD_ISSET(it->getSocket(), &readfds)) {
            ssize_t bytesReceived = recv(it->getSocket(), buffer, BUFFER_SIZE, 0);

            if (bytesReceived <= 0) {
                if (bytesReceived == 0) {LOG_INFO(RED "Client disconnected ");}
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
                    LOG_INFO(RED "client out");
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
