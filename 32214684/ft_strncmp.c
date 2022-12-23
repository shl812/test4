/**
 *  ft_strncmp.c
 *  @author : 32214684 3분반 최지원
 *  @email : belle021202@naver.com
 *  @version : 1.0
 *  @date : 2022. 12. 08
 *  @environment : goorm ide
**/

#include "shell.h"

int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t	i;

	i = 0;
	if (n == 0)
		return (0);
	while (i + 1 < n && s1[i] == s2[i] && s2[i] && s1[i] != '\n')
		i++;
	if (s1[i] == '\n' && s2[i] == '\0')
		return (0);
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}