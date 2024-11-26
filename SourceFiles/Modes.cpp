#include "../HeaderFiles/Server.hpp"

void sendModeIRepleyToChannel(Client &client, Channel &channel, bool inviteOnly) {
    std::stringstream ss;

    ss << ":" << client.getNickName()
        << " 703 " << channel.getName()
        << " :" << " This Channel is "
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

// this is for /mode +/- o
void    ft_removeAddOperator(Client &operatorClient, Client *newOperator, Channel *channel, const char &adding) {
    if (adding == '+')
    {
        if (channel->getOperators().find(newOperator->getNickName()) == channel->getOperators().end())
        {
            newOperator->RPL_NEWOPERATOR(*newOperator, channel->getName(), operatorClient, false);
            channel->addOperator(newOperator);
        }
        else
            operatorClient.RPL_ALREADYOPERATOR(operatorClient, channel->getName(), newOperator->getNickName(), true);
    }
    else
    {
        if (channel->getOperators().find(newOperator->getNickName()) != channel->getOperators().end())
        {
            newOperator->RPL_NEWOPERATOR(*newOperator, channel->getName(), operatorClient, true);
            channel->removeOperator(newOperator);
        }
        else
            operatorClient.RPL_ALREADYOPERATOR(operatorClient, channel->getName(), newOperator->getNickName(), false);
    }
}
