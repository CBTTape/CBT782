@echo off
set _DEBUG=0

::-----------------------------------------------------------------------------
::     (the format of the command as it should appear in VS2005)
::-----------------------------------------------------------------------------

REM   makesetup.cmd  "AWSBrowse"  "AWSBrowse.nsi"  "AWSBrowse32.exe"  "$(SolutionDir)dist"  "$(OutDir)"


::-----------------------------------------------------------------------------
::                             makesetup.cmd
::-----------------------------------------------------------------------------
::
::  This batch file will invoke the "makensis.exe" NSIS (Nullsoft Scriptable
::  Install System) compiler to build a setup program. NSIS is a tool that
::  allows programmers to create such installers for Windows. It is released
::  under an open source license and is completely free for any use. The URL
::  is: http://nsis.sf.net
::
::  The "grep", "egrep" and "cut" programs are pure Win32 equivalent ports
::  of the original *nix versions from the "UnxUtils" and "GnuWin32" packages
::  since I no longer have Cygwin installed. Search the web for "UnxUtils"
::  and "GnuWin32" to find them. The "verinfo" utility I found elsewhere on
::  the web but should be easy enough to find as well.
::
::-----------------------------------------------------------------------------
::
::  Change History:
::
::  MM/DD/YY   XXXXXXXXXXXXXXXX.........
::
::-----------------------------------------------------------------------------


REM Args: 1 = "<product-name>"            ("AWSBrowse")            (always)
REM       2 = "<scriptname>.nsi"          ("AWSBrowse.nsi")        (always)
REM       3 = "<win32-release-filename>"  ("AWSBrowse32.exe")      (always) (so we can extract the VERSION info from it)
REM       4 = "<prod-dist-i/p-dir>"       ("$(SolutionDir)dist")   (always)
REM       5 = $(OutDir)                   ("All")                  (always)


set  PRODUCT_NAME=%~1
set  scriptname=%~2
set  Win32_Release_filename=%~3
set  PRODUCT_DIST_INDIR=%~4
set  PRODUCT_SETUP_OUTDIR=%~5


REM  The following is for script debugging purposes...

goto :skip
echo  %0: PRODUCT_NAME            =  %PRODUCT_NAME%
echo  %0: scriptname              =  %scriptname%
echo  %0: Win32_Release_filename  =  %Win32_Release_filename%
echo  %0: PRODUCT_DIST_INDIR      =  %PRODUCT_DIST_INDIR%
echo  %0: PRODUCT_SETUP_OUTDIR    =  %PRODUCT_SETUP_OUTDIR%
:skip


::-----------------------------------------------------------------------------
::
::  We need to define four environment variables that our NSIS build script
::  (processed by the "makensis.exe" compiler) expects to be set as follows:
::
::     PRODUCT_DIST_INDIR       Product distribution staging directory
::                              (where our product files were copied to
::                              to make it easier for makensis to find)
::                              This value is currenty passed to us as
::                              a command-line argument.
::
::     PRODUCT_SETUP_OUTDIR     This project's $(OutDir), i.e. where the
::                              resulting setup program should be placed.
::                              This value is currenty passed to us as a
::                              command-line argument.
::
::     PRODUCT_VERSION_SHORT    The major, intermediate, minor values from
::                              our product's version# (i.e. just the first
::                              three numbers, *without* the build number)
::                              The value for this variable is determined
::                              dynamically by examining a product binary.
::
::     PRODUCT_BUILDNUM         The last component of our version# string,
::                              being the automatically-incremented build
::                              number. (See "AutoBuildCount.h") The value
::                              for this variable is determined dynamically
::                              by examining a product binary.
::
::  The 'PRODUCT_DIST_INDIR' and 'PRODUCT_SETUP_OUTDIR' values are passed
::  to us on the command-line. 'PRODUCT_VERSION_SHORT' and 'PRODUCT_BUILDNUM'
::  however are extracted dynamically from an actual product binary that
::  should already have been previously built.
::
::-----------------------------------------------------------------------------

set rc=0
set V=2
if not "%_DEBUG%" == "0" set V=4


::-----------------------------------------------------------------------------
:: Copy needed files to PRODUCT_DIST_INDIR distribution staging directory...
::-----------------------------------------------------------------------------

if  not exist "%PRODUCT_DIST_INDIR%"   mkdir "%PRODUCT_DIST_INDIR%"

REM  call :WINXXCPY                  REM already done by WinZip project
REM  if not %rc% equ 0 goto :exit    REM already done by WinZip project


::-----------------------------------------------------------------------------
:: Preliminary checks...
::-----------------------------------------------------------------------------

if  not exist "%PRODUCT_DIST_INDIR%\%Win32_Release_filename%"  (

    echo %0^(1^) : error C9999 : 1 error^(s^) in %0: PRODUCT_DIST_INDIR does not exist or is empty!
    echo %0^(1^) : ^(did you build WinZip project first?^)
    set rc=1
    exit /b %rc%
)


::-----------------------------------------------------------------------------
:: Extract version# directly from executable...
::-----------------------------------------------------------------------------

for /f %%i in ('verinfo "%PRODUCT_DIST_INDIR%\%Win32_Release_filename%" ^| grep -i "ProductVersion" ^| cut -c 18-') do set VVMMRRBB=%%i


::-----------------------------------------------------------------------------
:: Break version# into separate pieces...
::-----------------------------------------------------------------------------
::
::  PROGRAMMING NOTE: in the below "(set PRODUCT_VERSION_SHORT=%%a.%%b.%%c&&set PRODUCT_BUILDNUM=%%d)"
::  clause, there is NO SPACE between the '%%c' and the '&&' ampersands and 'set' command.
::  This is done on purpose so that there are no trailing spaces in the value being set.
::
::-----------------------------------------------------------------------------

for /f "tokens=1,2,3,4 delims=." %%a in ('echo %VVMMRRBB%') do (set PRODUCT_VERSION_SHORT=%%a.%%b.%%c&&set PRODUCT_BUILDNUM=%%d)

set PRODUCT_VERSION_LONG=%PRODUCT_VERSION_SHORT%.%PRODUCT_BUILDNUM%


::-----------------------------------------------------------------------------
:: Delete anything left over from previous build...
::-----------------------------------------------------------------------------

if  not exist "%PRODUCT_SETUP_OUTDIR%"   mkdir "%PRODUCT_SETUP_OUTDIR%"

if  exist "%PRODUCT_SETUP_OUTDIR%\*.exe"  del /f /q "%PRODUCT_SETUP_OUTDIR%\*.exe"  > NUL
if  exist "%PRODUCT_SETUP_OUTDIR%\*.log"  del /f /q "%PRODUCT_SETUP_OUTDIR%\*.log"  > NUL


::-----------------------------------------------------------------------------
::  Now  build our various installer flavors...
::-----------------------------------------------------------------------------

REM  ** The below constants MUST MATCH the ones !defined in the installer! **

set STD_INSTALLER=1
set DEV_INSTALLER=2
set MIN_INSTALLER=3
set WEB_INSTALLER=4

if /i "%RUNJOBS_DEFAULT_MAXJOBS%" == "" (

  echo. 
  echo Building installers...
  echo. 
  goto :makesetup
)


::-------------------------------------
::  Pass 1: build RUNJOBS file
::-------------------------------------

set JOBSFILE=makesetup.jobs
if exist "%JOBSFILE%" del "%JOBSFILE%"
set /a jobnum=0
set pass=1
echo. 


:makesetup
::-------------------------------------
::  Win32  "x86"  installers...
::-------------------------------------

set   _WIN64=0
set    ARCH=x86

call  :make_platform_installer_types
if    not %rc% equ 0  goto :exit


::-------------------------------------
::  Win64  "x64"  installers...
::-------------------------------------

set   _WIN64=1
set    ARCH=x64

call  :make_platform_installer_types
if    not %rc% equ 0  goto :exit

if /i "%RUNJOBS_DEFAULT_MAXJOBS%" == "" goto :exit


::-------------------------------------
::  Pass 2: run jobs, parse results
::-------------------------------------

if /i "%pass%" == "2" goto :exit

echo. 
echo Building installers...
echo. 

RUNJOBS -j 0 "%JOBSFILE%"

set pass=2
goto :makesetup


::-----------------------------------------------------------------------------
::  Exit w/Return Code...
::-----------------------------------------------------------------------------

:exit
echo. 
exit /b %rc%


::*****************************************************************************
::           'make_platform_installer_types' called subroutine
::*****************************************************************************

:make_platform_installer_types

    if /i "%pass%" == "1" set /a jobnum=%jobnum%+1
    if /i "%pass%" == "1" echo Job %jobnum% = %ARCH% STD installer...
    if /i "%JOBSFILE%" == "" echo    Building %ARCH% STD installer...

    set  _INSTALLER_TYPE=%STD_INSTALLER%
    set  _OUTFILE=%PRODUCT_NAME%_%PRODUCT_VERSION_LONG%_%ARCH%_std_setup.exe
    set  logfilepath=%PRODUCT_SETUP_OUTDIR%\%scriptname%.%ARCH%.std.log

    call  :makensis
    if    not %rc% equ 0  goto :EOF
    if /i "%JOBSFILE%" == "" echo. 

    ::-------------------------------------------------------------------------

    if /i "%pass%" == "1" set /a jobnum=%jobnum%+1
    if /i "%pass%" == "1" echo Job %jobnum% = %ARCH% WEB installer...
    if /i "%JOBSFILE%" == "" echo    Building %ARCH% WEB installer...

    set  _INSTALLER_TYPE=%WEB_INSTALLER%
    set  _OUTFILE=%PRODUCT_NAME%_%PRODUCT_VERSION_LONG%_%ARCH%_web_setup.exe
    set  logfilepath=%PRODUCT_SETUP_OUTDIR%\%scriptname%.%ARCH%.web.log

    call  :makensis
    if    not %rc% equ 0  goto :EOF
    if /i "%JOBSFILE%" == "" echo. 

    goto :EOF


::*****************************************************************************
::                    'makensis' called subroutine
::*****************************************************************************

:makensis

    set MAKENSIS_CMD=makensis.exe  /V%V%  /D_DEBUG=%_DEBUG%  /D_WIN64=%_WIN64%  /D_OUTFILE=%_OUTFILE%  /D_INSTALLER_TYPE=%_INSTALLER_TYPE%  "/O%logfilepath%"  "%scriptname%"

    if /i "%JOBSFILE%" == "" (

        %MAKENSIS_CMD%
        u2d "%logfilepath%" > NUL

    ) else (

        if /i "%pass%" == "1" (

            echo %MAKENSIS_CMD% >> "%JOBSFILE%"
            set rc=0
            goto :EOF

        ) else (

            u2d "%logfilepath%" > NUL
        )
    )

    egrep  -B 5  -i  "^Error"  "%logfilepath%"  > "egrep.txt"
    if  not %errorlevel% equ 0  goto :makensis_no_errors

    echo ...
    type "egrep.txt"

    for /f %%i in ('wc --lines "%logfilepath%"') do set linenum=%%i

    echo %logfilepath%(%linenum%) : error C9999 : 1 error(s) or more detected in script "%scriptname%"; setup creation aborted.
    echo %scriptname%(1) : (cause is probably in here somewhere)

    set rc=1

:makensis_no_errors

    egrep  -B 5  -i  "^warning:"   "%logfilepath%"  > "egrep.txt"
    if  not %errorlevel% equ 0  goto :makensis_no_warnings

    echo ...
    type "egrep.txt"

    echo %logfilepath%(1) : warning C9999 : 1 warning(s) or more detected in script "%scriptname%".
    echo %scriptname%(1) : (cause is probably in here somewhere)

:makensis_no_warnings

    if  exist "egrep.txt"  del /f /q "egrep.txt" > NUL

    goto :EOF


::*****************************************************************************

