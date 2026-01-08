/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_commands.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 06:37:09 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/08 07:07:59 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static char *remove_quotes(char *str)
{
	char	*result;
	int		i;
	int		j;
	int		in_sq;
	int		in_dq;

	if (!str)
		return (NULL);
	result = malloc(ft_strlen(str) + 1);
	if (!result)
		return (NULL);	
	i = 0;
	j = 0;
	in_sq = 0;
	in_dq = 0;	
	while (str[i])
	{
		if (str[i] == '\'' && !in_dq)
		{
			in_sq = !in_sq;
			i++;
		}
		else if (str[i] == '"' && !in_sq)
		{
			in_dq = !in_dq;
			i++;
		}
		else
			result[j++] = str[i++];
	}
	result[j] = '\0';
	return (result);
}

static char **expand_single_arg(char *arg, t_shell *shell, int was_quoted)
{
	char	*expanded;
	char	**splits;
	char	**result;
	char	*unquoted;
	int		i;

	expanded = expand_string(arg, shell);
	if (!expanded)
		return (NULL);
	if (!was_quoted && ft_strchr(expanded, ' '))
	{
		splits = split_words(expanded);
		free(expanded);
		if (!splits || !splits[0])
			return (NULL);
		i = 0;
		while (splits[i])
		{
			unquoted = remove_quotes(splits[i]);
			if (!unquoted)
				unquoted = ft_strdup("");
			free(splits[i]);
			splits[i] = unquoted;
			i++;
		}
		return (splits);
	}
	else
	{
		unquoted = remove_quotes(expanded);
		free(expanded);
		if (!unquoted)
			unquoted = ft_strdup("");		
		result = malloc(sizeof(char *) * 2);
		if (!result)
		{
			free(unquoted);
			return (NULL);
		}
		result[0] = unquoted;
		result[1] = NULL;
		return (result);
	}
}

static int count_expanded_args(t_command *cmd, t_shell *shell)
{
	char	**expanded;
	int		total;
	int		i;
	int		j;

	total = 0;
	i = 0;
	while (cmd->args && cmd->args[i])
	{
		expanded = expand_single_arg(cmd->args[i], shell, 
			cmd->args_was_quoted[i]);
		if (expanded)
		{
			j = 0;
			while (expanded[j])
			{
				total++;
				free(expanded[j]);
				j++;
			}
			free(expanded);
		}
		i++;
	}
	return (total);
}

static void expand_command_args(t_command *cmd, t_shell *shell)
{
	char	**new_args;
	int		*new_quoted;
	char	**expanded;
	int		total;
	int		i;
	int		j;
	int		k;

	if (!cmd->args || !cmd->args[0])
		return;
	total = count_expanded_args(cmd, shell);
	if (total == 0)
		return;
	new_args = malloc(sizeof(char *) * (total + 1));
	new_quoted = malloc(sizeof(int) * (total + 1));
	if (!new_args || !new_quoted)
	{
		free(new_args);
		free(new_quoted);
		return;
	}
	k = 0;
	i = 0;
	while (cmd->args[i])
	{
		expanded = expand_single_arg(cmd->args[i], shell, 
			cmd->args_was_quoted[i]);
		if (expanded)
		{
			j = 0;
			while (expanded[j])
			{
				new_args[k] = expanded[j];
				new_quoted[k] = 0;
				k++;
				j++;
			}
			free(expanded);
		}
		i++;
	}
	new_args[k] = NULL;
	new_quoted[k] = 0;
	i = 0;
	while (cmd->args[i])
	{
		free(cmd->args[i]);
		i++;
	}
	free(cmd->args);
	free(cmd->args_was_quoted);
	
	cmd->args = new_args;
	cmd->args_was_quoted = new_quoted;
}

static void expand_redirections(t_command *cmd, t_shell *shell)
{
	t_redir	*redir;
	char	*expanded;
	char	*unquoted;

	// Input redirections
	redir = cmd->input_redirection;
	while (redir)
	{
		expanded = expand_string(redir->file, shell);
		
		// ✅ FIX 1 : Vérifier si vide après expansion
		if (!expanded || expanded[0] == '\0')
		{
			ft_putstr_fd("minishell: ", 2);
			ft_putstr_fd(redir->file, 2);
			ft_putstr_fd(": ambiguous redirect\n", 2);
			if (expanded)
				free(expanded);
			// Marquer comme erreur (TODO: gérer le retour d'erreur)
			redir = redir->next;
			continue;
		}
		
		// ✅ FIX 2 : Vérifier si contient des espaces après expansion
		if (ft_strchr(expanded, ' '))
		{
			ft_putstr_fd("minishell: ", 2);
			ft_putstr_fd(redir->file, 2);
			ft_putstr_fd(": ambiguous redirect\n", 2);
			free(expanded);
			redir = redir->next;
			continue;
		}
		
		unquoted = remove_quotes(expanded);
		free(expanded);
		
		// ✅ FIX 3 : Re-vérifier après quote removal
		if (!unquoted || unquoted[0] == '\0')
		{
			ft_putstr_fd("minishell: ", 2);
			ft_putstr_fd(redir->file, 2);
			ft_putstr_fd(": ambiguous redirect\n", 2);
			if (unquoted)
				free(unquoted);
			redir = redir->next;
			continue;
		}
		
		free(redir->file);
		redir->file = unquoted;
		
		redir = redir->next;
	}
	
	// Output redirections (même logique)
	redir = cmd->output_redirection;
	while (redir)
	{
		expanded = expand_string(redir->file, shell);
		
		// ✅ Vérifier si vide
		if (!expanded || expanded[0] == '\0')
		{
			ft_putstr_fd("minishell: ", 2);
			ft_putstr_fd(redir->file, 2);
			ft_putstr_fd(": ambiguous redirect\n", 2);
			if (expanded)
				free(expanded);
			redir = redir->next;
			continue;
		}
		
		// ✅ Vérifier si contient des espaces
		if (ft_strchr(expanded, ' '))
		{
			ft_putstr_fd("minishell: ", 2);
			ft_putstr_fd(redir->file, 2);
			ft_putstr_fd(": ambiguous redirect\n", 2);
			free(expanded);
			redir = redir->next;
			continue;
		}
		
		unquoted = remove_quotes(expanded);
		free(expanded);
		
		// ✅ Re-vérifier après quote removal
		if (!unquoted || unquoted[0] == '\0')
		{
			ft_putstr_fd("minishell: ", 2);
			ft_putstr_fd(redir->file, 2);
			ft_putstr_fd(": ambiguous redirect\n", 2);
			if (unquoted)
				free(unquoted);
			redir = redir->next;
			continue;
		}
		
		free(redir->file);
		redir->file = unquoted;
		
		redir = redir->next;
	}
}

/* ========== FONCTION PRINCIPALE ========== */

/**
 * Expand toutes les commandes
 * À appeler APRÈS parse(), AVANT execute()
 * 
 * Pipeline d'expansion :
 * 1. Variable expansion ($VAR, ${VAR}, $?)
 * 2. Word splitting (IFS simplifié: espaces)
 * 3. Quote removal
 * 4. Globbing (TODO - non implémenté ici)
 */
void expand_commands(t_command *commands, t_shell *shell)
{
	t_command *current;

	if (!commands || !shell)
		return;

	current = commands;
	while (current)
	{
		// 1. Expand les arguments (avec word splitting)
		expand_command_args(current, shell);
		
		// 2. Expand les redirections (sans word splitting)
		expand_redirections(current, shell);
		
		current = current->next;
	}
}