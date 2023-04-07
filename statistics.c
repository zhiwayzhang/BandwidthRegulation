#include "nvme.c"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
const int MOD = 10000;

int to_csv(double *total_utilization, double *gc_utilization, int length)
{
	FILE *fileptr;
	fileptr = fopen("./data.csv", "w");
	if (fileptr == NULL) {
		printf("file open error\n");
		return -1;
	}
	fprintf(fileptr, "time,total_util,gc_util\n");
	for (int i = 0; i < length; i++) {
		fprintf(fileptr, "%d,%lf,%lf\n", i+1, total_utilization[i], gc_utilization[i]);
	}
	fclose(fileptr);
	return 0;
}

static int do_statistic(int dev_fd, double *total_utilization, double *gc_utilization, 
					int length, double sampling_interval, double sampling_time)
{
	int err;
	int start_time = time(NULL);
	int writer = 0;
	while (true) {
		if (writer >= length) break;
		int now_time = time(NULL);
		if (now_time - start_time < sampling_interval) continue;
		start_time = now_time;
		// do nvme admin command submti
		unsigned int result;
		err = nvme_sct_op(dev_fd, 3, 1, 0, 0, 0, &result);
		if (err < 0) {
			printf("errno: %d\n", err);
			return -1;
		}
		// parse command result
		// get the last 4 digits
		gc_utilization[writer] = 1.0*(result % MOD)/MOD;
		total_utilization[writer] = 1.0*result/MOD/MOD;
		if (total_utilization[writer] > 1.0) total_utilization[writer] = 1.0;
		writer++;
	}
	return 0;
}

static int get_device_fd()
{
	int err;
	err = open("/dev/nvme0n1", O_RDONLY);
	if (err < 0) {
		printf("device open error");
		return err;
	}
	int dev_fd = err;
	return dev_fd;
}

int main()
{
	int dev_fd = get_device_fd();
	if (dev_fd < 0) {
		return 0;
	}
	// statistic array
	double *total_utilization = NULL;
	double *gc_utilization = NULL;
	double sampling_interval = 1; // seconds
	double sampling_time = 10; // seconds
	int length = sampling_time/sampling_interval;
    total_utilization = malloc(sizeof(double)*length);
    gc_utilization = malloc(sizeof(double)*length);
	do_statistic(dev_fd, total_utilization, gc_utilization, length, sampling_interval, sampling_time);
	close(dev_fd);
	to_csv(total_utilization, gc_utilization, length);
	return 0;
}
