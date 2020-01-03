psxy -R0/360/-90/-63 -JA0/-90/40i -T -K --PS_MEDIA=42ix42i > t.ps
#psxy -R -J -Gyellow `cat ocean_island_straddle.lis` -O -K >> t.ps
#psxy -R -J -O -K -Gcyan `cat ocean_islands.lis` >> t.ps
#psxy -R -J -O -K -Ggreen `cat ice_islands.lis` >> t.ps
psxy -R -J -O -K -Gorange `cat ice_islands_straddle.lis` >> t.ps
psxy -R -J -O antarctic_coastline.txt -W0.25p,blue -K >> t.ps
psxy -R -J -O -W0.25p,red antarctic_grounding_line.txt >> t.ps
ps2raster t.ps -Tf -P -A
open t.pdf
