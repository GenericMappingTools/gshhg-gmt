/* Compatibility with GMT 6 includes */

extern void GMT_free (void *addr);
extern void *GMT_memory (void *prev_addr, size_t nelem, size_t size, char *progname);

#define i_swap gmt_M_int_swap


