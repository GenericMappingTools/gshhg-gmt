/*
 * polygon_in_pol reads a binary file with polygons and a single ASCII
 * polygon and reports which polygons are inside the single polygon.
 */
#include "wvs.h"

int main (int argc, char **argv) {
	int n = 0, intest, ix[100], iy[100], level = 0;
	double x, y;
	char line[BUFSIZ];
	FILE *fp = NULL, *fp_in = NULL;
	struct GMT3_POLY h;
	struct LONGPAIR p;
	
	if (!(argc == 3 || argc == 4)) {
		fprintf (stderr, "polygon_in_pol - Report polygon IDs inside given polygon [of given level]\n");
		fprintf (stderr, "usage: polygon_in_pol final_polygons.b thepolygon.txt [level]\n");
		exit (EXIT_FAILURE);
	}

	if ((fp = fopen (argv[2], "r")) == NULL) {
		fprintf (stderr, "polygon_in_pol: Cannot open %s\n", argv[2]);
		exit (EXIT_FAILURE);
	}
	if (argc == 4) level = atoi (argv[3]);
	while (fgets (line, BUFSIZ, fp)) {
		if (line[0] == '#') continue;
		sscanf (line, "%lf %lf", &x, &y);
		ix[n] = lrint (x * MILL);
		iy[n] = lrint (y * MILL);
		n++;
	}
	fclose (fp);
	if (!(ix[0] == ix[n-1] && iy[0] == iy[n-1])) {	/* Close explicitly */
		ix[n] = ix[0];	iy[n] = iy[0];	n++;
	}
	fprintf (stderr,"polygon_in_pol:  Read %d crude polygon points.\n", n);
		
	fp_in = fopen (argv[1], "r");
	
	while (pol_readheader (&h, fp_in) == 1) {
		
		if (pol_fread (&p, 1, fp_in) != 1) {
			fprintf(stderr,"polygon_in_pol:  ERROR  reading file.\n");
			exit(-1);
		}
		if ((h.greenwich & 1) && p.x > h.datelon) p.x -= M360;
		
		fseek (fp_in, (long)((h.n-1)*sizeof(struct LONGPAIR)), 1);
		intest = non_zero_winding2 (p.x, p.y, ix, iy, n);
		if (intest == 2 && (level == 0 || level == h.level))
			printf ("%d\n", h.id);
		else if (intest == 1)
			fprintf (stderr, "%d is exactly on the edge - fix your input polygon?\n", h.id);
	}	
	fclose (fp_in);

	exit (EXIT_SUCCESS);
}	
