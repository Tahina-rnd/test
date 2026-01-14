/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins_parent.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 23:05:52 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/13 14:09:31 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	setup_fd_backup(int *original_stdin, int *original_stdout)
{
	*original_stdin = dup(STDIN_FILENO);
	*original_stdout = dup(STDOUT_FILENO);
	if (*original_stdin == -1 || *original_stdout == -1)
	{
		if (*original_stdin != -1)
			close(*original_stdin);
		if (*original_stdout != -1)
			close(*original_stdout);
		perror("minishell: dup");
		return (1);
	}
	return (0);
}

static void	restore_fd(int original_stdin, int original_stdout)
{
	dup2(original_stdin, STDIN_FILENO);
	dup2(original_stdout, STDOUT_FILENO);
	close(original_stdin);
	close(original_stdout);
}

void	exec_builtin_parent(t_command *cmd, t_shell *shell)
{
	int	original_stdin;
	int	original_stdout;
	int	ret;

	if (setup_fd_backup(&original_stdin, &original_stdout))
	{
		shell->last_exit_status = 1;
		return ;
	}
	ret = handle_redirections(cmd);
	if (ret == 0)
		shell->last_exit_status = execute_builtin(cmd, shell);
	else
		shell->last_exit_status = 1;
	restore_fd(original_stdin, original_stdout);
}
