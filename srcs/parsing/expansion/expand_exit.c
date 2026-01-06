/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_exit.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 10:39:50 by tarandri          #+#    #+#             */
/*   Updated: 2025/12/29 16:04:08 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static char	*expand_exit_code(t_shell *shell)
{
	char	*exit_code_str;

	exit_code_str = ft_itoa(shell->last_exit_status);
	return (exit_code_str);
}

char	*handle_exit_code_expansion(t_shell *shell, char *str, int *i)
{
	char	*result;

	if (str[*i] == '?')
	{
		(*i)++;
		result = expand_exit_code(shell);
	}
	else if (str[*i] == '{' && str[*i + 1] == '?')
	{
		*i += 2;
		if (str[*i] == '}')
		{
			(*i)++;
			result = expand_exit_code(shell);
		}
		else
			result = ft_strdup("${?");
	}
	else
	{
		result = NULL;
	}
	return (result);
}
