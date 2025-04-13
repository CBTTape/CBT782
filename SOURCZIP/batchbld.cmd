@echo off

::------------------------------------------------------

echo. 
echo *** BATCHBLD STARTED ***
echo. 

set VCBUILDTYPE=%1
set SolutionDir=%~2
set SolutionPath=%SolutionDir%\foo.sln

set VCBUILD_DEFAULT_OPTIONS=/M4

::------------------------------------------------------

if not "%VS90COMNTOOLS%" == "" (

    rem note "vSvars.bat", not "vCvars.bat"!

    call "%VS90COMNTOOLS%vsvars32.bat"

) else if not "%VS80COMNTOOLS%" == "" (

    rem note "vSvars.bat", not "vCvars.bat"!

    call "%VS80COMNTOOLS%vsvars32.bat"

) else (

    echo %0: ERROR: Neither VS2005 nor VS2008 is installed!
    set rc=-1
    goto :error
)

::------------------------------------------------------

runjobs "awsbrowse-vcbuild.jobs"
set rc=%ERRORLEVEL%
if %rc% GTR 0 goto :error

::------------------------------------------------------

VCBUILD /%VCBUILDTYPE%   "..\NSIS\NSIS.vcproj" "Release|Win32" /time /nologo
set rc=%ERRORLEVEL%
if %rc% GTR 0 goto :error

VCBUILD /%VCBUILDTYPE%   "WinZip.vcproj"       "All|Win32"     /time /nologo
set rc=%ERRORLEVEL%
if %rc% GTR 0 goto :error

VCBUILD /%VCBUILDTYPE%   "Setup\Setup.vcproj"  "All|Win32"     /time /nologo
set rc=%ERRORLEVEL%
if %rc% GTR 0 goto :error

::------------------------------------------------------

echo. 
echo *** BATCHBLD SUCCESSFUL COMPLETION ***
echo. 
exit /B %rc%

:error
echo. 
echo *** BATCHBLD ERRORS OR WARNINGS DETECTED ***
echo. 
exit /B %rc%

::------------------------------------------------------
