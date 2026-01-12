/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strncmp.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/04 11:43:18 by miokrako          #+#    #+#             */
/*   Updated: 2025/03/31 10:19:11 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t			i;
	unsigned int	num;
	unsigned char	*src1;
	unsigned char	*src2;

	src1 = (unsigned char *)s1;
	src2 = (unsigned char *)s2;
	num = (unsigned int)n;
	i = 0;
	while (((src1[i] == src2[i]) && (src1[i] || src2[i])) && ((num - 1) > 0))
	{
		i++;
		num--;
	}
	if (num == 0)
		return (0);
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}
