/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_to_array.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 22:42:18 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/14 16:16:07 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

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

static int	fill_env_array(char **array, t_env *env)
{
	t_env	*current;
	int		i;

	current = env;
	i = 0;
	while (current)
	{
		array[i] = create_env_string(current->key, current->value);
		if (!array[i])
		{
			while (i > 0)
				free(array[--i]);
			return (0);
		}
		current = current->next;
		i++;
	}
	array[i] = NULL;
	return (1);
}

char	**env_to_array(t_env *env)
{
	char	**array;
	int		size;

	size = count_env_vars(env);
	array = malloc(sizeof(char *) * (size + 1));
	if (!array)
		return (NULL);
	if (!fill_env_array(array, env))
	{
		free(array);
		return (NULL);
	}
	return (array);
}
