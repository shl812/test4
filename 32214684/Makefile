TARGET = myshell

SRCS = command_collection.c ft_split.c ft_strncmp.c myshell.c 
OBJS = $(SRCS:.c=.o)
INC = .

CC = gcc

all : $(TARGET)
$(TARGET) : $(OBJS)
		$(CC) -o $(TARGET) $(OBJS)
%.o: %.c 
	$(CC) $< -o $@ -I $(INC) -c
 
 clean :
	rm -rf $(OBJS)

 fclean : clean
	rm -rf $(TARGET)

 re : fclean all
 
 .PHONY : all clean fclean re	
