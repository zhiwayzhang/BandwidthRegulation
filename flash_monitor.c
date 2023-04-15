#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/blk_types.h>
#include <linux/blkdev.h>
#include <host/nvme.h>
#include <linux/fs.h>
#include <linux/delay.h>
static struct task_struct *flash_monitor;

int flash_monitor_main(void *data) 
{
	struct block_device *ssd = blkdev_get_by_path("/dev/nvme0n1", FMODE_READ, THIS_MODULE);
	struct nvme_ns_head *head = NULL;
	struct nvme_ns *ns;
	struct device *dev = disk_to_dev(ssd->bd_disk);
	ns = nvme_get_ns_from_dev(dev);
	printk("load successfully !\n");
	struct nvme_command c;
	memset(&c, 0, sizeof(c));
	c.common.opcode = 3;
	c.common.cdw2[0] = 0;
	c.common.cdw2[1] = 0;
	c.common.cdw10 = 1;
	c.common.cdw11 = 0;
	c.common.cdw12 = 0;
	c.common.cdw13 = 0;
	c.common.cdw14 = 0;
	c.common.cdw15 = 0;
	c.common.cdw15 = 0;
	int counter = 0;
	unsigned long buff_len = 0;
	union nvme_result res = {};
	while (true) {
		if (counter >= 60) {
			break;
		}
		printk("start submit nvme admin command !\n");
		int status = __nvme_submit_sync_cmd(ns->ctrl->admin_q, &c, &res, NULL, buff_len, 0, NVME_QID_ANY, 0, 0, false);
		printk("end submit nvme admin command !\n");
		printk("status = %d\n", status);
		if (status < 0) {
			break;
		}
		printk("result = %u\n", le32_to_cpu(res.u32));
		counter += 1;
		ssleep(1);
	}
    blkdev_put(ssd, FMODE_READ | FMODE_EXCL);
	kfree(ns);
	printk("close ssd device successfully !\n");
	return 0;
}

int init_monitor(void)
{
    char thread_name[20] = "nand_flash_util";
    flash_monitor = kthread_create(flash_monitor_main, NULL, thread_name);
    if ((flash_monitor)) {
        printk(KERN_INFO "Error");
        wake_up_process(flash_monitor);
    }
    return 0;
}

void monitor_exit(void) 
{
    int ret;
    ret = kthread_stop(flash_monitor);
    if (!ret)
        printk(KERN_INFO "Thread Stopped!");
	printk("monitor exit successfully !\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhangzhiwei");
MODULE_DESCRIPTION("nand flash utilization");

module_init(init_monitor);
module_exit(monitor_exit);
