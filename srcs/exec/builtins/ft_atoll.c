/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoll.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 23:16:22 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	skip_whitespace(const char *str)
{
	int	i;

	i = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	return (i);
}

static int	get_sign(const char *str, int *i)
{
	int	sign;

	sign = 1;
	if (str[*i] == '-' || str[*i] == '+')
	{
		if (str[*i] == '-')
			sign = -1;
		(*i)++;
	}
	return (sign);
}

static int	check_overflow_pos(unsigned long long res, char digit)
{
	if (res > LLONG_MAX / 10)
		return (-1);
	if (res == LLONG_MAX / 10 && (digit - '0') > LLONG_MAX % 10)
		return (-1);
	return (0);
}

static int	check_overflow_neg(unsigned long long res, char digit)
{
	if (res > ((unsigned long long)LLONG_MAX + 1) / 10)
		return (-1);
	if (res == ((unsigned long long)LLONG_MAX + 1) / 10 && (digit - '0') > 8)
		return (-1);
	return (0);
}

int	ft_atoll_safe(const char *str, long long *result)
{
	unsigned long long	res;
	int					sign;
	int					i;

	res = 0;
	i = skip_whitespace(str);
	sign = get_sign(str, &i);
	while (str[i] >= '0' && str[i] <= '9')
	{
		if (sign == 1 && check_overflow_pos(res, str[i]) == -1)
			return (-1);
		if (sign == -1 && check_overflow_neg(res, str[i]) == -1)
			return (-1);
		res = res * 10 + (str[i] - '0');
		i++;
	}
	*result = res * sign;
	return (0);
}
