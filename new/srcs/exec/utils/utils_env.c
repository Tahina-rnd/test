/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_env.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tarandri <tarandri@student.42antananarivo. +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 10:49:05 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/11 13:47:31 by tarandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

t_env	*new_env_node(char *str)
{
	t_env	*node;
	int		i;
	char	*key;
	char	*value;

	if (!str)
		return (NULL);
	node = malloc(sizeof(t_env));
	if (!node)
		return (NULL);
	i = 0;
	while (str[i] && str[i] != '=')
		i++;
	key = ft_substr(str, 0, i);
	if (!key)
	{
		free(node);
		return (NULL);
	}
	if (str[i] == '=')
		value = ft_strdup(str + i + 1);
	else
		value = ft_strdup("");
	if (!value)
	{
		free(key);
		free(node);
		return (NULL);
	}
	node->key = key;
	node->value = value;
	node->next = NULL;
	return (node);
}

static t_env	*new_env_node_kv(char *key, char *value)
{
	t_env	*node;

	if (!key || !value)
		return (NULL);
	node = malloc(sizeof(t_env));
	if (!node)
		return (NULL);
	node->key = ft_strdup(key);
	node->value = ft_strdup(value);
	if (!node->key || !node->value)
	{
		if (node->key)
			free(node->key);
		if (node->value)
			free(node->value);
		free(node);
		return (NULL);
	}
	node->next = NULL;
	return (node);
}

static void	add_env_node(t_env **env, t_env *node)
{
	t_env	*curr;

	if (!env || !node)
		return ;
	if (!*env)
	{
		*env = node;
		return ;
	}
	curr = *env;
	while (curr->next)
		curr = curr->next;
	curr->next = node;
}

static void	init_pwd(t_env **env)
{
	char	*cwd;
	t_env	*node;

	if (get_env_value(*env, "PWD"))
		return ;
	cwd = getcwd(NULL, 0);
	if (!cwd)
		cwd = ft_strdup("/");
	node = new_env_node_kv("PWD", cwd);
	free(cwd);
	if (node)
		add_env_node(env, node);
}

static void	init_shlvl(t_env **env)
{
	char	*shlvl_str;
	int		shlvl;
	char	*new_shlvl;
	t_env	*node;
	t_env	*curr;

	shlvl_str = get_env_value(*env, "SHLVL");
	if (shlvl_str)
	{
		shlvl = ft_atoi(shlvl_str);
		if (shlvl < 0)
			shlvl = 0;
		shlvl++;
		new_shlvl = ft_itoa(shlvl);
		curr = *env;
		while (curr)
		{
			if (ft_strcmp(curr->key, "SHLVL") == 0)
			{
				free(curr->value);
				curr->value = new_shlvl;
				return ;
			}
			curr = curr->next;
		}
		free(new_shlvl);
	}
	else
	{
		node = new_env_node_kv("SHLVL", "1");
		if (node)
			add_env_node(env, node);
	}
}

static void	init_underscore(t_env **env)
{
	t_env	*node;

	if (get_env_value(*env, "_"))
		return ;
	node = new_env_node_kv("_", "/usr/bin/env");
	if (node)
		add_env_node(env, node);
}

void	init_env(t_shell *shell, char **envp)
{
	t_env	*curr;
	t_env	*new;
	int		i;

	if (!shell)
		return ;
	shell->env = NULL;
	i = 0;
	while (envp && envp[i])
	{
		new = new_env_node(envp[i]);
		if (!new)
		{
			free_env(shell->env);
			shell->env = NULL;
			ft_putstr_fd("minishell: malloc error\n", 2);
			exit(1);
		}
		if (!shell->env)
			shell->env = new;
		else
		{
			curr = shell->env;
			while (curr->next)
				curr = curr->next;
			curr->next = new;
		}
		i++;
	}
	init_pwd(&shell->env);
	init_shlvl(&shell->env);
	init_underscore(&shell->env);
}

char	**env_to_tab(t_env *env)
{
	char	**env_tab;
	char	*temp;
	int		count;
	int		i;

	if (!env)
		return (NULL);
	count = count_env_nodes(env);
	env_tab = (char **)malloc(sizeof(char *) * (count + 1));
	if (!env_tab)
		return (NULL);
	i = 0;
	while (env)
	{
		temp = ft_strjoin(env->key, "=");
		if (!temp)
		{
			free_tab_partial(env_tab, i);
			return (NULL);
		}
		env_tab[i] = ft_strjoin(temp, env->value);
		free(temp);
		if (!env_tab[i])
		{
			free_tab_partial(env_tab, i);
			return (NULL);
		}
		env = env->next;
		i++;
	}
	env_tab[i] = NULL;
	return (env_tab);
}

void	exp_add_env_node_back(t_env **head, t_env *new_node)
{
	t_env	*current;

	if (!head || !new_node)
		return ;
	if (*head == NULL)
	{
		*head = new_node;
		return ;
	}
	current = *head;
	while (current->next)
		current = current->next;
	current->next = new_node;
}

void	update_env_var(t_env *env, char *key, char *new_value)
{
	t_env	*current;

	if (!env || !key || !new_value)
		return ;
	current = env;
	while (current)
	{
		if (ft_strcmp(current->key, key) == 0)
		{
			free(current->value);
			current->value = ft_strdup(new_value);
			return ;
		}
		current = current->next;
	}
}
