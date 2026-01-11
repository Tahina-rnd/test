/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_args.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 07:03:05 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/09 00:06:48 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static int	get_arg_count(t_arg *args)
{
	int	count;

	count = 0;
	if (!args)
		return (0);
	while (args[count].value)
		count++;
	return (count);
}

static t_arg	*realloc_args(t_arg *old_args, int old_count, int new_count)
{
	t_arg	*new_args;
	int		i;

	new_args = (t_arg *)malloc(sizeof(t_arg) * (new_count + 1));
	if (!new_args)
		return (NULL);
	i = 0;
	while (i < old_count)
	{
		new_args[i].value = old_args[i].value;
		new_args[i].was_quoted = old_args[i].was_quoted;
		i++;
	}
	while (i <= new_count)
	{
		new_args[i].value = NULL;
		new_args[i].was_quoted = 0;
		i++;
	}
	if (old_args)
		free(old_args);
	return (new_args);
}

int	add_arg_to_command(t_command *cmd, t_token *token)
{
	int		count;
	t_arg	*new_args;

	if (!cmd || !token)
		return (0);
	count = get_arg_count(cmd->args);
	new_args = realloc_args(cmd->args, count, count + 1);
	if (!new_args)
		return (0);
	new_args[count].value = ft_strdup(token->value);
	if (!new_args[count].value)
	{
		free(new_args);
		return (0);
	}
	new_args[count].was_quoted = token->was_quoted;
	new_args[count + 1].value = NULL;
	new_args[count + 1].was_quoted = 0;
	cmd->args = new_args;
	return (1);
}
