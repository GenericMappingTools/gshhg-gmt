#
#		Makefile for GSHHG src directory
#
#	This suite is used to prepare GSHHS geographic data.
#	Use the main Makefile in the top directory
#	When done, clean out directory with "make clean".
#
#	Author:	Paul Wessel, SOEST, U. of Hawaii
#
#	Date:	27-APR-2016
#
#	Note: Most of these tools were last used in 1995/96 and only
#	minor changes to make them compile have been performed.
#	Several are probably obsolete.
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

include ../config.mk.orig	# Default settings
sinclude ../config.mk		# Over-ride with user settings
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk

INCLUDES	= -I$(srcdir) $(NETCDF_INC) $(GDAL_INC)
DEFINES		= -DGSHHG_NEW_VERSION=\"$(GSHHG_NEW_VERSION)\" -DGSHHG_COMPATIBILITY_VERSION=$(GSHHG_COMPATIBILITY_VERSION) -DYEAR=`date +%Y`
bindir		= ../bin
# Douglas-Peucker parameters:
DP_h 		= 0.2
DP_i		= 1
DP_l		= 5
DP_c		= 25
# Bin size in degrees:
BIN_f		= 1
BIN_h		= 2
BIN_i		= 5
BIN_l		= 10
BIN_c		= 20
# Get dependencies for data files:
include coast.dep

PROGS_H	= shore.h wvs.h

PROGS_O	= border_to_bins.o ciaman_fix.o line_shrink.o linemaker.o swap.o polygon_close.o  \
	  lines_to_bins.o man_fix.o man_fix2.o polygon_bincount.o gshhs_to_polygon.o polygon_to_shape.o \
	  polygon_checkarea.o polygon_consistency.o polygon_deldups.o polygon_compare.o \
	  polygon_dump.o polygon_extract.o polygon_extract2.o polygon_fixnegwesn.o polygon_setarea.o \
	  polygon_extract_all.o polygon_final_info.o polygon_findarea.o polygon_findlevel.o \
	  polygon_fix.o polygon_fixlevel.o polygon_get.o polygon_in_pol.o polygon_rect.o polygon_sync.o \
	  polygon_id.o polygon_merge.o polygon_report.o polygon_restore.o  segment_restore.o \
	  polygon_set.o polygon_setnodes.o polygon_setwesn.o polygon_shrink.o polygon_match.o\
	  polygon_sort.o polygon_stats.o polygon_to_bins.o polygon_to_gshhs.o polygon_hierarchy.o \
	  polygon_update.o polygon_xover.o read_wvs.o river_fix.o segment_clean.o \
	  segment_connect.o segment_dump.o segment_final_dump.o segment_report.o \
	  shoremaker.o wvs_crosscheck.o wvs_segment_dump.o wvs_segment_restore2.o \
	  ascii_to_binary.o binary_to_ascii.o polygon_skipdups.o

LIB	= libcoast.a

#-------------------------------------------------------------------------------
#	software targets
#-------------------------------------------------------------------------------

all:		$(PROGS)

install:	all
		mkdir -p $(bindir)
		$(INSTALL) $(PROGS) $(bindir)

uninstall:
		rm -rf $(bindir)

clean:
		rm -f *.o *% $(PROGS)

spotless:	clean
		rm -f $(LIB) shit.grd
		rm -rf gshhs wdb GSHHS_shp WDBII_shp

# Data targets:

data:		full high int low crude

info:		info_f info_h info_i info_l info_c

set_new:
		cp -f res_?/binned_*_?.cdf $(rootdir)/share/coast

delete:		
		rm res_?/binned_*_?.{bin,seg,pt,pol,ndid}

build-gshhs:	update wdbfiles
		mkdir -p gshhs
		$(bindir)/polygon_to_gshhs res_f/GSHHS_f_polygons.b > gshhs/gshhs_f.b
		$(bindir)/polygon_to_gshhs res_h/GSHHS_h_polygons.b > gshhs/gshhs_h.b
		$(bindir)/polygon_to_gshhs res_i/GSHHS_i_polygons.b > gshhs/gshhs_i.b
		$(bindir)/polygon_to_gshhs res_l/GSHHS_l_polygons.b > gshhs/gshhs_l.b
		$(bindir)/polygon_to_gshhs res_c/GSHHS_c_polygons.b > gshhs/gshhs_c.b
		$(bindir)/polygon_to_gshhs -l ../WDBII/borders/WDBII_Borders_segments.b > gshhs/wdb_borders_f.b
		$(bindir)/polygon_to_gshhs -l wdb/wdb_h_borders.b > gshhs/wdb_borders_h.b
		$(bindir)/polygon_to_gshhs -l wdb/wdb_i_borders.b > gshhs/wdb_borders_i.b
		$(bindir)/polygon_to_gshhs -l wdb/wdb_l_borders.b > gshhs/wdb_borders_l.b
		$(bindir)/polygon_to_gshhs -l wdb/wdb_c_borders.b > gshhs/wdb_borders_c.b
		$(bindir)/polygon_to_gshhs -l ../WDBII/rivers/WDBII_Rivers_segments.b > gshhs/wdb_rivers_f.b
		$(bindir)/polygon_to_gshhs -l wdb/wdb_h_rivers.b > gshhs/wdb_rivers_h.b
		$(bindir)/polygon_to_gshhs -l wdb/wdb_i_rivers.b > gshhs/wdb_rivers_i.b
		$(bindir)/polygon_to_gshhs -l wdb/wdb_l_rivers.b > gshhs/wdb_rivers_l.b
		$(bindir)/polygon_to_gshhs -l wdb/wdb_c_rivers.b > gshhs/wdb_rivers_c.b

shape-gshhs:	update
		rm -rf GSHHS_shp GSHHS_[fhilc]_L[123456]
		mkdir -p GSHHS_shp GSHHS_shp/f GSHHS_shp/h GSHHS_shp/i GSHHS_shp/l GSHHS_shp/c
		$(bindir)/polygon_to_shape res_f/GSHHS_f_polygons.b GSHHS_f
		$(bindir)/polygon_to_shape res_h/GSHHS_h_polygons.b GSHHS_h
		$(bindir)/polygon_to_shape res_i/GSHHS_i_polygons.b GSHHS_i
		$(bindir)/polygon_to_shape res_l/GSHHS_l_polygons.b GSHHS_l
		$(bindir)/polygon_to_shape res_c/GSHHS_c_polygons.b GSHHS_c
		mv GSHHS_f_L[1-6]/* GSHHS_shp/f
		mv GSHHS_h_L[1-6]/* GSHHS_shp/h
		mv GSHHS_i_L[1-6]/* GSHHS_shp/i
		mv GSHHS_l_L[1-6]/* GSHHS_shp/l
		mv GSHHS_c_L[1-6]/* GSHHS_shp/c
		rm -rf GSHHS_[fhilc]_L[123456] GSHHS_[fhilc]_L[123456].gmt
		cp ../README.TXT GSHHS_shp
		cp ../ChangeLog GSHHS_shp

shape-wdbii:	update wdbfiles
		rm -rf WDBII_shp WDBII_border_[fhilc]_L[1-3] WDBII_river_[fhilc]_L[01][0-9]
		mkdir -p WDBII_shp WDBII_shp/f WDBII_shp/h WDBII_shp/i WDBII_shp/l WDBII_shp/c
		$(bindir)/polygon_to_shape ../WDBII/borders/WDBII_Borders_segments.b WDBII_border_f -o
		$(bindir)/polygon_to_shape wdb/wdb_h_borders.b WDBII_border_h -o
		$(bindir)/polygon_to_shape wdb/wdb_i_borders.b WDBII_border_i -o
		$(bindir)/polygon_to_shape wdb/wdb_l_borders.b WDBII_border_l -o
		$(bindir)/polygon_to_shape wdb/wdb_c_borders.b WDBII_border_c -o
		mv WDBII_border_f_L[1-3]/* WDBII_shp/f
		mv WDBII_border_h_L[1-3]/* WDBII_shp/h
		mv WDBII_border_i_L[1-3]/* WDBII_shp/i
		mv WDBII_border_l_L[1-3]/* WDBII_shp/l
		mv WDBII_border_c_L[1-3]/* WDBII_shp/c
		rm -rf WDBII_border_[fhilc]_L[1-3] WDBII_border_[fhilc]_L[1-3].gmt
		$(bindir)/polygon_to_shape ../WDBII/rivers/WDBII_Rivers_segments.b WDBII_river_f -i
		$(bindir)/polygon_to_shape wdb/wdb_h_rivers.b WDBII_river_h -i
		$(bindir)/polygon_to_shape wdb/wdb_i_rivers.b WDBII_river_i -i
		$(bindir)/polygon_to_shape wdb/wdb_l_rivers.b WDBII_river_l -i
		$(bindir)/polygon_to_shape wdb/wdb_c_rivers.b WDBII_river_c -i
		mv WDBII_river_f_L[01][0-9]/* WDBII_shp/f
		mv WDBII_river_h_L[01][0-9]/* WDBII_shp/h
		mv WDBII_river_i_L[01][0-9]/* WDBII_shp/i
		mv WDBII_river_l_L[01][0-9]/* WDBII_shp/l
		mv WDBII_river_c_L[01][0-9]/* WDBII_shp/c
		rm -rf WDBII_river_[fhilc]_L[01][0-9] WDBII_river_[fhilc]_L[01][0-9].gmt
		cp ../README.TXT ../ChangeLog WDBII_shp

#-------------------------------------------------------------------------------
#	object file dependencies
#-------------------------------------------------------------------------------

$(PROGS_O):	$(PROGS_H)
coast.o:	$(PROGS_H)

#-------------------------------------------------------------------------------
#	library
#-------------------------------------------------------------------------------

lib:		$(LIB)

$(LIB):		poly_check_subs.o string_check_subs.o douglas_peucker.o coast_io.o poly_misc_subs.o
		$(AR) cvr $@ $?
		$(RANLIB) $@

#-------------------------------------------------------------------------------
#	program rules
#-------------------------------------------------------------------------------

$(PROGS):	$(LIB) $(PROGS_O)
		$(CC) $(LDFLAGS) $(@:.exe=).o -L. -lcoast $(GMT_LIB) $(NETCDF_LIB) $(GDAL_LIB) $(LIBS) -o $@
		$(COMPRESS) $@
