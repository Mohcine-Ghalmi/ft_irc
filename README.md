# ft_irc

sending file using dcc (direct client to client)

/set dcc_download_path

/set dcc_download_path /desired/download/directory


1. Start irssi on Two Terminals (Simulating Two Users)
Run two instances of irssi on the same machine to simulate two users:


        irssi --home ~/irssi_user1

        irssi --home ~/irssi_user2
Connect both instances to your IRC server running on localhost:

        /connect localhost
Join both users to the same channel (e.g., #chat):


        /join #chat
2. Send a File Using /dcc send
From one irssi client (user1), send a file to the other:


        /dcc send user2 /path/to/file.txt
For example:

        /dcc send user2 /tmp/testfile.txt
You will see a message in user2's irssi client indicating a file transfer request:

        DCC SEND request from user1: testfile.txt (127.0.0.1:12345) [1024 bytes].
3. Accept the File on the Receiving Client
In the second irssi instance (user2), accept the file:

        /dcc get user1
The transfer will begin immediately, and you’ll see progress updates in the status window.

