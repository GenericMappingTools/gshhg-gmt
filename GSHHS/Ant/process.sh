#!/bin/bash
# Check if any islands are fully inside or outside the grounding polygon
# and if anyone is straddling that line.  Same test for Antarctica polygon.
# We want to identify two populations:
#  A: Actual islands to be plotted with Antarctica polygon
#  B: Actual islands to be plotted with Grounding polygon
# B should be > A.
. gmt_shell_functions.sh

log=log.txt
let G=0
let A=0
let GS=0
let AS=0
echo "===> Test against Grounding polygon:" > $log
while read name; do
	echo "Process $name vs antarctic_grounding_line.txt"
	total=`gmt_nrecords islands/$name`
	gmtselect -fg -Fantarctic_grounding_line.txt islands/$name > tmp
	inside=`gmt_nrecords tmp`
	delta=`gmtmath -Q $total $inside SUB =`
	if [ $inside -eq 0 ]; then
		printf "%s : Outside Grounding polygon\n" $name >> $log
		let G=G+1
	elif [ $delta -eq 0 ]; then
		printf "%s : Inside Grounding polygon\n" $name >> $log
	else
		printf "%s : G %6.6d %6.6d\n" $name $total $inside >> $log
		let GS=GS+1
	fi
done < islands.lis
echo "===> Test against Antarctic polygon:" >> $log
while read name; do
	echo "Process $name vs antarctic_coastline.txt"
	total=`gmt_nrecords islands/$name`
	gmtselect -fg -Fantarctic_coastline.txt islands/$name > tmp
	inside=`gmt_nrecords tmp`
	delta=`gmtmath -Q $total $inside SUB =`
	if [ $inside -eq 0 ]; then
		printf "%s : Outside Antarctic polygon\n" $name >> $log
		let A=A+1
	elif [ $delta -eq 0 ]; then
		printf "%s : Inside Antarctic polygon\n" $name >> $log
	else
		printf "%s : A %6.6d %6.6d\n" $name $total $inside >> $log
		let AS=AS+1
	fi
done < islands.lis
echo "Results are in $log"
echo "Trouble polygons for Grounding: $GS"
echo "Trouble polygons for Antarctic: $AS"
