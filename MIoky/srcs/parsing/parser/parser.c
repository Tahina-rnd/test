/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:45:47 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/09 12:05:55 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

int	syntax_error(char *token)
{
	ft_putstr_fd("minishell: syntax error near unexpected token `", 2);
	if (token)
		ft_putstr_fd(token, 2);
	else
		ft_putstr_fd("newline", 2);
	ft_putendl_fd("'", 2);
	return (0);
}

int	validate_tokens(t_token *tokens)
{
	t_token	*current;

	if (!tokens)
		return (0);
	if (tokens->type == PIPE)
		return (syntax_error("|"));
	current = tokens;
	while (current && current->type != END)
	{
		if (current->type == PIPE)
		{
			if (!current->next || current->next->type == END)
				return (syntax_error("|"));
			if (current->next->type == PIPE)
				return (syntax_error("|"));
		}
		if (current->type >= REDIRECT_IN && current->type <= HEREDOC)
		{
			if (!current->next || current->next->type != WORD)
				return (syntax_error(current->value));
		}
		current = current->next;
	}
	return (1);
}

int	parse_simple_command(t_token **tokens, t_command *cmd)
{
	while (*tokens && (*tokens)->type != PIPE && (*tokens)->type != END)
	{
		if ((*tokens)->type == REDIRECT_IN)
		{
			if (!handle_input_redir(tokens, cmd))
				return (0);
		}
		else if ((*tokens)->type == REDIRECT_OUT)
		{
			if (!handle_output_redir(tokens, cmd))
				return (0);
		}
		else if ((*tokens)->type == HEREDOC)
		{
			if (!handle_heredoc(tokens, cmd))
				return (0);
		}
		else if ((*tokens)->type == APPEND)
		{
			if (!handle_append(tokens, cmd))
				return (0);
		}
		else if ((*tokens)->type == WORD)
		{
			if (!add_arg_to_command(cmd, *tokens))
				return (0);
			*tokens = (*tokens)->next;
		}
		else
			*tokens = (*tokens)->next;
	}
	return (1);
}

void	append_command(t_command **cmds, t_command *new_cmd)
{
	t_command	*last;

	if (!new_cmd)
		return ;
	if (!*cmds)
	{
		*cmds = new_cmd;
		return ;
	}
	last = get_last_command(*cmds);
	last->next = new_cmd;
}

t_command	*parser(t_token *tokens)
{
	t_command	*cmds;
	t_command	*current;

	if (!validate_tokens(tokens))
		return (NULL);
	cmds = NULL;
	while (tokens && tokens->type != END)
	{
		current = new_command();
		if (!current)
		{
			free_commands(cmds);
			return (NULL);
		}
		if (!parse_simple_command(&tokens, current))
		{
			free_command(current);
			free_commands(cmds);
			return (NULL);
		}
		append_command(&cmds, current);
		if (tokens && tokens->type == PIPE)
			tokens = tokens->next;
	}
	return (cmds);
}
