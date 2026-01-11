/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 11:01:22 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 13:55:13 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXEC_H
# define EXEC_H
# define BUFFER_SIZE 1024
# include "parsing.h"

// ========== PATH & EXECUTION ==========

void	child_process(t_command *cmd, t_shell *shell, int prev[2], int curr[2]);
void	wait_all_children(pid_t last_pid, t_shell *shell);
char	*get_path(t_env *env, char *cmd);
// void	exec_simple_cmd(t_command *cmd, t_env *env, t_shell *shell);
void	executor(t_shell *shell);
void	exec_simple_cmd_with_array(t_command *cmd, t_env *env, char **args_array, t_shell *shell);
// ========== ENVIRONMENT ==========
void	init_env(t_shell *shell, char **envp); //tsy miasa
void	update_env_var(t_env *env, char *key, char *new_value);
char	**env_to_tab(t_env *env);

// ========== REDIRECTIONS ==========
int		handle_redirections(t_command *cmd);
t_redir	*get_last_redir(t_redir *redir_list);

// ========== HEREDOC ==========
int		create_heredoc_file(char *delimiter, char *filename);
int		process_heredocs(t_shell *shell);
void	cleanup_heredocs(t_shell *shell);
void	expand_heredoc_line(char *line, int fd, t_shell *shell);

// ========== BUILTINS ==========
int		is_builtin(char *cmd);
int		execute_builtin(t_command *cmd, t_shell *shell);
int		check_parent_builtin(char *cmd, t_shell *shell);
void	exec_builtin_parent(t_command *cmd, t_shell *shell);
int		builtin_echo(char **args);
int		builtin_pwd(void);
int		builtin_exit(char **args, t_shell *shell);
int		builtin_env(t_env *env);
int		builtin_cd(char **args, t_env *env);
int		ft_export(t_shell *shell, char **args);
int		ft_unset(t_shell *shell, char **args);

// ========== SIGNALS ==========
void	sig_handler_prompt(int sig);
void	sig_handler_exec(int sig);
void	sig_handler_heredoc(int sig);
void	setup_prompt_signal(void);
void	setup_exec_signals(void);
void	setup_child_signals(void);
void	setup_heredoc_signals(void);

// ========== MEMORY MANAGEMENT ==========
void	free_tab(char **tab);
void	free_tab_partial(char **tab, int count);
void	free_env(t_env *env);
// void	free_command(t_command *cmd);
void	free_commands(t_command *commands);
void	cleanup_shell(t_shell *shell);
void	cleanup_child(t_shell *shell);

// ========== UTILITY ==========
char	*get_next_line(int fd);
void	exp_add_env_node_back(t_env **head, t_env *new_node);

#endif
