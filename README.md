# ft_irc

## issue to handel 

        18:28 -!- Irssi: Connection to localhost established
        18:28 -!- 🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥
        18:28 -!-     🔥          Welcome to       🔥
        18:28 -!-     🔥            Hell!          🔥
        18:28 -!-     🔥                           🔥
        18:28 -!-     🔥   👿  Beware of the       🔥
        18:28 -!-     🔥   Darkness and Flames!    🔥
        18:28 -!-     🔥                           🔥
        18:28 -!- 🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥
        18:28 -!- Capabilities supported:
        18:28 !localhost Please provide your nickname using the NICK command.
        18:28 !localhost Please provide a valid NICK command.
        18:28 !localhost Please provide your nickname using the NICK command.
        18:28 !localhost Please provide a valid NICK command.
        18:28 !localhost Please provide your nickname using the NICK command.
        18:28 !localhost Please provide a valid NICK command.
        18:28 !localhost Please provide your nickname using the NICK command.
        18:28 !localhost Please provide a valid NICK command.
        18:28 !localhost Please provide your nickname using the NICK command.
        18:28 !localhost your nickname is settled

after logged in the irssi client send multi text like showing bellow  making issues for nickname validating

        Sent CAP LS response to client
        NICK mghalmi
        USER mghalmi mghalmi localhost :Mohcine Ghalmi

still need in  this function to set the nickname in irssi server 
    void sendHellGate(int client_socket, std::string name)


also irssi can't accept //PASS command need to be fixed