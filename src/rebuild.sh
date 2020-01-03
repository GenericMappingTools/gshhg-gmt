#!/bin/sh
#
# 7 Steps for reupdating GSHHG after adding/removing polygons
# Must be run in the src directory
# Give the task number as the single argument or nothing to get the menu

if [ $# -eq 0 ]; then
	cat << EOF >&2
rebuild.sh - Update the GSHHG files following modifications

Give a task number to perform that tasks, choose from
	1 = Remove all old binary files
	2 = Recreate binary files from ASCII files
	3 = Sort polygons on size and renumber them
	4 = Redo the levels
	5 = Determine polygon hierarchy
	6 = Update all binary files
	7 = Recreate ASCII masters from the new binary files
EOF
	exit 0
fi

let choice=$1
if [ $choice -lt 1 ] || [ $choice -gt 7 ]; then
	echo "rebuild.sh: Bad task number, must be from 1-7" >&2
	exit -1
fi

# TASK 1: Remove old binary files
if [ $choice -eq 1 ]; then
	echo "rebuild.sh: CLEAN out old binary files"
	rm -f ../GSHHS/res_?/GSHHS_?_polygons*.b
fi

# 2. TASK 2: Rebuild the binary polygon files
if [ $choice -eq 2 ]; then
	echo "rebuild.sh: CREATE new binary files"
	for res in f h i l c; do
		../bin/ascii_to_binary C ${res} -V
	done
fi

# 3. TASK 3: Reorder based on area [gives polygons new ID numbers]
if [ $choice -eq 3 ]; then
	echo "rebuild.sh: SORT on polygon areas and renumber"
	for res in f h i l c; do
		../bin/polygon_sort ../GSHHS/res_${res}/GSHHS_${res}_polygons.b ../GSHHS/res_${res}/GSHHS_${res}_polygons_sorted.b
	done
fi

# 4. TASK 4: Determine the levels of each polygon for each resolution
if [ $choice -eq 4 ]; then
	echo "rebuild.sh: LEVEL all polygons"
	for res in f h i l c; do
		../bin/polygon_findlevel ../GSHHS/res_${res}/GSHHS_${res}_polygons_sorted.b ../GSHHS/res_${res}/GSHHS_${res}_polygons_sorted_level.b
	done
fi

# 5. TASK 5: Determine the hierarchy, who is parent, siblings in other resolutions
if [ $choice -eq 5 ]; then
	echo "rebuild.sh: HIERarchy determination of polygons"
	../bin/polygon_hierarchy ../GSHHS/res_f/GSHHS_f_polygons_sorted_level.b
fi

# 6. TASK 6: Read the new hierarchy file and update all binary polygon files
if [ $choice -eq 6 ]; then
	echo "rebuild.sh: SYNC hierarchy in binary files"
	../bin/polygon_sync ../GSHHS/res_f/GSHHS_f_polygons_sorted_level.b
fi

# 7. TASK 7: Recreate the ascii files from dir above GSHHS
if [ $choice -eq 7 ]; then
	cd ..
	echo "rebuild.sh: ARCHIVE new ascii files"
	for res in f h i l c; do
		bin/binary_to_ascii GSHHS/res_${res}/GSHHS_${res}_polygons_sorted_level.b C -V
	done
	cd src
fi
