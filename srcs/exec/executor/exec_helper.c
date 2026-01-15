/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_helper.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 08:12:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/14 23:53:37 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

void update_exit_status(t_shell *shell, int last_status, int sig_int, int sig_quit)
{
    // ✅ PRIORITÉ 1 : SIGQUIT
    if (sig_quit)
    {
        shell->last_exit_status = 131;
        ft_putstr_fd("Quit (core dumped)\n", 2);
        return;
    }

    // ✅ PRIORITÉ 2 : SIGINT
    if (sig_int)
    {
        shell->last_exit_status = 130;
        write(1, "\n", 1);
        return;
    }

    // ✅ PRIORITÉ 3 : Exit status normal
    if (WIFEXITED(last_status))
    {
        shell->last_exit_status = WEXITSTATUS(last_status);
    }
    else if (WIFSIGNALED(last_status))
    {
        shell->last_exit_status = 128 + WTERMSIG(last_status);
    }
}

void	setup_pipes(t_command *cmd, int prev_pipe[2], int curr_pipe[2])
{
	if (prev_pipe[0] != -1)
	{
		dup2(prev_pipe[0], STDIN_FILENO);
		close(prev_pipe[0]);
		if (prev_pipe[1] != -1)
			close(prev_pipe[1]);
	}
	if (cmd->next)
	{
		if (curr_pipe[0] != -1)
			close(curr_pipe[0]);
		if (curr_pipe[1] != -1)
		{
			dup2(curr_pipe[1], STDOUT_FILENO);
			close(curr_pipe[1]);
		}
	}
}
char *get_path(t_env *env, char *cmd)
{
    char *path_var;
    char **paths;
    char *result;
    char *cwd_path;  // ✅ Nouveau

    if (cmd[0] == '\0')
        return (NULL);

    // Si la commande contient "/", retourner tel quel
    if (ft_strchr(cmd, '/'))
    {
        if (access(cmd, F_OK) == -1)
            return (NULL);
        return (ft_strdup(cmd));
    }

    path_var = get_env_value(env, "PATH");

    // ✅ Si PATH est vide/unset, chercher dans le répertoire courant
    if (!path_var)
    {
        cwd_path = ft_strjoin("./", cmd);
        if (!cwd_path)
            return (NULL);
        if (access(cwd_path, F_OK | X_OK) == 0)
            return (cwd_path);
        free(cwd_path);
        return (NULL);  // Pas trouvé dans le répertoire courant
    }

    // Chercher dans PATH normalement
    paths = ft_split(path_var, ':');
    if (!paths)
        return (NULL);
    result = try_paths(paths, cmd);
    free_tab(paths);
    return (result);
}

void check_special_cases(char *cmd, t_shell *shell, char **args_array)
{
    // Cas spéciaux : "." et ".."
    if (ft_strcmp(cmd, ".") == 0)
    {
        ft_putstr_fd("minishell: .: filename argument required\n", 2);
        ft_putstr_fd(".: usage: . filename [arguments]\n", 2);
        free(args_array);
        cleanup_child(shell);
        exit(2);  // Bash retourne 2 pour cette erreur
    }
    if (ft_strcmp(cmd, "..") == 0)
    {
        ft_putstr_fd("minishell: ..: command not found\n", 2);
        free(args_array);
        cleanup_child(shell);
        exit(127);
    }

    // ✅ NOUVEAU : Si la commande contient "/" mais n'existe pas
    if (ft_strchr(cmd, '/'))
    {
        // Vérifier si le chemin existe
        if (access(cmd, F_OK) == -1)
        {
            ft_putstr_fd("minishell: ", 2);
            ft_putstr_fd(cmd, 2);
            ft_putstr_fd(": No such file or directory\n", 2);
            free(args_array);
            cleanup_child(shell);
            exit(127);
        }
    }
}

void	handle_command_not_found(char *cmd, char **args, t_shell *shell)
{
	char *path_var = get_env_value(shell->env, "PATH");

	ft_putstr_fd("minishell: ", 2);
	ft_putstr_fd(cmd, 2);

	// ✅ Message adapté selon PATH
	if (!path_var)
		ft_putstr_fd(": No such file or directory\n", 2);
	else
		ft_putstr_fd(": command not found\n", 2);
	free(args);
	cleanup_child(shell);
	exit(127);
}
