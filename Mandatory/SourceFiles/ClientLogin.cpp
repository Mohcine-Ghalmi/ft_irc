#include "../HeaderFiles/Server.hpp"

void sendHellGate(int client_socket) {
    std::string welcomeBanner =
    "╔══════════════════════ Welcome to IRC Hell ═════════════════════╗\n"
    "║                                                                ║\n"
    "║     🌋 CONNECTING TO DAEMON.HELL.IRC.NET [PORT: 6667]          ║\n"
    "║                                                                ║\n"
    "║     ⚠️  WARNING: ALL CONNECTIONS ARE MONITORED BY CERBERUS      ║\n"
    "║                                                                ║\n"
    "║     📜 Server Rules:                                           ║\n"
    "║        • No flooding/spamming (auto-/kick + ban)               ║\n"
    "║        • No bot abuse (permanent ban)                          ║\n"
    "║        • Respect the Channel Operators                         ║\n"
    "║                                                                ║\n"
    "║    🔥 Available Commands:                                      ║\n"
    "║        /join #hellgate  - Main discussion channel              ║\n"
    "║        /msg CerberusBot - Server assistance                    ║\n"
    "║                                                                ║\n"
    "║     💀 Current Users Online: 666                               ║\n"
    "║     🕯️ Server Uptime: 666 days, 6 hours                         ║\n"
    "║                                                                ║\n"
    "║                                                                ║\n"
    "╚════════════════════ ESTABLISHED CONNECTION ════════════════════╝\n";


    send(client_socket, welcomeBanner.c_str(), welcomeBanner.length(), 0);
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
        if (nickname[i] == '"' || nickname[i] == ' ' || nickname[i] == ',' || nickname[i] == '*' || nickname[i] == '?' || nickname[i] == '!' || nickname[i] == '@')
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
    if (!this->proccessCommandHelper(message, "USER"))
        return false;

    size_t colonPos = message.find(':');
    std::string paramsPart = message.substr(5, colonPos - 5); // Parameters before ':'
    std::string realnamePart = (colonPos != std::string::npos) ? message.substr(colonPos + 1) : "";

    if (!realnamePart.empty() && (realnamePart.back() == '\r' || realnamePart.back() == '\n'))
        realnamePart.pop_back();

    std::istringstream ss(paramsPart);
    std::vector<std::string> params;
    std::string param;
    while (ss >> param)
        if (params.size() < 4)
            params.push_back(param);

    if (params.size() == 4 && colonPos == std::string::npos)
        realnamePart = params[3];
    else if (params.size() < 3) {
        client.ERR_NEEDMOREPARAMS(client, "USER");
        LOG_ERROR("Not enough parameters in USER command");
        return false;
    } else if (params.size() >= 4 && colonPos != std::string::npos)
        realnamePart = (colonPos != std::string::npos) ? realnamePart : params[3];

    std::string username = params[0];
    std::string hostname = params[1];
    std::string servername = params[2];

    if (username.empty()) {
        client.ERR_NEEDMOREPARAMS(client, "USER");
        LOG_ERROR("Username is empty");
        return false;
    }

    if (realnamePart.empty()) {
        client.ERR_NEEDMOREPARAMS(client, "USER");
        LOG_ERROR("Realname is missing in USER command");
        return false;
    }

    client.setUserName(username);
    client.setHostName(hostname);
    client.setRealName(realnamePart);

    LOG_SERVER("Processed USER command for client: username=" + username + ", hostname=" + hostname + ", servername=" + servername + ", realname=" + realnamePart);
    return true;
}


int findLoginInfo(std::string messag) {
    if (messag.find("NICK"))
        return 1;
    if (messag.find("USER"))
        return (1);
    if (messag.find("PASS"))
        return (1);
    return 0;
}

bool Server::setUpClient(Client &client, int &flag) {
    std::vector<std::string> messages = splitMessages(client.getBuffer());

    if (client.isAuthenticated())
        return false;

    for (std::vector<std::string>::size_type i = 0; i < messages.size(); ) {
        const std::string &message = messages[i];
        flag += findLoginInfo(message);
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
    client.sendReply(003, client);
    client.sendReply(004, client);
    client.sendReply(005, client);
    client.authenticate();
    return true;
}

void sendCapResponse(int clientSocket) {
    std::string capResponse = ":localhost CAP * LS :\r\n"; // sending Empty CAP List (do not support any advanced features)

    send(clientSocket, capResponse.c_str(), capResponse.size(), 0);
    LOG_SERVER("Sent CAP LS response to client");
}
