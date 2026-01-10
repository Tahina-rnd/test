/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/08 15:45:17 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/08 16:00:35 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

/* ========================================================================== */
/*                         EXPANSION DU CONTENU HEREDOC                       */
/* ========================================================================== */

static void	expand_var_value(char *line, int *i, int fd, t_shell *shell)
{
	char	*var_name;
	char	*var_value;
	int		start;

	start = *i;
	while (line[*i] && (ft_isalnum(line[*i]) || line[*i] == '_'))
		(*i)++;
	var_name = ft_substr(line, start, *i - start);
	var_value = get_env_value(shell->env, var_name);
	if (var_value)
		write(fd, var_value, ft_strlen(var_value));
	free(var_name);
}

static void	expand_dollar(char *line, int *i, int fd, t_shell *shell)
{
	char	*var_value;

	(*i)++;
	if (line[*i] == '?')
	{
		var_value = ft_itoa(shell->last_exit_status);
		write(fd, var_value, ft_strlen(var_value));
		free(var_value);
		(*i)++;
	}
	else if (ft_isalpha(line[*i]) || line[*i] == '_')
		expand_var_value(line, i, fd, shell);
	else if (ft_isdigit(line[*i]))
		(*i)++;
	else
		write(fd, "$", 1);
}

void	expand_heredoc_line(char *line, int fd, t_shell *shell)
{
	int	i;

	i = 0;
	while (line[i])
	{
		if (line[i] == '$' && line[i + 1])
			expand_dollar(line, &i, fd, shell);
		else
		{
			write(fd, &line[i], 1);
			i++;
		}
	}
	write(fd, "\n", 1);
}

/* ========================================================================== */
/*                      DÉTECTION DE L'EXPANSION NÉCESSAIRE                   */
/* ========================================================================== */

static int	should_expand(char *delimiter)
{
	if (ft_strchr(delimiter, '\'') || ft_strchr(delimiter, '"'))
		return (0);
	return (1);
}

static char	*clean_delimiter(char *delimiter)
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

/* ========================================================================== */
/*                        CRÉATION DU FICHIER HEREDOC                         */
/* ========================================================================== */

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
int	create_heredoc_file(char *delimiter, char *filename)
{
	int		fd;
	char	*line;

	fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1)
	{
		perror("minishell: heredoc");
		return (-1);
	}
	setup_heredoc_signals();
	g_received_signal = 0;
	while (1)
	{
		if (g_received_signal == SIGINT)
		{
			close(fd);
			unlink(filename);
			return (-1);
		}
		line = readline("> ");
		if (g_received_signal == SIGINT)
		{
			return (handle_heredoc_interrupt(line, fd, filename));
		}
		if (!line)
		{
			return (handle_heredoc_eof(delimiter, fd));
		}
		if (ft_strcmp(line, delimiter) == 0)
		{
			free(line);
			close(fd);
			return (0);
		}
		write(fd, line, ft_strlen(line));
		write(fd, "\n", 1);
		free(line);
	}
}

/* ========================================================================== */
/*                     EXPANSION DU CONTENU DU HEREDOC                        */
/* ========================================================================== */

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

static void	expand_heredoc_content(t_shell *shell, char *tmpfile)
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

/* ========================================================================== */
/*                    GESTION DES PROCESSUS HEREDOC                           */
/* ========================================================================== */

static int	child_heredoc_process(char *clean_delim, char *tmpfile,
		int do_expand, t_shell *shell)
{
	if (create_heredoc_file(clean_delim, tmpfile) == -1)
	{
		free(clean_delim);
		cleanup_child(shell);
		exit(130);
	}
	if (do_expand)
		expand_heredoc_content(shell, tmpfile);
	free(clean_delim);
	cleanup_child(shell);
	exit(0);
}

static int	parent_wait_heredoc(pid_t pid, t_shell *shell)
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

/* ========================================================================== */
/*                   TRAITEMENT D'UN SEUL NŒUD HEREDOC                        */
/* ========================================================================== */

static char	*generate_tmpfile_name(char *delimiter, int index)
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

static int	process_single_heredoc_node(t_redir *heredoc_node, t_shell *shell,
		int index)
{
	int		do_expand;
	char	*clean_delim;
	char	*tmpfile;
	pid_t	pid;

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
		free(heredoc_node->file);
		heredoc_node->file = tmpfile;
	}
	free(clean_delim);
	return (0);
}

/* ========================================================================== */
/*              TRAITEMENT DE TOUS LES HEREDOCS D'UNE COMMANDE                */
/* ========================================================================== */

static int	process_cmd_heredocs(t_command *cmd, t_shell *shell)
{
	t_redir	*current;
	int		result;
	int		index;

	if (!cmd || !cmd->heredoc)
		return (0);
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

/* ========================================================================== */
/*             TRAITEMENT DE TOUS LES HEREDOCS DE TOUTES LES COMMANDES       */
/* ========================================================================== */

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

/* ========================================================================== */
/*                     NETTOYAGE DES FICHIERS TEMPORAIRES                     */
/* ========================================================================== */

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
			if (heredoc->file)
				unlink(heredoc->file);
			heredoc = heredoc->next;
		}
		cmd = cmd->next;
	}
}
