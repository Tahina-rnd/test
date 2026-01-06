/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_redir.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:57:18 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/05 10:55:15 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

/**
 * Gère les redirections d'entrée: cmd < file
 * Ajoute le fichier à la liste des redirections d'entrée
 */
int	handle_input_redirection(t_command *cmd, t_token **current)
{
	t_redir	*new_redir;

	*current = (*current)->next;
	if (!*current || (*current)->type != WORD)
		return (0);

	// Créer un nouveau nœud de redirection
	new_redir = create_redir_node((*current)->value);
	if (!new_redir)
		return (0);

	// Ajouter à la liste des redirections d'entrée
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

    // Créer avec append_mode = 0 (trunc)
    new_redir = create_redir_node((*current)->value);
    if (!new_redir)
        return (0);

    // Ajouter à la liste UNIQUE output_redirection
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

    // Créer avec append_mode = 1 (append)
    new_redir = create_append_redir_node((*current)->value);
    if (!new_redir)
        return (0);

    // Ajouter à la MÊME liste output_redirection
    add_redir_back(&cmd->output_redirection, new_redir);

    *current = (*current)->next;
    return (1);
}
/**
 * Gère les heredocs: cmd << delimiter
 * Ajoute le délimiteur à la liste des heredocs
 */
int	handle_heredoc(t_command *cmd, t_token **current)
{
	t_redir	*new_redir;

	*current = (*current)->next;
	if (!*current || (*current)->type != WORD)
		return (0);

	// Créer un nouveau nœud de redirection avec le délimiteur
	new_redir = create_redir_node((*current)->value);
	if (!new_redir)
		return (0);

	// Ajouter à la liste des heredocs
	add_redir_back(&cmd->heredoc, new_redir);

	*current = (*current)->next;
	return (1);
}
