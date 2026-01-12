/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:44:36 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/12 07:16:51 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static t_arg	*new_arg_node(void)
{
	t_arg	*node;

	node = malloc(sizeof(t_arg));
	if (!node)
		return (NULL);
	node->value = NULL;
	node->next = NULL;
	node->segments = NULL;
	return (node);
}

static void	append_val(t_arg *arg, char *str)
{
	char	*tmp;

	if (!arg->value)
		arg->value = ft_strdup("");
	tmp = arg->value;
	arg->value = ft_strjoin_free(tmp, str);
}

/* --- PARTIE 2 : LOGIQUE WORD SPLITTING --- */

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
			while (val[i] && (val[i] == ' ' || val[i] == '\t' || val[i] == '\n'))
				i++;
			continue ;
		}
		buf[0] = val[i];
		append_val(curr, buf);
		i++;
	}
	return (curr);
}

static t_arg	*expand_arg_to_list(t_arg *arg, t_shell *shell)
{
	t_arg		*head;
	t_arg		*curr;
	t_segment	*seg;
	char		*exp_val;

	head = new_arg_node();
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
	if (curr->value == NULL && head == curr)
	{
		free(head);
		return (NULL);
	}
	return (head);
}

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

/* --- PARTIE 3 : REDIRECTIONS (AMBIGUOUS REDIRECT) --- */

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

static int	process_redirs(t_redir *lst, t_shell *shell)
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

/* --- PARTIE 4 : MAIN EXPANDER --- */

void	expander(t_shell *shell, t_command *cmd)
{
	t_command	*c;
	t_arg		*curr;
	t_arg		*prev;
	t_arg		*next_save;
	t_arg		*expanded_list;

	c = cmd;
	while (c)
	{
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
		if (!process_redirs(c->input_redirection, shell)
			|| !process_redirs(c->output_redirection, shell))
			shell->last_exit_status = 1;
		c = c->next;
	}
}
