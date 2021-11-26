/*
 * polygon_xover checks for propoer closure and crossings
 * within polygons
 *
 */

#include "wvs.h"

struct POLYGON {
	struct GMT3_POLY h;
	double *lon;
	double *lat;
} P[N_POLY];

int main (int argc, char **argv)
{
	FILE	*fp;
	int i, n_id, id1, id2, nx, nx_tot, verbose, cnt, full, in, eur_id = 0, cont_no, first = 0, N[N_CONTINENTS][2];
	double x_shift = 0.0, *X, *Y, *CX[N_CONTINENTS][2], *CY[N_CONTINENTS][2];
	struct GMT_XSEGMENT *ylist1, *ylist2;
	struct GMT_XOVER XC;
	struct LONGPAIR p;

	if (argc < 2 || argc > 4) {
		fprintf(stderr,"usage:  polygon_xover wvs_polygons.b [<firstpol>] [-V] > report.lis\n");
		exit(-1);
	}
	for (i = 2; i < argc; i++) {
		if (!strncmp (argv[i], "-V", 2))
			verbose = 1;
		else if (isdigit (argv[i][0]))
			first = atoi (argv[i]);
	}

	/* Open file and read everything into memory */
	
	if ((fp = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr,"polygon_xover: Could not open file %s\n", argv[1]);
		exit(-1);
	}
	argc = GMT_begin (argc, argv);
#ifdef DEBUG
	GMT_memtrack_off (GMT_mem_keeper);
#endif
	if (verbose) fprintf (stderr, "start at polygon # %d\n", first);
	n_id = 0;
	while (pol_readheader (&P[n_id].h, fp) == 1) {
		P[n_id].lon = (double *) GMT_memory (VNULL, P[n_id].h.n, sizeof (double), "polygon_xover");
		P[n_id].lat = (double *) GMT_memory (VNULL, P[n_id].h.n, sizeof (double), "polygon_xover");
		if (P[n_id].h.east < 0.0 && P[n_id].h.west< 0.0) {
			fprintf (stderr, "Pol %d has negative w/e values.  Run polygon_fixnegwesn\n", P[n_id].h.id);
			exit(-1);
		}
		cont_no = P[n_id].h.continent;	/* Continent number 1-6 (0 if not a continent) */
		if (cont_no == EURASIA) eur_id = n_id;	/* blob with Eurasia */

		for (i = 0; i < P[n_id].h.n; i++) {
			if (pol_fread (&p, 1, fp) != 1) {
				fprintf(stderr,"polygon_xover:  ERROR  reading file.\n");
				exit(-1);
			}
			if ((P[n_id].h.greenwich & 1) && p.x > P[n_id].h.datelon) p.x -= M360;
			P[n_id].lon[i] = p.x * I_MILL;
			P[n_id].lat[i] = p.y * I_MILL;
		}
		/* lon,lat is now -180/+270 and continuous */
		if (Pol_Is_Antarctica (cont_no)) {	/* Special r,theta conversion */
			for (i = 0; i < P[n_id].h.n; i++) xy2rtheta (&P[n_id].lon[i], &P[n_id].lat[i]);
		}

		n_id++;
	}
	fclose(fp);
	if (verbose) fprintf (stderr, "polygon_xover: Found %d polygons\n", n_id);
	
	crude_init (CX, CY, N);
	
	/* Now do double do-loop to find all xovers */
	
	full = (P[eur_id].h.n > 1000000);	/* Only the full resolution has more than 1 mill points for EUR-AFR polygon */
	
	nx_tot = 0;
	for (id1 = first; id1 < n_id; id1++) {
#ifdef TEST
		if (P[id1].h.id != 3) continue;
#endif
		cont_no = P[id1].h.continent;	/* Continent number 1-6 (0 if not a continent) */
		
		gmt_init_track (P[id1].lat, P[id1].h.n, &ylist1);
			
		for (id2 = MAX (N_CONTINENTS, id1 + 1); id2 < n_id; id2++) {	/* Dont start earlier than N_CONTINENTS since no point comparing continents */
#ifdef TEST
			if (P[id2].h.id != 187083) continue;
#endif
			if ((P[id2].h.source + P[id1].h.source) == 5) continue;	/* Don't compare icelines and grounding lines */
			if (Pol_Is_Antarctica (cont_no)) {	/* Must compare to id1 polygon (Antarctica) using r,theta */
				if (P[id2].h.south > P[id1].h.north) continue;	/* Too far north to matter */
				X = (double *) GMT_memory (VNULL, P[id2].h.n, sizeof (double), "polygon_xover");
				Y = (double *) GMT_memory (VNULL, P[id2].h.n, sizeof (double), "polygon_xover");
				for (i = 0; i < P[id2].h.n; i++) {	/* Special r,theta conversion */
					X[i] = P[id2].lon[i];	Y[i] = P[id2].lat[i];
					xy2rtheta (&X[i], &Y[i]);
				}
			}
			else {	/* Regular Cartesian testing */
		
				if (nothing_in_common (&P[id1].h, &P[id2].h, &x_shift)) continue;	/* No area in common */

				/* GMT_non_zero_winding returns 2 if inside, 1 if on line, and 0 if outside */
			
				if (full && cont_no) {	/* Use coarse outlines to determine if id2 is inside/outside a continent */
					cnt = cont_no - 1;
					for (i = in = 0; i < P[id2].h.n; i++) in += GMT_non_zero_winding (P[id2].lon[i] + x_shift, P[id2].lat[i], CX[cnt][OUTSIDE], CY[cnt][OUTSIDE], N[cnt][OUTSIDE]);
					if (in == 0) continue;	/* Polygon id2 completely outside the "outside" polygon */
					for (i = in = 0; i < P[id2].h.n; i++) in += GMT_non_zero_winding (P[id2].lon[i] + x_shift, P[id2].lat[i], CX[cnt][INSIDE], CY[cnt][INSIDE], N[cnt][INSIDE]);
					if (in == (2 * P[id2].h.n)) continue;	/* Polygon id2 completely inside the "inside" polygon 1 */
				}
				/* Crude test were inconclusive; now do full test with actual polygons */
				X = P[id2].lon;
				Y = P[id2].lat;
				if (!GMT_IS_ZERO (x_shift)) for (i = 0; i < P[id2].h.n; i++) P[id2].lon[i] += x_shift;
			}
			
			/* Get here when no cheap determination worked and we must do full crossover calculation */
			
			if (verbose) fprintf (stderr, "polygon_xover: %6d vs %6d [T = %6d]\r", P[id1].h.id, P[id2].h.id, nx_tot);
			
			gmt_init_track (Y, P[id2].h.n, &ylist2);

			nx = gmt_crossover (P[id1].lon, P[id1].lat, NULL, ylist1, P[id1].h.n, X, Y, NULL, ylist2, P[id2].h.n, FALSE, TRUE, &XC);
			GMT_free ((void *)ylist2);
			if (Pol_Is_Antarctica (cont_no)) {	/* Undo projection for crossover results */
				for (i = 0; i < nx; i++) rtheta2xy (&XC.x[i], &XC.y[i]);
				GMT_free ((void *)X);
				GMT_free ((void *)Y);
			}
			else if (!GMT_IS_ZERO (x_shift)) {	/* Undo longitude adjustment */
				for (i = 0; i < P[id2].h.n; i++) P[id2].lon[i] -= x_shift;
			}
			if (nx) {
				for (i = 0; i < nx; i++) printf ("%d\t%d\t%d\t%d\t%f\t%f\n", P[id1].h.id, P[id2].h.id, (int)floor(XC.xnode[0][i]), (int)floor(XC.xnode[1][i]), XC.x[i], XC.y[i]);
				GMT_x_free (&XC);
			}
			nx_tot += nx;
		}
		GMT_free ((void *)ylist1);
		GMT_free (P[id1].lon);
		GMT_free (P[id1].lat);
	}
	crude_free (CX, CY, N);
	
	if (verbose) fprintf (stderr, "\npolygon_xover: %d external crossovers\n", nx_tot);

#ifdef DEBUG
	GMT_memtrack_on (GMT_mem_keeper);
#endif
	GMT_end (argc, argv);

	exit (EXIT_SUCCESS);
}
