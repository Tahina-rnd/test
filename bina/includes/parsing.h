/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:22:34 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/09 12:07:30 by miokrako         ###   ########.fr       */
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
char		*env_get_value(t_env *env_list, char *key);
void		free_env_list(t_env *env_list);
t_env		*create_env_node(const char *key, const char *value);
void		add_env_node_back(t_env **head, t_env *new_node);
t_env		*dup_env(char **envp);
char		**env_to_array(t_env *env);
void		free_env_array(char **env_array);

// LEXER
t_token		*lexer(char *input);
void		skip_spaces(char *input, int *i);
int			is_redirection(char *input, int i);
void		handle_redir(char *input, int *i, t_token **tokens);
void		handle_word(char *input, int *i, t_token **tokens);
char		*extract_word(char *input, int *i, int *was_quoted);
int			handle_quotes(char *input, int *i, char quote, char **result);
void		add_token(t_token **tokens, t_token_type type, char *value);
t_token		*create_token(t_token_type type, char *value);
void		free_tokens(t_token *tokens);
t_token		*get_last_token(t_token *tokens);
int			is_quote(char c);
int			is_special_char(char c);

// PARSER
t_command	*parser(t_token *tokens);
t_command	*new_command(void);
int			parse_simple_command(t_token **tokens, t_command *cmd);
void		append_command(t_command **cmds, t_command *new_cmd);
int			syntax_error(char *token);
int			validate_tokens(t_token *tokens);
void		free_commands(t_command *commands);
void		free_command(t_command *cmd);
void		free_redirections(t_redir *redir);
void		free_args(t_arg *args, int count);
int			count_args(t_command *cmd);
t_command	*get_last_command(t_command *cmds);
int			handle_input_redir(t_token **tokens, t_command *cmd);
int			handle_output_redir(t_token **tokens, t_command *cmd);
int			handle_heredoc(t_token **tokens, t_command *cmd);
int			handle_append(t_token **tokens, t_command *cmd);
void		add_input_redir(t_command *cmd, char *file);
void		add_output_redir(t_command *cmd, char *file, int append);
void		add_heredoc(t_command *cmd, char *delimiter);
int			add_arg_to_command(t_command *cmd, t_token *token);
t_arg		*create_args_array(t_command *cmd, int count);

// EXPANDER
void		expander(t_command *commands, t_env *env, int last_exit_status);
void		expand_command(t_command *cmd, t_env *env, int last_exit_status);
void		expand_args(t_command *cmd, t_env *env, int last_exit_status);
void		expand_redirections(t_command *cmd, t_env *env, int exit_status);
char		*expand_variables(char *str, t_env *env, int last_exit_status);
char		*get_env_value(t_env *env, char *key);
char		*expand_exit_status(int status);
int			is_valid_var_char(char c);
char		**word_split(char *str, t_env *env);
int			is_ifs_char(char c, char *ifs);
char		*get_ifs(t_env *env);
int			count_words(char *str, char *ifs);
char		*extract_word_split(char *str, int *i, char *ifs);
void		push_arg(t_arg **args, int *count, char *value, int was_quoted);
void		replace_args(t_command *cmd, t_arg *new_args, int count);
int			is_ambiguous_redirect(char *expanded, int was_quoted);
void		free_split(char **split);
char		*expand_redir_file(char *file, t_env *env, int exit_status);
char		*extract_var_name(char *str, int *i);
char		*append_string(char *result, char *str);

#endif
