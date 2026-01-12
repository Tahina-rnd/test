/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipe_and_fork.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 08:28:00 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/12 08:30:20 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

int	prepare_pipe(t_command *cmd, int prev_pipe[2], int curr_pipe[2])
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

void	handle_parent_pipes(t_command *cmd, int prev[2], int curr[2])
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

int	fork_handler(t_shell *sh, t_command *cmd, int prev[2], int curr[2])
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
