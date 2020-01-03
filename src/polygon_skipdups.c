/*
 * Reads polygon file and writes out same except for duplicate points.
 */
#include "wvs.h"
struct LONGPAIR p_in[N_LONGEST], p_out[N_LONGEST];

int main (int argc, char **argv) {
	int i, j, last_x = 0, last_y = 0;
	FILE *fp;
	struct GMT3_POLY h;
	
	if (argc != 2) {
		fprintf (stderr, "usage: polygon_skipdups new.b > new.b\n");
		exit (EXIT_FAILURE);
	}

	/* Read new file */
	if ((fp = fopen (argv[1], "r")) == NULL) {
		fprintf (stderr, "polygon_skipdups: Cannot open file %s\n", argv[1]);
		exit (EXIT_FAILURE);
	}
	while (pol_readheader (&h, fp) == 1) {
		if (pol_fread (p_in, h.n, fp) != h.n) {
			fprintf(stderr,"polygon_skipdups:  ERROR  reading %d points from file %s.\n", h.n, argv[1]);
			exit (EXIT_FAILURE);
		}
		for (i = j = 0; i < h.n; i++) {
			if (i > 0 && last_x == p_in[i].x && last_y == p_in[i].y && !(Pol_Is_Antarctica (h.continent) && (p_in[i].x == 0 || p_in[i].x == M360))) {
				printf ("%d\tduplicate point line %d\n", h.id, i);
			}
			else {
				p_out[j].x = p_in[i].x;
				p_out[j].y = p_in[i].y;
				j++;
			}
			last_x = p_in[i].x;
			last_y = p_in[i].y;
		}
		h.n = j;
		if (pol_writeheader (&h, stdout) != 1) {
			fprintf(stderr,"polygon_skipdups:  ERROR  writing polygon header to stdout.\n");
			exit (EXIT_FAILURE);
		}
		if (pol_fwrite (p_out, h.n, stdout) != h.n) {
			fprintf(stderr,"polygon_skipdups:  ERROR  writing %d points to stdout.\n", h.n);
			exit (EXIT_FAILURE);
		}
	}
	fclose (fp);

	exit (EXIT_SUCCESS);
}	
