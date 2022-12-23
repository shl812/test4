/**
 * builtin.c: builtin function in myshell program
 * @author	: Taehyeong Kim
 * @email	: taehyeok02@gmail.com
 * @version	: 1.0
 * @date	: 2022.12.05
*/

#include "myshell.h"

int	cd(char **argvs, t_data *data)
{
	char	*path;

	if (argvs[1] == 0)
	{
		path = get_env("HOME", data->env);
		if (path == NULL)
		{
			errno = 1;
			write(2, "cd: HOME not set\n", strlen("cd: HOME not set\n"));
			return (1);
		}
	}
	else
		path = strdup(argvs[1]);
	if (chdir(path) == -1)
	{
		free(path);
		perror(argvs[0]);
		return (errno);
	}
	free(path);
	return (0);
}

static void	write_str(char **argvs, int mode)
{
	int	idx;

	if (mode == 1)
		idx = 1;
	else
		idx = 2;
	while (argvs[idx])
	{
		if (idx != mode)
			write(1, " ", 1);
		decode_text(argvs[idx]);
		write(1, argvs[idx], strlen(argvs[idx]));
		idx++;
	}
	if (mode == 1)
		write(1, "\n", 1);
}

int	echo(char **argvs)
{
	if (!argvs[1])
		write(1, "\n", 1);
	else if (strcmp(argvs[1], "-n") == 0)
		write_str(argvs, 2);
	else
		write_str(argvs, 1);
	return (0);
}

void	lstadd_front(t_list **lst, t_list *new)
{
	if (new == NULL || !lst)
		return ;
	new->next = *lst;
	*lst = new;
}

char	*find_key(char *s)
{
	int	idx;

	idx = 0;
	while (s[idx] != '=')
		idx++;

	return (str_cut_front(s, idx));
	s[idx] = '\0';
	return (strdup(s));
}

char	*find_val(char *s)
{
	while (*s != '=')
		s++;
	return (strdup(++s));
}

t_list	*init_env(char **envp)
{
	int		idx;
	char	*key;
	char	*val;
	t_list	*node;
	t_list	*head;

	idx = 0;
	head = NULL;
	while (envp[idx])
	{
		key = find_key(envp[idx]);
		val = find_val(envp[idx]);
		node = lstnew(key, val);
		lstadd_front(&head, node);
		head = node;
		idx++;
	}
	return (head);
}

int	fenv(char **argv, t_list *env)
{
	if (argv[1] != 0)
	{
		write(2, "No such file or directory\n", \
		strlen("No such file or directory\n"));
		return (127);
	}
	while (env && env->key)
	{
		if (env->value)
			printf("%s=%s\n", env->key, env->value);
		env = env->next;
	}
	return (0);
}

void	no_input_export(t_list *env)
{
	while (env && env->key)
	{
		if (env->value)
			printf("declare -x %s=\"%s\"\n", env->key, env->value);
		else
			printf("declare -x %s\n", env->key);
		env = env->next;
	}
}

void	valid_export(char *str, t_data *data)
{
	int		idx;
	t_list	*temp;
	char	*temp_key;
	char	*temp_val;

	idx = 0;
	while (str[idx] && str[idx] != '=')
		idx++;
	if (!str[idx])
	{
		temp_key = str_cut_front(str, idx + 1);
		temp_val = NULL;
	}
	else
	{
		temp_key = str_cut_front(str, idx + 1);
		temp_val = str_cut_back(str, idx);
	}
	temp = data->env;
	while (temp && temp->key)
	{
		if (!strcmp(temp_key, temp->key))
		{
			if (!temp_val && (strlen(temp->value) == 0))
				temp->value = strdup("");
			else if (!temp_val)
				;
			else
			{
				free(temp->value);
				temp->value = temp_val;
			}
			free(temp_key);
			return ;
		}	
		temp = temp->next;
	}
	lstadd_front(&data->env, lstnew(temp_key, temp_val));
}

void	invalid_export(char *str)
{
	write(2, "export: `", strlen("export: `"));
	write(2, str, strlen(str));
	write(2, "': not a valid identifier\n", \
	strlen("': not a valid identifier\n"));
}

int	export(char **argvs, t_data *data)
{
	int		idx;
	int		ret;

	idx = 1;
	ret = 0;
	if (!argvs[1])
		no_input_export(data->env);
	else
	{
		while (argvs[idx])
		{
			decode_text(argvs[idx]);
			if (is_valid(argvs[idx][0]))
				valid_export(argvs[idx], data);
			else
			{
				invalid_export(argvs[idx]);
				ret = 1;
			}
			idx++;
		}
	}
	return (ret);
}

void	lstdel_value(t_list *lst, char *val)
{
	t_list	*temp;

	while (lst->next->key)
	{
		if (!strcmp(lst->next->key, val))
		{
			temp = lst->next;
			lst->next = lst->next->next;
			free(temp->key);
			free(temp->value);
			free(temp);
			return ;
		}
		else
			lst = lst->next;
	}
}

int	unset(char **argv, t_data *data)
{
	int	idx;
	int	ret;
	t_list	*temp;

	idx = 1;
	ret = 0;
	if(!argv[idx])
		return (ret);
	while (argv[idx])
	{
		decode_text(argv[idx]);
		if (is_valid(argv[idx][0]))
		{
			// 맨 처음 환경변수 삭제
			if (!strcmp(data->env->key, argv[idx]))
			{
				temp = data->env;
				data->env = data->env->next;
				free(temp->key);
				free(temp->value);
				free(temp);
			}
			else
				lstdel_value(data->env, argv[idx]);
		}
		else
		{
			write(2, "unset: `", strlen("unset: `"));
			write(2, argv[idx], strlen(argv[idx]));
			write(2, "': not a valid identifier", \
			strlen("': not a valid identifier"));
			ret = 1;
		}
		idx++;
	}
	return (ret);
}

int	pwd(char **argvs)
{
	(void)argvs;
	char	*path;

	path = getcwd(NULL, 0);
	if (path == NULL)
		return (1);
	printf("%s\n", path);
	free(path);
	return (0);
}

int	is_numeric(char *s)
{
	int	idx;

	idx = 0;
	while (s[idx])
	{
		if (s[idx] >= '0' && s[idx] <= '9')
			;
		else
			return (0);
		idx++;
	}
	return (1);
}

int	fexit(char **argvs)
{
	printf("exit\n");
	if (!argvs[1])
		exit(0);
	if (!is_numeric(argvs[1]))
	{
		write(2, "exit: numeric argument required\n", \
		strlen("exit: numeric argument required\n"));
		return (1);
	}

	if (argvs[2])
	{
		write(2, "exit: too many arguments\n", \
		strlen("exit: too many arguments\n"));
		return (1);
	}
	exit(atoi(argvs[1]));
}
