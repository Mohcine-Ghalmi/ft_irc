NAME = ircserv

CC	    = c++
FLAGS   = -Wall -Wextra -Werror -std=c++98 -g -fsanitize=address
RM	    = rm -rf

GREEN	= \e[92;5;118m
YELLOW	= \e[93;5;226m
GRAY	= \e[33;2;37m
RESET	= \e[0m

SRC = main.cpp SourceFiles/server.cpp SourceFiles/client.cpp SourceFiles/Replies.cpp SourceFiles/Channel.cpp \
	  SourceFiles/Modes.cpp SourceFiles/ClientLogin.cpp SourceFiles/ChannelCommands.cpp

HDR = HeaderFiles/Server.hpp HeaderFiles/Client.hpp HeaderFiles/Replies.hpp HeaderFiles/Channel.hpp

.PHONY: all clean re

all: $(NAME)

$(NAME): $(SRC) $(HDR)
	@printf "$(GRAY) - Compiling $(NAME)... $(RESET)\n"
	@ $(CC) $(FLAGS) $(SRC) -o $(NAME)
	@printf "$(GREEN) - Executable ready.\n$(RESET)"

clean:
	@$(RM) $(NAME)
	@printf "$(YELLOW) - Executable removed.$(RESET)\n"

fclean:
	@$(RM) $(NAME)
	@printf "$(YELLOW) - Executable removed.$(RESET)\n"

re: clean all
