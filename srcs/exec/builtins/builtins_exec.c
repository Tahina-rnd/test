/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins_exec.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 23:05:52 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/14 15:28:58 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	count_args(t_arg *args)
{
	int		count;
	t_arg	*current;

	count = 0;
	current = args;
	while (current)
	{
		count++;
		current = current->next;
	}
	return (count);
}

char	**args_to_array(t_arg *args)
{
	char	**result;
	int		i;
	t_arg	*current;

	if (!args)
		return (NULL);
	result = (char **)malloc(sizeof(char *) * (count_args(args) + 1));
	if (!result)
		return (NULL);
	i = 0;
	current = args;
	while (current)
	{
		result[i++] = current->value;
		current = current->next;
	}
	result[i] = NULL;
	return (result);
}

static int	execute_builtin_part1(t_shell *shell, char **args_array)
{
	if (ft_strcmp(args_array[0], "echo") == 0)
		return (builtin_echo(args_array));
	if (ft_strcmp(args_array[0], "cd") == 0)
		return (builtin_cd(args_array, shell->env));
	if (ft_strcmp(args_array[0], "pwd") == 0)
		return (builtin_pwd(shell));
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

int	execute_builtin(t_command *cmd, t_shell *shell)
{
	int		result;
	char	**args_array;
	int		is_exit_cmd;

	if (!cmd || !cmd->args || !cmd->args->value)
		return (1);
	args_array = args_to_array(cmd->args);
	if (!args_array)
		return (1);
	is_exit_cmd = (ft_strcmp(args_array[0], "exit") == 0);
	result = execute_builtin_part1(shell, args_array);
	if (result != -1)
	{
		free(args_array);
		return (result);
	}
	result = execute_builtin_part2(shell, args_array);
	if (!is_exit_cmd || result != 0)
		free(args_array);
	return (result);
}
