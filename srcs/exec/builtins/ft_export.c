/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 22:37:54 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/12 17:33:24 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	is_valid_identifier_export(const char *str)
{
	int	i;

	if (!str || (!ft_isalpha(str[0]) && str[0] != '_'))
		return (0);
	i = 1;
	while (str[i] && str[i] != '=')
	{
		if (!ft_isalnum(str[i]) && str[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

static void	print_export(t_shell *shell)
{
	t_env	*current;

	current = shell->env;
	while (current)
	{
		if (current->value && *current->value)
			printf("declare -x %s=\"%s\"\n", current->key, current->value);
		else
			printf("declare -x %s\n", current->key);
		current = current->next;
	}
}

static int	process_export_arg(t_shell *shell, char *arg)
{
	char	*key;
	char	*value;

	key = extract_key(arg);
	value = extract_value(arg);
	if (value)
	{
		update_or_add_env(shell, key, value);
		free(value);
	}
	else
		update_or_add_env(shell, key, "");
	free(key);
	return (0);
}

int	ft_export(t_shell *shell, char **args)
{
	int		i;
	int		has_error;

	if (!args[1])
	{
		print_export(shell);
		return (0);
	}
	i = 1;
	has_error = 0;
	while (args[i])
	{
		if (!is_valid_identifier_export(args[i]))
		{
			printf("export: `%s': not a valid identifier\n", args[i]);
			has_error = 1;
			i++;
			continue ;
		}
		process_export_arg(shell, args[i]);
		i++;
	}
	return (has_error);
}
