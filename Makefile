CC     = clang
CFLAGS = -std=c99 -Wall -pedantic -O2

saferase: main.c
	$(CC) -o $@ $(CFLAGS) $^
