/*
 * Reads the master ascii GSHHS or WDBII files and creates the corresponding binary files.
 *
 * GSHHS data are stored in separate dirs called res_? (? = f,h,i,l,c), with one file for each
 *	level (1-4) except that Africa, Eurasia, NAmerica, SAmerica, Australia, and Antactica are stored
 *	in 6 separate continent files for ease of manipulation. Level 2 features (lakes) contains
 *	both lakes and "river-lakes" (marked as having negative area).
 * WDBII data are stored in a separate dir called WDBII.  There is one file per level.  For
 *	borders there are 3 levels; for rivers there are 10
 *
 * Output is either GSHHS_<res>_polygons.b, WDBII_rivers.b, or WDBII_borders.b
 *
 * Paul Wessel, Updated July 2011 to also flag WDBII river-lakes.
 *
 */

#include "wvs.h"

#define WDBII_MAX	13
#define NO_PARENT	-1

#define IS_RIVER	0
#define IS_BORDER	1
#define IS_COAST	2

 struct POLY {
	struct GMT3_POLY h;
	struct LONGPAIR *p;
} P[N_POLY];

int main (int argc, char **argv)
{
	FILE *fp_out, *fp;
	int k, id, n_id = 0, rec, res = 0, level, unit, ustart, n_alloc, max_level, ns = 0, cont_no, feature, n_skip, verbose = FALSE;
	int cont_id[N_CONTINENTS] = {1, 2, 3, 4, 5, 6, 7};
	char file[BUFSIZ], line[BUFSIZ], *resolution[5] = {"full", "high", "intermediate", "low", "crude"};
	char *continents[N_CONTINENTS] = {"Eurasia", "Africa", "NAmerica", "SAmerica", "Antarctica", "Antarctica_G", "Australia"};
	char west[32], east[32], south[32], north[32], parent[8], ancestor[8];
	char *prefix[3] = {"Rivers", "Borders", "Coastlines"};
	char *dir[3] = {"rivers", "borders", "GSHHS"};
	double x, y;
	struct GMT3_POLY H;
       
	if (argc == 1) {
		fprintf (stderr, "ascii_to_binary - Converting the CVS ascii GSHHS or WDBII files to a single binary polygon file\n");
		fprintf (stderr, "usage:  ascii_to_binary <feature> [<res>] [-V]\n");
		fprintf (stderr, "	<feature> is either C (coastlines), R (rivers) or B (borders)\n");
		fprintf (stderr, "	If C, then <res> must be one of f, h, i, l, c\n");
		fprintf (stderr, "	-V will report on progress\n");
		exit (EXIT_FAILURE);
	}

	if (argv[1][0] == 'R' || argv[1][0] == 'r')
		feature = IS_RIVER;
	else if (argv[1][0] == 'B' || argv[1][0] == 'b')
		feature = IS_BORDER;
	else if (argv[1][0] == 'C' || argv[1][0] == 'c')
		feature = IS_COAST;
	else {
		fprintf(stderr,"ascii_to_binary: ERROR: Feature must be C, R or B\n");
		exit (EXIT_FAILURE);
	}
	memset ((void *)P, 0, N_POLY * sizeof (struct POLY));
	memset ((void *)&H, 0, sizeof (struct GMT3_POLY));
	
	if (feature == 2) {
		switch (argv[2][0]) {
			case 'f':
				res = 0;
				break;
			case 'h':
				res = 1;
				break;
			case 'i':
				res = 2;
				break;
			case 'l':
				res = 3;
				break;
			case 'c':
				res = 4;
				break;
			default:
				fprintf(stderr,"ascii_to_binary: ERROR - Must give valid resolution [f, h, i, l, or c]\n");
				exit (EXIT_FAILURE);
				break;
		}
		sprintf (file, "../GSHHS/res_%c/GSHHS_%c_polygons.b", resolution[res][0], resolution[res][0]);
		max_level = GMT_MAX_GSHHS_LEVEL + N_CONTINENTS;
	}
	else {
		sprintf (file, "../WDBII/%s/WDBII_%s_segments.b", dir[feature], prefix[feature]);
		max_level = (feature) ? GMT_N_BLEVELS : WDBII_MAX;
	}
			
	if ((fp_out = fopen (file, "wb")) == NULL) {
		fprintf (stderr, "ascii_to_binary: ERROR - unable to create file %s\n", file);
		exit (EXIT_FAILURE);
	}
	if (!strcmp (argv[argc-1], "-V")) verbose = TRUE;
		
	/* Open input files */
	
	ustart = (feature == 0) ? -1 : 0;	/* So we can start at level 0 for rivers */
	for (unit = ustart; unit < max_level; unit++) {	/*  Process all the CVS masterfiles for this resolution */
		level = unit + 1;
		cont_no = 0;	/* Not a continent, unless reset below */
		if (feature == IS_COAST) {
			if (unit < GMT_MAX_GSHHS_LEVEL) {	/* Each of levels 1-4 */
				sprintf (file, "../GSHHS/res_%c/GSHHS_%c_Level_%d.txt", resolution[res][0], resolution[res][0], level);
				if (verbose) fprintf (stderr, "ascii_to_binary: Process level %d polygons\n", level);
			}
			else {	/* The 6 continent files (2 for Antarctica) */
				level = 1;
				cont_no = cont_id[unit - GMT_MAX_GSHHS_LEVEL];
				sprintf (file, "../GSHHS/res_%c/GSHHS_%c_%s.txt", resolution[res][0], resolution[res][0], continents[unit-GMT_MAX_GSHHS_LEVEL]);
				if (verbose) fprintf (stderr, "ascii_to_binary: Process continent %s polygon, cont_no = %d\n", continents[unit-GMT_MAX_GSHHS_LEVEL], cont_no);
			}
		}
		else {	/* Each of levels 0-13 (rivers) or 1-3 (borders) */
			sprintf (file, "../WDBII/%s/WDBII_%s_Level_%2.2d.txt", dir[feature], prefix[feature], level);
		}
		if (feature == IS_RIVER && (level == 5 || level == 9 || level == 12)) continue;	/* These river features do not exist */
		
		if ((fp = fopen (file, "r")) == NULL) {
			fprintf (stderr, "ascii_to_binary: ERROR - unable to read file %s\n", file);
			exit (EXIT_FAILURE);
		}
		if (verbose) fprintf (stderr, "ascii_to_binary: Reading file %s\n", file);
		rec = 1;
		while (fgets (line, BUFSIZ, fp) && line[0] == '#') rec++;	/* Skip the headers */
		while (!feof(fp) && line[0] == '>') {	/* Another segment to process */
			/* Decode the segment header */
			if (feature == IS_COAST) {
			ns = sscanf (&line[7], "%d %*s %*s %d %*s %*s %d %*s %*s %d %*s %*s %d %*s %*s %d %*s %*s %[^/]/%[^/]/%[^/]/%s %*s %*s %lf %*s %*s %lf %*s %*s %s %*s %*s %s\n",
				&H.id, &H.n, &H.greenwich, &H.level, &H.datelon, &H.source, west, east, south, north, &H.area, &H.area_res, parent, ancestor);
				H.datelon *= MILL;
				H.west = atof (west);
				H.east = atof (east);
				H.south = atof (south);
				H.north = atof (north);
			}
			else {	/* Borders and river lines */
				/* > Id = 0 N = 15401 G = 0 L = 0 E = 180 S = 0 R = 286.991000/307.294000/-4.479720/-0.240556 A = 0 */
				ns = sscanf (&line[7], "%d %*s %*s %d %*s %*s %d %*s %*s %d %*s %*s %d %*s %*s %d %*s %*s %[^/]/%[^/]/%[^/]/%s %*s %*s %lf\n",
					&H.id, &H.n, &H.greenwich, &H.level, &H.datelon, &H.source, west, east, south, north, &H.area);
					H.datelon *= MILL;
					H.west = atof (west);
					H.east = atof (east);
					H.south = atof (south);
					H.north = atof (north);
					H.area_res = 0.0;
			}
			/* Basic range checking */
			if (H.id < 0 || (H.id > N_POLY && H.id < 300000)) {
				fprintf (stderr, "ascii_to_binary: ID out of range at line %d - fix this in the ASCII Master File %s\n", rec, file);
				exit (EXIT_FAILURE);
			}
			if (H.n < 0 || H.n > N_LONGEST) {
				fprintf (stderr, "ascii_to_binary: N out of range at line %d - fix this in the ASCII Master File %s\n", rec, file);
				exit (EXIT_FAILURE);
			}
			if (H.greenwich < 0 || H.greenwich > 3) {
				fprintf (stderr, "ascii_to_binary: G out of range at line %d - fix this in the ASCII Master File %s\n", rec, file);
				exit (EXIT_FAILURE);
			}
			if (H.level != level) {
				fprintf (stderr, "ascii_to_binary: L out of range at line %d - fix this in the ASCII Master File %s\n", rec, file);
				exit (EXIT_FAILURE);
			}
			if (!(H.datelon == 270000000 || H.datelon == 180000000)) {
				fprintf (stderr, "ascii_to_binary: E out of range at line %d - fix this in the ASCII Master File %s\n", rec, file);
				exit (EXIT_FAILURE);
			}
			if (!(H.source == 0 || H.source == 1) && !(H.north < -60.0 && (H.source == 2 || H.source == 3))) {
				fprintf (stderr, "ascii_to_binary: S out of range at line %d - fix this in the ASCII Master File %s\n", rec, file);
				exit (EXIT_FAILURE);
			}
			if (H.west > H.east  || H.south > H.north || H.west < -30.0 || H.east > 360.0 || H.south < -90.0 || H.north > 90.0) {
				fprintf (stderr, "ascii_to_binary: R out of range at line %d - fix this in the ASCII Master File %s\n", rec, file);
				exit (EXIT_FAILURE);
			}
			if (H.area < 0.0 && H.level != 2) {	/* Only some fat rivers turned to lakes are flagged with negative area */
				fprintf (stderr, "ascii_to_binary: A out of range at line %d - fix this in the ASCII Master File %s\n", rec, file);
				/* exit (EXIT_FAILURE); */
			}
			H.parent = NO_PARENT;	/* no parent */
			H.ancestor = NO_PARENT;	/* no ancestor */
			if (feature == IS_COAST) {
				H.river = (H.area < 0.0 && H.level == 2);	/* River-lake */
				H.continent = cont_no;				/* 1-6, or 0 if not a continent */
				if (H.level == 1) {	/* No parent possible so should have P = - */
					if (parent[0] != '-') {
						fprintf (stderr, "ascii_to_binary: P out of range at line %d - fix this in the ASCII Master File %s\n", rec, file);
						fprintf (stderr, "%s [%d]\n", line, ns);
						/* exit (EXIT_FAILURE); */
					}
				}
				else if (parent[0] != '-')
					H.parent = atoi (parent);
				if (res == 0) {	/* No ancestor possible so should have F = - */
					if (ancestor[0] != '-') {
						fprintf (stderr, "ascii_to_binary: F (1) out of range at line %d - fix this in the ASCII Master File %s\n", rec, file);
						fprintf (stderr, "%s [%d]\n", line, ns);
						/* exit (EXIT_FAILURE); */
					}
				}
				else if (ancestor[0] != '-') {
					H.ancestor = atoi (ancestor);
					if (H.ancestor < 0) {
						fprintf (stderr, "ascii_to_binary: F (2) out of range at line %d - fix this in the ASCII Master File %s\n", rec, file);
						fprintf (stderr, "%s [%d]\n", line, ns);
					}
				}
				else {	/* Missing ancestor */
					fprintf (stderr, "ascii_to_binary: F (3) out of range at line %d - fix this in the ASCII Master File %s\n", rec, file);
					fprintf (stderr, "%s [%d]\n", line, ns);
				}
			}
			else if (feature == IS_RIVER) {
				H.river = (H.level == 0);	/* The level 0 rivers are river-lakes; flag for WDBII lines as such */	
			}
			H.area = fabs (H.area);		/* Area is a positive quantity with signed stored in H.river until polygon_to_bins adds back in the sign */
			id = H.id;
			if (P[id].h.n) {	/* Error, already set */
				fprintf (stderr, "ascii_to_binary: ID = %d used more than once at line %d - fix this in the ASCII Master File %s\n", id, rec, file);
				exit (EXIT_FAILURE);
			}
			if (id > n_id) n_id = id;
			P[id].h = H;
			n_alloc = H.n;
			P[id].p = (struct LONGPAIR *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (struct LONGPAIR), "ascii_to_binary");

			k = 0;
			while (fgets (line, BUFSIZ, fp) && line[0] != '>') { /* Process data records */
				rec++;
				if (k == n_alloc) {
					fprintf (stderr, "ascii_to_binary: Warning - Polygon ID = %d had more points that stated in the multisegment header (%s)\n", id, file);
					n_alloc += GMT_CHUNK;
					P[id].p = (struct LONGPAIR *) GMT_memory ((void *)P[id].p, (size_t)n_alloc, sizeof (struct LONGPAIR), "ascii_to_binary");
				}
				sscanf (line, "%lf %lf", &x, &y);
				if (x < -20.0 || x > 360.0 || y < -90.0 || y > 90.0) {
					fprintf (stderr, "ascii_to_binary: (x,y) out of range at line %d - fix this in the ASCII Master File %s\n", rec, file);
					exit (EXIT_FAILURE);
				}
				P[id].p[k].x = lrint (x * 1e6);
				P[id].p[k].y = lrint (y * 1e6);
				if (P[id].p[k].x < 0) P[id].p[k].x += M360;
				k++;
			}
			rec++;
			if (P[id].h.n != k) fprintf (stderr, "ascii_to_binary: Warning: Polygon %d had %d but says %d points - fix this in the ASCII Master File %s\n", id, k, P[id].h.n, file);
			
			P[id].h.n = k;
		}
		fclose (fp);
	}
	
	/* Writing binary file */
	
	for (id = k = n_skip = 0; id <= n_id; id++) {
		if (P[id].h.n == 0) continue;	/* Skip missing polygons */
		if (P[id].h.n == 2 && P[id].p[0].x == P[id].p[1].x && P[id].p[0].y == P[id].p[1].y) {
			n_skip++;
			continue;
		}
		if (pol_writeheader (&P[id].h, fp_out) != 1) {
			fprintf (stderr, "polygon_update: Write error!\n");
			exit (EXIT_FAILURE);
		}
		if (pol_fwrite (P[id].p, P[id].h.n, fp_out) != P[id].h.n) {
			fprintf (stderr, "polygon_update: Write error!\n");
			exit (EXIT_FAILURE);
		}
		GMT_free ((void *)P[id].p);
		k++;
	}
	if (verbose) {
		if (feature == IS_COAST)
			fprintf (stderr, "ascii_to_binary: Resolution %s: %d polygons\n", resolution[res], k);
		else {
			fprintf (stderr, "ascii_to_binary: %d segments\n", k);
			if (n_skip) fprintf (stderr, "ascii_to_binary: %d zero-length segments skipped\n", n_skip);
		}
	}
	
	fclose (fp_out);

	exit (EXIT_SUCCESS);
}
