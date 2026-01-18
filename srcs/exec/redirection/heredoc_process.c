/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc_process.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 16:43:56 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/15 12:02:12 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

int	child_heredoc_process(char *clean_delim, char *tmpfile,
		int do_expand, t_shell *shell)
{
	if (create_heredoc_file(clean_delim, tmpfile) == -1)
	{
		free(clean_delim);
		free(tmpfile);
		cleanup_child(shell);
		exit(130);
	}
	if (do_expand)
		expand_heredoc_content(shell, tmpfile);
	free(clean_delim);
	free(tmpfile);
	cleanup_child(shell);
	exit(0);
}

int	parent_wait_heredoc(pid_t pid, t_shell *shell)
{
	int	status;
	int	exit_code;

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	waitpid(pid, &status, 0);
	setup_prompt_signal();
	if (WIFEXITED(status))
	{
		exit_code = WEXITSTATUS(status);
		if (exit_code != 0)
		{
			shell->last_exit_status = exit_code;
			return (1);
		}
	}
	else if (WIFSIGNALED(status))
	{
		shell->last_exit_status = 128 + WTERMSIG(status);
		return (1);
	}
	return (0);
}

static char	*generate_tmpfile_base(char *delimiter)
{
	char	*pid_str;
	char	*tmp;
	char	*result;

	pid_str = ft_itoa(getpid());
	if (!pid_str)
		return (NULL);
	tmp = ft_strjoin("/tmp/.heredoc_", pid_str);
	free(pid_str);
	if (!tmp)
		return (NULL);
	result = ft_strjoin(tmp, "_");
	free(tmp);
	if (!result)
		return (NULL);
	tmp = ft_strjoin(result, delimiter);
	free(result);
	if (!tmp)
		return (NULL);
	result = ft_strjoin(tmp, "_");
	free(tmp);
	return (result);
}

char	*generate_tmpfile_name(char *delimiter, int index)
{
	char	*index_str;
	char	*base;
	char	*result;

	base = generate_tmpfile_base(delimiter);
	if (!base)
		return (NULL);
	index_str = ft_itoa(index);
	if (!index_str)
		return (free(base), NULL);
	result = ft_strjoin(base, index_str);
	free(base);
	free(index_str);
	return (result);
}
