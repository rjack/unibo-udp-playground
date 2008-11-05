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
	int i, j;
	struct sockaddr_in *addr[2];

	if (argc != 3) {
		print_usage (stderr, argv[0]);
		exit (EXIT_FAILURE);
	}

	for (i = 0, j = i + 1; i < argc - 1; i++, j++) {
		addr[i] = new_addr_from_str (argv[j]);
		if (addr[i] == NULL) {
			fprintf (stderr, "%s non e' una coppia "
			                 "indirizzo:porta valida.\n",
			                 argv[j]);
			exit (EXIT_FAILURE);
		}
	}

	return 0;
}


/****************************************************************************
			       Funzioni private
****************************************************************************/

static struct sockaddr_in *
new_addr_from_str (const char *string)
{
	int ok = 0;
	char *ip_str = NULL;
	char *port_str = NULL;
	struct sockaddr_in *addr = NULL;

	/* ip_str duplica string */
	ip_str = malloc ((strlen (string) + 1) * sizeof(*string));
	if (ip_str == NULL)
		goto error;
	if (!strcpy (ip_str, string))
		goto error;

	/* port_str punta in mezzo a ip_str */
	port_str = strchr (ip_str, ':');
	if (port_str == NULL)
		goto error;
	*port_str = '\0';
	port_str++;

	/* allocazione sockaddr_in */
	addr = (struct sockaddr_in *) calloc (1, sizeof(*addr));
	if (addr == NULL)
		goto error;

	/* parsing ip */
	ok = inet_pton (AF_INET, ip_str, &(addr->sin_addr));
	if (!ok)
		goto error;

	return addr;

error:
	if (addr != NULL)
		free (addr);
	if (ip_str != NULL)
		free (ip_str);
	return NULL;
}


static void
print_usage (FILE *out, const char *program_name)
{
	assert (out != NULL);
	assert (program_name != NULL);

	fprintf (out, "uso:\n");
	fprintf (out, "  %s INDIRIZZO_LOCALE INDIRIZZO_REMOTO\n",
			program_name);
	fprintf (out, "dove gli indirizzi hanno forma ip:porta\n");
	fprintf (out, "\n");
}
