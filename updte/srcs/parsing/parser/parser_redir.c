/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_redir.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:40 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/12 07:12:07 by tarandri         ###   ########.fr       */
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

/**
 * Parse une redirection (< > << >>)
 * 
 * IMPORTANT : Cette fonction AVANCE **curr_token de 1 position
 * Le token pointé devient le fichier/délimiteur
 * 
 * Avant : *curr_token pointe sur '<'
 * Après  : *curr_token pointe sur 'file.txt'
 * 
 * Le parser principal doit faire tokens = tokens->next après l'appel
 */
int	parse_redir(t_command *cmd, t_token **curr_token)
{
	t_token	*op;
	t_token	*file;
	t_redir	*new_redir;

	op = *curr_token;
	file = op->next;
	
	// Vérification d'erreur de syntaxe
	if (!file || file->type != WORD)
	{
		ft_putstr_fd("Minishell: syntax error near unexpected token `", 2);
		if (!file || !file->literal) 
			ft_putstr_fd("newline", 2);
		else
			ft_putstr_fd(file->literal, 2);
		ft_putstr_fd("'\n", 2);
		return (0);
	}
	
	// Création du nœud de redirection
	new_redir = create_redir_node(file, op->type);
	if (!new_redir)
		return (0);
	
	// Distribution selon le type
	if (op->type == REDIRECT_IN)
		redir_add_back(&(cmd->input_redirection), new_redir);
	else if (op->type == HEREDOC)
		redir_add_back(&(cmd->heredoc), new_redir);
	else // REDIRECT_OUT ou APPEND
		redir_add_back(&(cmd->output_redirection), new_redir);
	
	// ✅ CRITIQUE : Avance curr_token sur le fichier
	// Cela empêche le fichier d'être ajouté aux arguments
	*curr_token = file;
	
	return (1);
}
