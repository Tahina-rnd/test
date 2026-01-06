/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   extract_word.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/19 11:47:28 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/02 05:40:41 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static void	switch_quote(char c, int *sq, int *dq)
{
	if (c == '\'' && *dq == 0)
		*sq = !(*sq);
	else if (c == '"' && *sq == 0)
		*dq = !(*dq);
}

static int	is_limit(char c, int sq, int dq)
{
	if (sq || dq)
		return (0);
	if (ft_isspace(c) || is_operator(c))
		return (1);
	return (0);
}

char	*extract_word(char *s, int *i)
{
	int		start;
	int		sq;
	int		dq;

	start = *i;
	sq = 0;
	dq = 0;
	while (s[*i])
	{
		switch_quote(s[*i], &sq, &dq);
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
