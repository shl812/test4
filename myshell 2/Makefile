CC			= gcc
CFLAGS		= -g -O2
TARGET		= myshell
OBJECTS		= checking.o executes.o redirection.o tokenize.o
MAIN_SOURCE	= main.c

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(MAIN_SOURCE)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)

.PHONY:
	clean

