#pragma once
#include <string>
#include <vector>
#include "Client.hpp"
#include "Server.hpp"
#include <set>

class Channel {
    private:
        std::string name;
        std::string topic;
        std::string topicSetter;
        std::map<std::string, Client> members;       // A set of pointers to clients in this channel
        std::map<std::string, Client> operators;     // Channel operators
        std::map<std::string, Client> invitedUsers;     // Channel invited users
        bool inviteOnly;                 // i mode
        bool topicRestriction;           // t mode
        bool keyProtected;               // k mode
        std::string key;                 // Channel key (password)
        int userLimit;                   // l mode, 0 means no limit

   public:
        Channel(const std::string &channelName);
        Channel();
        Channel(const Channel &org);


        // Basic getter/setter functions
        const std::string& getName() const;
        const std::string& getTopic() const;
        const std::string& getTopicSetter() const;
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
        //
        void addInvitedUser(Client* client);
        void removeInvitedUser(Client* client);
        std::map<std::string, Client>& getMembers();
        std::map<std::string, Client>& getOperators();
        std::map<std::string, Client>& getInvitedUsers();

        void setInviteOnly(bool value);
        void setTopicRestriction(bool value);
        void setKeyProtection(bool value, const std::string &keyVal);
        void setUserLimit(int limit);
};
