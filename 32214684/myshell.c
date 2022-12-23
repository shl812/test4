/**
 *  myshell.c
 *  @author : 32214684 3분반 최지원
 *  @email : belle021202@naver.com
 *  @version : 1.0
 *  @date : 2022. 12. 08
 *  @environment : goorm ide
**/

#include "shell.h"

struct command built_in_commands[] = {
	{"cd", command_cd},
	{"exit", command_exit}
};

struct command redirect_commands[] = {
	{">", command_output_redirect},
	{"<", command_input_redirect},
	{">>", command_append_redirect},
	{"<<", command_here_document}
};

const int	built_in_commands_size = sizeof(built_in_commands) / sizeof(struct command);

const int	redirect_commands_size = sizeof(redirect_commands) / sizeof(struct command);

int	run(char *line)
{
	char	**tokens;
	int		tokens_len;
	pid_t	child;
	int		exit_status;
	int		i;
	int		j;

	tokens = ft_split(line, ' ', &tokens_len);
	exit_status = -1;
	i = 0;
	j = 0;
	if (tokens_len == 0)
		return (1);
	//background processing
	if (!strcmp(tokens[tokens_len - 1], "&"))
	{
		tokens[tokens_len - 1] = 0;
		return (do_background(tokens_len, tokens));
	}
	//redirection
	while (j < redirect_commands_size)
	{
		i = 0;
		while (tokens[i])
		{
			if (!strcmp(redirect_commands[j].command_name, tokens[i]))
			{
				if (!redirect_commands[j].cmd_func(tokens_len, tokens))
					return (1);
				else
					break;
			}
			i++;
		}
		j++;
	}
	//built-in-commands (cd, exit)
	i = 0;
	while (i < built_in_commands_size)
	{
		if (!strcmp(built_in_commands[i].command_name, tokens[0]))
			return (built_in_commands[i].cmd_func(tokens_len, tokens));
		i++;
	}
	//external commands - 기본 명령
	child = fork();
	if (child == 0)
	{
		execvp(tokens[0], tokens);
		printf("No such file\n");
		exit(1);
	}
	else if (child < 0)
		printf("Failed to fork()\n");
	else
		wait(&exit_status);
	return (1);
}

int	main()
{
	char	line[1024];
	int		a;
	int		b;

	while (1)
	{
		printf("%s $ ", getcwd(line, 1024));
		a = dup(1);
		b = dup(0);
		fgets(line, sizeof(line) - 1, stdin);
		if (!run(line))
			break;
		dup2(a, 1);
		dup2(b, 0);
	}
	return (0);
}
