/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc_expand.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 16:43:56 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 23:43:05 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static void	expand_var_value(char *line, int *i, int fd, t_shell *shell)
{
	char	*var_name;
	char	*var_value;
	int		start;

	start = *i;
	while (line[*i] && (ft_isalnum(line[*i]) || line[*i] == '_'))
		(*i)++;
	var_name = ft_substr(line, start, *i - start);
	var_value = get_env_value(shell->env, var_name);
	if (var_value)
		write(fd, var_value, ft_strlen(var_value));
	free(var_name);
}

static void	expand_dollar(char *line, int *i, int fd, t_shell *shell)
{
	char	*var_value;

	(*i)++;
	if (line[*i] == '?')
	{
		var_value = ft_itoa(shell->last_exit_status);
		write(fd, var_value, ft_strlen(var_value));
		free(var_value);
		(*i)++;
	}
	else if (ft_isalpha(line[*i]) || line[*i] == '_')
		expand_var_value(line, i, fd, shell);
	else if (ft_isdigit(line[*i]))
		(*i)++;
	else
		write(fd, "$", 1);
}

void	expand_heredoc_line(char *line, int fd, t_shell *shell)
{
	int	i;

	i = 0;
	while (line[i])
	{
		if (line[i] == '$' && line[i + 1])
			expand_dollar(line, &i, fd, shell);
		else
		{
			write(fd, &line[i], 1);
			i++;
		}
	}
	write(fd, "\n", 1);
}
