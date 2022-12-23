#ifndef SHELL_H
# define SHELL_H

# include <unistd.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <stdlib.h>
# include <fcntl.h>
# include <sys/wait.h>

struct command
{
	char	*command_name;
	int		(*cmd_func)(int, char **);
};

int	run(char *line);
int	ft_strncmp(const char *s1, const char *s2, size_t n);
int	command_cd(int tokens_len, char *tokens[]);
int	command_exit(int tokens_len, char *tokens[]);
int	command_output_redirect(int tokens_len, char *tokens[]);
int	command_append_redirect(int tokens_len, char *tokens[]);
int	command_input_redirect(int tokens_len, char *tokens[]);
int	command_here_document(int tokens_len, char *tokens[]);
int	do_background(int tokens_len, char *tokens[]);
int	is_c(char word, char c);
void	save_char(char *arr, char const *s, char c);
static void	put_char(char **arr, char const *s, char c, size_t *word_cnt);
static int	count_s_word(char const *s, char c);
char	**ft_split(char const *s, char c, int *tokens_len);

#endif
