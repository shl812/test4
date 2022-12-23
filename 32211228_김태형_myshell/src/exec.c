/**
 * exec.c	: excute function in myshell program
 * @author	: Taehyeong Kim
 * @email	: taehyeok02@gmail.com
 * @version	: 1.0
 * @date	: 2022.12.05
*/

#include "myshell.h"

void	freearr(char **arr)
{
	int	idx = 0;

	if (!arr)
		return ;
	while (arr[idx])
	{
		free(arr[idx]);
		idx++;
	}
	free(arr);
}
char	**make_env(t_data *data)
{
	int		len;
	t_list	*temp;
	char	**env;
	char	*tmp;
	int		idx;

	len = 0;
	temp = data->env;
	while (temp)
	{
		len++;
		temp = temp->next;
	}
	env = (char **)malloc((len + 1) * sizeof(char *));
	temp = data->env;
	idx = -1;
	while (temp && temp->key)
	{
		env[++idx] = strdup(temp->key);
		tmp = env[idx];
		env[idx] = strjoin2(env[idx], "=");
		free(tmp);
		tmp = env[idx];
		env[idx] = strjoin2(env[idx], temp->value);
		free(tmp);
		tmp = NULL;
		temp = temp->next;
	}
	env[len] = 0;
	return (env);
}

static char	*check_slash(char *path, char *cmd)
{
	char	*temp;
	char	*ret;
	int		len;

	if (cmd[0] == '/')
		return (strdup(cmd));
	len = strlen(path);
	if (path[len - 1] == '/')
		ret = strjoin2(path, cmd);
	else
	{
		temp = strjoin2("/", cmd);
		ret = strjoin2(path, temp);
		free(temp);
	}
	return (ret);
}

static int	is_inslash(char *s)
{
	while (*s)
	{
		if (*s == '/')
			return (1);
		s++;
	}
	return (0);
}

char	*check_path(t_list *env, char *cmd)
{
	char	*str;
	char	**path;
	int		idx = 0;

	path = NULL;
	if (is_inslash(cmd) && !access(cmd, F_OK))
	{
		if (!access(cmd, X_OK))
			return (strdup(cmd));
		return (NULL);
	}
	while (env->key)
	{
		if (!strcmp(env->key, "PATH"))
		{
			path = split(env->value, ':');
			break ;
		}
		env = env->next;
	}
	while (path && path[idx])
	{
		str = check_slash(path[idx], cmd);
		if (!access(str, F_OK))
		{
			freearr(path);
			return (str);
		}
		else
			free(str);
		idx++;
	}
	freearr(path);
	return (NULL);
}

int	is_background(char **cmds)
{
	int	idx;

	idx = 0;
	while (cmds[idx])
		idx++;
	if (!strcmp(cmds[idx - 1], "&"))
	{
		cmds[idx - 1] = 0;
		return (1);
	}
	return (0);
}

int	is_builtin(char **cmds, t_data *data)
{
	if (!strcmp(cmds[0], "echo"))
		data->exit_code = echo(cmds);
	else if (!strcmp(cmds[0], "cd"))
		data->exit_code = cd(cmds, data);
	else if (!strcmp(cmds[0], "pwd"))
		data->exit_code = pwd(cmds);
	else if (!strcmp(cmds[0], "export"))
		data->exit_code = export(cmds, data);
	else if (!strcmp(cmds[0], "unset"))
		data->exit_code = unset(cmds, data);
	else if (!strcmp(cmds[0], "env"))
		data->exit_code = fenv(cmds, data->env);
	else if (!strcmp(cmds[0], "exit"))
		data->exit_code = fexit(cmds);
	else
		return (0);
	return (1);
}

void	excute(char *cmd, t_data *data)
{
	char	**env;
	char	**cmds;
	char	*path;
	int		background;
	pid_t	pid;

	env = make_env(data);
	cmds = split(cmd, ' ');
	path = check_path(data->env, cmds[0]);
	background = is_background(cmds);
	if (data->infile_fd != -1)
		dup2(data->infile_fd, 0);
	if (data->outfile_fd != -1)
		dup2(data->outfile_fd, 1);
	if (!background && is_builtin(cmds, data))
		;
	else if (background)
	{
		pid = fork();
		if (pid == 0)
		{
			pid_t	child;

			child = fork();
			if (child == 0)
			{
				if (is_builtin(cmds, data))
					exit(0);
				else
					execve(path, cmds, env);
				write(2, cmds[0], strlen(cmds[0]));
				write(2, ": command not found\n", strlen(": command not found\n"));
				exit(127);
			}
			dup2(data->stdout_fd, 1);
			printf("Background process [pid %d] : %s\n", pid, cmds[0]);
			waitpid(child, 0, 0);
			printf("Done [pid %d] : %s\n", pid, cmds[0]);
		}
	}
	else
	{
		pid = fork();
		if (pid == 0)
		{
			if (is_builtin(cmds, data))
				exit(0);
			else
				execve(path, cmds, env);
			write(2, cmds[0], strlen(cmds[0]));
			write(2, ": command not found\n", strlen(": command not found\n"));
			exit(127);
		}
		dup2(data->stdin_fd, 0);
		dup2(data->stdout_fd, 1);
		close(data->infile_fd);
		close(data->outfile_fd);
		data->infile_fd = -1;
		data->outfile_fd = -1;
		waitpid(pid, &data->exit_code, 0);
	}
	freearr(env);
	freearr(cmds);
	free(path);
	unlink(".here_doc");
}
