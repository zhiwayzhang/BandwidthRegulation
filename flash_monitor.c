#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/netlink.h>
#include <net/net_namespace.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/delay.h>
#include <linux/kallsyms.h>
#include <linux/rwlock_types.h>
#include <linux/rwlock.h>
#include <linux/mutex.h>
#include <flash_monitor.h>
#define NETLINK_USER 31
static struct task_struct *flash_monitor;
static struct task_struct *bw_regulator;
static struct sock *nl_sk = NULL;
double shared_total_util_info;
double shared_gc_util_info;
rwlock_t lock;
rwlock_t control;
static DEFINE_MUTEX(ci_mutex);

struct control_info buffer[10];
int buffer_length;

static void my_nl_recv_msg(struct sk_buff *skb)
{
	printk("hello skb");
	struct sk_buff *skb_out;
	struct nlmsghdr *nlh;
	int msg_size;
	int *data;
	int pid;
	int res;

	nlh = (struct nlmsghdr *)skb->data;
	pid = nlh->nlmsg_pid; /* pid of sending process */
	data = (int *)NLMSG_DATA(nlh);
	msg_size = sizeof(data);
	printk(KERN_INFO "netlink_test: Received from pid %d, size: %d\n", pid, msg_size);
	mutex_lock(&ci_mutex);
    buffer[buffer_length].pid = data[0];
	buffer[buffer_length].tag = data[1];
	buffer_length += 1;
	mutex_unlock(&ci_mutex);
    printk(KERN_INFO "Received integers: %d, %d\n", data[0], data[1]);
}

static struct netlink_kernel_cfg nl_cfg = {
    .input = my_nl_recv_msg,
};

// create cgroup
// delete cgroup (omit)

// io.max
// bwset {pid} {tag/cgroup} {bandwidth}
// set bandwidth

// bwunset {pid} {tag/cgroup}
// unset bandwidth

// cgroup.procs
// inesrt {pid} {tag/cgroup}
// insert pid to cgroup

// remove {pid} {tag/cgroup}
// remove pid from cgroup

int bw_regulator_main(void *data) 
{
	// kernel sys calls
	long (*do_rmdir)(int dfd, const char __user *pathname);
	long (*do_mkdirat)(int dfd, const char __user *pathname, umode_t mode);
	off_t (*ksys_lseek)(unsigned int fd, off_t offset, unsigned int whence);
	ssize_t (*ksys_write)(unsigned int fd, const char __user *buf, size_t count);
	long (*do_sys_open)(int dfd, const char __user *filename, int flags, umode_t mode);
	// kallsyms lookup by name
	do_sys_open = (void *)kallsyms_lookup_name("do_sys_open");
	do_rmdir = (void *)kallsyms_lookup_name("do_rmdir");
	ksys_lseek = (void *)kallsyms_lookup_name("ksys_lseek");
	do_mkdirat = (void *)kallsyms_lookup_name("do_mkdirat");
	ksys_write = (void *)kallsyms_lookup_name("ksys_write");
	// init cgroup fs
	int err = do_mkdirat(-100, "/sys/fs/cgroup/front", 0755);
	if (err < 0) {
		printk("mk /sys/fs/cgroup/front Failed !\n");
	}
	err = do_mkdirat(-100, "/sys/fs/cgroup/back", 0755);
	if (err < 0) {
		printk("mk /sys/fs/cgroup/back Failed !\n");
	}
	// Flag for Status
	bool THRESHOLD_ENABLED = false;
	unsigned int result = 0;
	double gc_util = 0;
	double total_util = 0;
	while (!kthread_should_stop())
	{
		read_lock(&lock);
		total_util = shared_total_util_info;
		gc_util = shared_gc_util_info;
		read_unlock(&lock);
		int d_total_util = total_util*MOD;
		int d_gc_util = gc_util*MOD;
		printk("REGULATOR: in last 5s : total=%d gc=%d\n", d_total_util, d_gc_util);
		if (1 - total_util < UTIL_THRESHOLD && !THRESHOLD_ENABLED) {
			// do threshold
			THRESHOLD_ENABLED = true;
		}
		if (1 - total_util > UTIL_THRESHOLD && THRESHOLD_ENABLED) {
			// disable threshold
			THRESHOLD_ENABLED = false;
		}
		ssleep(1);
	}
	// remove cgroup fs
	err = do_rmdir(-100, "/sys/fs/cgroup/front");
	if (err < 0) {
		printk("rmdir /sys/fs/cgroup/front Failed !\n");
	}
	err = do_rmdir(-100, "/sys/fs/cgroup/back");
	if (err < 0) {
		printk("rmdir /sys/fs/cgroup/back Failed !\n");
	}
	// clear pointer
	do_sys_open = NULL;
	do_rmdir = NULL;
	ksys_lseek = NULL;
	do_mkdirat = NULL;
	ksys_write = NULL;
	return 0;
}

int flash_monitor_main(void *data)
{
	// kernel sys calls
	long (*do_sys_open)(int dfd, const char __user *filename, int flags, umode_t mode);
	int (*ksys_ioctl)(unsigned int fd, unsigned int cmd, unsigned long arg);
	// kallsyms lookup by name
	do_sys_open = (void *)kallsyms_lookup_name("do_sys_open");
	ksys_ioctl = (void *)kallsyms_lookup_name("ksys_ioctl");
	// do sys open
	// open fd of /dev/nvme0n1
	int fd = do_sys_open(-100, "/dev/nvme0n1", O_RDONLY | O_LARGEFILE, 0660);
	if (fd < 0)
	{
		printk("open device failed errno = %d !\n", fd);
		goto free;
		return 0;
	}
	// for nvme admin request
	struct nvme_passthru_cmd cmd = {
		.opcode = 3,
		.flags = 0,
		.rsvd1 = 0,
		.nsid = 0x0,
		.cdw2 = 0,
		.cdw3 = 0,
		.metadata = NULL,
		.addr = NULL,
		.metadata_len = 0,
		.data_len = 0,
		.cdw10 = 1,
		.cdw11 = 0,
		.cdw12 = 0,
		.cdw13 = 0,
		.cdw14 = 0,
		.cdw15 = 0,
		.timeout_ms = 0,
	};
	// for statistic
	int counter = 0;
	double total_util = 0;
	double gc_util = 0;
	unsigned int result = 0;
	int status = 0;
	while (!kthread_should_stop())
	{
		status = ksys_ioctl(fd, NVME_IOCTL_ADMIN_CMD, &cmd);
		if (status < 0)
		{
			printk("encounter ioctl error!\n");
			break;
		}
		counter += 1;
		printk("counter = %d, result = %u \n", counter, le32_to_cpu(cmd.result));
		result = le32_to_cpu(cmd.result);

		gc_util += 1.0*(result % MOD)/MOD;
		total_util += 1.0*result/MOD/MOD;
		// process threshold
		if (counter == SAMPLING_TIMES) {
			gc_util /= SAMPLING_TIMES;
			total_util /= SAMPLING_TIMES;
			int d_total_util = total_util*MOD;
			int d_gc_util = gc_util*MOD;
			printk("MONITOR: in last 5s : total=%d gc=%d\n", d_total_util, d_gc_util);
			write_lock(&lock);
			shared_total_util_info = total_util;
        	shared_gc_util_info = gc_util;
        	write_unlock(&lock);
			counter = 0;
			gc_util = 0;
			total_util = 0;
		}
 		ssleep(SAMPLING_INTERVAL);
	}
	// close fd
	int res = ksys_close(fd);
	if (res < 0)
	{
		printk("close fd failed !\n");
		return 0;
	}
free:
	fdput(fdget(fd));
	printk("closed fd !\n");
	do_sys_open = NULL;
	ksys_ioctl = NULL;
	printk("close ssd device successfully !\n");
	return 0;
}

int init_monitor(void)
{
	rwlock_init(&lock);
	mutex_init(&ci_mutex);

	buffer_length = 0;
	// create netlink socket
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &nl_cfg);
    if (!nl_sk) {
        printk(KERN_ERR "Failed to create netlink socket\n");
        return -ENOMEM;
    }

	shared_gc_util_info = 0;
	shared_total_util_info = 0;
	
	flash_monitor = kthread_create(flash_monitor_main, NULL, "nand_flash_util");
	if ((flash_monitor))
	{
		printk("Start");
		wake_up_process(flash_monitor);
		printk("flash_monitor kthread start !\n");
	}

	bw_regulator = kthread_create(bw_regulator_main, NULL, "bw_regulator");
	if ((bw_regulator))
	{
		printk("Start");
		wake_up_process(bw_regulator);
		printk("bw_regulator kthread start !\n");
	}

	return 0;
}

void monitor_exit(void)
{
	int ret;
	ret = kthread_stop(flash_monitor);
	if (!ret)
	{
		printk("Thread Stopped !\n");
		printk("flash_monitor exit successfully !\n");
	}
	ret = kthread_stop(bw_regulator);
	if (!ret)
	{
		printk("Thread Stopped !\n");
		printk("bw_regulator exit successfully !\n");
	}
	// rwlock_destroy(&lock);
	printk("release lock ! \n");
	if (nl_sk) {
        netlink_kernel_release(nl_sk);
    }
	printk("release netlink socket ! \n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhangzhiwei");
MODULE_DESCRIPTION("nand flash utilization");

module_init(init_monitor);
module_exit(monitor_exit);