/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:19:16 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/08 22:07:00 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

t_token	*create_token(t_token_type type, char *value)
{
	t_token	*token;

	token = (t_token *)malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->type = type;
	token->value = value;
	token->was_quoted = 0;
	token->next = NULL;
	return (token);
}

void	free_tokens(t_token *tokens)
{
	t_token	*tmp;

	while (tokens)
	{
		tmp = tokens;
		tokens = tokens->next;
		if (tmp->value)
			free(tmp->value);
		free(tmp);
	}
}

t_token	*get_last_token(t_token *tokens)
{
	if (!tokens)
		return (NULL);
	while (tokens->next)
		tokens = tokens->next;
	return (tokens);
}

void	add_token(t_token **tokens, t_token_type type, char *value)
{
	t_token	*new_token;
	t_token	*last;

	new_token = create_token(type, value);
	if (!new_token)
	{
		if (value)
			free(value);
		return ;
	}
	if (!*tokens)
	{
		*tokens = new_token;
		return ;
	}
	last = get_last_token(*tokens);
	last->next = new_token;
}

int	is_quote(char c)
{
	return (c == '\'' || c == '"');
}
