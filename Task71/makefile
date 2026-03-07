CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_GNU_SOURCE
TARGETS = chat client

all: $(TARGETS)

chat: chat.c
	$(CC) $(CFLAGS) -o chat chat.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean