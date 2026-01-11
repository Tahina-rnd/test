/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:44:36 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/09 00:29:22 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

void	expand_args(t_command *cmd, t_env *env, int last_exit_status)
{
	t_arg	*new_args;
	int		count;
	int		i;
	char	*expanded;
	char	**words;
	int		j;

	new_args = NULL;
	count = 0;
	i = 0;
	if (!cmd->args)
		return ;
	while (cmd->args[i].value)
	{
		expanded = expand_variables(cmd->args[i].value, env, last_exit_status);
		if (!expanded)
			expanded = ft_strdup("");
		if (!cmd->args[i].was_quoted)
		{
			words = word_split(expanded, env);
			j = 0;
			while (words && words[j])
			{
				push_arg(&new_args, &count, words[j], 0);
				j++;
			}
			free_split(words);
		}
		else
			push_arg(&new_args, &count, expanded, 1);
		free(expanded);
		i++;
	}
	replace_args(cmd, new_args, count);
}

void	expand_redirections(t_command *cmd, t_env *env, int exit_status)
{
	t_redir	*redir;
	char	*expanded;
	char	*tmp;

	redir = cmd->input_redirection;
	while (redir)
	{
		expanded = expand_redir_file(redir->file, env, exit_status);
		if (expanded)
		{
			tmp = redir->file;
			redir->file = expanded;
			free(tmp);
		}
		redir = redir->next;
	}
	redir = cmd->output_redirection;
	while (redir)
	{
		expanded = expand_redir_file(redir->file, env, exit_status);
		if (expanded)
		{
			tmp = redir->file;
			redir->file = expanded;
			free(tmp);
		}
		redir = redir->next;
	}
}

void	expand_command(t_command *cmd, t_env *env, int last_exit_status)
{
	if (!cmd)
		return ;
	expand_args(cmd, env, last_exit_status);
	expand_redirections(cmd, env, last_exit_status);
}

void	expander(t_command *commands, t_env *env, int last_exit_status)
{
	t_command	*current;

	current = commands;
	while (current)
	{
		expand_command(current, env, last_exit_status);
		current = current->next;
	}
}
