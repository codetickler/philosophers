NAME = philo

FLAGS = -Wall -Wextra -Werror -g -pthread -fsanitize=thread

SRC = main.c parse.c

OBJ_DIR = obj

OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.c=.o))

all: $(NAME)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	@cc $(FLAGS) -c $< -o $@

$(NAME): $(OBJ)
	@cc $(FLAGS) $(OBJ) -o $(NAME)
	@echo philosophers compiled!

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all