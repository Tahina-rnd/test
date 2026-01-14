/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/27 15:18:09 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/14 20:32:21 by tarandri         ###   ########.fr       */
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

static void	print_export_line(char *str)
{
	int	i;

	ft_putstr_fd("declare -x ", 1);
	i = 0;
	while (str[i] && str[i] != '=')
	{
		ft_putchar_fd(str[i], 1);
		i++;
	}
	if (str[i] == '=')
	{
		ft_putstr_fd("=\"", 1);
		ft_putstr_fd(str + i + 1, 1);
		ft_putstr_fd("\"", 1);
	}
	ft_putchar_fd('\n', 1);
}

void	export_onl(t_shell *shell)
{
	char	**envp;
	int		i;

	envp = env_to_array(shell->env);
	if (!envp)
		return ;
	sort_strings(envp);
	i = 0;
	while (envp[i])
	{
		print_export_line(envp[i]);
		i++;
	}
	free_tab(envp);
}

