/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export_utils2.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 21:11:16 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/18 17:50:18 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

int	is_append_mode(const char *arg)
{
	int	i;

	i = 0;
	while (arg[i] && arg[i] != '=')
		i++;
	if (i > 0 && arg[i - 1] == '+' && arg[i] == '=')
		return (1);
	return (0);
}

int	handle_append_export(t_shell *shell, char *key, char *value)
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
