/**
 * main.c	: Main Program in myshell
 * @author	: Taehyeong Kim
 * @email	: taehyeok02@gmail.com
 * @version	: 1.0
 * @date	: 2022.12.05
*/

#include "myshell.h"

void	make_list(t_list *env, char **envp)
{
	int	idx;
	int	idx2;

	idx = -1;
	while (envp[++idx])
	{
		idx2 = -1;
		while (envp[idx][++idx2] != '=')
			;
		env->key = str_cut_front(envp[idx], idx2 + 1);
		env->value = str_cut_back(envp[idx], idx2);
		if (!envp[idx + 1])
			env->next = NULL;
		else
			env->next = (t_list *)malloc(sizeof(t_list));
		env = env->next;
	}
}

void	init_info(t_data *data)
{
	data->infile_fd = -1;
	data->outfile_fd = -1;
	data->is_error = 0;
}

int check_empty(char *str)
{
	while (*str)
	{
		if (*str++ != ' ')
			return (0);
	}
	return (1);
}

int	main(int argc, char **argv, char **envp)
{
	t_data	*data = malloc(sizeof(t_data));
	t_list	*env = malloc(sizeof(t_list));
	t_list	*rd_lst;
	char	*path;
	char	line[1024];
	char	*cmd;
	char	*r_cmd;

	(void) argv;
	if (argc != 1)
		return (1);
	make_list(env, envp);
	data->env = env;
	data->stdin_fd = dup(0);
	data->stdout_fd = dup(1);
	while (1)
	{
		init_info(data);
		path = getcwd(NULL, 0);
		printf("MYSHELL:%s# ", path);
		free(path);
		fgets(line, 1024, stdin);
		cut_newline(line);
		if (check_empty(line))
			continue ;
		cmd = set_text(line, data);
		rd_lst = parse_redirection(cmd);
		set_redirection(rd_lst, data);
		r_cmd = cut_redirection(cmd);
		free(cmd);
		if (*r_cmd != '\0' && data->is_error == 0)
			excute(r_cmd, data);
		free(r_cmd);
	}
	return (0);
}
