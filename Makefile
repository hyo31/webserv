NAME = webserver

CC = g++

CFLAGS = -Wall -Werror -Wextra -std=c++98

SRC_DIR =	src/
OBJ_DIR =	obj/

<<<<<<< HEAD
HEADER_FILES = inc/Server.hpp
=======
HEADER_FILES = inc/Server.hpp inc/Socket.hpp
>>>>>>> e7fc36f9fd757ace779fc37ca88d957aefede42d

SRC =	src/webserv.cpp\
		src/Socket.cpp\
		src/Server.cpp

OBJ = $(SRC:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)

all: $(OBJ_DIR) $(NAME)

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

re: fclean all

.PHONY: all clean fclean re
