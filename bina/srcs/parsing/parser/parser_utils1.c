/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_utils1.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 16:00:00 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/09 00:04:10 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

t_command	*new_command(void)
{
	t_command	*cmd;

	cmd = (t_command *)malloc(sizeof(t_command));
	if (!cmd)
		return (NULL);
	cmd->args = NULL;
	cmd->input_redirection = NULL;
	cmd->output_redirection = NULL;
	cmd->heredoc = NULL;
	cmd->next = NULL;
	return (cmd);
}

t_command	*get_last_command(t_command *cmds)
{
	if (!cmds)
		return (NULL);
	while (cmds->next)
		cmds = cmds->next;
	return (cmds);
}

void	free_redirections(t_redir *redir)
{
	t_redir	*tmp;

	while (redir)
	{
		tmp = redir;
		redir = redir->next;
		if (tmp->file)
			free(tmp->file);
		free(tmp);
	}
}

void	free_args(t_arg *args, int count)
{
	int	i;

	if (!args)
		return ;
	i = 0;
	while (i < count)
	{
		if (args[i].value)
			free(args[i].value);
		i++;
	}
	free(args);
}

void	free_command(t_command *cmd)
{
	int	count;

	if (!cmd)
		return ;
	if (cmd->args)
	{
		count = count_args(cmd);
		free_args(cmd->args, count);
	}
	free_redirections(cmd->input_redirection);
	free_redirections(cmd->output_redirection);
	free_redirections(cmd->heredoc);
	free(cmd);
}
