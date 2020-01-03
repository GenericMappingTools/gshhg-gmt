#!/bin/bash
# New Data from Greenland. Credit to National Snow and Ice Data Center
#
# [Derived from MEaSUREs MODIS Mosaic of Greenland 2005 (MOG2005) Image Map]
# Downloaded 4 files from https://daacdata.apps.nsidc.org/pub/DATASETS/nsidc0547_MEASURES_mog2005_v1/shapefile/
# 1. Convert from shapefiles to gmt/ogr; must ignore failures
#ogr2ogr -skipfailures -f "OGR_GMT" mog100_geus_coastline_v1.1.gmt mog100_geus_coastline_v1.1.shp
# 2. Undo the map projection
#gmt mapproject  -Js-45/90/70/1:1 -R-180/180/30/90 -Fe -C -I mog100_geus_coastline_v1.1.gmt --PROJ_ELLIPSOID=WGS-84 > greenland_raw.txt
# 3a. Separate closed from open polygons
#gmt connect -fg greenland_raw.txt -Cgreenland_closed.txt > greenland_open.txt
# 3b. Connect open polygons using a 10 meter threshold
#gmt connect -fg greenland_open.txt -T10e > greenland_next.txt -V
# 3c. Extract the new closted polygons and add to those found before.
#gmt connect -fg greenland_next.txt -Cgreenland_closed2.txt > greenland_open2.txt
#cat greenland_closed2.txt >> greenland_closed.txt
# mv greenland_closed.txt greenland_full.txt
# 4. Make KML files for full resolution and compare to GSHHG existing.
#gmt 2kml greenland_full.txt -Fl -W1p,red > greenland_full.kml
#gmt pscoast -M -R286/349/59.5/84 -Df -W | gmt 2kml -Fl -W1p,green > greenland_full_old.kml
# 5. Create the lower-resolution versions - delete if empty
cat <<- EOF > res.lis
h	0.2
i	1
l	5
c	25
EOF
B=/Users/pwessel/GMTdev/gshhg/trunk/bin
# Stick to ~50m resolution for GSHHG full
#gmt simplify greenland_full.txt -T50e > greenland_f_1.txt
#gmt convert -Dpolygon.%6.6d+o200000 -Td greenland_f_1.txt -fg --FORMAT_GEO_OUT=ddd.xxxxxx
#ls polygon.* | awk -F. '{print $2}' > f1.lis
#$B/polygon_restore f1.lis new_f1.b 1
while read res dist; do
	gmt simplify greenland_f_1.txt -T${dist}k > greenland_${res}_1.txt
	if [ ! -s greenland_${res}_1.txt ]; then
		rm -f greenland_${res}_1.txt
	fi
done < res.lis
# THINGS TO DO:
# 1. No lakes so need to use existing WDBII lakes but these are very
#    misregistered.  We need to move them to fit the new shifts in coastlines
# 2. gmtsimplify probably created crossovers.  Need to process as explained in
#    src/GSHHG_updating.html
exit
R=286/349/59.5/84
#R=-20/-18.2/81.75/82.25
gmt pscoast -R$R -JM9ih -Df -Wfaint -K -Bafg -P > g.ps
gmt psxy -R -J greenland_f_1.txt -W0.25p,blue -O -K >> g.ps
gmt psxy -R -J -O -K lakes.txt -W0.25p,green >> g.ps
gmt psxy -R -J -O -T >> g.ps
gmt psconvert g.ps -Tf -P
xpdf g.pdf
# To find the old IDs we need to run many polygon_rect jobs and keep the unique ones.
# This is the list of w e s n to use
cat << greenland_box.lis
-58	-26	59	68
-60	-11	68	74
-74	-11	74	79
-60	-11	79	84
-70	-60	79	80
-68	-60	80	80.75
-67	-60	80.75	80.875
-66	-60	80.875	81.125
-64	-60	81.125	81.625
-62	-60	81.625	82
EOF
