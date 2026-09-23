/*
 * gshhg_binmode.c
 *
 * The GSHHG tools were written for Unix and read/write binary data with
 * fopen modes "r"/"w" and via stdin/stdout.  On Windows that means text
 * mode, which corrupts binary data (CR/LF translation, ^Z as EOF).
 * This file is compiled into every executable on Windows and switches the
 * CRT default file mode, stdin and stdout to binary before main() runs.
 * It is a no-op elsewhere.
 */

#ifdef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <crtdbg.h>

static int gshhg_set_binmode(void) {
	_set_fmode(_O_BINARY);
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
	/* Debug CRT: report asserts/errors (e.g. fread on a NULL FILE from a failed fopen)
	 * on stderr and abort instead of popping up a modal dialog that hangs batch runs.
	 * These are no-ops in release builds. */
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
	return (0);
}

#ifdef _MSC_VER
/* Register as a CRT initializer (runs after stdio is set up, before main) */
#pragma section(".CRT$XCU", read)
__declspec(allocate(".CRT$XCU")) int (*gshhg_binmode_init)(void) = gshhg_set_binmode;
#pragma comment(linker, "/include:gshhg_binmode_init")
#else
/* MinGW and friends */
__attribute__((constructor)) static void gshhg_binmode_ctor(void) {
	(void)gshhg_set_binmode();
}
#endif
#else
/* Avoid an empty translation unit */
typedef int gshhg_binmode_unused;
#endif
