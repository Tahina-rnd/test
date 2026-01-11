/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/29 14:57:18 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/10 21:41:48 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

t_command	*create_command(void)
{
	t_command	*cmd;

	cmd = malloc(sizeof(t_command));
	if (!cmd)
		return (NULL);
	cmd->args = NULL;
	cmd->input_redirection = NULL;
	cmd->output_redirection = NULL;
	cmd->heredoc = NULL;
	cmd->next = NULL;
	return (cmd);
}

void	cmd_add_back(t_command **lst, t_command *new)
{
	t_command	*temp;

	if (!lst || !new)
		return ;
	if (!*lst)
	{
		*lst = new;
		return ;
	}
	temp = *lst;
	while (temp->next)
		temp = temp->next;
	temp->next = new;
}

/* ** Crée un node t_arg et l'ajoute à la fin de la liste cmd->args
*/
int	add_arg(t_command *cmd, t_token *token)
{
	t_arg	*new_arg;
	t_arg	*temp;

	new_arg = malloc(sizeof(t_arg));
	if (!new_arg)
		return (0);
	new_arg->value = NULL; // On construira la valeur finale après expansion
	new_arg->segments = token->segments; // On récupère les segments
	token->segments = NULL; // IMPORTANT : On détache du token pour éviter le double free
	new_arg->next = NULL;
	if (!cmd->args)
		cmd->args = new_arg;
	else
	{
		temp = cmd->args;
		while (temp->next)
			temp = temp->next;
		temp->next = new_arg;
	}
	return (1);
}

/* ** Fonction helper pour nettoyer la liste des redirections 
*/
static void	free_redirs(t_redir *lst)
{
	t_redir	*temp;

	while (lst)
	{
		temp = lst;
		lst = lst->next;
		if (temp->file)
			free(temp->file);
		free(temp);
	}
}

/*
** Fonction helper pour nettoyer la liste des arguments 
** (Maintenant possible car t_arg a un next !)
*/
static void	free_args(t_arg *lst)
{
	t_arg	*temp;

	while (lst)
	{
		temp = lst;
		lst = lst->next;
		if (temp->value)
			free(temp->value);
		free(temp);
	}
}

void	free_commands(t_command *cmds)
{
	t_command	*temp;

	while (cmds)
	{
		temp = cmds;
		cmds = cmds->next;
		free_args(temp->args);
		free_redirs(temp->input_redirection);
		free_redirs(temp->output_redirection);
		free_redirs(temp->heredoc);
		free(temp);
	}
}
