/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/15 09:40:43 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/12 16:32:21 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/exec.h"

int	g_received_signal = 0;

static void	process_input(t_shell *shell)
{
	signal(SIGINT, SIG_IGN);
	shell->tokens = lexer(shell->input);
	if (shell->tokens)
	{
		shell->commands = parser(shell->tokens, shell);
		if (shell->commands)
		{
			expander(shell, shell->commands);
			executor(shell);
		}
	}
}

static void	check_interrupted_signal(t_shell *shell)
{
	if (g_received_signal == SIGINT)
	{
		shell->last_exit_status = 130;
		g_received_signal = 0;
	}
}

static void	minishell_loop(t_shell *shell)
{
	while (1)
	{
		setup_prompt_signal();
		g_received_signal = 0;
		shell->input = readline("minishell$> ");
		check_interrupted_signal(shell);
		if (!shell->input)
		{
			printf("exit\n");
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
	}
}

int	main(int ac, char **av, char **envp)
{
	t_shell	shell;

	(void)ac;
	(void)av;
	ft_memset(&shell, 0, sizeof(t_shell));
	init_shell(&shell, envp);
	minishell_loop(&shell);
	cleanup_exit(&shell);
	return (shell.last_exit_status);
}
