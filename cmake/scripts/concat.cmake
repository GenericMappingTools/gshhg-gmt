#
# Concatenate files:  cmake -DIN="a|b|..." -DOUT=file -P concat.cmake
#
# Used to stitch GSHHS_f_Level_1a.txt and GSHHS_f_Level_1b.txt back into
# GSHHS_f_Level_1.txt (the file was split to fit in GitHub).
#

string(REPLACE "|" ";" _in "${IN}")
set(_tmp "${OUT}.tmp")
file(REMOVE "${_tmp}")
foreach (_f ${_in})
	if (NOT EXISTS "${_f}")
		message(FATAL_ERROR "concat: missing input ${_f}")
	endif ()
	file(READ "${_f}" _txt)
	file(APPEND "${_tmp}" "${_txt}")
endforeach ()
file(RENAME "${_tmp}" "${OUT}")
