/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_to_array.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 22:42:18 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/14 20:31:29 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static char	*create_env_string(const char *key, const char *value)
{
	char	*result;
	char	*tmp;

	if (!value)
		return (ft_strdup(key));
	tmp = ft_strjoin(key, "=");
	if (!tmp)
		return (NULL);
	result = ft_strjoin(tmp, value);
	free(tmp);
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
