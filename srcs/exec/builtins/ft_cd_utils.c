/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_cd_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 23:15:13 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

char	*build_absolute_path(char *current_pwd, char *dir)
{
	char	*temp;
	char	*result;

	if (!current_pwd || !dir)
		return (NULL);
	if (dir[0] == '\0' || (dir[0] == '.' && dir[1] == '\0'))
		return (ft_strdup(current_pwd));
	temp = ft_strjoin(current_pwd, "/");
	if (!temp)
		return (NULL);
	result = ft_strjoin(temp, dir);
	free(temp);
	return (result);
}

static char	*build_normalized_path(char **stack, int j)
{
	char	*result;
	char	*temp;
	int		i;

	if (j == 0)
		return (ft_strdup("/"));
	result = ft_strdup("");
	i = 0;
	while (i < j)
	{
		temp = ft_strjoin(result, "/");
		free(result);
		result = ft_strjoin(temp, stack[i]);
		free(temp);
		i++;
	}
	return (result);
}

static void	process_path_segment(char **parts, char **stack, int *j, int i)
{
	if (ft_strcmp(parts[i], "..") == 0)
	{
		if (*j > 0)
			(*j)--;
	}
	else if (ft_strcmp(parts[i], ".") != 0 && parts[i][0] != '\0')
	{
		stack[*j] = parts[i];
		(*j)++;
	}
}

char	*normalize_path(char *path)
{
	char	**parts;
	char	**stack;
	int		i;
	int		j;
	char	*result;

	if (!path || path[0] != '/')
		return (ft_strdup(path));
	parts = ft_split(path, '/');
	if (!parts)
		return (ft_strdup(path));
	i = 0;
	while (parts[i])
		i++;
	stack = malloc(sizeof(char *) * (i + 1));
	if (!stack)
		return (free_tab(parts), ft_strdup(path));
	i = 0;
	j = 0;
	while (parts[i])
		process_path_segment(parts, stack, &j, i++);
	result = build_normalized_path(stack, j);
	free(stack);
	free_tab(parts);
	return (result);
}
