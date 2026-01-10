/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_process.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 10:39:50 by tarandri          #+#    #+#             */
/*   Updated: 2026/01/10 22:31:30 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/parsing.h"

static int	get_var_len(char *str)
{
	int	i;

	i = 0;
	if (str[i] == '?')
		return (1);
	while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
		i++;
	return (i);
}

/*
** Helper pour récupérer la valeur (gère le $? ou appelle get_env_value)
*/
static char	*fetch_value(char *key, t_shell *shell)
{
	if (ft_strncmp(key, "?", 2) == 0)
		return (ft_itoa(shell->last_exit_status));
	return (get_env_value(shell->env, key));
}

static char	*append_var(char *res, char *str, int *i, t_shell *shell)
{
	int		len;
	char	*key;
	char	*val;

	(*i)++;
	len = get_var_len(str + *i);
	if (len == 0)
		return (ft_strjoin_free(res, "$"));
	key = ft_substr(str, *i, len);
	if (!key)
		return (res);
	// Changement ici : on passe par fetch_value ou on gère le ?
	val = fetch_value(key, shell);
	free(key);
	res = ft_strjoin_free(res, val);
	free(val);
	*i += len;
	return (res);
}

static char	*append_char(char *res, char c)
{
	char	tmp[2];

	tmp[0] = c;
	tmp[1] = '\0';
	return (ft_strjoin_free(res, tmp));
}

char	*expand_text(char *str, t_shell *shell)
{
	char	*res;
	int		i;

	res = ft_strdup("");
	if (!res)
		return (NULL);
	i = 0;
	while (str[i])
	{
		if (str[i] == '$')
			res = append_var(res, str, &i, shell);
		else
		{
			res = append_char(res, str[i]);
			i++;
		}
	}
	return (res);
}
