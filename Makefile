NAME = webserver

CC = g++

CFLAGS = -Wall -Werror -Wextra -std=c++98

SRC_DIR =	src/
OBJ_DIR =	obj/

HEADER_FILES = inc/Server.hpp inc/Socket.hpp inc/Client.hpp inc/Config.hpp

SRC =	src/webserv.cpp\
		src/Socket.cpp\
		src/Server.cpp\
		src/HandleEvents.cpp\
		src/utils.cpp\
		src/Client.cpp\
		src/ParseRequests.cpp\
		src/Config.cpp\
		src/AutoIndex.cpp\
		src/GetResponse.cpp\

OBJ = $(SRC:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)

all:
	@$(MAKE) $(OBJ_DIR) $(NAME) -j4

$(NAME): $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o $(NAME)

$(OBJ): $(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(HEADER_FILES)
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

$(OBJ_DIR):
	mkdir obj

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(NAME)
	@rm -f uploads/*
	@rm -f logs/*

re: fclean all

.PHONY: all clean fclean re
