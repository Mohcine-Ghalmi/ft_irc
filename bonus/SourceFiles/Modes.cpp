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
        operatorClient.ERR_NOSUCHCHANNEL(operatorClient, channelName);
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
    char action = '+';

    for (size_t i = 0; i < modes.length(); ++i) {
        if (modes[i] != '-' && modes[i] != '+' && modes[i] != 'o' && modes[i] != 'k' && modes[i] != 'l' && modes[i] != 't' && modes[i] != 'i')
        {
            operatorClient.ERR_UNKNOWNMODE(operatorClient ,channelName , modes[i]);
            return false;
        }
    }
    for (size_t i = 0; i < modes.length(); ++i) {
        char mode = modes[i];

        if (mode == '+' || mode == '-') {
            action = mode;
            continue;
        }

        switch (mode) {
            case 'i':
                ft_setInviteOnly(channel, operatorClient, action);
                break;

            case 't':
                if ((action == '+' && channel->isTopicRestricted()) || (action == '-' && !channel->isTopicRestricted()))
                    break ;
                channel->setTopicRestriction((action == '+') ? 1 : 0);
                operatorClient.broadcastModeChange(operatorClient.getNickName(), 't', "", channel->getMembers(), channel->getName(), action);
                break;

            case 'k':
                if (paramsInc < params.size()) {
                    if (action == '+') {
                        channel->setKeyProtection(1, params[paramsInc++]);
                    } else if (action == '-' && params[paramsInc++] == channel->getKey()) {
                        channel->setKeyProtection(0, "");
                    } else {
                        operatorClient.ERR_BADCHANNELKEY_CHANNEL(operatorClient, channel->getName());
                        break ;
                    }
                    operatorClient.broadcastModeChange(operatorClient.getNickName(), 'k', "", channel->getMembers(), channel->getName(), action);
                } else {
                    LOG_ERROR("Key protection requires a parameter");
                }
                break;

            case 'o':
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

            case 'l':
                if (action == '+') {
                    if (paramsInc < params.size()) {
                        int userLimit = std::stoi(params[paramsInc++]);
                        if (userLimit > 0) {
                            channel->setUserLimit(userLimit);
                            operatorClient.broadcastModeChange(operatorClient.getNickName(), 'l', "", channel->getMembers(), channel->getName(), '+');
                        } else {
                            LOG_ERROR("Invalid user limit");
                        }
                    } else {
                        LOG_ERROR("User limit requires a parameter");
                    }
                } else if (action == '-') {
                    channel->setUserLimit(0);
                    operatorClient.broadcastModeChange(operatorClient.getNickName(), 'l', "", channel->getMembers(), channel->getName(), '-');
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

void    ft_setInviteOnly(Channel *channel, Client &operatorClient, char mode) {
    int value = (mode == '+' ? 1 : 0);
    if ((value && channel->isInviteOnly()) || (!value && !channel->isInviteOnly()))
        return ;
    operatorClient.broadcastModeChange(operatorClient.getNickName(), 'i', "", channel->getMembers(), channel->getName(), mode);
    channel->setInviteOnly(value);
}

void    ft_removeAddOperator(Client &operatorClient, Client *newOperator, Channel *channel, const char &adding) {
    if (adding == '+') {
        if (channel->getOperators().find(newOperator->getNickName()) == channel->getOperators().end()) {
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

            newOperator->RPL_NEWOPERATOR(*newOperator,operatorClient ,channel->getName(), true, channel->getMembers());
            channel->removeOperator(newOperator);

        } else {
            operatorClient.RPL_ALREADYOPERATOR(operatorClient, channel->getName(), newOperator->getNickName(), false);
        }
    }
}
