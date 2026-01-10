/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 10:39:18 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/09 14:41:59 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

void	free_split(char **split)
{
	int	i;

	if (!split)
		return ;
	i = 0;
	while (split[i])
	{
		free(split[i]);
		i++;
	}
	free(split);
}

void	push_arg(t_arg **args, int *count, char *value, int was_quoted)
{
	t_arg	*new_args;
	int		i;

	new_args = (t_arg *)malloc(sizeof(t_arg) * (*count + 2));
	if (!new_args)
		return ;
	i = 0;
	while (i < *count)
	{
		new_args[i].value = (*args)[i].value;
		new_args[i].was_quoted = (*args)[i].was_quoted;
		i++;
	}
	new_args[*count].value = ft_strdup(value);
	new_args[*count].was_quoted = was_quoted;
	new_args[*count + 1].value = NULL;
	new_args[*count + 1].was_quoted = 0;
	if (*args)
		free(*args);
	*args = new_args;
	(*count)++;
}

void	replace_args(t_command *cmd, t_arg *new_args, int count)
{
	int	i;

	if (cmd->args)
	{
		i = 0;
		while (cmd->args[i].value)
		{
			free(cmd->args[i].value);
			i++;
		}
		free(cmd->args);
	}
	cmd->args = new_args;
	(void)count;
}

int	is_ambiguous_redirect(char *expanded, int was_quoted)
{
	int	word_count;
	int	i;

	if (was_quoted)
		return (0);
	if (!expanded || !expanded[0])
		return (1);
	word_count = 0;
	i = 0;
	while (expanded[i])
	{
		while (expanded[i] && (expanded[i] == ' ' || expanded[i] == '\t'))
			i++;
		if (expanded[i])
		{
			word_count++;
			while (expanded[i] && expanded[i] != ' ' && expanded[i] != '\t')
				i++;
		}
	}
	return (word_count != 1);
}

char	*expand_redir_file(char *file, t_env *env, int exit_status)
{
	char	*expanded;

	expanded = expand_variables(file, env, exit_status);
	if (!expanded)
		return (NULL);
	// printf ("expand_redir : expanded = %s\n", expanded);
	if (is_ambiguous_redirect(expanded, 0))
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(file, 2);
		ft_putendl_fd(": ambiguous redirect", 2);
		free(expanded);
		return (NULL);
	}
	return (expanded);
}
