CC ?= gcc
CFLAGS = -Wall -I.
LDFLAGS = -lm -lraylib

UNAME := $(shell uname) # check if the OS used is macOS
ifeq ($(UNAME), Darwin)
    LDFLAGS += -framework OpenGL -framework Cocoa -framework IOKit
else
    LDFLAGS += -lGL
endif

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
TARGET = double_pendulum

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)