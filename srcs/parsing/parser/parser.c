/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:45:47 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/19 12:37:46 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	handle_pipe(t_command **current_cmd, t_token **curr_token)
{
	if (!(*current_cmd)->args && !(*current_cmd)->input_redirection
		&& !(*current_cmd)->output_redirection && !(*current_cmd)->heredoc)
		return (0);
	if ((*curr_token)->next && (*curr_token)->next->type == END)
		return (0);
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

static void	*handle_syntax_error(t_shell *shell, t_command *head, t_token *tok)
{
	shell->last_exit_status = 2;
	shell->commands = head;
	process_heredocs(shell);
	ft_putstr_fd("Minishell: syntax error near unexpected token `", 2);
	if (tok->type == PIPE)
		ft_putstr_fd("|", 2);
	else if (tok->type == END || !tok->next)
		ft_putstr_fd("newline", 2);
	else if (tok->next && tok->next->literal)
		ft_putstr_fd(tok->next->literal, 2);
	else
		ft_putstr_fd("newline", 2);
	ft_putstr_fd("'\n", 2);
	cleanup_heredocs(shell);
	shell->commands = NULL;
	free_commands(head);
	return (NULL);
}

t_command	*parser(t_token *tokens, t_shell *shell)
{
	t_command	*head;
	t_command	*curr_cmd;

	if (!tokens)
		return (NULL);
	if (tokens->type == PIPE)
	{
		ft_putstr_fd("Minishell: syntax error near unexpected token `|'\n", 2);
		shell->last_exit_status = 2;
		return (NULL);
	}
	head = create_command();
	if (!head)
		return (NULL);
	curr_cmd = head;
	while (tokens && tokens->type != END)
	{
		if (!process_token(&curr_cmd, &tokens, &head, shell))
			return (handle_syntax_error(shell, head, tokens));
		tokens = tokens->next;
	}
	return (head);
}
