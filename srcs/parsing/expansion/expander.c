/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:44:36 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/07 17:05:53 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static char	*handle_dollar_expansion(char *str, int *i, char *result,
				t_shell *shell, int in_dq)
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
	if ((str[*i] == '"' || str[*i]=='\'') && !in_dq)
		return (result);
	if (str[*i] == '{')
	{
		var_name = extract_var_name_braces(str, i);
		if (!var_name)
		{
			result = str_append_str(result, "${");
			return (result);
		}
		var_value = get_env_value_copy(shell, var_name);
		if (var_value)
		{
			result = str_append_str(result, var_value);
			free(var_value);
		}
		free(var_name);
		return (result);
	}
	if (is_valid_var_char(str[*i]))
	{
		var_name = extract_var_name_simple(str, i);
		if (!var_name)
		{
			result = str_append_char(result, '$');
			return (result);
		}
		var_value = get_env_value_copy(shell, var_name);
		if (var_value)
		{
			result = str_append_str(result, var_value);
			free(var_value);
		}
		free(var_name);
		return (result);
	}
	result = str_append_char(result, '$');
	return (result);
}

void	expand_tokens(t_token *tokens, t_shell *shell)
{
	t_token	*current;
	t_token	*prev;
	char	*expanded;
	int		original_was_quoted;

	current = tokens;
	prev = NULL;
	while (current)
	{
		if (current->type == WORD && (!prev || prev->type != HEREDOC))
		{
			original_was_quoted = current->was_quoted;  // Sauvegarder
			// DEBUG: AJOUTER CES LIGNES
			// printf("🔍 DEBUG expand AVANT: value='%s', was_quoted=%d\n", 
			//        current->value, current->was_quoted);
			expanded = expand_string(current->value, shell);
			free(current->value);
			current->value = expanded;
			current->was_quoted = original_was_quoted;  // RESTAURER
			// DEBUG: AJOUTER CES LIGNES
			// printf("🔍 DEBUG expand APRES: value='%s', was_quoted=%d\n", 
			//        current->value, current->was_quoted);
		}
		prev = current;
		current = current->next;
	}
}

char	*expand_string(char *str, t_shell *shell)
{
	char	*result;
	int		i;
	int		in_sq;
	int		in_dq;

	result = ft_strdup("");
	i = 0;
	in_sq = 0;
	in_dq = 0;

	while (str[i])
	{
		if (str[i] == '\'' && !in_dq)
		{
			in_sq = !in_sq;
			i++;
		}
		else if (str[i] == '"' && !in_sq)
		{
			in_dq = !in_dq;
			i++;
		}
		else if (str[i] == '$' && !in_sq)
			result = handle_dollar_expansion(str, &i, result, shell, in_dq);
		else
			result = str_append_char(result, str[i++]);
	}
	return (result);
}


