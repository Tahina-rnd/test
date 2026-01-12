/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_free.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:57:18 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/12 12:49:28 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

void	free_segments(t_segment *seg)
{
	t_segment	*tmp;

	while (seg)
	{
		tmp = seg;
		seg = seg->next;
		if (tmp->value)
			free(tmp->value);
		free(tmp);
	}
}

static void	free_redirs(t_redir *lst)
{
	t_redir	*temp;

	while (lst)
	{
		temp = lst;
		lst = lst->next;
		if (temp->file)
			free(temp->file);
		free(temp);
	}
}

static void	free_args(t_arg *lst)
{
	t_arg	*temp;

	while (lst)
	{
		temp = lst;
		lst = lst->next;
		if (temp->value)
			free(temp->value);
		if (temp->segments)
			free_segments(temp->segments);
		free(temp);
	}
}

void	free_commands(t_command *cmds)
{
	t_command	*temp;

	while (cmds)
	{
		temp = cmds;
		cmds = cmds->next;
		free_args(temp->args);
		free_redirs(temp->input_redirection);
		free_redirs(temp->output_redirection);
		free_redirs(temp->heredoc);
		free(temp);
	}
}
