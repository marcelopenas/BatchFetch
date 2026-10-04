CC = gcc
TARGET = batchfetch
SRCS = main.c arguments.c downloader.c signals.c url_list.c url_utils.c
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Og -g -D_POSIX_C_SOURCE=200809L
LIBS = -lcurl

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
