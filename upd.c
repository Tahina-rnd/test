/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_redir.c & parser.c modifications            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:40 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/19 11:00:00 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

/* ==================== PARSER_REDIR.C ==================== */

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

/* Retourne -1 pour erreur de syntaxe, 0 pour erreur malloc, 1 pour succès */
int	parse_redir(t_command *cmd, t_token **curr_token, t_shell *shell)
{
	t_token	*op;
	t_token	*file;
	t_redir	*new_redir;

	op = *curr_token;
	file = op->next;
	
	/* Pour heredoc, toujours créer le noeud si le file est WORD */
	if (op->type == HEREDOC)
	{
		if (file && file->type == WORD)
		{
			new_redir = create_redir_node(file, op->type);
			if (!new_redir)
				return (0);
			new_redir->index = cmd->redir_counter++;
			redir_add_back(&(cmd->heredoc), new_redir);
			*curr_token = file;
			return (1);
		}
		if_not_word(file, shell);
		return (-1);
	}
	
	/* Pour les autres redirections */
	if (!file || file->type != WORD)
	{
		if_not_word(file, shell);
		return (-1);
	}
	
	new_redir = create_redir_node(file, op->type);
	if (!new_redir)
		return (0);
	new_redir->index = cmd->redir_counter++;
	
	if (op->type == REDIRECT_IN)
		redir_add_back(&(cmd->input_redirection), new_redir);
	else
		redir_add_back(&(cmd->output_redirection), new_redir);
	
	*curr_token = file;
	return (1);
}

/* ==================== EXEC.C MODIFICATIONS ==================== */

void	executor(t_shell *shell)
{
	t_command	*cmd;

	cmd = shell->commands;
	
	/* Si last_exit_status == 2, c'est une erreur de syntaxe */
	/* On traite les heredocs puis on sort sans exécuter */
	if (shell->last_exit_status == 2)
	{
		process_heredocs(shell);
		cleanup_heredocs(shell);
		return;
	}
	
	if (process_heredocs(shell) != 0)
		return ;
	if (cmd && !cmd->next && cmd->args && cmd->args->value
		&& check_parent_builtin(cmd->args->value, shell))
	{
		exec_builtin_parent(cmd, shell);
		cleanup_heredocs(shell);
		return ;
	}
	exec_loop(shell, cmd);
	cleanup_heredocs(shell);
} */

static int	handle_pipe(t_command **current_cmd, t_token **curr_token)
{
	if (!(*current_cmd)->args && !(*current_cmd)->input_redirection
		&& !(*current_cmd)->output_redirection && !(*current_cmd)->heredoc)
	{
		ft_putstr_fd("Minishell: syntax error near unexpected token `|'\n", 2);
		return (0);
	}
	if ((*curr_token)->next && (*curr_token)->next->type == END)
	{
		ft_putstr_fd("Minishell: syntax error near unexpected token `|'\n", 2);
		return (0);
	}
	*current_cmd = create_command();
	if (!*current_cmd)
		return (0);
	return (1);
}

static int	process_token(t_command **curr_cmd, t_token **tokens,
	t_command **head, t_shell *shell, int *syntax_error)
{
	int	ret;

	if ((*tokens)->type == WORD)
	{
		if (!add_arg(*curr_cmd, *tokens))
			return (0);
	}
	else if ((*tokens)->type >= REDIRECT_IN && (*tokens)->type <= HEREDOC)
	{
		ret = parse_redir(*curr_cmd, tokens, shell);
		if (ret == -1)  /* Erreur de syntaxe */
		{
			*syntax_error = 1;
			return (1);  /* Continue pour traiter les heredocs */
		}
		if (ret == 0)  /* Erreur malloc */
			return (0);
	}
	else if ((*tokens)->type == PIPE)
	{
		if (!handle_pipe(curr_cmd, tokens))
		{
			*syntax_error = 1;
			return (1);
		}
		cmd_add_back(head, *curr_cmd);
	}
	return (1);
}

t_command	*parser(t_token *tokens, t_shell *shell)
{
	t_command	*head;
	t_command	*curr_cmd;
	int			syntax_error;

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
	syntax_error = 0;
	while (tokens && tokens->type != END)
	{
		if (!process_token(&curr_cmd, &tokens, &head, shell, &syntax_error))
		{
			shell->last_exit_status = 2;
			free_commands(head);
			return (NULL);
		}
		tokens = tokens->next;
	}
	if (syntax_error)
	{
		shell->last_exit_status = 2;
		/* On retourne quand même head pour que les heredocs soient traités */
		return (head);
	}
	return (head);
}