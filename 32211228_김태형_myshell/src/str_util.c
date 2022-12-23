/**
 * str_util.c	: various utils in myshell
 * @author	: Taehyeong Kim
 * @email	: taehyeok02@gmail.com
 * @version	: 1.0
 * @date	: 2022.12.05
*/

#include "myshell.h"

size_t	f_strlcpy(char *dst, const char *src, size_t dstsize)
{
	size_t	idx;
	size_t	len_src;

	idx = 0;
	len_src = strlen(src);
	if (dstsize != 0)
	{
		while (src[idx] != '\0' && idx < len_src && idx + 1 < dstsize)
		{
			dst[idx] = src[idx];
			idx++;
		}
		dst[idx] = '\0';
	}
	return (len_src);
}

void	cut_newline(char *s)
{
	int	idx = 0;

	while (s[idx])
	{
		if (s[idx] == '\n')
			s[idx] = 0;
		idx++;
	}
}

int	is_valid(char c)
{
	if (c >= 'A' && c <= 'Z')
		return (1);
	if (c >= 'a' && c <= 'z')
		return (1);
	if (c == '_')
		return (1);
	return (0);
}

char	*str_cut_front(char *str, int idx)
{
	char	*tmp;

	while (*str == ' ')
	{
		str++;
		idx--;
	}
	tmp = malloc((idx + 1) * sizeof(char));
	f_strlcpy(tmp, str, idx);
	tmp[idx] = '\0';
	return (tmp);
}

char	*str_cut_back(char *str, int idx)
{
	char	*tmp;
	int		len;

	while (*(str + idx + 1) == ' ')
		idx++;
	len = strlen(str);
	while (*(str + len - 1) == ' ')
		len--;
	tmp = malloc((len - idx) * sizeof(char));
	f_strlcpy(tmp, str + idx + 1, len - idx);
	return (tmp);
}

char	*strjoin(char *str1, char *str2, int len)
{
	char	*joinstr;
	size_t	joinstr_len;
	size_t	idx1;
	size_t	idx2;

	idx1 = 0;
	idx2 = 0;
	joinstr_len = strlen(str1) + len + 1;
	joinstr = (char *)malloc(joinstr_len * sizeof(char));
	if (!joinstr)
		return (NULL);
	while (*(str1 + idx1))
	{
		*(joinstr + idx1) = *(str1 + idx1);
		idx1++;
	}
	while (*(str2 + idx2) && len-- > 0)
	{
		*(joinstr + idx1 + idx2) = *(str2 + idx2);
		idx2++;
	}
	*(joinstr + idx1 + idx2) = 0;
	free(str1);
	return (joinstr);
}

char	*strjoin2(char *str1, char *str2)
{
	char	*joinstr;
	size_t	joinstr_len;
	size_t	idx1;
	size_t	idx2;

	idx1 = 0;
	idx2 = 0;
	joinstr_len = strlen(str1) + strlen(str2) + 1;
	joinstr = (char *)malloc(joinstr_len * sizeof(char));
	if (!joinstr)
		return (NULL);
	while (*(str1 + idx1))
	{
		*(joinstr + idx1) = *(str1 + idx1);
		idx1++;
	}
	while (*(str2 + idx2))
	{
		*(joinstr + idx1 + idx2) = *(str2 + idx2);
		idx2++;
	}
	*(joinstr + idx1 + idx2) = 0;
	return (joinstr);
}

static size_t	ft_cntword(char const *s, char c)
{
	size_t	idx;
	size_t	cntword;

	idx = 0;
	cntword = 0;
	while (s[idx] != '\0')
	{
		if ((s[idx + 1] == c || s[idx + 1] == '\0') && s[idx] != c)
			cntword++;
		idx++;
	}
	return (cntword);
}

static void	ft_copy(char const *s, char c, char **arr)
{
	size_t	cnt_word;
	size_t	idx;
	size_t	jdx;

	cnt_word = 0;
	idx = 0;
	while (s[idx] != '\0')
	{
		jdx = 0;
		if (s[idx] == c)
			idx++;
		else
		{
			while (s[idx + jdx] != c && s[idx + jdx] != '\0')
				jdx++;
			arr[cnt_word] = (char *)malloc(sizeof(char) * (jdx + 1));
			f_strlcpy(arr[cnt_word], &s[idx], jdx + 1);
			cnt_word++;
			idx += jdx;
		}
	}
	arr[cnt_word] = 0;
}

char	**split(char const *s, char c)
{
	char	**arr;
	size_t	cnt_word;

	if (!s)
		return (0);
	cnt_word = ft_cntword(s, c);
	arr = (char **)malloc(sizeof(char *) * (cnt_word + 1));
	if (!arr)
		return (0);
	ft_copy(s, c, arr);
	return (arr);
}

void	decode_text(char *str)
{
	while (*str)
	{
		if (*str == -1)
			*str = ' ';
		str++;
	}
}
