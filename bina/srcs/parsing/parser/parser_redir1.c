/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_redir.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:31:40 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/09 00:05:28 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static t_redir	*create_redir(char *file, int append_mode)
{
	t_redir	*redir;

	redir = (t_redir *)malloc(sizeof(t_redir));
	if (!redir)
		return (NULL);
	redir->file = ft_strdup(file);
	if (!redir->file)
	{
		free(redir);
		return (NULL);
	}
	redir->fd = -1;
	redir->append_mode = append_mode;
	redir->next = NULL;
	return (redir);
}

static void	add_redir_to_list(t_redir **list, t_redir *new_redir)
{
	t_redir	*current;

	if (!new_redir)
		return ;
	if (!*list)
	{
		*list = new_redir;
		return ;
	}
	current = *list;
	while (current->next)
		current = current->next;
	current->next = new_redir;
}

void	add_input_redir(t_command *cmd, char *file)
{
	t_redir	*redir;

	redir = create_redir(file, 0);
	if (!redir)
		return ;
	add_redir_to_list(&cmd->input_redirection, redir);
}

void	add_output_redir(t_command *cmd, char *file, int append)
{
	t_redir	*redir;

	redir = create_redir(file, append);
	if (!redir)
		return ;
	add_redir_to_list(&cmd->output_redirection, redir);
}

void	add_heredoc(t_command *cmd, char *delimiter)
{
	t_redir	*redir;

	redir = create_redir(delimiter, 0);
	if (!redir)
		return ;
	add_redir_to_list(&cmd->heredoc, redir);
}
