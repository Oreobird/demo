SRC:=$(wildcard *.c)
OBJ:=$(patsubst %.c, %.o, $(notdir ${SRC}))
BIN:=test

CC=gcc

all: $(BIN) 

$(BIN):$(OBJ)

%.o:%.c
	$(CC) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o $(BIN)
