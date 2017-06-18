obj-m:=netlink_kernel.o
KERNELBUILD :=/lib/modules/$(shell uname -r)/build  
default: 
	make -C $(KERNELBUILD) M=$(shell pwd) modules  
	echo insmod ./netlink_demo.ko to turn it on  
	gcc -o nu netlink_user.c
clean:  
	rm -rf *.o *.ko *.mod.c .*.cmd *.markers *.order *.symvers .tmp_versions nu
