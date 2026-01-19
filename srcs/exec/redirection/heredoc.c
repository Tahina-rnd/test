/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/08 15:45:17 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/19 12:38:03 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

int	should_expand(char *delimiter)
{
	if (ft_strchr(delimiter, '\'') || ft_strchr(delimiter, '"'))
		return (0);
	return (1);
}

char	*clean_delimiter(char *delimiter)
{
	char	*cleaned;
	int		i;
	int		j;
	int		in_quote;

	cleaned = malloc(ft_strlen(delimiter) + 1);
	if (!cleaned)
		return (NULL);
	i = 0;
	j = 0;
	in_quote = 0;
	while (delimiter[i])
	{
		if ((delimiter[i] == '\'' || delimiter[i] == '"') && !in_quote)
			in_quote = delimiter[i];
		else if (delimiter[i] == in_quote)
			in_quote = 0;
		else
			cleaned[j++] = delimiter[i];
		i++;
	}
	cleaned[j] = '\0';
	return (cleaned);
}

int	process_heredocs(t_shell *shell)
{
	t_command	*cmd;

	if (!shell || !shell->commands)
		return (0);
	cmd = shell->commands;
	while (cmd)
	{
		if (process_cmd_heredocs(cmd, shell))
			return (1);
		cmd = cmd->next;
	}
	return (0);
}

void	cleanup_heredocs(t_shell *shell)
{
	t_command	*cmd;
	t_redir		*heredoc;

	if (!shell || !shell->commands)
		return ;
	cmd = shell->commands;
	while (cmd)
	{
		heredoc = cmd->heredoc;
		while (heredoc)
		{
			if (heredoc->file && ft_strncmp(heredoc->file, "/tmp/.heredoc_", 14) == 0)
				unlink(heredoc->file);
			heredoc = heredoc->next;
		}
		cmd = cmd->next;
	}
}
