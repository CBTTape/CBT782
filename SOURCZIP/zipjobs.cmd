@echo off

REM ***************************************************************************
REM **                        zipjobs.cmd                                    **
REM ** --------------------------------------------------------------------- **
REM **                                                                       **
REM **    This batch file, after calling the WinXXCpy.cmd batch file,        **
REM **    is responsible for creating the .ZIP file that contains the        **
REM **    complete Visual Studio project source code for our product.        **
REM **                                                                       **
REM ***************************************************************************

REM ***************************************************************************
REM **  Reminder: this batch file invoked as a project "Post-Build Event"...
REM ***************************************************************************

REM ***************************************************************************
REM **  Reminder: this file is expected to reside in the main $(SolutionDir)
REM **            directory and be invoked with it as its current directory...
REM ***************************************************************************


::-----------------------------------------------------------------------------
::  Set work variables based on passed variables but without the quotes

set SolutionDir=%~1


::-----------------------------------------------------------------------------
:: If the expected $(SolutionDir) variable wasn't passed, presume the current
:: directory (i.e. where we're presumably running from) is the same thing...

if "%SolutionDir%" == "" set SolutionDir=%cd%


goto :skip
echo %0: SolutionDir = %SolutionDir%
:skip


::-----------------------------------------------------------------------------
:: Define the binary to use for extracting our version information from...
::
:: (we don't really need to set it so early in the batch file but it's handy
:: to have it right at the top so we can see it since its value is different
:: for each product's 'zipjobs.cmd' file...)

set Release_filename=AWSBrowse32.exe


::-----------------------------------------------------------------------------
:: Copy all needed files from their $(OutDir) output directories
:: into the distribution staging directory for easier access...

call  WinXXCpy.cmd  "%SolutionDir%"

if not %errorlevel% equ 0 exit /b %errorlevel%


::-----------------------------------------------------------------------------
:: Delete any temporary work files left over from last run...

if exist "*.zip"      del /f /q "*.zip"     > NUL
if exist "*temp.WJF"  del /f /q "*temp.WJF" > NUL


::-----------------------------------------------------------------------------
:: Extract version# directly from executable...

REM                 ** PROGRAMMING NOTE **
REM
REM  The below grep, cut, and sed programs are pure Win32 ports of
REM  the *nix originals obtained from the "UnxUtils" and "GnuWin32"
REM  packages since I no longer have Cygwin installed (search the
REM  web for where to find them).
REM
REM  The handy "verinfo" utility I found elsewhere on the web. (Do
REM  a search and you'll find it.)

for /f %%i in ('verinfo dist^\%Release_filename% ^| grep -i "ProductVersion" ^| cut -c 18-') do set vvmmrrbb=%%i


::-----------------------------------------------------------------------------
:: Break version# into separate pieces...

REM                 ** PROGRAMMING NOTE **
REM
REM  In the below "(set PRODUCT_VERSION_SHORT=%%a.%%b.%%c&&set PRODUCT_BUILDNUM=%%d)"
REM  clause, there is NO SPACE between the '%%c' and the '&&' ampersands and 'set' command.
REM  This is done on purpose so that there are no trailing spaces in the value being set.

for /f "tokens=1,2,3,4 delims=." %%a in ('echo %vvmmrrbb%') do (set vvmmrr=%%a.%%b.%%c&&set bb=%%d)


::-----------------------------------------------------------------------------
:: Plug actual version# into [temporary] WinZip job files...

type src.WJF | sed --text s/VVV.MMM.RRR.BBB/%vvmmrrbb%/ > src-temp.WJF


::-----------------------------------------------------------------------------
:: Now run those [temporary] WinZip jobs...

REM                 ** PROGRAMMING NOTE **
REM
REM  We need to do it this way (creating a temporary copy of the
REM  actual job we wish to run and then run that temporary copy
REM  instead of the original) because for some odd reason WinZip
REM  refuses to include the job it's executing in with the rest of
REM  the files it's including in the job. Thus we make a copy of
REM  the actual job we wish to run and then run that temporary copy
REM  instead, so that the original job (that we made a copy of)
REM  thus gets included in with the rest of the files being zipped
REM  since it's not being executed; only the temp copy is (but not
REM  the original one that was copied so we could run it).
REM
REM  Another good/valid reason for doing it this way is so we can
REM  modify the original "generic" job so it uses filename values
REM  that contain the actual version of the binaries we just built.

src-temp.WJF


::-----------------------------------------------------------------------------
:: Delete the [temporary] WinZip jobs now that we're done with them...

if exist "*temp.WJF"  del /f /q "*temp.WJF" > NUL


::-----------------------------------------------------------------------------
:: Move the just-created zip files to the product distribution directory...

move  /y   "*.zip"   "dist"


::-----------------------------------------------------------------------------
:: Make a copy of it but without the version# in its name for web installs...

set basename=AWSBrowse
copy  /B  /Y  "%SolutionDir%\dist\%basename%_%vvmmrrbb%_src.zip"  "%SolutionDir%\dist\%basename%_%vvmmrr%_src.zip"


::-----------------------------------------------------------------------------
:: We're done... Exit...

goto :EOF
