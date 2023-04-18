obj-m += flash_monitor.o
ccflags-y += -I/usr/src/`uname -r`/include/
ccflags-y += -I$(src)/
kernel:
	make -C /lib/modules/`uname -r`/build/ M=$(PWD) modules
