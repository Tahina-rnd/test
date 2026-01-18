/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_update_pwd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/16 14:14:53 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

char	*get_new_pwd_absolute(char *new_dir)
{
	return (normalize_path(new_dir));
}

char	*get_new_pwd_relative(char *old_pwd, char *new_dir)
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
	int		found_oldpwd;

	found_oldpwd = 0;
	handle_oldpwd_update(env, old_pwd, &found_oldpwd);
	new_pwd = get_new_pwd(old_pwd, new_dir);
	if (new_pwd)
	{
		update_env_var(env, "PWD", new_pwd);
		free(new_pwd);
	}
}

void	update_pwd_vars_unnormalized(t_env *env, char *old_pwd, char *new_dir)
{
	char	*new_pwd;
	int		found_oldpwd;
	char	*temp;

	found_oldpwd = 0;
	handle_oldpwd_update(env, old_pwd, &found_oldpwd);
	if (!new_dir || !old_pwd)
		return ;
	if (new_dir[0] == '/')
		new_pwd = ft_strdup(new_dir);
	else
	{
		temp = ft_strjoin(old_pwd, "/");
		if (!temp)
			return ;
		new_pwd = ft_strjoin(temp, new_dir);
		free(temp);
	}
	if (new_pwd)
	{
		update_env_var(env, "PWD", new_pwd);
		free(new_pwd);
	}
}
