/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_cd_utils2.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 20:56:50 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/16 21:00:01 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

char	*save_current_pwd(t_env *env)
{
	char	*old_pwd_val;
	char	*old_pwd_copy;

	old_pwd_val = get_env_value(env, "PWD");
	if (!old_pwd_val)
		old_pwd_copy = getcwd(NULL, 0);
	else
		old_pwd_copy = ft_strdup(old_pwd_val);
	return (old_pwd_copy);
}

int	cd_error_no_home(char *old_pwd)
{
	ft_putstr_fd("minishell: cd: HOME not set\n", 2);
	if (old_pwd)
		free(old_pwd);
	return (1);
}

char	*get_target_directory(char **args, t_env *env)
{
	char	*dir;

	if (!args || !args[1])
	{
		dir = get_env_value(env, "HOME");
		if (!dir)
		{
			cd_error_no_home(NULL);
			return (NULL);
		}
		return (dir);
	}
	return (args[1]);
}
