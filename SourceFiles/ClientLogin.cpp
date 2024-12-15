#include "../HeaderFiles/Server.hpp"

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
            client.sendReply(464, client);
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
            return true;
    return false;
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

bool Server::processUserCommand(Client &client, const std::string &message) {
    if (this->proccessCommandHelper(message, "USER")) {
        std::stringstream ss(message.substr(5));
        std::string username, hostname, realname;

        ss >> username >> hostname;

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
    client.sendReply(001, client);
    client.sendReply(002, client);
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

