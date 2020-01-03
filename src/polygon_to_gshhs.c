/*
  *	read polygon.b format and write a GSHHS file to stdout
 *	For version 1.4 we standardize GSHHS header to only use 4-byte ints.
 *	We also enforce writing of positive longitudes (0-360) * 1e6
 *	Now excludes the extra duplicate point in Antarctica.
 */

#include "wvs.h"
#include "gshhg/gshhg.h"

#define GSHHS_INV_SCL	(1.0 / GSHHG_SCL)	/* Convert degrees to micro-degrees */
#define set_level(h) ((h.source == GSHHS_ANTARCTICA_ICE_SRC) ? ANTARCTICA : ((h.source == GSHHS_ANTARCTICA_GROUND_SRC) ? ANTARCTICA_G : h.level))
#define swabi4(i4) (((i4) >> 24) + (((i4) >> 8) & 65280) + (((i4) & 65280) << 8) + (((i4) & 255) << 24))

int main (int argc, char **argv)
{
	FILE *fp_in = NULL;
	int k, version = GSHHG_COMPATIBILITY_VERSION, lines = 0, np, q = 0, m = 0, n_alloc = 0, antarctica;
	unsigned int level;
	double scale = 1.0;
	struct LONGPAIR *p = NULL;
	struct GMT3_POLY h;
	struct GSHHG_HEADER gshhs_header;

	if (argc < 2 || argc > 3) {
		fprintf (stderr,"usage:  polygon_to_gshhs [-l] file_res.b > gshhs_res.b\n");
		fprintf (stderr,"	-l indicates data are lines (rivers, borders) and not polygons\n");
		exit (EXIT_FAILURE);
	}
	if (argc == 3 && !strcmp (argv[1], "-l")) lines = 1;
	fp_in = fopen(argv[1+lines], "r");
	memset ((void *)&h, 0, sizeof (struct GSHHG_HEADER));

#ifndef WORDS_BIGENDIAN
	fprintf (stderr,"polygon_to_gshhg will perform swab on output\n");
#endif
	while (pol_readheader (&h, fp_in) == 1) {
		gshhs_header.west	= lrint (h.west * GSHHS_INV_SCL);
		gshhs_header.east	= lrint (h.east * GSHHS_INV_SCL);
		gshhs_header.south	= lrint (h.south * GSHHS_INV_SCL);
		gshhs_header.north	= lrint (h.north * GSHHS_INV_SCL);
		gshhs_header.id		= h.id;
		gshhs_header.n		= h.n;
		if (lines) {	/* Lines have no area */
			gshhs_header.area_full = gshhs_header.area = 0;
		}
		else {
			m = 9 - lrint (ceil (log10 (fabs (h.area))));	/* The magnitude of scaling as in 10^p */
			scale = pow (10.0, (double)m);
			gshhs_header.area_full	= lrint (scale * h.area);
			gshhs_header.area	= lrint (scale * h.area_res);
		}
		/* For Antractica (level 1) we encode the level as 5 (ice-front) & 6 (grounding line) and leave source as 0 */
		level = set_level(h);
		if (level > 4) {	/* Antarctica polygon; flag via level instead */
			h.level = level;
			h.source = 0;
		}
		gshhs_header.flag	= h.level + (version << 8) + ((h.greenwich & 3) << 16) + (h.source << 24) + (h.river << 25) + (m << 26);
		gshhs_header.container	= h.parent;
		gshhs_header.ancestor	= h.ancestor;
		np = gshhs_header.n;	/* How many to read */
		if ((gshhs_header.east - gshhs_header.west) == M360) {
			gshhs_header.n--;	/* Antarctica, drop the duplicated point for GSHHS */
			gshhs_header.west	= -M180;
			gshhs_header.east	= +M180;
			antarctica = TRUE;
		}
		else
			antarctica = FALSE;

#ifndef WORDS_BIGENDIAN
		/* Must swap header explicitly on little-endian machines */
		gshhs_header.west	= swabi4 ((unsigned int)gshhs_header.west);
		gshhs_header.east	= swabi4 ((unsigned int)gshhs_header.east);
		gshhs_header.south	= swabi4 ((unsigned int)gshhs_header.south);
		gshhs_header.north	= swabi4 ((unsigned int)gshhs_header.north);
		gshhs_header.id		= swabi4 ((unsigned int)gshhs_header.id);
		gshhs_header.n		= swabi4 ((unsigned int)gshhs_header.n);
		gshhs_header.area	= swabi4 ((unsigned int)gshhs_header.area);
		gshhs_header.area_full	= swabi4 ((unsigned int)gshhs_header.area_full);
		gshhs_header.flag	= swabi4 ((unsigned int)gshhs_header.flag);
		gshhs_header.container	= swabi4 ((unsigned int)gshhs_header.container);
		gshhs_header.ancestor	= swabi4 ((unsigned int)gshhs_header.ancestor);
#endif
		if (np > n_alloc) {
			n_alloc = np;
			p = (struct LONGPAIR *) GMT_memory (VNULL, n_alloc, sizeof (struct LONGPAIR), "polygon_to_gshhs");
		}
		if (pol_fread (p, np, fp_in) != np) {
			fprintf (stderr,"polygon_to_gshhs:  ERROR  reading file %s.\n", argv[1]);
			exit (EXIT_FAILURE);
		}
		if (antarctica) {	/* Antarctica - switch to -180/+180 organization */
			int kk = 0, nn = 0;
			struct LONGPAIR pk;	/* Use copy since pol_fwrite2 does a swab */
			for (k = kk = 0; k < np && kk == 0; k++) if (p[k].x == M180) kk = k;	/* First first point lon = 180 */
			if (kk == 0) fprintf (stderr, "Could not find x = 180 in Antarctica polygon %d\n", gshhs_header.id);
			/* Predetermine how many points to write so we can update header.n */
			for (k = kk; k < np; k++) {	/* Write 180 to 0 first */
				if (p[k].x < 0) p[k].x += M360;
				if (p[k].x == M360) continue;
				nn++;
			}
			for (k = 0; k <= kk; k++) {	/* Write <360 to 180 next but as negative degrees now, ending at -180 */
				if (p[k].x < 0) p[k].x += M360;
				if (p[k].x == M360) continue;
				if (p[k].x > 0) p[k].x -= M360;
				nn++;
			}
			/* So nn has the new count, update header and write it */
			fprintf (stderr, "np in = %d np out = %d\n", np, nn);
			gshhs_header.n = nn;
#ifndef WORDS_BIGENDIAN
			gshhs_header.n = swabi4 ((unsigned int)gshhs_header.n);
#endif
			fwrite ((char *)&gshhs_header, sizeof (struct GSHHG_HEADER), 1, stdout);
			/* No process the nn records */
			for (k = kk; k < np; k++) {	/* Write 180 to 0 first */
				if (p[k].x < 0) p[k].x += M360;
				if (p[k].x == M360) continue;
				pk = p[k];
				if (k < np && pol_fwrite2 (&pk, 1, stdout) != 1) {
					fprintf (stderr,"polygon_to_gshhs:  ERROR  writing to stdout.\n");
					exit (EXIT_FAILURE);
				}
				nn++;
			}
			for (k = 0; k <= kk; k++) {	/* Write <360 to 180 next but as negative degrees now, ending at -180 */
				if (p[k].x < 0) p[k].x += M360;
				if (p[k].x == M360) continue;
				if (p[k].x > 0) p[k].x -= M360;
				pk = p[k];
				if (k < np && pol_fwrite2 (&pk, 1, stdout) != 1) {
					fprintf (stderr,"polygon_to_gshhs:  ERROR  writing to stdout.\n");
					exit (EXIT_FAILURE);
				}
				nn++;
			}
		}
		else {
			fwrite((char *)&gshhs_header, sizeof (struct GSHHG_HEADER), 1, stdout);
			for (k = 0; k < h.n; k++) {
				if (p[k].x < 0) p[k].x += M360;
				if (k < np && pol_fwrite2 (&p[k], 1, stdout) != 1) {
					fprintf (stderr,"polygon_to_gshhs:  ERROR  writing to stdout.\n");
					exit (EXIT_FAILURE);
				}
			}
		}
		q++;
	}

	fclose (fp_in);
	GMT_free ((void *)p);

	exit (EXIT_SUCCESS);
}
