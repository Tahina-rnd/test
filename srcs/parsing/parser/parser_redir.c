/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_redir.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:40 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/14 22:12:32 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static t_redir	*create_redir_node(t_token *file_token, int type)
{
	t_redir	*node;

	node = malloc(sizeof(t_redir));
	if (!node)
		return (NULL);
	node->file = ft_strdup(file_token->literal);
	node->fd = -1;
	node->append_mode = 0;
	if (type == APPEND)
		node->append_mode = 1;
	node->next = NULL;
	return (node);
}

static void	redir_add_back(t_redir **lst, t_redir *new)
{
	t_redir	*temp;

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

static void	if_not_word(t_token *file, t_shell *shell)
{
	ft_putstr_fd("Minishell: syntax error near unexpected token `", 2);
	if (!file || !file->literal)
		ft_putstr_fd("newline", 2);
	else
		ft_putstr_fd(file->literal, 2);
	ft_putstr_fd("'\n", 2);
	shell->last_exit_status = 2;
}

int	parse_redir(t_command *cmd, t_token **curr_token, t_shell *shell)
{
	t_token	*op;
	t_token	*file;
	t_redir	*new_redir;

	op = *curr_token;
	file = op->next;
	if (!file || file->type != WORD)
	{
		if_not_word(file, shell);
		return (0);
	}
	new_redir = create_redir_node(file, op->type);
	if (!new_redir)
		return (0);
	new_redir->index = cmd->redir_counter++;
	if (op->type == REDIRECT_IN)
		redir_add_back(&(cmd->input_redirection), new_redir);
	else if (op->type == HEREDOC)
		redir_add_back(&(cmd->heredoc), new_redir);
	else
		redir_add_back(&(cmd->output_redirection), new_redir);
	*curr_token = file;
	return (1);
}
