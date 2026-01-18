/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   try_paths.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/16 20:27:08 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/16 21:19:17 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static char	*build_full_path(char *path, char *cmd)
{
	char	*part_path;
	char	*full_path;

	part_path = ft_strjoin(path, "/");
	if (!part_path)
		return (NULL);
	full_path = ft_strjoin(part_path, cmd);
	free(part_path);
	return (full_path);
}

static char	*check_path_access(char *full_path, char **permission_denied)
{
	if (access(full_path, F_OK) == 0)
	{
		if (access(full_path, X_OK) == 0)
		{
			if (*permission_denied)
				free(*permission_denied);
			return (full_path);
		}
		if (!*permission_denied)
			*permission_denied = ft_strdup(full_path);
	}
	return (NULL);
}

char	*try_paths(char **paths, char *cmd)
{
	char	*full_path;
	char	*permission_denied;
	char	*result;
	int		i;

	i = 0;
	permission_denied = NULL;
	while (paths[i])
	{
		full_path = build_full_path(paths[i], cmd);
		if (!full_path)
			return (NULL);
		result = check_path_access(full_path, &permission_denied);
		if (result)
			return (result);
		free(full_path);
		i++;
	}
	return (permission_denied);
}
