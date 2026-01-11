/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memcmp.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/15 10:49:29 by miokrako          #+#    #+#             */
/*   Updated: 2025/03/31 10:18:32 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_memcmp(const void *ptr1, const void *ptr2, size_t num)
{
	unsigned char	*p1;
	unsigned char	*p2;

	p1 = (unsigned char *)ptr1;
	p2 = (unsigned char *)ptr2;
	while (num > 0)
	{
		if (*p1 != *p2)
			return (*p1 - *p2);
		num--;
		p1++;
		p2++;
	}
	return (0);
}
