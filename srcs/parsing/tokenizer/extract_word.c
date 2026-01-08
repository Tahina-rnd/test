/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   extract_word.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/19 11:47:28 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/07 06:29:00 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static void	switch_quote(char c, int *sq, int *dq, int *was_quoted)
{
	if (c == '\'' && *dq == 0)
	{
		*sq = !(*sq);
		*was_quoted = 1;
	}
	else if (c == '"' && *sq == 0)
	{
		*dq = !(*dq);
		*was_quoted = 1;
	}
}

static int	is_limit(char c, int sq, int dq)
{
	if (sq || dq)
		return (0);
	if (ft_isspace(c) || is_operator(c))
		return (1);
	return (0);
}

char	*extract_word(char *s, int *i, int *was_quoted)
{
	int		start;
	int		sq;
	int		dq;

	start = *i;
	sq = 0;
	dq = 0;
	*was_quoted = 0;	
	while (s[*i])
	{
		switch_quote(s[*i], &sq, &dq, was_quoted);
		if (is_limit(s[*i], sq, dq))
			break ;
		(*i)++;
	}
	if (sq || dq)
	{
		printf("minishell: syntax error: unclosed quote\n");
		return (NULL);
	}
	return (ft_substr(s, start, *i - start));
}
