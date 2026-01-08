/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   word_split.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 14:24:04 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/07 14:29:38 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static int	count_words(char *str)
{
	int	count;
	int	in_word;
	int	i;

	count = 0;
	in_word = 0;
	i = 0;
	while (str[i])
	{
		if (ft_isspace(str[i]))
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

static char	*extract_next_word(char *str, int *i)
{
	int	start;	
	int	len;

	while (str[*i] && ft_isspace(str[*i]))
		(*i)++;
	if (!str[*i])
		return (NULL);
	start = *i;
	len = 0;
	while (str[*i] && !ft_isspace(str[*i]))
	{
		len++;
		(*i)++;
	}
	return (ft_substr(str, start, len));
}

char	**split_words(char *str)
{
	char	**words;
	int		word_count;
	int		i;
	int		j;

	if (!str || !str[0])
		return (NULL);
	word_count = count_words(str);
	if (word_count == 0)
		return (NULL);
	words = malloc(sizeof(char *) * (word_count + 1));
	if (!words)
		return (NULL);
	i = 0;
	j = 0;
	while (j < word_count)
	{
		words[j] = extract_next_word(str, &i);
		if (!words[j])
		{
			while (--j >= 0)
				free(words[j]);
			free(words);
			return (NULL);
		}
		j++;
	}
	words[j] = NULL;
	return (words);
}
