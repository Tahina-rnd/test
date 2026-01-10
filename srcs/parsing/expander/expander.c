/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:44:36 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/10 07:18:02 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static char	*remove_quotes(char *str)
{
	char	*result;
	int		i;
	int		j;

	if (!str)
		return (NULL);
	result = (char *)malloc(ft_strlen(str) + 1);
	if (!result)
		return (NULL);
	i = 0;
	j = 0;
	while (str[i])
	{
		if (str[i] != '\'' && str[i] != '"')
		{
			result[j] = str[i];
			j++;
		}
		i++;
	}
	result[j] = '\0';
	return (result);
}

void	expand_args(t_command *cmd, t_env *env, int last_exit_status)
{
	t_arg	*new_args;
	int		count;
	int		i;
	char	*expanded;
	char	*without_quotes;
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
		without_quotes = remove_quotes(expanded);
		free(expanded);
		if (!without_quotes)
			without_quotes = ft_strdup("");
		if (!cmd->args[i].was_quoted)
		{
			words = word_split(without_quotes, env);
			j = 0;
			while (words && words[j])
			{
				push_arg(&new_args, &count, words[j], 0);
				j++;
			}
			free_split(words);
		}
		else
			push_arg(&new_args, &count, without_quotes, 1);
		free(without_quotes);
		i++;
	}
	replace_args(cmd, new_args, count);
}

void	expand_redirections(t_command *cmd, t_env *env, int exit_status)
{
	t_redir	*redir;
	char	*expanded;
	char	*without_quotes;
	char	*tmp;

	redir = cmd->input_redirection;
	while (redir)
	{
		expanded = expand_variables(redir->file, env, exit_status);
		without_quotes = remove_quotes(expanded);
		free(expanded);
		if (without_quotes && is_ambiguous_redirect(without_quotes, 0))
		{
			ft_putstr_fd("minishell: ", 2);
			ft_putstr_fd(redir->file, 2);
			ft_putendl_fd(": ambiguous redirect", 2);
			free(without_quotes);
		}
		else if (without_quotes)
		{
			tmp = redir->file;
			redir->file = without_quotes;
			free(tmp);
		}
		redir = redir->next;
	}
	redir = cmd->output_redirection;
	while (redir)
	{
		expanded = expand_variables(redir->file, env, exit_status);
		without_quotes = remove_quotes(expanded);
		free(expanded);
		if (without_quotes && is_ambiguous_redirect(without_quotes, 0))
		{
			ft_putstr_fd("minishell: ", 2);
			ft_putstr_fd(redir->file, 2);
			ft_putendl_fd(": ambiguous redirect", 2);
			free(without_quotes);
		}
		else if (without_quotes)
		{
			tmp = redir->file;
			redir->file = without_quotes;
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
