/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_builtins.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/14 15:25:39 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

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

// int	builtin_pwd(void)
// {
// 	char	*cwd;

// 	cwd = getcwd(NULL, 0);
// 	if (cwd)
// 	{
// 		printf("%s\n", cwd);
// 		free(cwd);
// 		return (0);
// 	}
// 	else
// 	{
// 		perror("minishell: pwd");
// 		return (1);
// 	}
// }
int	builtin_pwd(t_shell *shell)
{
	char	*cwd;
	char	*pwd_env;

	cwd = getcwd(NULL, 0);
	if (cwd)
	{
		printf("%s\n", cwd);
		free(cwd);
		return (0);
	}

	//FALLBACK : Utiliser $PWD si getcwd() échoue
	pwd_env = get_env_value(shell->env, "PWD");
	if (pwd_env)
	{
		printf("%s\n", pwd_env);
		return (0);
	}

	//Dernier recours : Message d'erreur clair
	ft_putstr_fd("minishell: pwd: cannot access parent directories: ", 2);
	ft_putstr_fd("No such file or directory\n", 2);
	return (1);
}

int	builtin_env(t_env *env)
{
	while (env)
	{
		if (env->value && env->value[0] != '\0')
			printf("%s=%s\n", env->key, env->value);
		env = env->next;
	}
	return (0);
}
