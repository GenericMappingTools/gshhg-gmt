#!/bin/sh
# To shrink polygons are recalculate areas and number of points
. gmt_shell_functions.sh
FILE=$1
RES=$2
if [ "X${RES}" = "Xh" ]; then
	dist=0.2
elif [ "X${RES}" = "Xi" ]; then
	dist=1
elif [ "X${RES}" = "Xl" ]; then
	dist=5
elif [ "X${RES}" = "Xc" ]; then
	dist=25
fi
mkdir -p tmp_dir
gmtconvert $FILE -Dtmp_dir/segment_%4.4d.txt
cd tmp_dir
ls segment_*.txt > t.lis
let nout=0
let nin=0
Q=`gmt_ndatarecords t.lis`
if [ $Q -eq 1 ]; then
	G=3
else
	G=0
fi
while read file; do
	let nin=nin+1
	gmtsimplify -T${dist}k $file --FORMAT_GEO_OUT=+ddd.xxxxxx > tmp.txt
	area=`gmtspatial -fg -Qk -o2 tmp.txt`
	no=`awk '{print $4}' $file`
	S=`awk '{print $19}' $file`
	R=`awk '{print $22}' $file`
	A=`awk '{print $25}' $file`
	N=`gmt_ndatarecords tmp.txt`
	if [ $N -gt 3 ]; then
		echo "> Id = $no N = $N G = $G L = 1 E = 180 S = $S R = $R A = $A B = $area P = - F = $no"
		gmtconvert tmp.txt -fg --FORMAT_GEO_OUT=+ddd.xxxxxx | grep -v '^>'
		let nout=nout+1
	fi
done < t.lis
cd ..
rm -rf tmp_dir
echo "Processed $nin segments resulting in $nout shrunk segments" >&2
