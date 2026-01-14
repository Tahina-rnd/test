/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_builtin_cd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/14 16:03:05 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	cd_error_no_home(char *old_pwd)
{
	ft_putstr_fd("minishell: cd: HOME not set\n", 2);
	free(old_pwd);
	return (1);
}

static int	cd_error_chdir(char *dir, char *old_pwd)
{
	ft_putstr_fd("minishell: cd: ", 2);
	ft_putstr_fd(dir, 2);
	ft_putstr_fd(": ", 2);
	ft_putstr_fd(strerror(errno), 2);
	ft_putstr_fd("\n", 2);
	free(old_pwd);
	return (1);
}

static char	*get_old_pwd(t_env *env)
{
	char	*old_pwd;

	old_pwd = get_env_value(env, "PWD");
	if (!old_pwd)
		old_pwd = getcwd(NULL, 0);
	else
		old_pwd = ft_strdup(old_pwd);
	if (!old_pwd)
	{
		perror("minishell: cd: getcwd");
		return (NULL);
	}
	return (old_pwd);
}

// int	builtin_cd(char **args, t_env *env)
// {
// 	char	*dir;
// 	char	*old_pwd;

// 	old_pwd = get_old_pwd(env);
// 	if (!old_pwd)
// 		return (1);
// 	if (!args || !args[1])
// 		dir = get_env_value(env, "HOME");
// 	else
// 		dir = args[1];
// 	if (!dir)
// 		return (cd_error_no_home(old_pwd));
// 	if (chdir(dir) != 0)
// 		return (cd_error_chdir(dir, old_pwd));
// 	update_pwd_vars(env, old_pwd, dir);
// 	free(old_pwd);
// 	return (0);
// }


int	builtin_cd(char **args, t_env *env)
{
	char	*dir;
	char	*old_pwd;
	char	*test_cwd;

	old_pwd = get_old_pwd(env);
	if (!old_pwd)
		return (1);

	if (!args || !args[1])
		dir = get_env_value(env, "HOME");
	else
		dir = args[1];

	if (!dir)
		return (cd_error_no_home(old_pwd));

	// ✅ ÉTAPE 1 : Essayer chdir()
	if (chdir(dir) != 0)
		return (cd_error_chdir(dir, old_pwd));

	// ✅ ÉTAPE 2 : Vérifier si getcwd() fonctionne
	test_cwd = getcwd(NULL, 0);
	if (!test_cwd)
	{
		// ⚠️ getcwd() a échoué → Le répertoire parent n'existe plus
		ft_putstr_fd("cd: error retrieving current directory: ", 2);
		ft_putstr_fd("getcwd: cannot access parent directories: ", 2);
		ft_putstr_fd("No such file or directory\n", 2);

		// ✅ Mettre à jour PWD avec le chemin NON NORMALISÉ
		update_pwd_vars_unnormalized(env, old_pwd, dir);
		free(old_pwd);
		return (0);  // ⚠️ Retourne 0 (comme bash)
	}

	// ✅ ÉTAPE 3 : getcwd() a réussi → Tout est normal
	free(test_cwd);
	update_pwd_vars(env, old_pwd, dir);
	free(old_pwd);
	return (0);
}
