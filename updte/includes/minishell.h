/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 20:22:34 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/12 07:45:04 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>
# include <string.h>
# include <limits.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <errno.h>
# include <signal.h>
# include <readline/readline.h>
# include <readline/history.h>
# include "../libft/libft.h"

extern int g_received_signal;


typedef struct s_env
{
	char			*key;
	char			*value;
	struct s_env	*next;
}	t_env;

typedef enum e_token_type
{
	WORD,
	PIPE,
	REDIRECT_IN,
	REDIRECT_OUT,
	APPEND,
	HEREDOC,
	END,
	ERROR
}	t_token_type;

typedef enum e_quote_type
{
	QUOTE_NONE,
	QUOTE_SINGLE,
	QUOTE_DOUBLE
}	t_quote_type;

typedef struct s_segment
{
	char            *value;
	t_quote_type    quote;
	struct s_segment *next;
}	t_segment;

typedef struct s_token
{
    t_token_type    type;
    t_segment       *segments;
    char            *literal;
    struct s_token  *next;
}   t_token;

typedef struct s_redir
{
    char            *file;
    int             fd;
    int             append_mode;
    struct s_redir  *next;
} t_redir;

typedef struct s_arg
{
    char				*value;
	struct s_segment 	*segments;
    int					was_quoted;
    struct s_arg		*next;
}	t_arg;

typedef struct s_command
{
    t_arg			*args;
    t_redir			*input_redirection;
    t_redir			*output_redirection;
    t_redir			*heredoc;
    struct s_command *next;
} t_command;

typedef struct s_shell
{
	char		*input;
	t_env		*env;
	t_token		*tokens;
	t_command	*commands;
	int			last_exit_status;
}	t_shell;


void    init_shell(t_shell *shell, char **envp);
void    reset_loop(t_shell *shell);
void    cleanup_exit(t_shell *shell);
int     is_exit_command(char *input);

#endif
