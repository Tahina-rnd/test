/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_process2.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 10:39:50 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/12 16:37:32 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static	int	ft_isspace(int c)
{
	return (c == ' ' || c == '\t' || c == '\n'
		|| c == '\v' || c == '\f' || c == '\r');
}

static t_arg	*handle_unquoted(t_arg *curr, char *val)
{
	int		i;
	char	buf[2];

	i = 0;
	buf[1] = '\0';
	while (val[i])
	{
		if (ft_isspace(val[i]))
		{
			if (curr->value != NULL)
			{
				curr->next = new_arg_node();
				curr = curr->next;
			}
			while (val[i] && ft_isspace(val[i]))
				i++;
			continue ;
		}
		buf[0] = val[i];
		append_val(curr, buf);
		i++;
	}
	return (curr);
}

static void	segment_condition(t_segment *seg, t_arg *curr, t_shell *shell)
{
	char		*exp_val;

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
}

t_arg	*expand_arg_to_list(t_arg *arg, t_shell *shell)
{
	t_arg		*head;
	t_arg		*curr;
	t_segment	*seg;

	head = new_arg_node();
	curr = head;
	seg = arg->segments;
	while (seg)
	{
		segment_condition(seg, curr, shell);
		seg = seg->next;
	}
	if (curr->value == NULL && head == curr)
		return (free(head), NULL);
	return (head);
}
