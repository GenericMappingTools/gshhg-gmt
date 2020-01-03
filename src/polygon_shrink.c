/* 
 * polygon_shrink applies the Douglas-Peucker algorithm to simplify a line
 * segment given a tolerance.
 */

#include "wvs.h"

int main (int argc, char **argv)
{
	FILE	*fp_in, *fp_out;
	int	n_id, n_out, n, k, sign, verbose = FALSE, *x, *y, *index;
	int	n_tot_in, n_tot_out;
	double	redux, redux2, tolerance = 0.0;
	double *flon = NULL, *flat = NULL;
	struct	GMT3_POLY h;
	struct	LONGPAIR p;
        
	if (argc < 2 || !(argc == 4 || argc == 5)) {
		fprintf (stderr,"usage:  polygon_shrink final_polygons.b tolerance shrink_polygons.b [-v]\n");
		fprintf (stderr,"	tolerance is maximum mismatch in km\n");
		exit (-1);
	}

	tolerance = atof (argv[2]);
	fprintf (stderr,"polygon_shrink: Tolerance is %g km\n", tolerance);
	fp_in = fopen(argv[1], "r");
	fp_out = fopen(argv[3], "w");
	
	verbose = (argc == 5);
		
	/* Start shrink loop */
	
	n_id = n_out = n_tot_in = n_tot_out = 0;
	
	x = (int *) GMT_memory (VNULL, N_LONGEST, sizeof (int), "polygon_shrink");
	y = (int *) GMT_memory (VNULL, N_LONGEST, sizeof (int), "polygon_shrink");
	index = (int *) GMT_memory (CNULL, N_LONGEST, sizeof (int), "polygon_shrink");
	area_init ();
	flon = (double *) GMT_memory (NULL, N_LONGEST, sizeof(double), "polygon_sort");
	flat = (double *) GMT_memory (NULL, N_LONGEST, sizeof(double), "polygon_sort");
	
	while (pol_readheader (&h, fp_in) == 1) {
	
		if (verbose) fprintf (stderr, "Poly %6d", h.id);	
		
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"polygon_shrink:  ERROR  reading file.\n");
				exit(-1);
			}
			x[k] = p.x;
			y[k] = p.y;
		}
		n_tot_in += h.n;
		
		n = Douglas_Peucker_i (x, y, h.n-1, tolerance, index);	/* since last point repeats */
		
		if (n > 2) {
			index[n] = 0;
			n++;
			redux = 100.0 * (double) n / (double) h.n;
			h.id = n_out;
			h.n = n;
			/* Get float array */
			for (k = 0; k < n; k++) {
				flon[k] = x[index[k]] * 1.0e-6;
				flat[k] = y[index[k]] * 1.0e-6;
			}
			h.area = 1.0e-6 * area_size (flon, flat, h.n, &sign); /* in km^2 */
			if (pol_writeheader (&h, fp_out) != 1) {
				fprintf(stderr,"polygon_shrink:  ERROR  writing file.\n");
				exit(-1);
			}
			for (k = 0; k < n; k++) {
				p.x = x[index[k]];
				p.y = y[index[k]];
				if (pol_fwrite (&p, 1, fp_out) != 1) {
					fprintf(stderr,"polygon_shrink:  ERROR  writing file.\n");
					exit(-1);
				}
			}
			n_out++;
			n_tot_out += n;
		}
		else
			redux = 0.0;
		if (verbose) fprintf (stderr, "\t%.1f %% retained\n", redux);
		
		n_id++;
	}
		
	free ((void *)x);	
	free ((void *)y);	
	free ((void *)index);	
	GMT_free ((void *)flon);
	GMT_free ((void *)flat);
		
	fclose (fp_in);
	fclose (fp_out);

	redux = 100.0 * (double) n_tot_out / (double) n_tot_in;
	redux2 = 100.0 * (double) n_out / (double) n_id;
	printf ("polygon_shrink at %g: N = %.1f%% (%d of %d) P = %.1f%% (%d of %d)\n", tolerance, redux, n_tot_out, n_tot_in, redux2, n_out, n_id);

	exit (0);
}
