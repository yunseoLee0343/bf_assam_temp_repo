CC = gcc
CFLAGS = -Wall -g -I./include
LDFLAGS = -lrt

SRC = src/main.c src/timer.c src/child_process.c src/message_queue.c
OBJ = $(SRC:.c=.o)
EXEC = scheduler

all: $(EXEC)

$(EXEC): $(OBJ)
    $(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

%.o: %.c
    $(CC) $(CFLAGS) -c $<

clean:
    rm -f $(OBJ) $(EXEC)