/*
 * pingo_pongo.c - ripasso UDP
 *
 * uso:
 * pingo_pongo -l indirizzo:porta -r indirizzo:porta
 *
 * TODO permettere invocazioni :porta per intendere localhost:porta
 * TODO interrogazione DNS
 */

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


/****************************************************************************
				     Tipi
****************************************************************************/

struct chan {
	struct sockaddr_in ch_locaddr;
	struct sockaddr_in ch_remaddr;


};

/****************************************************************************
			  Prototipi funzioni private
****************************************************************************/

static void
print_usage (FILE *out, const char *program_name);


static struct sockaddr_in *
new_addr_from_str (const char *string);


/****************************************************************************
			      Funzioni pubbliche
****************************************************************************/

int
main (const int argc, const char **argv)
{
	int i;
	struct chan *local;
	struct chan *remote;

	struct addrinfo local_hints;
	struct addrinfo remote_hints;

	if (argc != 5) {
		print_usage (stderr, argv[0]);
		exit (EXIT_FAILURE);
	}

	local_chan = new_listening_chan (argv[1], argv[2], &local_hints);
	if (local_chan == NULL) {
		perror ("[server] error");
		exit (EXIT_FAILURE);
	}

	remote_chan = new_connecting_chan (argv[3], argv[4], &remote_hints);
	if (local_chan == NULL) {
		perror ("[client] error");
		exit (EXIT_FAILURE);
	}

	select_loop (local, remote);

	free_chan (local_chan);
	free_chan (remote_chan);

	return 0;
}


/****************************************************************************
			       Funzioni private
****************************************************************************/

static struct chan*
new_connecting_chan (const char *name, const char *service,
                     const struct addrinfo *hints)
{
	struct chan *nc;

	nc = (struct chan *) malloc (sizeof(*nc));
	if (nc == NULL)
		return NULL;

	return nc;
}


static void
print_usage (FILE *out, const char *program_name)
{
	assert (out != NULL);
	assert (program_name != NULL);

	fprintf (out, "uso:\n"
	              "  %s INDIRIZZO_LOCALE PORTA_LOCALE INDIRIZZO_REMOTO "
	              "PORTA_REMOTA\n"
		      "\n"
		      "\"localhost\" e \"127.0.0.x\" possono essere "
		      "specificati con \"-\"\n", program_name);
}
