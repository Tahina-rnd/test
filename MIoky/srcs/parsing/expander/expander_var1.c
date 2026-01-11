/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_var1.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:44:36 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/09 12:06:49 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"


char	*get_env_value(t_env *env, char *key)
{
	if (!env || !key)
		return (NULL);
	while (env)
	{
		if (ft_strcmp(env->key, key) == 0)
			return (env->value);
		env = env->next;
	}
	return (NULL);
}

char	*expand_exit_status(int status)
{
	return (ft_itoa(status));
}

int	is_valid_var_char(char c)
{
	return (ft_isalnum(c) || c == '_');
}

char	*extract_var_name(char *str, int *i)
{
	int		start;
	int		len;
	char	*var_name;

	start = *i;
	while (str[*i] && is_valid_var_char(str[*i]))
		(*i)++;
	len = *i - start;
	if (len == 0)
		return (NULL);
	var_name = (char *)malloc(len + 1);
	if (!var_name)
		return (NULL);
	ft_memcpy(var_name, str + start, len);
	var_name[len] = '\0';
	return (var_name);
}

char	*append_string(char *result, char *str)
{
	char	*tmp;

	if (!str)
		return (result);
	if (!result)
		return (ft_strdup(str));
	tmp = ft_strjoin(result, str);
	free(result);
	return (tmp);
}
