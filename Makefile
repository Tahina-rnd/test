NAME = minishell
CC = cc
CFLAGS = -Wall -Wextra -Werror -g
RM = rm -rf

SRCS =  srcs/main.c \
		srcs/parsing/lexer/lexer.c \
		srcs/parsing/lexer/lexer_utils.c \
		srcs/parsing/lexer/lexer_word.c \
		srcs/parsing/lexer/handle_redir.c \
		srcs/parsing/parser/parser.c \
		srcs/parsing/parser/parser_utils1.c \
		srcs/parsing/parser/parser_utils2.c \
		srcs/parsing/parser/parser_args.c \
		srcs/parsing/parser/parser_redir1.c \
		srcs/parsing/parser/parser_redir2.c \
		srcs/parsing/expander/expander.c \
		srcs/parsing/expander/expander_var1.c \
		srcs/parsing/expander/expander_var2.c \
		srcs/parsing/expander/expander_split.c \
		srcs/parsing/expander/expander_utils.c \
		srcs/parsing/env/env_duplicate.c \
		srcs/parsing/env/env_dup_utils.c \
		srcs/parsing/env/env_to_array.c \
		srcs/parsing/utils/ft_strcmp.c \
		srcs/parsing/utils/ft_strndup.c \
		srcs/parsing/utils/ft_error.c\
		srcs/exec/executor/exec.c \
		srcs/exec/executor/exec_child.c \
		srcs/exec/executor/exec_cmd.c \
		srcs/exec/builtins/builtins.c \
		srcs/exec/builtins/ft_builtins.c \
		srcs/exec/builtins/ft_export.c \
		srcs/exec/builtins/ft_unset.c \
		srcs/exec/redirection/handle_redirection.c \
		srcs/exec/redirection/redir_utils.c \
		srcs/exec/redirection/heredoc.c \
		srcs/exec/signal/signal.c \
		srcs/exec/utils/utils_env.c \
		srcs/exec/utils/utils_free.c \
		srcs/exec/utils/get_next_line.c

OBJS = $(SRCS:.c=.o)

LIBFT_DIR = libft
LIBFT = $(LIBFT_DIR)/libft.a

all: $(NAME)

$(NAME): $(LIBFT) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBFT) -lreadline -o $(NAME)

$(LIBFT):
	make -C $(LIBFT_DIR)

%.o: %.c
	$(CC) $(CFLAGS) -I./includes -I$(LIBFT_DIR) -c $< -o $@

clean:
	$(RM) $(OBJS)
	make clean -C $(LIBFT_DIR)

fclean: clean
	$(RM) $(NAME)
	make fclean -C $(LIBFT_DIR)

re: fclean all

.PHONY: all clean fclean re
