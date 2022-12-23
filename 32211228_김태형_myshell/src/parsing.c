/**
 * parsing.c: Parsing text in myshell
 * @author	: Taehyeong Kim
 * @email	: taehyeok02@gmail.com
 * @version	: 1.0
 * @date	: 2022.12.05
*/

#include "myshell.h"

int	check_smallquotes(char *str)
{
	int	idx;

	idx = -1;
	while (str[++idx])
	{
		if (str[idx] == '\'')
			return (1);
	}
	return (0);
}

int	check_bigquotes(char *str)
{
	int	idx;

	idx = -1;
	while (str[++idx])
	{
		if (str[idx] == '\"')
			return (1);
	}
	return (0);
}

char	*get_env(char *str, t_list *env)
{
	while (env && env->key)
	{
		if (!strcmp(str, env->key))
			return (strdup(env->value));
		env = env->next;
	}
	return (NULL);
}

char	*check_env(char *str, int *idx, t_data *data)
{
	char	*temp;
	char	*ret;

	temp = strdup("");
	while (*str && is_valid(*str))
	{
		temp = strjoin(temp, str++, 1);
		(*idx)++;
	}
	(*idx)--;
	ret = get_env(temp, data->env);
	free(temp);
	return (ret);
}

char	*interpret_smallquotes(char *str, char *ret, int *idx)
{
	if (check_smallquotes(&str[*idx + 1]))
	{
		while (str[++(*idx)] != '\'')
		{
			if (str[*idx] == ' ')
				str[*idx] = -1;
			ret = strjoin(ret, &str[*idx], 1);
		}
	}
	else
		ret = strjoin(ret, &str[*idx], 1);
	return (ret);
}

char	*interpret_dollar(char *str, char *ret, int *idx, t_data *data)
{
	char	*temp;

	if (!str[++(*idx)] || (str[*idx - 2] == '\"' && str[*idx] == '\"'))
	{
		ret = strjoin(ret, "$", 1);
		return (ret);
	}
	else if (str[(*idx)] == ' ')
	{
		ret = strjoin(ret, "$", 1);
		(*idx)--;
		return (ret);
	}
	else if (str[(*idx)] == '\"' || str[(*idx)] == '\'')
	{
		(*idx)--;
		return (ret);
	}
	if (str[(*idx)] && !is_valid(str[*idx]))
	{
		(*str)++;
		return (ret);
	}
	temp = check_env(&str[(*idx)], idx, data);
	if (!temp)
		return (ret);
	ret = strjoin(ret, temp, strlen(temp));
	free(temp);
	return (ret);
}

char	*interpret_bigquotes(char *str, char *ret, int *idx, t_data *data)
{
	if (check_bigquotes(&str[*idx + 1]))
	{
		while (str[++(*idx)] && str[*idx] != '\"')
		{
			if (str[*idx] == ' ')
				str[*idx] = -1;
			if (str[*idx] == '$')
				ret = interpret_dollar(str, ret, idx, data);
			else
				ret = strjoin(ret, &str[*idx], 1);
		}
	}
	else
		ret = strjoin(ret, &str[*idx], 1);
	return (ret);
}

char	*set_text(char *str, t_data *data)
{
	int		idx;
	char	*ret;

	idx = -1;
	ret = strdup("");
	while (str[++idx])
	{
		if (str[idx] == '\'')
			ret = interpret_smallquotes(str, ret, &idx);
		else if (str[idx] == '\"')
			ret = interpret_bigquotes(str, ret, &idx, data);
		else if (str[idx] == '$')
			ret = interpret_dollar(str, ret, &idx, data);
		else
			ret = strjoin(ret, &str[idx], 1);
	}
	return (ret);
}
