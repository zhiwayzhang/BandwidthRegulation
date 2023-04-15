obj-m += flash_monitor.o
ccflags-y += -I/usr/src/linux-headers-5.4.0-64/include/
ccflags-y += -I$(src)/nvme
kernel:
	make -C /lib/modules/`uname -r`/build/ M=$(PWD) modules
