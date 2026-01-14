/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 11:01:22 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/14 16:33:36 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXEC_H
# define EXEC_H
# define BUFFER_SIZE 1024
# define MAX_HEREDOCS 16

# include "parsing.h"

void	child_process(t_command *cmd, t_shell *shell, int prev[2], int curr[2]);
void	wait_all_children(pid_t last_pid, t_shell *shell);
char	*get_path(t_env *env, char *cmd);
void	executor(t_shell *shell);
void	exec_simple_cmd_with_array(t_command *cmd, t_env *env,
			char **args_array, t_shell *shell);
void	init_env(t_shell *shell, char **envp);
char	*get_env_value(t_env *env, char *key);
void	update_env_var(t_env *env, char *key, char *new_value);
char	**env_to_tab(t_env *env);
int		count_env_vars(t_env *env);
int		handle_redirections(t_command *cmd);
t_redir	*get_last_redir(t_redir *redir_list);
int		create_heredoc_file(char *delimiter, char *filename);
int		process_heredocs(t_shell *shell);
void	cleanup_heredocs(t_shell *shell);
void	expand_heredoc_line(char *line, int fd, t_shell *shell);
int		is_builtin(char *cmd);
int		execute_builtin(t_command *cmd, t_shell *shell);
int		check_parent_builtin(char *cmd, t_shell *shell);
void	exec_builtin_parent(t_command *cmd, t_shell *shell);
int		builtin_echo(char **args);
int	builtin_pwd(t_shell *shell);
int		builtin_exit(char **args, t_shell *shell);
int		builtin_env(t_env *env);
int		builtin_cd(char **args, t_env *env);
int		ft_export(t_shell *shell, char **args);
int		ft_unset(t_shell *shell, char **args);
void	sig_handler_prompt(int sig);
void	sig_handler_exec(int sig);
void	sig_handler_heredoc(int sig);
void	setup_prompt_signal(void);
void	setup_exec_signals(void);
void	setup_child_signals(void);
void	setup_heredoc_signals(void);
void	free_tab(char **tab);
void	free_tab_partial(char **tab, int count);
void	free_env(t_env *env);
void	free_commands(t_command *commands);
void	cleanup_shell(t_shell *shell);
void	cleanup_child(t_shell *shell);
char	*get_next_line(int fd);
void	exp_add_env_node_back(t_env **head, t_env *new_node);
int		is_valid_exit_arg(char *str);
char	*build_absolute_path(char *current_pwd, char *dir);
char	*normalize_path(char *path);
void	update_pwd_vars(t_env *env, char *old_pwd, char *new_dir);
t_env	*new_env_node_kv(char *key, char *value);
t_env	*new_env_node(char *str);
void	add_env_node(t_env **env, t_env *node);
void	init_pwd(t_env **env);
void	init_shlvl(t_env **env);
void	init_underscore(t_env **env);
void	expand_heredoc_line(char *line, int fd, t_shell *shell);
int		create_heredoc_file(char *delimiter, char *filename);
void	expand_heredoc_content(t_shell *shell, char *tmpfile);
int		child_heredoc_process(char *clean_delim, char *tmpfile,
			int do_expand, t_shell *shell);
int		parent_wait_heredoc(pid_t pid, t_shell *shell);
char	*generate_tmpfile_name(char *delimiter, int index);
int		process_single_heredoc_node(t_redir *heredoc_node,
			t_shell *shell, int index);
int		process_cmd_heredocs(t_command *cmd, t_shell *shell);
int		should_expand(char *delimiter);
char	*clean_delimiter(char *delimiter);
t_env	*exp_create_env_node(const char *key, const char *value);
int		is_valid_identifier(const char *str);
char	*extract_key(const char *arg);
char	*extract_value(const char *arg);
int		update_or_add_env(t_shell *shell, char *key, char *value);
int		open_input_file(t_redir *last_redir);
int		open_output_file(t_redir *redir);
void	setup_pipes(t_command *cmd, int prev_pipe[2], int curr_pipe[2]);
void	update_exit_status(t_shell *shell, int last_status, int sig_int,
			int sig_quit);
void	check_special_cases(char *cmd, t_shell *shell, char **args_array);
int		prepare_pipe(t_command *cmd, int prev_pipe[2], int curr_pipe[2]);
void	handle_parent_pipes(t_command *cmd, int prev[2], int curr[2]);
int		fork_handler(t_shell *sh, t_command *cmd, int prev[2], int curr[2]);
char	*try_paths(char **paths, char *cmd);
int		ft_atoll_safe(const char *str, long long *result);
char	**args_to_array(t_arg *args);
void	init_shell(t_shell *shell, char **envp);
void	reset_loop(t_shell *shell);
void	cleanup_exit(t_shell *shell);
int		is_exit_command(char *input);
void	handle_command_not_found(char *cmd, char **args, t_shell *shell);
void	init_oldpwd(t_env **env);
int		is_valid_identifier_export(const char *str);
int		handle_export_error(char *arg);
void	update_pwd_vars_unnormalized(t_env *env, char *old_pwd, char *new_dir);
void	export_onl(t_shell *shell);
char	**env_to_array(t_env *env);

#endif
