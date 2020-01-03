#ifndef _WVS_H_
#define MILL (1000000)
#define GSHHS_MAX_DELTA 65535				/* Largest value to store in a ushort, used as largest dx or dy in bin  */
#define M360 360000000
#define M180 180000000
#define M90   90000000

typedef unsigned short ushort;
#endif

struct SEGMENT {		/* NOT 64-bit OK to read/write as is */
	int GSHHS_ID;		/* Original GSHHS ID for the polygon that this segment is part of */
	int level;		/* 1-4 for regular features, plus level = 6 for Antarctic gorund line polygons */
	int n;
	int entry, exit;		/* 4 for a poly completely inside bin; else w,e,s,n = 3,1,0,2  */
	struct SEGMENT *next_seg;
	struct SHORT_PAIR *p;		/* A chain of x,y points is tacked on here  */
};

struct SHORT_PAIR {		/* 64-bit OK for i/o */
	ushort	dx;		/* Relative distance from SW corner of bin, units of B_WIDTH/GSHHS_MAX_DELTA  */
	ushort	dy;
};

struct SEGMENT_HEADER {		/* 64-bit OK for i/o */
	int GSHHS_ID;		/* Original GSHHS ID for the polygon that this segment is part of */
	int GSHHS_parent;	/* ID of polygon that the GSHHS_id polygon is contained by (-1 if level == 1) */
	int info;		/* Combination of n, entry, exit, level, source */
	int first_p;		/* Id of first point */
};

struct GMT3_FILE_HEADER {	/* 64-bit OK for i/o */
	int n_bins;		/* Number of blocks */
	int n_points;		/* Total number of points */
	int bsize;		/* Bin size in minutes */
	int nx_bins;		/* # of bins in 0-360 */
	int ny_bins;		/* # of bins in -90 - +90 */
	int n_segments;		/* Total number of segments */
};

struct GMT3_BIN_HEADER {	/* 64-bit OK for i/o */
	int first_seg_id;
	short n_segments;
	short node_levels;	/* Stores the level of the four corner per bin */
	short node_levels_ANT;	/* Stores the level of the four corner when Antarctica grounding line is in effect */
};

static inline size_t * chunksize (size_t size) {
	/* For optimal performance, set the number of elements in a given chunk
	 * dimension (n) to be the ceiling of the number of elements in that
	 * dimension of the array variable (d) divided by a natural number N>1.
	 * That is, set n = ceil (d / Ν).  Using a chunk size slightly larger than
	 * this value is also acceptable.  For example: 129 = ceil (257 / 2).
	 * Do NOT set n = floor (d / N), for example 128 = floor (257 / 2). */

	size_t chunksize = 32768; /* default chunk size */
	static size_t final_chunksize; /* returned chunk size */

	if (size < 2 * chunksize)
		/* reduce chunk size */
		chunksize = size;
	else
		/* determine optimal chunk size in the range [chunksize,2*chunksize) */
		chunksize = (size_t) ceil (size / floor ((double)size / chunksize));

	final_chunksize = chunksize;
	return &final_chunksize;
}
