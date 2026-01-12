/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc_process.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 16:43:56 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 23:41:38 by miokrako         ###   ########.fr       */
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

char	*generate_tmpfile_name(char *delimiter, int index)
{
	char	*index_str;
	char	*tmp1;
	char	*tmp2;
	char	*result;

	index_str = ft_itoa(index);
	if (!index_str)
		return (NULL);
	tmp1 = ft_strjoin("/tmp/.heredoc_", delimiter);
	if (!tmp1)
	{
		free(index_str);
		return (NULL);
	}
	tmp2 = ft_strjoin(tmp1, "_");
	free(tmp1);
	if (!tmp2)
	{
		free(index_str);
		return (NULL);
	}
	result = ft_strjoin(tmp2, index_str);
	free(tmp2);
	free(index_str);
	return (result);
}
