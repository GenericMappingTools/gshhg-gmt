#!/bin/sh
# Takes four args: 1 = infilename, 2 = deltalon to add, 3 = delta_lat to add, 4 = newfile
#
# Initially used to pply the Tahiti -0.005 longitude shift to given file
# But -0.0155 was needed so take out -0.0105 more for a total of -0.0155

echo "Add dlon = $2 and dlat = $3 to file $1 with updated data in file $4"
gmtmath -fg $1 -C0 $2 ADD -C1 $3 ADD -Ca = | gmtconvert -fg --FORMAT_GEO_OUT=+D --FORMAT_FLOAT_OUT=%.6f > $4
minmax -fg --FORMAT_GEO_OUT=+D --FORMAT_FLOAT_OUT=%.6f $4
