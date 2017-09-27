CC=gcc
CFLAGS=-Wall
LDFLAGS=-lpthread

TARGET=test

SRCS=thread.c bsem.c task_queue.c thread_pool.c test.c

OBJS=$(SRCS:.c=.o)

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) *.o $(TARGET)
