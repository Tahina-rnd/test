/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_word.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:44:36 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/13 16:23:46 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static t_arg	*handle_unquoted(t_arg *curr, char *val)
{
	int		i;
	char	buf[2];

	i = 0;
	buf[1] = '\0';
	while (val[i])
	{
		if (val[i] == ' ' || val[i] == '\t' || val[i] == '\n')
		{
			if (curr->value != NULL)
			{
				curr->next = new_arg_node();
				curr = curr->next;
			}
			while (val[i] && (val[i] == ' '
					|| val[i] == '\t' || val[i] == '\n'))
				i++;
			continue ;
		}
		buf[0] = val[i];
		append_val(curr, buf);
		i++;
	}
	return (curr);
}

static t_arg	*process_segments(t_arg *head, t_arg *arg, t_shell *shell)
{
	t_arg		*curr;
	t_segment	*seg;
	char		*exp_val;

	curr = head;
	seg = arg->segments;
	while (seg)
	{
		if (seg->quote == QUOTE_SINGLE)
			append_val(curr, seg->value);
		else
		{
			exp_val = expand_text(seg->value, shell);
			if (seg->quote == QUOTE_DOUBLE)
				append_val(curr, exp_val);
			else
				curr = handle_unquoted(curr, exp_val);
			free(exp_val);
		}
		seg = seg->next;
	}
	return (curr);
}

t_arg	*expand_arg_to_list(t_arg *arg, t_shell *shell)
{
	t_arg	*head;
	t_arg	*curr;

	head = new_arg_node();
	curr = process_segments(head, arg, shell);
	if (curr->value == NULL && head == curr)
	{
		free(head);
		return (NULL);
	}
	return (head);
}
