CC = gcc
CFLAGS = -Wall -Wextra -std=c99
OBJS = main.o list.o tokenizer.o
TARGET = program

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c list.h tokenizer.h
	$(CC) $(CFLAGS) -c main.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

tokenizer.o: tokenizer.c tokenizer.h list.h
	$(CC) $(CFLAGS) -c tokenizer.c

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean