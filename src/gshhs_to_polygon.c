/*
 * 
 *	read a GSHHG file and and write a polygon.b format file to stdout
 */

#include "wvs.h"
#include "gshhg/gshhg.h"

#define swabi4(i4) (((i4) >> 24) + (((i4) >> 8) & 65280) + (((i4) & 65280) << 8) + (((i4) & 255) << 24))

int main (int argc, char **argv)
{
	FILE	*fp_in;
	int	k;
	struct	LONGPAIR p;
	struct GMT3_POLY h;
	struct GSHHG_HEADER gshhs_header;
        
	if (argc != 2) {
		fprintf (stderr,"usage:  gshhs_to_polygon gshhs_res.b > polygonfile_res.b\n");
		exit (EXIT_FAILURE);
	}
	
	fp_in = fopen(argv[1], "rb");
		
	while (fread ((void *)&gshhs_header, sizeof (struct GSHHG_HEADER), 1, fp_in) == 1) {
#ifndef WORDS_BIGENDIAN
		/* Must swap header explicitly on little-endian machines */
		gshhs_header.id = swabi4 ((unsigned int)gshhs_header.id);
		gshhs_header.n  = swabi4 ((unsigned int)gshhs_header.n);
		gshhs_header.flag = swabi4 ((unsigned int)gshhs_header.flag);
		gshhs_header.west  = swabi4 ((unsigned int)gshhs_header.west);
		gshhs_header.east  = swabi4 ((unsigned int)gshhs_header.east);
		gshhs_header.south = swabi4 ((unsigned int)gshhs_header.south);
		gshhs_header.north = swabi4 ((unsigned int)gshhs_header.north);
		gshhs_header.area  = swabi4 ((unsigned int)gshhs_header.area);
#endif
		h.west = (double) gshhs_header.west * GSHHG_SCL;
		h.east = (double) gshhs_header.east * GSHHG_SCL;
		h.south = (double) gshhs_header.south * GSHHG_SCL;
		h.north = (double) gshhs_header.north * GSHHG_SCL;
		h.id = gshhs_header.id;
		h.n = gshhs_header.n;
		h.greenwich = (gshhs_header.flag >> 16) & 255;
		h.level = gshhs_header.flag & 255;
		h.source = (gshhs_header.flag >> 24) & 255;
		h.area = gshhs_header.area * 0.1;
		pol_writeheader (&h, stdout);
		for (k = 0; k < h.n; k++) {
			if (pol_fread (&p, 1, fp_in) != 1) {
				fprintf (stderr,"gshhs_to_polygon:  ERROR  reading file %s.\n", argv[1]);
				exit (EXIT_FAILURE);
			}
			if (pol_fwrite (&p, 1, stdout) != 1) {
				fprintf (stderr,"gshhs_to_polygon:  ERROR  writing to stdout.\n");
				exit (EXIT_FAILURE);
			}
		}
	}
		
	fclose (fp_in);

	exit (EXIT_SUCCESS);
}
