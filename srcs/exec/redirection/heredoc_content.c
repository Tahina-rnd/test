/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc_content.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 16:43:56 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 23:42:05 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static void	copy_expanded_file(int fd_in, int fd_out)
{
	char	buffer[4096];
	ssize_t	bytes;

	while (1)
	{
		bytes = read(fd_in, buffer, 4096);
		if (bytes <= 0)
			break ;
		write(fd_out, buffer, bytes);
	}
}

static void	process_heredoc_lines(int fd_in, int fd_out, t_shell *shell)
{
	char	*line;
	int		len;

	line = get_next_line(fd_in);
	while (line != NULL)
	{
		len = ft_strlen(line);
		if (len > 0 && line[len - 1] == '\n')
			line[len - 1] = '\0';
		expand_heredoc_line(line, fd_out, shell);
		free(line);
		line = get_next_line(fd_in);
	}
}

static void	replace_heredoc_file(char *tmpfile)
{
	int		fd_in;
	int		fd_final;
	char	*final_file;

	final_file = ft_strjoin(tmpfile, ".final");
	if (!final_file)
		return ;
	fd_in = open(final_file, O_RDONLY);
	fd_final = open(tmpfile, O_WRONLY | O_TRUNC);
	if (fd_in != -1 && fd_final != -1)
		copy_expanded_file(fd_in, fd_final);
	if (fd_in != -1)
		close(fd_in);
	if (fd_final != -1)
		close(fd_final);
	unlink(final_file);
	free(final_file);
}

void	expand_heredoc_content(t_shell *shell, char *tmpfile)
{
	int		fd_in;
	int		fd_out;
	char	*final_file;

	final_file = ft_strjoin(tmpfile, ".final");
	if (!final_file)
		return ;
	fd_in = open(tmpfile, O_RDONLY);
	fd_out = open(final_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd_in != -1 && fd_out != -1)
	{
		process_heredoc_lines(fd_in, fd_out, shell);
		close(fd_in);
		close(fd_out);
		replace_heredoc_file(tmpfile);
	}
	else
	{
		if (fd_in != -1)
			close(fd_in);
		if (fd_out != -1)
			close(fd_out);
	}
	free(final_file);
}
