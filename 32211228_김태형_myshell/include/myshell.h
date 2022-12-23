/**
 * myshell.h: Header file in myshell
 * @author	: Taehyeong Kim
 * @email	: taehyeok02@gmail.com
 * @version	: 1.0
 * @date	: 2022.12.05
*/

#ifndef MYSHELL_H
# define MYSHELL_H

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>

#include <signal.h>
#include <stdio.h>
#include <errno.h>

typedef struct s_list
{
	char			*key;
	char			*value;
	struct s_list	*next;
}	t_list;

typedef struct s_data
{
	t_list	*env;
	int		outfile_fd;
	int		infile_fd;
	int		stdout_fd;
	int		stdin_fd;
	int		exit_code;
	int		is_error;
}	t_data;

char	*set_text(char *str, t_data *data);
char	*strjoin(char *str1, char *str2, int len);
char	*str_cut_front(char *str, int idx);
char	*str_cut_back(char *str, int idx);
int		is_valid(char c);
t_list	*parse_redirection(char *str);
void	set_redirection(t_list *rd_lst, t_data *data);
char	*cut_redirection(char *str);
char	*strjoin2(char *str1, char *str2);
char	**split(char const *s, char c);
char	*get_env(char *str, t_list *env);
t_list	*lstnew(char *key, char *value);
size_t	f_strlcpy(char *dst, const char *src, size_t dstsize);
int		cd(char **argvs, t_data *data);
int		echo(char **argvs);
int		fenv(char **argv, t_list *env);
int		export(char **argvs, t_data *data);
int		unset(char **argv, t_data *data);
int		pwd(char **argvs);
int		fexit(char **argvs);
void	excute(char *cmd, t_data *data);
void	decode_text(char *str);
void	cut_newline(char *s);

#endif