/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmd.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 20:32:05 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 10:43:39 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

// ... (try_paths et get_path inchangés sauf petit fix) ...

static char	*try_paths(char **paths, char *cmd)
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
		return (NULL);
	paths = ft_split(path_var, ':');
	free(path_var); // IMPORTANT: get_env_value retourne un strdup, il faut le free
	if (!paths)
		return (NULL);
	result = try_paths(paths, cmd);
	free_tab(paths);
	return (result);
}

// ... (handle_exec_error et check_special_cases inchangés) ...

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

static void	check_special_cases(char *cmd)
{
	if (ft_strcmp(cmd, ".") == 0)
	{
		ft_putstr_fd("minishell: .: filename argument required\n", 2);
		exit(2);
	}
	if (ft_strcmp(cmd, "..") == 0)
	{
		ft_putstr_fd("minishell: ..: command not found\n", 2);
		exit(127);
	}
}

/*
** CORRECTION ICI : Conversion Liste -> Tableau pour exec_simple_cmd
*/
void	exec_simple_cmd(t_command *cmd, t_env *env)
{
	char	*path;
	char	**env_tab;
	char	**sh_args;
	char	**args_array;
	t_arg	*curr;
	int		i;

	// Correction accès
	if (!cmd || !cmd->args || !cmd->args->value)
		exit(0);
	check_special_cases(cmd->args->value);

	// 1. Compter (Liste)
	i = 0;
	curr = cmd->args;
	while (curr)
	{
		i++;
		curr = curr->next;
	}

	// 2. Allouer
	args_array = (char **)malloc(sizeof(char *) * (i + 1));
	if (!args_array)
		exit(1);

	// 3. Remplir
	i = 0;
	curr = cmd->args;
	while (curr)
	{
		args_array[i] = curr->value;
		curr = curr->next;
		i++;
	}
	args_array[i] = NULL;

	// Utiliser la bonne valeur pour le path (cmd->args->value)
	path = get_path(env, cmd->args->value);
	if (!path)
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(cmd->args->value, 2);
		ft_putstr_fd(": command not found\n", 2);
		free(args_array);
		exit(127);
	}
	handle_exec_error(cmd->args->value, path);
	env_tab = env_to_tab(env);
	execve(path, args_array, env_tab);
	if (errno == ENOEXEC)
	{
		sh_args = malloc(sizeof(char *) * 3);
		if (sh_args)
		{
			sh_args[0] = ft_strdup("/bin/sh");
			sh_args[1] = ft_strdup(path);
			sh_args[2] = NULL;
			execve(sh_args[0], sh_args, env_tab);
		}
	}
	perror("minishell");
	free(path);
	free(args_array);
	free_tab(env_tab);
	exit(126);
}

// ... (exec_simple_cmd_with_array inchangé) ...
void	exec_simple_cmd_with_array(t_command *cmd, t_env *env, char **args_array)
{
	char	*path;
	char	**env_tab;
	char	**sh_args;

	(void)cmd;
	if (!args_array || !args_array[0])
		exit(0);
	check_special_cases(args_array[0]);
	path = get_path(env, args_array[0]);
	if (!path)
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(args_array[0], 2);
		ft_putstr_fd(": command not found\n", 2);
		exit(127);
	}
	handle_exec_error(args_array[0], path);
	env_tab = env_to_tab(env);
	execve(path, args_array, env_tab);
	if (errno == ENOEXEC)
	{
		sh_args = malloc(sizeof(char *) * 3);
		if (sh_args)
		{
			sh_args[0] = ft_strdup("/bin/sh");
			sh_args[1] = ft_strdup(path);
			sh_args[2] = NULL;
			execve(sh_args[0], sh_args, env_tab);
		}
	}
	perror("minishell");
	free(path);
	free_tab(env_tab);
	exit(126);
}
