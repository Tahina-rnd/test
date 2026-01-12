/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:22:34 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/12 07:58:57 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSING_H
# define PARSING_H

# include "minishell.h"

// UTILS
int			ft_strcmp(char *s1, char *s2);
char		*ft_strndup(const char *s, size_t n);
void		ft_error(char *str);

// ENV
char		*get_env_value(t_env *env_list, char *key);
void		free_env_list(t_env *env_list);
t_env		*create_env_node(const char *key, const char *value);
void		add_env_node_back(t_env **head, t_env *new_node);
t_env		*dup_env(char **envp);

// LEXER
t_token		*lexer(char *input);
int			is_whitespace(char c);
int			is_operator(char c);
void		token_add_back(t_token **lst, t_token *new);
void		segment_add_back(t_segment **lst, t_segment *new);
void		free_tokens(t_token *tokens);
t_token		*create_token(t_token_type type, char *literal);
t_token		*handle_operator(char *input, int *i);
t_token		*handle_word(char *input, int *i);
t_segment	*create_segment(char *value, t_quote_type type);
int			extract_segment(t_token *token, char *input, int *i);

// PARSER
t_command	*parser(t_token *tokens);
t_command	*create_command(void);
void		cmd_add_back(t_command **lst, t_command *new);
int			add_arg(t_command *cmd, t_token *token);
void		free_commands(t_command *cmds);
void		free_segments(t_segment *seg);
int			parse_redir(t_command *cmd, t_token **curr_token);

// EXPANDER
void		expander(t_shell *shell, t_command *cmd);
char		*ft_strjoin_free(char *s1, char *s2);
char		*expand_text(char *str, t_shell *shell);

// other
void		free_command(t_command *cmd);

#endif
