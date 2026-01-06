/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 11:29:26 by miokrako          #+#    #+#             */
/*   Updated: 2025/03/31 10:19:12 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strnstr(const char *str, const char *find, size_t len)
{
	size_t	i;
	int		j;
	char	*ptr;

	i = 0;
	j = 0;
	if (find[i] == '\0')
		return ((char *)str);
	while (str[i] != '\0' && i < len)
	{
		if (str[i] == find[j])
		{
			ptr = (char *) str + i;
			while (str[i + j] == find[j] && i + j < len)
			{
				if (find[j + 1] == '\0')
					return (ptr);
				j++;
			}
			j = 0;
			ptr = 0;
		}
		i++;
	}
	return (NULL);
}
