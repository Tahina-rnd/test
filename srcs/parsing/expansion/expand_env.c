/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_env.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 10:39:18 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/07 15:50:00 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

char	*get_env_value_copy(t_shell *shell, char *name)
{
	t_env	*current;

	if (!name || !shell || !shell->env)
		return (NULL);
	current = shell->env;
	while (current)
	{
		if (ft_strcmp(current->key, name) == 0)
		{
			if (current->value)
				return (ft_strdup(current->value));
			else
				return (ft_strdup(""));
		}
		current = current->next;
	}
	return (NULL);
}

char	*extract_var_name_braces(char *str, int *i)
{
	int	start;
	int	len;

	(*i)++;
	start = *i;
	len = 0;
	if (!str[*i] || str[*i] == '}')
	{
		return (NULL);
	}
	if (!ft_isalpha(str[*i]) && str[*i] != '_')
	{
		return (NULL);
	}
	while (str[*i] && str[*i] != '}')
	{
		if (!is_valid_var_char(str[*i]))
		{
			return (NULL);
		}
		len++;
		(*i)++;
	}
	if (str[*i] != '}')
	{
		return (NULL);
	}
	(*i)++;
	return (ft_substr(str, start, len));
}

char	*extract_var_name_simple(char *str, int *i)
{
	int	start;
	int	len;

	start = *i;
	len = 0;
	if (!ft_isalpha(str[*i]) && str[*i] != '_')
		return (NULL);
	while (str[*i] && is_valid_var_char(str[*i]))
	{
		len++;
		(*i)++;
	}
	if (len == 0)
		return (NULL);
	return (ft_substr(str, start, len));
}

int	is_valid_var_char(char c)
{
	return (ft_isalnum(c) || c == '_');
}
