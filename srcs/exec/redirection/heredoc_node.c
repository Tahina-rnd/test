/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc_node.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 16:43:56 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/14 16:39:54 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	fork_heredoc(char *clean_delim, char *tmpfile,
		int do_expand, t_shell *shell)
{
	pid_t	pid;

	pid = fork();
	if (pid == -1)
	{
		perror("minishell: fork");
		free(clean_delim);
		free(tmpfile);
		return (1);
	}
	if (pid == 0)
		child_heredoc_process(clean_delim, tmpfile, do_expand, shell);
	else
	{
		if (parent_wait_heredoc(pid, shell))
		{
			free(clean_delim);
			free(tmpfile);
			return (1);
		}
	}
	return (0);
}

int	process_single_heredoc_node(t_redir *heredoc_node, t_shell *shell,
		int index)
{
	int		do_expand;
	char	*clean_delim;
	char	*tmpfile;

	if (!heredoc_node || !heredoc_node->file)
		return (1);
	tmpfile = generate_tmpfile_name(heredoc_node->file, index);
	if (!tmpfile)
		return (1);
	do_expand = should_expand(heredoc_node->file);
	clean_delim = clean_delimiter(heredoc_node->file);
	if (!clean_delim)
	{
		free(tmpfile);
		return (1);
	}
	if (fork_heredoc(clean_delim, tmpfile, do_expand, shell))
		return (1);
	free(heredoc_node->file);
	heredoc_node->file = tmpfile;
	free(clean_delim);
	return (0);
}

// int	process_cmd_heredocs(t_command *cmd, t_shell *shell)
// {
// 	t_redir	*current;
// 	int		result;
// 	int		index;

// 	if (!cmd || !cmd->heredoc)
// 		return (0);
// 	current = cmd->heredoc;
// 	index = 0;
// 	while (current)
// 	{
// 		result = process_single_heredoc_node(current, shell, index);
// 		if (result != 0)
// 			return (result);
// 		current = current->next;
// 		index++;
// 	}
// 	return (0);
// }


int	process_cmd_heredocs(t_command *cmd, t_shell *shell)
{
	t_redir	*current;
	int		result;
	int		index;
	int		count;

	if (!cmd || !cmd->heredoc)
		return (0);

	// Compter le nombre de heredocs
	current = cmd->heredoc;
	count = 0;
	while (current)
	{
		count++;
		current = current->next;
	}

	// Vérifier la limite
	if (count > MAX_HEREDOCS)
	{
		ft_putstr_fd("minishell: maximum here-document count exceeded\n", 2);
		// shell->last_exit_status = 2;
		// return (1);
		cleanup_child(shell);
		exit (2);
	}

	// Traitement normal
	current = cmd->heredoc;
	index = 0;
	while (current)
	{
		result = process_single_heredoc_node(current, shell, index);
		if (result != 0)
			return (result);
		current = current->next;
		index++;
	}
	return (0);
}
