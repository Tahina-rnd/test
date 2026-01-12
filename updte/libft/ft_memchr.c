/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 10:24:58 by miokrako          #+#    #+#             */
/*   Updated: 2025/03/31 10:18:31 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memchr(const void *ptr, int value, size_t num)
{
	unsigned char	*p;
	unsigned int	i;

	i = 0;
	p = (unsigned char *)ptr;
	while (i < num)
	{
		if (p[i] == (unsigned char)value)
			return ((void *)p + i);
		i++;
	}
	return (NULL);
}
