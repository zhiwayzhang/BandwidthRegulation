#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/delay.h>
#include <linux/kallsyms.h>
#include <flash_monitor.h>
static struct task_struct *flash_monitor;

int flash_monitor_main(void *data)
{
	// kernel sys calls
	long (*do_sys_open)(int dfd, const char __user *filename, int flags, umode_t mode);
	int (*ksys_ioctl)(unsigned int fd, unsigned int cmd, unsigned long arg);
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
	int status = 0;
	while (!kthread_should_stop())
	{
		status = ksys_ioctl(fd, NVME_IOCTL_ADMIN_CMD, &cmd);
		if (status < 0)
		{
			break;
		}
		printk("result = %u\n", counter, le32_to_cpu(cmd.result));
		ssleep(1);
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
	flash_monitor = kthread_create(flash_monitor_main, NULL, "nand_flash_util");
	if ((flash_monitor))
	{
		printk(KERN_INFO "Start");
		wake_up_process(flash_monitor);
		printk("kthread start !\n");
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
		printk("monitor exit successfully !\n");
		return;
	}
	printk("Thread Stop Failed");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhangzhiwei");
MODULE_DESCRIPTION("nand flash utilization");

module_init(init_monitor);
module_exit(monitor_exit);