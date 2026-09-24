#
# Template for user settings of the GSHHG CMake build.
#
# Copy this file to cmake/ConfigUser.cmake and edit it. It is read before
# anything else, so values set here act as defaults for the cache variables.
# Everything can also be given on the command line with -D<VAR>=<value>.
#

# GMT 6 install prefix, i.e. the directory with include/gmt and lib
# [by default found via gmt-config]
#set(GMT_ROOT "C:/progs_cygw/GMTdev/gmt5/compileds/gmt6/VC14_64")

# netCDF and GDAL prefixes (directories with include and lib)
# [by default the ones GMT was built with, as reported by gmt-config --dep-libs]
#set(NETCDF_ROOT "C:/programs/compa_libs/netcdf_vcpkg/compileds/VC14_64")
#set(GDAL_ROOT   "C:/programs/compa_libs/gdal_GIT/compileds/VC14_64")

# Data version to be released next and binary compatibility version
#set(GSHHG_NEW_VERSION "2.3.7" CACHE STRING "")
#set(GSHHG_COMPATIBILITY_VERSION "15" CACHE STRING "")
