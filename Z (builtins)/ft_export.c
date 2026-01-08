/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 22:37:54 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/07 17:40:09 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

int	ft_export(t_shell *shell, char **args)
{
	int		i;
	char	*key;
	char	*value;

	if (!args[1])
	{
		print_export(shell);
		return (0);
	}
	i = 1;
	while (args[i])
	{
		if (!is_valid_identifier(args[i]))
		{
			printf("export: `%s': not a valid identifier\n", args[i]);
			return (1);
		}
		key = extract_key(args[i]);
		value = extract_value(args[i]);
		if (value)
		{
			update_or_add_env(shell, key, value);
			free(value);
		}
		free(key);
		i++;
	}
	return (0);
}
