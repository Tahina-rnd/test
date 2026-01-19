/* ************************************************************************** */
/* */
/* :::      ::::::::   */
/* exec.c                                             :+:      :+:    :+:   */
/* +:+ +:+         +:+     */
/* By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/* +#+#+#+#+#+   +#+           */
/* Created: 2025/11/24 15:43:21 by miokrako          #+#    #+#             */
/* Updated: 2026/01/19 10:45:00 by miokrako         ###   ########.fr       */
/* */
/* ************************************************************************** */

#include "../../../includes/exec.h"

/*
** scan_heredocs:
** Parcourt toutes les commandes avant l'exécution pour traiter les heredocs.
** Cela permet de capturer l'entrée utilisateur (et créer les fichiers tmp)
** AVANT de vérifier les erreurs de redirection (comme < fichier_inconnu).
*/
static int	scan_heredocs(t_shell *shell)
{
	t_command	*cmd;

	cmd = shell->commands;
	while (cmd)
	{
		if (process_cmd_heredocs(cmd, shell) == 1)
		{
			// Si le heredoc est interrompu (ex: Ctrl+C), on arrête tout.
			shell->last_exit_status = 130;
			return (1);
		}
		cmd = cmd->next;
	}
	return (0);
}

void	executor(t_shell *shell)
{
	t_command	*cmd;
	int			prev_pipe[2];
	int			curr_pipe[2];

	// 1. PHASE DE PRÉ-TRAITEMENT : Heredocs
	// On traite tous les heredocs ici, avant de lancer quoi que ce soit.
	if (scan_heredocs(shell) == 1)
		return ;

	// 2. PHASE D'EXÉCUTION
	prev_pipe[0] = -1;
	prev_pipe[1] = -1;
	cmd = shell->commands;
	while (cmd)
	{
		prepare_pipe(cmd, prev_pipe, curr_pipe);
		if (cmd->next == NULL && is_builtin(cmd->args->value)
			&& check_parent_builtin(cmd->args->value, shell))
			exec_builtin_parent(cmd, shell);
		else
			fork_handler(shell, cmd, prev_pipe, curr_pipe);
		handle_parent_pipes(cmd, prev_pipe, curr_pipe);
		cmd = cmd->next;
	}
	wait_all_children(shell->last_pid, shell);
	
	// Nettoyage des fichiers temporaires créés par les heredocs
	cleanup_heredocs(shell);
}
