#!/bin/sh
. gmt_shell_functions.sh
# We must determine area, region, np etc for each island and place in header
LIS=$1
SRC=$2
# Set start ID number to something >> highest actual ID; these will be reset later
if [ $# -eq 3 ]; then
	let no=$3
else
	let no=300000
fi
echo "First ID is $no, SRC = $SRC" >&2
Q=`gmt_ndatarecords $LIS`
if [ $Q -eq 1 ]; then
	G=3
else
	G=0
fi
while read file; do
	gmtsimplify -T50e $file > tmp.txt
	area=`gmtspatial -fg -Qk -o2 tmp.txt`
	if [ $Q -eq 1 ]; then
		Y=`minmax -fg -C --FORMAT_GEO_OUT=+ddd.xxxxxx tmp.txt -o3`
		R=0.000000/360.000000/-90.000000/$Y
	else
		R=`minmax -fg -C --FORMAT_GEO_OUT=+ddd.xxxxxx --IO_COL_SEPARATOR=/ tmp.txt`
	fi
	N=`gmt_ndatarecords tmp.txt`
	echo "> Id = $no N = $N G = $G L = 1 E = 180 S = $SRC R = $R A = $area B = $area P = - F = -"
	gmtconvert tmp.txt -fg --FORMAT_GEO_OUT=+ddd.xxxxxx | grep -v '^>'
	let no=no+1
done < $LIS
echo "Next ID is $no" >&2
