#include <linux/types.h>
#include <linux/list.h>

struct nvme_user_io
{
	__u8 opcode;
	__u8 flags;
	__u16 control;
	__u16 nblocks;
	__u16 rsvd;
	__u64 metadata;
	__u64 addr;
	__u64 slba;
	__u32 dsmgmt;
	__u32 reftag;
	__u16 apptag;
	__u16 appmask;
};

struct nvme_passthru_cmd
{
	__u8 opcode;
	__u8 flags;
	__u16 rsvd1;
	__u32 nsid;
	__u32 cdw2;
	__u32 cdw3;
	__u64 metadata;
	__u64 addr;
	__u32 metadata_len;
	__u32 data_len;
	__u32 cdw10;
	__u32 cdw11;
	__u32 cdw12;
	__u32 cdw13;
	__u32 cdw14;
	__u32 cdw15;
	__u32 timeout_ms;
	__u32 result;
};

struct nvme_passthru_cmd64
{
	__u8 opcode;
	__u8 flags;
	__u16 rsvd1;
	__u32 nsid;
	__u32 cdw2;
	__u32 cdw3;
	__u64 metadata;
	__u64 addr;
	__u32 metadata_len;
	__u32 data_len;
	__u32 cdw10;
	__u32 cdw11;
	__u32 cdw12;
	__u32 cdw13;
	__u32 cdw14;
	__u32 cdw15;
	__u32 timeout_ms;
	__u32 rsvd2;
	__u64 result;
};

struct control_info {
	int pid;
	int tag;
	bool in_cgroup;
	struct list_head list;
};

struct netlink_message {
	int opcode;
	int pid;
	int tag;
};

#define nvme_admin_cmd nvme_passthru_cmd

#define NVME_IOCTL_ID _IO('N', 0x40)
#define NVME_IOCTL_ADMIN_CMD _IOWR('N', 0x41, struct nvme_admin_cmd)
#define NVME_IOCTL_SUBMIT_IO _IOW('N', 0x42, struct nvme_user_io)
#define NVME_IOCTL_IO_CMD _IOWR('N', 0x43, struct nvme_passthru_cmd)
#define NVME_IOCTL_RESET _IO('N', 0x44)
#define NVME_IOCTL_SUBSYS_RESET _IO('N', 0x45)
#define NVME_IOCTL_RESCAN _IO('N', 0x46)
#define NVME_IOCTL_ADMIN64_CMD _IOWR('N', 0x47, struct nvme_passthru_cmd64)
#define NVME_IOCTL_IO64_CMD _IOWR('N', 0x48, struct nvme_passthru_cmd64)
#define MAX_BANDWIDTH 400 // MB per second
#define THRESHOLD_BANDWIDTH 10 // # of MB per second
#define THRESHOLD_IOPS 300
#define THRESHOLD_BANDWIDTH_WHEN_GC 1
#define THRESHOLD_IOPS_WHEN_GC 10
#define BITS_PER_MB 1048576
#define MOD 10000
#define SAMPLING_TIMES 3 // times
#define SAMPLING_INTERVAL 3 // second
#define UTIL_THRESHOLD 0.2
#define GC_THRESHOLD 0.85
#define FRONTGROUND 0
#define BACKGROUND 1
#define OPCODE_ADD 0
#define OPCODE_DETACH 1
#define OPCODE_MODIFY 2
#define NETLINK_USER 31