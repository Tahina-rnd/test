NAME = minishell
CC = cc
CFLAGS = -Wall -Wextra -Werror -g3
RM = rm -rf

SRCS =  srcs/main.c \
		srcs/parsing/utils/ft_strcmp.c \
		srcs/parsing/utils/ft_strndup.c \
		srcs/parsing/tokenizer/extract_word.c \
		srcs/parsing/tokenizer/token_checker.c \
		srcs/parsing/tokenizer/token.c \
		srcs/parsing/tokenizer/tokenizer_utils.c \
		srcs/parsing/tokenizer/tokenizer.c \
		srcs/parsing/expansion/expand_env.c \
		srcs/parsing/expansion/expand_exit.c \
		srcs/parsing/expansion/expand_utils.c \
		srcs/parsing/expansion/expander.c \
		srcs/parsing/expansion/word_split.c \
		srcs/parsing/expansion/expand_commands.c \
		srcs/parsing/parser/add_args.c \
		srcs/parsing/parser/free_cmd.c \
		srcs/parsing/parser/handle_redir.c \
		srcs/parsing/parser/redir_utils.c \
		srcs/parsing/parser/parser.c \
		srcs/parsing/parser/ambiguous_redir.c \
		srcs/parsing/parser/check_ambiguous.c \
		srcs/parsing/env/env_dup_utils.c \
		srcs/parsing/env/env_duplicate.c \
		srcs/parsing/env/env_to_array.c \
		srcs/exec/executor/exec.c \
		srcs/exec/builtins/builtins.c\
		srcs/exec/builtins/ft_builtins.c\
		srcs/exec/redirection/handle_redirection.c \
		srcs/exec/utils/utils_env.c\
		srcs/exec/utils/utils_free.c\
		srcs/exec/signal/signal.c \
		srcs/exec/redirection/heredoc.c \
		srcs/exec/utils/get_next_line.c \
		srcs/exec/builtins/ft_export.c \
		srcs/exec/builtins/ft_unset.c

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
