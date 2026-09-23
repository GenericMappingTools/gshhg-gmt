#
# CMake build for the GSHHG = GSHHS + WDBII master data files.
#
# Replaces the old GMT4-based Makefiles. The processing tools in src are
# built against the GMT 6 development library (gmt_dev.h + libgmt), and the
# data products are built through custom targets that mirror the old make
# targets:
#
#   (default)       Compile the processing tools in src
#   binary          Build the native binary files from their ASCII counterparts
#   data            Build the binned coastline, border and river netCDF files
#                   (also: full high int low crude)
#   info            Polygon info listings for all resolutions
#   build-gshhs     Native GSHHS binary files (src/gshhs)
#   shapefiles      Shapefiles of polygons and lines (needs ogr2ogr)
#   tar_gshhg_nc    gshhg-gmt-<version>.tar.gz (netCDF-4 files for GMT)
#   zip_gshhg_bin   gshhg-bin-<version>.zip
#   zip_gshhg_shp   gshhg-shp-<version>.zip
#   build-all       All three release products
#   checksum        MD5 checksum of the GMT tarball
#   spotless        Remove all generated data products
#
# Paths to GMT and netCDF are set in cmake/ConfigUser.cmake (copy it from
# cmake/ConfigUserTemplate.cmake) or on the command line, e.g.
#
#   cmake -S . -B build -G Ninja -DGMT_ROOT=C:/progs_cygw/GMTdev/gmt5/compileds/gmt6/VC14_64
#   cmake --build build
#   cmake --build build --target data
#

cmake_minimum_required(VERSION 3.18)

project(GSHHG C)

# User settings (paths to GMT, netCDF, GDAL, ...)
include("${CMAKE_SOURCE_DIR}/cmake/ConfigUser.cmake" OPTIONAL)

# ------------------------------------------------------------------------------
# Release settings (were in config.mk.orig)
# ------------------------------------------------------------------------------

# GSHHG data version to be released next
set(GSHHG_NEW_VERSION "2.3.7" CACHE STRING "GSHHG data version to be released next")
# GSHHG compatibility version (8 bit integer). Increase only when a new
# release breaks binary backwards compatibility
set(GSHHG_COMPATIBILITY_VERSION "15" CACHE STRING "GSHHG binary compatibility version")
# Prefix of the tar/zip balls
set(GSHHG_TAG "gshhg")
# Copyright year stamped into the netCDF files
string(TIMESTAMP GSHHG_YEAR "%Y")

# Remote sites used by the "place" target
set(GMT_FTPSITE   "ftp.soest.hawaii.edu:/export/ftp1/ftp/pub/gmt" CACHE STRING "FTP repository for gmt")
set(GSHHG_FTPSITE "ftp.soest.hawaii.edu:/export/ftp1/ftp/pub/gshhg" CACHE STRING "FTP repository for gshhg")
set(GSHHG_WWWSITE "imina.soest.hawaii.edu:/export/imina2/httpd/htdocs/pwessel/gshhg" CACHE STRING "HTTP repository for gshhg")
set(GMT_WWWSITE   "gmtserver.soest.hawaii.edu:/opt/bitnami/rubystack-3.2.3-0/apps/files/files" CACHE STRING "HTTP repository for the GMT site")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

find_package(GMT REQUIRED)
find_package(NETCDF REQUIRED)

# gmt_dev.h includes the GDAL headers, so we need them (but not the library)
set(_gdal_hints ${GDAL_ROOT} $ENV{GDAL_ROOT})
foreach (_lib ${GMT_DEP_LIBS})	# The GDAL that GMT was built with
	if (_lib MATCHES "gdal" AND EXISTS "${_lib}")
		get_filename_component(_dir "${_lib}" DIRECTORY)
		get_filename_component(_dir "${_dir}" DIRECTORY)
		list(APPEND _gdal_hints "${_dir}")
	endif ()
endforeach ()
find_path(GDAL_INCLUDE_DIR gdal.h HINTS ${_gdal_hints} PATH_SUFFIXES include include/gdal)
if (NOT GDAL_INCLUDE_DIR)
	message(FATAL_ERROR "GDAL headers (gdal.h) not found; gmt_dev.h needs them. Set GDAL_ROOT.")
endif ()

# ------------------------------------------------------------------------------
# Compiler settings
# ------------------------------------------------------------------------------

set(CMAKE_C_STANDARD 99)
if (MSVC)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)
	# The old code is full of implicit int/double/short conversions
	add_compile_options(/wd4244 /wd4267 /wd4305 /wd4996)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(src)

# ------------------------------------------------------------------------------
# Summary
# ------------------------------------------------------------------------------

message(STATUS "GSHHG version        : ${GSHHG_NEW_VERSION} (compatibility ${GSHHG_COMPATIBILITY_VERSION})")
message(STATUS "GMT include dir      : ${GMT_INCLUDE_DIR}")
message(STATUS "GMT library          : ${GMT_LIBRARY}")
message(STATUS "netCDF include dir   : ${NETCDF_INCLUDE_DIR}")
message(STATUS "netCDF library       : ${NETCDF_LIBRARY}")
message(STATUS "GDAL include dir     : ${GDAL_INCLUDE_DIR}")
