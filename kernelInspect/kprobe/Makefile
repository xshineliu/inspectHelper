# builds the kprobes example kernel modules;
# then to use one (as root):  insmod <module_name.ko>

obj-m += a.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean


