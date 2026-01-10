/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_var2.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 10:01:54 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/10 07:40:07 by tarandri         ###   ########.fr       */
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

static void	handle_dquote(char *str, int *i, char **result, t_env *env, int es)
{
	char	*tmp;

	// *result = append_char_to_str(*result, '"');
	(*i)++;
	while (str[*i] && str[*i] != '"')
	{
		if (str[*i] == '$')
		{
			tmp = handle_dollar(str, i, env, es);
			*result = append_string(*result, tmp);
			free(tmp);
		}
		else
		{
			*result = append_char_to_str(*result, str[*i]);
			(*i)++;
		}
	}
	if (str[*i] == '"')
	{
		// *result = append_char_to_str(*result, '"');
		(*i)++;
	}
}

static void	handle_squote(char *str, int *i, char **result)
{
	// *result = append_char_to_str(*result, '\'');
	(*i)++;
	while (str[*i] && str[*i] != '\'')
	{
		*result = append_char_to_str(*result, str[*i]);
		(*i)++;
	}
	if (str[*i] == '\'')
	{
		// *result = append_char_to_str(*result, '\'');
		(*i)++;
	}
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
		if (str[i] == '\'')
			handle_squote(str, &i, &result);
		else if (str[i] == '"')
			handle_dquote(str, &i, &result, env, last_exit_status);
		else if (str[i] == '$')
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
