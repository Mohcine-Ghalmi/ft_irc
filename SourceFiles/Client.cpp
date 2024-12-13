#include "../HeaderFiles/Client.hpp"
#include <sstream>

Client::Client(int socket) : clientSocket(socket),  authenticated(false), password("") {}

Client::Client(const Client &client) : clientSocket(client.clientSocket), nickName(client.nickName), userName(client.userName), hostName(client.hostName), realName(client.realName), authenticated(client.authenticated), buffer(client.buffer), password(client.password), replies(client.replies) { }

Client::~Client() { /**close(clientSocket);**/}
void Client::clearBuffer() { buffer.clear();}

std::string Client::getNickName() { return nickName; }
std::string Client::getUserName() { return userName; }
std::string Client::getBuffer() { return buffer;}
int Client::getSocket() { return clientSocket;}
std::string Client::getPassword() { return password; }
std::string Client::getHostname() {return hostName; }
bool Client::isAuthenticated() { return authenticated;}


void Client::setPassword(std::string &password) { this->password = password; }
void Client::setNickName(const std::string &nick) { nickName = nick;}
void Client::setUserName(const std::string &user) { userName = user; }
void Client::setHostName(const std::string &host) { hostName = host; }
void Client::setRealName(const std::string &real) { realName = real; }
void Client::appendToBuffer(const std::string &data) { buffer += data;}
void Client::authenticate() { authenticated = true;}


void Client::sendReply(int replyNumber, Client &client) {
    std::string message;

    switch (replyNumber) {
        case 001:
            message = replies.RPL_WELCOME(client.getNickName(), client.getUserName());
            break;
        case 002:
            message = replies.RPL_YOURHOST(client.getHostname(), client.getNickName());
            break;
        case 003:
            message = replies.RPL_CREATED(client.getHostname(), client.getNickName());
        case 004:
            message = replies.RPL_MYINFO(client.getHostname(), client.getNickName());
            break;
        case 005:
            message = replies.RPL_ISUPPORT(client.getHostname());
            break;
        case 464:
            message = replies.ERR_PASSWDMISMATCH(client.getNickName());
            break;
        default:
            break;
    }
    send(this->getSocket(), message.c_str(), message.length(), 0);
}

void Client::ERR_NICKNAMEINUSE(Client &client, const std::string &newNick) {
    std::stringstream ss;

    ss << ":" << client.getHostname() << " 433 " << client.getNickName() << " " << newNick <<" :Nickname is already in use\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::ERR_NOSUCHNICK(Client &client,  const std::string &targetNick) {
    std::stringstream ss;

    ss << ":" << client.getNickName() << " 401 " << client.getNickName()  << " " << targetNick << " :No such nick/channel\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::ERR_USERNOTINCHANNEL(Client &client, const std::string &targetNick, const std::string &channelName) {
    std::stringstream ss;

    ss << ":" << targetNick << " 441 " << channelName
       << " :They aren't on that channel\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::RPL_PUBLICCHANNEL(Client &client, const std::string &channelName, const bool &inviteOnly) {
    std::stringstream ss;

    ss << ":" << client.getNickName()
        << " 722 " << channelName
        << (inviteOnly ? " :user already invited\r\n" : " :this channel is public\r\n");
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::ERR_BADCHANNELKEY(Client &client, const std::string &channelName) {
    std::stringstream ss;

    // Constructing the error message according to the IRC protocol
    ss << ":" << client.getHostname()
       << " 475 " << client.getNickName()
       << " " << channelName
       << " :Cannot join channel (+k)\r\n";

    // Sending the error message to the client's socket
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}


void Client::RPL_TOPIC(Client &client, const std::string &channelName, const std::string &topic) {
    std::stringstream ss;

    ss << ":" << client.getHostname()
       << " 332 " << client.getNickName()
       << " " << channelName
       << " :" << topic << "\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::ERR_ERRONEUSNICKNAME(Client &client, const std::string &invalidNick) {
    std::stringstream ss;

    ss <<  ":" << client.getHostname() << " 432 " << client.getNickName() << " " << invalidNick << " :Erroneous nickname\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::ERR_NEEDMOREPARAMS(Client &client, std::string cmd) {
    std::stringstream ss;

    ss <<  ":" << client.getHostname() << " 461 " << client.getNickName() << " " << cmd << " :Not enough parameters\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::ERR_NOSUCHCHANNEL(Client &client, const std::string &channelName) {
    std::stringstream ss;

    ss << ":" << client.getHostname()
       << " 403 " << client.getNickName()
       << " " << channelName
       << " :No such channel\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::RPL_INVITE(Client &client, const std::string &invitedUser, const std::string &channelName) {
    std::stringstream ss;

    ss << ":" << client.getHostname()
       << " INVITE " << invitedUser
       << " :" << channelName << "\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

#include <set>
void Client::broadcastModeChange(const std::string &setterNick, const std::string &mode, const std::string &targetNick,  std::map<std::string, Client> &members, const std::string &channelName) {
    std::stringstream ss;
    ss << ":" << setterNick << " MODE " << channelName << " " << mode << " " << targetNick << "\r\n";

    for (std::map<std::string, Client >::iterator it = members.begin(); it != members.end(); ++it) {
        Client recipient = it->second;
        send(recipient.getSocket(), ss.str().c_str(), ss.str().length(), 0);
    }
}

//Irssi: (default) critical nicklist_set_host: assertion 'host != NULL' failed
void Client::RPL_NAMREPLY(Client &newUser, const std::string &channelName,
                         std::map<std::string, Client> &members,
                         std::map<std::string, Client> &operators) {
    // 1. Broadcast JOIN message to all current members
    std::stringstream joinMessage;
    joinMessage << ":" << newUser.getNickName() << "!" << newUser.getUserName() << "@" << newUser.getHostname()
            << " JOIN :" << channelName << "\r\n";
    for (std::map<std::string, Client>::iterator memberIt = members.begin(); memberIt != members.end(); ++memberIt) {
        Client &recipient = memberIt->second;
        send(recipient.getSocket(), joinMessage.str().c_str(), joinMessage.str().length(), 0);
    }

    // 2. Send RPL_NAMREPLY and RPL_ENDOFNAMES to the new user
    std::stringstream namesReply;
    namesReply << ":" << newUser.getNickName() << " 353 " << newUser.getNickName()
               << " = " << channelName << " :";

    // Add operators with @ prefix
    for (std::map<std::string, Client>::iterator opIt = operators.begin(); opIt != operators.end(); ++opIt) {
        namesReply << "@" << opIt->second.getNickName() << " ";
    }

    // Add regular members
    for (std::map<std::string, Client>::iterator memIt = members.begin(); memIt != members.end(); ++memIt) {
        if (operators.find(memIt->first) == operators.end()) { // Skip if already listed as an operator
            namesReply << memIt->second.getNickName() << " ";
        }
    }

    namesReply << "\r\n";
    send(newUser.getSocket(), namesReply.str().c_str(), namesReply.str().length(), 0);

    // Send RPL_ENDOFNAMES
    std::stringstream endReply;
    endReply << ":" << newUser.getNickName() << " 366 " << newUser.getNickName()
             << " " << channelName << " :End of /NAMES list\r\n";
    send(newUser.getSocket(), endReply.str().c_str(), endReply.str().length(), 0);
}


void Client::ERR_INVITEONLYCHAN(Client &client, const std::string &channel) {
    std::stringstream ss;

    ss << ":" << client.getNickName()
       << " 473 " << channel
       << " " << channel
       << " :Cannot join channel (+i)\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}


void Client::ERR_USERONCHANNEL(Client &client, const std::string &nick, const std::string &channelName)
{
    std::stringstream ss;

    ss << ":" << client.getHostname()
        << " 443 " << channelName
        << " " << nick
        << " :is already on channel\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}


void Client::ERR_CHANOPRIVSNEEDED(Client &client, const std::string &channelName) {
    std::stringstream ss;

    ss << ":" << client.getNickName()
        << " 482 " << channelName
        << " :You're not channel operator\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}


// Custom Replies (using 600+ range)
void Client::RPL_ALREADYOPERATOR(Client &client, const std::string &channelName, const std::string &newOperator, const bool &isOperator) {
    std::stringstream ss;

    ss << ":" << client.getNickName() << " 482 " << channelName
              << " :User " << newOperator << (isOperator ? " is already an operator." : " is not an operator.") << "\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::RPL_CANTKICKSELF(Client &client, const std::string &channelName) {
    std::stringstream ss;

    ss << ":" << client.getNickName()
       << " 701 " << channelName
       << " :You cannot kick yourself.\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::ERR_BADCHANNELKEY_CHANNEL(Client &client, const std::string &channelName) {
    std::stringstream ss;

    ss << ":" << client.getNickName()
       << " 800 " << channelName
       << " :This Channel is not protected with a key or the key you are providing is wrong\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::ERR_UNKNOWNMODE(Client &client, const std::string &channelName, char modechar) {
    std::stringstream ss;

    ss << ":" << client.getNickName()
        << " 472 " << channelName
        << " " << modechar
        << " :is unknown mode char to me\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}


void Client::RPL_INVITESENTTO(Client &client, const std::string &channelName, const std::string &userInvited) {
    std::stringstream ss;

    ss << ":" << client.getNickName()
       << " 702 " << client.getNickName()
       << " " << ":You where invited to " << channelName << " by " << userInvited << "\r\n";

    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::RPL_NEWOPERATOR(Client &newOperator, Client &oldOperator, const std::string &channelName, bool remove, std::map<std::string, Client> &members) {
    std::stringstream ss;

    ss << ":" << oldOperator.getNickName() << "!" << oldOperator.getUserName()
        << "@" << oldOperator.getHostname()
        << " MODE " << channelName
        << (remove ? " -o " : " +o ") << newOperator.getNickName() << "\r\n";


    std::string message = ss.str();

    for (std::map<std::string, Client>::iterator it = members.begin(); it != members.end(); ++it) {
        Client &recipient = it->second;

        if (send(recipient.getSocket(), message.c_str(), message.length(), 0) == -1) {
            std::cerr << "Error sending message to " << recipient.getNickName() << std::endl;
        }
    }
}


void Client::RPL_KICKED(Client &client, const std::string &channelName, std::string &reason) {
    // Ensure reason is not empty
    if (reason.empty() || reason == ":")
        reason = "behave yourself please";

    std::string kickMessage =
        ":" + client.getNickName() +
        " KICK " + channelName +
        " " + client.getNickName() +
        " :" + reason + "\r\n";

    // Send the kick message
    send(client.getSocket(), kickMessage.c_str(), kickMessage.length(), 0);
}


// void Client::MODE_NOTIFY(const std::string &channelName, const std::string &modeChange, const std::string &target, const std::vector<Client *> &channelClients) {
//     std::stringstream ss;

//     ss << ":" << getNickname()
//        << " MODE " << channelName
//        << " " << modeChange;

//     if (!target.empty()) {
//         ss << " " << target;
//     }
//     ss << "\r\n";

//     // Send the message to all clients in the channel
//     for (std::vector<Client *>::const_iterator it = channelClients.begin(); it != channelClients.end(); ++it) {
//         send((*it)->getSocket(), ss.str().c_str(), ss.str().length(), 0);
//     }
// }
