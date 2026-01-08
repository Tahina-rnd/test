/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_ambiguous.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 07:03:05 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/07 14:21:58 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static int	check_redir_token(t_token *file)
{
	if (!file || file->type != WORD)
		return (0);
	if (is_ambiguous_redirect(file->value, file->was_quoted))
	{
		print_ambiguous_error(file->value);
		return (1);
	}
	return (0);
}

int	check_ambiguous_redirects(t_token *tokens)
{
	t_token	*current;

	current = tokens;
	while (current)
	{
		if (current->type == REDIRECT_IN
			|| current->type == REDIRECT_OUT
			|| current->type == APPEND)
		{
			if (check_redir_token(current->next))
				return (1);
		}
		current = current->next;
	}
	return (0);
}
