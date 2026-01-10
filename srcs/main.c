/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:33:27 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/09 14:59:06 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/exec.h"

int	g_received_signal = 0;

int	main(int argc, char **argv, char **envp)
{
	t_shell	shell;

	(void)argc;
	(void)argv;
	shell.input = NULL;
	shell.env = NULL;
	shell.tokens = NULL;
	shell.commands = NULL;
	shell.last_exit_status = 0;
	if (!envp)
	{
		ft_putstr_fd("minishell: invalid environment\n", 2);
		return (1);
	}
	init_env(&shell, envp);
	setup_prompt_signal();
	while (1)
	{
		g_received_signal = 0;
		shell.input = readline("minishell> ");
		// printf("input = %s\n", shell.input);
		if (g_received_signal == SIGINT)
		{
			shell.last_exit_status = 130;
			g_received_signal = 0;
			if (!shell.input)
				continue ;
		}
		if (!shell.input)
		{
			ft_putstr_fd("exit\n", 2);
			break ;
		}
		if (shell.input[0] == '\0')
		{
			free(shell.input);
			shell.input = NULL;
			continue ;
		}
		add_history(shell.input);
		shell.tokens = lexer(shell.input);
		// printf("token_1 = %s\n", shell.tokens->next->value);
		// printf("token_1 = %i\n", shell.tokens->next->was_quoted);
		if (!shell.tokens)
		{
			free(shell.input);
			shell.input = NULL;
			continue ;
		}
		if (!validate_tokens(shell.tokens))
		{
			shell.last_exit_status = 2;
			free_tokens(shell.tokens);
			shell.tokens = NULL;
			free(shell.input);
			shell.input = NULL;
			continue ;
		}
		shell.commands = parser(shell.tokens);
		if (!shell.commands)
		{
			shell.last_exit_status = 1;
			free_tokens(shell.tokens);
			shell.tokens = NULL;
			free(shell.input);
			shell.input = NULL;
			continue ;
		}
		// printf("post parser = %s\n", shell.commands->args->value);
		// printf("post parser = %s\n", shell.commands->input_redirection->file);
		// printf("post parser = %i\n", shell.commands->args->was_quoted);
		expander(shell.commands, shell.env, shell.last_exit_status);
		if (shell.commands->args)
			executor(&shell);
		if (shell.tokens)
		{
			free_tokens(shell.tokens);
			shell.tokens = NULL;
		}
		if (shell.commands)
		{
			free_commands(shell.commands);
			shell.commands = NULL;
		}
		if (shell.input)
		{
			free(shell.input);
			shell.input = NULL;
		}
	}
	if (shell.env)
		free_env(shell.env);
	if (shell.tokens)
		free_tokens(shell.tokens);
	if (shell.commands)
		free_commands(shell.commands);
	if (shell.input)
		free(shell.input);
	rl_clear_history();
	return (shell.last_exit_status);
}
