/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_cmd.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:40 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/05 10:58:51 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static void free_cmd(t_command *cmd)
{
    int i;

    if (!cmd)
        return;

    if (cmd->args)
    {
        i = 0;
        while (cmd->args[i])
        {
            free(cmd->args[i]);
            i++;
        }
        free(cmd->args);
    }

    if (cmd->input_redirection)
        free_redir_list(cmd->input_redirection);
    if (cmd->output_redirection)  // Une seule liste maintenant
        free_redir_list(cmd->output_redirection);
    if (cmd->heredoc)
        free_redir_list(cmd->heredoc);
    // Plus de cmd->append

    free(cmd);
}

/**
 * Fonction de nettoyage en cas d'erreur pendant le parsing
 */
int	cleanup_and_return(t_command **cmd_list, t_command *current_cmd)
{
	free_cmds(*cmd_list);
	free_cmds(current_cmd);
	return (0);
}

/**
 * Libère toute la liste de commandes
 */
void	free_cmds(t_command *cmds)
{
	t_command	*tmp;

	while (cmds)
	{
		tmp = cmds;
		cmds = cmds->next;
		free_cmd(tmp);
	}
}
