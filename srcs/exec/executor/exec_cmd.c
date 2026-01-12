/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmd.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 20:32:05 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/12 09:41:16 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

char	*try_paths(char **paths, char *cmd)
{
	char	*part_path;
	char	*full_path;
	int		i;

	i = 0;
	while (paths[i])
	{
		part_path = ft_strjoin(paths[i], "/");
		if (!part_path)
			return (NULL);
		full_path = ft_strjoin(part_path, cmd);
		free(part_path);
		if (!full_path)
			return (NULL);
		if (access(full_path, F_OK | X_OK) == 0)
			return (full_path);
		free(full_path);
		i++;
	}
	return (NULL);
}

static void	handle_exec_error(char *cmd, char *path)
{
	struct stat	path_stat;

	if (stat(path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(cmd, 2);
		ft_putstr_fd(": is a directory\n", 2);
		free(path);
		exit(126);
	}
	if (access(path, X_OK) == -1)
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(cmd, 2);
		ft_putstr_fd(": Permission denied\n", 2);
		free(path);
		exit(126);
	}
}

static void	handle_command_not_found(char *cmd, char **args, t_shell *shell)
{
	ft_putstr_fd("minishell: ", 2);
	ft_putstr_fd(cmd, 2);
	ft_putstr_fd(": command not found\n", 2);
	free(args);
	cleanup_child(shell);
	exit(127);
}

static void	try_shell_exec(char *path, char **env_tab)
{
	char	**sh_args;

	if (errno != ENOEXEC)
		return ;
	sh_args = malloc(sizeof(char *) * 3);
	if (!sh_args)
		return ;
	sh_args[0] = ft_strdup("/bin/sh");
	sh_args[1] = ft_strdup(path);
	sh_args[2] = NULL;
	execve(sh_args[0], sh_args, env_tab);
	if (sh_args[0])
		free(sh_args[0]);
	if (sh_args[1])
		free(sh_args[1]);
	free(sh_args);
}

void	exec_simple_cmd_with_array(t_command *cmd, t_env *env,
		char **args_array, t_shell *shell)
{
	char	*path;
	char	**env_tab;

	if (!cmd || !args_array || !args_array[0])
		exit(0);
	check_special_cases(args_array[0], shell, args_array);
	path = get_path(env, args_array[0]);
	if (!path)
		handle_command_not_found(args_array[0], args_array, shell);
	handle_exec_error(args_array[0], path);
	env_tab = env_to_tab(env);
	execve(path, args_array, env_tab);
	try_shell_exec(path, env_tab);
	perror("minishell");
	free(path);
	free_tab(env_tab);
	exit(126);
}
