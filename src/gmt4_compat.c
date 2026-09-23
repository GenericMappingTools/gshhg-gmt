/*
 * gmt4_compat.c
 *
 * Implementation of the GMT4 -> GMT 6 compatibility layer used by the
 * GSHHG processing tools.  See gmt4_compat.h.
 */

#include "gmt4_compat.h"
#undef GMT	/* Here GMT must be the plain struct member name */

char *gshhg_program = "gshhg";

static void *gshhg_API = NULL;	/* The one GMT 6 session shared by all tools */

static void gshhg_atexit(void) {
	if (gshhg_API) GMT_Destroy_Session(gshhg_API);
	gshhg_API = NULL;
}

struct GMT_CTRL *gshhg_get_ctrl(void) {
	/* Create the GMT 6 session the first time it is needed */
	if (gshhg_API == NULL) {
		if ((gshhg_API = GMT_Create_Session(gshhg_program, GMT_PAD_DEFAULT, GMT_SESSION_NORMAL, NULL)) == NULL) {
			fprintf(stderr, "%s: Failure to create a GMT 6 session\n", gshhg_program);
			exit(EXIT_FAILURE);
		}
		atexit(gshhg_atexit);
	}
	return (((struct GMTAPI_CTRL *)gshhg_API)->GMT);
}

int gshhg_begin(int argc, char **argv) {
	/* GMT4's GMT_begin also parsed common options; the GSHHG tools do not use them */
	char *p;
	if (argc > 0 && argv && argv[0]) {	/* Use the basename of the executable as program name */
		gshhg_program = argv[0];
		if ((p = strrchr(gshhg_program, '/'))) gshhg_program = p + 1;
		if ((p = strrchr(gshhg_program, '\\'))) gshhg_program = p + 1;
	}
	(void)gshhg_get_ctrl();
	return (argc);
}

void gshhg_end(void) {
	gshhg_atexit();
}

void *gshhg_memory(void *prev_addr, size_t nelem, size_t size, const char *progname) {
	/* Same semantics as GMT4's GMT_memory: realloc if prev_addr, else zeroed calloc */
	void *tmp;
	if (nelem == 0) {	/* Take care of n = 0 */
		if (prev_addr) free(prev_addr);
		return (NULL);
	}
	tmp = (prev_addr) ? realloc(prev_addr, nelem * size) : calloc(nelem, size);
	if (tmp == NULL) {
		fprintf(stderr, "%s: Could not allocate memory [%.0f bytes]\n", (progname) ? progname : gshhg_program, (double)(nelem * size));
		exit(EXIT_FAILURE);
	}
	return (tmp);
}

void gshhg_free(void *addr) {
	if (addr) free(addr);
}

void gshhg_err_fail(int err, const char *file) {
	if (err == 0) return;
	if (file && file[0])
		fprintf(stderr, "%s: Error %d for %s\n", gshhg_program, err, file);
	else
		fprintf(stderr, "%s: Error %d\n", gshhg_program, err);
	exit(EXIT_FAILURE);
}

FILE *gshhg_fopen(const char *file, const char *mode) {
	FILE *fp = NULL;
	if (file == NULL || (fp = fopen(file, mode)) == NULL) {
		fprintf(stderr, "%s: Cannot open file %s\n", gshhg_program, (file) ? file : "<none>");
		exit(EXIT_FAILURE);
	}
	return (fp);
}

void gshhg_write_float_grid(const char *file, float *z, unsigned int nx, unsigned int ny,
	double wesn[], double inc, const char *title, const char *z_units) {
	/* Write an unpadded, gridline-registered float array (row 0 = north) as a GMT grid */
	unsigned int row, col;
	uint64_t ij;
	double incs[2];
	struct GMT_GRID *G = NULL;
	void *API = gshhg_get_ctrl()->parent;

	incs[0] = incs[1] = inc;
	if ((G = GMT_Create_Data(API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, wesn, incs,
	                         GMT_GRID_NODE_REG, 0, NULL)) == NULL) {
		fprintf(stderr, "%s: Unable to create grid for %s\n", gshhg_program, file);
		exit(EXIT_FAILURE);
	}
	if (G->header->n_columns != nx || G->header->n_rows != ny) {
		fprintf(stderr, "%s: Grid dimension mismatch for %s (%u x %u vs %u x %u)\n", gshhg_program, file,
		        G->header->n_columns, G->header->n_rows, nx, ny);
		exit(EXIT_FAILURE);
	}
	for (row = 0, ij = 0; row < ny; row++)
		for (col = 0; col < nx; col++, ij++)
			G->data[gmt_M_ijp(G->header, row, col)] = (gmt_grdfloat)z[ij];
	strncpy(G->header->x_units, "Longitude", GMT_GRID_UNIT_LEN80-1);
	strncpy(G->header->y_units, "Latitude", GMT_GRID_UNIT_LEN80-1);
	if (z_units) strncpy(G->header->z_units, z_units, GMT_GRID_UNIT_LEN80-1);
	if (title) strncpy(G->header->title, title, GMT_GRID_TITLE_LEN80-1);
	if (GMT_Write_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, G) != GMT_NOERROR) {
		fprintf(stderr, "%s: Unable to write grid %s\n", gshhg_program, file);
		exit(EXIT_FAILURE);
	}
	GMT_Destroy_Data(API, &G);
}

float *gshhg_read_float_grid(const char *file, unsigned int *nx, unsigned int *ny) {
	/* Read a GMT grid into a new unpadded float array (row 0 = north) */
	unsigned int row, col;
	uint64_t ij;
	float *z = NULL;
	struct GMT_GRID *G = NULL;
	void *API = gshhg_get_ctrl()->parent;

	if ((G = GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, NULL)) == NULL) {
		fprintf(stderr, "%s: Unable to read grid %s\n", gshhg_program, file);
		exit(EXIT_FAILURE);
	}
	*nx = G->header->n_columns;
	*ny = G->header->n_rows;
	z = gshhg_memory(NULL, (size_t)(*nx) * (size_t)(*ny), sizeof(float), gshhg_program);
	for (row = 0, ij = 0; row < *ny; row++)
		for (col = 0; col < *nx; col++, ij++)
			z[ij] = (float)G->data[gmt_M_ijp(G->header, row, col)];
	GMT_Destroy_Data(API, &G);
	return (z);
}
