/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/15 09:40:43 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/15 12:06:38 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/exec.h"

static void	if_env(t_env	*shlvl_node, t_env *underscore_node,
					t_env *oldpwd_node, t_shell *shell)
{
	if (shlvl_node)
		add_env_node(&shell->env, shlvl_node);
	if (underscore_node)
		add_env_node(&shell->env, underscore_node);
	if (oldpwd_node)
		add_env_node(&shell->env, oldpwd_node);
}

void	init_shell(t_shell *shell, char **envp)
{
	char	*cwd;
	t_env	*pwd_node;
	t_env	*shlvl_node;
	t_env	*underscore_node;
	t_env	*oldpwd_node;

	init_env(shell, envp);
	if (!shell->env)
	{
		cwd = getcwd(NULL, 0);
		if (!cwd)
			cwd = ft_strdup("/");
		pwd_node = new_env_node_kv("PWD", cwd);
		shlvl_node = new_env_node_kv("SHLVL", "1");
		underscore_node = new_env_node_kv("_", "/usr/bin/env");
		oldpwd_node = new_env_node_kv("OLDPWD", "");
		free(cwd);
		if (pwd_node)
			shell->env = pwd_node;
		if_env(shlvl_node, underscore_node, oldpwd_node, shell);
	}
	shell->input = NULL;
	shell->tokens = NULL;
	shell->commands = NULL;
	shell->last_exit_status = 0;
}

void	reset_loop(t_shell *shell)
{
	if (shell->commands)
	{
		free_commands(shell->commands);
		shell->commands = NULL;
	}
	if (shell->tokens)
	{
		free_tokens(shell->tokens);
		shell->tokens = NULL;
	}
	if (shell->input)
	{
		free(shell->input);
		shell->input = NULL;
	}
}

void	cleanup_exit(t_shell *shell)
{
	reset_loop(shell);
	if (shell->env)
		free_env_list(shell->env);
	rl_clear_history();
}

int	is_exit_command(char *input)
{
	int	i;

	i = 0;
	while (input[i] && (input[i] == ' ' || input[i] == '\t'))
		i++;
	if (ft_strncmp(&input[i], "exit", 4) == 0)
	{
		i += 4;
		while (input[i] && (input[i] == ' ' || input[i] == '\t'))
			i++;
		if (input[i] == '\0')
			return (1);
	}
	return (0);
}
