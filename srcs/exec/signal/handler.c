/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handler.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 23:32:57 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 23:33:57 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

void	sig_handler_prompt(int sig)
{
	(void)sig;
	g_received_signal = SIGINT;
	write(STDOUT_FILENO, "\n", 1);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}

void	sig_handler_exec(int sig)
{
	if (sig == SIGINT)
	{
		g_received_signal = SIGINT;
	}
	else if (sig == SIGQUIT)
	{
		g_received_signal = SIGQUIT;
	}
}

void	sig_handler_heredoc(int sig)
{
	if (sig == SIGINT)
	{
		g_received_signal = SIGINT;
		write(STDOUT_FILENO, "\n", 1);
		close(STDIN_FILENO);
	}
}
