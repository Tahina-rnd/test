/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:44:36 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/06 16:43:25 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static char	*handle_dollar_expansion(char *str, int *i, char *result,
				t_shell *shell)
{
	char	*var_name;
	char	*var_value;

	(*i)++;
	if (str[*i] == '?')
	{
		var_value = ft_itoa(shell->last_exit_status);
		result = str_append_str(result, var_value);
		free(var_value);
		(*i)++;
		return (result);
	}
	if (str[*i] == '{')
		var_name = extract_var_name_braces(str, i);
	else if (is_valid_var_char(str[*i]))
		var_name = extract_var_name_simple(str, i);
	else
	{
		result = str_append_char(result, '$');
		return (result);
	}
	if (var_name)
	{
		var_value = get_env_value_copy(shell, var_name);
		if (var_value)
		{
			result = str_append_str(result, var_value);
			free(var_value);
		}
		else
		{
			free(var_name);
			return (result);
		}
		free(var_name);
	}
	return (result);
}

void	expand_tokens(t_token *tokens, t_shell *shell)
{
	t_token	*current;
	t_token	*prev;
	char	*expanded;

	current = tokens;
	prev = NULL;
	while (current)
	{
		if (current->type == WORD && (!prev || prev->type != HEREDOC))
		{
			expanded = expand_string(current->value, shell);
			free(current->value);
			current->value = expanded;
		}
		prev = current;
		current = current->next;
	}
}

char	*expand_string(char *str, t_shell *shell)
{
	char	*result;
	int		i;
	int		in_single_quote;
	int		in_double_quote;

	result = ft_strdup("");
	i = 0;
	in_single_quote = 0;
	in_double_quote = 0;

	while (str[i])
	{
		if (str[i] == '\'' && !in_double_quote)
		{
			if (str[i + 1] == '\'')
			{
				// result = str_append_char(result, ' ');
				i += 2;
			}
			else
			{
				in_single_quote = !in_single_quote;
				i++;
			}
		}
		else if (str[i] == '"' && !in_single_quote)
		{
			if (str[i + 1] == '"')
			{
				result = str_append_char(result, ' ');
				i += 2;
			}
			else
			{
				in_double_quote = !in_double_quote;
				i++;
			}
		}
		else if (str[i] == '$' && !in_single_quote)
		{
			char *temp = result;
			result = handle_dollar_expansion(str, &i, result, shell);
			if (result == temp)
			{
				result = str_append_char(result, '$');
				i--;
			}
		}
		else
			result = str_append_char(result, str[i++]);
	}
	return (result);
}

