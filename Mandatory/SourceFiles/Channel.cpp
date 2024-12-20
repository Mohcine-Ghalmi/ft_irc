#include "../HeaderFiles/Channel.hpp"

Channel::Channel(const std::string &channelName) : name(channelName), topic(""), inviteOnly(false) \
                                                , topicRestriction(false), keyProtected(false), userLimit(0) {}

const std::string& Channel::getName() const { return name; }
const std::string& Channel::getTopic() const { return topic; }
const std::string& Channel::getKey() const { return key; }
std::map<std::string, Client>& Channel::getInvites() {return invitedUsers;}
bool Channel::isInviteOnly() const { return inviteOnly; }
bool Channel::isTopicRestricted() const { return topicRestriction; }
bool Channel::hasUserLimit() const { return userLimit > 0; }
bool Channel::isKeyProtected() const { return keyProtected; }
int Channel::getUserLimit() const { return userLimit; }
const std::string& Channel::getTopicSetter() const { return topicSetter; }
void Channel::setTopicSetter(const std::string &newTopicSetter) { topicSetter = newTopicSetter; }
bool Channel::isOperator(Client* client) const { return operators.find(client->getNickName()) != operators.end(); }

void Channel::setTopic(const std::string &newTopic) { topic = newTopic; }
void Channel::addOperator(Client* client) { operators.insert(std::pair<std::string, Client>(client->getNickName(), *client));}
void Channel::removeOperator(Client* client) { operators.erase(client->getNickName()); }
void Channel::addMember(Client* client) {members.insert(std::pair<std::string, Client>(client->getNickName(), *client));}
void Channel::removeMember(Client* client) {members.erase(client->getNickName());}
void Channel::addInvitedUser(Client* client) {invitedUsers.insert(std::pair<std::string, Client>(client->getNickName(), *client));}
void Channel::removeInvitedUser(Client* client) {invitedUsers.erase(client->getNickName());}
bool Channel::isMember(Client* client) const { return members.find(client->getNickName()) != members.end(); }
std::map<std::string, Client>& Channel::getMembers()  {return members;}
std::map<std::string, Client>& Channel::getOperators()  {return operators;}
std::map<std::string, Client>& Channel::getInvitedUsers()  {return invitedUsers;}

void Channel::setInviteOnly(bool value) { inviteOnly = value; }
void Channel::setTopicRestriction(bool value) { topicRestriction = value; }
void Channel::setKeyProtection(bool value, const std::string &keyVal) {
    keyProtected = value;
    key = keyVal;
}

void Channel::setUserLimit(int limit) { userLimit = limit; }

Channel::Channel() : name(""), topic(""), inviteOnly(false) \
                                        , topicRestriction(false), keyProtected(false), userLimit(0) {}

Channel::Channel(const Channel &org)  {
    this->name = org.name;
    this->topic = org.topic;
    this->inviteOnly = org.inviteOnly;
    this->topicRestriction = org.topicRestriction;
    this->keyProtected = org.keyProtected;
    this->userLimit = org.userLimit;
}
