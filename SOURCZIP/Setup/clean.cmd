@echo off

::-----------------------------------------------------------------------------
::     (the format of the command as it should appear in VS2005)
::-----------------------------------------------------------------------------

REM clean.cmd  "$(SolutionDir)dist"  "$(OutDir)"

::-----------------------------------------------------------------------------
::                             clean.cmd
::-----------------------------------------------------------------------------
::
::  This batch file cleans out all of the files from the Solution's 'dist'
::  directory as part of a Visual Studio 'clean' build command. The 'Setup'
::  project -- which is a subproject of the main Solution -- is designed as
::  a "makefile" project (as the NSIS compiler is a command-line driven tool)
::  and we specify this batch file as the project's nmake "Clean command-line"
::  property value. Thus whenever 'Clean Solution' is chosen, not only do the
::  intermediate work directories get cleaned, but our main Solution's 'dist'
::  distribution staging (packaging) directory gets cleaned out as well.
::
::-----------------------------------------------------------------------------

REM  Args  1  =  "<prod-dist-i/p-dir>"    ("$(SolutionDir)dist")   (always)
REM        2  =  $(OutDir)                ("All")                  (always)

set PRODUCT_DIST_INDIR=%~1
set PRODUCT_SETUP_OUTDIR=%~2

goto :skip
echo %0: PRODUCT_DIST_INDIR     = %PRODUCT_DIST_INDIR%
echo %0: PRODUCT_SETUP_OUTDIR   = %PRODUCT_SETUP_OUTDIR%
:skip

if not exist "%PRODUCT_DIST_INDIR%"    mkdir "%PRODUCT_DIST_INDIR%"
if not exist "%PRODUCT_SETUP_OUTDIR%"  mkdir "%PRODUCT_SETUP_OUTDIR%"

if exist "%PRODUCT_DIST_INDIR%\*.*"    del /f /q  "%PRODUCT_DIST_INDIR%\*.*"    > NUL
if exist "%PRODUCT_SETUP_OUTDIR%\*.*"  del /f /q  "%PRODUCT_SETUP_OUTDIR%\*.*"  > NUL

goto :EOF
