NAME = ircserv
NAMEBONUS = ircserv_bonus

CC	    = c++
FLAGS   = -Wall -Wextra -Werror -std=c++98 -lcurl #-g -fsanitize=address
RM	    = rm -rf

GREEN	= \e[92;5;118m
YELLOW	= \e[93;5;226m
GRAY	= \e[33;2;37m
RESET	= \e[0m

SRC_MANDA = mandatory/main.cpp mandatory/SourceFiles/server.cpp mandatory/SourceFiles/client.cpp mandatory/SourceFiles/Replies.cpp mandatory/SourceFiles/Channel.cpp \
	  mandatory/SourceFiles/Modes.cpp mandatory/SourceFiles/ClientLogin.cpp mandatory/SourceFiles/ChannelCommands.cpp

SRC_BONUS = Bonus/main.cpp Bonus/SourceFiles/server.cpp Bonus/SourceFiles/client.cpp Bonus/SourceFiles/Replies.cpp Bonus/SourceFiles/Channel.cpp \
	  Bonus/SourceFiles/Modes.cpp Bonus/SourceFiles/ClientLogin.cpp Bonus/SourceFiles/ChannelCommands.cpp Bonus/SourceFiles/Bot.cpp

HDR_MANDA = mandatory/HeaderFiles/Server.hpp mandatory/HeaderFiles/Client.hpp mandatory/HeaderFiles/Replies.hpp mandatory/HeaderFiles/Channel.hpp

HDR_BONUS = Bonus/HeaderFiles/Server.hpp Bonus/HeaderFiles/Client.hpp Bonus/HeaderFiles/Replies.hpp Bonus/HeaderFiles/Channel.hpp

.PHONY: all clean re

all: $(NAME)

$(NAME): $(SRC_MANDA) $(HDR_MANDA)
	@printf "$(GRAY) - Compiling $(NAME)... $(RESET)\n"
	@ $(CC) $(FLAGS) $(SRC_MANDA) -o $(NAME)
	@printf "$(GREEN) - Executable ready.\n$(RESET)"

bonus: $(NAMEBONUS)
$(NAMEBONUS): $(SRC_BONUS) $(HDR_BONUS)
	@printf "$(GRAY) - Compiling $(NAMEBONUS)... $(RESET)\n"
	@ $(CC) $(FLAGS) $(SRC_BONUS) -o $(NAMEBONUS)
	@printf "$(GREEN) - Executable ready.\n$(RESET)"

clean:
	@$(RM) $(NAME) $(NAMEBONUS)
	@printf "$(YELLOW) - Executable removed.$(RESET)\n"

fclean:
	@$(RM) $(NAME) $(NAMEBONUS)
	@printf "$(YELLOW) - Executable removed.$(RESET)\n"

re: clean all
