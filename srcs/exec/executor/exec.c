/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 20:14:46 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/12 15:45:48 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	process_command(t_shell *shell, t_command *cmd,
		int prev[2], int curr[2])
{
	pid_t	pid;

	if (prepare_pipe(cmd, prev, curr))
		return (-1);
	pid = fork_handler(shell, cmd, prev, curr);
	if (pid == -1)
		return (-1);
	handle_parent_pipes(cmd, prev, curr);
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
		pid = process_command(shell, cmd, prev_pipe, curr_pipe);
		if (pid == -1)
		{
			shell->last_exit_status = 1;
			break ;
		}
		last_pid = pid;
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
	if (cmd && !cmd->next && cmd->args && cmd->args->value
		&& check_parent_builtin(cmd->args->value, shell))
	{
		exec_builtin_parent(cmd, shell);
		cleanup_heredocs(shell);
		return ;
	}
	exec_loop(shell, cmd);
	cleanup_heredocs(shell);
}
