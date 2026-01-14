/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 10:39:18 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/13 16:20:52 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

char	*ft_strjoin_free(char *s1, char *s2)
{
	char	*new_str;
	size_t	len1;
	size_t	len2;

	if (!s1)
		s1 = ft_strdup("");
	if (!s2)
		return (s1);
	len1 = ft_strlen(s1);
	len2 = ft_strlen(s2);
	new_str = malloc(sizeof(char) * (len1 + len2 + 1));
	if (!new_str)
		return (NULL);
	ft_memcpy(new_str, s1, len1);
	ft_memcpy(new_str + len1, s2, len2);
	new_str[len1 + len2] = '\0';
	free(s1);
	return (new_str);
}

t_arg	*new_arg_node(void)
{
	t_arg	*node;

	node = malloc(sizeof(t_arg));
	if (!node)
		return (NULL);
	node->value = NULL;
	node->next = NULL;
	node->segments = NULL;
	return (node);
}

void	append_val(t_arg *arg, char *str)
{
	char	*tmp;

	if (!arg->value)
		arg->value = ft_strdup("");
	tmp = arg->value;
	arg->value = ft_strjoin_free(tmp, str);
}
