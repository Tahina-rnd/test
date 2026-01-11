/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:44:36 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/11 11:16:30 by tarandri         ###   ########.fr       */
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

	// Si le fichier contient un $
	if (ft_strchr(redir->file, '$'))
	{
		expanded = expand_text(redir->file, shell);
		
		// Vérifie si vide OU si contient des espaces (ambiguous redirect)
		// Note: Pour être parfait (gérer "$VAR" avec espaces), il faudrait les segments.
		// Mais ici on fait le check basique.
		if (!expanded || ft_strlen(expanded) == 0 || ft_strchr(expanded, ' '))
		{
			printf("Minishell: %s: ambiguous redirect\n", redir->file);
			if (expanded)
				free(expanded);
			
			// === CORRECTION DOUBLE FREE ===
			free(redir->file); 
			redir->file = NULL; // Indispensable pour éviter le double free plus tard
			// ==============================
			
			return (0);
		}
		free(redir->file);
		redir->file = expanded;
	}
	return (1);
}

static int	process_redirs(t_redir *lst, t_shell *shell)
{
	int	status;

	status = 1;
	while (lst)
	{
		// On continue même si erreur pour traiter tous les fichiers,
		// mais on retiendra l'échec (0)
		if (!expand_redir_file(lst, shell))
			status = 0;
		lst = lst->next;
	}
	return (status);
}

void	expander(t_shell *shell, t_command *cmd)
{
	t_command	*curr_cmd;
	t_arg		*curr_arg;

	curr_cmd = cmd;
	while (curr_cmd)
	{
		curr_arg = curr_cmd->args;
		while (curr_arg)
		{
			expand_arg(curr_arg, shell);
			curr_arg = curr_arg->next;
		}

		// Si ambiguous redirect détecté (retour 0), on met le status à 1.
		// Grâce à la correction, redir->file est NULL, donc l'executor 
		// échouera proprement sans créer de fichier.
		if (!process_redirs(curr_cmd->input_redirection, shell)
			|| !process_redirs(curr_cmd->output_redirection, shell))
		{
			shell->last_exit_status = 1;
		}
		
		curr_cmd = curr_cmd->next;
	}
}
