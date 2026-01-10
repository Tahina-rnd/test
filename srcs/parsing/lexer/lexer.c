/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:25:06 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/10 09:18:32 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static void	skip_whitespace(char *input, int *i)
{
	while (input[*i] && is_whitespace(input[*i]))
		(*i)++;
}

static t_token	*get_next_token(char *input, int *i)
{
	t_token	*token;

	if (is_operator(input[*i]))
		token = handle_operator(input, i);
	else
		token = handle_word(input, i);
	return (token);
}

t_token	*lexer(char *input)
{
	t_token	*head;
	t_token	*new_tok;
	int		i;

	head = NULL;
	i = 0;
	while (input[i])
	{
		skip_whitespace(input, &i);
		if (!input[i])
			break ;
		new_tok = get_next_token(input, &i);
		if (!new_tok)
		{
			free_tokens(head);
			return (NULL);
		}
		token_add_back(&head, new_tok);
	}
	token_add_back(&head, create_token(END, NULL));
	return (head);
}
