#include <nvme/ioctl.h>
#include <libnvme.h>

// from nvme-cli plugin toshiba
static int nvme_sct_op(int fd,  __u32 opcode, __u32 cdw10, __u32 cdw11, void* data, __u32 data_len, unsigned int *result)
{
	void  *metadata = NULL;
	const __u32 cdw2 = 0;
	const __u32 cdw3 = 0;
	const __u32 cdw12 = 0;
	const __u32 cdw13 = 0;
	const __u32 cdw14 = 0;
	const __u32 cdw15 = 0;
	const __u32 timeout = 0;
	const __u32 metadata_len = 0;
	const __u32 namespace_id = 0x0;
	const __u32 flags = 0;
	const __u32 rsvd = 0;
	int err = 0;

	err = nvme_admin_passthru(fd, opcode, flags, rsvd,
				namespace_id, cdw2, cdw3, cdw10,
				cdw11, cdw12, cdw13, cdw14, cdw15,
				data_len, data, metadata_len, metadata,
				timeout, result);
	// for debug
	// printf("%u\n", *result);
	return err;
}
