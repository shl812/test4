/**
 *  command_collection.c
 *  @author : 32214684 3분반 최지원
 *  @email : belle021202@naver.com
 *  @version : 1.0
 *  @date : 2022. 12. 08
 *  @environment : goorm ide
**/

#include "shell.h"

int	command_cd(int tokens_len, char *tokens[])
{
	if (tokens_len == 1) //cd -> go home
		chdir(getenv("HOME"));
	else if (tokens_len == 2)
	{
		if (chdir(tokens[1]))
			printf("cd : no such file or directory: %s\n", tokens[1]);
	}
	else
		printf("USAGE: cd [dir]\n");
	return (1);
}

int	command_exit(int tokens_len, char *tokens[])
{
	if (tokens_len != 1)
	{
		printf("USAGE: exit\n");
		return (1);
	}
	return (0);
}

int	command_output_redirect(int tokens_len, char *tokens[])
{
	int	fd;
	int	i;

	i = 0;
	while (strcmp(tokens[i] , ">"))
		i++;
	if (tokens[i + 1] == 0 || tokens[i + 2] != 0)
	{
		printf("USAGE: command > outputfile\n");
		return (0);
	}
	fd = open(tokens[i + 1], O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
	{
		printf("Can't open %s file with errno %d\n", tokens[i + 1], errno);
		return (0);
	}
	dup2(fd, 1);
	close(fd);
	while (tokens[i] != NULL)
	{
		tokens[i] = NULL;
		i++;
	}
	return (1);
}

int	command_append_redirect(int tokens_len, char *tokens[])
{
	int fd;
	int	i;

	i = 0;
	while (strcmp(tokens[i], ">>"))
		i++;
	if (tokens[i + 1] == 0 || tokens[i + 2] != 0)
	{
		printf("USAGE: command >> outputfile\n");
		return (0);
	}
	fd = open(tokens[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (fd < 0)
	{
		printf("Can't open %s file with errno %d\n", tokens[i + 1], errno);
		return (0);
	}
	dup2(fd, STDOUT_FILENO);
	close(fd);
	while (tokens[i] != NULL)
	{
		tokens[i] = NULL;
		i++;
	}
	return (1);
}

int	command_input_redirect(int tokens_len, char *tokens[])
{
	int	fd;
	int	i;
	
	i = 0;
	while (strcmp(tokens[i], "<"))
			i++;
	if (tokens[i + 1] == 0 || tokens[i + 2] != 0 || strcmp(tokens[0], "cat"))
	{
		printf("USAGE: input stream command < inputfile\n");
		return (0);
	}
	fd = open(tokens[i + 1], O_RDONLY);
	if (fd < 0)
	{
		printf("Can't open %s file with errno %d\n", tokens[i + 1], errno);
		return (0);
	}
	dup2(fd, 0);
	close(fd);
	while (tokens[i] != NULL)
	{
		tokens[i] = NULL;
		i++;
	}
	i = 0;
	return (1);
}

int	command_here_document(int tokens_len, char *tokens[])
{
	int		fd;
	int		i;
	char	line[1024];

	i = 0;
	while (strcmp(tokens[i], "<<"))
		i++;
	if (tokens[i + 1] == 0 || tokens[i + 2] != 0 || strcmp(tokens[0], "cat"))
	{
		printf("USAGE: input stream command << TAG\n");
		return (0);
	}	
	fd = open("here_doc", O_CREAT | O_RDWR | O_TRUNC, 0644);
	while (1)
	{
		printf("heredoc> ");
		fgets(line, sizeof(line) - 1, stdin);
		if (!ft_strncmp(line, tokens[i + 1], 1024))
			break;
		write(fd, line, strlen(line));
	}
	while (tokens[i] != NULL)
	{
		tokens[i] = NULL;
		i++;
	}
	close(fd);
	fd = open("here_doc", O_RDONLY);
	dup2(fd, 0);
	return (1);
}

int	do_background(int tokens_len, char *tokens[])
{
	int	fd;
	pid_t	child;
	int	exit_status;

	exit_status = -1;
	child = fork();
	if (child == 0)
	{
		execvp(tokens[0], tokens);
		exit(0);
	}
	printf("Background ps %d\n", child);
	return (1);
}
