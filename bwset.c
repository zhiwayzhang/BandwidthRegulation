#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define NETLINK_USER 31
#define FRONTGROUND 0
#define BACKGROUND 1
#define OPCODE_ADD 0
#define OPCODE_DETACH 1
#define OPCODE_MODIFY 2
#define MAX_PAYLOAD 1024
#define SEND_SUCCESS 0
#define SEND_FAILED -1
#define OP_ADD "add"
#define OP_DETACH "detach"
#define OP_MODIFY "modify"
#define FRONT_TAG "front"
#define BACK_TAG "back"

int send_to_kernel(int opcode, int pid, int tag) {
	struct sockaddr_nl src_addr;
	struct sockaddr_nl dest_addr;
	struct nlmsghdr *nlh;
	struct msghdr msg;
	struct iovec iov;
	int sock_fd;
	int rc;

	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	if (sock_fd < 0) {
		printf("socket(): %s\n", strerror(errno));
		return SEND_FAILED;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();
	bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));

	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	int *data_ptr = (int *)NLMSG_DATA(nlh);

	*data_ptr = opcode;
	*(data_ptr + 1) = pid;
	*(data_ptr + 2) = tag;

	memset(&iov, 0, sizeof(iov));
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	rc = sendmsg(sock_fd, &msg, 0);
	if (rc < 0)
	{
		printf("netlink: sendmsg(): %s\n", strerror(errno));
	}
	close(sock_fd);
	return SEND_SUCCESS;
}

// args list
// bwset {op} {pid} {tag}
// bwset add 12345
// bwset detach 12345
int main(int argc, char **args)
{
	int opcode, pid, tag;
	if (argc < 2) {
		printf("invalid args !\n");
		return 0;
	}
	if (strcmp(args[1], OP_ADD) == 0) {
		opcode = OPCODE_ADD;
		pid = atoi(args[2]);
		if (strcmp(args[3], FRONT_TAG) == 0) {
			tag = FRONTGROUND;
		} else if (strcmp(args[3], BACK_TAG) == 0) {
			tag = BACKGROUND;
		}
	} else if (strcmp(args[1], OP_DETACH) == 0) {
		opcode = OPCODE_DETACH;
		pid = atoi(args[2]);
	} else if (strcmp(args[1], OP_MODIFY) == 0) {
		opcode = OPCODE_MODIFY;
		pid = atoi(args[2]);
		// TODO
	}
	int res = send_to_kernel(opcode, pid, tag);
	if (res < 0) {
		printf("send to kernel error !\n");
		return -1;
	}
	printf("send to kernel success !\n");
	switch (opcode)
	{
	case OPCODE_ADD:
		printf("mark pid: %d as %sground task !\n", pid, (tag == FRONTGROUND ? FRONT_TAG : BACK_TAG));	
		break;
	case OPCODE_DETACH:
		printf("detach pid: %d from root cgroup fs !\n", pid);
		break;
	case OPCODE_MODIFY:
		// TODO
		break;
	default:
		break;
	}
	return 0;
}