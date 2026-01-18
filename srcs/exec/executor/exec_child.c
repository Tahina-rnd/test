/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_child.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 20:13:41 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/15 11:06:08 by miokrako         ###   ########.fr       */
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
	if (!cmd->args || !cmd->args->value)
	{
		handle_redirections(cmd);
		cleanup_child(shell);
		exit(0);
	}
	if (handle_redirections(cmd) != 0)
	{
		cleanup_child(shell);
		exit(1);
	}
	if (is_builtin(cmd->args->value))
		exec_builtin_child(cmd, shell);
	else
		exec_external_cmd(cmd, shell);
}
