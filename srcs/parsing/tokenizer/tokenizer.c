/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:25:06 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/02 11:23:14 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static t_token	*handle_operator(char *input, int *i)
{
	if (input[*i] == '|')
	{
		(*i)++;
		return (create_token(PIPE, NULL));
	}
	if (input[*i] == '<')
	{
		if (input[*i + 1] == '<')
		{
			if (input[*i + 2] == '<')
			{
				(*i) += 3;
				return (create_token(ERROR, ft_strdup("<<<")));
			}
			(*i) += 2;
			return (create_token(HEREDOC, NULL));
		}
		else
		{
			(*i)++;
			return (create_token(REDIRECT_IN, NULL));
		}
	}
	if (input[*i] == '>')
	{
		if (input[*i + 1] == '>')
		{
			if (input[*i + 2] == '>')
			{
				(*i) += 3;
				return (create_token(ERROR, ft_strdup(">>>")));
			}
			(*i) += 2;
			return (create_token(APPEND, NULL));
		}
		else
		{
			(*i)++;
			return (create_token(REDIRECT_OUT, NULL));
		}
	}
	return (NULL);
}

t_token	*tokenize(char *input)
{
	t_token	*tokens;
	t_token	*new_token;
	int		i;
	char	*word;

	tokens = NULL;
	i = 0;
	if (!input || !*input)
		return (NULL);
	while (input[i])
	{
		while (ft_isspace(input[i]))
			i++;
		if (!input[i])
			break ;
		if (is_operator(input[i]))
		{
			new_token = handle_operator(input, &i);
			if (!new_token)
				return (free_tokens(tokens), NULL);
			add_token_back(&tokens, new_token);
			if (new_token->type == ERROR)
				return (tokens);
		}
		else
		{
			word = extract_word(input, &i);
			if (!word)
			{
				new_token = create_token(ERROR, ft_strdup("unclosed quote"));
				if (new_token)
					add_token_back(&tokens, new_token);
				return (tokens);
			}
			new_token = create_token(WORD, word);
			if (!new_token)
			{
				free(word);
				free_tokens(tokens);
				return (NULL);
			}
			add_token_back(&tokens, new_token);
		}
	}
	new_token = create_token(END, NULL);
	if (!new_token)
		return (free_tokens(tokens), NULL);
	add_token_back(&tokens, new_token);

	return (tokens);
}
