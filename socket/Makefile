obj=server client
CC=gcc

all: $(obj)

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@	

clean:
	rm -rf *.o $(obj)
