/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_builtins.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/05 15:11:52 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	ft_is_numeric(char *str)
{
	int	i;

	i = 0;
	if (str[i] == '-' || str[i] == '+')
		i++;
	if (!str[i])
		return (0);
	while (str[i])
	{
		if (!ft_isdigit(str[i]))
			return (0);
		i++;
	}
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

static int	handle_exit_numeric_error(char *arg, t_shell *shell)
{
	ft_putstr_fd("minishell: exit: ", 2);
	ft_putstr_fd(arg, 2);
	ft_putstr_fd(": numeric argument required\n", 2);
	cleanup_shell(shell);
	exit(2);
}

static int	handle_too_many_args(void)
{
	ft_putstr_fd("minishell: exit: too many arguments\n", 2);
	return (1);
}

int	builtin_exit(char **args, t_shell *shell)
{
	int	exit_code;

	ft_putstr_fd("exit\n", 2);
	exit_code = 0;
	if (args[1])
	{
		if (!ft_is_numeric(args[1]))
			handle_exit_numeric_error(args[1], shell);
		else if (args[2])
			return (handle_too_many_args());
		else
			exit_code = ft_atoi(args[1]) % 256;
	}
	else
		exit_code = shell->last_exit_status;
	cleanup_shell(shell);
	exit(exit_code);
}

int	builtin_env(t_env *env)
{
	while (env)
	{
		printf("%s=%s\n", env->key, env->value);
		env = env->next;
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
	if (args[2])
	{
		ft_putstr_fd("minishell: cd: too many arguments\n", 2);
		return(1);
	}
	if (!old_pwd)
	{
		perror("minishell: cd: getcwd");
		return (1);
	}
	if (args[1])
		dir = args[1];
	else
	{
		dir = get_env_value(env, "HOME");
		if (!dir)
			return (cd_error_no_home(old_pwd));
	}
	if (chdir(dir) != 0)
		return (cd_error_chdir(dir, old_pwd));
	update_pwd_vars(env, old_pwd);
	free(old_pwd);
	return (0);
}
