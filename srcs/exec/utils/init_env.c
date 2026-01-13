/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_env.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 10:49:05 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/13 11:53:26 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static void	handle_env_malloc_error(t_shell *shell)
{
	free_env(shell->env);
	shell->env = NULL;
	ft_putstr_fd("minishell: malloc error\n", 2);
	exit(1);
}

static void	add_env_to_list(t_shell *shell, t_env *new)
{
	t_env	*curr;

	if (!shell->env)
		shell->env = new;
	else
	{
		curr = shell->env;
		while (curr->next)
			curr = curr->next;
		curr->next = new;
	}
}

void	init_env(t_shell *shell, char **envp)
{
	t_env	*new;
	int		i;

	if (!shell)
		return ;
	shell->env = NULL;
	i = 0;
	while (envp && envp[i])
	{
		new = new_env_node(envp[i]);
		if (!new)
			handle_env_malloc_error(shell);
		add_env_to_list(shell, new);
		i++;
	}
	init_pwd(&shell->env);
	init_shlvl(&shell->env);
	init_underscore(&shell->env);
	init_oldpwd(&shell->env);
}
