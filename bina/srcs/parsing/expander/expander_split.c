/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_split.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 10:39:50 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/09 12:08:59 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

char	*get_ifs(t_env *env)
{
	char	*ifs;

	ifs = get_env_value(env, "IFS");
	if (!ifs)
		return (" \t\n");
	return (ifs);
}

int	is_ifs_char(char c, char *ifs)
{
	int	i;

	i = 0;
	while (ifs[i])
	{
		if (c == ifs[i])
			return (1);
		i++;
	}
	return (0);
}

int	count_words(char *str, char *ifs)
{
	int	count;
	int	in_word;
	int	i;

	count = 0;
	in_word = 0;
	i = 0;
	while (str[i])
	{
		if (is_ifs_char(str[i], ifs))
			in_word = 0;
		else if (!in_word)
		{
			in_word = 1;
			count++;
		}
		i++;
	}
	return (count);
}

char	*extract_word_split(char *str, int *i, char *ifs)
{
	int		start;
	int		len;
	char	*word;

	while (str[*i] && is_ifs_char(str[*i], ifs))
		(*i)++;
	start = *i;
	while (str[*i] && !is_ifs_char(str[*i], ifs))
		(*i)++;
	len = *i - start;
	if (len == 0)
		return (NULL);
	word = (char *)malloc(len + 1);
	if (!word)
		return (NULL);
	ft_memcpy(word, str + start, len);
	word[len] = '\0';
	return (word);
}

char	**word_split(char *str, t_env *env)
{
	char	**result;
	char	*ifs;
	int		word_count;
	int		i;
	int		j;

	if (!str || !str[0])
		return (NULL);
	ifs = get_ifs(env);
	word_count = count_words(str, ifs);
	if (word_count == 0)
		return (NULL);
	result = (char **)malloc(sizeof(char *) * (word_count + 1));
	if (!result)
		return (NULL);
	i = 0;
	j = 0;
	while (j < word_count)
	{
		result[j] = extract_word_split(str, &i, ifs);
		j++;
	}
	result[j] = NULL;
	return (result);
}
