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
        sender.ERR_NOSUCHCHANNEL(sender, channelName);//channel doesn't exist
        // sender.sendReply(ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }

    if (!channel->isMember(&sender)) {
        LOG_SERVER("client is not in teh channel");
        // sender.sendReply(ERR_CANNOTSENDTOCHAN, channelName + " :Cannot send to channel");
        return;
    }
    std::string formattedMessage = ":" + sender.getNickName() + " PRIVMSG " + channelName + " :" + messageText + "\r\n";
    for (std::map<std::string, Client>::iterator it =  channel->getMembers().begin(); it != channel->getMembers().end(); it++) {
        Client& targetClient = it->second;
        std::cout << targetClient.getNickName() << std::endl;
        if (targetClient.getNickName() != sender.getNickName())
            send(targetClient.getSocket(), formattedMessage.c_str(), formattedMessage.length(), 0);
    }
}


bool Server::processPrivMsgCommand(Client &sender, const std::string &message) {
    if (!this->proccessCommandHelper(message, "PRIVMSG"))
        return false;

    std::istringstream ss(message.substr(8));
    std::string targetList, messageText;
    ss >> targetList;
    std::getline(ss, messageText);

    if (!messageText.empty() && messageText[0] == ' ')
        messageText = messageText.substr(1);

    std::stringstream targetStream(targetList);
    std::string targetNick;
    removeCarriageReturn(messageText);
    while (std::getline(targetStream, targetNick, ',')) {
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

bool Server::checkInvitesToChannel(Client &operatorClient, Channel *channel, std::string &channelName, Client *userInvited)
{
    if (!channel) {
        operatorClient.ERR_NOSUCHCHANNEL(operatorClient, channelName);//channel doesn't exist
        LOG_ERROR("Channel Not Found");
        return false;
    }
    if (!channel->isInviteOnly())
    {
        operatorClient.ERR_INVITEONLYCHAN(operatorClient, channelName);
        LOG_ERROR("Channel is not invite-only");
        return false;
    }
    if (channel->getMembers().find(userInvited->getNickName()) != channel->getMembers().end())
    {
        operatorClient.ERR_USERONCHANNEL(operatorClient, userInvited->getNickName(), channelName);
        LOG_ERROR("User already on channel");
        return false;
    }
    return true;
}

void sendInviteReply(Client operatorClient,Channel &channel, const std::string &invited) {
    std::string message = ":" + operatorClient.getHostname() +
                          " INVITE " + invited +
                          " :" + channel.getName() + "\r\n";

    for (std::map<std::string, Client>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
        Client targetClient = it->second;
        std::cout << "Sending INVITE reply to: " << targetClient.getNickName() << std::endl;
        send(targetClient.getSocket(), message.c_str(), message.length(), 0);
    }
}

void sendkickRepleyToChannel(Client operatorClient,Channel &channel, const std::string &kickedUser) {
    std::stringstream ss3;
    ss3 << ":" << operatorClient.getHostname()
        << " NOTICE " << channel.getName()
        << " :" << "Kicked " << kickedUser << "\r\n";


    for (std::map<std::string, Client>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
        Client targetClient = it->second;
        send(targetClient.getSocket(), ss3.str().c_str(), ss3.str().length(), 0);
    }
}

void sendTopicRepleyToChannel(Client operatorClient,Channel &channel, const std::string &topic) {
    std::stringstream ss;
    ss << ":" << operatorClient.getNickName() << " TOPIC " << channel.getName()
       << " :" << topic << "\r\n";


    for (std::map<std::string, Client>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
        Client targetClient = it->second;
        send(targetClient.getSocket(), ss.str().c_str(), ss.str().length(), 0);
    }
}


#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> splitByDelimiter(const std::string &input, const std::string &delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end;
    while ((end = input.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(input.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(input.substr(start));
    return tokens;
}

bool Server::processKICKCommand(Client &operatorClient, const std::string &message) {
    // Split the message into individual KICK commands
    if (!proccessCommandHelper(message, "KICK"))
        return false;
    std::vector<std::string> commandList = splitByDelimiter(message, "\r\n");

    // Process each KICK command in the vector
    for (size_t i = 0; i < commandList.size(); ++i) {
        const std::string &singleMessage = commandList[i];
        if (!proccessCommandHelper(singleMessage, "KICK"))
            continue;

        std::stringstream ss(singleMessage);
        std::string command, channelName, user, reason;

        ss >> command >> channelName >> user;

        // Extract the reason from the message
        reason = ss.str().substr(ss.str().find(":"), ss.str().length());

        // Validate and process the KICK command
        if (user == operatorClient.getNickName()) {
            operatorClient.RPL_CANTKICKSELF(operatorClient, channelName);
            LOG_ERROR("You can't KICK yourself from a channel");
            continue;
        }

        Channel *channel = getChannel(channelName);
        if (!channel) {
            operatorClient.ERR_NOSUCHCHANNEL(operatorClient, channelName);
            LOG_ERROR("Channel not found: " << channelName);
            continue;
        }

        if (channel->getOperators().find(operatorClient.getNickName()) == channel->getOperators().end()) {
            operatorClient.ERR_CHANOPRIVSNEEDED(operatorClient, channelName);
            LOG_ERROR(operatorClient.getNickName() << " is not an operator on this channel");
            continue;
        }

        if (channel->getMembers().find(user) != channel->getMembers().end()) {
            Client *clientKicked = getClientByNick(user);
            channel->getMembers().erase(user);
            sendkickRepleyToChannel(operatorClient, *channel, user);
            clientKicked->RPL_KICKED(*clientKicked, channelName, operatorClient, reason);
        } else {
            operatorClient.ERR_NOSUCHNICKINCHANNEL(operatorClient, user, channelName);
            LOG_ERROR("User " << user << " is not in channel: " << channelName);
        }
    }

    return true; // Return true if processing completes successfully
}


bool Server::processTOPICcommand(Client &operatorClient, const std::string &message) {
    if (!this->proccessCommandHelper(message, "TOPIC"))
        return false;
    std::istringstream ss(message);
    std::string command, channelName, topic;
    ss >> command >> channelName;
    topic = ss.str().substr(ss.str().find(":") + 1, ss.str().length());
    Channel *channel = getChannel(channelName);
    if (channel->getOperators().find(operatorClient.getNickName()) == channel->getOperators().end()) {
        operatorClient.ERR_CHANOPRIVSNEEDED(operatorClient, channelName);
        LOG_ERROR(operatorClient.getNickName() << " is not an operator on this channel");
        return false;
    }
    if (!topic.empty())
    {
        channel->setTopic(topic);
        sendTopicRepleyToChannel(operatorClient, *channel, topic);
        LOG_MSG("the topic of " << channelName << " was changed to " << topic);
    }else {
        LOG_MSG("Topic : " << channel->getTopic());
    }
    return true;
}

bool Server::processINVITECommand(Client &operatorClient, const std::string &message) {
    if (!this->proccessCommandHelper(message, "INVITE"))
        return false;
    (void)operatorClient;
    std::istringstream ss(message);
    std::string command, userInvited, channelName;
    ss >> command >> userInvited >> channelName;
    if (channelName.empty() || userInvited.empty())
    {
        operatorClient.ERR_NEEDMOREPARAMS(operatorClient, "INVITE");
        return false;
    }
    Channel *channel = getChannel(channelName);
    Client *invitedClient = getClientByNick(userInvited);
    if (!invitedClient) {
        operatorClient.ERR_NOSUCHNICK(operatorClient, userInvited);//user doesn't exist
        LOG_ERROR("User Not Found");
        return false;
    }
    if (!checkInvitesToChannel(operatorClient, channel, channelName, invitedClient))
        return false;
    // operatorClient.RPL_INVITE(operatorClient, userInvited, channelName);
    sendInviteReply(operatorClient, *channel, userInvited);
    invitedClient->RPL_INVITESENTTO(*invitedClient, channelName, userInvited);
    return true;
}


bool Server::processModeCommand(Client &operatorClient, const std::string &message) {
    (void)operatorClient;
    if (!this->proccessCommandHelper(message, "MODE"))
        return false;
    std::istringstream ss(message);
    std::cout << message << std::endl;
    std::string command, channelName, modes, param;
    ss >> command >> channelName >> modes >> param;

    if (command.empty() || modes.empty())
    {
        operatorClient.ERR_NEEDMOREPARAMS(operatorClient, "MODE");
        return false;
    }
    Channel* channel = getChannel(channelName);
    if (!channel)
    {
        operatorClient.ERR_NOSUCHCHANNEL(operatorClient, channelName);//channel doesn't exist
        LOG_ERROR("Channel Not Found");
        return false;
    }
    if (channel->getOperators().find(operatorClient.getNickName()) == channel->getOperators().end())
    {
        operatorClient.ERR_CHANOPRIVSNEEDED(operatorClient, channelName);
        LOG_ERROR(operatorClient.getNickName() << " Is Not An Operator On This Channel");
        return false;
    }
    // if (!channel || !channel->isOperator(&operatorClient)) {
    //     operatorClient.sendReply(482, operatorClient);
    //     return false;
    // }
    for (size_t i = 0; i < modes.length(); ++i) {
        char mode = modes[i];
        switch (mode) {
            case 'i': // Invite-only
                if (modes[0] == '+')
                    channel->setInviteOnly(1);
                else
                    channel->setInviteOnly(0);
                break;
            case 't': // Topic restriction
                std::cout << "Mode Seted to t"<< std::endl;
                break;
            case 'k': // Channel key
                if (ss >> param) {
                    std::cout << "Mode Seted to k"<< std::endl;
                }
                break;
            case 'o': // Operator
                std::cout << "Mode Seted to o" << std::endl;
                // if (ss >> param) {
                //     Client* targetClient = getClientByNick(param);
                //     if (targetClient) {
                //         channel->addOperator(targetClient);
                //     }
                // }
                break;
            case 'l': // User limit
                if (ss >> param) {
                    // int limit = std::stoi(param);
                    std::cout << "Mode Seted to l"<< std::endl;
                }
                break;
        }
    }
    return true;
}

void Server::handleClientMessage(Client &client, const std::string &message) {
    client.appendToBuffer(message);
    if (hasNewline(message)) {
        LOG_CLIENT(client.getSocket(), message);
        if (message.find("CAP LS 302") != std::string::npos)
            sendCapResponse(client.getSocket());
        else if (message.find("PING") == 0)
            ping(message, client.getSocket());
        else {
            if (setUpClient(client)) {
                sendHellGate(client.getSocket());
                LOG_SERVER("Client setup completed successfully for " << client.getNickName());
            } else if (client.isAuthenticated()) {
                updateNickUser(client);
                if (processPrivMsgCommand(client, message)) {
                    LOG_INFO("message sent");}
                else if (processJoinCommand(client, message)) {
                    LOG_INFO("Joining done");
                }
                else if (processModeCommand(client, message)) {
                    LOG_INFO("Mode Set");
                }
                else if (processINVITECommand(client, message)) {
                    LOG_INFO("Invite sent");
                } else if (processPartCommand(client, message)) {
                    LOG_INFO("Part sent");
                } else if (processKICKCommand(client, message)) {
                    LOG_INFO("Part sent");
                }
                else if (processTOPICcommand(client, message)) {
                    LOG_INFO("Topic set");
                }
            }
            client.clearBuffer();
        }
    }
}

Channel* Server::createChannel(const std::string &channelName) {
    std::string realName; // this real name must be checked for the ',' so you get the names
    unsigned long pos = channelName.find(" ");
    if (pos == std::string::npos) realName = channelName;
    else realName = channelName.substr(0, pos);
    std::map<std::string, Channel>::iterator it = channels.find(realName);
    // Create the channel if it doesn't exist
    if (it == channels.end()){
        //creating the channel with it's name
        channels[channelName] = Channel(realName); // this should be after checking it it's a private or protected chat..
        // check options
        // meanwhile check the next part does it exists to get the key for protected channels.
    }
    if (pos != std::string::npos) return NULL; // TODO; this is should be an error cause the channel is already joind and the user is trying to set options : Know that this can make the code Crush
    return &channels[channelName];
}

Channel* Server::getChannel(const std::string &channelName) {
    std::map<std::string, Channel>::iterator itStart = channels.begin();
    std::map<std::string, Channel>::iterator itEnd = channels.end();

    while (itStart != itEnd)
        itStart++;

    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it != channels.end())
        return &it->second;
    return NULL;
}

bool Server::joinChannel(Client &client, const std::string &channelName) { // this should check the channel mode
    Channel* channel = createChannel(channelName);  // Create channel if it doesn't exist
    if (channel) {
        // if(channel->getMembers().find(client.getNickName()) != channel->getMembers().end()) {
        //     LOG_INFO("already a memeber "<< client.getNickName());
        //     return
        // }
        if (channel->isInviteOnly())
        {
            client.ERR_INVITEONLYCHAN(client, channelName);
            LOG_ERROR(channelName << "is invite only you can't join without invite");
            return false;
        }
        if (channel->getMembers().size() <= 0) // if the channel doesn't exists the first user joined it must be the operator
            channel->addOperator(&client);
        channel->addMember(&client);
        //client.sendReply(331, client);  // Send a welcome message or similar notification
        LOG_SERVER("client joind " << channel->getName() << " client number " << channel->getMembers().size());
        return true;
    }
    return false;
}

bool Server::processPartCommand(Client &client, const std::string &message) {
    if (!this->proccessCommandHelper(message, "PART"))
        return false;
    std::istringstream ss(message.substr(5));
    std::string channelName;
    ss >> channelName;

    if (leaveChannel(client, channelName)) {
        LOG_INFO("client " << client.getNickName() << "is out from " << channelName);
    }
    else
        LOG_INFO( channelName << "no channel founded");
    return true;
}


bool Server::leaveChannel(Client &client, const std::string &channelName) {
    Channel* channel = getChannel(channelName);
    if (channel && channel->isMember(&client)) {
        channel->removeMember(&client);
        if (channel->getMembers().empty()) {
            // Delete channel if empty
            channels.erase(channelName);
        }
        return true;
    }
    return false;
}

bool Server::processJoinCommand(Client &client, const std::string &message) {
    if (this->proccessCommandHelper(message, "JOIN")) {
        std::string channelName = message.substr(5);  // Assuming "JOIN #channel" format
        if (channelName.empty()) {
            client.ERR_NEEDMOREPARAMS(client, "JOIN");  // Send ERR_NEEDMOREPARAMS if no channel specified
            return false;
        }

        removeCarriageReturn(channelName);
        if (joinChannel(client, channelName)){
            client.sendReply(331, client);  // Send RPL_NOTOPIC or similar welcome
            return true;
        }
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
                clients.erase(it);
                it = clients.begin();
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
