/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_pwd_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 11:24:53 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/15 11:52:20 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

char	*get_new_pwd(char *old_pwd, char *new_dir)
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

void	handle_oldpwd_update(t_env *env, char *old_pwd, int *found)
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

char	*try_cwd_path(char *cmd)
{
	char	*cwd_path;

	cwd_path = ft_strjoin("./", cmd);
	if (!cwd_path)
		return (NULL);
	if (access(cwd_path, F_OK | X_OK) == 0)
		return (cwd_path);
	free(cwd_path);
	return (NULL);
}

void	check_dot_cases(char *cmd, t_shell *shell, char **args_array)
{
	if (ft_strcmp(cmd, ".") == 0)
	{
		ft_putstr_fd("minishell: .: filename argument required\n", 2);
		ft_putstr_fd(".: usage: . filename [arguments]\n", 2);
		free(args_array);
		cleanup_child(shell);
		exit(2);
	}
	if (ft_strcmp(cmd, "..") == 0)
	{
		ft_putstr_fd("minishell: ..: command not found\n", 2);
		free(args_array);
		cleanup_child(shell);
		exit(127);
	}
}

char	*get_path(t_env *env, char *cmd)
{
	char	*path_var;
	char	**paths;
	char	*result;

	if (cmd[0] == '\0')
		return (NULL);
	if (ft_strchr(cmd, '/'))
	{
		if (access(cmd, F_OK) == -1)
			return (NULL);
		return (ft_strdup(cmd));
	}
	path_var = get_env_value(env, "PATH");
	if (!path_var)
		return (try_cwd_path(cmd));
	paths = ft_split(path_var, ':');
	if (!paths)
		return (NULL);
	result = try_paths(paths, cmd);
	free_tab(paths);
	return (result);
}
