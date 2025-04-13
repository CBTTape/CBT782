@echo off

REM ***************************************************************************
REM **                        WinXXCpy.cmd                                   **
REM ** --------------------------------------------------------------------- **
REM **                                                                       **
REM **    This batch file simply copies everything to be distributed         **
REM **    into the distribution staging directory to make it easier          **
REM **    for the Setup installation program project to find them            **
REM **                                                                       **
REM ***************************************************************************

REM ***************************************************************************
REM **  Reminder: this batch file is called by "zipjobs.cmd"...
REM ***************************************************************************

REM ***************************************************************************
REM **  Reminder: this file is expected to reside in the main $(SolutionDir)
REM **            directory and be invoked with it as its current directory...
REM ***************************************************************************


::-----------------------------------------------------------------------------
::  Set work variables based on passed variables but without the quotes

set SolutionDir=%~1

goto :skip
echo %0: SolutionDir = %SolutionDir%
:skip


::-----------------------------------------------------------------------------
:: If the expected $(SolutionDir) variable wasn't passed, presume the current
:: directory (i.e. where we're presumably running from) is the same thing...

if "%SolutionDir%" == "" set SolutionDir=%cd%


::-----------------------------------------------------------------------------
:: Create O/P directories...

if not exist "%SolutionDir%\dist"      mkdir "%SolutionDir%\dist"


::-----------------------------------------------------------------------------
:: Set the initial return-code value...

set rc=0


::-----------------------------------------------------------------------------
:: Copy miscellaneous files...

for %%i in (

    LICENSE.rtf
    README.rtf

) do call :XCOPY_MISC "%%i"

if not %rc% equ 0 goto :exit


::-----------------------------------------------------------------------------
:: Copy the zlib & bzip2 DLLs...

xcopy  "%ZLIB_DIR%\zlib1.dll"           "%SolutionDir%\dist"   /V /C /F /H /R /K /O /X /Y
xcopy  "%BZIP2_DIR%\libbz2.dll"         "%SolutionDir%\dist"   /V /C /F /H /R /K /O /X /Y

ren    "%SolutionDir%\dist\zlib1.dll"   "zlib1_32.dll"
ren    "%SolutionDir%\dist\libbz2.dll"  "libbz2_32.dll"

xcopy  "%ZLIB_DIR%\x64\zlib1.dll"       "%SolutionDir%\dist"   /V /C /F /H /R /K /O /X /Y
xcopy  "%BZIP2_DIR%\x64\libbz2.dll"     "%SolutionDir%\dist"   /V /C /F /H /R /K /O /X /Y

ren    "%SolutionDir%\dist\zlib1.dll"   "zlib1_64.dll"
ren    "%SolutionDir%\dist\libbz2.dll"  "libbz2_64.dll"


::-----------------------------------------------------------------------------
:: Now copy the actual EXE files...

set  BinName=AWSBrowse
set  BinExt=exe
call :DO_XCOPY  "Release"       32
call :DO_XCOPY  "x64\Release"   64
if not %rc% equ 0 goto :exit


::-----------------------------------------------------------------------------
:: DONE!
::-----------------------------------------------------------------------------

:exit
exit /b %rc%


::-----------------------------------------------------------------------------
::                       (CALLED SUBROUTINES)
::-----------------------------------------------------------------------------

:XCOPY_MISC
set filename=%~1
xcopy "%SolutionDir%\%filename%"  "%SolutionDir%\dist"  /V /C /F /H /R /K /O /X /Y
if not %errorlevel% equ 0 set rc=1
goto :EOF

:XCOPY_ALL
call :XCOPY_ALL_32
call :XCOPY_ALL_64
goto :EOF

:XCOPY_ALL_PLAIN
call :XCOPY_ALL_32_PLAIN
call :XCOPY_ALL_64_PLAIN
goto :EOF

:XCOPY_ALL_32
call :XCOPY_MULTIBYTE_32
call :XCOPY_UNICODE_32
goto :EOF

:XCOPY_ALL_64
call :XCOPY_MULTIBYTE_64
call :XCOPY_UNICODE_64
goto :EOF

:XCOPY_ALL_32_PLAIN
call :XCOPY_MULTIBYTE_32_PLAIN
call :XCOPY_UNICODE_32_PLAIN
goto :EOF

:XCOPY_ALL_64_PLAIN
call :XCOPY_MULTIBYTE_64_PLAIN
call :XCOPY_UNICODE_64_PLAIN
goto :EOF

:XCOPY_MULTIBYTE_32
call :DO_XCOPY          "Debug"         32D
call :DO_XCOPY          "Release"       32
goto :EOF

:XCOPY_MULTIBYTE_64
call :DO_XCOPY          "x64\Debug"     64D
call :DO_XCOPY          "x64\Release"   64
goto :EOF

:XCOPY_UNICODE_32
call :DO_XCOPY          "UDebug"        32UD
call :DO_XCOPY          "URelease"      32U
goto :EOF

:XCOPY_UNICODE_64
call :DO_XCOPY          "x64\UDebug"    64UD
call :DO_XCOPY          "x64\URelease"  64U
goto :EOF

:XCOPY_MULTIBYTE_32_PLAIN
call :DO_XCOPY          "Debug"         D
call :DO_XCOPY          "Release"       
goto :EOF

:XCOPY_MULTIBYTE_64_PLAIN
call :DO_XCOPY          "x64\Debug"     D
call :DO_XCOPY          "x64\Release"   
goto :EOF

:XCOPY_UNICODE_32_PLAIN
call :DO_XCOPY          "UDebug"        UD
call :DO_XCOPY          "URelease"      U
goto :EOF

:XCOPY_UNICODE_64_PLAIN
call :DO_XCOPY          "x64\UDebug"    UD
call :DO_XCOPY          "x64\URelease"  U
goto :EOF

:DO_XCOPY
set OutDir=%~1
set BinSuffix=%~2
xcopy "%SolutionDir%\%ProjName%\%OutDir%\%BinName%%BinSuffix%.%BinExt%"  "%SolutionDir%\dist"  /V /C /F /H /R /K /O /X /Y
if not %errorlevel% equ 0 set rc=1
goto :EOF


::--------------------------------- ( E O F ) ---------------------------------
