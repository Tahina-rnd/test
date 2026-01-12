/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_redirection.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 13:32:52 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/12 11:19:31 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	handle_input_redirections(t_redir *input_list)
{
	t_redir	*last_redir;
	int		fd;

	if (!input_list)
		return (0);
	last_redir = get_last_redir(input_list);
	if (!last_redir || !last_redir->file)
	{
		return (1);
	}
	fd = open_input_file(last_redir);
	if (fd == -1)
		return (1);
	if (dup2(fd, STDIN_FILENO) == -1)
	{
		perror("minishell: dup2");
		close(fd);
		return (1);
	}
	close(fd);
	return (0);
}

static int	process_intermediate_outputs(t_redir *current)
{
	int	fd;

	while (current->next)
	{
		if (!current->file)
		{
			return (1);
		}
		fd = open_output_file(current);
		if (fd == -1)
			return (1);
		close(fd);
		current = current->next;
	}
	return (0);
}

static int	handle_last_output(t_redir *last_redir)
{
	int	fd;

	if (!last_redir->file)
	{
		return (1);
	}
	fd = open_output_file(last_redir);
	if (fd == -1)
		return (1);
	if (dup2(fd, STDOUT_FILENO) == -1)
	{
		perror("minishell: dup2");
		close(fd);
		return (1);
	}
	close(fd);
	return (0);
}

static int	handle_all_output_redirections(t_redir *output_list)
{
	t_redir	*current;

	if (!output_list)
		return (0);
	current = output_list;
	if (process_intermediate_outputs(current))
		return (1);
	return (handle_last_output(current));
}

int	handle_redirections(t_command *cmd)
{
	if (!cmd)
		return (0);
	if (cmd->heredoc)
	{
		if (handle_input_redirections(cmd->heredoc))
			return (1);
	}
	else if (cmd->input_redirection)
	{
		if (handle_input_redirections(cmd->input_redirection))
			return (1);
	}
	if (cmd->output_redirection)
	{
		if (handle_all_output_redirections(cmd->output_redirection))
			return (1);
	}
	return (0);
}
