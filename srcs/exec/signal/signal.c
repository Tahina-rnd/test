/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 15:17:38 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/05 11:58:47 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

void sig_handler_prompt(int sig)
{
	(void)sig;

	g_received_signal = SIGINT;
	write(STDOUT_FILENO, "\n", 1);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}

void sig_handler_exec(int sig)
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

void sig_handler_heredoc(int sig)
{
	if (sig == SIGINT)
	{
		g_received_signal = SIGINT;

		// Le ^C est déjà affiché par le terminal
		// On ajoute juste un newline
		write(STDOUT_FILENO, "\n", 1);
        close(STDIN_FILENO);

		// Forcer readline à retourner NULL
		// (équivalent à recevoir EOF)
		// rl_done = 1;
	}
}

void setup_prompt_signal(void)
{
	struct sigaction	sa_int;
	struct sigaction	sa_quit;

	sigemptyset(&sa_int.sa_mask);
	sa_int.sa_flags = SA_RESTART;  // Redémarre les syscalls interrompus
	sa_int.sa_handler = sig_handler_prompt;
	sigaction(SIGINT, &sa_int, NULL);

	sigemptyset(&sa_quit.sa_mask);
	sa_quit.sa_flags = 0;
	sa_quit.sa_handler = SIG_IGN;  // Ignorer complètement
	sigaction(SIGQUIT, &sa_quit, NULL);
}

void setup_exec_signals(void)
{
	struct sigaction	sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SIG_IGN;  // Ignorer dans le parent

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
}

void setup_child_signals(void)
{
	struct sigaction	sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SIG_DFL;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
}

void setup_heredoc_signals(void)
{
	struct sigaction	sa_int;
	struct sigaction	sa_quit;

	// Configuration pour SIGINT (Ctrl+C)
	sigemptyset(&sa_int.sa_mask);
	sa_int.sa_flags = 0;  // PAS de SA_RESTART - important!
	sa_int.sa_handler = sig_handler_heredoc;
	sigaction(SIGINT, &sa_int, NULL);

	// Configuration pour SIGQUIT (Ctrl+\)
	sigemptyset(&sa_quit.sa_mask);
	sa_quit.sa_flags = 0;
	sa_quit.sa_handler = SIG_IGN;  // Ignorer
	sigaction(SIGQUIT, &sa_quit, NULL);
}
