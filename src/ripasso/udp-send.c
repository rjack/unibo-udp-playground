/*
 * udp-send.c - simple UDP send test.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


static const char *program_name;


#define     BIND_ADDRESS     "192.168.1.11"
#define     BIND_PORT        2020

#define     REMOTE_ADDRESS   "192.128.1.100"
#define     REMOTE_PORT      3030

#define     _STR(x)          #x
#define     STR(x)           _STR(x)

void
print_info (const char *msg)
{
	printf ("[INFO] %s: %s\n", program_name, msg);
}


int
main (const int argc, const char *argv[])
{
	int err;
	int outfd;
	char *udp_pld;
	struct iovec iov;
	struct msghdr outhdr;
	struct msghdr inhdr;
	size_t udp_pld_len;
	struct sockaddr_in outaddr;
	struct sockaddr_in remoteaddr;
	socklen_t outaddr_len;
	socklen_t remoteaddr_len;
	ssize_t nsent;

	/*
	 * Init vars.
	 */
	program_name = argv[0];

	udp_pld = "Ciao, come stai?";
	udp_pld_len = strlen (udp_pld) + 1;

	outaddr_len = sizeof(outaddr);
	memset (&outaddr, 0, outaddr_len);
	outaddr.sin_family = AF_INET;
	err = inet_pton (AF_INET, BIND_ADDRESS, &outaddr.sin_addr);
	if (err != 1) {
		perror ("inet_pton BIND_ADDRESS");
		goto inet_pton_err;
	}
	outaddr.sin_port = htons (BIND_PORT);

	remoteaddr_len = sizeof(remoteaddr);
	memset (&remoteaddr, 0, remoteaddr_len);
	remoteaddr.sin_family = AF_INET;
	err = inet_pton (AF_INET, REMOTE_ADDRESS, &remoteaddr.sin_addr);
	if (err != 1) {
		perror ("inet_pton REMOTE_ADDRESS");
		goto inet_pton_err;
	}
	remoteaddr.sin_port = htons (REMOTE_PORT);

	/*
	 * UDP socket creation.
	 */
	outfd = socket (AF_INET, SOCK_DGRAM, 0);
	if (outfd == -1) {
		perror ("socket");
		goto socket_err;
	}

	/*
	 * Bind to a specific interface.
	 */
	err = bind (outfd, (struct sockaddr *)&outaddr, outaddr_len);
	if (err) {
		perror ("bind");
		goto bind_err;
	}
	print_info ("binded to "BIND_ADDRESS":"STR(BIND_PORT));

	/*
	 * Set the IP_RECVERR option.
	 */
	/* XXX `err' used as optval. */
	err = 1;
	err = setsockopt (outfd, IPPROTO_IP, IP_RECVERR, &err, sizeof(err));
	if (err) {
		perror ("setsockopt");
		goto setsockopt_err;
	}
	print_info ("IP_RECVERR option set");

	/*
	 * Fill the iovec and the msghdr.
	 */
	iov.iov_base = udp_pld;
	iov.iov_len = udp_pld_len;
	memset (&outhdr, 0, sizeof(outhdr));
	outhdr.msg_name = (void *) &remoteaddr;
	outhdr.msg_namelen = sizeof(remoteaddr);
	outhdr.msg_iov = &iov;
	outhdr.msg_iovlen = 1;

	/*
	 * Send the datagram.
	 */
	do {
		nsent = sendmsg (outfd, &outhdr, MSG_NOSIGNAL);
	} while (nsent == -1 && errno == EINTR);
	if (nsent == -1) {
		perror ("sendmsg");
		goto sendmsg_err;
	}

	print_info ("datagram sent to "REMOTE_ADDRESS":"STR(REMOTE_PORT));


	/*
	 * Recv a response or an error via MSG_ERRQUEUE.
	 */
	do {
		nrecv = recvmsg (outfd, &inhdr, MSG_ERRQUEUE);
	} while (nrecv == -1 && errno == EINTR);

	/* Happy ending :) */
	return 0;


	/* Errors :( */
sendmsg_err:
setsockopt_err:
bind_err:
	close (outfd);

socket_err:
inet_pton_err:
	exit (EXIT_FAILURE);
}
