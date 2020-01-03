/*
 * Creates the master ascii GSHHS or WDBII files for the first time.  Once this is done properly
 * this program is only run when modifications are done via the binary files and we need to recreate
 * the ASCII master.  It is also kept for documentation purposes and to recover from the inevitable
 * disasters when files are losts, etc.
 *
 * GSHHS data are stored in separate dirs called GSHHS/res_? (? = f,h,i,l,c), with one file for each
 *	level (1-4) except that Africa, Eurasia, NAmerica, SAmerica, Australia, and Antactica are stored
 *	in 6 separate continent files for ease of manipulation.
 * WDBII data are stored in separate dirs called WDBII/{borders,rivers}.  There is one file per level.
 *	For borders there are 3 levels; for rivers there are 13.  Note that Level 5 are the old raw
 * double-lined rivers thta are kept for historical reasons; we explicitly skip this level when building
 * the binary files (see ascii_to_binary.c).
 * Must be run from the directory containing the GSHHS and WDBII sub-directories.
 *
 * Paul Wessel, May 2007, revised July 2011.
 *
 */

#include "wvs.h"

#define IS_RIVER	0
#define IS_BORDER	1
#define IS_COAST	2

struct CHECK {
	int pos;
	struct GMT3_POLY h;
} P[N_POLY];

int main (int argc, char **argv)
{
	FILE *fp_in, *fp[16];
	int i, k, id, n_id, pos, level, res = 0, unit, feature, cont_id, greenwich, max_level = 0, min_level = 100000, renumber = FALSE, verbose = FALSE, n[17];
	struct	LONGPAIR p;
	char file[80], *resolution[5] = {"full", "high", "intermediate", "low", "crude"}, parent[8], ancestor[8];
	char *continents[N_CONTINENTS] = {"Eurasia", "Africa", "NAmerica", "SAmerica", "Antarctica", "Antarctica_G", "Australia"};
	char *prefix[3] = {"Rivers", "Borders", "Coastlines"};
	char *dir[3] = {"rivers", "borders", "GSHHS"}, *ext = "txt";
	char *kind[3] = {"segment", "segment", "polygon"};
        
	if (argc == 1) {	/* We have a strict syntax for this program */
		fprintf (stderr, "binary_to_ascii - Converting a single binary feature file to the CVS ascii multi-file version.\n");
		fprintf (stderr, "usage:  binary_to_ascii binary_database.b <feature> [-V]\n");
		fprintf (stderr, "\t<feature> is either C (coastlines), R (rivers) or B (borders)\n");
		fprintf (stderr, "\t-V will report on progress. Add x to renumber features\n");
		exit (EXIT_FAILURE);
	}

	if (argv[2][0] == 'R' || argv[2][0] == 'r')
		feature = IS_RIVER;
	else if (argv[2][0] == 'B' || argv[2][0] == 'b')
		feature = IS_BORDER;
	else if (argv[2][0] == 'C' || argv[2][0] == 'c')
		feature = IS_COAST;
	else {
		fprintf(stderr,"binary_to_ascii: ERROR: Feature must be C, R or B\n");
		exit (EXIT_FAILURE);
	}
	if ((fp_in = fopen (argv[1], "rb")) == NULL) {
		fprintf(stderr,"binary_to_ascii: ERROR: Cannot open file %s\n", argv[1]);
		exit (EXIT_FAILURE);
	}
	
	if (argc == 4 && !strncmp (argv[3], "-V", 2)) verbose = TRUE;
	if (argc == 4 && !strcmp (argv[3], "-Vx")) renumber = TRUE;
		
	n_id = pos = 0;
	memset ((void *)n, 0, 16*sizeof (int));
	while (pol_readheader (&P[n_id].h, fp_in) == 1) {
		pos += sizeof (struct GMT3_POLY);
		P[n_id].pos = pos;
		fseek (fp_in, P[n_id].h.n * sizeof(struct LONGPAIR), SEEK_CUR);
		pos += P[n_id].h.n * sizeof(struct LONGPAIR);
		if (P[n_id].h.level > max_level) max_level = P[n_id].h.level;
		if (P[n_id].h.level < min_level) min_level = P[n_id].h.level;
		n[P[n_id].h.level]++;
		n_id++;
	}
	if (verbose) fprintf (stderr, "binary_to_ascii: min/max level = %d/%d. Found %d features.\n", min_level, max_level, n_id);
	if (renumber) fprintf (stderr, "binary_to_ascii: Will renumber features 0-%d.\n", n_id-1);
	
	if (feature == IS_COAST) {
		/* Auto-determine which resolution this is */
	
		if (n_id > 180000)
			res = 0;
		else if (n_id > 100000)
			res = 1;
		else if (n_id > 30000)
			res = 2;
		else if (n_id > 5000)
			res = 3;
		else
			res = 4;
		
		if (verbose) fprintf (stderr, "binary_to_ascii: GSHHS Resolution %s: %d polygons\n", resolution[res], n_id);
		max_level = GMT_MAX_GSHHS_LEVEL;
	}
	else if (feature == IS_BORDER) {
		if (verbose) fprintf (stderr, "binary_to_ascii: WDBII Borders: %d segments\n", n_id);
		max_level = GMT_N_BLEVELS;
	}
	else  {
		if (verbose) fprintf (stderr, "binary_to_ascii: WDBII Rivers: %d segments\n", n_id);
		/* max_level = GMT_N_RLEVELS; Commented out since there is more stuff in WDB not used in GMT */
		max_level++;	/* Since we go 1-max_level but subtract 1 to get unit */
	}
	
	/* Create output files */
	
	for (level = 1; level <= max_level; level++) {	/* One for each level */
		unit = level - 1;
		if (feature == IS_COAST) {
			sprintf (file, "GSHHS/res_%c/GSHHS_%c_Level_%d.%s", resolution[res][0], resolution[res][0], level, ext);
			if ((fp[unit] = fopen (file, "w")) == NULL) {
				fprintf (stderr, "binary_to_ascii: ERROR - unable to create file %s\n", file);
				exit (EXIT_FAILURE);
			}
			fprintf (fp[unit], "# %cId$\n#\n# GSHHS Master File: Resolution = %s Level = %d\n",
				'$', resolution[res], level);
			if (level == 1) fprintf (fp[unit], "# See separate files for the six continents\n");
			fprintf (fp[unit], "#\n");
		}
		else if (n[unit]) {
			sprintf (file, "WDBII/%s/WDBII_%s_Level_%2.2d.%s", dir[feature], prefix[feature], unit, ext);
			if ((fp[unit] = fopen (file, "w")) == NULL) {
				fprintf (stderr, "line_to_ascii: ERROR - unable to create file %s\n", file);
				exit (EXIT_FAILURE);
			}
			fprintf (fp[unit], "# %cId$\n#\n# WDBII %s Master File: Level = %d\n", '$', prefix[feature], unit);
			fprintf (fp[unit], "#\n");
		}
	}
	for (i = 0; feature == IS_COAST && i < N_CONTINENTS; i++) {	/* Coastlines also has extra file for each of the continents */
		unit = GMT_MAX_GSHHS_LEVEL + i;
		n[unit+1] = 1;
		sprintf (file, "GSHHS/res_%c/GSHHS_%c_%s.%s", resolution[res][0], resolution[res][0], continents[i], ext);
		if ((fp[unit] = fopen (file, "w")) == NULL) {
			fprintf (stderr, "binary_to_ascii: ERROR - unable to create file %s\n", file);
			exit (EXIT_FAILURE);
		}
		fprintf (fp[unit], "# %cId$\n#\n# GSHHS Master File: Resolution = %s Continent = %s\n#\n",
			'$', resolution[res], continents[i]);
	}
	
	for (i = 0; i < n_id; i++) {
		id = (renumber) ? i : P[i].h.id;
		cont_id = P[i].h.continent;
		
		if (feature < IS_COAST)			/* Just rivers or borders */
			unit = P[i].h.level;
		else if (cont_id)		/* Continent polygon */
			unit = GMT_MAX_GSHHS_LEVEL + cont_id - 1;
		else					/* All else written one file per level */
			unit = P[i].h.level - 1;
		
		fprintf (stderr, "binary_to_ascii: Extracting %s %s # %7d\r", prefix[feature], kind[feature], id);	
		
		if (P[i].h.river) P[i].h.area = -P[i].h.area;	/* Flag riverlakes by assigning a negative area */
		greenwich = (P[i].h.greenwich & 3) == 1;	/* Just crossing Greenwhich */
		if (feature == IS_COAST) {			/* Coast */
			(feature == 2 && P[i].h.parent != -1) ? sprintf (parent, "%d", P[i].h.parent) : sprintf (parent, "-");
			(feature == 2 && P[i].h.ancestor != -1) ? sprintf (ancestor, "%d", P[i].h.ancestor) : sprintf (ancestor, "-");
			fprintf (fp[unit], "> Id = %d N = %d G = %d L = %d E = %d S = %d R = %.6f/%.6f/%.6f/%.6f A = %.12g B = %.12g P = %s F = %s\n",
				id, P[i].h.n, P[i].h.greenwich, P[i].h.level, P[i].h.datelon/MILL,
				P[i].h.source, P[i].h.west, P[i].h.east, P[i].h.south, P[i].h.north, P[i].h.area, P[i].h.area_res, parent, ancestor);
		}
		else {
			fprintf (fp[unit], "> Id = %d N = %d G = %d L = %d E = %d S = %d R = %.6f/%.6f/%.6f/%.6f A = %.12g\n",
				id, P[i].h.n, P[i].h.greenwich, P[i].h.level, P[i].h.datelon/MILL,
				P[i].h.source, P[i].h.west, P[i].h.east, P[i].h.south, P[i].h.north, P[i].h.area);
		}
		
		fseek (fp_in, P[i].pos, SEEK_SET);
		
		for (k = 0; k < P[i].h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf(stderr,"binary_to_ascii:  ERROR reading data record.\n");
				exit (EXIT_FAILURE);
			}
			if (p.x < 0) p.x += M360;
			if (greenwich && p.x > P[i].h.datelon) p.x -= M360;
			fprintf (fp[unit], "%10.6f\t%10.6f\n", 1.0e-6*p.x, 1.0e-6*p.y);	/* 10 is OK since the most negative x is only -17.xxx */
		}
	}
	fprintf (stderr, "\n");	
		
	fclose (fp_in);
	for (i = 0; i < max_level; i++) if (n[i+1]) fclose (fp[unit]);

	exit (EXIT_SUCCESS);
}
