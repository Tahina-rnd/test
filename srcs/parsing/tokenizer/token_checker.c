/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token_checker.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/21 15:14:21 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/07 15:36:39 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static int	print_syntax_error(char *token_value)
{
	if (token_value)
		printf("minishell: syntax error near unexpected token `%s'\n",
			token_value);
	else
		printf("minishell: syntax error near unexpected token `newline'\n");
	return (1);
}

static int	is_redirection(t_token_type type)
{
	return (type == REDIRECT_IN || type == REDIRECT_OUT
		|| type == APPEND || type == HEREDOC);
}

int	check_tokens(t_token *tokens)
{
	t_token	*current;
	t_token	*prev;

	current = tokens;
	prev = NULL;
	while (current)
	{
		if (current->type == ERROR)
			return (print_syntax_error(current->value));
		if (current->type == PIPE)
		{
			if (prev == NULL || prev->type == PIPE)
				return (print_syntax_error("|"));
			if (current->next == NULL || current->next->type == PIPE
				|| current->next->type == END)
				return (print_syntax_error("|"));
		}
		else if (is_redirection(current->type))
		{
			if (current->next == NULL || current->next->type == END
				|| current->next->type != WORD)
			{
				if (current->type == REDIRECT_IN)
					return (print_syntax_error("<"));
				else if (current->type == REDIRECT_OUT)
					return (print_syntax_error(">"));
				else if (current->type == APPEND)
					return (print_syntax_error(">>"));
				else if (current->type == HEREDOC)
					return (print_syntax_error("<<"));
			}
		}
		prev = current;
		current = current->next;
	}
	return (0);
}
