/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_to_tab.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 10:49:05 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/14 21:58:46 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static char	*build_env_string(t_env *env)
{
	char	*temp;
	char	*result;

	temp = ft_strjoin(env->key, "=");
	if (!temp)
		return (NULL);
	if (env->value)
		result = ft_strjoin(temp, env->value);
	else
		result = ft_strjoin(temp, "");
	free(temp);
	return (result);
}

static char	**allocate_env_tab(int count)
{
	char	**env_tab;

	env_tab = (char **)malloc(sizeof(char *) * (count + 1));
	return (env_tab);
}

static int	fill_env_tab(char **env_tab, t_env *env)
{
	int	i;

	i = 0;
	while (env)
	{
		env_tab[i] = build_env_string(env);
		if (!env_tab[i])
		{
			free_tab_partial(env_tab, i);
			return (-1);
		}
		env = env->next;
		i++;
	}
	env_tab[i] = NULL;
	return (0);
}

// char	**env_to_tab(t_env *env)
// {
// 	char	**env_tab;
// 	int		count;

// 	if (!env)
// 		return (NULL);
// 	count = count_env_vars(env);
// 	env_tab = allocate_env_tab(count);
// 	if (!env_tab)
// 		return (NULL);
// 	if (fill_env_tab(env_tab, env) == -1)
// 		return (NULL);
// 	return (env_tab);
// }
char **env_to_tab(t_env *env)
{
    char **env_tab;
    int count;

    // 🆕 Si env est NULL ou vide, retourner un tableau vide
	count = count_env_vars(env);
    if (!env || count == 0)
    {
        env_tab = malloc(sizeof(char *) * 1);
        if (!env_tab)
            return (NULL);
        env_tab[0] = NULL;
        return (env_tab);  // ✅ Tableau vide valide
    }
    env_tab = allocate_env_tab(count);
    if (!env_tab)
        return (NULL);

    if (fill_env_tab(env_tab, env) == -1)
        return (NULL);

    return (env_tab);
}
