/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   add_args.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:36:20 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/02 11:44:11 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static	int	cmd_args_len(char **args)
{
	int	i;

	if (!args)
		return (0);
	i = 0;
	while (args[i])
		i++;
	return (i);
}

int	add_argument(t_command *cmd, char *arg)
{
	int		i;
	int		j;
	char	**new_args;

	if (!cmd || !arg)
		return (0);
	i = cmd_args_len(cmd->args);
	j = 0;
	new_args = malloc((i + 2) * sizeof(char *));
	if (!new_args)
		return (0);
	while (j < i)
	{
		new_args[j] = cmd->args[j];
		j++;
	}
	new_args[i] = ft_strdup(arg);
	if (!new_args[i])
	{
		free(new_args);
		return (0);
	}
	new_args[i + 1] = NULL;
	if (cmd->args)
		free(cmd->args);
	cmd->args = new_args;
	return (1);
}
