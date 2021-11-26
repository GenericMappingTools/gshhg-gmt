/*
 * polygon_consistency checks for propoer closure and crossings
 * within polygons
 *
 */

#include "wvs.h"

double lon[N_LONGEST], lat[N_LONGEST];
double lon2[N_LONGEST], lat2[N_LONGEST];
#define P_LIMIT 0.001

int main (int argc, char **argv)
{
	FILE	*fp, *fp2 = NULL, *fp3 = NULL;
	int	i, n_id, this_n, nd, nx, n_x_problems, n_s_problems, n_c_problems, n_r_problems, n_b_problems;
	int	n_d_problems, n_a_problems, n_p_problems, n_i_problems, ix0 = 0, iy0 = 0, report_mismatch = 0;
	int	w, e, s, n, ixmin, ixmax, iymin, iymax, last_x = 0, last_y = 0, skip_i;
	int	ant_trouble = 0, found, A, B, end, n_adjust = 0, left, right, np, np2;
	struct GMT_XSEGMENT *ylist;
	struct GMT_XOVER XC;
	struct GMT3_POLY h, h2;
	struct LONGPAIR p;
	double dx1, dx2, dy1, dy2, off, cos_a, ratio, dist_limit = 2.0, dist;

	if (argc < 2 || argc > 3) {
		fprintf(stderr,"usage:  polygon_consistency GSHHS_polygons.b [dist_limit] > report.lis\n");
		fprintf(stderr,"	dist_limit is used for spike detection in meters [2]\n");
		exit (-1);
	}
	if (argc == 3) dist_limit = atof (argv[2]);
	argc = GMT_begin (argc, argv);

	fp = fopen(argv[1], "r");
	fp2 = fopen("area_ratios.txt", "w");
	fp3 = fopen("new.bin", "wb");
	
	n_id = n_c_problems = n_x_problems = n_r_problems = n_d_problems = n_s_problems = n_a_problems = n_i_problems = n_b_problems = n_p_problems = 0;
	while (pol_readheader (&h, fp) == 1) {
		h2 = h;	/* Make a copy */
		if (n_id == 0 && h.n > 1000000) report_mismatch = 1;
	
		if (Pol_Is_Antarctica (h.continent)) {
			if (h.south > -90.0) ant_trouble = TRUE;
		}
		if (h.area < 0 && h.level != 2) fprintf (stderr, "Pol %d has negative area and is level %d\n", h.id, h.level);
		if ((h.river & 1) && h.level != 2) fprintf (stderr, "Pol %d is a riverlake but level is %d\n", h.id, h.level);
		if (GMT_IS_ZERO (h.area_res)) {
			fprintf (stderr, "Pol %d has zero resolution area\n", h.id);
			n_b_problems++;
		}
		ratio = h.area_res / fabs (h.area);
		if (ratio < P_LIMIT) {	/* Less than 0.01% of original area */
			fprintf (stderr, "Pol %d has < %g %% of full resolution area\n", h.id, 100.0 * P_LIMIT);
			n_p_problems++;
		}
		fprintf (fp2, "%g\n", ratio);
		ixmin = iymin = M360;
		ixmax = iymax = -M360;
		w = lrint (h.west * 1e6);
		e = lrint (h.east * 1e6);
		s = lrint (h.south * 1e6);
		n = lrint (h.north * 1e6);
		last_x = last_y = 2*M360;
		for (i = nd = np = 0; i < h.n; i++) {
			if (pol_fread (&p, 1, fp) != 1) {
				fprintf(stderr,"polygon_consistency:  ERROR  reading file.\n");
				exit(-1);
			}
			if (p.x < 0 || p.x > M360) fprintf (stderr, "Pol %d, point %d: x outside range [%g]\n", h.id, i, p.x*1e-6);
			if (p.y < -M90 || p.y > M90) fprintf (stderr, "Pol %d, point %d: y outside range [%g]\n", h.id, i, p.y*1e-6);
			if ((h.greenwich & 1) && p.x > h.datelon) p.x -= M360;
			if (i == 0) {
				ix0 = p.x;
				iy0 = p.y;
			}
			lon[np] = p.x * 1e-6;
			lat[np] = p.y * 1e-6;
			if (p.x < ixmin) ixmin = p.x;
			if (p.x > ixmax) ixmax = p.x;
			if (p.y < iymin) iymin = p.y;
			if (p.y > iymax) iymax = p.y;
			/* Check for duplicate but excuse Antarcticas 0-360 duplication */
			if (i > 0 && last_x == p.x && last_y == p.y && !(Pol_Is_Antarctica (h.continent) && (p.x == 0 || p.x == M360))) {
				printf ("%d\tduplicate point line %d - skipped\n", h.id, i);
				nd++;
			}
			else
				np++;
			last_x = p.x;
			last_y = p.y;
		}
		
		if (nd) n_d_problems++;
		if (Pol_Is_Antarctica (h.continent)) iymin = -M90;
		if (! (p.x == ix0 && p.y == iy0)) {
			printf ("%d\tnot closed\n", h.id);
			n_c_problems++;
		}
		if (report_mismatch && !(ixmin == w && ixmax == e && iymin == s && iymax == n) && !Pol_Is_Antarctica (h.continent)) {
			printf ("%d\twesn mismatch.  Should be %.6f/%.6f/%.6f/%.6f\n", h.id, 1e-6 * ixmin, 1e-6 * ixmax, 1e-6 * iymin, 1e-6 * iymax);
			h2.west = 1e-6 * ixmin;		h2.east = 1e-6 * ixmax;
			h2.south = 1e-6 * iymin;	h2.north = 1e-6 * iymax;
			n_r_problems++;
		}
		this_n = (Pol_Is_Antarctica (h.continent)) ? np - 1 : np;
		gmt_init_track (lat, this_n, &ylist);
		//if (!GMT_IS_ZERO (h.east - h.west) && !Pol_Is_Antarctica (h.continent)) {
			nx = found = gmt_crossover (lon, lat, NULL, ylist, this_n, lon, lat, NULL, ylist, this_n, TRUE, TRUE, &XC);
			GMT_free ((void *)ylist);
			for (i = end = 0; i < nx; i++) {
				A = lrint (XC.xnode[0][i]);
				B = lrint (XC.xnode[1][i]);
				if ((A == 0 && B == (np-1)) || (B == 0 && A == (np-1))) {	/* Involving end point */
					off = MAX (fabs((double)A - XC.xnode[0][i]), fabs((double)B - XC.xnode[1][i]));
					if (GMT_IS_ZERO (off)) {
						/* Remove the crossover caused by the duplicate start/end points */
						end++;
						n_adjust++;
					}
				}
			}
			nx -= end;

			if (nx) {
				for (i = 0; i < nx; i++) printf ("P = %d\tnode = %d\t%10.5f\t%9.5f\n", h.id, (int)floor(XC.xnode[0][i]), XC.x[i], XC.y[i]);
				n_x_problems++;
			}
			if (found) GMT_x_free (&XC);
		//}
		/* Look for duplicate points separated by a single outlier (a non-area peninsula) */
		
		np2 = 0;
		for (i = 0; i < (np-1); i++) {
			skip_i = 0;
			left = (i) ? i - 1 : np - 2;	/* Skip around and avoid duplicate end point */
			right = i + 1;			/* Never wrap since we dont go to the end point */
			if (GMT_IS_ZERO (lon[left]-lon[right]) && GMT_IS_ZERO (lat[left]-lat[right])) {
				printf ("%d\tNon-area excursion on line %d-%d-%d\n", h.id, left, i, right);
				n_s_problems++;
				skip_i = 1;
			}
			/* Also check for 3 points on a line with zero angle between them */
			dy1 = lat[left] - lat[i];
			dy2 = lat[right] - lat[i];
			dx1 = (lon[left] - lon[i])  * cosd (lat[i]);
			dx2 = (lon[right] - lon[i]) * cosd (lat[i]);
			if (dy1 == 0.0 && dy2 == 0.0) {	/* Horizontal line, check x arrangement */
				if ((dx1 * dx2) > 0.0) {
					printf ("%d\tZero-angle excursion on line %d-%d-%d\n", h.id, left, i, right);
					n_a_problems++;
					skip_i = 1;
				}
			}
			else if (dx1 == 0.0 && dx2 == 0.0) {	/* Vertical line, check y arrangement */
				if ((dy1 * dy2) > 0.0) {
					printf ("%d\tZero-angle excursion on line %d-%d-%d\n", h.id, left, i, right);
					n_a_problems++;
					skip_i = 1;
				}
			}
			else {	/* General case */
				double t, sin_a, angle;
				cos_a = (dx1 * dx2 + dy1 * dy2) / (hypot (dx1, dy1) * hypot (dx2, dy2));	/* Normalized dot product of vectors from center node to the 2 neightbors */
				angle = acosd (cos_a);
				t = 1.0 - cos_a;
				sin_a = sqrt (1.0 - cos_a * cos_a);
				if (h.id == 32850) {
					dist = 9.0;
				}
				if (GMT_IS_ZERO (fabs (t))) {
					printf ("%d\tZero-angle excursion on line %d-%d-%d\n", h.id, left, i, right);
					n_a_problems++;
					skip_i = 1;
				}
				/* Look for spikes such that the intermediate (spike-making) point is < min_dist from the line defined by its two neighbors */
				if (((lon[right] >= lon[left] && lon[right] <= lon[i]) || (lon[right] <= lon[left] && lon[right] >= lon[i])) && 
					((lat[right] >= lat[left] && lat[right] <= lat[i]) || (lat[right] <= lat[left] && lat[right] >= lat[i]))) {	/* right may be a spike */
					dist = fabs (sin_a) * hypot (dx2, dy2) * project_info.DIST_M_PR_DEG;
					if (angle < 90.0 && dist < dist_limit) {
						printf ("%d\tNext spike excursion on line %d-%d-%d\n", h.id, left, i, right);
						skip_i = 1;
						n_i_problems++;
					}
				}
				else if (((lon[left] >= lon[right] && lon[left] <= lon[i]) || (lon[left] <= lon[right] && lon[left] >= lon[i])) && 
					((lat[left] >= lat[right] && lat[left] <= lat[i]) || (lat[left] <= lat[right] && lat[left] >= lat[i]))) {	/* left may be a spike */
					dist = fabs (sin_a) * hypot (dx1, dy1) * project_info.DIST_M_PR_DEG;
					if (angle < 90.0 && dist < dist_limit) {
						printf ("%d\tPrev spike excursion on line %d-%d-%d\n", h.id, left, i, right);
						skip_i = 1;
						n_i_problems++;
					}
				}
			}
			if (skip_i == 0) {
				if (np2 == 0 || !(lon[i] == lon2[np2] && lat[i] == lat2[np2])) {
					lon2[np2] = lon[i];	lat2[np2] = lat[i];
					np2++;
				}
			}
		}
		lon2[np2] = lon[0];	lat2[np2] = lat[0];
		np2++;
		h2.n = np2;
		pol_writeheader (&h2, fp3);
		for (i = 0; i < np2; i++) {
			if (lon2[i] < 0.0) lon2[i] += 360.0;
			p.x = irint (lon2[i] * MILL);
			p.y = irint (lat2[i] * MILL);
			if (pol_fwrite (&p, 1, fp3) != 1) {
				fprintf(stderr,"polygon_consistency:  ERROR  writing new.bin file.\n");
				exit (-1);
			}
			
		}
		n_id++;
	}
	
	fprintf (stderr, "polygon_consistency: Got %d polygons from file %s\n", n_id, argv[1]);
	if (n_b_problems) fprintf (stderr, "%d has no area.\n", n_b_problems);
	if (n_p_problems) fprintf (stderr, "%d has < %g %% of full area.\n", n_p_problems, 100.0 * P_LIMIT);
	if (n_c_problems) fprintf (stderr, "%d has closure problems.\n", n_c_problems);
	if (n_x_problems) fprintf (stderr, "%d has crossing problems.\n", n_x_problems);
	if (n_r_problems) fprintf (stderr, "%d has region problems.\n", n_r_problems);
	if (n_d_problems) fprintf (stderr, "%d has duplicate points.\n", n_d_problems);
	if (n_s_problems) fprintf (stderr, "%d has non-area excursions.\n", n_s_problems);
	if (n_a_problems) fprintf (stderr, "%d has zero-angle excursions\n", n_a_problems);
	if (n_i_problems) fprintf (stderr, "%d has spike excursions < %g m\n", n_i_problems, dist_limit);
	if (ant_trouble) fprintf (stderr, "polygon_consistency: Antarctica polygon has wrong south border\n");
	if (n_adjust) fprintf (stderr, "polygon_consistency: Skipped %d crossovers involving duplicate end points (polygon closure)\n", n_adjust);
	
	fclose(fp);
	fclose(fp2);
	fclose(fp3);
	fprintf (stderr, "polygon_consistency: Area ratios written to area_ratios.txt\n");
	fprintf (stderr, "polygon_consistency: Binary cleaned file written to new.bin\n");

	GMT_end (argc, argv);
	exit(0);
}
