/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_builtin_cd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 23:15:53 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

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

static char	*get_old_pwd(t_env *env)
{
	char	*old_pwd;

	old_pwd = get_env_value(env, "PWD");
	if (!old_pwd)
		old_pwd = getcwd(NULL, 0);
	else
		old_pwd = ft_strdup(old_pwd);
	if (!old_pwd)
	{
		perror("minishell: cd: getcwd");
		return (NULL);
	}
	return (old_pwd);
}

int	builtin_cd(char **args, t_env *env)
{
	char	*dir;
	char	*old_pwd;

	old_pwd = get_old_pwd(env);
	if (!old_pwd)
		return (1);
	if (!args || !args[1])
		dir = get_env_value(env, "HOME");
	else
		dir = args[1];
	if (!dir)
		return (cd_error_no_home(old_pwd));
	if (chdir(dir) != 0)
		return (cd_error_chdir(dir, old_pwd));
	update_pwd_vars(env, old_pwd, dir);
	free(old_pwd);
	return (0);
}
