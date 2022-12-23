# * Makefile: Makefile in myshell
# * @author	: Taehyeong Kim
# * @email	: taehyeok02@gmail.com
# * @version: 1.0
# * @date	: 2022.12.05


NAME = myshell
CC = gcc
CFLAGS = -Wall -Wextra -Werror -g

INCS_DIR = ./include/

SRCS_DIR = ./src/
SRCS = $(addprefix $(SRCS_DIR), $(SRC))
SRC = main.c parsing.c str_util.c rdir.c exec.c builtin.c
OBJS = $(SRCS:.c=.o)

all : $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFALGS) -o $(NAME) $(OBJS)

%.o : %.c
	$(CC) $(CFLAGS) -I$(INCS_DIR) -c -o $@ $<

clean :
	rm -f $(OBJS)

fclean : clean
	rm -f $(NAME)

re : 
	make fclean
	make all

.PHONY : all clean fclean re

