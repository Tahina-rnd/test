/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   wait_children.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 20:13:41 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/14 14:55:58 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static void	check_last_process_signal(int status, pid_t wpid,
	pid_t last_pid, int *last_sig_quit)
{
	if (wpid == last_pid && WIFSIGNALED(status))
	{
		if (WTERMSIG(status) == SIGQUIT)
			*last_sig_quit = 1;
	}
}

static void	wait_loop(pid_t last_pid, int *last_status,
	int *any_sig_int, int *last_sig_quit)
{
	pid_t	wpid;
	int		status;

	while (1)
	{
		wpid = waitpid(-1, &status, 0);
		if (wpid == -1)
			break ;
		if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
			*any_sig_int = 1;
		check_last_process_signal(status, wpid, last_pid, last_sig_quit);
		if (wpid == last_pid)
			*last_status = status;
	}
}

void	wait_all_children(pid_t last_pid, t_shell *shell)
{
	int	last_status;
	int	any_sig_int;
	int	last_sig_quit;

	any_sig_int = 0;
	last_sig_quit = 0;
	last_status = 0;
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	wait_loop(last_pid, &last_status, &any_sig_int, &last_sig_quit);
	setup_prompt_signal();
	update_exit_status(shell, last_status, any_sig_int, last_sig_quit);
}
