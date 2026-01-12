/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args_conversion.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 14:25:06 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/11 14:25:16 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

/**
 * Compte le nombre d'arguments dans la liste chaînée
 */
static int	count_args(t_arg *args)
{
	int		count;
	t_arg	*current;

	count = 0;
	current = args;
	while (current)
	{
		count++;
		current = current->next;
	}
	return (count);
}

/**
 * Convertit une liste chaînée t_arg en tableau char**
 * Retourne NULL en cas d'erreur d'allocation
 */
char	**args_to_array(t_arg *args)
{
	char	**array;
	t_arg	*current;
	int		i;
	int		total;

	if (!args)
	{
		array = (char **)malloc(sizeof(char *) * 1);
		if (!array)
			return (NULL);
		array[0] = NULL;
		return (array);
	}
	total = count_args(args);
	array = (char **)malloc(sizeof(char *) * (total + 1));
	if (!array)
		return (NULL);
	current = args;
	i = 0;
	while (current)
	{
		array[i] = current->value;
		current = current->next;
		i++;
	}
	array[i] = NULL;
	return (array);
}
