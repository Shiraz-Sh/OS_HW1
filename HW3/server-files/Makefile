#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#

OBJS = server.o request.o segel.o client.o log.o rw_lock.o fifo_queue.o
TARGET = server

CC = gcc
CFLAGS = -g -Wall

DEBUG ?= 0

ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG
endif

LIBS = -lpthread

.SUFFIXES: .c .o

all: server client output.cgi
	-mkdir -p public
	-cp output.cgi favicon.ico home.html public

server: server.o request.o segel.o log.o rw_lock.o fifo_queue.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

client: client.o segel.o
	$(CC) $(CFLAGS) -o client client.o segel.o

output.cgi: output.c
	$(CC) $(CFLAGS) -o output.cgi output.c

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) server client output.cgi
	-rm -rf public
