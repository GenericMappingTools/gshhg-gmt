ECHO OFF
REM	$Id: GSHHG_winbuild.bat 683 2017-06-24 20:11:45Z pwessel $
REM	Builds installer for GSHHG under Windows
REM	Paul Wessel with help from Joaquim Luis
REM
REM	Assumptions:
REM	1. You have run make build-all in the gshhg dir
REM	2. Inno Setup 5 has been installed and the path
REM	   to its command line tool is added to PATH
REM	3. 7zip has been installed and the path
REM	   to its command line tool is added to PATH

REM Change the version number to match what is to be released:

SET GSHHG=2.3.7

IF "%1%" == "home" (
	SET GSHHGDIR=Z:\UH\RESEARCH\CVSPROJECTS\GMTdev\gshhg\trunk
) ELSE (
	SET GSHHGDIR=%1%:\GMTdev\gshhg\trunk
)

REM echo === 1. Get all GSHHG %GSHHG% bzipped tar balls and extract files...

C:
cd C:\GMTdev
copy %GSHHGDIR%\gshhg-gmt-%GSHHG%.tar.gz C:\GMTdev\
7z x gshhg-gmt-%GSHHG%.tar.gz
7z x gshhg-gmt-%GSHHG%.tar -aoa
del gshhg-gmt-%GSHHG%.tar.gz
del gshhg-gmt-%GSHHG%.tar
rename gshhg-gmt-%GSHHG% GSHHG

echo === 1. Build the GSHHG %GSHHG% complete installer...

iscc /Q %GSHHGDIR%\guru\GSHHG_setup.iss

echo === 2. Copy GSHHG %GSHHG% complete installer...

copy INSTALLERS\gshhg-%GSHHG%-win32.exe %GSHHGDIR%

echo === 3. DONE
ECHO ON
