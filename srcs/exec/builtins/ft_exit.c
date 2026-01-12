/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_exit.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 23:16:44 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

int	is_valid_exit_arg(char *str)
{
	int	i;

	i = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '-' || str[i] == '+')
		i++;
	if (!ft_isdigit(str[i]))
		return (0);
	while (ft_isdigit(str[i]))
		i++;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] != '\0')
		return (0);
	return (1);
}

static int	handle_too_many_args(void)
{
	ft_putstr_fd("minishell: exit: too many arguments\n", 2);
	return (1);
}

static void	exit_with_error(char *arg, t_shell *shell, char **args_array)
{
	ft_putstr_fd("minishell: exit: ", 2);
	ft_putstr_fd(arg, 2);
	ft_putstr_fd(": numeric argument required\n", 2);
	free(args_array);
	if (shell->commands->next)
		cleanup_child(shell);
	else
		cleanup_shell(shell);
	exit(2);
}

static void	do_exit(t_shell *shell, long long exit_code, char **args)
{
	free(args);
	if (shell->commands->next)
		cleanup_child(shell);
	else
		cleanup_shell(shell);
	exit(exit_code % 256);
}

int	builtin_exit(char **args, t_shell *shell)
{
	long long	exit_code;

	if (!shell->commands->next)
		ft_putstr_fd("exit\n", 2);
	if (args[1])
	{
		if (!is_valid_exit_arg(args[1])
			|| ft_atoll_safe(args[1], &exit_code) == -1)
			exit_with_error(args[1], shell, args);
		if (args[2])
			return (handle_too_many_args());
	}
	else
		exit_code = shell->last_exit_status;
	do_exit(shell, exit_code, args);
	return (0);
}
