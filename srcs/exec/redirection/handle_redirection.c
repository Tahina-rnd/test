/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_redirection.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 13:32:52 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/16 15:08:46 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

int	handle_input_redirections(t_redir *input_list)
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

static int	handle_all_output_redirections(t_redir *output_list)
{
	t_redir	*current;
	int		fd;

	if (!output_list)
		return (0);
	current = output_list;
	while (current)
	{
		if (!current->file)
			return (1);
		fd = open_output_file(current);
		if (fd == -1)
			return (1);
		if (!current->next)
		{
			if (dup2(fd, STDOUT_FILENO) == -1)
			{
				close(fd);
				return (1);
			}
		}
		close(fd);
		current = current->next;
	}
	return (0);
}

static int	validate_input_redirection(t_redir *input_list)
{
	t_redir	*last_redir;

	if (!input_list)
		return (0);
	last_redir = get_last_redir(input_list);
	if (!last_redir || !last_redir->file)
		return (1);
	if (access(last_redir->file, F_OK) == -1)
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(last_redir->file, 2);
		ft_putstr_fd(": ", 2);
		ft_putstr_fd(strerror(errno), 2);
		ft_putstr_fd("\n", 2);
		return (1);
	}
	if (access(last_redir->file, R_OK) == -1)
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(last_redir->file, 2);
		ft_putstr_fd(": Permission denied\n", 2);
		return (1);
	}
	return (0);
}

static int	validate_input(t_command *cmd, t_redir *last_hd, t_redir *last_in)
{
	int	ret;

	if (last_hd && last_in)
	{
		if (last_hd->index > last_in->index)
			ret = validate_input_redirection(cmd->heredoc);
		else
			ret = validate_input_redirection(cmd->input_redirection);
	}
	else if (last_hd)
		ret = validate_input_redirection(cmd->heredoc);
	else if (last_in)
		ret = validate_input_redirection(cmd->input_redirection);
	else
		ret = 0;
	return (ret);
}

int	handle_redirections(t_command *cmd)
{
	t_redir	*last_hd;
	t_redir	*last_in;
	int		ret;

	if (!cmd)
		return (0);
	last_hd = NULL;
	last_in = NULL;
	if (cmd->heredoc)
		last_hd = get_last_redir(cmd->heredoc);
	if (cmd->input_redirection)
		last_in = get_last_redir(cmd->input_redirection);
	ret = validate_input(cmd, last_hd, last_in);
	if (ret != 0)
		return (1);
	ret = check_redir(cmd, last_hd, last_in);
	if (ret != 0)
		return (1);
	if (cmd->output_redirection)
	{
		if (handle_all_output_redirections(cmd->output_redirection))
			return (1);
	}
	return (0);
}
