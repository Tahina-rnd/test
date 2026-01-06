/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 09:56:04 by miokrako          #+#    #+#             */
/*   Updated: 2025/03/31 10:19:15 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static char	*f_begin(char const *s1, char const *set)
{
	size_t	i;
	size_t	j;
	int		inset;

	inset = 0;
	i = 0;
	j = 0;
	while (s1[i])
	{
		inset = 0;
		j = 0;
		while (set[j])
		{
			if (s1[i] == set[j])
				inset = 1;
			j++;
		}
		if (!inset)
			break ;
		i++;
	}
	return ((char *)s1 + i);
}

static char	*f_end(char const	*s1, char const	*set, char const	*begin)
{
	size_t	i;
	size_t	j;
	int		inset;

	inset = 0;
	j = 0;
	i = ft_strlen(s1) - 1;
	while (s1 + 1 >= begin)
	{
		inset = 0;
		j = 0;
		while (set[j])
		{
			if (s1[i] == set[j])
				inset = 1;
			j++;
		}
		if (!inset)
			break ;
		i--;
	}
	if (s1 + i < begin)
		return ((char *)begin);
	return ((char *)s1 + i);
}

static char	*fill_string(char const	*begin, char const *end)
{
	char	*new;
	size_t	i;

	new = malloc(sizeof(char) * (end - begin + 2));
	if (!new)
		return (NULL);
	i = 0;
	while (begin + i <= end)
	{
		new[i] = begin[i];
		i++;
	}
	new[i] = '\0';
	return (new);
}

char	*ft_strtrim(char const *s1, char const *set)
{
	char	*start;
	char	*end;
	char	*new;

	start = f_begin(s1, set);
	end = f_end(s1, set, s1);
	if (!s1[0] || end < start)
	{
		new = malloc(sizeof(char) * 1);
		if (!new)
			return (NULL);
		new[0] = '\0';
	}
	else
		new = fill_string(start, end);
	if (!new)
		return (NULL);
	return (new);
}
