SRC:=$(wildcard *.c)
OBJ:=$(patsubst %.c, %.o, $(notdir ${SRC}))
BIN:=list

CFLAGS:=$(CFLAG)
CC=gcc

all: $(BIN) 

$(BIN):$(OBJ)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o $(BIN)
