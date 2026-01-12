#include "../includes/exec.h"

int	g_received_signal = 0;

static void	handle_sigint(int sig)
{
	(void)sig;
	g_received_signal = sig;
	write(1, "\n", 1);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}

static void	init_shell(t_shell *shell, char **envp)
{
	char	*cwd;

	shell->env = dup_env(envp);
	if (!shell->env)
	{
		cwd = getcwd(NULL, 0);
		if (!cwd)
			cwd = ft_strdup("/");
		shell->env = create_env_node("PWD", cwd);
		free(cwd);
	}
	shell->input = NULL;
	shell->tokens = NULL;
	shell->commands = NULL;
	shell->last_exit_status = 0;
}

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

static void	cleanup_exit(t_shell *shell)
{
	reset_loop(shell);
	if (shell->env)
		free_env_list(shell->env);
	rl_clear_history();
}

static int	is_exit_command(char *input)
{
	int	i;

	i = 0;
	while (input[i] && (input[i] == ' ' || input[i] == '\t'))
		i++;
	if (ft_strncmp(&input[i], "exit", 4) == 0)
	{
		i += 4;
		while (input[i] && (input[i] == ' ' || input[i] == '\t'))
			i++;
		if (input[i] == '\0')
			return (1);
	}
	return (0);
}

static void	process_input(t_shell *shell)
{
	signal(SIGINT, SIG_IGN);
	shell->tokens = lexer(shell->input);
	if (shell->tokens)
	{
		shell->commands = parser(shell->tokens);
		if (shell->commands)
		{
			expander(shell, shell->commands);
			executor(shell);
		}
	}
}

static void	minishell_loop(t_shell *shell)
{
	while (1)
	{
		signal(SIGINT, handle_sigint);
		signal(SIGQUIT, SIG_IGN);
		shell->input = readline("minishell$> ");
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
