#!/bin/bash
# Make updated global distance to coastline file from GSHHG 2.3.7
BIN=/Users/pwessel/GMTdev/gmt-dev/trunk/rbuild/gmt6/bin
#$BIN/gmt grdmath -R0/90/45/90      -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q1Nt.nc &
#$BIN/gmt grdmath -R0/90/0/45       -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q1Nb.nc &
#$BIN/gmt grdmath -R90/180/45/90    -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q2Nt.nc &
#$BIN/gmt grdmath -R90/180/0/45     -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q2Nb.nc &
#$BIN/gmt grdmath -R180/270/45/90   -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q3Nt.nc &
#$BIN/gmt grdmath -R180/270/0/45   -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q3Nb.nc &
#$BIN/gmt grdmath -R270/360/45/90   -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q4Nt.nc &
#$BIN/gmt grdmath -R270/360/0/45    -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q4Nb.nc &
#$BIN/gmt grdmath -R0/90/-90/-45    -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q1Sb.nc &
#$BIN/gmt grdmath -R0/90/-45/0      -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q1St.nc &
#$BIN/gmt grdmath -R90/180/-90/-45  -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q2Sb.nc &
#$BIN/gmt grdmath -R90/180/-45/0    -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q2St.nc &
#$BIN/gmt grdmath -R180/270/-90/-45 -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q3Sb.nc &
#$BIN/gmt grdmath -R180/270/-45/0   -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q3St.nc &
#$BIN/gmt grdmath -R270/360/-90/-45 -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q4Sb.nc &
#$BIN/gmt grdmath -R270/360/-45/0   -Vl -I1m -Df -A0/1/1 -fg --PROJ_GEODESIC=Vincenty LDISTG = q4St.nc &
#$BIN/grdlandmask -Rg -I1m -Df -A0/0/1 -N-1/1 -Gmask.nc
# WHen the above has finished we do these
#$BIN/gmt grdblend -Rg -I1m q[1234][NS][bt].nc -Gdist.nc -Vl
#$BIN/grdmath --IO_NC4_DEFLATION_LEVEL=9 dist.nc mask.nc MUL = dist_to_GSHHG_v2.3.7_1m.nc
# CORRECTIONS: DUe to the pole averaging bug that averaged the +/-45 degree rows and not the poles we
# Must cut out the 46-89 band and blend in with the new 45-46 and 89-90 bands
# First cut
#gmt grdcut q1Nt.nc -R0/90/46/89    -Gq1Ntband.nc
#gmt grdcut q2Nt.nc -R90/180/46/89  -Gq2Ntband.nc
#gmt grdcut q3Nt.nc -R180/270/46/89 -Gq3Nttband.nc
#gmt grdcut q4Nt.nc -R270/360/46/89 -Gq4Ntband.nc
#gmt grdcut q1Sb.nc -R0/90/-89/-46     -Gq1Sbband.nc
#gmt grdcut q2Sb.nc -R90/180/-89/-46   -Gq2Sbband.nc
#gmt grdcut q3Sb.nc -R180/270/-89/-46  -Gq3Sbband.nc
#gmt grdcut q4Sb.nc -R270/360/-89/-46  -Gq4Sbband.nc
$BIN/gmt grdblend -Rg -I1m q[1234]Nb.nc q[1234]St.nc q???band.nc q???45.nc q???90.nc -Gdist2.nc -Vl
#$BIN/grdmath --IO_NC4_DEFLATION_LEVEL=9 dist.nc mask.nc MUL = dist_to_GSHHG_v2.3.7_1m.nc
# Latest attempt is to use the OpenMP version and do it all in one go:
BIN=/Users/pwessel/GMTdev/gmt-dev/trunk/rbuild-mp/gmt6/bin
$BIN/gmt grdmath -Df -A0/1/1 -x20 --PROJ_GEODESIC=Vincenty LDISTG mask.nc MUL = dist_to_GSHHG_v2.3.7_1m.nc
