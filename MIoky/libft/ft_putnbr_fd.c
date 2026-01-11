/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr_fd.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 10:06:31 by miokrako          #+#    #+#             */
/*   Updated: 2025/03/31 10:18:52 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	ft_printnb(long nb, int fd)
{
	if (nb / 10)
	{
		ft_printnb(nb / 10, fd);
		ft_printnb(nb % 10, fd);
	}
	else
		ft_putchar_fd(nb + '0', fd);
}

void	ft_putnbr_fd(int n, int fd)
{
	long	nb;

	nb = n;
	if (nb < 0)
	{
		write(fd, "-", 1);
		nb = -nb;
	}
	ft_printnb(nb, fd);
}
