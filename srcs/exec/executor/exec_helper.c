/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_helper.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 08:12:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/15 11:52:24 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

void	update_exit_status(t_shell *shell, int last_status,
	int sig_int, int sig_quit)
{
	if (sig_int)
		write(1, "\n", 1);
	if (sig_quit)
	{
		shell->last_exit_status = 131;
		ft_putstr_fd("Quit (core dumped)\n", 2);
		return ;
	}
	if (WIFEXITED(last_status))
	{
		shell->last_exit_status = WEXITSTATUS(last_status);
	}
	else if (WIFSIGNALED(last_status))
	{
		shell->last_exit_status = 128 + WTERMSIG(last_status);
	}
}

void	setup_pipes(t_command *cmd, int prev_pipe[2], int curr_pipe[2])
{
	if (prev_pipe[0] != -1)
	{
		dup2(prev_pipe[0], STDIN_FILENO);
		close(prev_pipe[0]);
		if (prev_pipe[1] != -1)
			close(prev_pipe[1]);
	}
	if (cmd->next)
	{
		if (curr_pipe[0] != -1)
			close(curr_pipe[0]);
		if (curr_pipe[1] != -1)
		{
			dup2(curr_pipe[1], STDOUT_FILENO);
			close(curr_pipe[1]);
		}
	}
}

static void	check_path_existence(char *cmd, t_shell *shell, char **args_array)
{
	if (ft_strchr(cmd, '/'))
	{
		if (access(cmd, F_OK) == -1)
		{
			ft_putstr_fd("minishell: ", 2);
			ft_putstr_fd(cmd, 2);
			ft_putstr_fd(": No such file or directory\n", 2);
			free(args_array);
			cleanup_child(shell);
			exit(127);
		}
	}
}

void	check_special_cases(char *cmd, t_shell *shell, char **args_array)
{
	check_dot_cases(cmd, shell, args_array);
	check_path_existence(cmd, shell, args_array);
}

void	handle_command_not_found(char *cmd, char **args, t_shell *shell)
{
	char	*path_var;

	path_var = get_env_value(shell->env, "PATH");
	ft_putstr_fd("minishell: ", 2);
	ft_putstr_fd(cmd, 2);
	if (!path_var)
		ft_putstr_fd(": No such file or directory\n", 2);
	else
		ft_putstr_fd(": command not found\n", 2);
	free(args);
	cleanup_child(shell);
	exit(127);
}
