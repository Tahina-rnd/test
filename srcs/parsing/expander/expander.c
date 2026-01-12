/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:44:36 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/12 09:13:06 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static void	splice_args(t_command *cmd, t_arg *prev, t_arg *curr, t_arg *new_l)
{
	t_arg	*last_new;

	if (new_l)
	{
		if (prev)
			prev->next = new_l;
		else
			cmd->args = new_l;
		last_new = new_l;
		while (last_new->next)
			last_new = last_new->next;
		last_new->next = curr->next;
	}
	else
	{
		if (prev)
			prev->next = curr->next;
		else
			cmd->args = curr->next;
	}
}

static void	process_arg_list(t_shell *shell, t_command *c)
{
	t_arg	*curr;
	t_arg	*prev;
	t_arg	*next_save;
	t_arg	*expanded_list;

	curr = c->args;
	prev = NULL;
	while (curr)
	{
		next_save = curr->next;
		expanded_list = expand_arg_to_list(curr, shell);
		splice_args(c, prev, curr, expanded_list);
		if (expanded_list)
		{
			prev = expanded_list;
			while (prev->next != next_save)
				prev = prev->next;
		}
		if (curr->value)
			free(curr->value);
		free_segments(curr->segments);
		free(curr);
		curr = next_save;
	}
}

void	expander(t_shell *shell, t_command *cmd)
{
	t_command	*c;

	c = cmd;
	while (c)
	{
		process_arg_list(shell, c);
		if (!process_redirs(c->input_redirection, shell)
			|| !process_redirs(c->output_redirection, shell))
			shell->last_exit_status = 1;
		c = c->next;
	}
}
