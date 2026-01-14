/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/27 15:18:09 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/14 17:04:37 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	strlen_tab(char **tab)
{
	int	i;

	i = 0;
	if (!tab)
		return (0);
	while (tab[i] != NULL)
		i++;
	return (i);
}

static void	swap_pointers(char **a, char **b)
{
	char	*tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static void	sort_strings(char **arr)
{
	int	i;
	int	j;
	int	size;

	size = strlen_tab(arr);
	i = 0;
	while (i < size - 1)
	{
		j = 0;
		while (j < size - 1 - i)
		{
			if (ft_strcmp(arr[j], arr[j + 1]) > 0)
				swap_pointers(&arr[j], &arr[j + 1]);
			j++;
		}
		i++;
	}
}

void	export_onl(t_shell *shell)
{
	t_env	*current;
	int		count;
	char	**envp;

	envp = env_to_array(shell->env);
	count = strlen_tab(envp);
	sort_strings(envp);
	current = dup_env(envp);
	while (current)
	{
		if (current->value && *current->value)
			printf("declare -x %s=\"%s\"\n", current->key, current->value);
		else
			printf("declare -x %s\n", current->key);
		current = current->next;
	}
	free(envp);
}
