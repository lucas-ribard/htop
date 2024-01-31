CC = gcc
CFLAGS = -Wall -Wextra -Werror -g3
LDFLAGS = -lncurses

SRC = main.c
TARGET = htop

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
