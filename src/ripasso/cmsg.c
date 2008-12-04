#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


#define     NUM_FD     10


int
main (int argc, char *argv[])
{
	struct msghdr msg = {0};
	struct cmsghdr *cmsg;
	int myfds[NUM_FD]; /* Contains the file descriptors to pass. */
	char buf[CMSG_SPACE(sizeof myfds)];  /* ancillary data buffer */
	int *fdptr;

	msg.msg_control = buf;
	msg.msg_controllen = sizeof buf;
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int) * NUM_FD);
	/* Initialize the payload: */
	fdptr = (int *) CMSG_DATA(cmsg);
	memcpy(fdptr, myfds, NUM_FD * sizeof(int));
	/* Sum of the length of all control messages in the buffer: */
	msg.msg_controllen = cmsg->cmsg_len;

	return 0;
}
