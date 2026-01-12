/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 23:30:45 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 23:31:19 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

void	cleanup_child(t_shell *shell)
{
	if (!shell)
		return ;
	if (shell->env)
	{
		free_env(shell->env);
		shell->env = NULL;
	}
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

void	cleanup_shell(t_shell *shell)
{
	if (!shell)
		return ;
	if (shell->env)
	{
		free_env(shell->env);
		shell->env = NULL;
	}
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
	rl_clear_history();
}
