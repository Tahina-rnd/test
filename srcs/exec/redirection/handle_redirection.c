/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_redirection.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 13:32:52 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/05 22:28:09 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

/**
 * Gère les redirections d'entrée
 *
 * CORRECTION:
 * - Vérification supplémentaire de last_redir->file
 * - Gestion d'erreur améliorée
 *
 * EXPLICATION:
 * En cas de redirections multiples (< f1 < f2 < f3):
 * - Seul f3 (le dernier) est utilisé pour STDIN
 * - Les autres fichiers ne sont même pas ouverts
 */
static int	handle_input_redirections(t_redir *input_list)
{
	t_redir	*last_redir;
	int		fd;

	if (!input_list)
		return (0);

	// Récupérer la dernière redirection
	last_redir = get_last_redir(input_list);
	if (!last_redir || !last_redir->file)
	{
		ft_putstr_fd("minishell: ambiguous redirect\n", 2);
		return (1);
	}

	// Ouvrir le fichier en lecture
	fd = open(last_redir->file, O_RDONLY);
	if (fd == -1)
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(last_redir->file, 2);
		ft_putstr_fd(": ", 2);
		ft_putstr_fd(strerror(errno), 2);
		ft_putstr_fd("\n", 2);
		return (1);
	}

	// Rediriger STDIN vers le fichier
	if (dup2(fd, STDIN_FILENO) == -1)
	{
		perror("minishell: dup2");
		close(fd);
		return (1);
	}

	close(fd);
	return (0);
}

static int handle_all_output_redirections(t_redir *output_list)
{
    t_redir *current;
    t_redir *last_redir;
    int     fd;
    int     flags;

    if (!output_list)
        return (0);

    // ÉTAPE 1: Traiter tous les fichiers sauf le dernier
    current = output_list;
    while (current->next)
    {
        if (!current->file)
        {
            ft_putstr_fd("minishell: ambiguous redirect\n", 2);
            return (1);
        }

        // Choisir les flags selon append_mode
        if (current->append_mode)
            flags = O_WRONLY | O_CREAT | O_APPEND;
        else
            flags = O_WRONLY | O_CREAT | O_TRUNC;

        // Ouvrir et fermer immédiatement (pour créer/modifier le fichier)
        fd = open(current->file, flags, 0644);
        if (fd == -1)
        {
            ft_putstr_fd("minishell: ", 2);
            ft_putstr_fd(current->file, 2);
            ft_putstr_fd(": ", 2);
            ft_putstr_fd(strerror(errno), 2);
            ft_putstr_fd("\n", 2);
            return (1);
        }
        close(fd);
        current = current->next;
    }

    // ÉTAPE 2: Le dernier fichier reçoit STDOUT
    last_redir = current;
    if (!last_redir->file)
    {
        ft_putstr_fd("minishell: ambiguous redirect\n", 2);
        return (1);
    }

    if (last_redir->append_mode)
        flags = O_WRONLY | O_CREAT | O_APPEND;
    else
        flags = O_WRONLY | O_CREAT | O_TRUNC;

    fd = open(last_redir->file, flags, 0644);
    if (fd == -1)
    {
        ft_putstr_fd("minishell: ", 2);
        ft_putstr_fd(last_redir->file, 2);
        ft_putstr_fd(": ", 2);
        ft_putstr_fd(strerror(errno), 2);
        ft_putstr_fd("\n", 2);
        return (1);
    }

    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("minishell: dup2");
        close(fd);
        return (1);
    }

    close(fd);
    return (0);
}

/**
 * Fonction principale - SIMPLIFIÉE
 */
int handle_redirections(t_command *cmd)
{
    if (!cmd)
        return (0);

    // 1. Input (heredoc prioritaire sur <)
    if (cmd->heredoc)
    {
        if (handle_input_redirections(cmd->heredoc))
            return (1);
    }
    else if (cmd->input_redirection)
    {
        if (handle_input_redirections(cmd->input_redirection))
            return (1);
    }

    // 2. Output (> et >> mélangés dans output_redirection)
    if (cmd->output_redirection)
    {
        if (handle_all_output_redirections(cmd->output_redirection))
            return (1);
    }

    return (0);
}
