/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:45:47 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/12 16:59:13 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static int	handle_pipe(t_command **current_cmd, t_token **curr_token)
{
	if (!(*current_cmd)->args && !(*current_cmd)->input_redirection
		&& !(*current_cmd)->output_redirection && !(*current_cmd)->heredoc)
	{
		printf("Minishell: syntax error near unexpected token `|'\n");
		return (0);
	}
	if ((*curr_token)->next && (*curr_token)->next->type == END)
	{
		printf("Minishell: syntax error near unexpected token `|'\n");
		return (0);
	}
	*current_cmd = create_command();
	if (!*current_cmd)
		return (0);
	return (1);
}

static int	process_token(t_command **curr_cmd, t_token **tokens,
	t_command **head, t_shell *shell)
{
	if ((*tokens)->type == WORD)
	{
		if (!add_arg(*curr_cmd, *tokens))
			return (0);
	}
	else if ((*tokens)->type >= REDIRECT_IN && (*tokens)->type <= HEREDOC)
	{
		if (!parse_redir(*curr_cmd, tokens, shell))
			return (0);
	}
	else if ((*tokens)->type == PIPE)
	{
		if (!handle_pipe(curr_cmd, tokens))
			return (0);
		cmd_add_back(head, *curr_cmd);
	}
	return (1);
}

static void	if_pipe(t_shell *shell)
{
	printf("Minishell: syntax error near unexpected token `|'\n");
	shell->last_exit_status = 2;
}

t_command	*parser(t_token *tokens, t_shell *shell)
{
	t_command	*head;
	t_command	*curr_cmd;

	if (!tokens)
		return (NULL);
	if (tokens->type == PIPE)
	{
		if_pipe(shell);
		return (NULL);
	}
	head = create_command();
	if (!head)
		return (NULL);
	curr_cmd = head;
	while (tokens && tokens->type != END)
	{
		if (!process_token(&curr_cmd, &tokens, &head, shell))
		{
			shell->last_exit_status = 2;
			free_commands(head);
			return (NULL);
		}
		tokens = tokens->next;
	}
	return (head);
}
