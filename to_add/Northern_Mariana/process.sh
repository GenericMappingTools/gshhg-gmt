#!/bin/bash
#Data from doug.graham@noaa.gov CUPS for N Mariana Islands
# Clearly superior to GSHHG. Large offsets in GSHHG for the smaller islands
# Plan:
# 1. Identify and remove old GSHHG versions of all these items
# 2. Produce h,i,l,c versions of all polygons
# 3. Deal with any crossings of polygons.  It is likely that some of the
#	detailed harbor areas will produce crossings and many tiny features
#	will not survive going to high resolution.
# 3. Run through the rebuild stages.

# GMT4 analysis
# Find old version IDs:
v4=0
v5=1
if [ $v4 -eq 1 ]; then
	polygon_rect res_f/GSHHS_f_polygons.b 144.5 146.1 13.2 20.6 > old.lis
	cat <<- EOF > res.lis
	h       0.2
	i       1
	l       5
	c       25
	EOF
	while read res dist; do
	        gmt simplify -T${dist}k CUSP_Mariana_Polyg.gmt > USP_Mariana_Polyg_${res}.txt
	done < t.lis
fi

# GMT5 plotting
#gmt simplify -T50e CUSP_Mariana_Polyg.gmt > s.txt
#gmt spatial -Sh s.txt > f.txt
# Delete a duplicate hole
if [ $v5 -eq 1 ]; then
	cat <<- EOF > res.lis
	h	0.2
	i	1
	l	5
	c	25
	EOF
	W=30
	H=`gmt mapproject -R144/147/13/21 -JM${W}i -Wh -Di`
	PX=`gmt math -Q $W 2 ADD =`
	PY=`gmt math -Q $H 2 ADD =`
	while read res dist; do
		gmt simplify -T${dist}k f.txt -fg > $res.txt
		gmt pscoast -R144/147/13/21 -JM${W}i -P -Glightgray -D${res} -Baf --PS_MEDIA=${PX}ix${PY}i -K > ${res}.ps
		gmt psxy -R -J -O -K -Wfaint $res.txt >> ${res}.ps
		gmt psxy -R -J -O -T >> ${res}.ps
		gmt psconvert ${res}.ps -Tf -P
		open ${res}.pdf
	done < res.lis
fi
