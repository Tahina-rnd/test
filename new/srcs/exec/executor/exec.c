/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 20:14:46 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/09 11:45:04 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	prepare_pipe(t_command *cmd, int prev_pipe[2], int curr_pipe[2])
{
	if (cmd->next)
	{
		if (pipe(curr_pipe) == -1)
		{
			perror("minishell: pipe");
			if (prev_pipe[0] != -1)
			{
				close(prev_pipe[0]);
				close(prev_pipe[1]);
			}
			return (1);
		}
	}
	return (0);
}

static void	handle_parent_pipes(t_command *cmd, int prev[2], int curr[2])
{
	if (prev[0] != -1)
	{
		close(prev[0]);
		if (prev[1] != -1)
			close(prev[1]);
	}
	if (cmd->next)
	{
		close(curr[1]);
		prev[0] = curr[0];
		prev[1] = -1;
	}
}

static int	fork_handler(t_shell *sh, t_command *cmd, int prev[2], int curr[2])
{
	pid_t	pid;

	pid = fork();
	if (pid == -1)
	{
		perror("minishell: fork");
		if (prev[0] != -1)
		{
			close(prev[0]);
			close(prev[1]);
		}
		if (cmd->next)
		{
			close(curr[0]);
			close(curr[1]);
		}
		return (-1);
	}
	if (pid == 0)
		child_process(cmd, sh, prev, curr);
	return (pid);
}

static void	exec_loop(t_shell *shell, t_command *cmd)
{
	int		prev_pipe[2];
	int		curr_pipe[2];
	pid_t	last_pid;
	pid_t	pid;

	prev_pipe[0] = -1;
	prev_pipe[1] = -1;
	last_pid = -1;
	setup_exec_signals();
	while (cmd)
	{
		if (prepare_pipe(cmd, prev_pipe, curr_pipe))
		{
			shell->last_exit_status = 1;
			break ;
		}
		pid = fork_handler(shell, cmd, prev_pipe, curr_pipe);
		if (pid == -1)
		{
			shell->last_exit_status = 1;
			break ;
		}
		last_pid = pid;
		handle_parent_pipes(cmd, prev_pipe, curr_pipe);
		cmd = cmd->next;
	}
	wait_all_children(last_pid, shell);
}

void	executor(t_shell *shell)
{
	t_command	*cmd;

	cmd = shell->commands;
	if (process_heredocs(shell) != 0)
		return ;
	if (cmd && !cmd->next && cmd->args && cmd->args[0].value
		&& check_parent_builtin(cmd->args[0].value, shell))
	{
		exec_builtin_parent(cmd, shell);
		cleanup_heredocs(shell);
		return ;
	}
	exec_loop(shell, cmd);
	cleanup_heredocs(shell);
}
