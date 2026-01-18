/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_builtin_cd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/16 21:03:22 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static char	*make_absolute_path(char *old_pwd, char *dir)
{
	char	*temp;
	char	*result;

	if (!old_pwd || !dir)
		return (NULL);
	if (dir[0] == '/')
		return (ft_strdup(dir));
	if (ft_strcmp(dir, "..") == 0)
		return (resolve_dotdot_logical(old_pwd));
	temp = ft_strjoin(old_pwd, "/");
	if (!temp)
		return (NULL);
	result = ft_strjoin(temp, dir);
	free(temp);
	return (result);
}

static int	try_logical_cd(char *old_pwd, char *dir, t_env *env)
{
	char	*target_path;
	int		chdir_ret;

	if (!old_pwd || dir[0] == '/')
		return (-1);
	target_path = make_absolute_path(old_pwd, dir);
	if (!target_path)
		return (-1);
	chdir_ret = chdir(target_path);
	if (chdir_ret == 0)
	{
		update_pwd_vars(env, old_pwd, target_path);
		free(target_path);
		return (0);
	}
	free(target_path);
	return (-1);
}

static int	execute_cd(char *dir, char *old_pwd_copy, t_env *env)
{
	int	ret;

	ret = try_logical_cd(old_pwd_copy, dir, env);
	if (ret == 0)
	{
		free(old_pwd_copy);
		return (0);
	}
	if (chdir(dir) != 0)
		return (handle_chdir_failure(dir, old_pwd_copy, env));
	update_pwd_after_cd(env, old_pwd_copy);
	return (0);
}

int	builtin_cd(char **args, t_env *env)
{
	char	*dir;
	char	*old_pwd_copy;

	dir = get_target_directory(args, env);
	if (!dir)
		return (1);
	old_pwd_copy = save_current_pwd(env);
	if (!old_pwd_copy)
		return (1);
	return (execute_cd(dir, old_pwd_copy, env));
}
