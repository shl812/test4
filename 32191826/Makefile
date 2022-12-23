CC			= gcc
CFLAGS		= -g -O2
TARGET		= myshell
SRCS        = $(shell find -name "*.c")
OBJS        = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)

.PHONY:
	clean

