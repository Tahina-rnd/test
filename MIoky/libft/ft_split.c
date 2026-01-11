/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 09:57:22 by miokrako          #+#    #+#             */
/*   Updated: 2025/03/31 10:18:55 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stddef.h>

static size_t	count(char const	*s, char c)
{
	size_t	w;
	size_t	i;

	w = 0;
	i = 0;
	while (s[i])
	{
		if (s[i] != c && (s[i +1] == c || s[i + 1] == '\0'))
			w++;
		i++;
	}
	return (w);
}

static void	to_tab(char	*n, char const *s, char c)
{
	size_t	j;

	j = 0;
	while (s[j] && s[j] != c)
	{
		n[j] = s[j];
		j++;
	}
	n[j] = '\0';
}

static void	set_mem(char	**tab, char const	*s, char c)
{
	size_t	count;
	size_t	index;
	size_t	i;

	index = 0;
	i = 0;
	while (s[index])
	{
		count = 0;
		while (s[index + count] && s[index + count] != c)
			count++;
		if (count > 0)
		{
			tab[i] = malloc(sizeof(char) * (count + 1));
			if (!tab[i])
				return ;
			to_tab(tab[i], (s + index), c);
			i++;
			index = index + count;
		}
		else
			index++;
	}
	tab[i] = 0;
}

char	**ft_split(char const *s, char c)
{
	size_t	words;
	char	**tab;

	words = count(s, c);
	tab = malloc(sizeof(char *) * (words + 1));
	if (!tab)
		return (NULL);
	set_mem(tab, s, c);
	return (tab);
}
