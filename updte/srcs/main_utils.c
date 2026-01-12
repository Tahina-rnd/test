/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/15 09:40:43 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/12 07:45:51 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/exec.h"

void	init_shell(t_shell *shell, char **envp)
{
	char	*cwd;

	shell->env = dup_env(envp);
	if (!shell->env)
	{
		cwd = getcwd(NULL, 0);
		if (!cwd)
			cwd = ft_strdup("/");
		shell->env = create_env_node("PWD", cwd);
		free(cwd);
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
