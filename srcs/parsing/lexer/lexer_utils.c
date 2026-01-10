/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lex_quotes.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:19:16 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/10 11:59:37 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

int	is_whitespace(char c)
{
	return (c == ' ' || c == '\t' || c == '\n'
		|| c == '\v' || c == '\f' || c == '\r');
}

int	is_operator(char c)
{
	return (c == '|' || c == '<' || c == '>');
}

void	token_add_back(t_token **lst, t_token *new)
{
	t_token	*temp;

	if (!lst || !new)
		return ;
	if (!*lst)
	{
		*lst = new;
		return ;
	}
	temp = *lst;
	while (temp->next)
		temp = temp->next;
	temp->next = new;
}

void	segment_add_back(t_segment **lst, t_segment *new)
{
	t_segment	*temp;

	if (!lst || !new)
		return ;
	if (!*lst)
	{
		*lst = new;
		return ;
	}
	temp = *lst;
	while (temp->next)
		temp = temp->next;
	temp->next = new;
}

void	free_tokens(t_token *tokens)
{
	t_token		*tmp_tok;
	t_segment	*tmp_seg;
	t_segment	*next_seg;

	while (tokens)
	{
		tmp_tok = tokens;
		tokens = tokens->next;
		if (tmp_tok->literal)
			free(tmp_tok->literal);
		tmp_seg = tmp_tok->segments;
		while (tmp_seg)
		{
			next_seg = tmp_seg->next;
			if (tmp_seg->value)
				free(tmp_seg->value);
			free(tmp_seg);
			tmp_seg = next_seg;
		}
		free(tmp_tok);
	}
}
