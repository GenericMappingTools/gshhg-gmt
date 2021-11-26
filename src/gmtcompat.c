#include "wvs.h"

/* Functions needed for this GMT4-ear codes to link with GMT6 */

void GMT_free (void *addr)
{
	if (!addr) return;	/* Do not try to free a NULL pointer! */
	free (addr);
}

void *GMT_memory (void *prev_addr, size_t nelem, size_t size, char *progname)
{
	/* Multi-functional memory allocation subroutine.
	   If prev_addr is NULL, allocate new memory of nelem elements of size bytes.
	   	Ignore when nelem == 0.
	   If prev_addr exists, reallocate the memory to a larger or smaller chunk of nelem elements of size bytes.
	   	When nelem = 0, free the memory.
	*/

	void *tmp = NULL;
	static char *m_unit[4] = {"bytes", "kb", "Mb", "Gb"};
	double mem;
	size_t k;

	if (nelem < 0) {	/* Probably 32-bit overflow */
		fprintf (stderr, "GMT Fatal Error: %s requesting negative n_items (%ld) - exceeding 32-bit counting?\n", progname, nelem);
		exit (EXIT_FAILURE);
	}

	if (prev_addr) {
		if (nelem == 0) { /* Take care of n = 0 */
			GMT_free ((void *) prev_addr);
			return (VNULL);
		}
		if ((tmp = realloc ((void *) prev_addr, (size_t)(nelem * size))) == VNULL) {
			mem = (double)(nelem * size);
			k = 0;
			while (mem >= 1024.0 && k < 3) mem /= 1024.0, k++;
			fprintf (stderr, "GMT Fatal Error: %s could not reallocate memory [%.2f %s, n_items = %ld]\n", progname, mem, m_unit[k], nelem);
			exit (EXIT_FAILURE);
		}
	}
	else {
		if (nelem == 0) return (VNULL); /* Take care of n = 0 */
		if ((tmp = calloc ((size_t)nelem, size)) == VNULL) {
			mem = (double)(nelem * size);
			k = 0;
			while (mem >= 1024.0 && k < 3) mem /= 1024.0, k++;
			fprintf (stderr, "GMT Fatal Error: %s could not allocate memory [%.2f %s, n_items = %ld]\n", progname, mem, m_unit[k], nelem);
			exit (EXIT_FAILURE);
		}
	}
	return (tmp);
}
