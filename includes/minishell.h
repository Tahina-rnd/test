/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 20:22:34 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/08 05:26:22 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>
# include <string.h>
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

typedef struct s_token
{
	t_token_type	type;
	char			*value;
	int				was_quoted;
	struct s_token	*next;
}	t_token;

typedef struct s_redir
{
    char            *file;
    int             fd;
    int             append_mode;  // 0 = >, 1 = >>
    struct s_redir  *next;
} t_redir;

typedef struct s_command
{
    char            **args;
	int             *args_was_quoted;
    t_redir         *input_redirection;
    t_redir         *output_redirection;  // Liste unique pour > et >>
    t_redir         *heredoc;
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

#endif
