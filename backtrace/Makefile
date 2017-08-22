obj=bt_arm
CC=gcc #no -O

CFLAGS+=-mapcs-frame -rdynamic
LDFLAGS:=-ldl

all: $(obj)

%.o:%.c
	$(CC) -c $(CFLAGS) $(LDFLAGS) $< -o $@	

clean:
	rm -rf *.o $(obj)
