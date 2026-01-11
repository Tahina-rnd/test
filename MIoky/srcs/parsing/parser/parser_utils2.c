/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_utils2.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:57:18 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/09 12:13:31 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

// void	free_commands(t_command *commands)
// {
// 	t_command	*tmp;

// 	while (commands)
// 	{
// 		tmp = commands;
// 		commands = commands->next;
// 		free_command(tmp);
// 	}
// }

int	count_args(t_command *cmd)
{
	int		count;
	t_arg	*current;

	if (!cmd || !cmd->args)
		return (0);
	count = 0;
	current = cmd->args;
	while (current && current->value)
	{
		count++;
		current++;
	}
	return (count);
}
