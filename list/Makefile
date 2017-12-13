SRC:=$(wildcard *.c)
OBJ:=$(patsubst %.c, %.o, $(notdir ${SRC}))
BIN:=list

#CFLAGS+=-DTEST_DOUBLE_LIST
CFLAGS+=-DTEST_SINGLE_LIST
CC=gcc

all: $(BIN) 

$(BIN):$(OBJ)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -rf *.o $(BIN)
