/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:57:18 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/12 12:49:56 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

t_command	*create_command(void)
{
	t_command	*cmd;

	cmd = malloc(sizeof(t_command));
	if (!cmd)
		return (NULL);
	cmd->args = NULL;
	cmd->input_redirection = NULL;
	cmd->output_redirection = NULL;
	cmd->heredoc = NULL;
	cmd->next = NULL;
	return (cmd);
}

void	cmd_add_back(t_command **lst, t_command *new)
{
	t_command	*temp;

	if (!lst || !new)
		return ;
	if (!*lst)
	{
		*lst = new;
		return ;
	}
	temp = *lst;
	while (temp->next)
		temp = temp->next;
	temp->next = new;
}

int	add_arg(t_command *cmd, t_token *token)
{
	t_arg	*new_arg;
	t_arg	*temp;

	new_arg = malloc(sizeof(t_arg));
	if (!new_arg)
		return (0);
	new_arg->value = NULL;
	new_arg->segments = token->segments;
	token->segments = NULL;
	new_arg->next = NULL;
	if (!cmd->args)
		cmd->args = new_arg;
	else
	{
		temp = cmd->args;
		while (temp->next)
			temp = temp->next;
		temp->next = new_arg;
	}
	return (1);
}
