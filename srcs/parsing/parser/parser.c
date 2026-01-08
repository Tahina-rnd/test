/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:45:47 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/08 05:31:07 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

/**
 * Initialise une nouvelle commande avec toutes les listes à NULL
 */
static t_command *init_command(void)
{
	t_command *command;

    command = malloc(sizeof(t_command));
    if (!command)
        return (NULL);

    command->args = malloc(sizeof(char *) * 1);
    if (!command->args)
    {
        free(command);
        return (NULL);
    }
    command->args[0] = NULL;    
    command->args_was_quoted = malloc(sizeof(int) * 1);
    if (!command->args_was_quoted)
    {
        free(command->args);
        free(command);
        return (NULL);
    }
    command->args_was_quoted[0] = 0;
    command->input_redirection = NULL;
    command->output_redirection = NULL;
    command->heredoc = NULL;
    command->next = NULL;

    return (command);
}

/**
 * Finalise une commande et l'ajoute à la liste des commandes
 */
int	finalize_command(t_command **command_list, t_command **current_cmd)
{
	t_command	*temp;

	if (!*current_cmd)
		return (0);
	if (!*command_list)
		*command_list = *current_cmd;
	else
	{
		temp = *command_list;
		while (temp->next)
			temp = temp->next;
		temp->next = *current_cmd;
	}
	*current_cmd = NULL;
	return (1);
}

/**
 * Parse les tokens et construit la liste de commandes
 */
t_command	*parse(t_token *tokens)
{
	t_command	*command_list;
	t_command	*current_cmd;
	t_token		*current;

	command_list = NULL;
	current_cmd = init_command();
	current = tokens;

	if (!tokens || !current_cmd)
		return (NULL);

	while (current)
	{
		if (current->type == WORD)
		{
			// Ajouter un argument
			if (!add_argument(current_cmd, current->value, current->was_quoted))
				return (cleanup_and_return(&command_list, current_cmd), NULL);
		}
		else if (current->type == REDIRECT_IN)
		{
			// Redirection d'entrée: <
			if (!handle_input_redirection(current_cmd, &current))
				return (cleanup_and_return(&command_list, current_cmd), NULL);
			continue;  // handle_input_redirection avance current
		}
		else if (current->type == REDIRECT_OUT)
		{
			// Redirection de sortie: >
			if (!handle_output_redirection(current_cmd, &current))
				return (cleanup_and_return(&command_list, current_cmd), NULL);
			continue;
		}
		else if (current->type == APPEND)
		{
			// Redirection en mode append: >>
			if (!handle_append_redirection(current_cmd, &current))
				return (cleanup_and_return(&command_list, current_cmd), NULL);
			continue;
		}
		else if (current->type == HEREDOC)
		{
			// Heredoc: <<
			if (!handle_heredoc(current_cmd, &current))
				return (cleanup_and_return(&command_list, current_cmd), NULL);
			continue;
		}
		else if (current->type == PIPE)
		{
			// Pipe: finaliser la commande courante et en créer une nouvelle
			if (!finalize_command(&command_list, &current_cmd))
				return (cleanup_and_return(&command_list, NULL), NULL);
			current_cmd = init_command();
			if (!current_cmd)
				return (cleanup_and_return(&command_list, NULL), NULL);
		}

		current = current->next;
	}

	// Finaliser la dernière commande
	if (current_cmd)
		finalize_command(&command_list, &current_cmd);

	return (command_list);
}
