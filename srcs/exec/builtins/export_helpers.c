/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export_helpers.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 23:05:52 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/13 06:26:03 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

t_env	*exp_create_env_node(const char *key, const char *value)
{
	t_env	*new_node;

	new_node = (t_env *)malloc(sizeof(t_env));
	if (!new_node)
		return (NULL);
	new_node->key = ft_strdup(key);
	if (value != NULL)
		new_node->value = strdup(value);
	else
		new_node->value = NULL;
	new_node->next = NULL;
	return (new_node);
}

char	*extract_key(const char *arg)
{
	int		i;
	char	*key;

	i = 0;
	while (arg[i] && arg[i] != '=')
		i++;
	key = malloc(sizeof(char) * (i + 1));
	if (!key)
		return (NULL);
	i = 0;
	while (arg[i] && arg[i] != '=')
	{
		key[i] = arg[i];
		i++;
	}
	key[i] = '\0';
	return (key);
}

char	*extract_value(const char *arg)
{
	char	*equal_sign;
	char	*value;

	equal_sign = ft_strchr(arg, '=');
	if (!equal_sign)
		return (NULL);
	value = ft_strdup(equal_sign + 1);
	return (value);
}

int	update_or_add_env(t_shell *shell, char *key, char *value)
{
	t_env	*current;

	current = shell->env;
	while (current)
	{
		if (ft_strcmp(current->key, key) == 0)
		{
			free(current->value);
			current->value = ft_strdup(value);
			return (0);
		}
		current = current->next;
	}
	exp_add_env_node_back(&shell->env, exp_create_env_node(key, value));
	return (0);
}
