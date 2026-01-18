/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/17 21:32:09 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/16 20:53:10 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

int	is_parent_builtin(char *cmd, t_shell *shell)
{
	if (!cmd || !shell || !shell->commands)
		return (0);
	if (!shell->commands->next)
	{
		if (ft_strcmp(cmd, "cd") == 0)
			return (1);
		if (ft_strcmp(cmd, "export") == 0)
			return (1);
		if (ft_strcmp(cmd, "unset") == 0)
			return (1);
		if (ft_strcmp(cmd, "exit") == 0)
			return (1);
	}
	return (0);
}

int	is_builtin(char *cmd)
{
	if (!cmd)
		return (0);
	if (ft_strcmp(cmd, "echo") == 0)
		return (1);
	if (ft_strcmp(cmd, "cd") == 0)
		return (1);
	if (ft_strcmp(cmd, "pwd") == 0)
		return (1);
	if (ft_strcmp(cmd, "export") == 0)
		return (1);
	if (ft_strcmp(cmd, "unset") == 0)
		return (1);
	if (ft_strcmp(cmd, "env") == 0)
		return (1);
	if (ft_strcmp(cmd, "exit") == 0)
		return (1);
	return (0);
}

int	check_parent_builtin(char *cmd, t_shell *shell)
{
	return (is_parent_builtin(cmd, shell));
}

char	*get_old_pwd(t_env *env)
{
	char	*old_pwd;
	char	*cwd;

	old_pwd = get_env_value(env, "PWD");
	if (old_pwd && old_pwd[0] != '\0')
		return (ft_strdup(old_pwd));
	cwd = getcwd(NULL, 0);
	if (cwd)
		return (cwd);
	return (NULL);
}
