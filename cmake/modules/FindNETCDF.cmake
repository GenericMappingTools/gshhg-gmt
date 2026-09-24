#
# Find the netCDF C library.
#
# Search hints: NETCDF_ROOT (CMake or environment variable), nc-config,
# and the netCDF library GMT itself was linked with (GMT_DEP_LIBS).
#
# Defines:
#   NETCDF_FOUND, NETCDF_INCLUDE_DIR, NETCDF_LIBRARY
#   NETCDF::NETCDF  imported target
#

set(_nc_hints ${NETCDF_ROOT} $ENV{NETCDF_ROOT})

# The netCDF library that GMT was built with (from gmt-config --dep-libs)
foreach (_lib ${GMT_DEP_LIBS})
	if (_lib MATCHES "netcdf" AND EXISTS "${_lib}")
		get_filename_component(_dir "${_lib}" DIRECTORY)
		get_filename_component(_dir "${_dir}" DIRECTORY)
		list(APPEND _nc_hints "${_dir}")
	endif ()
endforeach ()

find_program(NC_CONFIG nc-config HINTS ${_nc_hints} PATH_SUFFIXES bin)
if (NC_CONFIG AND NOT WIN32)
	execute_process(COMMAND "${NC_CONFIG}" --prefix OUTPUT_VARIABLE _nc_prefix
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
	list(APPEND _nc_hints "${_nc_prefix}")
endif ()

find_path(NETCDF_INCLUDE_DIR netcdf.h HINTS ${_nc_hints} PATH_SUFFIXES include)
find_library(NETCDF_LIBRARY NAMES netcdf HINTS ${_nc_hints} PATH_SUFFIXES lib lib64)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NETCDF REQUIRED_VARS NETCDF_LIBRARY NETCDF_INCLUDE_DIR)

if (NETCDF_FOUND AND NOT TARGET NETCDF::NETCDF)
	add_library(NETCDF::NETCDF UNKNOWN IMPORTED)
	set_target_properties(NETCDF::NETCDF PROPERTIES
		IMPORTED_LOCATION "${NETCDF_LIBRARY}"
		INTERFACE_INCLUDE_DIRECTORIES "${NETCDF_INCLUDE_DIR}")
endif ()

mark_as_advanced(NC_CONFIG NETCDF_INCLUDE_DIR NETCDF_LIBRARY)
