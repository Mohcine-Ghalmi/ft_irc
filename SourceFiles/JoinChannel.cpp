#include "../HeaderFiles/Server.hpp"

bool Server::processJoinCommand(Client &client, const std::string &message) {
    if (this->proccessCommandHelper(message, "JOIN")) {
        std::stringstream ss(message);
        std::string command, channelName, key;
        ss >> command >> channelName >> key;
        if (channelName.empty()) {
            client.ERR_NEEDMOREPARAMS(client, "JOIN");  // Send ERR_NEEDMOREPARAMS if no channel specified
            return false;
        }
        removeCarriageReturn(channelName);
        removeCarriageReturn(key);
        if (joinChannel(client, channelName, key)){

            // sendJoinedRepleyToChannel(client, *channel, client.getNickName());
            client.sendReply(331, client);  // Send RPL_NOTOPIC or similar welcome
            return true;
        }
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
