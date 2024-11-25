#include "../HeaderFiles/Server.hpp"

void sendModeIRepleyToChannel(Client &client, Channel &channel, bool inviteOnly) {
    std::stringstream ss;

    ss << ":" << " NOTICE " << channel.getName()
       << " :" << client.getNickName() << " This Channel is "
       << (inviteOnly ? "Private" : "Public")
       <<".\r\n";

    for (std::map<std::string, Client>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
        Client targetClient = it->second;
        send(targetClient.getSocket(), ss.str().c_str(), ss.str().length(), 0);
    }
}

void    ft_setInviteOnly(Channel *channel, Client &operatorClient, char mode) {
    int value = (mode == '+' ? 1 : 0);
    if ((value && channel->isInviteOnly()) || (!value && !channel->isInviteOnly()))
        return ;
    sendModeIRepleyToChannel(operatorClient, *channel, value);
    channel->setInviteOnly(value);
}
