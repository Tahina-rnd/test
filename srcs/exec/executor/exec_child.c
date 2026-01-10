/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_child.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 20:13:41 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/10 22:52:31 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static void	setup_pipes(t_command *cmd, int prev_pipe[2], int curr_pipe[2])
{
	if (prev_pipe[0] != -1)
	{
		dup2(prev_pipe[0], STDIN_FILENO);
		close(prev_pipe[0]);
		if (prev_pipe[1] != -1)
			close(prev_pipe[1]);
	}
	if (cmd->next)
	{
		if (curr_pipe[0] != -1)
			close(curr_pipe[0]);
		if (curr_pipe[1] != -1)
		{
			dup2(curr_pipe[1], STDOUT_FILENO);
			close(curr_pipe[1]);
		}
	}
}

// CORRECTION ICI : Parcours de liste chainee
static char	**convert_args_to_array(t_arg *args)
{
	char	**result;
	int		count;
	int		i;
	t_arg	*tmp;

	if (!args)
		return (NULL);
	count = 0;
	tmp = args;
	while (tmp)
	{
		count++;
		tmp = tmp->next;
	}
	result = (char **)malloc(sizeof(char *) * (count + 1));
	if (!result)
		return (NULL);
	i = 0;
	tmp = args;
	while (i < count)
	{
		result[i] = tmp->value;
		tmp = tmp->next;
		i++;
	}
	result[i] = NULL;
	return (result);
}

void	child_process(t_command *cmd, t_shell *shell, int prev[2], int curr[2])
{
	int		ret;
	char	**args_array;

	setup_child_signals();
	setup_pipes(cmd, prev, curr);
	if (handle_redirections(cmd) != 0)
	{
		cleanup_child(shell);
		exit(1);
	}
	// CORRECTION: cmd->args est un pointeur, cmd->args->value est le premier arg
	if (!cmd->args || !cmd->args->value)
	{
		cleanup_child(shell);
		exit(0);
	}
	if (is_builtin(cmd->args->value))
	{
		ret = execute_builtin(cmd, shell);
		cleanup_child(shell);
		exit(ret);
	}
	else
	{
		args_array = convert_args_to_array(cmd->args);
		if (!args_array)
		{
			cleanup_child(shell);
			exit(1);
		}
		exec_simple_cmd_with_array(cmd, shell->env, args_array);
		free(args_array);
		cleanup_child(shell);
		exit(126);
	}
}

// ... (Gardez wait_all_children tel quel)
void	wait_all_children(pid_t last_pid, t_shell *shell)
{
	pid_t	wpid;
	int		status;
	int		last_status;
	int		sig[2];

	sig[0] = 0;
	sig[1] = 0;
	last_status = 0;
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	while (1)
	{
		wpid = waitpid(-1, &status, 0);
		if (wpid == -1)
			break ;
		if (WIFSIGNALED(status))
		{
			if (WTERMSIG(status) == SIGINT)
				sig[0] = 1;
			else if (WTERMSIG(status) == SIGQUIT)
				sig[1] = 1;
		}
		if (wpid == last_pid)
			last_status = status;
	}
	setup_prompt_signal();
	if (WIFEXITED(last_status))
		shell->last_exit_status = WEXITSTATUS(last_status);
	else if (WIFSIGNALED(last_status))
		shell->last_exit_status = 128 + WTERMSIG(last_status);
	if (sig[0])
		write(1, "\n", 1);
	else if (sig[1])
		ft_putstr_fd("Quit (core dumped)\n", 2);
}
