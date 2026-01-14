/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 22:37:54 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/14 19:48:32 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	is_append_mode(const char *arg)
{
	int	i;

	i = 0;
	while (arg[i] && arg[i] != '=')
		i++;
	if (i > 0 && arg[i - 1] == '+' && arg[i] == '=')
		return (1);
	return (0);
}

static int	handle_append_export(t_shell *shell, char *key, char *value)
{
	char	*old_val;
	char	*new_val;

	if (key && key[ft_strlen(key) - 1] == '+')
		key[ft_strlen(key) - 1] = '\0';
	old_val = get_env_value(shell->env, key);
	if (old_val)
	{
		new_val = ft_strjoin(old_val, value);
		update_or_add_env(shell, key, new_val);
		free(new_val);
	}
	else
		update_or_add_env(shell, key, value);
	free(value);
	return (0);
}

static int	process_export_arg(t_shell *shell, char *arg)
{
	char	*key;
	char	*value;
	int		append;

	append = is_append_mode(arg);
	key = extract_key(arg);
	value = extract_value(arg);
	if (append && value)
		handle_append_export(shell, key, value);
	else if (value)
	{
		update_or_add_env(shell, key, value);
		free(value);
	}
	else
		update_or_add_env(shell, key, NULL);
	free(key);
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
