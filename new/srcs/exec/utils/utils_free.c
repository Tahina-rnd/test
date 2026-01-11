/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_free.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 13:28:19 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 14:20:39 by tarandri         ###   ########.fr       */
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
void cleanup_child(t_shell *shell)
{
    if (shell->env)
    {
        free_env(shell->env);
        shell->env = NULL;  // ← Ajoutez ceci
    }
    if (shell->commands)
    {
        free_commands(shell->commands);
        shell->commands = NULL;  // ← Ajoutez ceci
    }
    if (shell->tokens)
    {
        free_tokens(shell->tokens);
        shell->tokens = NULL;  // ← Ajoutez ceci
    }
    if (shell->input)
    {
        free(shell->input);
        shell->input = NULL;  // ← Ajoutez ceci
    }
}
