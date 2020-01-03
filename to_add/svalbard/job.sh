# Use 0.04 km2 for islands and 0.1 km2 for lakes
gmt spatial -Qe+h+p+sd+c10000 -V svalbard_land_full.txt > sv_islands_full.txt
gmt spatial -Qe+h+p+sd+c100000 -V svalbard_lake_full.txt > sv_lakes_full.txt
gmt convert -Dpolygon.%6.6d+o200000 sv_islands_full.txt -fg --FORMAT_GEO_OUT=ddd.xxxxxx
gmt convert -Dpolygon.%6.6d+o300000 sv_lakes_full.txt -fg --FORMAT_GEO_OUT=ddd.xxxxxx

gmt pscoast -R10/34/74/81 -JM7i -P -Bafg1 -Df -Glightgray -K -Xc > t.ps
gmt psxy -R -J -O -K -W0.25p,red sv_islands_full.txt >> t.ps
#gmt psxy -R -J -O -K -Gred jm_islands_full.txt >> t.ps
gmt psxy -R -J -O -K -Gblue sv_lakes_full.txt >> t.ps
gmt psxy -R -J -O -T >> t.ps
gmt psconvert -Tf -P -Z t.ps
open t.pdf
