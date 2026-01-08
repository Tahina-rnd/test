/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   appli_ws.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 14:29:54 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/07 15:23:17 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static void	replace_token_with_splits(t_token **tokens, t_token *to_split,
		char **splits)
{
	t_token	*current;
	t_token	*prev;
	t_token	*new_token;
	int		i;

	prev = NULL;
	current = *tokens;
	while (current && current != to_split)
	{
		prev = current;
		current = current->next;
	}
	i = 0;
	while (splits[i])
	{
		new_token = create_token(WORD, ft_strdup(splits[i]));
		if (!new_token)
			return ;
		new_token->was_quoted = 0;
		if (prev)
		{
			new_token->next = prev->next;
			prev->next = new_token;
		}
		else
		{
			new_token->next = *tokens;
			*tokens = new_token;
		}
		prev = new_token;
		i++;
	}
	if (prev && prev->next == to_split)
		prev->next = to_split->next;
	else if (*tokens == to_split)
		*tokens = to_split->next;
	free(to_split->value);
	free(to_split);
}

void	apply_word_splitting(t_token **tokens)
{
	t_token	*current;
	t_token	*next;
	char	**splits;
	int		i;

	current = *tokens;
	while (current)
	{
		next = current->next;
		if (current->type == WORD && !current->was_quoted
			&& current->value && ft_strchr(current->value, ' '))
		{
			splits = split_words(current->value);
			if (splits)
			{
				replace_token_with_splits(tokens, current, splits);
				i = 0;
				while (splits[i])
					free(splits[i++]);
				free(splits);
			}
		}
		current = next;
	}
}
