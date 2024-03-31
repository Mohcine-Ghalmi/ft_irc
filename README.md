# ft_irc

Core Classes

Server:

Attributes:
    port (int)
    password (string)
    clients (vector of Client pointers)
    channels (map of channel name to Channel object)
    socket_fd (int)
Methods:
    start() - Initialize the server, bind socket, start listening
    accept_connections() - Accept new clients, create Client objects
    handle_io() - Use poll() or equivalent to monitor socket events
    process_client_message(Client* client) - Parse and handle commands
    broadcast_message(message, Channel* channel) - Send messages to channel members


Client:

Attributes:
    socket_fd (int)
    nickname (string)
    username (string)
    realname (string)
    is_operator (bool)
    buffer (string or custom buffer class for incoming data)
Methods:
    send_message(string message)
    join_channel(string channel_name)
    leave_channel(string channel_name)
    set_nickname(string nickname)
    (all methods handling user commands)


Channel:

Attributes:
    name (string)
    topic (string)
    clients (vector of Client pointers)
    modes (string - representing i, t, k, o, l modes)
    key (string - optional password)
    Methods:
    add_client(Client* client)
    remove_client(Client* client)
    set_topic(string topic)
    set_mode(string mode)
    has_mode(char mode_char)

Utility Classes (Optional)

CommandParser: Parses command strings, splitting them into command, arguments, etc.
MessageBuilder: Helps construct correctly formatted IRC messages for sending to clients.
ConfigReader: Loads configuration settings (if you choose to have a config file).
Design Considerations

Data Structures: Think about how to represent messages in transit, as well as internal representations of channel modes and client privileges.
Inheritance: You might consider a base class for users, with a derived class for operators.
Modularity: Aim to keep classes focused on specific responsibilities (think Single Responsibility Principle).

