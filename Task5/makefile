CC = gcc
CFLAGS = -g -Wall -Wextra
TARGET = myshell

all: $(TARGET)

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c -o list.o

tree.o: tree.c tree.h list.h
	$(CC) $(CFLAGS) -c tree.c -o tree.o

exec.o: exec.c exec.h tree.h
	$(CC) $(CFLAGS) -c exec.c -o exec.o

main.o: main.c list.h tree.h exec.h
	$(CC) $(CFLAGS) -c main.c -o main.o

$(TARGET): main.o list.o tree.o exec.o
	$(CC) $(CFLAGS) main.o list.o tree.o exec.o -o $(TARGET)

clean:
	rm -f *.o $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run