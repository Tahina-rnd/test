/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_var2.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 10:01:54 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/09 12:08:30 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static char	*append_char_to_str(char *result, char c)
{
	char	*tmp;
	char	str[2];

	str[0] = c;
	str[1] = '\0';
	if (!result)
		return (ft_strdup(str));
	tmp = ft_strjoin(result, str);
	free(result);
	return (tmp);
}

static char	*handle_dollar(char *str, int *i, t_env *env, int exit_status)
{
	char	*var_name;
	char	*value;
	char	*result;

	(*i)++;
	if (str[*i] == '?')
	{
		(*i)++;
		return (expand_exit_status(exit_status));
	}
	if (!is_valid_var_char(str[*i]))
		return (ft_strdup("$"));
	var_name = extract_var_name(str, i);
	if (!var_name)
		return (ft_strdup(""));
	value = get_env_value(env, var_name);
	free(var_name);
	if (!value)
		return (ft_strdup(""));
	result = ft_strdup(value);
	return (result);
}

char	*expand_variables(char *str, t_env *env, int last_exit_status)
{
	char	*result;
	char	*tmp;
	int		i;

	result = ft_strdup("");
	i = 0;
	while (str[i])
	{
		if (str[i] == '$')
		{
			tmp = handle_dollar(str, &i, env, last_exit_status);
			result = append_string(result, tmp);
			free(tmp);
		}
		else
		{
			result = append_char_to_str(result, str[i]);
			i++;
		}
	}
	return (result);
}
