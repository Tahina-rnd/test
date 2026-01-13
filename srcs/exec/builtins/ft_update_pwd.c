/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_update_pwd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/13 12:31:38 by miokrako         ###   ########.fr       */
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

static void	handle_oldpwd_update(t_env *env, char *old_pwd, int *found)
{
	t_env	*current;
	t_env	*new_node;

	current = env;
	while (current)
	{
		if (ft_strcmp(current->key, "OLDPWD") == 0)
		{
			free(current->value);
			current->value = ft_strdup(old_pwd);
			*found = 1;
			break ;
		}
		current = current->next;
	}
	if (!(*found))
	{
		new_node = new_env_node_kv("OLDPWD", old_pwd);
		if (new_node)
			exp_add_env_node_back(&env, new_node);
	}
}

static char	*get_new_pwd(char *old_pwd, char *new_dir)
{
	char	*new_pwd;

	if (new_dir && new_dir[0] == '/')
		new_pwd = get_new_pwd_absolute(new_dir);
	else if (new_dir)
		new_pwd = get_new_pwd_relative(old_pwd, new_dir);
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
