SRV:=producer
CLI:=costomer
CC=gcc

srv_src:=producer.c
cli_src:=costomer.c

com_src:=$(filter-out $(srv_src) $(cli_src), $(wildcard *.c))
com_obj:=$(patsubst %.c,%.o, $(notdir ${com_src}))

CFLAGS:=CFLAG
CFLAGS+=-W -Wall -Werror

all: $(SRV) $(CLI)

$(SRV):${com_obj} ${SRV}.o
	$(CC) ${com_obj} ${SRV}.o $(CFLAGS) $(LDFLAGS) -o $@

$(CLI):${com_obj} ${CLI}.o
	$(CC) ${com_obj} ${CLI}.o $(CFLAGS) $(LDFLAGS) -o $@

%.o:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@	

.PHONY:clean
clean:
	rm -rf *.o $(obj) $(SRV) $(CLI)
