/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmd.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 20:32:05 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/16 21:19:02 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static void	cleanup_and_exit(char *path, char **args_array,
	t_shell *shell, int code)
{
	free(path);
	free(args_array);
	cleanup_child(shell);
	exit(code);
}

static void	handle_exec_error(char *cmd, char *path, char **args_array,
		t_shell *shell)
{
	struct stat	path_stat;

	if (access(path, F_OK) == -1)
	{
		ft_error(": No such file or directory\n", cmd);
		cleanup_and_exit(path, args_array, shell, 127);
	}
	if (stat(path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
	{
		ft_error(": is a directory\n", cmd);
		cleanup_and_exit(path, args_array, shell, 126);
	}
	if (access(path, X_OK) == -1)
	{
		ft_error(": Permission denied\n", cmd);
		cleanup_and_exit(path, args_array, shell, 126);
	}
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
	handle_exec_error(args_array[0], path, args_array, shell);
	env_tab = env_to_tab(env);
	if (!env_tab)
	{
		perror("minishell: malloc");
		free(path);
		free(args_array);
		cleanup_child(shell);
		exit(1);
	}
	execve(path, args_array, env_tab);
	try_shell_exec(path, env_tab);
	perror("minishell");
	free(path);
	free_tab(env_tab);
	exit(126);
}
