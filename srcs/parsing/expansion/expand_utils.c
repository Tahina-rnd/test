/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 10:01:54 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/07 06:14:59 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

char	*str_append_char(char *str, char c)
{
	char	*new_str;
	int		len;

	if (!str)
		str = ft_strdup("");
	len = ft_strlen(str);
	new_str = malloc(sizeof(char) * (len + 2));
	if (!new_str)
	{
		free(str);
		return (NULL);
	}
	ft_strlcpy(new_str, str, len + 1);
	new_str[len] = c;
	new_str[len + 1] = '\0';
	free(str);
	return (new_str);
}

char	*str_append_str(char *src, char *dest)
{
	char	*new_str;
	int		len_src;
	int		len_dest;

	if (!src)
		src = ft_strdup("");
	if (!dest)
		dest = "";
	len_src = ft_strlen(src);
	len_dest = ft_strlen(dest);
	new_str = malloc(sizeof(char) * (len_src + len_dest + 1));
	if (!new_str)
	{
		free(src);
		return (NULL);
	}
	ft_strlcpy(new_str, src, len_src + 1);
	ft_strlcpy(new_str + len_src, dest, len_dest + 1);
	free(src);
	return (new_str);
}

void	clean_empty_tokens(t_token **tokens)
{
	t_token	*current;
	t_token	*prev;
	t_token	*to_delete;

	current = *tokens;
	prev = NULL;	
	while (current)
	{
		if (current->type == WORD
			&& (!current->value || current->value[0] == '\0')
			&& current->was_quoted == 0)
		{
			to_delete = current;
			if (prev)
				prev->next = current->next;
			else
				*tokens = current->next;
			current = current->next;
			if (to_delete->value)
				free(to_delete->value);
			free(to_delete);
		}
		else
		{
			prev = current;
			current = current->next;
		}
	}
}

