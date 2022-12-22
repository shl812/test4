#include "../include/shell.h"

void	handler(int sign)
{
	printf("\n");
	(void)sign;
}

void	err_exit()
{
	perror("");
	exit(1);
}

void	cmd_err(char *cmd)
{
	printf("bash: %s: command not found\n", cmd);
	exit(127);
}

/* 'PATH='이 시작이면 건너뛰고 ':'를 기준으로 건너뜀 */
int	find_path(char **path)
{
	if (!strncmp(*path, "PATH=", 5))
	{
		*path += 5;
		return (1);
	}
	*path = strchr(*path, ':') + 1;
	if ((long long)*path > 1)
		return (1);
	return (0);
}

int	my_strlen(char *str, char lmt)
{
	int	len;

	len = 0;
	while (str[len] != lmt && str[len])
		len++;
	return (len);
}

char	*parsing_cmd(char *ori_cmd)
{
	char	*cmd;
	char	*path;
	int		path_size;
	int		aloc_size;

	/* size of cmd check */
	aloc_size = my_strlen(ori_cmd, ' ');
	/* 환경변수에서 PATH 부분을 path변수에 초기화 */
	path = getenv("PATH");
	while (find_path(&path))
	{
		path_size = my_strlen(path, ':');
		cmd = calloc(aloc_size + path_size + 2, sizeof(char));
		if (!cmd)
			err_exit();
		/* 명령어 경로 대입 */
		strlcpy(cmd, path, path_size + 1);
		/* '/' 경로의 형식을 위해 추가 */
		strlcat(cmd, "/", path_size + 2);
		/* cmd를 마지막에 추가 */
		strlcat(cmd, ori_cmd, aloc_size + path_size + 2);
		/* 만들어진 경로에 접근했을 때, 파일이 없거나, 실행할 수 없다면 탈출 후 아래에서 에러 체크*/
		if (!access(cmd, F_OK | X_OK))
			break ;
		/* 아니라면 다시 체크해야 하므로 free */
		free(cmd);
	}
	if (access(cmd, F_OK | X_OK))
		cmd_err(ori_cmd);
	return (cmd);
}

void	exec(t_info *info)
{
	char	*cmd;

	/* redirection in 체크 */
	if (!strcmp(info->redir_in[0], "<"))
		info->fds[IN][IN] = open(info->redir_in[1], O_RDONLY, 0);
	// else if (!strcmp(info->redir_in[0], "<<"))
	else
		info->fds[IN][IN] = dup(STDIN_FILENO);

	/* redirection out 체크 */
	if (!strcmp(info->redir_out[0], ">"))
		info->fds[OUT][OUT] = open(info->redir_out[1], O_WRONLY | O_CREAT | O_TRUNC, DEFFILEMODE);
	else if (!strcmp(info->redir_out[0], ">>"))
		info->fds[OUT][OUT] = open(info->redir_out[1], O_WRONLY | O_CREAT | O_APPEND, DEFFILEMODE);
	else
		info->fds[OUT][OUT] = dup(STDOUT_FILENO);

	/* dup2로 STDIN, STDOUT가 가리키는 위치를 fds들이 가리키는 것들로 교환 후 close */
	dup2(info->fds[IN][IN], STDIN_FILENO);
	close(info->fds[IN][IN]);
	dup2(info->fds[OUT][OUT], STDOUT_FILENO);
	close(info->fds[OUT][OUT]);

	/* 명령어 */
	cmd = parsing_cmd(info->cmd);
	if (execve(cmd, info->av, info->env) == -1)
		cmd_err(cmd);
}

t_info parsing_line(char *line)
{
	t_info info;
	char **s;
	char **tmp;
	int cnt = 0;

	s = ft_split(line, ' ');
	tmp = s;
	
	while (*tmp) {
		tmp++;
		cnt++;
	}
	info.av = (char **)malloc(sizeof(char *) * cnt);
	tmp = info.av;
	while (*s)
	{
		if (strcmp(*s, ">>") == 0 || strcmp(*s, ">") == 0)
		{	
			info.redir_out[0] = *s;
			s++;
			info.redir_out[1] = *s;
		}
		else if (strcmp(*s, "<<") == 0|| strcmp(*s, "<") == 0)
		{
			info.redir_in[0] = *s;
			s++;
			info.redir_in[1] = *s;
			// s++;
		}
		else {
			*(info.av) = *s;
			(info.av)++;
		}
		s++;
	}
	if (cnt > 1)
	{
		s--;
		if (!strcmp(*s, "&"))
		{
			info.background = WNOHANG;
			(info.av)--;
			*(info.av) = NULL;
		}
	}
	info.av = tmp;

	info.cmd = info.av[0];
	return (info);
}

void	inter_cmd(char *line, char *env[])
{
	t_info		info;

	info.background = 0;
	// info.av = 0;
	info.env = env;
	info = parsing_line(line);
	info.pid = fork();
	/* fork error check */
	if (info.pid < 0)
		exit (1);
	/* 부모 프로세스 */
	if (info.pid > 0)
		waitpid(info.pid, &info.stat, info.background);
	/* 자식 프로세스 */	
	else
		exec(&info);
}

int		main(int ac, char *av[], char *env[])
{
	char	*line;

	printf("32213538 이지수 22.12.09\n");
	/* error 방지 */
	(void)ac; (void)av;
	/* 시그널 인터럽트 */
	signal(SIGINT, handler);
	while (true)
	{
		/* 한 줄씩 읽기 */
		line = readline("jisoolee$> ");
		/* line이 비어있다면 다시 입력 */
		if (line && *line)
		{
			/* line이 exit이라면 종료 */
			if (!strcmp(line, "exit"))
				exit (0);
			/* line 해석 */
			inter_cmd(line, env);
			add_history(line);
			free(line);
		}
	}
	return (0);
}
