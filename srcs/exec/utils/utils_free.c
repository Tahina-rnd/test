/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_free.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 13:28:19 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/10 22:34:14 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

void	free_env(t_env *env)
{
	t_env	*tmp;

	while (env)
	{
		tmp = env->next;
		free(env->key);
		free(env->value);
		free(env);
		env = tmp;
	}
}

void	free_command(t_command *cmd)
{
	int	i;

	if (cmd->args)
	{
		i = 0;
		while (cmd->args[i])
		{
			free(cmd->args[i]);
			i++;
		}
		free(cmd->args);
	}
	if (cmd->input_redirection)
		free(cmd->input_redirection);
	if (cmd->output_redirection)
		free(cmd->output_redirection);
	free(cmd);
}

void	free_commands(t_command *commands)
{
	t_command	*tmp;

	while (commands)
	{
		tmp = commands->next;
		free_command(commands);
		commands = tmp;
	}
}

void	cleanup_shell(t_shell *shell)
{
	if (shell->env)
		free_env(shell->env);
	if (shell->commands)
		free_commands(shell->commands);
	if (shell->input)
		free(shell->input);
}

void	free_tab_partial(char **tab, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		free(tab[i]);
		i++;
	}
	free(tab);
}

void	free_tab(char **tab)
{
	int	i;

	i = 0;
	while (tab[i])
	{
		free(tab[i]);
		i++;
	}
	free(tab);
}

// void	cleanup_child(t_shell *shell)
// {
// 	if (shell->env)
// 		free_env(shell->env);
// 	if (shell->commands)
// 		free_commands(shell->commands);
// }
void	cleanup_child(t_shell *shell)
{
	if (shell->env)
		free_env(shell->env);
	if (shell->commands)
		free_commands(shell->commands);
	if (shell->tokens)
		free_tokens(shell->tokens);
	if (shell->input)
		free(shell->input);
}
