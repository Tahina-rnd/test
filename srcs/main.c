/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:33:27 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/08 06:49:59 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/exec.h"

int g_received_signal = 0;

int main(int argc, char **argv, char **envp)
{
	t_shell	shell;

	(void)argc;
	(void)argv;

	// ========== INITIALISATION ==========
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

	// ========== BOUCLE PRINCIPALE ==========
	while (1)
	{
		g_received_signal = 0;

		// ===== 1. LECTURE DE L'ENTRÉE =====
		shell.input = readline("minishell> ");

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

		// ===== 2. TOKENISATION (LEXER) =====
		shell.tokens = tokenize(shell.input);
		if (!shell.tokens)
		{
			free(shell.input);
			shell.input = NULL;
			continue;
		}

		// ===== 3. VÉRIFICATION SYNTAXE =====
		if (check_tokens(shell.tokens))
		{
			shell.last_exit_status = 2;
			free_tokens(shell.tokens);
			shell.tokens = NULL;
			free(shell.input);
			shell.input = NULL;
			continue;
		}

		// ===== 4. PARSING (CONSTRUCTION AST) =====
		shell.commands = parse(shell.tokens);
		if (!shell.commands)
		{
			free_tokens(shell.tokens);
			shell.tokens = NULL;
			free(shell.input);
			shell.input = NULL;
			continue;
		}

		// ===== 5. EXPANSION =====
		// - Variable expansion ($VAR)
		// - Word splitting (IFS)
		// - Quote removal
		// - Globbing (TODO)
		expand_commands(shell.commands, &shell);

		// Vérification commande valide après expansion
		if (!shell.commands->args || !shell.commands->args[0])
		{
			// Commande vide après expansion (ex: VAR="" ; echo $VAR)
			free_tokens(shell.tokens);
			shell.tokens = NULL;
			free_cmds(shell.commands);
			shell.commands = NULL;
			free(shell.input);
			shell.input = NULL;
			continue;
		}

		// ===== 6. EXÉCUTION =====
		executor(&shell);

		// ===== 7. NETTOYAGE =====
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

	// ========== NETTOYAGE FINAL ==========
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
