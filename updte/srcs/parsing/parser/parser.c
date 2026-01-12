/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 07:45:47 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/11 14:58:39 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static int	handle_pipe(t_command **current_cmd, t_token **curr_token)
{
	// Vérifie qu'on n'a pas un pipe vide ou au début
	if (!(*current_cmd)->args && !(*current_cmd)->input_redirection
		&& !(*current_cmd)->output_redirection && !(*current_cmd)->heredoc)
	{
		printf("Minishell: syntax error near unexpected token `|'\n");
		return (0);
	}
	// Vérifie qu'on n'a pas un pipe juste avant la fin
	if ((*curr_token)->next && (*curr_token)->next->type == END)
	{
		printf("Minishell: syntax error near unexpected token `|'\n");
		return (0);
	}
	*current_cmd = create_command();
	if (!*current_cmd)
		return (0);
	return (1);
}

static int	process_token(t_command **curr_cmd, t_token **tokens,
	t_command **head)
{
	if ((*tokens)->type == WORD)
	{
		// ✅ Ajoute le mot comme argument
		if (!add_arg(*curr_cmd, *tokens))
			return (0);
	}
	else if ((*tokens)->type >= REDIRECT_IN && (*tokens)->type <= HEREDOC)
	{
		// ✅ CRITIQUE : Parse la redirection
		// Cette fonction va consommer le token suivant (le fichier/délimiteur)
		if (!parse_redir(*curr_cmd, tokens))
			return (0);
		// ⚠️ IMPORTANT : parse_redir avance déjà tokens sur le fichier
		// Donc on ne doit PAS avancer ici, c'est fait en fin de boucle
	}
	else if ((*tokens)->type == PIPE)
	{
		if (!handle_pipe(curr_cmd, tokens))
			return (0);
		cmd_add_back(head, *curr_cmd);
	}
	return (1);
}

t_command	*parser(t_token *tokens)
{
	t_command	*head;
	t_command	*curr_cmd;

	if (!tokens)
		return (NULL);
	if (tokens->type == PIPE)
	{
		printf("Minishell: syntax error near unexpected token `|'\n");
		return (NULL);
	}
	head = create_command();
	if (!head)
		return (NULL);
	curr_cmd = head;
	while (tokens && tokens->type != END)
	{
		if (!process_token(&curr_cmd, &tokens, &head))
		{
			free_commands(head);
			return (NULL);
		}
		tokens = tokens->next;
	}
	return (head);
}
