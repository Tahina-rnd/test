/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   add_args.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:36:20 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/08 05:29:34 by tarandri         ###   ########.fr       */
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

int add_argument(t_command *cmd, char *arg, int was_quoted)
{
	int		i;
	int		j;
	char	**new_args;
	int		*new_quoted;

	if (!cmd || !arg)
		return (0);
	
	i = cmd_args_len(cmd->args);
	new_args = malloc((i + 2) * sizeof(char *));
	if (!new_args)
		return (0);
	new_quoted = malloc((i + 2) * sizeof(int));
	if (!new_quoted)
	{
		free(new_args);
		return (0);
	}
	j = 0;
	while (j < i)
	{
		new_args[j] = cmd->args[j];
		new_quoted[j] = cmd->args_was_quoted[j];
		j++;
	}
	new_args[i] = ft_strdup(arg);
	if (!new_args[i])
	{
		free(new_args);
		free(new_quoted);
		return (0);
	}
	new_quoted[i] = was_quoted;	
	new_args[i + 1] = NULL;
	new_quoted[i + 1] = 0;
	free(cmd->args);
	free(cmd->args_was_quoted);
	cmd->args = new_args;
	cmd->args_was_quoted = new_quoted;	
	return (1);
}
