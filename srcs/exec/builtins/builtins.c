/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/17 21:32:09 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/02 11:43:19 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	is_parent_builtin(char *cmd, t_shell *shell)
{
	if (!cmd || !shell || !shell->commands)
		return (0);
	if (!shell->commands->next)
	{
		if (ft_strcmp(cmd, "cd") == 0)
			return (1);
		if (ft_strcmp(cmd, "export") == 0)
			return (1);
		if (ft_strcmp(cmd, "unset") == 0)
			return (1);
		if (ft_strcmp(cmd, "exit") == 0)
			return (1);
	}
	return (0);
}

int	is_builtin(char *cmd)
{
	if (!cmd)
		return (0);
	if (ft_strcmp(cmd, "echo") == 0)
		return (1);
	if (ft_strcmp(cmd, "cd") == 0)
		return (1);
	if (ft_strcmp(cmd, "pwd") == 0)
		return (1);
	if (ft_strcmp(cmd, "export") == 0)
		return (1);
	if (ft_strcmp(cmd, "unset") == 0)
		return (1);
	if (ft_strcmp(cmd, "env") == 0)
		return (1);
	if (ft_strcmp(cmd, "exit") == 0)
		return (1);
	return (0);
}

static int	execute_builtin_part1(t_command *cmd, t_shell *shell)
{
	if (ft_strcmp(cmd->args[0], "echo") == 0)
		return (builtin_echo(cmd->args));
	if (ft_strcmp(cmd->args[0], "cd") == 0)
		return (builtin_cd(cmd->args, shell->env));
	if (ft_strcmp(cmd->args[0], "pwd") == 0)
		return (builtin_pwd());
	if (ft_strcmp(cmd->args[0], "export") == 0)
		return (ft_export(shell, cmd->args));
	return (-1);
}

static int	execute_builtin_part2(t_command *cmd, t_shell *shell)
{
	if (ft_strcmp(cmd->args[0], "unset") == 0)
		return (ft_unset(shell, cmd->args));
	if (ft_strcmp(cmd->args[0], "env") == 0)
		return (builtin_env(shell->env));
	if (ft_strcmp(cmd->args[0], "exit") == 0)
		return (builtin_exit(cmd->args, shell));
	return (1);
}

int	execute_builtin(t_command *cmd, t_shell *shell)
{
	int	result;

	if (!cmd || !cmd->args || !cmd->args[0])
		return (1);
	result = execute_builtin_part1(cmd, shell);
	if (result != -1)
		return (result);
	return (execute_builtin_part2(cmd, shell));
}

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

int	check_parent_builtin(char *cmd, t_shell *shell)
{
	return (is_parent_builtin(cmd, shell));
}
