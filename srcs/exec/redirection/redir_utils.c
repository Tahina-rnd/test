/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 10:30:00 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/19 10:44:47 by miokrako         ###   ########.fr       */
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

void	print_permission_error(char *file)
{
	ft_putstr_fd("minishell: ", 2);
	ft_putstr_fd(file, 2);
	ft_putstr_fd(": Permission denied\n", 2);
}

void	print_file_error(char *file)
{
	ft_putstr_fd("minishell: ", 2);
	ft_putstr_fd(file, 2);
	ft_putstr_fd(": ", 2);
	ft_putstr_fd(strerror(errno), 2);
	ft_putstr_fd("\n", 2);
}

int	check_file_access(char *file)
{
	if (access(file, F_OK) == -1)
	{
		print_file_error(file);
		return (1);
	}
	if (access(file, R_OK) == -1)
	{
		print_permission_error(file);
		return (1);
	}
	return (0);
}
