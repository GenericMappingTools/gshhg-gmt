#!/bin/bash
res=c
B=/Users/pwessel/GMTdev/gshhg/trunk/bin
gmt convert -Dpolygon.%6.6d+o200000 -Td sv_islands_${res}.txt -fg --FORMAT_GEO_OUT=ddd.xxxxxx
gmt convert -Dpolygon.%6.6d+o300000 -Td sv_lakes_${res}.txt -fg --FORMAT_GEO_OUT=ddd.xxxxxx
ls polygon.200* | sed -e 's/polygon\.//g' > new_${res}1.lis
ls polygon.300* | sed -e 's/polygon\.//g' > new_${res}2.lis
$B/polygon_restore new_${res}1.lis ${res}1.b 1
$B/polygon_restore new_${res}2.lis ${res}2.b 2
cat ${res}1.b ${res}2.b > ${res}.b
rm -f polygon.* ${res}?.b
$B/polygon_consistency ${res}.b ${res}[12].lis
