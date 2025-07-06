#
# To compile, type "make" or "make all"
# To remove files, type "make clean"
#

TARGET = test
OBJS = test.o malloc_3.o

CC = g++
CFLAGS = -g -Wall

# Uncomment to enable debug mode:
# CFLAGS += -DDEBUG

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)