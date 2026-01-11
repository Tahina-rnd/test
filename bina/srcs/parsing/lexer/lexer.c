/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:25:06 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/08 23:51:29 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

void	skip_spaces(char *input, int *i)
{
	while (input[*i] && (input[*i] == ' ' || input[*i] == '\t'
			|| input[*i] == '\n'))
		(*i)++;
}

int	is_pipe(char c)
{
	return (c == '|');
}

int	is_redirection(char *input, int i)
{
	if (input[i] == '<' || input[i] == '>')
		return (1);
	return (0);
}

void	handle_word(char *input, int *i, t_token **tokens)
{
	char	*word;
	int		was_quoted;
	t_token	*last;

	was_quoted = 0;
	word = extract_word(input, i, &was_quoted);
	if (!word)
	{
		add_token(tokens, ERROR, NULL);
		return ;
	}
	add_token(tokens, WORD, word);
	if (*tokens)
	{
		last = get_last_token(*tokens);
		if (last && last->type == WORD)
			last->was_quoted = was_quoted;
	}
}

t_token	*lexer(char *input)
{
	int		i;
	t_token	*tokens;

	i = 0;
	tokens = NULL;
	while (input[i])
	{
		skip_spaces(input, &i);
		if (!input[i])
			break ;
		if (is_pipe(input[i]))
		{
			add_token(&tokens, PIPE, ft_strdup("|"));
			i++;
		}
		else if (is_redirection(input, i))
			handle_redir(input, &i, &tokens);
		else
			handle_word(input, &i, &tokens);
		if (tokens && get_last_token(tokens)->type == ERROR)
		{
			free_tokens(tokens);
			return (NULL);
		}
	}
	add_token(&tokens, END, NULL);
	return (tokens);
}
