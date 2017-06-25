OBJ=pthread_demo
CC=gcc

all:$(OBJ)

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ) *.o
