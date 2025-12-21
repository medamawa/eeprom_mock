CC  = cc
CFLAGS  = -Wall -Wextra -Werror

TARGET = eeprom_test

SRCS =	general/src/m24c32.c \
		middleware/src/eeprom_directory.c \
		middleware/src/eeprom_alloc.c

OBJS = $(SRCS:.c=.o)

INCLUDES = -Igeneral/inc -Imiddleware/inc

TEST_SRCS = test/src/main.c

TEST_OBJS = $(TEST_SRCS:.c=.o)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(TEST_OBJS)

test: $(TARGET)
	./$(TARGET)
.PHONY: test

clean:
	rm -f $(OBJS) $(TEST_OBJS)
.PHONY: clean

fclean: clean
	rm -f $(TARGET)
.PHONY: fclean

re: fclean test
.PHONY: re
