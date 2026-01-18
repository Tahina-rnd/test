/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_duplicate.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 14:28:37 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/15 12:06:00 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

t_env	*create_env_node(const char *key, const char *value)
{
	t_env	*new_node;

	new_node = (t_env *)malloc(sizeof(t_env));
	if (!new_node)
		return (NULL);
	new_node->key = ft_strdup(key);
	if (!new_node->key)
	{
		free(new_node);
		return (NULL);
	}
	if (value != NULL)
	{
		new_node->value = ft_strdup(value);
		if (!new_node->value)
		{
			free(new_node->key);
			free(new_node);
			return (NULL);
		}
	}
	else
		new_node->value = NULL;
	new_node->next = NULL;
	return (new_node);
}

void	add_env_node_back(t_env **head, t_env *new_node)
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

static t_env	*extract_env_node(char *str)
{
	int		j;
	char	*key;
	char	*value;
	t_env	*node;

	j = 0;
	while (str[j] && str[j] != '=')
		j++;
	key = ft_substr(str, 0, j);
	if (!key)
		return (NULL);
	value = ft_strdup(str + j + 1);
	if (!value)
	{
		free(key);
		return (NULL);
	}
	node = create_env_node(key, value);
	free(key);
	free(value);
	return (node);
}

t_env	*dup_env(char **envp)
{
	t_env	*env_list;
	t_env	*node;
	int		i;

	env_list = NULL;
	i = 0;
	while (envp && envp[i])
	{
		if (ft_strchr(envp[i], '='))
		{
			node = extract_env_node(envp[i]);
			if (!node)
			{
				free_env_list(env_list);
				return (NULL);
			}
			add_env_node_back(&env_list, node);
		}
		i++;
	}
	return (env_list);
}
