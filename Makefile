#	$Id: Makefile 682 2017-06-15 20:44:01Z pwessel $
#
#	Makefile for gshhg = GSHHS+WDBII Master Data Files
#
#	The GSHHG suite refers to the gmt includes and libraries
#	and makefile macros in the GMT4 directory.  This suite is used
#	to prepare coastline and river & border data.
#	To compile/link the programs, try "make all", then "make install".
#	When done, clean out directory with "make clean".
#
#	The following products can be built:
#
#	gshhg-gmt-<version>.tar.gz	NetCDF4 files for GMT
#	gshhg-bin-<version>.zip		Native binary polygons and lines [zip]
#	gshhg-shp-<version>.zip		Shapefiles of polygons and lines [zip]
#
#	Normal sequence of events would be:
#	make update build-all
#
#	Note: These targets require GMT 4.
#
#	Author:	Paul Wessel, SOEST, U. of Hawaii
#
#	Date:	29-DEC-2014
#
#
#
#-------------------------------------------------------------------------------
#	!! STOP EDITING HERE, THE REST IS FIXED !!
#-------------------------------------------------------------------------------

include config.mk.orig		# Default settings
sinclude config.mk		# Over-ride with user settings
include $(GMTSRCDIR)config.mk
include $(GMTSRCDIR)common.mk
# Prefix of tar/zip balls:
TAG	= gshhg
#-------------------------------------------------------------------------------

help::
		@grep '^#!' Makefile | cut -c3-
#!-------------------- MAKE HELP FOR GSHHS+WDBII --------------------
#!
#!make <target>, where <target> can be:
#!
#!all             : Compile programs in src required for building datasets
#!install         : Install programs in bin directory
#!uninstall       : Uninstall the programs from bin
#!update          : Check repository from the latest updates
#!binary          : Build the native binary files from ASCII counterparts
#!data            : Build the coastline, border, and river binary files
#!build-all       : Build all the 5 release products (listed below)
#!tar_gshhg_nc    : Create tarball of GSHHG for GMT distribution (deflated netCDF-4)
#!zip_gshhg_bin   : Create tarzip archive of GSHHG polygons/lines (binary)
#!zip_gshhg_shp   : Create tarzip archive of GSHHG polygons/lines (shapefiles)
#!place           : Uses scp to copy the files over to SOEST ftp and http sites and the GMT website
#!checksum        : Compute MD5 checksum for the tar.gz file
#!spotless        : Clean up and remove created files of all types
#!
#!	REMEMBER: GSHHG BUILDING STILL REQUIRES GMT 4 [gmtswitch gmt4]
#!

all:
	cd src ; $(MAKE) all

install:
	cd src ; $(MAKE) install

uninstall:
	cd src ; $(MAKE) uninstall

clean:
	cd src ; $(MAKE) clean

spotless:	uninstall clean
	cd src ; $(MAKE) spotless
	rm -f GSHHS/res_?/GSHHS_?_polygons.b WDBII/borders/WDBII_Borders_segments.b WDBII/rivers/WDBII_Rivers_segments.b
	rm -rf src/res_?
	rm -rf $(TAG)-shp-$(GSHHG_NEW_VERSION).zip $(TAG)-bin-$(GSHHG_NEW_VERSION).zip $(TAG)-gmt-$(GSHHG_NEW_VERSION).tar.gz

# Note: coastline files now stored relative to share, instead of GMT/share

.PHONY:		checksum place

build-all:	install tar_gshhg_nc zip_gshhg_bin zip_gshhg_shp

checksum:
		md5sum $(TAG)-gmt-$(GSHHG_NEW_VERSION).tar.gz | awk '{printf "Update gshhg.info with the new check sum: %s\n", $$1}'

tar_gshhg_nc:	data
		echo "make $(TAG)-gmt-$(GSHHG_NEW_VERSION).tar.gz"
		rm -f $(TAG)-gmt-$(GSHHG_NEW_VERSION).tar.gz
		mkdir $(TAG)-gmt-$(GSHHG_NEW_VERSION)
		echo $(GSHHG_NEW_VERSION) > $(TAG)-gmt-$(GSHHG_NEW_VERSION)/VERSION
		cd $(TAG)-gmt-$(GSHHG_NEW_VERSION); ln -sf \
				../src/res_f/binned_GSHHS_f.nc \
				../src/res_f/binned_border_f.nc \
				../src/res_f/binned_river_f.nc \
				../src/res_h/binned_GSHHS_h.nc \
				../src/res_h/binned_border_h.nc \
				../src/res_h/binned_river_h.nc \
				../src/res_i/binned_GSHHS_i.nc \
				../src/res_i/binned_border_i.nc \
				../src/res_i/binned_river_i.nc \
				../src/res_l/binned_GSHHS_l.nc \
				../src/res_l/binned_border_l.nc \
				../src/res_l/binned_river_l.nc \
				../src/res_c/binned_GSHHS_c.nc \
				../src/res_c/binned_border_c.nc \
				../src/res_c/binned_river_c.nc \
				../README.TXT \
				../LICENSE.TXT \
				../COPYINGv3 \
				../COPYING.LESSERv3 .
		COPYFILE_DISABLE=true $(GNUTAR) --owner 0 --group 0 --mode a=rX,u=rwX -czhvf $(TAG)-gmt-$(GSHHG_NEW_VERSION).tar.gz $(TAG)-gmt-$(GSHHG_NEW_VERSION)
		rm -rf $(TAG)-gmt-$(GSHHG_NEW_VERSION)
		chmod og+r $(TAG)-gmt-$(GSHHG_NEW_VERSION).tar.gz

zip_gshhg_bin:	data build-gshhs
		echo "make $(TAG)-bin-$(GSHHG_NEW_VERSION).zip"
		rm -f $(TAG)-bin-$(GSHHG_NEW_VERSION).zip
		zip -r -9 -l -q $(TAG)-bin-$(GSHHG_NEW_VERSION).zip README.TXT LICENSE.TXT COPYING.LESSERv3 COPYING.LESSERv3
		cd src/gshhs; zip -r -g -9 -q ../../$(TAG)-bin-$(GSHHG_NEW_VERSION).zip *_[clihf].b
		chmod og+r $(TAG)-bin-$(GSHHG_NEW_VERSION).zip

zip_gshhg_shp:	shapefiles
		echo "make $(TAG)-shp-$(GSHHG_NEW_VERSION).zip"
		rm -f $(TAG)-shp-$(GSHHG_NEW_VERSION).zip
		zip -r -9 -l -q $(TAG)-shp-$(GSHHG_NEW_VERSION).zip README.TXT SHAPEFILES.TXT LICENSE.TXT COPYING.LESSERv3 COPYING.LESSERv3
		cd src ; zip -r -g -q -v -9 ../$(TAG)-shp-$(GSHHG_NEW_VERSION).zip GSHHS_shp/[clihf]/*
		cd src ; zip -r -g -q -v -9 ../$(TAG)-shp-$(GSHHG_NEW_VERSION).zip WDBII_shp/[clihf]/*
		cd src ; zip -r -9 -l -q ../$(TAG)-shp-$(GSHHG_NEW_VERSION).zip GSHHS_shp/README.TXT WDBII_shp/README.TXT
		chmod og+r $(TAG)-shp-$(GSHHG_NEW_VERSION).zip

place:
		echo "Place $(GSHHG_NEW_VERSION) tar file on GMT ftp site $(GMT_FTPSITE)"
		scp $(TAG)-gmt-$(GSHHG_NEW_VERSION).tar.gz $(GMT_FTPSITE)
		echo "Place $(GSHHG_NEW_VERSION) files on ftp site $(GSHHG_FTPSITE)"
		scp $(TAG)-gmt-$(GSHHG_NEW_VERSION).tar.gz $(TAG)-bin-$(GSHHG_NEW_VERSION).zip $(TAG)-shp-$(GSHHG_NEW_VERSION).zip $(GSHHG_FTPSITE)
		echo "Place $(GSHHG_NEW_VERSION) files on http site $(GSHHG_WWWSITE)"
		scp $(TAG)-gmt-$(GSHHG_NEW_VERSION).tar.gz $(TAG)-bin-$(GSHHG_NEW_VERSION).zip $(TAG)-shp-$(GSHHG_NEW_VERSION).zip $(GSHHG_WWWSITE)
		echo "Place $(GSHHG_NEW_VERSION) tar file on GMT web site $(GMT_WWWSITE)"
		scp $(TAG)-gmt-$(GSHHG_NEW_VERSION).tar.gz gmtdaemon@$(GMT_WWWSITE)

#-------------------------------------------------------------------------------
#	Data activities
#-------------------------------------------------------------------------------

update:
		svn update

binary:
		cd src ; $(MAKE) binary

data:
		cd src ; $(MAKE) data

build-gshhs:
		cd src ; $(MAKE) build-gshhs

shapefiles:
		cd src; $(MAKE) shape-gshhs shape-wdbii
