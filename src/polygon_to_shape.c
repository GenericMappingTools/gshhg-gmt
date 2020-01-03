/* 
 *	Reads a polygon (or line) file and creates a multisegment GMT file with
 *	appropriate GIS tags so ogr2ogr can convert it to a shapefile.
 */

#include "gmt.h"
#include "wvs.h"

#define M270 270000000
#define set_level(h) ((h.source == GSHHS_ANTARCTICA_ICE_SRC) ? ANTARCTICA : ((h.source == GSHHS_ANTARCTICA_GROUND_SRC) ? ANTARCTICA_G : h.level))

struct POLYGON {
	struct GMT3_POLY h;
	struct	LONGPAIR *p;
} P[N_POLY];

#define W 0
#define E 1

int main (int argc, char **argv)
{
	FILE *fp_in, *fp;
	int err = 0;
	GMT_LONG n_id = 0, id, k, level, lev, x, x0 = 0, y0 = 0, ymin = M90, ymax = -M90, hemi, first, lines = 0, n_levels, river, border;
	GMT_LONG np[2], limit, level_id[11] = {0,1,2,3,4,6,7,8,10,11,13}, this_level, n_in_level[11];
	char file[BUFSIZ], cmd[BUFSIZ], *SRC[2] = {"WDBII", "WVS"}, *ANT[2] = {"ANT-I", "ANT-G"}, *H = "EW", *ITEM[3] = {"polygon", "border", "river"}, fmt[BUFSIZ], *S = NULL;
	char *header[2] = {"# @VGMT1.0 @GPOLYGON @Nid|level|source|parent_id|sibling_id|area @Tchar|integer|char|integer|integer|double\n",
		"# @VGMT1.0 @GLINESTRING @Nid|level @Tchar|integer\n"};
	double area, *lon[2] = {NULL, NULL}, *lat[2] = {NULL, NULL};
        
	argc = GMT_begin (argc, argv);
	
	if (argc < 2 || argc > 4) {
		fprintf (stderr,"usage:  polygon_to_shape file_res.b prefix [-b|i]\n");
		fprintf (stderr,"	file_res.b is the binary local file with all polygon info for a resolution\n");
		fprintf (stderr,"	prefix is used to form the files prefix_L[1-4].gmt\n");
		fprintf (stderr,"	These are then converted to shapefiles via ogr2ogr\n");
		fprintf (stderr,"	Append -o if file_res.b is a border line file\n");
		fprintf (stderr,"	Append -i if file_res.b is a river line file\n");
		exit (EXIT_FAILURE);
	}
	border = (argc == 4 && !strcmp (argv[3], "-o"));
	river  = (argc == 4 && !strcmp (argv[3], "-i"));
	lines = 2*river + border;	/* Since only one is set to 1 we get 0, 1, or 2 */
	fp_in = fopen (argv[1], "r");
		
	while (pol_readheader (&P[n_id].h, fp_in) == 1) {
		P[n_id].p = (struct LONGPAIR *) GMT_memory (VNULL, P[n_id].h.n, sizeof (struct LONGPAIR), "polygon_to_shape");
		if (pol_fread (P[n_id].p, P[n_id].h.n, fp_in) != P[n_id].h.n) {
			fprintf(stderr,"polygon_to_shape:  ERROR  reading file.\n");
			exit(-1);
		}
		for (k = 0; k < P[n_id].h.n; k++) {
			if (P[n_id].p[k].y < ymin) ymin = P[n_id].p[k].y;
			if (P[n_id].p[k].y > ymax) ymax = P[n_id].p[k].y;
		}
		n_id++;
	}
	fclose (fp_in);
	fprintf (stderr, "polygon_to_shape: Found %ld %ss\n", n_id, ITEM[lines]);
	if (lines == 0) {	/* Adjust for two Antarcticas */
		level_id[5] = 5;
		level_id[6] = 6;
		ymin = -90000000;	/* Because of Antarctica */
	}
	n_levels = (lines == 0) ? 6 : ((border) ? 3 : 11);
	if (river == 0) {
		for (k = 0; k < n_levels; k++) level_id[k]++;	/* Get 1,2,3[,4] if not river */
		sprintf (fmt, "%s_L%%ld.gmt", argv[2]);
	}
	else
		sprintf (fmt, "%s_L%%2.2ld.gmt", argv[2]);
	memset (n_in_level, 0, 11*sizeof(int));
	for (lev = 1; lev <= n_levels; lev++) {	/* Make separate files for each level*/
		level = level_id[lev-1];
		sprintf (file, fmt, lev);
		fprintf (stderr, "Create file %s\n", file);
		if ((fp = fopen (file, "w")) == NULL) {
			fprintf(stderr,"polygon_to_shape:  ERROR  creating file %s.\n", file);
			exit(-1);
		}
		fprintf (fp, "%s", header[lines>0]);
		fprintf (fp, "# @R-180/180/%.6f/%.6f @Jp\"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs\"\n# FEATURE_DATA\n", ymin*I_MILL, ymax*I_MILL);
		for (id = 0; id < n_id; id++) {
			this_level = set_level (P[id].h);
			if (this_level != level) continue;
			limit = (id == 0) ? M270 : M180;
			n_in_level[lev-1]++;
			/* Here we found a polygon of the required level.  Write out polygon tag and info */
			area = P[id].h.area;
			if (P[id].h.river) area = -area;	/* Flag river lakes with negative area */
			S = (lev > 4) ? ANT[lev-5] : SRC[P[id].h.source];
			if ((P[id].h.id == (ANTARCTICA-1) || P[id].h.id == (ANTARCTICA_G-1)) && !lines) {	/* Antarctica requires special treatment */
				P[id].h.n--;	/* Skip the duplicate point */

				for (hemi = 0; hemi < 2; hemi++) {
					fprintf (fp, "> GSHHS polygon Id = %d-%c Level = %ld Area = %.12g\n# @P @D%d-%c|%ld|%s|%d|%d|%.12g\n",
						P[id].h.id, H[hemi], lev, area, P[id].h.id, H[hemi], lev, S, P[id].h.parent, P[id].h.ancestor, area);
					for (k = 0, first = TRUE; k < P[id].h.n; k++) {	/* Set up lons that go -20 to + 192 */
						if (hemi == 0) {
							if (P[id].p[k].x > M180) continue;
							x = P[id].p[k].x;
							if (first) {x0 = x; y0 = P[id].p[k].y;}
						}
						else if (hemi == 1) {
							if (P[id].p[k].x < M180) continue;
							x = P[id].p[k].x - M360;
							if (first) {x0 = x; y0 = P[id].p[k].y;}
						}
						fprintf (fp, "%.6f\t%.6f\n", x * I_MILL, P[id].p[k].y * I_MILL);
						first = FALSE;
					}
					x = (hemi == 0) ? 0 : -M180;
					fprintf (fp, "%.6f\t%.6f\n%.6f\t%.6f\n%.6f\t%.6f\n", x * I_MILL, -90.0, x0 * I_MILL, -90.0, x0 * I_MILL, y0 * I_MILL);
				}
				
			}
			else if (!lines && (P[id].h.west < 180.0 && P[id].h.east > 180.0)) {	/* Straddles dateline; must split into two parts thanx to GIS brilliance */
				for (hemi = 0; hemi < 2; hemi++) {	/* Add one extra space in case of closure issue */
					lon[hemi] = (double *)GMT_memory (VNULL, sizeof (double), P[id].h.n+1, GMT_program);
					lat[hemi] = (double *)GMT_memory (VNULL, sizeof (double), P[id].h.n+1, GMT_program);
				}
				
				for (k = np[0] = np[1] = 0; k < P[id].h.n; k++) {
					if (P[id].h.id == 0 && P[id].p[k].x > M270) {	/* part of western Europe requires negative longs */
						lon[W][np[W]] = (P[id].p[k].x - M360) * I_MILL;	lat[W][np[W]++] = P[id].p[k].y * I_MILL;
					} else if (P[id].p[k].x < M180) {	/* part of west keep positive longs */
							lon[W][np[W]] = P[id].p[k].x * I_MILL;	lat[W][np[W]++] = P[id].p[k].y * I_MILL;
					} else if (P[id].p[k].x > M180) {	/* part of east, switch to negative lons */
						lon[E][np[E]] = (P[id].p[k].x - M360) * I_MILL;	lat[E][np[E]++] = P[id].p[k].y * I_MILL;
					} else if (P[id].p[k].x == M180) {	/* part of both */
						lon[W][np[W]] = P[id].p[k].x * I_MILL;	lat[W][np[W]++] = P[id].p[k].y * I_MILL;
						lon[E][np[E]] = (P[id].p[k].x - M360) * I_MILL;	lat[E][np[E]++] = P[id].p[k].y * I_MILL;
					}
				}
				for (hemi = 0; hemi < 2; hemi++) {
					if (!((lon[hemi][0] == lon[hemi][np[hemi]-1]) && (lat[hemi][0] == lat[hemi][np[hemi]-1]))) {	/* Not closed */
						lon[hemi][np[hemi]] = lon[hemi][0];
						lat[hemi][np[hemi]] = lat[hemi][0];
						np[hemi]++;
						fprintf (stderr, "Needed to close E/W split polygon %d-%c explicitly\n", P[id].h.id, H[hemi]);
					}
					fprintf (fp, "> GSHHS polygon Id = %d-%c Level = %ld Area = %.12g\n# @P @D%d-%c|%ld|%s|%d|%d|%.12g\n",
						P[id].h.id, H[hemi], lev, area, P[id].h.id, H[hemi], lev, S, P[id].h.parent, P[id].h.ancestor, area);
					fprintf (fp, "%.6f\t%.6f\n", lon[hemi][0], lat[hemi][0]);
					for (k = 1; k < np[hemi]; k++) {	/* Avoid printing zero increments */
						if (!(GMT_IS_ZERO (lon[hemi][k]-lon[hemi][k-1]) && GMT_IS_ZERO (lat[hemi][k]-lat[hemi][k-1]))) fprintf (fp, "%.6f\t%.6f\n", lon[hemi][k], lat[hemi][k]);
					}
					GMT_free ((void *)lon[hemi]);	GMT_free ((void *)lat[hemi]);
				}
			}
			else {	/* No problems, just write as is */
				if (lines)
					fprintf (fp, "> WDBII %s line Id = %d Level = %ld\n# @D%d|%ld\n", ITEM[lines], P[id].h.id, lev, P[id].h.id, lev);
				else {
					fprintf (fp, "> GSHHS polygon Id = %d Level = %ld Area = %.12g\n# @P @D%d|%ld|%s|%d|%d|%.12g\n",
					P[id].h.id, lev, area, P[id].h.id, lev, S, P[id].h.parent, P[id].h.ancestor, area);
				}
				for (k = 0; k < P[id].h.n; k++) {
					if (P[id].h.west == 180.0)
						x = (P[id].p[k].x >= P[id].h.datelon) ? P[id].p[k].x - M360 : P[id].p[k].x;
					else
						x = (P[id].p[k].x > P[id].h.datelon) ? P[id].p[k].x - M360 : P[id].p[k].x;
					fprintf (fp, "%.6f\t%.6f\n", x * I_MILL, P[id].p[k].y * I_MILL);
				}
			}
		}
		fclose (fp);	/* Done with this set */
	}
	
	for (id = 0; id < n_id; id++) GMT_free ((void *)P[id].p);
	
	fprintf (stderr,"Now convert to ESRI Shapefiles: ");
	if (river)
		sprintf (fmt, "ogr2ogr -f \"ESRI Shapefile\" %s_L%%2.2ld %s_L%%2.2ld.gmt\n", argv[2], argv[2]);
	else
		sprintf (fmt, "ogr2ogr -f \"ESRI Shapefile\" %s_L%%ld %s_L%%ld.gmt\n", argv[2], argv[2]);
	
	for (lev = 1; lev <= n_levels; lev++) {	/* Make separate files for each level*/
		if (n_in_level[lev-1] == 0) continue;
		fprintf (stderr, "Got %ld items for level %ld\n", n_in_level[lev-1], lev);
		sprintf (cmd, fmt, lev, lev);
		fprintf (stderr,"Run: %s\n", cmd);
		if ((err = system (cmd)))
			fprintf (stderr,"ERROR: System call %s failed with error %d\n", cmd, err);
		
		if (lev != level_id[lev-1])
			fprintf (stderr, " %ld [was %ld]", lev, level_id[lev-1]);
		else
			fprintf (stderr, " %ld", lev);
	}
	if (river)
		fprintf (stderr," done\nThe shapefiles will be in directories %s_L[01-%ld]\n", argv[2], n_levels);
	else
		fprintf (stderr," done\nThe shapefiles will be in directories %s_L[1-%ld]\n", argv[2], n_levels);
	
	GMT_end (argc, argv);
	
	exit (EXIT_SUCCESS);
}
