/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_builtins.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 14:40:10 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	is_valid_exit_arg(char *str)
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

static int	is_n_flag(char *arg)
{
	int	i;

	i = 1;
	if (ft_strncmp(arg, "-n", 2) != 0)
		return (0);
	while (arg[i])
	{
		if (arg[i] != 'n')
			return (0);
		i++;
	}
	return (1);
}

int	builtin_echo(char **args)
{
	int	i;
	int	n_flag;

	i = 1;
	n_flag = 0;
	while (args[i] && is_n_flag(args[i]))
	{
		n_flag = 1;
		i++;
	}
	while (args[i])
	{
		ft_putstr_fd(args[i], 1);
		if (args[i + 1])
			ft_putstr_fd(" ", 1);
		i++;
	}
	if (!n_flag)
		ft_putstr_fd("\n", 1);
	return (0);
}

int	builtin_pwd(void)
{
	char	*cwd;

	cwd = getcwd(NULL, 0);
	if (cwd)
	{
		printf("%s\n", cwd);
		free(cwd);
		return (0);
	}
	else
	{
		perror("minishell: pwd");
		return (1);
	}
}

static int	handle_too_many_args(void)
{
	ft_putstr_fd("minishell: exit: too many arguments\n", 2);
	return (1);
}
int	ft_atoll_safe(const char *str, long long *result)
{
	unsigned long long	res;
	int					sign;
	int					i;

	res = 0;
	sign = 1;
	i = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i++] == '-')
			sign = -1;
	}
	while (str[i] >= '0' && str[i] <= '9')
	{
		if (sign == 1 && (res > LLONG_MAX / 10 || (res == LLONG_MAX / 10
					&& (str[i] - '0') > LLONG_MAX % 10)))
			return (-1);
		if (sign == -1 && (res > (unsigned long long)LLONG_MAX + 1 / 10
				|| (res == ((unsigned long long)LLONG_MAX + 1) / 10 && (str[i]
						- '0') > 8)))
			return (-1);
		res = res * 10 + (str[i++] - '0');
	}
	*result = res * sign;
	return (0);
}
static void	exit_with_error(char *arg, t_shell *shell)
{
	ft_putstr_fd("minishell: exit: ", 2);
	ft_putstr_fd(arg, 2);
	ft_putstr_fd(": numeric argument required\n", 2);
	if (shell->commands->next)
		cleanup_child(shell);
	else
		cleanup_shell(shell);
	exit(2);
}

int	builtin_exit(char **args, t_shell *shell)
{
	long long	exit_code;

	if (!shell->commands->next)
		ft_putstr_fd("exit\n", 2);
	if (args[1])
	{
		if (!is_valid_exit_arg(args[1]) || ft_atoll_safe(args[1], &exit_code) ==
			-1)
			exit_with_error(args[1], shell);
		if (args[2])
			return (handle_too_many_args());
	}
	else
		exit_code = shell->last_exit_status;
	if (shell->commands->next)
		cleanup_child(shell);
	else
		cleanup_shell(shell);
	exit(exit_code % 256);
}

int	builtin_env(t_env *env)
{
	t_env	*current;

	current = env;
	while (current)
	{
		if (current->value)
			printf("%s=%s\n", current->key, current->value);
		current = current->next;
	}
	return (0);
}

static int	cd_error_no_home(char *old_pwd)
{
	ft_putstr_fd("minishell: cd: HOME not set\n", 2);
	free(old_pwd);
	return (1);
}

static int	cd_error_chdir(char *dir, char *old_pwd)
{
	ft_putstr_fd("minishell: cd: ", 2);
	ft_putstr_fd(dir, 2);
	ft_putstr_fd(": ", 2);
	ft_putstr_fd(strerror(errno), 2);
	ft_putstr_fd("\n", 2);
	free(old_pwd);
	return (1);
}

static void	update_pwd_vars(t_env *env, char *old_pwd)
{
	char	*new_pwd;

	update_env_var(env, "OLDPWD", old_pwd);
	new_pwd = getcwd(NULL, 0);
	if (new_pwd)
	{
		update_env_var(env, "PWD", new_pwd);
		free(new_pwd);
	}
}

int	builtin_cd(char **args, t_env *env)
{
	char	*dir;
	char	*old_pwd;

	old_pwd = getcwd(NULL, 0);
	if (!old_pwd)
	{
		perror("minishell: cd: getcwd");
		return (1);
	}

	// Vérification du nombre d'arguments
	if (args && args[1] && args[2])
	{
		ft_putstr_fd("minishell: cd: too many arguments\n", 2);
		free(old_pwd);
		return (1);
	}

	// Déterminer le répertoire cible
	if (!args || !args[1])
		dir = get_env_value(env, "HOME");
	else
		dir = args[1];

	if (!dir)
		return (cd_error_no_home(old_pwd));

	if (chdir(dir) != 0)
		return (cd_error_chdir(dir, old_pwd));

	update_pwd_vars(env, old_pwd);
	free(old_pwd);
	return (0);
}
