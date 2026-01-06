/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:33:27 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/05 15:23:46 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/exec.h"

int g_received_signal = 0;

int main(int argc, char **argv, char **envp)
{
	t_shell	shell;

	(void)argc;
	(void)argv;

	// Initialisation complète à NULL/0
	shell.input = NULL;
	shell.env = NULL;
	shell.tokens = NULL;
	shell.commands = NULL;
	shell.last_exit_status = 0;

	// Vérification de base
	if (!envp)
	{
		ft_putstr_fd("minishell: invalid environment\n", 2);
		return (1);
	}

	// Duplication de l'environnement
	shell.env = dup_env(envp);
	if (!shell.env)
	{
		ft_putstr_fd("minishell: error initializing environment\n", 2);
		return (1);
	}

	// Configuration des signaux
	setup_prompt_signal();

	// Boucle principale
	while (1)
	{
		g_received_signal = 0;

		// Lecture de l'entrée
		shell.input = readline("minishell> ");

		if (g_received_signal == SIGINT)
        {
            shell.last_exit_status = 130;
            g_received_signal = 0;

            // Si readline a retourné NULL après Ctrl+C
            if (!shell.input)
                continue;
        }
		// EOF (Ctrl+D)
		if (!shell.input)
		{
			ft_putstr_fd("exit\n", 2);
			break;
		}

		// Ligne vide
		if (shell.input[0] == '\0')
		{
			free(shell.input);
			shell.input = NULL;
			continue;
		}

		// Ajout à l'historique
		add_history(shell.input);

		// Tokenisation
		shell.tokens = tokenize(shell.input);
		if (!shell.tokens)
		{
			free(shell.input);
			shell.input = NULL;
			continue;
		}

		// Vérification syntaxe
		if (check_tokens(shell.tokens))
		{
			shell.last_exit_status = 2;
			free_tokens(shell.tokens);
			shell.tokens = NULL;
			free(shell.input);
			shell.input = NULL;
			continue;
		}

		// Expansion
		expand_tokens(shell.tokens, &shell);
		clean_empty_tokens(&shell.tokens);

		// Si plus de tokens après nettoyage
		if (!shell.tokens)
		{
			free(shell.input);
			shell.input = NULL;
			continue;
		}

		// Parsing
		shell.commands = parse(shell.tokens);
		if (!shell.commands)
		{
			free_tokens(shell.tokens);
			shell.tokens = NULL;
			free(shell.input);
			shell.input = NULL;
			continue;
		}

		// Vérification commande valide
		if (shell.commands->args && shell.commands->args[0])
		{
			// Exécution
			executor(&shell);
		}

		// Nettoyage
		if (shell.tokens)
		{
			free_tokens(shell.tokens);
			shell.tokens = NULL;
		}
		if (shell.commands)
		{
			free_cmds(shell.commands);
			shell.commands = NULL;
		}
		if (shell.input)
		{
			free(shell.input);
			shell.input = NULL;
		}
	}

	// Nettoyage final
	if (shell.env)
		free_env_list(shell.env);
	if (shell.tokens)
		free_tokens(shell.tokens);
	if (shell.commands)
		free_cmds(shell.commands);
	if (shell.input)
		free(shell.input);

	rl_clear_history();

	return (shell.last_exit_status);
}
