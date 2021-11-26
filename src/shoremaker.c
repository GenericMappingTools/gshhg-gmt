/*
 * Author: Paul Wessel
 * Date:	3-OCT-97
 * Version:	2.1	Reads/writes netCDF files instead of native binary
 * Version:	3.1	Writes netCDF 3 files
 * Version:	3.2	Writes netCDF 4 files
 */

#include "gmt_dev.h"
#include "shore.h"

int main (int argc, char **argv) {
	int i, dims, n_id, n_nodes, n_ant, old_fill_mode;
	int bin_info_id_ANT, seg_info_ANT;	/* Id for variable bin_info_ANT */
	int cmode;
	size_t start, count;
	int *bin_firstseg, *seg_info, *seg_start, *seg_GSHHS_id;
	int *GSHHS_parent, *GSHHS_area_fraction, *GSHHS_node;
	double *GSHHS_area;

	short *bin_info, *bin_info_ANT, *bin_nseg, *pt_dx, *pt_dy;
	char file[512], *prefix;
	signed char *seg_ANT = NULL;
	FILE *fp_bin, *fp_seg, *fp_ant, *fp_pt, *fp_gshhs, *fp_ndid;
	struct GMT_SHORE s;
	struct SHORT_PAIR p;
	struct GMT3_FILE_HEADER file_head;
	struct GMT3_BIN_HEADER bin_head;
	struct SEGMENT_HEADER seg_head;

	argc = GMT_begin (argc, argv);

	if (argc < 2 || argc > 3) {
		fprintf (stderr, "usage: shoremaker shore_prefix [-n]\n");
		fprintf (stderr, "	Use -n to build classic netCDF file\n");
		exit (EXIT_FAILURE);
	}

	prefix = argv[1];

	sprintf (file, "%s.bin", prefix);
	if ((fp_bin = fopen (file, "r")) == NULL) {
		fprintf (stderr, "shoremaker:  Cannot open shore bin file %s\n", file);
		exit (EXIT_FAILURE);
	}
	sprintf (file, "%s.seg", prefix);
	if ((fp_seg = fopen (file, "r")) == NULL) {
		fprintf (stderr, "shoremaker:  Cannot open shore seg file %s\n", file);
		exit (EXIT_FAILURE);
	}
	sprintf (file, "%s.ant", prefix);
	if ((fp_ant = fopen (file, "r")) == NULL) {
		fprintf (stderr, "shoremaker:  Cannot open shore ant file %s\n", file);
		exit (EXIT_FAILURE);
	}
	sprintf (file, "%s.pt", prefix);
	if ((fp_pt = fopen (file, "r")) == NULL) {
		fprintf (stderr, "shoremaker:  Cannot open shore point file %s\n", file);
		exit (EXIT_FAILURE);
	}
	sprintf (file, "%s.pol", prefix);
	if ((fp_gshhs = fopen (file, "r")) == NULL) {
		fprintf (stderr, "shoremaker:  Cannot open shore pol file %s\n", file);
		exit (EXIT_FAILURE);
	}
	sprintf (file, "%s.ndid", prefix);
	if ((fp_ndid = fopen (file, "r")) == NULL) {
		fprintf (stderr, "shoremaker:  Cannot open shore node id file %s\n", file);
		exit (EXIT_FAILURE);
	}

	sprintf (file, "%s.nc", prefix);
	cmode = (argc == 3 && !strcmp (argv[2], "-n")) ? NC_CLASSIC_MODEL : NC_NETCDF4;
	GMT_err_fail (nc_create (file, cmode, &s.cdfid), file);
	GMT_err_fail (nc_set_fill (s.cdfid, NC_NOFILL, &old_fill_mode), file);

	fprintf (stderr, "shoremaker:  Process header file\n");

	if (fread ((void *)&file_head, sizeof (struct GMT3_FILE_HEADER), (size_t)1, fp_bin) != 1) {
		fprintf (stderr, "shoremaker:  Error reading file header\n");
		exit (EXIT_FAILURE);
	}

	s.bin_size = file_head.bsize;
	s.bin_nx = file_head.nx_bins;
	s.bin_ny = file_head.ny_bins;
	s.n_bin = file_head.n_bins;
	s.n_seg = file_head.n_segments;
	s.n_pt = file_head.n_points;

	bin_firstseg =  (int *) GMT_memory (CNULL, s.n_bin, sizeof (int), "shoremaker");
	bin_info     = (short *) GMT_memory (CNULL, s.n_bin, sizeof (short), "shoremaker");
	bin_info_ANT     = (short *) GMT_memory (CNULL, s.n_bin, sizeof (short), "shoremaker");
	bin_nseg     = (short *) GMT_memory (CNULL, s.n_bin, sizeof (short), "shoremaker");

	for (i = 0; i < s.n_bin; i++) {

		if (fread ((void *)&bin_head, sizeof (struct GMT3_BIN_HEADER), (size_t)1, fp_bin) != 1) {
			fprintf (stderr, "shoremaker:  Error reading bin header %d\n", i);
			exit (-1);
		}

		bin_firstseg[i] = bin_head.first_seg_id;
		bin_info[i] = bin_head.node_levels;
		bin_info_ANT[i] = bin_head.node_levels_ANT;
		bin_nseg[i] = bin_head.n_segments;
	}

	fclose (fp_bin);

	fprintf (stderr, "shoremaker:  Process segment file\n");

	seg_info	 = (int *) GMT_memory (CNULL, s.n_seg, sizeof (int), "shoremaker");
	seg_start	 = (int *) GMT_memory (CNULL, s.n_seg, sizeof (int), "shoremaker");
	seg_GSHHS_id	 = (int *) GMT_memory (CNULL, s.n_seg, sizeof (int), "shoremaker");

	for (i = 0; i < s.n_seg; i++) {

		if (fread ((void *)&seg_head, sizeof (struct SEGMENT_HEADER), (size_t)1, fp_seg) != 1) {
			fprintf (stderr, "shoremaker:  Error reading seg header %d\n", i);
			exit (-1);
		}

		seg_info[i] 	    = seg_head.info;
		seg_start[i]        = seg_head.first_p;
		seg_GSHHS_id[i]	    = seg_head.GSHHS_ID;
	}

	fclose (fp_seg);

	seg_ANT = (signed char *) GMT_memory (CNULL, s.n_seg, sizeof (signed char), "shoremaker");
	
	if (fread ((void *)seg_ANT, sizeof (signed char), (size_t)s.n_seg, fp_ant) != s.n_seg) {
		fprintf (stderr, "shoremaker:  Error reading ant array\n");
		exit (-1);
	}
	
	fclose (fp_ant);
	for (i = n_ant = 0; i < s.n_seg; i++) if (seg_ANT[i]) n_ant++;
	fprintf (stderr, "shoremaker:  Antarctic segment flags: %d of %d set\n", n_ant, s.n_seg);

	fprintf (stderr, "shoremaker:  Process point file\n");

	pt_dx = (short *) GMT_memory (CNULL, s.n_pt, sizeof (short), "shoremaker");
	pt_dy = (short *) GMT_memory (CNULL, s.n_pt, sizeof (short), "shoremaker");

	for (i = 0; i < s.n_pt; i++) {

		if (fread ((void *)&p, sizeof (struct SHORT_PAIR), (size_t)1, fp_pt) != 1) {
			fprintf (stderr, "shoremaker:  Error reading point %d\n", i);
			exit (-1);
		}

		pt_dx[i] = p.dx;
		pt_dy[i] = p.dy;
	}

	fclose (fp_pt);

	fprintf (stderr, "shoremaker:  Process parent file\n");

	if (fread ((void *)&n_id, sizeof (int), 1, fp_gshhs) != 1) {
		fprintf (stderr, "shoremaker: Error reading # of GSHHS parents\n");
		exit (EXIT_FAILURE);
	}
	GSHHS_parent = (int *) GMT_memory(VNULL, n_id, sizeof(int), "shoremaker");
	if (fread ((void *)GSHHS_parent, sizeof (int), n_id, fp_gshhs) != n_id) {
		fprintf (stderr, "shoremaker: Error reading GSHHS parents\n");
		exit (EXIT_FAILURE);
	}
	GSHHS_area = (double *) GMT_memory(VNULL, n_id, sizeof(double), "shoremaker");
	if (fread ((void *)GSHHS_area, sizeof (double), n_id, fp_gshhs) != n_id) {
		fprintf (stderr, "shoremaker: Error reading GSHHS area\n");
		exit (EXIT_FAILURE);
	}
	GSHHS_area_fraction = (int *) GMT_memory(VNULL, n_id, sizeof(int), "shoremaker");
	if (fread ((void *)GSHHS_area_fraction, sizeof (int), n_id, fp_gshhs) != n_id) {
		fprintf (stderr, "shoremaker: Error reading GSHHS area_fraction\n");
		exit (EXIT_FAILURE);
	}
	fclose (fp_gshhs);
	s.n_poly = n_id;

	fprintf (stderr, "shoremaker:  Process node ID file\n");

	if (fread ((void *)&n_nodes, sizeof (int), 1, fp_ndid) != 1) {
		fprintf (stderr, "shoremaker: Error reading # of nodes\n");
		exit (EXIT_FAILURE);
	}
	GSHHS_node = (int *) GMT_memory(VNULL, n_nodes, sizeof(int), "shoremaker");
	if (fread ((void *)GSHHS_node, sizeof (int), n_nodes, fp_ndid) != n_nodes) {
		fprintf (stderr, "shoremaker: Error reading GSHHS node ids\n");
		exit (EXIT_FAILURE);
	}
	fclose (fp_ndid);
	s.n_nodes = n_nodes;

	/* Create array of segment addresses, npoints, and type */

	if (s.bin_size == 60)
		strcpy (s.units, "1/65535 of 1 degree relative to south-west corner of bin");
	else
		sprintf (s.units, "1/65535 of %d degrees relative to south-west corner of bin", s.bin_size/60);

	/* define variables */

	GMT_err_fail (nc_def_dim (s.cdfid, "Dimension_of_scalar", 1, &dims), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Bin_size_in_minutes", NC_INT, (size_t)1, &dims, &s.bin_size_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_bins_in_360_longitude_range", NC_INT, (size_t)1, &dims, &s.bin_nx_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_bins_in_180_degree_latitude_range", NC_INT, (size_t)1, &dims, &s.bin_ny_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_bins_in_file", NC_INT, (size_t)1, &dims, &s.n_bin_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_polygons_in_file", NC_INT, (size_t)1, &dims, &s.n_poly_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_segments_in_file", NC_INT, (size_t)1, &dims, &s.n_seg_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_points_in_file", NC_INT, (size_t)1, &dims, &s.n_pt_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_nodes_in_file", NC_INT, (size_t)1, &dims, &s.n_node_id), file);

	GMT_err_fail (nc_def_dim (s.cdfid, "Dimension_of_polygon_array", s.n_poly, &dims), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Id_of_parent_polygons", NC_INT, (size_t)1, &dims, &s.GSHHS_parent_id), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, s.GSHHS_parent_id, NC_CHUNKED, chunksize(s.n_poly)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, s.GSHHS_parent_id, 1, 1, 9), file);
	GMT_err_fail (nc_def_var (s.cdfid, "The_km_squared_area_of_polygons", NC_DOUBLE, (size_t)1, &dims, &s.GSHHS_area_id), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, s.GSHHS_area_id, NC_CHUNKED, chunksize(s.n_poly)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, s.GSHHS_area_id, 1, 1, 9), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Micro_fraction_of_full_resolution_area", NC_INT, (size_t)1, &dims, &s.GSHHS_areafrac_id), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, s.GSHHS_areafrac_id, NC_CHUNKED, chunksize(s.n_poly)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, s.GSHHS_areafrac_id, 1, 1, 9), file);
	GMT_err_fail (nc_def_dim (s.cdfid, "Dimension_of_node_arrays", s.n_nodes, &dims), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Id_of_node_polygons", NC_INT, (size_t)1, &dims, &s.GSHHS_node_id), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, s.GSHHS_node_id, NC_CHUNKED, chunksize(s.n_nodes)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, s.GSHHS_node_id, 1, 1, 9), file);
	GMT_err_fail (nc_def_dim (s.cdfid, "Dimension_of_bin_arrays", s.n_bin, &dims), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Id_of_first_segment_in_a_bin", NC_INT, (size_t)1, &dims, &s.bin_firstseg_id), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, s.bin_firstseg_id, NC_CHUNKED, chunksize(s.n_bin)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, s.bin_firstseg_id, 1, 1, 9), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Embedded_node_levels_in_a_bin", NC_SHORT, (size_t)1, &dims, &s.bin_info_id), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, s.bin_info_id, NC_CHUNKED, chunksize(s.n_bin)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, s.bin_info_id, 1, 1, 9), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Embedded_node_levels_in_a_bin_ANT", NC_SHORT, (size_t)1, &dims, &bin_info_id_ANT), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, bin_info_id_ANT, NC_CHUNKED, chunksize(s.n_bin)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, bin_info_id_ANT, 1, 1, 9), file);
	GMT_err_fail (nc_def_var (s.cdfid, "N_segments_in_a_bin", NC_SHORT, (size_t)1, &dims, &s.bin_nseg_id), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, s.bin_nseg_id, NC_CHUNKED, chunksize(s.n_bin)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, s.bin_nseg_id, 1, 1, 9), file);

	GMT_err_fail (nc_def_dim (s.cdfid, "Dimension_of_segment_arrays", s.n_seg, &dims), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Embedded_npts_levels_exit_entry_for_a_segment", NC_INT, (size_t)1, &dims, &s.seg_info_id), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, s.seg_info_id, NC_CHUNKED, chunksize(s.n_seg)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, s.seg_info_id, 1, 1, 9), file);
	//GMT_err_fail (nc_def_var (s.cdfid, "Embedded_npts_source_levels_exit_entry_for_a_segment", NC_INT, (size_t)1, &dims, &s.seg_info_id), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Id_of_first_point_in_a_segment", NC_INT, (size_t)1, &dims, &s.seg_start_id), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, s.seg_start_id, NC_CHUNKED, chunksize(s.n_seg)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, s.seg_start_id, 1, 1, 9), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Id_of_GSHHS_ID", NC_INT, (size_t)1, &dims, &s.seg_GSHHS_ID_id), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, s.seg_GSHHS_ID_id, NC_CHUNKED, chunksize(s.n_seg)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, s.seg_GSHHS_ID_id, 1, 1, 9), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Embedded_ANT_flag", NC_BYTE, (size_t)1, &dims, &seg_info_ANT), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, seg_info_ANT, NC_CHUNKED, chunksize(s.n_seg)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, seg_info_ANT, 1, 1, 9), file);

	GMT_err_fail (nc_def_dim (s.cdfid, "Dimension_of_point_arrays", s.n_pt, &dims), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Relative_longitude_from_SW_corner_of_bin", NC_SHORT, (size_t)1, &dims, &s.pt_dx_id), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, s.pt_dx_id, NC_CHUNKED, chunksize(s.n_pt)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, s.pt_dx_id, 1, 1, 9), file);
	GMT_err_fail (nc_def_var (s.cdfid, "Relative_latitude_from_SW_corner_of_bin", NC_SHORT, (size_t)1, &dims, &s.pt_dy_id), file);
	GMT_err_fail (nc_def_var_chunking (s.cdfid, s.pt_dy_id, NC_CHUNKED, chunksize(s.n_pt)), file);
	GMT_err_fail (nc_def_var_deflate (s.cdfid, s.pt_dy_id, 1, 1, 9), file);

	/* assign attributes */

	/* Remember title is only 80 char long in GMT_SHORE */
	strcpy (s.title, "Derived from World Vector Shoreline, CIA WDB-II, and Atlas of the Cryosphere");
	sprintf (s.source, "Processed by Paul Wessel and Walter H. F. Smith, 1994-%d", YEAR);
	sprintf (s.version, "%s", GSHHG_NEW_VERSION);

	GMT_err_fail (nc_put_att_text (s.cdfid, s.pt_dx_id, "units", strlen(s.units), s.units), file);
	GMT_err_fail (nc_put_att_text (s.cdfid, s.pt_dy_id, "units", strlen(s.units), s.units), file);
	GMT_err_fail (nc_put_att_text (s.cdfid, NC_GLOBAL, "title", strlen(s.title), s.title), file);
	GMT_err_fail (nc_put_att_text (s.cdfid, NC_GLOBAL, "source", strlen(s.source), s.source), file);
	GMT_err_fail (nc_put_att_text (s.cdfid, NC_GLOBAL, "version", strlen(s.version), s.version), file);

	/* leave define mode */

        GMT_err_fail (nc_enddef (s.cdfid), file);

	start = 0;

        GMT_err_fail (nc_put_var1_int (s.cdfid, s.bin_size_id, &start, &s.bin_size), file);
        GMT_err_fail (nc_put_var1_int (s.cdfid, s.bin_nx_id, &start, &s.bin_nx), file);
        GMT_err_fail (nc_put_var1_int (s.cdfid, s.bin_ny_id, &start, &s.bin_ny), file);
        GMT_err_fail (nc_put_var1_int (s.cdfid, s.n_poly_id, &start, &s.n_poly), file);
        GMT_err_fail (nc_put_var1_int (s.cdfid, s.n_bin_id, &start, &s.n_bin), file);
        GMT_err_fail (nc_put_var1_int (s.cdfid, s.n_seg_id, &start, &s.n_seg), file);
        GMT_err_fail (nc_put_var1_int (s.cdfid, s.n_pt_id, &start, &s.n_pt), file);
        GMT_err_fail (nc_put_var1_int (s.cdfid, s.n_node_id, &start, &s.n_nodes), file);

	count = s.n_poly;

        GMT_err_fail (nc_put_vara_int(s.cdfid, s.GSHHS_parent_id, &start, &count, GSHHS_parent), file);
        GMT_err_fail (nc_put_vara_double(s.cdfid, s.GSHHS_area_id, &start, &count, GSHHS_area), file);
        GMT_err_fail (nc_put_vara_int(s.cdfid, s.GSHHS_areafrac_id, &start, &count, GSHHS_area_fraction), file);

	count = s.n_nodes;

        GMT_err_fail (nc_put_vara_int(s.cdfid, s.GSHHS_node_id, &start, &count, GSHHS_node), file);

	count = s.n_bin;

        GMT_err_fail (nc_put_vara_short(s.cdfid, s.bin_info_id, &start, &count, bin_info), file);
        GMT_err_fail (nc_put_vara_short(s.cdfid, bin_info_id_ANT, &start, &count, bin_info_ANT), file);
        GMT_err_fail (nc_put_vara_int(s.cdfid, s.bin_firstseg_id, &start, &count, bin_firstseg), file);
        GMT_err_fail (nc_put_vara_short(s.cdfid, s.bin_nseg_id, &start, &count, bin_nseg), file);

	count = s.n_seg;

        GMT_err_fail (nc_put_vara_int(s.cdfid, s.seg_info_id, &start, &count, seg_info), file);
        GMT_err_fail (nc_put_vara_int(s.cdfid, s.seg_start_id, &start, &count, seg_start), file);
        GMT_err_fail (nc_put_vara_int(s.cdfid, s.seg_GSHHS_ID_id, &start, &count, seg_GSHHS_id), file);
        GMT_err_fail (nc_put_vara_schar(s.cdfid, seg_info_ANT, &start, &count, seg_ANT), file);

	count = s.n_pt;

        GMT_err_fail (nc_put_vara_short(s.cdfid, s.pt_dx_id, &start, &count, pt_dx), file);
        GMT_err_fail (nc_put_vara_short(s.cdfid, s.pt_dy_id, &start, &count, pt_dy), file);

        GMT_err_fail (nc_close (s.cdfid), file);

	GMT_free ((void *)GSHHS_parent);
	GMT_free ((void *)GSHHS_area);
	GMT_free ((void *)GSHHS_area_fraction);

	GMT_free ((void *)GSHHS_node);

	GMT_free ((void *)bin_firstseg);
	GMT_free ((void *)bin_info_ANT);
	GMT_free ((void *)bin_info);
	GMT_free ((void *)bin_nseg);

	GMT_free ((void *)seg_info);
	GMT_free ((void *)seg_start);
	GMT_free ((void *)seg_GSHHS_id);
	GMT_free ((void *)seg_ANT);

	GMT_free ((void *)pt_dx);
	GMT_free ((void *)pt_dy);

	GMT_end (argc, argv);

	exit (EXIT_SUCCESS);
}
