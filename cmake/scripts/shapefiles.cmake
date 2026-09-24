#
# Build the GSHHS or WDBII shapefiles (was the shape-gshhs/shape-wdbii make targets):
#
#   cmake -DMODE=gshhs|wdbii -DPROG=<polygon_to_shape> -DWD=<src dir> -DDOCS="a|b" -P shapefiles.cmake
#
# polygon_to_shape writes <prefix>_L<level>.gmt files and calls ogr2ogr to turn
# each into a directory <prefix>_L<level> with the shapefile; those are then
# collected into GSHHS_shp/<res> or WDBII_shp/<res>.
#

find_program(OGR2OGR ogr2ogr)
if (NOT OGR2OGR)
	message(FATAL_ERROR "ogr2ogr (GDAL) must be in the PATH to create shapefiles")
endif ()

set(_res f h i l c)

# Run polygon_to_shape on <input> creating <prefix>_L* and move the results to <dest>
function(make_shapes input prefix option dest)
	execute_process(COMMAND "${PROG}" ${input} ${prefix} ${option} WORKING_DIRECTORY "${WD}" RESULT_VARIABLE _r)
	if (NOT _r EQUAL 0)
		message(FATAL_ERROR "polygon_to_shape failed for ${input}")
	endif ()
	file(GLOB _dirs LIST_DIRECTORIES true "${WD}/${prefix}_L*")
	foreach (_d ${_dirs})
		if (IS_DIRECTORY "${_d}")
			file(GLOB _files "${_d}/*")
			foreach (_f ${_files})
				get_filename_component(_name "${_f}" NAME)
				file(RENAME "${_f}" "${dest}/${_name}")
			endforeach ()
		endif ()
		file(REMOVE_RECURSE "${_d}")	# Also removes the <prefix>_L*.gmt files
	endforeach ()
endfunction()

string(REPLACE "|" ";" _docs "${DOCS}")

if (MODE STREQUAL "gshhs")
	set(_top "${WD}/GSHHS_shp")
	file(REMOVE_RECURSE "${_top}")
	foreach (r ${_res})
		file(MAKE_DIRECTORY "${_top}/${r}")
		make_shapes(res_${r}/GSHHS_${r}_polygons.b GSHHS_${r} "" "${_top}/${r}")
	endforeach ()
elseif (MODE STREQUAL "wdbii")
	set(_top "${WD}/WDBII_shp")
	file(REMOVE_RECURSE "${_top}")
	foreach (r ${_res})
		file(MAKE_DIRECTORY "${_top}/${r}")
		if (r STREQUAL "f")
			set(_borders ../WDBII/borders/WDBII_Borders_segments.b)
			set(_rivers  ../WDBII/rivers/WDBII_Rivers_segments.b)
		else ()
			set(_borders wdb/wdb_${r}_borders.b)
			set(_rivers  wdb/wdb_${r}_rivers.b)
		endif ()
		make_shapes(${_borders} WDBII_border_${r} -o "${_top}/${r}")
		make_shapes(${_rivers}  WDBII_river_${r}  -i "${_top}/${r}")
	endforeach ()
else ()
	message(FATAL_ERROR "shapefiles.cmake: MODE must be gshhs or wdbii")
endif ()

foreach (_f ${_docs})
	if (EXISTS "${_f}")
		file(COPY "${_f}" DESTINATION "${_top}")
	endif ()
endforeach ()
