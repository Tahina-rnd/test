/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcpy.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 16:19:11 by miokrako          #+#    #+#             */
/*   Updated: 2025/03/31 10:19:07 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	ft_strlcpy(char *dst, const char *src, size_t dstsize)
{
	int				s;
	unsigned int	i;

	i = 0;
	s = 0;
	while (src[s] != '\0')
		s++;
	if (dstsize == 0)
		return (s);
	while (src[i] != '\0' && i < dstsize -1)
	{
		dst[i] = src[i];
		i++;
	}
	if (dstsize > 0)
		dst[i] = '\0';
	return (s);
}
