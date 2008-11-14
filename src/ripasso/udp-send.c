/*
 * udp-send.c - simple UDP send test.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <linux/errqueue.h>
#include <linux/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


static const char *program_name;


#define     BIND_ADDRESS     "127.0.0.1"
#define     BIND_PORT        2020

#define     REMOTE_ADDRESS   "127.0.0.1"
#define     REMOTE_PORT      3030

#define     _STR(x)          #x
#define     STR(x)           _STR(x)

#define     INBUFLEN         100

#define     ARRAYLEN(a)      (sizeof((a))/sizeof(*(a)))

#define     CMSGBUFLEN       1024

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
	char inbuf[INBUFLEN];
	char *udp_pld;
	struct iovec outiov;
	struct iovec iniov;
	struct msghdr outhdr;
	struct msghdr inhdr;
	size_t udp_pld_len;
	struct sockaddr_in bindaddr;
	struct sockaddr_in remoteaddr;
	socklen_t bindaddr_len;
	socklen_t remoteaddr_len;
	ssize_t nsent;
	ssize_t nrecv;

	/*
	 * Init vars.
	 */
	program_name = argv[0];

	udp_pld = "Ciao, come stai?";
	udp_pld_len = strlen (udp_pld) + 1;

	bindaddr_len = sizeof(bindaddr);
	memset (&bindaddr, 0, bindaddr_len);
	bindaddr.sin_family = AF_INET;
	err = inet_pton (AF_INET, BIND_ADDRESS, &bindaddr.sin_addr);
	if (err != 1) {
		perror ("inet_pton BIND_ADDRESS");
		goto inet_pton_err;
	}
	bindaddr.sin_port = htons (BIND_PORT);

	remoteaddr_len = sizeof(remoteaddr);
	memset (&remoteaddr, 0, remoteaddr_len);
	remoteaddr.sin_family = AF_INET;
	err = inet_pton (AF_INET, REMOTE_ADDRESS, &remoteaddr.sin_addr);
	if (err != 1) {
		perror ("inet_pton REMOTE_ADDRESS");
		goto inet_pton_err;
	}
	remoteaddr.sin_port = htons (REMOTE_PORT);

	outiov.iov_base = udp_pld;
	outiov.iov_len = udp_pld_len;
	memset (&outhdr, 0, sizeof(outhdr));
	outhdr.msg_name = (void *) &remoteaddr;
	outhdr.msg_namelen = sizeof(remoteaddr);
	outhdr.msg_iov = &outiov;
	outhdr.msg_iovlen = 1;

	memset (&inbuf, '\0', sizeof(inbuf));

	iniov.iov_base = inbuf;
	iniov.iov_len = sizeof(inbuf);
	memset (&inhdr, 0, sizeof(inhdr));
	inhdr.msg_name = (void *) &remoteaddr;
	inhdr.msg_namelen = sizeof(remoteaddr);
	inhdr.msg_iov = &iniov;
	inhdr.msg_iovlen = 1;


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
	err = bind (outfd, (struct sockaddr *)&bindaddr, bindaddr_len);
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
	 * poll for events.
	 */
	for (;;) {
		int nready;
		char cmsgbuf[CMSGBUFLEN];
		struct pollfd pfd[1];

		inhdr.msg_control = &cmsgbuf;
		inhdr.msg_controllen = sizeof(cmsgbuf);

		memset (&cmsgbuf, 0, sizeof(cmsgbuf));

		nready = 0;
		pfd[0].fd = outfd;
		pfd[0].events = 0 | POLLIN | POLLERR;

		nready = poll (pfd, ARRAYLEN(pfd), -1);
		if (nready == -1) {
			perror ("poll");
			goto poll_err;
		}
		assert (nready != 0);

		/*
		 * Let's see what happened.
		 */
		if (pfd[0].revents & POLLERR) {
			struct cmsghdr *curcmsg;

			print_info ("POLLERR, reading error queue...");

			do {
				nrecv = recvmsg (outfd, &inhdr, MSG_ERRQUEUE);
			} while (nrecv == -1 && errno == EINTR);

			if (nrecv == -1) {
				perror ("recvmsg MSG_ERRQUEUE");
				goto recvmsg_err;
			}

			assert (inhdr.msg_flags & MSG_ERRQUEUE);

			for (curcmsg = CMSG_FIRSTHDR (&inhdr);
			     curcmsg != NULL;
			     curcmsg = CMSG_NXTHDR (&inhdr, curcmsg)) {
				print_info ("curcmsg");
			}
		} else {
			assert (pfd[0].revents & POLLIN);

			do {
				nrecv = recvmsg (outfd, &inhdr, 0);
			} while (nrecv == -1 && errno == EINTR);

			assert (!(inhdr.msg_flags & MSG_ERRQUEUE));

			if (nrecv == -1) {
				perror ("recvmsg");
				goto recvmsg_err;
			}

			print_info ("Messaggio ricevuto:");
			printf ("%s", inbuf);
			fflush (stdout);
		}
	}

	/* Happy ending :) */
	return 0;


	/*
	 * Errors :(
	 */
poll_err:
recvmsg_err:
sendmsg_err:
setsockopt_err:
bind_err:
	close (outfd);

socket_err:
inet_pton_err:
	exit (EXIT_FAILURE);
}
