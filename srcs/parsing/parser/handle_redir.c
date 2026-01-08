/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_redir.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:57:18 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/07 07:05:33 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

int	handle_input_redirection(t_command *cmd, t_token **current)
{
	t_redir	*new_redir;

	*current = (*current)->next;
	if (!*current || (*current)->type != WORD)
		return (0);	
	new_redir = create_redir_node((*current)->value);
	if (!new_redir)
		return (0);
	add_redir_back(&cmd->input_redirection, new_redir);

	*current = (*current)->next;
	return (1);
}

int handle_output_redirection(t_command *cmd, t_token **current)
{
	t_redir *new_redir;

	*current = (*current)->next;
	if (!*current || (*current)->type != WORD)
		return (0);
	new_redir = create_redir_node((*current)->value);
	if (!new_redir)
		return (0);

	add_redir_back(&cmd->output_redirection, new_redir);

	*current = (*current)->next;
	return (1);
}

int handle_append_redirection(t_command *cmd, t_token **current)
{
	t_redir *new_redir;

	*current = (*current)->next;
	if (!*current || (*current)->type != WORD)
		return (0);
	new_redir = create_append_redir_node((*current)->value);
	if (!new_redir)
		return (0);
	
	add_redir_back(&cmd->output_redirection, new_redir);
	*current = (*current)->next;
	return (1);
}

int	handle_heredoc(t_command *cmd, t_token **current)
{
	t_redir	*new_redir;

	*current = (*current)->next;
	if (!*current || (*current)->type != WORD)
		return (0);
	new_redir = create_redir_node((*current)->value);
	if (!new_redir)
		return (0);
	add_redir_back(&cmd->heredoc, new_redir);
	*current = (*current)->next;
	return (1);
}
