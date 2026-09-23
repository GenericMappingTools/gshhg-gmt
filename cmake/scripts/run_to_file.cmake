#
# Run a program and send its standard output to a file (portable "prog args > file"):
#
#   cmake -DCMD="prog|arg1|arg2" -DOUT=file [-DWD=dir] -P run_to_file.cmake
#
# Arguments are separated by "|" since ";" would be split by add_custom_command.
#

string(REPLACE "|" ";" _cmd "${CMD}")
if (NOT WD)
	set(WD "${CMAKE_CURRENT_BINARY_DIR}")
endif ()
execute_process(COMMAND ${_cmd} OUTPUT_FILE "${OUT}.tmp" WORKING_DIRECTORY "${WD}" RESULT_VARIABLE _res)
if (NOT _res EQUAL 0)
	file(REMOVE "${OUT}.tmp")
	message(FATAL_ERROR "Command failed (${_res}): ${_cmd}")
endif ()
file(RENAME "${OUT}.tmp" "${OUT}")
