obj-m += flash_monitor.o
ccflags-y += -I/usr/src/`uname -r`/include/
ccflags-y += -I$(src)/
ccflags-y += -msse2
kernel:
	make -C /lib/modules/`uname -r`/build/ M=$(PWD) modules
bwset:
	gcc bwset.c -o bwset
install:
	cp bwset /usr/local/bin/bwset
uninstall:
	rm /usr/local/bin/bwset