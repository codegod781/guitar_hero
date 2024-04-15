CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99 -D_XOPEN_SOURCE=600
LDFLAGS=-lSDL2 -lpthread -lpng

SRCS=game_logic.c sprites.c VGA.c vga_emulator.c
OBJS=$(SRCS:.c=.o)
TARGET=game_logic

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
