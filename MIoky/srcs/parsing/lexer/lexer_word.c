/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_word.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:21:01 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/08 23:46:21 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

int	is_special_char(char c)
{
	return (c == '|' || c == '<' || c == '>' || c == ' '
		|| c == '\t' || c == '\n');
}

static int	append_char(char **result, char c)
{
	char	*new_str;
	int		len;

	if (!result)
		return (0);
	len = 0;
	if (*result)
		len = ft_strlen(*result);
	new_str = malloc(len + 2);
	if (!new_str)
		return (0);
	if (*result)
	{
		ft_memcpy(new_str, *result, len);
		free(*result);
	}
	new_str[len] = c;
	new_str[len + 1] = '\0';
	*result = new_str;
	return (1);
}

int	handle_quotes(char *input, int *i, char quote, char **result)
{
	(*i)++;
	while (input[*i] && input[*i] != quote)
	{
		if (!append_char(result, input[*i]))
			return (0);
		(*i)++;
	}
	if (input[*i] == '\0')
		return (0);
	(*i)++;
	return (1);
}

static int	process_char(char *input, int *i, char **result, int *was_quoted)
{
	if (is_quote(input[*i]))
	{
		*was_quoted = 1;
		if (!handle_quotes(input, i, input[*i], result))
			return (-1);
	}
	else if (!is_special_char(input[*i]))
	{
		if (!append_char(result, input[*i]))
			return (0);
		(*i)++;
	}
	else
		return (0);
	return (1);
}

char	*extract_word(char *input, int *i, int *was_quoted)
{
	char	*result;
	int		ret;

	*was_quoted = 0;
	result = ft_strdup("");
	if (!result)
		return (NULL);
	while (input[*i] && !is_special_char(input[*i]))
	{
		ret = process_char(input, i, &result, was_quoted);
		if (ret == 0)
			break ;
		else if (ret == -1)
		{
			ft_putendl_fd("minishell: syntax error: unclosed quotes", 2);
			free(result);
			return (NULL);
		}
	}
	if (result && result[0] == '\0' && *was_quoted == 0)
	{
		free(result);
		return (NULL);
	}
	return (result);
}
