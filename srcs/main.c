/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/15 09:40:43 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/19 10:21:13 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/exec.h"

int	g_received_signal = 0;

static void	process_input(t_shell *shell)
{
	signal(SIGINT, SIG_IGN);
	shell->tokens = lexer(shell);
	if (shell->tokens)
	{
		shell->commands = parser(shell->tokens, shell);
		if (shell->commands)
		{
			expander(shell, shell->commands);
			shell->command_running = 1;
			executor(shell);
			shell->command_running = 0;
		}
	}
}

static void	check_interrupted_signal(t_shell *shell)
{
	if (g_received_signal == SIGINT)
	{
		if (shell->last_exit_status < 128)
			shell->last_exit_status = 130;
		g_received_signal = 0;
	}
}

static void	minishell_loop(t_shell *shell, struct termios *term)
{
	while (1)
	{
		setup_prompt_signal();
		g_received_signal = 0;
		shell->input = readline("minishell$> ");
		check_interrupted_signal(shell);
		if (!shell->input)
		{
			ft_putstr_fd("exit\n", 2);
			break ;
		}
		if (*shell->input)
		{
			add_history(shell->input);
			if (is_exit_command(shell->input))
			{
				free(shell->input);
				shell->input = NULL;
				break ;
			}
			process_input(shell);
		}
		reset_loop(shell);
		tcsetattr(STDIN_FILENO, TCSANOW, term);
	}
}

int	main(int ac, char **av, char **envp)
{
	t_shell			shell;
	struct termios	term;

	(void)ac;
	(void)av;
	tcgetattr(STDIN_FILENO, &term);
	ft_memset(&shell, 0, sizeof(t_shell));
	init_shell(&shell, envp);
	minishell_loop(&shell, &term);
	cleanup_exit(&shell);
	return (shell.last_exit_status);
}
