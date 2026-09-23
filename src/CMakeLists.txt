#
# GSHHG processing tools and data products.
#
# Note: Most of these tools were last used in 1995/96 and only minor changes
# to make them compile have been performed.  Several are probably obsolete.
#

# ------------------------------------------------------------------------------
# libcoast: routines shared by the tools
# ------------------------------------------------------------------------------

add_library(coast STATIC
	gmt4_compat.c
	poly_check_subs.c
	string_check_subs.c
	douglas_peucker.c
	coast_io.c
	poly_misc_subs.c)

target_include_directories(coast PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${GDAL_INCLUDE_DIR}")
target_compile_definitions(coast PUBLIC
	GSHHG_NEW_VERSION="${GSHHG_NEW_VERSION}"
	GSHHG_COMPATIBILITY_VERSION=${GSHHG_COMPATIBILITY_VERSION}
	YEAR=${GSHHG_YEAR})
target_link_libraries(coast PUBLIC GMT::GMT NETCDF::NETCDF)
if (NOT WIN32)
	target_link_libraries(coast PUBLIC m)
endif ()

# ------------------------------------------------------------------------------
# The programs
# ------------------------------------------------------------------------------

set(GSHHG_PROGRAMS
	ascii_to_binary binary_to_ascii border_to_bins ciaman_fix gshhs_to_polygon
	line_shrink linemaker lines_to_bins man_fix man_fix2
	polygon_bincount polygon_checkarea polygon_close polygon_compare polygon_consistency
	polygon_deldups polygon_dump polygon_extract polygon_extract2 polygon_extract_all
	polygon_final_info polygon_findarea polygon_findlevel polygon_fix polygon_fixlevel
	polygon_fixnegwesn polygon_get polygon_hierarchy polygon_id polygon_in_pol
	polygon_match polygon_merge polygon_rect polygon_report polygon_restore
	polygon_set polygon_setarea polygon_setnodes polygon_setwesn polygon_shrink
	polygon_skipdups polygon_sort polygon_stats polygon_sync polygon_to_bins
	polygon_to_gshhs polygon_to_shape polygon_update polygon_xover
	read_wvs river_fix segment_clean segment_connect segment_dump
	segment_final_dump segment_report segment_restore shoremaker swap
	wvs_crosscheck wvs_segment_dump wvs_segment_restore2)

foreach (_prog ${GSHHG_PROGRAMS})
	# gshhg_binmode.c forces binary stdio on Windows (no-op elsewhere)
	add_executable(${_prog} ${_prog}.c gshhg_binmode.c)
	target_link_libraries(${_prog} PRIVATE coast)
endforeach ()

add_custom_target(programs DEPENDS ${GSHHG_PROGRAMS})

install(TARGETS ${GSHHG_PROGRAMS} RUNTIME DESTINATION bin)

# ------------------------------------------------------------------------------
# Data products
# ------------------------------------------------------------------------------

include(GSHHGData.cmake)
