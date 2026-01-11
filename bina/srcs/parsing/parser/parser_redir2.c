/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_redir2.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:40 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/09 00:06:11 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

int	handle_input_redir(t_token **tokens, t_command *cmd)
{
	*tokens = (*tokens)->next;
	if (!*tokens || (*tokens)->type != WORD)
		return (syntax_error("<"));
	add_input_redir(cmd, (*tokens)->value);
	*tokens = (*tokens)->next;
	return (1);
}

int	handle_output_redir(t_token **tokens, t_command *cmd)
{
	*tokens = (*tokens)->next;
	if (!*tokens || (*tokens)->type != WORD)
		return (syntax_error(">"));
	add_output_redir(cmd, (*tokens)->value, 0);
	*tokens = (*tokens)->next;
	return (1);
}

int	handle_heredoc(t_token **tokens, t_command *cmd)
{
	*tokens = (*tokens)->next;
	if (!*tokens || (*tokens)->type != WORD)
		return (syntax_error("<<"));
	add_heredoc(cmd, (*tokens)->value);
	*tokens = (*tokens)->next;
	return (1);
}

int	handle_append(t_token **tokens, t_command *cmd)
{
	*tokens = (*tokens)->next;
	if (!*tokens || (*tokens)->type != WORD)
		return (syntax_error(">>"));
	add_output_redir(cmd, (*tokens)->value, 1);
	*tokens = (*tokens)->next;
	return (1);
}
