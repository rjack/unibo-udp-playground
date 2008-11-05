/*
 * pingo_pongo.c - ripasso UDP
 *
 * uso:
 * pingo_pongo -l indirizzo:porta -r indirizzo:porta
 */

#include <assert.h>


/****************************************************************************
			  Prototipi funzioni private
****************************************************************************/


/****************************************************************************
			      Funzioni pubbliche
****************************************************************************/

int
main (const int argc, const char **argv)
{

	return 0;
}


/****************************************************************************
			       Funzioni private
****************************************************************************/

static void
print_usage (FILE *out, const char *program_name)
{
	assert (out != NULL);
	assert (program_name != NULL);

	fprintf (out, "uso:\n");
	fprintf (out, "  %s -l indirizzo:porta -r indirizzo:porta\n",
			program_name);
}
