/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 16:00:00 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/05 10:54:31 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

t_redir *create_redir_node(char *file)
{
    t_redir *new_redir;

    if (!file)
        return (NULL);
    new_redir = malloc(sizeof(t_redir));
    if (!new_redir)
        return (NULL);
    new_redir->file = ft_strdup(file);
    if (!new_redir->file)
    {
        free(new_redir);
        return (NULL);
    }
    new_redir->fd = -1;
    new_redir->append_mode = 0;  // Par défaut, mode trunc (>)
    new_redir->next = NULL;
    return (new_redir);
}

void	add_redir_back(t_redir **head, t_redir *new_redir)
{
	t_redir	*current;

	if (!head || !new_redir)
		return ;
	if (*head == NULL)
	{
		*head = new_redir;
		return ;
	}
	current = *head;
	while (current->next)
		current = current->next;
	current->next = new_redir;
}

void	free_redir_list(t_redir *redir_list)
{
	t_redir	*tmp;

	while (redir_list)
	{
		tmp = redir_list;
		redir_list = redir_list->next;
		if (tmp->file)
			free(tmp->file);
		if (tmp->fd != -1)
			close(tmp->fd);
		free(tmp);
	}
}

int	count_redir_nodes(t_redir *redir_list)
{
	int	count;

	count = 0;
	while (redir_list)
	{
		count++;
		redir_list = redir_list->next;
	}
	return (count);
}

/**
 * Récupère la dernière redirection de la liste
 * (Bash utilise toujours la dernière en cas de multiples redirections)
 * @param redir_list: liste de redirections
 * @return dernier nœud ou NULL
 */
t_redir	*get_last_redir(t_redir *redir_list)
{
	if (!redir_list)
		return (NULL);
	while (redir_list->next)
		redir_list = redir_list->next;
	return (redir_list);
}
t_redir *create_append_redir_node(char *file)
{
    t_redir *new_redir;

    new_redir = create_redir_node(file);
    if (new_redir)
        new_redir->append_mode = 1;  // Mode append (>>)
    return (new_redir);
}
