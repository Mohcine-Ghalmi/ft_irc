#include "../HeaderFiles/Server.hpp"

std::vector<std::string> parameters(const std::string &message, std::string &modeSection, std::string &channel) {
    std::istringstream ss(message);
    std::string command;
    std::vector<std::string> parameters;

    ss >> command;
    ss >> channel;
    ss >> modeSection;
    std::string param;
    while (ss >> param)
        parameters.push_back(param);

    return parameters;
}

void sendModeIRepleyToChannel_TMP(Client &client, Channel &channel, bool isRemoving, char mode) {
    std::stringstream ss;

    ss << ":" << client.getNickName() << " 324 " << client.getNickName() << " "
       << channel.getName() << " "
       << (isRemoving ? "+" : "-") << mode << "\r\n";
    for (std::map<std::string, Client>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
        Client targetClient = it->second;
        send(targetClient.getSocket(), ss.str().c_str(), ss.str().length(), 0);
    }
}

bool Server::processModeCommand(Client &operatorClient, const std::string &message) {
    (void)operatorClient;
    if (!this->proccessCommandHelper(message, "MODE"))
        return false;

    std::string channelName, modes;
    std::vector<std::string> params = parameters(message, modes, channelName);
    std::string command = "MODE";

    if (command.empty() || modes.empty()) {
        operatorClient.ERR_NEEDMOREPARAMS(operatorClient, "MODE");
        return false;
    }

    Channel* channel = getChannel(channelName);
    if (!channel) {
        operatorClient.ERR_NOSUCHCHANNEL(operatorClient, channelName); // Channel doesn't exist
        LOG_ERROR("Channel Not Found");
        return false;
    }

    if (channel->getOperators().find(operatorClient.getNickName()) == channel->getOperators().end()) {
        operatorClient.ERR_CHANOPRIVSNEEDED(operatorClient, channelName);
        LOG_ERROR(operatorClient.getNickName() << " Is Not An Operator On This Channel");
        return false;
    }

    size_t paramsInc = 0;
    Client *newOperator = nullptr;
    char action = '+'; // Default action

    for (size_t i = 0; i < modes.length(); ++i) {
        char mode = modes[i];

        // Update action when encountering '+' or '-'
        if (mode == '+' || mode == '-') {
            action = mode;
            continue;
        }

        switch (mode) {
            case 'i': // Invite-only
                ft_setInviteOnly(channel, operatorClient, action);
                break;

            case 't': // Topic restriction
                channel->setTopicRestriction((action == '+') ? 1 : 0);
                break;

            case 'k': // Channel key
                if (paramsInc < params.size()) {
                    if (action == '+') {
                        channel->setKeyProtection(1, params[paramsInc++]);
                    } else if (action == '-' && params[paramsInc++] == channel->getKey()) {
                        channel->setKeyProtection(0, "");
                    } else {
                        operatorClient.ERR_BADCHANNELKEY_CHANNEL(operatorClient, channel->getName());
                        break ;
                    }
                    sendModeIRepleyToChannel_TMP(operatorClient, *channel, (action == '+' ? 1 : 0), 'k');
                } else {
                    LOG_ERROR("Key protection requires a parameter");
                }
                break;

            case 'o': // Operator
                if (paramsInc >= params.size()) {
                    LOG_ERROR("Operator change requires a parameter");
                    return false;
                }

                removeCarriageReturn(params[paramsInc]);
                newOperator = getClientByNick(params[paramsInc]);

                if (!newOperator) {
                    operatorClient.ERR_USERNOTINCHANNEL(operatorClient, params[paramsInc], channelName);
                    LOG_ERROR("User Not Found");
                    paramsInc++;
                    break;
                }

                if (channel->getMembers().find(newOperator->getNickName()) != channel->getMembers().end()) {
                    ft_removeAddOperator(operatorClient, newOperator, channel, action);
                    paramsInc++;
                } else {
                    operatorClient.ERR_USERNOTINCHANNEL(operatorClient, newOperator->getNickName(), channelName);
                    return false;
                }
                break;

            default:
                LOG_ERROR("Unknown mode: " << mode);
                operatorClient.ERR_UNKNOWNMODE(operatorClient ,channelName , mode);
                break;
        }
    }

    return true;
}

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
    if (adding == '+') {
        if (channel->getOperators().find(newOperator->getNickName()) == channel->getOperators().end()) {
            // Add the new operator to the channel
            operatorClient.RPL_NEWOPERATOR(*newOperator, operatorClient, channel->getName(), false, channel->getMembers());
            channel->addOperator(newOperator);
        } else {
            operatorClient.RPL_ALREADYOPERATOR(operatorClient, channel->getName(), newOperator->getNickName(), true);
        }
    } else if (adding == '-') {
        if (channel->getOperators().find(newOperator->getNickName()) != channel->getOperators().end()) {
            // Prevent the operator from removing themselves
            if (operatorClient.getNickName() == newOperator->getNickName()) {
                operatorClient.RPL_ALREADYOPERATOR(*newOperator, channel->getName(), newOperator->getNickName(), true);
                return ;
            }

            // Remove the operator from the channel
            newOperator->RPL_NEWOPERATOR(*newOperator,operatorClient ,channel->getName(), true, channel->getMembers());
            channel->removeOperator(newOperator);

        } else {
            operatorClient.RPL_ALREADYOPERATOR(operatorClient, channel->getName(), newOperator->getNickName(), false);
        }
    }
}
