/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:33:27 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/10 22:24:27 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/exec.h"

int	g_received_signal = 0;

/*
** Gestionnaire de signaux simple pour le mode interactif (readline)
*/
void	handle_sigint(int sig)
{
	(void)sig;
	g_received_signal = sig;
	write(1, "\n", 1);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}

/*
** Initialisation de la structure Shell et de l'environnement
*/
static void	init_shell(t_shell *shell, char **envp)
{
	shell->env = dup_env(envp);
	// Si env est vide (env -i), on crée un env minimal pour éviter les segfaults
	if (!shell->env)
	{
		shell->env = create_env_node("PWD", getcwd(NULL, 0)); // Hack rapide
		// Idéalement faudrait aussi gérer SHLVL et _
	}
	shell->input = NULL;
	shell->tokens = NULL;
	shell->commands = NULL;
	shell->last_exit_status = 0;
}

/*
** Nettoyage à chaque tour de boucle (Loop Garbage Collector)
*/
static void	reset_loop(t_shell *shell)
{
	if (shell->commands)
	{
		free_commands(shell->commands);
		shell->commands = NULL;
	}
	if (shell->tokens)
	{
		free_tokens(shell->tokens);
		shell->tokens = NULL;
	}
	if (shell->input)
	{
		free(shell->input);
		shell->input = NULL;
	}
}

/*
** Nettoyage final à la sortie du programme (exit)
*/
static void	cleanup_exit(t_shell *shell)
{
	reset_loop(shell); // Nettoie le tour courant si besoin
	if (shell->env)
		free_env_list(shell->env);
	rl_clear_history();
}

/*
** Boucle principale
*/
static void	minishell_loop(t_shell *shell)
{
	while (1)
	{
		// 1. Configuration des signaux pour le prompt
		signal(SIGINT, handle_sigint);
		signal(SIGQUIT, SIG_IGN); // Ignore Ctrl+\ dans le prompt

		// 2. Lecture
		shell->input = readline("minishell$> ");
		if (!shell->input) // Ctrl+D détecté
		{
			printf("exit\n");
			break ;
		}
		if (*shell->input)
		{
			add_history(shell->input);
			
			// 3. Parsing Pipeline
			shell->tokens = lexer(shell->input);
			if (shell->tokens)
			{
				// Note : Parser peut retourner NULL si erreur de syntaxe
				shell->commands = parser(shell->tokens);
				
				if (shell->commands)
				{
					// 4. Expansion
					expander(shell, shell->commands);
					
					// 5. Exécution (On ignore les signaux pendant l'exec parent)
					// C'est l'executor qui gèrera les signaux pour les enfants
					signal(SIGINT, SIG_IGN);
					
					executor(shell); 
				}
			}
		}
		// 6. Nettoyage de l'itération
		reset_loop(shell);
	}
}

int	main(int ac, char **av, char **envp)
{
	t_shell	shell;

	(void)ac;
	(void)av;

	// Initialisation
	ft_memset(&shell, 0, sizeof(t_shell));
	init_shell(&shell, envp);

	// Lancement
	minishell_loop(&shell);

	// Sortie propre
	cleanup_exit(&shell);
	return (shell.last_exit_status);
}
