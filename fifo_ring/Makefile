INC=-I../common
OBJ=fifo_ring
CC=gcc

all:$(OBJ)
	$(CC) $(INC) -DTEST_FIFO_RING -o $(OBJ) fifo_ring.c -lpthread

clean:
	rm -rf $(OBJ) *.o
