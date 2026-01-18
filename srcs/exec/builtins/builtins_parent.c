/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins_parent.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 23:05:52 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/16 16:04:34 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	setup_fd_backup(int *original_stdin, int *original_stdout)
{
	*original_stdin = dup(STDIN_FILENO);
	*original_stdout = dup(STDOUT_FILENO);
	if (*original_stdin == -1 || *original_stdout == -1)
	{
		if (*original_stdin != -1)
			close(*original_stdin);
		if (*original_stdout != -1)
			close(*original_stdout);
		perror("minishell: dup");
		return (1);
	}
	return (0);
}

static void	restore_fd(int original_stdin, int original_stdout)
{
	dup2(original_stdin, STDIN_FILENO);
	dup2(original_stdout, STDOUT_FILENO);
	close(original_stdin);
	close(original_stdout);
}

void	update_pwd_after_cd(t_env *env, char *old_pwd)
{
	char	*cwd;

	cwd = getcwd(NULL, 0);
	if (!cwd)
		ft_putstr_fd("cd: error retrieving current directory: getcwd: \
cannot access parent directories: No such file or directory\n", 2);
	else
	{
		update_env_var(env, "PWD", cwd);
		free(cwd);
	}
	if (old_pwd)
	{
		update_env_var(env, "OLDPWD", old_pwd);
		if (old_pwd != get_env_value(env, "PWD"))
			free(old_pwd);
	}
}

int	handle_chdir_failure(char *dir, char *old_pwd, t_env *env)
{
	ft_putstr_fd("minishell: cd: ", 2);
	perror(dir);
	if (old_pwd && old_pwd != get_env_value(env, "PWD"))
		free(old_pwd);
	return (1);
}

void	exec_builtin_parent(t_command *cmd, t_shell *shell)
{
	int	original_stdin;
	int	original_stdout;
	int	ret;
	int	is_exit_builtin;

	is_exit_builtin = (cmd->args && cmd->args->value
			&& ft_strcmp(cmd->args->value, "exit") == 0);
	if (!is_exit_builtin)
	{
		if (setup_fd_backup(&original_stdin, &original_stdout))
		{
			shell->last_exit_status = 1;
			return ;
		}
	}
	ret = handle_redirections(cmd);
	if (ret == 0)
		shell->last_exit_status = execute_builtin(cmd, shell);
	else
		shell->last_exit_status = 1;
	if (!is_exit_builtin)
		restore_fd(original_stdin, original_stdout);
}
