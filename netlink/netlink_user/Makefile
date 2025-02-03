CC = gcc
CFLAGS = -Iinclude
SRC = src/main.c
TARGET = netlink_user_app

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
