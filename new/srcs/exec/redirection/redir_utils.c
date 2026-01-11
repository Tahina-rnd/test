/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 10:30:00 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/09 11:47:37 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

t_redir	*get_last_redir(t_redir *redir_list)
{
	t_redir	*current;

	if (!redir_list)
		return (NULL);
	current = redir_list;
	while (current->next)
		current = current->next;
	return (current);
}
