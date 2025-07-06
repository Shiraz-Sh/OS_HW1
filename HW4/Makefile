#
# To compile, type "make" or "make all"
# To remove files, type "make clean"
#

TARGET2 = test2
TARGET3 = test3
OBJS2 = test_malloc_2.o malloc_2.o
OBJS3 = test_malloc_3.o malloc_3.o

CC = g++
CFLAGS = -g -Wall

# Uncomment to enable debug mode:
# CFLAGS += -DDEBUG

.PHONY: all clean

all: $(TARGET2) $(TARGET3) 

$(TARGET2): $(OBJS2)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET3): $(OBJS3)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS2) $(TARGET2) $(OBJS3) $(TARGET3)