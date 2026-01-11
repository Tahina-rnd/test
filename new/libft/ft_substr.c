/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_substr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 15:42:29 by miokrako          #+#    #+#             */
/*   Updated: 2025/03/31 10:19:16 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	unsigned int	i;
	char			*dest;

	if (start >= ((unsigned int)ft_strlen(s)))
		len = 0;
	else if (len > ((unsigned int)ft_strlen(s + start)))
		len = ft_strlen(s + start);
	dest = malloc((len + 1) * sizeof(char));
	if (dest == NULL)
		return (NULL);
	i = 0;
	while (i + start < (unsigned int)ft_strlen(s) && i < len)
	{
		dest[i] = s[i + start];
		i++;
	}
	dest[i] = '\0';
	return (dest);
}
