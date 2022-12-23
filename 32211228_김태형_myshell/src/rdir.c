/**
 * rdir.c	: redirection in myshell
 * @author	: Taehyeong Kim
 * @email	: taehyeok02@gmail.com
 * @version	: 1.0
 * @date	: 2022.12.05
*/

#include "myshell.h"

char	*get_dir_type(char *str)
{
	char	buf[1024];
	int		offset;

	offset = 0;
	while (strchr("<>", str[offset]))
		offset++;
	f_strlcpy(buf, str, offset + 1);
	return (strdup(buf));
}

char	*get_filename(char *str, int *i)
{
	char	buf[1024];
	int		offset;

	offset = 0;
	while (strchr("<>", *str))
	{
		str++;
		(*i)++;
	}
	while (isspace(*str))
	{
		str++;
		(*i)++;
	}
	while (!isspace(str[offset]) && str[offset] != '\0')
	{
		offset++;
		(*i)++;
	}
	f_strlcpy(buf, str, offset + 1);
	return (strdup(buf));
}

void	lstadd_back(t_list **lst, t_list *new)
{
	t_list	*cur;

	if (!lst || !new)
		return ;
	if (!(*lst))
	{
		*lst = new;
		return ;
	}
	cur = *lst;
	while (cur->next)
		cur = cur->next;
	cur->next = new;
}

t_list	*lstnew(char *key, char *value)
{
	t_list	*lst;

	lst = (t_list *)malloc(sizeof(t_list) * 1);
	if (!lst)
		return (0);
	lst->key = key;
	lst->value = value;
	lst->next = NULL;
	return (lst);
}

t_list	*parse_redirection(char *str)
{
	t_list	*rd_lst = 0;
	char	*rdir;
	char	*filename;
	int		idx;

	idx = 0;
	while (str[idx])
	{
		if (strchr("<>", str[idx]))
		{
			rdir = get_dir_type(&str[idx]);
			filename = get_filename(&str[idx], &idx);
			lstadd_back(&rd_lst, lstnew(rdir, filename));
		}
		else
			idx++;
	}
	return (rd_lst);
}

char	*cut_redirection(char *str)
{
	int		idx = 0;
	char	*cmd = strdup("");

	while (str[idx])
	{
		if (strchr("<>", str[idx]))
		{
			str[idx++] = 0;
			cmd = strjoin(cmd, str, 1024);
			while (strchr("<>", str[idx]))
				idx++;
			while (isspace(str[idx]))
				idx++;
			while (str[idx] && !isspace(str[idx]))
				idx++;
			str = &str[idx];
			idx = 0;
		}
		else
			idx++;
	}
	cmd = strjoin(cmd, str, 1024);
	return (cmd);
}

void	r_heredoc(char *delimeter, t_data *data)
{
	int	fd = open(".here_doc", O_RDWR | O_CREAT | O_TRUNC, 0644);
	char buf[1024] = {0};

	if (fd < 0)
	{
		data->is_error = 1;
		perror("here_doc");
		return ;
	}
	while (1)
	{
		printf("> ");
		fgets(buf, 1024, stdin);
		cut_newline(buf);
		if (!strcmp(buf, delimeter))
			break ;
		write(fd, buf, strlen(buf));
	}
	write(fd, "\n", 1);
	close(fd);
	if (data->infile_fd != -1)
		close(data->infile_fd);
	data->infile_fd = open(".here_doc", O_RDONLY, 0644);
}

void	r_infile(char *filename, t_data *data)
{
	int	fd = open(filename, O_RDONLY, 0644);

	if (fd < 0)
	{
		data->is_error = 1;
		perror(filename);
		return ;
	}
	if (data->infile_fd != -1)
		close(data->infile_fd);
	data->infile_fd = fd;
}

void	r_outfile(char *filename, t_data *data)
{
	int	fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);

	if (fd < 0)
	{
		data->is_error = 1;
		perror(filename);
		return ;
	}
	if (data->outfile_fd != -1)
		close(data->outfile_fd);
	data->outfile_fd = fd;
}

void	r_appendfile(char *filename, t_data *data)
{
	int	fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);

	if (fd < 0)
	{
		data->is_error = 1;
		perror(filename);
		return ;
	}
	if (data->outfile_fd != -1)
		close(data->outfile_fd);
	data->outfile_fd = fd;
}

void	set_redirection(t_list *rd_lst, t_data *data)
{
	t_list	*lst;

	lst = rd_lst;
	while (lst)
	{
		if (!strcmp(lst->key, "<<"))
			r_heredoc(lst->value, data);
		if (!strcmp(lst->key, "<"))
			r_infile(lst->value, data);
		if (!strcmp(lst->key, ">"))
			r_outfile(lst->value, data);
		if (!strcmp(lst->key, ">>"))
			r_appendfile(lst->value, data);
		lst = lst->next;
	}
}
