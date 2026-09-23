#
# Find the GMT 6 development headers (gmt_dev.h) and library.
#
# Search hints, in order:
#   GMT_ROOT (CMake or environment variable): the GMT install prefix,
#            i.e. the directory holding include/gmt and lib
#   gmt-config: when found (and a bash is available) its --includedir,
#            --libdir and --dep-libs output is used
#
# Defines:
#   GMT_FOUND, GMT_INCLUDE_DIR, GMT_LIBRARY, GMT_VERSION
#   GMT_DEP_LIBS   list of libraries GMT was linked with (from gmt-config),
#                  used as hints to find netCDF and GDAL
#   GMT::GMT       imported target
#

find_program(GMT_CONFIG gmt-config HINTS ${GMT_ROOT} $ENV{GMT_ROOT} PATH_SUFFIXES bin)

set(_gmt_inc_hint "")
set(_gmt_lib_hint "")
if (GMT_CONFIG)
	# gmt-config is a bash script, so run it through bash where needed (Windows)
	find_program(GMT_BASH NAMES bash sh)
	if (GMT_BASH)
		foreach (_opt includedir libdir dep-libs version)
			execute_process(COMMAND "${GMT_BASH}" "${GMT_CONFIG}" --${_opt}
				OUTPUT_VARIABLE _out OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE _res)
			if (_res EQUAL 0)
				string(REPLACE "-" "_" _var "${_opt}")
				set(_gmt_cfg_${_var} "${_out}")
			endif ()
		endforeach ()
		set(_gmt_inc_hint "${_gmt_cfg_includedir}")
		set(_gmt_lib_hint "${_gmt_cfg_libdir}")
		if (_gmt_cfg_dep_libs)
			separate_arguments(_deps NATIVE_COMMAND "${_gmt_cfg_dep_libs}")
			set(GMT_DEP_LIBS "${_deps}" CACHE INTERNAL "Libraries GMT was linked with")
		endif ()
		if (_gmt_cfg_version)
			set(GMT_VERSION "${_gmt_cfg_version}")
		endif ()
	endif ()
endif ()

find_path(GMT_INCLUDE_DIR gmt_dev.h
	HINTS ${GMT_ROOT} $ENV{GMT_ROOT} ${_gmt_inc_hint}
	PATH_SUFFIXES include/gmt gmt include)

find_library(GMT_LIBRARY NAMES gmt gmt_w64 gmt_w32
	HINTS ${GMT_ROOT} $ENV{GMT_ROOT} ${_gmt_lib_hint}
	PATH_SUFFIXES lib lib64)

if (GMT_INCLUDE_DIR AND NOT GMT_VERSION AND EXISTS "${GMT_INCLUDE_DIR}/gmt_version.h")
	file(STRINGS "${GMT_INCLUDE_DIR}/gmt_version.h" _ver REGEX "define GMT_(MAJOR|MINOR|RELEASE)_VERSION ")
	string(REGEX REPLACE ".*MAJOR_VERSION ([0-9]+).*MINOR_VERSION ([0-9]+).*RELEASE_VERSION ([0-9]+).*" "\\1.\\2.\\3" GMT_VERSION "${_ver}")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMT
	REQUIRED_VARS GMT_LIBRARY GMT_INCLUDE_DIR
	VERSION_VAR GMT_VERSION)

if (GMT_FOUND AND NOT TARGET GMT::GMT)
	add_library(GMT::GMT UNKNOWN IMPORTED)
	set_target_properties(GMT::GMT PROPERTIES
		IMPORTED_LOCATION "${GMT_LIBRARY}"
		INTERFACE_INCLUDE_DIRECTORIES "${GMT_INCLUDE_DIR}")
endif ()

mark_as_advanced(GMT_CONFIG GMT_BASH GMT_INCLUDE_DIR GMT_LIBRARY)
