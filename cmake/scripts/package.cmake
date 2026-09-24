#
# Create a release archive from a staging directory:
#
#   cmake -DOUT=<archive> -DFORMAT=gnutar|zip [-DTOPDIR=<name>] [-DVERSION=<v>]
#         -DFILES="f1|f2|..." [-DDIRS="d1|d2|..."] -P package.cmake
#
# FILES are copied to the archive root (or TOPDIR), DIRS are copied there as
# whole directories. Missing files are skipped with a warning. With VERSION
# a VERSION file holding it is added. A .tar.gz is written when FORMAT=gnutar.
#

get_filename_component(_outdir "${OUT}" DIRECTORY)
get_filename_component(_outname "${OUT}" NAME)
set(_stage "${_outdir}/_stage_${_outname}")
file(REMOVE_RECURSE "${_stage}")

if (TOPDIR)
	set(_content "${_stage}/${TOPDIR}")
else ()
	set(_content "${_stage}")
endif ()
file(MAKE_DIRECTORY "${_content}")

string(REPLACE "|" ";" _files "${FILES}")
string(REPLACE "|" ";" _dirs "${DIRS}")
foreach (_f ${_files})
	if (EXISTS "${_f}")
		file(COPY "${_f}" DESTINATION "${_content}")
	else ()
		message(WARNING "package: ${_f} not found, not included in ${_outname}")
	endif ()
endforeach ()
foreach (_d ${_dirs})
	if (IS_DIRECTORY "${_d}")
		file(COPY "${_d}" DESTINATION "${_content}")
	else ()
		message(FATAL_ERROR "package: directory ${_d} not found")
	endif ()
endforeach ()
if (VERSION)
	file(WRITE "${_content}/VERSION" "${VERSION}\n")
endif ()

if (TOPDIR)
	set(_entries "${TOPDIR}")
else ()
	file(GLOB _entries RELATIVE "${_stage}" "${_stage}/*")
endif ()

if (FORMAT STREQUAL "zip")
	set(_flags cf)
else ()
	set(_flags czf)
endif ()
file(REMOVE "${OUT}")
execute_process(COMMAND "${CMAKE_COMMAND}" -E tar ${_flags} "${OUT}" --format=${FORMAT} ${_entries}
	WORKING_DIRECTORY "${_stage}" RESULT_VARIABLE _res)
file(REMOVE_RECURSE "${_stage}")
if (NOT _res EQUAL 0)
	message(FATAL_ERROR "package: failed to create ${OUT}")
endif ()
message(STATUS "Created ${OUT}")
