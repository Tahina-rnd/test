/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 22:37:54 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/19 10:28:50 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static void	parse_export_components(char *arg, char **key,
	char **value, int *append)
{
	*append = is_append_mode(arg);
	*key = extract_key(arg);
	*value = extract_value(arg);
}

static void	update_existing_var(t_env *exist, char *value,
	int append, t_shell *shell)
{
	exist->exported = 1;
	if (value != NULL)
	{
		if (append)
			handle_append_export(shell, exist->key, value);
		else
		{
			free(exist->value);
			exist->value = ft_strdup(value);
		}
	}
}

static int	find_and_update_var(t_shell *shell, char *key,
	char *value, int append)
{
	t_env	*exist;

	exist = shell->env;
	while (exist)
	{
		if (ft_strcmp(exist->key, key) == 0)
		{
			update_existing_var(exist, value, append, shell);
			return (1);
		}
		exist = exist->next;
	}
	return (0);
}

static int	process_export_arg(t_shell *shell, char *arg)
{
	char	*key;
	char	*value;
	int		append;

	parse_export_components(arg, &key, &value, &append);
	if (find_and_update_var(shell, key, value, append))
	{
		free(key);
		if (value)
			free(value);
		return (0);
	}
	if (append && value)
		handle_append_export(shell, key, value);
	else
		update_or_add_env(shell, key, value);
	free(key);
	if (value)
		free(value);
	return (0);
}

int	ft_export(t_shell *shell, char **args)
{
	int		i;
	int		has_error;

	if (!args[1])
	{
		export_onl(shell);
		return (0);
	}
	i = 1;
	has_error = 0;
	while (args[i])
	{
		if (!is_valid_identifier_export(args[i]))
		{
			has_error = handle_export_error(args[i]);
			i++;
			continue ;
		}
		process_export_arg(shell, args[i]);
		i++;
	}
	return (has_error);
}
