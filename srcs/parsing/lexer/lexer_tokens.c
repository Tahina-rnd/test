/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_tokens.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:19:16 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/12 08:55:22 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

t_token	*create_token(t_token_type type, char *literal)
{
	t_token	*token;

	token = malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->type = type;
	token->literal = literal;
	token->segments = NULL;
	token->next = NULL;
	return (token);
}

static t_token_type	get_op_type(char *s)
{
	if (!ft_strncmp(s, "<<", 2))
		return (HEREDOC);
	if (!ft_strncmp(s, ">>", 2))
		return (APPEND);
	if (!ft_strncmp(s, "<", 1))
		return (REDIRECT_IN);
	if (!ft_strncmp(s, ">", 1))
		return (REDIRECT_OUT);
	if (!ft_strncmp(s, "|", 1))
		return (PIPE);
	return (ERROR);
}

t_token	*handle_operator(char *input, int *i)
{
	t_token_type	type;
	char			*literal;
	int				len;

	len = 1;
	if ((input[*i] == '<' && input[*i + 1] == '<')
		|| (input[*i] == '>' && input[*i + 1] == '>'))
		len = 2;
	literal = ft_substr(input, *i, len);
	type = get_op_type(literal);
	*i += len;
	return (create_token(type, literal));
}

t_token	*handle_word(char *input, int *i)
{
	t_token	*token;
	int		start;

	start = *i;
	token = create_token(WORD, NULL);
	if (!token)
		return (NULL);
	while (input[*i] && !is_whitespace(input[*i]) && !is_operator(input[*i]))
	{
		if (!extract_segment(token, input, i))
		{
			free_tokens(token);
			return (NULL);
		}
	}
	token->literal = ft_substr(input, start, *i - start);
	return (token);
}
