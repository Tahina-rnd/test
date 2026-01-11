/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_redir.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/21 15:14:21 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/08 23:50:18 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

void	handle_redir(char *input, int *i, t_token **tokens)
{
	char	type;
	char	*value;

	type = input[*i];
	(*i)++;
	if (input[*i] == type)
	{
		if (type == '<')
		{
			value = ft_strdup("<<");
			add_token(tokens, HEREDOC, value);
		}
		else
		{
			value = ft_strdup(">>");
			add_token(tokens, APPEND, value);
		}
		(*i)++;
	}
	else
	{
		if (type == '<')
		{
			value = ft_strdup("<");
			add_token(tokens, REDIRECT_IN, value);
		}
		else
		{
			value = ft_strdup(">");
			add_token(tokens, REDIRECT_OUT, value);
		}
	}
}
