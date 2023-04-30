#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define NETLINK_USER 31

#define MAX_PAYLOAD 1024

int main(int argc, char **argv)
{
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
    return 1;
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
  *data_ptr = 12222;
  *(data_ptr + 1) = 1;
  memset(&iov, 0, sizeof(iov));
  iov.iov_base = (void *)nlh;
  iov.iov_len = nlh->nlmsg_len;

  memset(&msg, 0, sizeof(msg));
  msg.msg_name = (void *)&dest_addr;
  msg.msg_namelen = sizeof(dest_addr);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  printf("Send to kernel: %s\n", argv[1]);

  rc = sendmsg(sock_fd, &msg, 0);
  if (rc < 0)
  {
    printf("sendmsg(): %s\n", strerror(errno));
  }
  close(sock_fd);
  return 0;
}