/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_process2.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:44:36 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/13 16:17:11 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static int	expand_redir_file(t_redir *redir, t_shell *shell)
{
	char	*expanded;

	if (ft_strchr(redir->file, '$'))
	{
		expanded = expand_text(redir->file, shell);
		if (!expanded || ft_strlen(expanded) == 0 || ft_strchr(expanded, ' '))
		{
			printf("Minishell: %s: ambiguous redirect\n", redir->file);
			if (expanded)
				free(expanded);
			free(redir->file);
			redir->file = NULL;
			return (0);
		}
		free(redir->file);
		redir->file = expanded;
	}
	return (1);
}

int	process_redirs(t_redir *lst, t_shell *shell)
{
	int	status;

	status = 1;
	while (lst)
	{
		if (!expand_redir_file(lst, shell))
			status = 0;
		lst = lst->next;
	}
	return (status);
}
