/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_env_node.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 10:49:05 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 23:27:41 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static void	free_env_node_partial(t_env *node)
{
	if (node->key)
		free(node->key);
	if (node->value)
		free(node->value);
	free(node);
}

t_env	*new_env_node_kv(char *key, char *value)
{
	t_env	*node;

	if (!key || !value)
		return (NULL);
	node = malloc(sizeof(t_env));
	if (!node)
		return (NULL);
	node->key = ft_strdup(key);
	node->value = ft_strdup(value);
	if (!node->key || !node->value)
	{
		free_env_node_partial(node);
		return (NULL);
	}
	node->next = NULL;
	return (node);
}

void	exp_add_env_node_back(t_env **head, t_env *new_node)
{
	t_env	*current;

	if (!head || !new_node)
		return ;
	if (*head == NULL)
	{
		*head = new_node;
		return ;
	}
	current = *head;
	while (current->next)
		current = current->next;
	current->next = new_node;
}

void	update_env_var(t_env *env, char *key, char *new_value)
{
	t_env	*current;

	if (!env || !key || !new_value)
		return ;
	current = env;
	while (current)
	{
		if (ft_strcmp(current->key, key) == 0)
		{
			free(current->value);
			current->value = ft_strdup(new_value);
			return ;
		}
		current = current->next;
	}
}
