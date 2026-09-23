#
# Custom commands and targets that build the GSHHG data products.
# (Was coast.dep and the data targets of the old Makefiles.)
#
# The tools use hard-wired paths relative to src (e.g. ../GSHHS/res_f/...),
# so, as before, all commands run in src and the products are written into
# the source tree (the files are in .gitignore). Only the release archives
# are written to the build directory.
#

set(SRC "${CMAKE_CURRENT_SOURCE_DIR}")
set(TOP "${CMAKE_SOURCE_DIR}")
set(SCRIPTS "${CMAKE_SOURCE_DIR}/cmake/scripts")

set(GSHHG_RES f h i l c)
set(GSHHG_RES_NAME_f full)
set(GSHHG_RES_NAME_h high)
set(GSHHG_RES_NAME_i int)
set(GSHHG_RES_NAME_l low)
set(GSHHG_RES_NAME_c crude)

# Bin size in degrees
set(GSHHG_BIN_f 1  CACHE STRING "Bin size (degrees) for full resolution")
set(GSHHG_BIN_h 2  CACHE STRING "Bin size (degrees) for high resolution")
set(GSHHG_BIN_i 5  CACHE STRING "Bin size (degrees) for intermediate resolution")
set(GSHHG_BIN_l 10 CACHE STRING "Bin size (degrees) for low resolution")
set(GSHHG_BIN_c 20 CACHE STRING "Bin size (degrees) for crude resolution")
# Douglas-Peucker tolerances used to decimate the WDBII lines
set(GSHHG_DP_h 0.2 CACHE STRING "Douglas-Peucker tolerance for high resolution lines")
set(GSHHG_DP_i 1   CACHE STRING "Douglas-Peucker tolerance for intermediate resolution lines")
set(GSHHG_DP_l 5   CACHE STRING "Douglas-Peucker tolerance for low resolution lines")
set(GSHHG_DP_c 25  CACHE STRING "Douglas-Peucker tolerance for crude resolution lines")

# Documentation files that go into the release archives
set(GSHHG_DOC_DIR "${CMAKE_SOURCE_DIR}" CACHE PATH "Directory with README.TXT, LICENSE.TXT, COPYINGv3 and COPYING.LESSERv3 for the archives")
set(_docs_gmt "${GSHHG_DOC_DIR}/README.TXT|${GSHHG_DOC_DIR}/LICENSE.TXT|${GSHHG_DOC_DIR}/COPYINGv3|${GSHHG_DOC_DIR}/COPYING.LESSERv3")
set(_docs_shp "${GSHHG_DOC_DIR}/README.TXT|${TOP}/ChangeLog")

# ------------------------------------------------------------------------------
# binary: native binary files from their ASCII masters
# ------------------------------------------------------------------------------

# The full resolution Level 1 file was split in two to fit in GitHub; stitch it
set(_level1 "${TOP}/GSHHS/res_f/GSHHS_f_Level_1.txt")
if (EXISTS "${TOP}/GSHHS/res_f/GSHHS_f_Level_1a.txt")
	add_custom_command(OUTPUT "${_level1}"
		COMMAND "${CMAKE_COMMAND}" "-DIN=${TOP}/GSHHS/res_f/GSHHS_f_Level_1a.txt|${TOP}/GSHHS/res_f/GSHHS_f_Level_1b.txt"
			"-DOUT=${_level1}" -P "${SCRIPTS}/concat.cmake"
		DEPENDS "${TOP}/GSHHS/res_f/GSHHS_f_Level_1a.txt" "${TOP}/GSHHS/res_f/GSHHS_f_Level_1b.txt"
		COMMENT "Stitching GSHHS_f_Level_1.txt from its a and b parts"
		VERBATIM)
endif ()

set(_binary_files "")
foreach (r ${GSHHG_RES})
	file(GLOB _txt "${TOP}/GSHHS/res_${r}/GSHHS_${r}_*.txt")
	list(FILTER _txt EXCLUDE REGEX "Level_1[ab]\\.txt$")
	if (r STREQUAL "f")
		list(APPEND _txt "${_level1}")
		list(REMOVE_DUPLICATES _txt)
	endif ()
	set(_out "${TOP}/GSHHS/res_${r}/GSHHS_${r}_polygons.b")
	add_custom_command(OUTPUT "${_out}"
		COMMAND ascii_to_binary C ${r} -V
		DEPENDS ascii_to_binary ${_txt}
		WORKING_DIRECTORY "${SRC}"
		COMMENT "Building GSHHS_${r}_polygons.b"
		VERBATIM)
	list(APPEND _binary_files "${_out}")
endforeach ()

foreach (_kind Borders Rivers)
	string(TOLOWER ${_kind} _dir)
	string(SUBSTRING ${_kind} 0 1 _flag)
	file(GLOB _txt "${TOP}/WDBII/${_dir}/WDBII_${_kind}_Level_*.txt")
	set(_out "${TOP}/WDBII/${_dir}/WDBII_${_kind}_segments.b")
	add_custom_command(OUTPUT "${_out}"
		COMMAND ascii_to_binary ${_flag} -V
		DEPENDS ascii_to_binary ${_txt}
		WORKING_DIRECTORY "${SRC}"
		COMMENT "Building WDBII_${_kind}_segments.b"
		VERBATIM)
	list(APPEND _binary_files "${_out}")
endforeach ()

add_custom_target(binary DEPENDS ${_binary_files})

# ------------------------------------------------------------------------------
# Working copies in src/res_? and src/wdb (were symbolic links)
# ------------------------------------------------------------------------------

foreach (r ${GSHHG_RES})
	add_custom_command(OUTPUT "${SRC}/res_${r}/GSHHS_${r}_polygons.b"
		COMMAND "${CMAKE_COMMAND}" -E make_directory "${SRC}/res_${r}"
		COMMAND "${CMAKE_COMMAND}" -E copy "${TOP}/GSHHS/res_${r}/GSHHS_${r}_polygons.b" "${SRC}/res_${r}/GSHHS_${r}_polygons.b"
		DEPENDS "${TOP}/GSHHS/res_${r}/GSHHS_${r}_polygons.b"
		VERBATIM)
endforeach ()
add_custom_command(OUTPUT "${SRC}/wdb/wdb_f_borders.b"
	COMMAND "${CMAKE_COMMAND}" -E make_directory "${SRC}/wdb"
	COMMAND "${CMAKE_COMMAND}" -E copy "${TOP}/WDBII/borders/WDBII_Borders_segments.b" "${SRC}/wdb/wdb_f_borders.b"
	DEPENDS "${TOP}/WDBII/borders/WDBII_Borders_segments.b"
	VERBATIM)
add_custom_command(OUTPUT "${SRC}/wdb/wdb_f_rivers.b"
	COMMAND "${CMAKE_COMMAND}" -E make_directory "${SRC}/wdb"
	COMMAND "${CMAKE_COMMAND}" -E copy "${TOP}/WDBII/rivers/WDBII_Rivers_segments.b" "${SRC}/wdb/wdb_f_rivers.b"
	DEPENDS "${TOP}/WDBII/rivers/WDBII_Rivers_segments.b"
	VERBATIM)

# Decimated WDBII lines for the lower resolutions
set(_wdb_files "${SRC}/wdb/wdb_f_borders.b" "${SRC}/wdb/wdb_f_rivers.b")
foreach (r h i l c)
	foreach (_kind borders rivers)
		add_custom_command(OUTPUT "${SRC}/wdb/wdb_${r}_${_kind}.b"
			COMMAND line_shrink wdb/wdb_f_${_kind}.b ${GSHHG_DP_${r}} wdb/wdb_${r}_${_kind}.b
			DEPENDS line_shrink "${SRC}/wdb/wdb_f_${_kind}.b"
			WORKING_DIRECTORY "${SRC}"
			COMMENT "Decimating WDBII ${_kind} for resolution ${r}"
			VERBATIM)
		list(APPEND _wdb_files "${SRC}/wdb/wdb_${r}_${_kind}.b")
	endforeach ()
endforeach ()
add_custom_target(wdbfiles DEPENDS ${_wdb_files})

# ------------------------------------------------------------------------------
# data: binned netCDF files for GMT, per resolution
# ------------------------------------------------------------------------------

set(GSHHG_NC_FILES "")
set(_info_files "")
foreach (r ${GSHHG_RES})
	set(_res "${SRC}/res_${r}")
	set(_pol "${_res}/GSHHS_${r}_polygons.b")
	set(_bin ${GSHHG_BIN_${r}})

	# Node levels and node polygon IDs
	add_custom_command(OUTPUT "${_res}/GSHHS_${r}_nodes.nc" "${_res}/GSHHS_${r}_ndid.b"
		BYPRODUCTS "${_res}/GSHHS_${r}_nodes_ID.grd"
		COMMAND polygon_setnodes res_${r}/GSHHS_${r}_polygons.b ${_bin} res_${r}/GSHHS_${r}_nodes.nc res_${r}/GSHHS_${r}_ndid.b
		DEPENDS polygon_setnodes "${_pol}"
		WORKING_DIRECTORY "${SRC}"
		COMMENT "Setting node levels for resolution ${r}"
		VERBATIM)

	# Coastlines
	add_custom_command(OUTPUT "${_res}/binned_GSHHS_${r}.pt"
		BYPRODUCTS "${_res}/binned_GSHHS_${r}.bin" "${_res}/binned_GSHHS_${r}.seg" "${_res}/binned_GSHHS_${r}.ant"
			"${_res}/binned_GSHHS_${r}.pol" "${_res}/binned_GSHHS_${r}.ndid"
		COMMAND polygon_to_bins res_${r}/GSHHS_${r}_polygons.b ${_bin} res_${r}/GSHHS_${r}_nodes.nc res_${r}/GSHHS_${r}_ndid.b res_${r}/binned_GSHHS_${r}
		DEPENDS polygon_to_bins "${_pol}" "${_res}/GSHHS_${r}_nodes.nc" "${_res}/GSHHS_${r}_ndid.b"
		WORKING_DIRECTORY "${SRC}"
		COMMENT "Binning coastlines for resolution ${r}"
		VERBATIM)
	add_custom_command(OUTPUT "${_res}/binned_GSHHS_${r}.nc"
		COMMAND shoremaker res_${r}/binned_GSHHS_${r}
		DEPENDS shoremaker "${_res}/binned_GSHHS_${r}.pt"
		WORKING_DIRECTORY "${SRC}"
		COMMENT "Creating binned_GSHHS_${r}.nc"
		VERBATIM)
	set(_nc_r "${_res}/binned_GSHHS_${r}.nc")

	# Borders and rivers
	foreach (_kind border river)
		add_custom_command(OUTPUT "${_res}/binned_${_kind}_${r}.pt"
			BYPRODUCTS "${_res}/binned_${_kind}_${r}.bin" "${_res}/binned_${_kind}_${r}.seg"
			COMMAND lines_to_bins wdb/wdb_${r}_${_kind}s.b ${_bin} res_${r}/binned_${_kind}_${r}
			DEPENDS lines_to_bins "${SRC}/wdb/wdb_${r}_${_kind}s.b"
			WORKING_DIRECTORY "${SRC}"
			COMMENT "Binning ${_kind}s for resolution ${r}"
			VERBATIM)
		add_custom_command(OUTPUT "${_res}/binned_${_kind}_${r}.nc"
			COMMAND linemaker res_${r}/binned_${_kind}_${r}
			DEPENDS linemaker "${_res}/binned_${_kind}_${r}.pt"
			WORKING_DIRECTORY "${SRC}"
			COMMENT "Creating binned_${_kind}_${r}.nc"
			VERBATIM)
		list(APPEND _nc_r "${_res}/binned_${_kind}_${r}.nc")
	endforeach ()

	add_custom_target(${GSHHG_RES_NAME_${r}} DEPENDS ${_nc_r})
	list(APPEND GSHHG_NC_FILES ${_nc_r})

	# Polygon information listing
	add_custom_command(OUTPUT "${_res}/GSHHS_${r}_info.lis"
		COMMAND "${CMAKE_COMMAND}" "-DCMD=$<TARGET_FILE:polygon_final_info>|res_${r}/GSHHS_${r}_polygons.b"
			"-DOUT=${_res}/GSHHS_${r}_info.lis" "-DWD=${SRC}" -P "${SCRIPTS}/run_to_file.cmake"
		DEPENDS polygon_final_info "${_pol}"
		COMMENT "Creating GSHHS_${r}_info.lis"
		VERBATIM)
	add_custom_target(info_${r} DEPENDS "${_res}/GSHHS_${r}_info.lis")
	list(APPEND _info_files "${_res}/GSHHS_${r}_info.lis")
endforeach ()

add_custom_target(data DEPENDS ${GSHHG_NC_FILES})
add_custom_target(info DEPENDS ${_info_files})

# ------------------------------------------------------------------------------
# build-gshhs: native binary GSHHS files (src/gshhs)
# ------------------------------------------------------------------------------

set(_gshhs_files "")
foreach (r ${GSHHG_RES})
	set(_in_c "res_${r}/GSHHS_${r}_polygons.b")
	if (r STREQUAL "f")
		set(_in_b "../WDBII/borders/WDBII_Borders_segments.b")
		set(_in_r "../WDBII/rivers/WDBII_Rivers_segments.b")
	else ()
		set(_in_b "wdb/wdb_${r}_borders.b")
		set(_in_r "wdb/wdb_${r}_rivers.b")
	endif ()
	foreach (_item "gshhs_${r}|${_in_c}|" "wdb_borders_${r}|${_in_b}|-l" "wdb_rivers_${r}|${_in_r}|-l")
		string(REPLACE "|" ";" _item "${_item}")
		list(GET _item 0 _name)
		list(GET _item 1 _in)
		list(LENGTH _item _n)
		set(_cmd "$<TARGET_FILE:polygon_to_gshhs>")
		if (_n GREATER 2)
			list(GET _item 2 _opt)
			if (_opt)
				string(APPEND _cmd "|${_opt}")
			endif ()
		endif ()
		string(APPEND _cmd "|${_in}")
		add_custom_command(OUTPUT "${SRC}/gshhs/${_name}.b"
			COMMAND "${CMAKE_COMMAND}" -E make_directory "${SRC}/gshhs"
			COMMAND "${CMAKE_COMMAND}" "-DCMD=${_cmd}" "-DOUT=${SRC}/gshhs/${_name}.b" "-DWD=${SRC}" -P "${SCRIPTS}/run_to_file.cmake"
			DEPENDS polygon_to_gshhs "${SRC}/${_in}"
			COMMENT "Creating gshhs/${_name}.b"
			VERBATIM)
		list(APPEND _gshhs_files "${SRC}/gshhs/${_name}.b")
	endforeach ()
endforeach ()
add_custom_target(build-gshhs DEPENDS ${_gshhs_files})

# ------------------------------------------------------------------------------
# Shapefiles (need ogr2ogr)
# ------------------------------------------------------------------------------

set(_pol_all "")
foreach (r ${GSHHG_RES})
	list(APPEND _pol_all "${SRC}/res_${r}/GSHHS_${r}_polygons.b")
endforeach ()
add_custom_command(OUTPUT "${SRC}/GSHHS_shp/.stamp"
	COMMAND "${CMAKE_COMMAND}" -DMODE=gshhs "-DPROG=$<TARGET_FILE:polygon_to_shape>" "-DWD=${SRC}"
		"-DDOCS=${_docs_shp}" -P "${SCRIPTS}/shapefiles.cmake"
	COMMAND "${CMAKE_COMMAND}" -E touch "${SRC}/GSHHS_shp/.stamp"
	DEPENDS polygon_to_shape ${_pol_all}
	COMMENT "Creating GSHHS shapefiles"
	VERBATIM)
add_custom_command(OUTPUT "${SRC}/WDBII_shp/.stamp"
	COMMAND "${CMAKE_COMMAND}" -DMODE=wdbii "-DPROG=$<TARGET_FILE:polygon_to_shape>" "-DWD=${SRC}"
		"-DDOCS=${_docs_shp}" -P "${SCRIPTS}/shapefiles.cmake"
	COMMAND "${CMAKE_COMMAND}" -E touch "${SRC}/WDBII_shp/.stamp"
	DEPENDS polygon_to_shape ${_wdb_files}
	COMMENT "Creating WDBII shapefiles"
	VERBATIM)
add_custom_target(shape-gshhs DEPENDS "${SRC}/GSHHS_shp/.stamp")
add_custom_target(shape-wdbii DEPENDS "${SRC}/WDBII_shp/.stamp")
add_custom_target(shapefiles DEPENDS "${SRC}/GSHHS_shp/.stamp" "${SRC}/WDBII_shp/.stamp")

# ------------------------------------------------------------------------------
# Release archives (written to the build directory)
# ------------------------------------------------------------------------------

set(GSHHG_TAR_GMT "${CMAKE_BINARY_DIR}/${GSHHG_TAG}-gmt-${GSHHG_NEW_VERSION}.tar.gz")
set(GSHHG_ZIP_BIN "${CMAKE_BINARY_DIR}/${GSHHG_TAG}-bin-${GSHHG_NEW_VERSION}.zip")
set(GSHHG_ZIP_SHP "${CMAKE_BINARY_DIR}/${GSHHG_TAG}-shp-${GSHHG_NEW_VERSION}.zip")

string(REPLACE ";" "|" _nc_list "${GSHHG_NC_FILES}")
add_custom_command(OUTPUT "${GSHHG_TAR_GMT}"
	COMMAND "${CMAKE_COMMAND}" "-DOUT=${GSHHG_TAR_GMT}" -DFORMAT=gnutar
		"-DTOPDIR=${GSHHG_TAG}-gmt-${GSHHG_NEW_VERSION}" "-DVERSION=${GSHHG_NEW_VERSION}"
		"-DFILES=${_nc_list}|${_docs_gmt}" -P "${SCRIPTS}/package.cmake"
	DEPENDS ${GSHHG_NC_FILES}
	COMMENT "Creating ${GSHHG_TAG}-gmt-${GSHHG_NEW_VERSION}.tar.gz"
	VERBATIM)
add_custom_target(tar_gshhg_nc DEPENDS "${GSHHG_TAR_GMT}")

string(REPLACE ";" "|" _gshhs_list "${_gshhs_files}")
add_custom_command(OUTPUT "${GSHHG_ZIP_BIN}"
	COMMAND "${CMAKE_COMMAND}" "-DOUT=${GSHHG_ZIP_BIN}" -DFORMAT=zip
		"-DFILES=${_docs_gmt}|${_gshhs_list}" -P "${SCRIPTS}/package.cmake"
	DEPENDS ${_gshhs_files}
	COMMENT "Creating ${GSHHG_TAG}-bin-${GSHHG_NEW_VERSION}.zip"
	VERBATIM)
add_custom_target(zip_gshhg_bin DEPENDS "${GSHHG_ZIP_BIN}")

add_custom_command(OUTPUT "${GSHHG_ZIP_SHP}"
	COMMAND "${CMAKE_COMMAND}" "-DOUT=${GSHHG_ZIP_SHP}" -DFORMAT=zip
		"-DFILES=${GSHHG_DOC_DIR}/README.TXT|${GSHHG_DOC_DIR}/SHAPEFILES.TXT|${GSHHG_DOC_DIR}/LICENSE.TXT|${GSHHG_DOC_DIR}/COPYINGv3|${GSHHG_DOC_DIR}/COPYING.LESSERv3"
		"-DDIRS=${SRC}/GSHHS_shp|${SRC}/WDBII_shp" -P "${SCRIPTS}/package.cmake"
	DEPENDS "${SRC}/GSHHS_shp/.stamp" "${SRC}/WDBII_shp/.stamp"
	COMMENT "Creating ${GSHHG_TAG}-shp-${GSHHG_NEW_VERSION}.zip"
	VERBATIM)
add_custom_target(zip_gshhg_shp DEPENDS "${GSHHG_ZIP_SHP}")

add_custom_target(build-all DEPENDS tar_gshhg_nc zip_gshhg_bin zip_gshhg_shp)

add_custom_target(checksum
	COMMAND "${CMAKE_COMMAND}" -E md5sum "${GSHHG_TAR_GMT}"
	DEPENDS "${GSHHG_TAR_GMT}"
	COMMENT "Update gshhg.info with this check sum"
	VERBATIM)

# Copy the archives to the distribution sites (needs scp and the right accounts)
find_program(SCP_PROGRAM scp)
if (SCP_PROGRAM)
	add_custom_target(place
		COMMAND "${SCP_PROGRAM}" "${GSHHG_TAR_GMT}" "${GMT_FTPSITE}"
		COMMAND "${SCP_PROGRAM}" "${GSHHG_TAR_GMT}" "${GSHHG_ZIP_BIN}" "${GSHHG_ZIP_SHP}" "${GSHHG_FTPSITE}"
		COMMAND "${SCP_PROGRAM}" "${GSHHG_TAR_GMT}" "${GSHHG_ZIP_BIN}" "${GSHHG_ZIP_SHP}" "${GSHHG_WWWSITE}"
		COMMAND "${SCP_PROGRAM}" "${GSHHG_TAR_GMT}" "gmtdaemon@${GMT_WWWSITE}"
		COMMENT "Placing ${GSHHG_NEW_VERSION} archives on the ftp and web sites"
		VERBATIM)
endif ()

# ------------------------------------------------------------------------------
# spotless: remove every generated data product
# ------------------------------------------------------------------------------

set(_spotless "")
foreach (r ${GSHHG_RES})
	list(APPEND _spotless "${TOP}/GSHHS/res_${r}/GSHHS_${r}_polygons.b" "${SRC}/res_${r}")
endforeach ()
if (EXISTS "${TOP}/GSHHS/res_f/GSHHS_f_Level_1a.txt")
	list(APPEND _spotless "${_level1}")
endif ()
list(APPEND _spotless
	"${TOP}/WDBII/borders/WDBII_Borders_segments.b" "${TOP}/WDBII/rivers/WDBII_Rivers_segments.b"
	"${SRC}/wdb" "${SRC}/gshhs" "${SRC}/GSHHS_shp" "${SRC}/WDBII_shp"
	"${GSHHG_TAR_GMT}" "${GSHHG_ZIP_BIN}" "${GSHHG_ZIP_SHP}")
add_custom_target(spotless
	COMMAND "${CMAKE_COMMAND}" -E rm -rf ${_spotless}
	COMMENT "Removing all generated data products"
	VERBATIM)
