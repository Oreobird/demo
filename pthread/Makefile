OBJ=pthread_demo
CC=gcc

all:$(OBJ)

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@ -lpthread

clean:
	rm -rf $(OBJ) *.o
