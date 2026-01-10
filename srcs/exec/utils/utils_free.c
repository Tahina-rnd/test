/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_free.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 13:28:19 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/10 23:04:11 by tarandri         ###   ########.fr       */
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
	t_arg	*current_arg;
	t_arg	*next_arg;

	if (!cmd)
		return ;
	// Correction ici : Parcours de liste chaînée au lieu d'indexation de tableau
	if (cmd->args)
	{
		current_arg = cmd->args;
		while (current_arg)
		{
			next_arg = current_arg->next;
			if (current_arg->value)
				free(current_arg->value);
			// Pensez à libérer 'segments' ici si nécessaire, pour éviter les leaks
			free(current_arg);
			current_arg = next_arg;
		}
	}
	if (cmd->input_redirection)
		free_redirections(cmd->input_redirection); // Assurez-vous d'avoir accès à cette fonction ou réimplémentez la boucle
	if (cmd->output_redirection)
		free_redirections(cmd->output_redirection);
	if (cmd->heredoc)
		free_redirections(cmd->heredoc);
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
