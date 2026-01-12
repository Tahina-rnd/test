/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_update_pwd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 23:14:47 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static char	*get_new_pwd_absolute(char *new_dir)
{
	return (normalize_path(new_dir));
}

static char	*get_new_pwd_relative(char *old_pwd, char *new_dir)
{
	char	*temp;
	char	*new_pwd;

	temp = build_absolute_path(old_pwd, new_dir);
	if (temp)
	{
		new_pwd = normalize_path(temp);
		free(temp);
	}
	else
		new_pwd = getcwd(NULL, 0);
	return (new_pwd);
}

void	update_pwd_vars(t_env *env, char *old_pwd, char *new_dir)
{
	char	*new_pwd;

	update_env_var(env, "OLDPWD", old_pwd);
	if (new_dir && new_dir[0] == '/')
		new_pwd = get_new_pwd_absolute(new_dir);
	else if (new_dir)
		new_pwd = get_new_pwd_relative(old_pwd, new_dir);
	else
		new_pwd = getcwd(NULL, 0);
	if (new_pwd)
	{
		update_env_var(env, "PWD", new_pwd);
		free(new_pwd);
	}
}
