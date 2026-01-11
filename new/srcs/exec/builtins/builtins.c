/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/17 21:32:09 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 14:30:56 by tarandri         ###   ########.fr       */
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

static int	execute_builtin_part1(t_shell *shell, char **args_array)
{
	if (ft_strcmp(args_array[0], "echo") == 0)
		return (builtin_echo(args_array));
	if (ft_strcmp(args_array[0], "cd") == 0)
		return (builtin_cd(args_array, shell->env));
	if (ft_strcmp(args_array[0], "pwd") == 0)
		return (builtin_pwd());
	if (ft_strcmp(args_array[0], "export") == 0)
		return (ft_export(shell, args_array));
	return (-1);
}

static int	execute_builtin_part2(t_shell *shell, char **args_array)
{
	if (ft_strcmp(args_array[0], "unset") == 0)
		return (ft_unset(shell, args_array));
	if (ft_strcmp(args_array[0], "env") == 0)
		return (builtin_env(shell->env));
	if (ft_strcmp(args_array[0], "exit") == 0)
		return (builtin_exit(args_array, shell));
	return (1);
}

// NOUVELLE VERSION
int	execute_builtin(t_command *cmd, t_shell *shell)
{
	int		result;
	char	**args_array;

	if (!cmd || !cmd->args || !cmd->args->value)
		return (1);

	args_array = args_to_array(cmd->args);
	if (!args_array)
		return (1);

	result = execute_builtin_part1(shell, args_array);
	if (result != -1)
	{
		free(args_array);
		return (result);
	}
	result = execute_builtin_part2(shell, args_array);
	free(args_array);
	return (result);
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
