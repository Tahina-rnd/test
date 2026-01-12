/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_child.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 20:13:41 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/12 16:23:32 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static void	exec_builtin_child(t_command *cmd, t_shell *shell)
{
	int	ret;

	ret = execute_builtin(cmd, shell);
	cleanup_child(shell);
	exit(ret);
}

static void	exec_external_cmd(t_command *cmd, t_shell *shell)
{
	char	**args_array;

	args_array = args_to_array(cmd->args);
	if (!args_array)
	{
		cleanup_child(shell);
		exit(1);
	}
	exec_simple_cmd_with_array(cmd, shell->env, args_array, shell);
	free(args_array);
	cleanup_child(shell);
	exit(126);
}

void	child_process(t_command *cmd, t_shell *shell, int prev[2], int curr[2])
{
	setup_child_signals();
	setup_pipes(cmd, prev, curr);
	if (handle_redirections(cmd) != 0)
	{
		cleanup_child(shell);
		exit(1);
	}
	if (!cmd->args || !cmd->args->value)
	{
		cleanup_child(shell);
		exit(0);
	}
	if (is_builtin(cmd->args->value))
		exec_builtin_child(cmd, shell);
	else
		exec_external_cmd(cmd, shell);
}

static void	check_signal(int status, int *sig_int, int *sig_quit)
{
	if (WIFSIGNALED(status))
	{
		if (WTERMSIG(status) == SIGINT)
			*sig_int = 1;
		else if (WTERMSIG(status) == SIGQUIT)
			*sig_quit = 1;
	}
}

void	wait_all_children(pid_t last_pid, t_shell *shell)
{
	pid_t	wpid;
	int		status;
	int		last_status;
	int		sig[2];

	sig[0] = 0;
	sig[1] = 0;
	last_status = 0;
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	while (1)
	{
		wpid = waitpid(-1, &status, 0);
		if (wpid == -1)
			break ;
		check_signal(status, &sig[0], &sig[1]);
		if (wpid == last_pid)
			last_status = status;
	}
	setup_prompt_signal();
	update_exit_status(shell, last_status, sig[0], sig[1]);
}
