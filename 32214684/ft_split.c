/**
 *  ft_split.c
 *  @author : 32214684 3분반 최지원
 *  @email : belle021202@naver.com
 *  @version : 1.0
 *  @date : 2022. 12. 08
 *  @environment : goorm ide
**/

#include "shell.h"

int	is_c(char word, char c)
{
	if (word == c || word == 0 || word == '\n')
		return (1);
	return (0);
}

void	save_char(char *arr, char const *s, char c)
{
	size_t	i;

	i = 0;
	while (!is_c(s[i], c))
	{
		arr[i] = s[i];
		i++;
	}
	arr[i] = 0;
}

static void	put_char(char **arr, char const *s, char c, size_t *word_cnt)
{
	size_t	i;
	size_t	j;

	i = 0;
	j = 0;
	while (s[i] != '\0')
	{
		j = 0;
		if (is_c(s[i], c))
			i++;
		else
		{
			while (!is_c(s[i + j], c))
				j++;
			arr[*word_cnt] = (char *)malloc(sizeof(char) * (j + 1));
			if (arr[*word_cnt] == 0)
				return ;
			save_char(arr[*word_cnt], &s[i], c);
			(*word_cnt)++;
			i += j;
		}
	}
}

static int	count_s_word(char const *s, char c)
{
	size_t	i;
	int		cnt;

	i = 0;
	cnt = 0;
	while (s[i])
	{
		if (!(is_c(s[i], c)) && is_c(s[i + 1], c))
			cnt++;
		i++;
	}
	return (cnt);
}

char	**ft_split(char const *s, char c, int *tokens_len)
{
	char	**arr;
	int		arr_size;
	size_t	i;
	size_t	word_cnt;

	i = 0;
	word_cnt = 0;
	arr_size = count_s_word(s, c);
	*tokens_len = arr_size;
	arr = (char **)malloc(sizeof(char *) * (arr_size + 1));
	if (arr == 0)
		return (0);
	put_char(arr, s, c, &word_cnt);
	if (word_cnt != arr_size)
	{
		while (i < word_cnt)
		{
			free(arr[i]);
			i++;
		}
		free(arr);
		return (0);
	}
	arr[arr_size] = 0;
	return (arr);
}
