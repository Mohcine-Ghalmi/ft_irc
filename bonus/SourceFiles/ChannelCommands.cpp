#include "../HeaderFiles/Server.hpp"

void Server::sendMessageToChannel(Client &sender, const std::string &channelName, const std::string &messageText) {
    Channel* channel = getChannel(channelName);
    if (!channel) {
        LOG_SERVER("channel doesn't exist");
        sender.ERR_NOSUCHCHANNEL(sender, channelName);
        return;
    }

    if (!channel->isMember(&sender)) {
        LOG_SERVER("client is not in the channel");
        return;
    }

    std::string msg = (messageText[0] == ':') ? messageText.substr(1) : messageText;
    std::stringstream formattedMessage;

    formattedMessage << ":" << sender.getNickName()
                     << " PRIVMSG " << channelName
                     << " :" << msg << "\r\n";

    for (std::map<std::string, Client>::iterator it =  channel->getMembers().begin(); it != channel->getMembers().end(); it++) {
        Client& targetClient = it->second;
        if (targetClient.getNickName() != sender.getNickName()) {
            send(targetClient.getSocket(), formattedMessage.str().c_str(), formattedMessage.str().length(), 0);
        }
    }
}

bool Server::processPrivMsgCommand(Client &sender, const std::string &message) {
    if (!this->proccessCommandHelper(message, "PRIVMSG"))
        return false;
    if (processBotcommand(sender, message)) {
        LOG_INFO("Bot Called");
        return false;
    }
    std::istringstream ss(message.substr(8));
    std::string targetList, messageText;
    ss >> targetList;
    std::getline(ss, messageText);
    if (!messageText.empty() && messageText[0] == ' ')
        messageText = messageText.substr(2);
    std::stringstream targetStream(targetList);
    std::string targetNick;
    removeCarriageReturn(messageText);
    while (std::getline(targetStream, targetNick, ',')) {
        if (targetNick[0] == '#') {
            Channel *targetChannel = getChannel(targetNick);
            if (targetChannel)
                sendMessageToChannel(sender, targetNick, messageText);
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
        operatorClient.ERR_NOSUCHCHANNEL(operatorClient, channelName);
        LOG_ERROR("Channel Not Found");
        return false;
    }
    if (channel->getOperators().find(operatorClient.getNickName()) == channel->getOperators().end())
    {
        operatorClient.ERR_CHANOPRIVSNEEDED(operatorClient, channelName);
        LOG_ERROR(operatorClient.getNickName() << " is not an operator on this channel");
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
    std::string message = ":" + operatorClient.getNickName() +
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
    ss3 << ":" << operatorClient.getNickName()
        << " 703 " << channel.getName()
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


bool Server::processKICKCommand(Client &operatorClient, const std::string &message) {
    if (!proccessCommandHelper(message, "KICK"))
        return false;
    std::vector<std::string> commandList = splitByDelimiter(message, "\r\n");

    for (size_t i = 0; i < commandList.size(); ++i) {
        const std::string &singleMessage = commandList[i];
        if (!proccessCommandHelper(singleMessage, "KICK"))
            continue;

        std::stringstream ss(singleMessage);
        std::string command, channelName, user, reason;

        ss >> command >> channelName >> user;

        size_t reasonPos = ss.str().find(":");
        reason = (reasonPos != std::string::npos)
            ? ss.str().substr(reasonPos + 1)
            : "";

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

        if (user == operatorClient.getNickName()) {
            operatorClient.RPL_CANTKICKSELF(operatorClient, channelName);
            LOG_ERROR("You can't KICK yourself from a channel");
            continue;
        }
        if (channel->getMembers().find(user) != channel->getMembers().end()) {
            Client *clientKicked = getClientByNick(user);
            channel->getMembers().erase(user);
            operatorClient.RPL_KICKED(*clientKicked, channelName, reason);
            sendkickRepleyToChannel(operatorClient, *channel, user);
            leaveChannel(*clientKicked, channel->getName());
            channel->removeOperator(clientKicked);
        } else {
            operatorClient.ERR_USERNOTINCHANNEL(operatorClient, user, channelName);
            LOG_ERROR("User " << user << " is not in channel: " << channelName);
        }
    }

    return true;
}


bool Server::processTOPICcommand(Client &operatorClient, const std::string &message) {
    if (!this->proccessCommandHelper(message, "TOPIC"))
        return false;
    std::istringstream ss(message);
    std::string command, channelName, topic;
    ss >> command >> channelName;
    if (ss.str().find(":") != std::string::npos)
        topic = ss.str().substr(ss.str().find(":") + 1, ss.str().length());
    Channel *channel = getChannel(channelName);
    if (!channel) {
        operatorClient.ERR_NOSUCHCHANNEL(operatorClient, channelName);
        return false;
    }
    if (channel && channel->isTopicRestricted() && channel->getOperators().find(operatorClient.getNickName()) == channel->getOperators().end()) {
        operatorClient.ERR_CHANOPRIVSNEEDED(operatorClient, channelName);
        LOG_ERROR(operatorClient.getNickName() << " is not an operator on this channel");
        return false;
    }
    removeCarriageReturn(topic);
    if (topic.empty())
        return false;
    else {
        if (channel->getTopic() == topic)
            return true;
        channel->setTopic(topic);
        channel->setTopicSetter(operatorClient.getNickName());
        sendTopicRepleyToChannel(operatorClient, *channel, topic);
        LOG_MSG("the topic of " << channelName << " was changed to " << topic);
    }
    return true;
}

bool Server::processINVITECommand(Client &operatorClient, const std::string &message) {
    if (!this->proccessCommandHelper(message, "INVITE"))
        return false;
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
        operatorClient.ERR_NOSUCHNICK(operatorClient, userInvited);
        LOG_ERROR("User Not Found");
        return false;
    }
    if (!checkInvitesToChannel(operatorClient, channel, channelName, invitedClient))
        return false;
    if (channel->isInviteOnly() && channel->getInvitedUsers().find(invitedClient->getNickName()) == channel->getInvitedUsers().end()) {
        channel->addInvitedUser(invitedClient);
        sendInviteReply(operatorClient, *channel, userInvited);
        invitedClient->RPL_INVITESENTTO(*invitedClient, channelName, operatorClient.getNickName());
    }
    else {
        operatorClient.RPL_PUBLICCHANNEL(operatorClient, channelName, channel->isInviteOnly());
        return false;
    }
    return true;
}


Channel* Server::createChannel(const std::string &channelName) {
    std::string realName;
    unsigned long pos = channelName.find(" ");
    if (pos == std::string::npos) realName = channelName;
    else realName = channelName.substr(0, pos);
    std::map<std::string, Channel>::iterator it = channels.find(realName);
    // Create the channel if it doesn't exist
    if (it == channels.end())
        channels[channelName] = Channel(realName);
    if (pos != std::string::npos) return NULL;
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
        if (channel->getMembers().empty())
            channels.erase(channelName);
        return true;
    }
    return false;
}

bool Server::processJoinCommand(Client &client, const std::string &message) {
    if (this->proccessCommandHelper(message, "JOIN")) {
        std::stringstream ss(message);
        std::string command, channelName, key;
        ss >> command >> channelName >> key;
        if (channelName.empty()) {
            client.ERR_NEEDMOREPARAMS(client, "JOIN");
            return false;
        }
        removeCarriageReturn(channelName);
        removeCarriageReturn(key);
        if (joinChannel(client, channelName, key))
            return true;
    }
    return false;
}

bool Server::joinChannel(Client& client, const std::string& channelName, const std::string& key) {
    Channel* channel = createChannel(channelName);
    if (!channel) return false;

    if (channel->isKeyProtected() && channel->getKey() != key) {
        client.ERR_BADCHANNELKEY(client, channelName);
        LOG_ERROR(channelName << " is protected (you need a key/password)");
        return false;
    }

    if (channel->isInviteOnly() &&
        channel->getInvitedUsers().find(client.getNickName()) == channel->getInvitedUsers().end()) {
        client.ERR_INVITEONLYCHAN(client, channelName);
        LOG_ERROR(channelName << " is invite only you can't join without invite");
        return false;
    }

    if (channel->getMembers().empty())
        channel->addOperator(&client);

    channel->addMember(&client);
    client.RPL_NAMREPLY(client, channel->getName(), channel->getMembers(), channel->getOperators());

    if (!channel->getTopic().empty())
        client.RPL_TOPIC(client, channel->getName(), channel->getTopicSetter());

    LOG_SERVER("client joined " << channel->getName() << " client number " << channel->getMembers().size());
    return true;
}
