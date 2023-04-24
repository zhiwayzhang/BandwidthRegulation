#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/kallsyms.h>
#include <flash_monitor.h>
static struct task_struct *flash_monitor;

int flash_monitor_main(void *data) 
{
	long long total = 0;
	long (*do_sys_open)(int dfd, const char __user *filename, int flags,
			umode_t mode);
	int (*ksys_ioctl)(unsigned int fd, unsigned int cmd, unsigned long arg);
	do_sys_open = (void*)kallsyms_lookup_name("do_sys_open");
	ksys_ioctl = (void*)kallsyms_lookup_name("ksys_ioctl");
	long fd = do_sys_open(-100, "/dev/nvme0n1", O_RDONLY | O_LARGEFILE, 0660);
	if (fd < 0) {
		printk("open device failed errno = %ld !\n", fd);
		goto free;
		return 0;
	}
	ktime_t start_time;
	ktime_t end_time;
	printk("fd = %ld\n", fd);
	printk("load successfully !\n");
	struct nvme_passthru_cmd cmd = {
		.opcode		= 3,
		.flags		= 0,
		.rsvd1		= 0,
		.nsid		= 0x0,
		.cdw2		= 0,
		.cdw3		= 0,
		.metadata	= NULL,
		.addr		= NULL,
		.metadata_len	= 0,
		.data_len	= 0,
		.cdw10		= 1,
		.cdw11		= 0,
		.cdw12		= 0,
		.cdw13		= 0,
		.cdw14		= 0,
		.cdw15		= 0,
		.timeout_ms	= 0,
	};
	int counter = 0;
	int status = 1;
	while (true) {
		counter += 1;
		start_time = ktime_get();
		status = ksys_ioctl(fd, NVME_IOCTL_ADMIN_CMD, &cmd);
		end_time = ktime_get();
		total += ktime_to_ns(ktime_sub(end_time, start_time));
		if (status < 0) {
			break;
		}
		printk("%d : result = %u\n", counter, le32_to_cpu(cmd.result));
		ssleep(1);
		if (counter >= 3) {
			break;
		}
	}
	printk("total = %lld\n", total);
free:
	status = ksys_close(fd);
	if (status < 0) {
		printk("close fd failed !\n");
		return 0;
	}
	printk("start close ssd device and clean !\n");
	// fdput(fdget(fd));
	printk("closed fd !\n");
	do_sys_open = NULL;
	ksys_ioctl = NULL;
	// kfree(do_sys_open);
	printk("closed do_sys_open !\n");
	// kfree(ksys_ioctl);
	printk("closed ksys_ioctl !\n");
	printk("close ssd device successfully !\n");
	return 0;
}

int init_monitor(void)
{
    char thread_name[16] = "nand_flash_util";
    flash_monitor = kthread_create(flash_monitor_main, NULL, thread_name);
    if ((flash_monitor)) {
        printk(KERN_INFO "Start");
        wake_up_process(flash_monitor);
		printk("kthread start !\n");
    }
    return 0;
}

void monitor_exit(void) 
{
    // int ret;
    // ret = kthread_stop(flash_monitor);
    // if (!ret) {
		printk(KERN_INFO "Thread Stopped!");
		printk("monitor exit successfully !\n");
	// }
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhangzhiwei");
MODULE_DESCRIPTION("nand flash utilization");

module_init(init_monitor);
module_exit(monitor_exit);
