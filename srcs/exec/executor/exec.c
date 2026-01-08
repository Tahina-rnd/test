/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 15:27:36 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/06 21:08:23 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

char *get_path(t_env *env, char *cmd)
{
    char    *path_var;
    char    **paths;
    char    *full_path;
    char    *part_path;
    int     i;

    if (ft_strchr(cmd, '/'))
    {
        // Vérifier si le fichier existe
        if (access(cmd, F_OK) == -1)
            return (NULL);  // 127 - command not found

        // Le fichier existe, vérifier les permissions d'exécution
        if (access(cmd, X_OK) == -1)
        {
            // IMPORTANT: Retourner le chemin même sans permission
            // exec_simple_cmd() gérera l'erreur 126
            return (ft_strdup(cmd));
        }

        return (ft_strdup(cmd));
    }
    path_var = get_env_value(env, "PATH");
    if (!path_var)
        return (NULL);
    paths = ft_split(path_var, ':');
    if (!paths)
        return (NULL);
    i = 0;
    while (paths[i])
    {
        part_path = ft_strjoin(paths[i], "/");
        if (!part_path)
        {
            free_tab(paths);
            return (NULL);
        }
        full_path = ft_strjoin(part_path, cmd);
        free(part_path);
        if (!full_path)
        {
            free_tab(paths);
            return (NULL);
        }
        if (access(full_path, F_OK | X_OK) == 0)
        {
            free_tab(paths);
            return (full_path);
        }
        free(full_path);
        i++;
    }
    free_tab(paths);
    return (NULL);
}

void exec_simple_cmd(t_command *cmd, t_env *env)
{
    char    *path;
    char    **env_tab;
    struct stat path_stat;

    if (!cmd || !cmd->args || !cmd->args[0])
        exit(0);
    if (ft_strcmp(cmd->args[0], ".") == 0)
    {
        ft_putstr_fd("minishell: .: filename argument required\n", 2);
        exit(2);
    }
    if (ft_strcmp(cmd->args[0], "..") == 0)
    {
        ft_putstr_fd("minishell: ..: command not found\n", 2);
        exit(127);
    }

    path = get_path(env, cmd->args[0]);
    if (!path)
    {
        ft_putstr_fd("minishell: ", 2);
        ft_putstr_fd(cmd->args[0], 2);
        ft_putstr_fd(": command not found\n", 2);
        exit(127);
    }
    if (stat(path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
    {
        ft_putstr_fd("minishell: ", 2);
        ft_putstr_fd(cmd->args[0], 2);
        ft_putstr_fd(": is a directory\n", 2);
        free(path);
        exit(126);
    }
    if (access(path, X_OK) == -1)
    {
        ft_putstr_fd("minishell: ", 2);
        ft_putstr_fd(cmd->args[0], 2);
        ft_putstr_fd(": Permission denied\n", 2);
        free(path);
        exit(126);  // Permission denied
    }
    env_tab = env_to_tab(env);
    if (!env_tab)
    {
        perror("minishell: malloc");
        free(path);
        exit(1);
    }
    if (execve(path, cmd->args, env_tab) == -1)
    {
        if (errno == ENOEXEC)
        {
            char **sh_args = malloc(sizeof(char *) * 3);
            if (sh_args)
            {
                sh_args[0] = ft_strdup("/bin/sh");
                sh_args[1] = ft_strdup(path);
                sh_args[2] = NULL;
                execve(sh_args[0], sh_args, env_tab);
                free_tab(sh_args);
            }
        }
        perror("minishell");
        free(path);
        free_tab(env_tab);
        exit(126);
    }
}

static void child_process(t_command *cmd, t_shell *shell,
                         int prev_pipe[2], int curr_pipe[2])
{
    setup_child_signals();
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
    if (handle_redirections(cmd) != 0)
    {
        cleanup_child(shell);
        exit(1);
    }
    if (is_builtin(cmd->args[0]))
    {
        int ret = execute_builtin(cmd, shell);
        cleanup_child(shell);
        exit(ret);
    }
    else
    {
        exec_simple_cmd(cmd, shell->env);
        cleanup_child(shell);
        exit(126);
    }
}
static void wait_all_children(pid_t last_pid, t_shell *shell, int has_pipe)
{
    pid_t   wpid;
    int     status;
    int     last_status;

    last_status = 0;
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    while (1)
    {
        wpid = waitpid(-1, &status, 0);
        if (wpid == -1)
            break;

        if (wpid == last_pid)
            last_status = status;
    }

    setup_prompt_signal();

    if (WIFEXITED(last_status))
        shell->last_exit_status = WEXITSTATUS(last_status);
    else if (WIFSIGNALED(last_status))
    {
        int sig = WTERMSIG(last_status);
        shell->last_exit_status = 128 + sig;

        // MODIFICATION: Distinguer commande simple vs pipeline
        if (sig == SIGINT)  // Ctrl+C
        {
            // Afficher newline SEULEMENT si pas de pipe
            if (has_pipe)
                write(1, "\n", 1);
            // Sinon (pipeline), ne rien afficher
        }
        else if (sig == SIGQUIT)  // Ctrl+
        {
            ft_putstr_fd("Quit (core dumped)\n", 2);
        }
    }
}

void executor(t_shell *shell)
{
    t_command   *cmd;
    int         prev_pipe[2];
    int         curr_pipe[2];
    pid_t       pid;
    pid_t       last_pid;
    int         has_pipe;  // NOUVEAU

    prev_pipe[0] = -1;
    prev_pipe[1] = -1;
    cmd = shell->commands;

    // NOUVEAU: Détecter si on a un pipeline
    has_pipe = (cmd && cmd->next) ? 1 : 0;

    if (process_heredocs(shell) != 0)
    {
        return;
    }
    if (cmd && !cmd->next && cmd->args && cmd->args[0]
        && check_parent_builtin(cmd->args[0], shell))
    {
        exec_builtin_parent(cmd, shell);
        cleanup_heredocs(shell);
        return;
    }
    setup_exec_signals();
    while (cmd)
    {
        if (cmd->next)
        {
            if (pipe(curr_pipe) == -1)
            {
                perror("minishell: pipe");
                if (prev_pipe[0] != -1)
                {
                    close(prev_pipe[0]);
                    close(prev_pipe[1]);
                }
                shell->last_exit_status = 1;
                cleanup_heredocs(shell);
                return;
            }
        }
        pid = fork();
        if (pid == -1)
        {
            perror("minishell: fork");
            if (prev_pipe[0] != -1)
            {
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }
            if (cmd->next)
            {
                close(curr_pipe[0]);
                close(curr_pipe[1]);
            }
            shell->last_exit_status = 1;
            cleanup_heredocs(shell);
            return;
        }
        if (pid == 0)
        {
            child_process(cmd, shell, prev_pipe, curr_pipe);
        }
        else
        {
            last_pid = pid;
            if (prev_pipe[0] != -1)
            {
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }
            if (cmd->next)
            {
                close(curr_pipe[1]);
                prev_pipe[0] = curr_pipe[0];
                prev_pipe[1] = -1;
            }
        }
        cmd = cmd->next;
    }

    // MODIFICATION: Passer has_pipe à wait_all_children
    wait_all_children(last_pid, shell, has_pipe);
    cleanup_heredocs(shell);
}
