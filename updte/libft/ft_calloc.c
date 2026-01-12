/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 12:14:11 by miokrako          #+#    #+#             */
/*   Updated: 2025/03/31 10:17:59 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static	void	*ft_memalloc(size_t s)
{
	void	*new;
	size_t	i;

	new = (void *)malloc(s);
	if (!new)
		return (NULL);
	i = 0;
	while (i < s)
	{
		*(unsigned char *)(new + i) = 0;
		i++;
	}
	return (new);
}

void	*ft_calloc(size_t nmemb, size_t size)
{
	if (size != 0 && nmemb > ((size_t) - 1 / size))
		return (NULL);
	return (ft_memalloc(nmemb * size));
}
