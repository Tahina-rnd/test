/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ambiguous_redir.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 06:31:37 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/07 14:11:57 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static int	has_spaces(char *str)
{
	int	i;

	if (!str)
		return (0);
	i = 0;
	while (str[i])
	{
		if (ft_isspace(str[i]))
			return (1);
		i++;
	}
	return (0);
}

int	is_ambiguous_redirect(char *filename, int was_quoted)
{
	if (was_quoted)
		return (0);
	if (!filename || filename[0] == '\0')
		return (1);
	if (has_spaces(filename))
		return (1);
	return (0);
}

void	print_ambiguous_error(char *filename)
{
	ft_putstr_fd("minishell: ", 2);
	if (filename && filename[0])
		ft_putstr_fd(filename, 2);
	ft_putstr_fd(": ambiguous redirect\n", 2);
}
