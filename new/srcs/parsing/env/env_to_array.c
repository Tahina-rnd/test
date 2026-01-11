/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_to_array.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 22:42:18 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/11 13:54:25 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static int	count_env_nodes(t_env *env)
{
	int		count;
	t_env	*current;

	count = 0;
	current = env;
	while (current)
	{
		count++;
		current = current->next;
	}
	return (count);
}

static char	*create_env_string(const char *key, const char *value)
{
	int		key_len;
	int		value_len;
	char	*result;
	int		i;
	int		j;

	key_len = ft_strlen(key);
	value_len = ft_strlen(value);
	result = malloc(sizeof(char) * (key_len + value_len + 2));
	if (!result)
		return (NULL);
	i = 0;
	while (key[i])
	{
		result[i] = key[i];
		i++;
	}
	result[i++] = '=';
	j = 0;
	while (value[j])
		result[i++] = value[j++];
	result[i] = '\0';
	return (result);
}

// mety tsy miasa an
char	**env_to_array(t_env *env)
{
	char	**array;
	t_env	*current;
	int		i;

	array = malloc(sizeof(char *) * (count_env_nodes(env) + 1));
	if (!array)
		return (NULL);
	current = env;
	i = 0;
	while (current)
	{
		array[i] = create_env_string(current->key, current->value);
		if (!array[i])
		{
			while (--i >= 0)
				free(array[i]);
			free(array);
			return (NULL);
		}
		current = current->next;
		i++;
	}
	array[i] = NULL;
	return (array);
}

void	free_env_array(char **env_array)
{
	int	i;

	if (!env_array)
		return ;
	i = 0;
	while (env_array[i])
	{
		free(env_array[i]);
		i++;
	}
	free(env_array);
}
