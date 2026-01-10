/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:44:36 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/10 22:05:03 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static void	process_segment(t_arg *arg, t_segment *seg, t_shell *shell)
{
	char	*expanded;
	char	*old_val;

	if (seg->quote == QUOTE_SINGLE)
		expanded = ft_strdup(seg->value);
	else
		expanded = expand_text(seg->value, shell);
	if (!expanded)
		return ;
	old_val = arg->value;
	arg->value = ft_strjoin_free(old_val, expanded);
	free(expanded);
}

static void	expand_arg(t_arg *arg, t_shell *shell)
{
	t_segment	*seg;

	if (arg->value)
		free(arg->value);
	arg->value = ft_strdup("");
	seg = arg->segments;
	while (seg)
	{
		process_segment(arg, seg, shell);
		seg = seg->next;
	}
}

static int	expand_redir_file(t_redir *redir, t_shell *shell)
{
	char	*expanded;

	// Note: Ici on simplifie. Dans le vrai minishell, il faudrait utiliser
	// la même logique de segments que pour les args pour être parfait.
	// Mais pour l'instant, faisons simple :
	
	// Si le fichier contient un $, on l'étend
	if (ft_strchr(redir->file, '$'))
	{
		expanded = expand_text(redir->file, shell);
		if (!expanded || ft_strlen(expanded) == 0)
		{
			// C'est ICI qu'on détecte l'Ambiguous Redirect
			printf("Minishell: %s: ambiguous redirect\n", redir->file);
			if (expanded) free(expanded);
			return (0); // Erreur
		}
		free(redir->file);
		redir->file = expanded;
	}
	return (1); // Succès
}

static int	process_redirs(t_redir *lst, t_shell *shell)
{
	while (lst)
	{
		// On n'étend pas les heredocs (sauf si on gère les quotes heredoc, bonus)
		if (!expand_redir_file(lst, shell))
			return (0);
		lst = lst->next;
	}
	return (1);
}

void	expander(t_shell *shell, t_command *cmd)
{
	t_command	*curr_cmd;
	t_arg		*curr_arg;

	curr_cmd = cmd;
	while (curr_cmd)
	{
		// 1. Expansion des arguments (ls $USER)
		curr_arg = curr_cmd->args;
		while (curr_arg)
		{
			expand_arg(curr_arg, shell);
			curr_arg = curr_arg->next;
		}

		// 2. Expansion des redirections (echo a > $FICHIER)
		// Si ambiguous redirect, on peut choisir de marquer la commande comme invalide
		if (!process_redirs(curr_cmd->input_redirection, shell)
			|| !process_redirs(curr_cmd->output_redirection, shell))
		{
			// Astuce : On peut mettre une commande vide ou un flag d'erreur
			// pour que l'executor ne l'exécute pas.
			shell->last_exit_status = 1;
		}
		
		curr_cmd = curr_cmd->next;
	}
}
