Got kmz/kml files from the Bohlander and Scambos (2007) via Atlas of the Cryosphere.
Converted these kml to txt, joined east and west halfs, skipped all the segment
markers that seems to serve no real purpose, and made three files:

antarctic_coastline.txt
antarctic_grounding_line.txt
antarctic_islands.txt

process.sh tested then how clearly inside/outside the islands were w.r.t. the two
coastline files. Turns out pretty good, and the minor crossings can be ignored,
i.e., islands crossing the coastline are 99% inside and should not be considered
islands when the coastline is used, but will be considered islands when the
grounding line is concerned.  Plots made by job.sh shows this clearly.
Two islands are inside grounding line and was removed:

islands/Ant_island_564.txt
islands/Ant_island_657.txt

These are the islands to include when coastline is used:
	ocean_islands.lis
These are the islands to include when grounding line is used:
	ocean_islands.lis
	ocean_island_straddle.lis + ice_islands.lis = grounding_islands.lis

# 0. First we made the 5 resolutions:
# Note: Part of the processing includes very mild D-P reduction using 50 m threshold;
# This only removes redundant points along straight lines.

#f: Mostly reformatting and computing areas etc
# PS: Must manually paste in GSHHG 4 header lines and set ID to 4 and 5
echo antarctic_coastline.txt > t.lis
build_seg.sh t.lis 2 > BS07_ANT_coastline_f.txt
echo antarctic_grounding_line.txt > t.lis
build_seg.sh t.lis 3 > BS07_ANT_groundingline_f.txt
build_seg.sh ocean_islands.lis 2 > BS07_ANT_islands_f.txt
build_seg.sh grounding_islands.lis 3 300328 >> BS07_ANT_islands_f.txt

# Had to do gmtconvert BS07_ANT_islands_f.txt -fg  -Dtmp_dir/segment_%4.4d.txt
# Then cd tmp_dir and run gmtspatial -Ie segment_0* > log
# Found 5 polygons that needed to be modified a tiny bit to prevent crossovers (tangents)
# These were 109, 288, 358, 370, 475; fixed in GE and new data updated in segment files

# Create the 4 other resolutons
for res in h i l c; do
	shrink_seg.sh BS07_ANT_coastline_f.txt ${res} > BS07_ANT_coastline_${res}.txt
	shrink_seg.sh BS07_ANT_groundingline_f.txt ${res} > BS07_ANT_groundingline_${res}.txt
	shrink_seg.sh BS07_ANT_islands_f.txt ${res} > BS07_ANT_islands_${res}.txt
done
# Above, h created one intersection; edited pol 112
# Above, i, l, c created no intersections

So we are ready to change GSHHG accordingly.

1. Extract and remove old Antarctic coastline and files from GSHHG
Found header records of all features south of 60S.  For each res_? we have
 14  res_c/Ant_remove_c.lis
628  res_f/Ant_remove_f.lis
625  res_h/Ant_remove_h.lis
517  res_i/Ant_remove_i.lis
 76  res_l/Ant_remove_l.lis

We  saved the original GSHHS Level 1 files in GSHHS_?_Level_1_2013.10.20.txt
and GSHHS_?_Antarctica_2013.10.20.txt in case of some disaster.
Then removed the old segments and appended the new ones from above.
The coastline_? files became the new GSHHS_?_Antarctica.txt while the grounding
lines became GSHHS_?_Antarctica_G.txt.  We committed and this became r566.

# Update the now 2 Antarctica polygons:
cp -f BS07_ANT_groundingline_f.txt ../res_f/GSHHS_f_Antarctica_G.txt
cp -f BS07_ANT_coastline_f.txt ../res_f/GSHHS_f_Antarctica.txt
cp -f BS07_ANT_groundingline_h.txt ../res_h/GSHHS_h_Antarctica_G.txt
cp -f BS07_ANT_coastline_h.txt ../res_h/GSHHS_h_Antarctica.txt
cp -f BS07_ANT_groundingline_i.txt ../res_i/GSHHS_i_Antarctica_G.txt
cp -f BS07_ANT_coastline_i.txt ../res_i/GSHHS_i_Antarctica.txt
cp -f BS07_ANT_groundingline_l.txt ../res_l/GSHHS_l_Antarctica_G.txt
cp -f BS07_ANT_coastline_l.txt ../res_l/GSHHS_l_Antarctica.txt
cp -f BS07_ANT_groundingline_c.txt ../res_c/GSHHS_c_Antarctica_G.txt
cp -f BS07_ANT_coastline_c.txt ../res_c/GSHHS_c_Antarctica.txt

# ALso changed all AUstralia polygons from ID = 5 to 6
# Then we appended the BS07_ANT_islands_?.txt files to the GSHHS_?_Level_1.txt file
@ that we had removed the old islands from.  Note: Had to change polygon ID from
# first items from 6 to 301000 to avoid conflict with grounding line pol ID.
cat BS07_ANT_islands_f.txt >> ../res_f/GSHHS_f_Level_1.txt
cat BS07_ANT_islands_h.txt >> ../res_h/GSHHS_h_Level_1.txt
cat BS07_ANT_islands_i.txt >> ../res_i/GSHHS_i_Level_1.txt
cat BS07_ANT_islands_l.txt >> ../res_l/GSHHS_l_Level_1.txt
cat BS07_ANT_islands_c.txt >> ../res_c/GSHHS_c_Level_1.txt
