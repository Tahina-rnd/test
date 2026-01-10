/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_word.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:21:01 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/10 11:58:49 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

t_segment	*create_segment(char *value, t_quote_type type)
{
	t_segment	*seg;

	seg = malloc(sizeof(t_segment));
	if (!seg)
		return (NULL);
	seg->value = value;
	seg->quote = type;
	seg->next = NULL;
	return (seg);
}

static int	get_quoted_len(char *input, char quote)
{
	int	i;

	i = 1;
	while (input[i] && input[i] != quote)
		i++;
	if (!input[i])
	{
		printf("Error: Unclosed quote\n");
		return (-1);
	}
	return (i);
}

static int	get_unquoted_len(char *input)
{
	int	i;

	i = 0;
	while (input[i] && !is_whitespace(input[i]) && !is_operator(input[i])
		&& input[i] != '\'' && input[i] != '"')
		i++;
	return (i);
}

int	extract_segment(t_token *token, char *input, int *i)
{
	t_segment	*seg;
	char		*value;
	int			len;
	t_quote_type type;

	type = QUOTE_NONE;
	if (input[*i] == '\'' || input[*i] == '"')
	{
		if (input[*i] == '\'')
			type = QUOTE_SINGLE;
		else
			type = QUOTE_DOUBLE;
		len = get_quoted_len(input + *i, input[*i]);
		if (len == -1)
			return (0);
		value = ft_substr(input, *i + 1, len - 1);
		*i += len + 1;
	}
	else
	{
		len = get_unquoted_len(input + *i);
		value = ft_substr(input, *i, len);
		*i += len;
	}
	seg = create_segment(value, type);
	segment_add_back(&(token->segments), seg);
	return (1);
}

