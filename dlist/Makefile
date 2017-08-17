obj=dlist
CC=gcc #no -O

CFLAGS+=-maps-frame -rdynamic
LDFLAGS:=-ldl

all: $(obj)

%.o:%.c
	$(CC) -c $(CFLAGS) $(LDFLAGS) $< -o $@	

clean:
	rm -rf *.o $(obj)
