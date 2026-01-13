/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_unset.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 22:41:18 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/13 14:05:41 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

int	is_valid_identifier(const char *str)
{
	int	i;

	if (!str || (!ft_isalpha(str[0]) && str[0] != '_'))
		return (0);
	i = 1;
	while (str[i])
	{
		if (!ft_isalnum(str[i]) && str[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

static void	remove_env_node(t_env **head, char *key)
{
	t_env	*current;
	t_env	*prev;

	if (!head || !*head)
		return ;
	current = *head;
	prev = NULL;
	while (current)
	{
		if (ft_strcmp(current->key, key) == 0)
		{
			if (prev == NULL)
				*head = current->next;
			else
				prev->next = current->next;
			free(current->key);
			free(current->value);
			free(current);
			return ;
		}
		prev = current;
		current = current->next;
	}
}

int	ft_unset(t_shell *shell, char **args)
{
	int	i;
	int	error;

	if (!args[1])
		return (0);
	i = 1;
	error = 0;
	while (args[i])
	{
		if (!is_valid_identifier(args[i]))
		{
			ft_putstr_fd("unset : ", 2);
			ft_putstr_fd(args[i], 2);
			ft_putstr_fd(" : not a valid identifier\n", 2);
			shell->last_exit_status = 1;
			error = 1;
		}
		else
			remove_env_node(&shell->env, args[i]);
		i++;
	}
	return (error);
}
