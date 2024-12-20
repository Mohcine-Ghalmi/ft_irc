#pragma once
#include "Client.hpp"
#include "Server.hpp"

class Channel {
    private:
        std::string name;
        std::string topic;
        std::string topicSetter;
        std::map<std::string, Client> members;
        std::map<std::string, Client> operators;
        std::map<std::string, Client> invitedUsers;
        bool inviteOnly;
        bool topicRestriction;
        bool keyProtected;
        std::string key;
        int userLimit;

   public:
        Channel(const std::string &channelName);
        Channel();
        Channel(const Channel &org);


        const std::string& getName() const;
        const std::string& getTopic() const;
        const std::string& getTopicSetter() const;
        const std::string& getKey() const;
        bool isInviteOnly() const;
        bool isTopicRestricted() const;
        bool hasUserLimit() const;
        bool isKeyProtected() const;
        int getUserLimit() const;
        bool isOperator(Client* client) const;

        void setTopic(const std::string &newTopic);
        void setTopicSetter(const std::string &newTopicSetter);
        void addOperator(Client* client);
        void removeOperator(Client* client);
        void addMember(Client* client);
        void removeMember(Client* client);
        bool isMember(Client* client) const;

        void addInvitedUser(Client* client);
        void removeInvitedUser(Client* client);
        std::map<std::string, Client>& getInvites();
        std::map<std::string, Client>& getMembers();
        std::map<std::string, Client>& getOperators();
        std::map<std::string, Client>& getInvitedUsers();

        void setInviteOnly(bool value);
        void setTopicRestriction(bool value);
        void setKeyProtection(bool value, const std::string &keyVal);
        void setUserLimit(int limit);
};
