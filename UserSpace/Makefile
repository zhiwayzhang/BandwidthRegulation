clean:
	rm *.o

all:
	gcc -static -L/usr/local/lib/x86_64-linux-gnu/ statistics.c nvme.c -o statistics -l:libnvme.a

upload:
	gcc -static -L/usr/local/lib/x86_64-linux-gnu/ statistics.c nvme.c -o statistics -l:libnvme.a
	scp -P 8080 statistics femu@localhost:/home/femu/
