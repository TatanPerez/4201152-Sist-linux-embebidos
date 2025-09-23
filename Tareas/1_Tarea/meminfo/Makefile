CC=gcc
CFLAGS=-Wall -Wextra -I./src
SRC=src/main.c src/utility.c
OBJ=$(SRC:.c=.o)
EXEC=meminfo
LIBS=-lncurses

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
