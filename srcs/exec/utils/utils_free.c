/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_free.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 13:28:19 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/02 14:09:08 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"


void free_env(t_env *env)
{
    t_env *tmp;

    while (env)
    {
        tmp = env->next;
        free(env->key);
        free(env->value);
        free(env);
        env = tmp;
    }
}

void free_command(t_command *cmd)
{
    int i;

    if (cmd->args)
    {
        i = 0;
        while (cmd->args[i])
        {
            free(cmd->args[i]);
            i++;
        }
        free(cmd->args);
    }

    if (cmd->input_redirection)
        free(cmd->input_redirection);
    if (cmd->output_redirection)
        free(cmd->output_redirection);

    free(cmd);
}

void free_commands(t_command *commands)
{
    t_command *tmp;

    while (commands)
    {
        tmp = commands->next;
        free_command(commands);
        commands = tmp;
    }
}

void cleanup_shell(t_shell *shell)
{
    if (shell->env)
        free_env(shell->env);
    if (shell->commands)
        free_commands(shell->commands);
    if (shell->input)
        free(shell->input);
}
void free_tab_partial(char **tab, int count)
{
    int i = 0;
    while (i < count)
    {
        free(tab[i]);
        i++;
    }
    free(tab);
}
void free_tab(char **tab)
{
    int i = 0;
    while (tab[i])
    {
        free(tab[i]);
        i++;
    }
    free(tab);
}

void cleanup_child(t_shell *shell)
{
    // Libérer l'environnement
    if (shell->env)
        free_env(shell->env);

    // Libérer les commandes
    if (shell->commands)
        free_commands(shell->commands);

    // Ne PAS libérer input (appartient au parent)
    // Ne PAS libérer tokens (appartient au parent)

    // Note: On ne fait PAS rl_clear_history() car c'est un fils
}
