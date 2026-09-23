/*
 * gmt4_compat.h
 *
 * Thin compatibility layer that lets the old GMT4-era GSHHG tools build
 * against the GMT 6 development library (gmt_dev.h + libgmt).
 *
 * The GMT4 tools used global structures (project_info, gmtdefs) and
 * GMT_xxx functions without a session pointer.  Here a single GMT 6 session
 * is created on demand and exposed through the GMT macro, and the GMT4
 * names are mapped onto their GMT 6 equivalents (or local replacements).
 */

#ifndef GMT4_COMPAT_H
#define GMT4_COMPAT_H

/* We do not use any glib threading here; pretend gmt_glib.h was already
 * included so we do not need the glib headers (no struct depends on it). */
#ifndef GMT_GLIB_H
#define GMT_GLIB_H
#define GMT_xg_OPT " "
#define GMT_ADD_xg_OPT ""
#define GMT_declare_gmutex
#define GMT_set_gmutex
#define GMT_unset_gmutex
#endif

#include "gmt_dev.h"

/* ---------- Session handling ---------- */

extern struct GMT_CTRL *gshhg_get_ctrl(void);
extern int  gshhg_begin(int argc, char **argv);
extern void gshhg_end(void);

/* Every GMT4 global becomes a lookup in the (lazily created) GMT 6 session */
#define GMT		gshhg_get_ctrl()
#define GMT_begin(argc,argv)	gshhg_begin(argc, argv)
#define GMT_end(argc,argv)	gshhg_end()
#define GMT_program		gshhg_program
extern char *gshhg_program;

/* ---------- Memory ---------- */

extern void *gshhg_memory(void *prev_addr, size_t nelem, size_t size, const char *progname);
extern void  gshhg_free(void *addr);
#define GMT_memory(prev,n,size,prog)	gshhg_memory(prev, (size_t)(n), (size_t)(size), prog)
#define GMT_free(addr)			gshhg_free(addr)
#define GMT_memtrack_on(keeper)
#define GMT_memtrack_off(keeper)

/* ---------- Errors ---------- */

extern void gshhg_err_fail(int err, const char *file);
/* fopen that exits with a message instead of returning NULL */
extern FILE *gshhg_fopen(const char *file, const char *mode);
#define GMT_err_fail(err,file)	gshhg_err_fail(err, file)

/* ---------- Misc GMT4 names ---------- */

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif
#define VNULL	((void *)NULL)
#define CNULL	((char *)NULL)
#define GMT_LONG	long
#define GMT_CONV_LIMIT	GMT_CONV8_LIMIT
#define GMT_IS_ZERO(x)	(fabs(x) < GMT_CONV_LIMIT)
#define GMT_MAX_GSHHS_LEVEL	GSHHS_MAX_LEVEL
#define GMT_N_BLEVELS	GSHHS_N_BLEVELS
#define GMT_N_RLEVELS	GSHHS_N_RLEVELS
#define irint(x)	((int)lrint(x))
#define i_swap(x,y)	gmt_M_int_swap(x, y)

#define GMT_swab4(u)	bswap32((uint32_t)(u))

/* ---------- Geometry functions that now need the session pointer ---------- */

#define GMT_init_track(y,n,S)	gmt_init_track(GMT, y, (uint64_t)(n), S)
#define GMT_crossover(xa,ya,sa,A,na,xb,yb,sb,B,nb,internal,geo,X) \
	(int)gmt_crossover(GMT, xa, ya, sa, A, (uint64_t)(na), xb, yb, sb, B, (uint64_t)(nb), internal, geo, X)
#define GMT_x_free(X)		gmt_x_free(GMT, X)
#define GMT_non_zero_winding(xp,yp,x,y,n)	(int)gmt_non_zero_winding(GMT, xp, yp, x, y, (uint64_t)(n))
#define GMT_geo_to_cart(lat,lon,a,deg)	gmt_geo_to_cart(GMT, lat, lon, a, deg)
#define GMT_cart_to_geo(lat,lon,a,deg)	gmt_cart_to_geo(GMT, lat, lon, a, deg)
#define GMT_normalize3v(a)	gmt_normalize3v(GMT, a)
/* GMT4 returned the great circle distance in degrees */
#define GMT_great_circle_dist(x0,y0,x1,y1) \
	(gmt_great_circle_dist_meter(GMT, x0, y0, x1, y1) / GMT->current.proj.DIST_M_PR_DEG)
/* Free a GMT_XSEGMENT list allocated by gmt_init_track */
#define GMT_free_track(S)	gmt_M_free(GMT, S)

/* ---------- Grid helpers (replace GMT4 GMT_read_grd/GMT_write_grd) ---------- */

/* Write a gridline-registered float grid (no pad, row 0 = north) */
extern void gshhg_write_float_grid(const char *file, float *z, unsigned int nx, unsigned int ny,
	double wesn[], double inc, const char *title, const char *z_units);
/* Read a float grid into a newly allocated, unpadded array (row 0 = north) */
extern float *gshhg_read_float_grid(const char *file, unsigned int *nx, unsigned int *ny);

#endif /* GMT4_COMPAT_H */
