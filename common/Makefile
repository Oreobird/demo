SRC:=$(wildcard *.c)
OBJ:=$(patsubst %.c,%.o,$(notdir ${SRC}))
BIN:=$(patsubst %.c,%,$(notdir ${SRC}))

all: $(BIN)
	
%.o:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm $(BIN) 
	#find . -name *.o -exec rm -rf {}
