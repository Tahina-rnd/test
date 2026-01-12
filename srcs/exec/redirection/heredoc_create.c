/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc_create.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 16:43:56 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 23:42:40 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	handle_heredoc_interrupt(char *line, int fd, char *filename)
{
	if (line)
		free(line);
	if (fd != -1)
		close(fd);
	if (filename)
		unlink(filename);
	return (-1);
}

static int	handle_heredoc_eof(char *delimiter, int fd)
{
	ft_putstr_fd("minishell: warning: here-document delimited ", 2);
	ft_putstr_fd("by end-of-file (wanted `", 2);
	ft_putstr_fd(delimiter, 2);
	ft_putstr_fd("')\n", 2);
	close(fd);
	return (0);
}

static int	check_heredoc_line(char *line, char *delimiter, int fd,
		char *filename)
{
	if (g_received_signal == SIGINT)
		return (handle_heredoc_interrupt(line, fd, filename));
	if (!line)
		return (handle_heredoc_eof(delimiter, fd));
	if (ft_strcmp(line, delimiter) == 0)
	{
		free(line);
		close(fd);
		return (0);
	}
	return (-2);
}

static int	heredoc_loop(int fd, char *delimiter, char *filename)
{
	char	*line;
	int		status;

	while (1)
	{
		if (g_received_signal == SIGINT)
		{
			close(fd);
			unlink(filename);
			return (-1);
		}
		line = readline("> ");
		status = check_heredoc_line(line, delimiter, fd, filename);
		if (status != -2)
			return (status);
		write(fd, line, ft_strlen(line));
		write(fd, "\n", 1);
		free(line);
	}
}

int	create_heredoc_file(char *delimiter, char *filename)
{
	int		fd;

	fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1)
	{
		perror("minishell: heredoc");
		return (-1);
	}
	setup_heredoc_signals();
	g_received_signal = 0;
	return (heredoc_loop(fd, delimiter, filename));
}
