#!/bin/bash
# We are only considering replaceing old data in FULL resolution.
# These are changes done post GSHHG 2.3.7 which was the final version.
# Find old version IDs:
v4=0
v5=1
if [ $v5 -eq 1 ]; then

# New data for Svalbard.  Credit to Norwegian Polar Institute
#
# Downloaded from https://data.npolar.no/dataset/63730e2e-b7a6-4d14-b341-c661ccdc5254
#SCALE=100
# 1. Convert from shapefiles to gmt/ogr
#ogr2ogr -f "OGR_GMT" S${SCALE}_Land_f.gmt S${SCALE}_Land_f.shp
#ogr2ogr -f "OGR_GMT" S${SCALE}_Vann_f.gmt S${SCALE}_Vann_f.shp
# 2. Undo the UTM projection
#gmt mapproject  -Ju33N/1:1 -R14/16/76/78 -Fe -C -I --PROJ_SCALE_FACTOR=0.9996 --PROJ_ELLIPSOID=GRS-80 S${SCALE}_Land_f.gmt > svalbard_land_full.txt
#gmt mapproject  -Ju33N/1:1 -R14/16/76/78 -Fe -C -I --PROJ_SCALE_FACTOR=0.9996 --PROJ_ELLIPSOID=GRS-80 S${SCALE}_Vann_f.gmt > svalbard_lake_full.txt
# 3. Make KML files for full resolution and compare to GSHHG existing.
#gmt 2kml svalbard_land_full.txt -Fl -W1p,red > svalbard_land_full.kml
#gmt 2kml svalbard_lake_fuqll.txt -Fl -W1p,yellow > svalbard_lake_full.kml
#gmt pscoast -M -R-10/-7/70/72 -Df -W | gmt 2kml -Fl -W1p,green > svalbard_land_full_old.kml
# 4. Create the lower-resolution versions - delete if empty
# Use 0.04 km2 for islands and 0.1 km2 for lakes
	gmt spatial -Qe+h+p+sd+c10000 -V svalbard_land_full.txt > sv_islands_f.txt
	gmt spatial -Qe+h+p+sd+c100000 -V svalbard_lake_full.txt > sv_lakes_f.txt
	gmt convert -Dpolygon.%6.6d+o200000 -Td sv_islands_f.txt -fg --FORMAT_GEO_OUT=ddd.xxxxxx
	gmt convert -Dpolygon.%6.6d+o300000 -Td sv_lakes_f.txt -fg --FORMAT_GEO_OUT=ddd.xxxxxx
fi

if [ $v4 -eq 1 ]; then
	B=/Users/pwessel/GMTdev/gshhg/trunk/bin
	$B/polygon_rect res_f/GSHHS_f_polygons.b 10 34 74 81 > res_f/old_f.lis
fi
rm -f res.lis
