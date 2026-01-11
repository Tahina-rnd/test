/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 09:57:56 by miokrako          #+#    #+#             */
/*   Updated: 2025/03/31 10:18:30 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static	size_t	count_size(long nb)
{
	size_t	size;

	size = 0;
	if (nb < 0)
	{
		nb = nb * (-1);
		size = 1;
	}
	if (nb == 0)
		size = 1;
	else
	{
		while (nb)
		{
			nb = nb / 10;
			size++;
		}
	}
	return (size);
}

char	*ft_itoa(int n)
{
	size_t	size;
	long	nb;
	char	*str;
	int		negative;

	size = count_size((long)n);
	str = (char *)malloc(sizeof(char) * (size + 1));
	if (str == NULL)
		return (NULL);
	nb = (long)n;
	negative = 0;
	if (nb < 0)
	{
		nb = nb * (-1);
		str[0] = '-';
		negative = 1;
	}
	str[size] = '\0';
	while (size > (size_t)negative)
	{
		str[size - 1] = nb % 10 + '0';
		nb = nb / 10;
		size--;
	}
	return (str);
}
