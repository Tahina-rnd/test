/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_env_init.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 10:49:05 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/12 15:48:06 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

void	init_pwd(t_env **env)
{
	char	*cwd;
	t_env	*node;

	if (get_env_value(*env, "PWD"))
		return ;
	cwd = getcwd(NULL, 0);
	if (!cwd)
		cwd = ft_strdup("/");
	node = new_env_node_kv("PWD", cwd);
	free(cwd);
	if (node)
		add_env_node(env, node);
}

static void	update_shlvl_existing(t_env *env, char *new_shlvl)
{
	t_env	*curr;

	curr = env;
	while (curr)
	{
		if (ft_strcmp(curr->key, "SHLVL") == 0)
		{
			free(curr->value);
			curr->value = new_shlvl;
			return ;
		}
		curr = curr->next;
	}
	free(new_shlvl);
}

void	init_shlvl(t_env **env)
{
	char	*shlvl_str;
	int		shlvl;
	char	*new_shlvl;
	t_env	*node;

	shlvl_str = get_env_value(*env, "SHLVL");
	if (shlvl_str)
	{
		shlvl = ft_atoi(shlvl_str);
		if (shlvl < 0)
			shlvl = 0;
		shlvl++;
		new_shlvl = ft_itoa(shlvl);
		update_shlvl_existing(*env, new_shlvl);
	}
	else
	{
		node = new_env_node_kv("SHLVL", "1");
		if (node)
			add_env_node(env, node);
	}
}

void	init_underscore(t_env **env)
{
	t_env	*node;
	t_env	*curr;

	/* Vérifier si _ existe déjà */
	curr = *env;
	while (curr)
	{
		if (ft_strcmp(curr->key, "_") == 0)
		{
			/* Mettre à jour la valeur */
			free(curr->value);
			curr->value = ft_strdup("/usr/bin/env");
			return ;
		}
		curr = curr->next;
	}

	/* Si _ n'existe pas, le créer */
	node = new_env_node_kv("_", "/usr/bin/env");
	if (node)
		add_env_node(env, node);
}
