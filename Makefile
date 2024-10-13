NAME = ircserv

CC	    = c++
FLAGS   = -Wall -Wextra -Werror -std=c++98
RM	    = rm -rf

GREEN	= \e[92;5;118m
YELLOW	= \e[93;5;226m
GRAY	= \e[33;2;37m
RESET	= \e[0m

SRC = main.cpp #Server.hpp

HDR = #Server.hpp

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
