NAME        = minishell
CC          = cc
CFLAGS      = -Wall -Wextra -Werror -g3
LIBFT_DIR   = ./libft
LIBFT       = $(LIBFT_DIR)/libft.a
INCLUDES    = -I./includes -I. -I$(LIBFT_DIR)

SRCS_MAIN   = srcs/main.c

# Parsing Sources
SRCS_PARSING = srcs/parsing/lexer/lexer.c \
              srcs/parsing/lexer/lexer_utils.c \
              srcs/parsing/lexer/lexer_tokens.c \
              srcs/parsing/lexer/lexer_segments.c \
              srcs/parsing/parser/parser.c \
              srcs/parsing/parser/parser_utils.c \
              srcs/parsing/parser/parser_redir.c \
              srcs/parsing/expander/expander.c \
              srcs/parsing/expander/expander_utils.c \
              srcs/parsing/expander/expander_process.c \
              srcs/parsing/utils/ft_strcmp.c \
              srcs/parsing/utils/ft_error.c \
              srcs/parsing/utils/ft_strndup.c

# Env Sources (On retire env_to_array.c car utils_env.c gere deja ca)
SRCS_ENV     = srcs/parsing/env/env_dup_utils.c \
              srcs/parsing/env/env_duplicate.c \
              srcs/parsing/env/env_to_array.c

# Exec Sources
SRCS_EXEC    = srcs/exec/executor/exec.c \
              srcs/exec/executor/exec_child.c \
              srcs/exec/executor/exec_cmd.c \
              srcs/exec/builtins/builtins.c \
              srcs/exec/builtins/ft_builtins.c \
              srcs/exec/builtins/ft_export.c \
              srcs/exec/builtins/ft_unset.c \
              srcs/exec/redirection/handle_redirection.c \
              srcs/exec/redirection/redir_utils.c \
              srcs/exec/redirection/heredoc.c \
              srcs/exec/utils/utils_env.c \
              srcs/exec/utils/utils_free.c \
              srcs/exec/utils/get_next_line.c \
              srcs/exec/utils/args_conversion.c \
              srcs/exec/signal/signal.c

SRCS        = $(SRCS_MAIN) $(SRCS_PARSING) $(SRCS_ENV) $(SRCS_EXEC)
OBJS        = $(SRCS:.c=.o)
LIBS        = -L$(LIBFT_DIR) -lft -lreadline

all: $(NAME)

$(NAME): $(LIBFT) $(OBJS)
	@echo "Compiling Minishell..."
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(NAME)
	@echo "✅ Minishell ready!"

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIBFT):
	@echo "Compiling Libft..."
	@make -C $(LIBFT_DIR)

clean:
	@echo "Cleaning objects..."
	@rm -f $(OBJS)
	@make clean -C $(LIBFT_DIR)

fclean: clean
	@echo "Cleaning executables..."
	@rm -f $(NAME)
	@make fclean -C $(LIBFT_DIR)

re: fclean all

.PHONY: all clean fclean re