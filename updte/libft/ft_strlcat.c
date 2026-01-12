/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/04 15:18:02 by miokrako          #+#    #+#             */
/*   Updated: 2025/03/31 10:19:05 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	ft_strlcat(char *dst, const char *src, size_t size)
{
	size_t	index;
	size_t	i;
	size_t	dlen;

	dlen = ft_strlen(dst);
	index = 0;
	while (dst[index])
		index++;
	i = 0;
	while (src[i] && (i + index + 1) < (size))
	{
		dst[index + i] = src[i];
		i++;
	}
	if (i < size)
		dst[index + i] = '\0';
	if (size <= dlen)
		return (ft_strlen(src) + size);
	else
		return (ft_strlen(src) + dlen);
}
