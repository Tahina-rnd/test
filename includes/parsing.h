/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:22:34 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/05 11:00:36 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSING_H
# define PARSING_H

# include "minishell.h"

// ========== UTILS ==========
int			ft_strcmp(char *s1, char *s2);
char		*ft_strndup(const char *s, size_t n);

// ========== TOKENIZER ==========
char		*extract_word(char *s, int *i);
int			check_tokens(t_token *tokens);
t_token		*create_token(t_token_type type, char *value);
void		add_token_back(t_token **head, t_token *new);
void		free_tokens(t_token *tokens);
int			ft_isspace(char c);
int			is_operator(char c);
int			is_quote(char c);
t_token		*tokenize(char *input);

// ========== EXPANSION ==========
char		*get_env_value_copy(t_shell *shell, char *name);
char		*extract_var_name_braces(char *str, int *i);
char		*extract_var_name_simple(char *str, int *i);
char		*handle_exit_code_expansion(t_shell *shell, char *str, int *i);
char		*str_append_char(char *str, char c);
char		*str_append_str(char *src, char *dest);
void		clean_empty_tokens(t_token **tokens);
void		expand_tokens(t_token *tokens, t_shell *shell);
char		*expand_string(char *str, t_shell *shell);
int			is_valid_var_char(char c);

// ========== REDIR UTILS (NOUVEAU) ==========
t_redir		*create_redir_node(char *file);
void		add_redir_back(t_redir **head, t_redir *new_redir);
void		free_redir_list(t_redir *redir_list);
int			count_redir_nodes(t_redir *redir_list);
t_redir		*get_last_redir(t_redir *redir_list);
t_redir *create_append_redir_node(char *file);


// ========== PARSER ==========
int			add_argument(t_command *cmd, char *arg);
int			cleanup_and_return(t_command **cmd_list, t_command *current_cmd);
void		free_cmds(t_command *cmds);
int			handle_input_redirection(t_command *cmd, t_token **current);
int			handle_output_redirection(t_command *cmd, t_token **current);
int			handle_append_redirection(t_command *cmd, t_token **current);
int			handle_heredoc(t_command *cmd, t_token **current);
int			finalize_command(t_command **command_list, t_command **current_cmd);
t_command	*parse(t_token *tokens);

// ========== ENV ==========
char		*env_get_value(t_env *env_list, char *key);
void		free_env_list(t_env *env_list);
t_env		*create_env_node(const char *key, const char *value);
void		add_env_node_back(t_env **head, t_env *new_node);
t_env		*dup_env(char **envp);
char		**env_to_array(t_env *env);
void		free_env_array(char **env_array);

#endif
