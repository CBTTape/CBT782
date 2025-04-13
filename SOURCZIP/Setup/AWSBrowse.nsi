/***************************************************************************/
/*      ${PRODUCT_NAME}.nsi             NullSoft Installer script          */
/***************************************************************************/
/*                                                                         */
/*  Change History:                                                        */
/*                                                                         */
/*  MM/DD/YY   XXXXXXXXXXXXXXXX.........                                   */
/*                                                                         */
/***************************************************************************/

!AddIncludeDir  "${NSISDIR}\include\local"     ; (so we can find pvt hdrs)

!include "product.nsh"              ; "PRODUCT_NAME", etc...  (** MUST BE FIRST! **)
!include "MUI.nsh"                  ; use Modern User-Interface (MUI)
!include "FileFunc.nsh"             ; need "GetFileAttributes", "GetFileVersion", etc
!include "TextFunc.nsh"             ; need "TrimNewLines", etc.
!include "WordFunc.nsh"             ; need "WordFind", "VersionCompare", etc
!define  AddToPathOnly              ; skip "AddToEnvVar" for AWSBrowse
!include "EnvVar.nsh"               ; need "AddToPath", "AddToEnvVar", etc.
!include "x64.nsh"                  ; need "RunningX64"
!include "CkIfInst.nsh"             ; Need "IsProductInstalled" macro...
!include "VCRedist.nsh"             ; need "Install_SP1_X86_VC80Redistributables_IfNeeded"
!ifdef  WEB_INSTALLER
!include "GetParms.nsh"             ; need "GetParameterValue" cmdline arg parser
!endif /* WEB_INSTALLER */

/*-------------------------------------------------------------------------*/
/*                    Identification / Options                             */
/*-------------------------------------------------------------------------*/

Name               "${PRODUCT_NAME} version ${PRODUCT_VERSION_SHORT}"
OutFile            "${PRODUCT_SETUP_OUTDIR}\${PRODUCT_SETUP_EXENAME}"
InstallDir         "${PRODUCT_DEFAULT_INSTALLDIR}"

!ifdef  WEB_INSTALLER
ShowInstDetails    nevershow    /* (details messup download progress page) */
!else
ShowInstDetails    show
!endif
ShowUnInstDetails  show         /* (ALWAYS show full UNinstall details)    */
XPStyle            on

/*-------------------------------------------------------------------------*/
/*                          Strings Table                                  */
/*-------------------------------------------------------------------------*/

var        hUninstLog
!define     UninstLogFilename       "${PRODUCT_NAME}_InstallLog.txt"
!define     UninstEXEName           "Uninstall_${PRODUCT_NAME}.exe"
!define     STR_LANGUAGE             LANG_ENGLISH
LangString  PlatformError            ${STR_LANGUAGE}  "This install package is designed only for 64-bit Windows systems."
LangString  InstallInUseError        ${STR_LANGUAGE}  "$\t$\t$\t$\t$\t***  E R R O R  ***$\n$\n\
${PRODUCT_NAME} is currently in use. \
A new version of ${PRODUCT_NAME} cannot be installed while the current version is still in use. \
Please close any programs that may still be using ${PRODUCT_NAME} and try again."
LangString  UninstallInUseError      ${STR_LANGUAGE}  "$\t$\t$\t$\t$\t***  E R R O R  ***$\n$\n\
${PRODUCT_NAME} is currently in use. \
${PRODUCT_NAME} cannot be uninstalled while the current version is still in use. \
Please close any programs that may still be using ${PRODUCT_NAME} and try again."
LangString  BackLevelError           ${STR_LANGUAGE}  "$\t$\t$\t$\t$\t***  W A R N I N G  ***$\n$\n\
A NEWER version of ${PRODUCT_NAME} is ALREADY INSTALLED! \
Installing an OLDER version of ${PRODUCT_NAME} may break existing ${PRODUCT_COMPANY_SHORT} products! \
Are you SURE you want to install an OLDER version of ${PRODUCT_NAME}?"
LangString  MainBinaries_SEC_Desc    ${STR_LANGUAGE}  "The main ${PRODUCT_NAME} binaries needed to run ${PRODUCT_DESCRIPTION}."
LangString  SourceCode_SEC_Desc      ${STR_LANGUAGE}  "The .ZIP file containing the complete Visual C++ 2005 project source code for ${PRODUCT_NAME}."
LangString  UninstLogMissing         ${STR_LANGUAGE}  "Uinstall control file $\"${UninstLogFilename}$\" not found!$\r$\nUninstallation is not possible!"
!define     DelRegEntsPageINIFilename                 "UnRemoveRegEntriesPage.ini"
LangString  DelRegEntsPageTitle      ${STR_LANGUAGE}  "Registry Settings"
LangString  DelRegEntsPageSubtitle   ${STR_LANGUAGE}  "Choose whether to also remove your settings from the registry."
!ifdef  WEB_INSTALLER
!define     ChooseMirrorPageINIFilename               "ChooseMirrorPage.ini"
LangString  ChooseMirrorPageTitle    ${STR_LANGUAGE}  "Select Mirror"
LangString  ChooseMirrorPageSubtitle ${STR_LANGUAGE}  "Please choose the mirror closest to you where the files should be downloaded from."
LangString  AskConnect               ${STR_LANGUAGE}  "May I connect to the internet?"
LangString  ConnectError             ${STR_LANGUAGE}  "Attempt to connect to internet failed."
LangString  NoConnection             ${STR_LANGUAGE}  "No connection to internet; installation aborted."
LangString  NoCmdlineMirror          ${STR_LANGUAGE}  "No '/MIRROR=' value passed on command-line!"
!endif   /* WEB_INSTALLER */
!ifndef  MIN_INSTALLER
LangString  InitFishLibInstall       ${STR_LANGUAGE}  "Initiating automatic installation of ${FISHLIB_PRODUCT_NAME} dependency..."
LangString  DownloadingFishLib       ${STR_LANGUAGE}  "Downloading ${FISHLIB_PRODUCT_NAME} installer..."
LangString  FishLibDownloadFailed    ${STR_LANGUAGE}  "Download of '${FISHLIB_SETUP_EXENAME}' failed!"
LangString  LaunchingFishLib         ${STR_LANGUAGE}  "Launching ${FISHLIB_PRODUCT_NAME} installer..."
LangString  FishLibInstallSuccess    ${STR_LANGUAGE}  "${FISHLIB_PRODUCT_NAME} successfully installed."
LangString  AutoFishLibInstallFailed ${STR_LANGUAGE}  "Automatic installation of ${FISHLIB_PRODUCT_NAME} FAILED!"
LangString  AutoFishLibFailedMsgBox  ${STR_LANGUAGE}  "**ERROR!**: Automatic installation of ${FISHLIB_PRODUCT_NAME} appears to have failed! \
You will need to manually install ${FISHLIB_PRODUCT_NAME} yourself before ${PRODUCT_NAME} will work correctly."
!endif /* !MIN_INSTALLER */

/*-------------------------------------------------------------------------*/
/*                     Support macros/functions                            */
/*-------------------------------------------------------------------------*/

!define  File               "!insertmacro File"
!define  CopyFiles          "!insertmacro CopyFiles"
!define  CreateDirectory    "!insertmacro CreateDirectory"
!define  CreateShortCut     "!insertmacro CreateShortCut"
!define  CreateURLShortCut  "!insertmacro CreateURLShortCut"

!ifndef  MIN_INSTALLER
!insertmacro  WordFind
!insertmacro  WordReplace
!endif /* !MIN_INSTALLER */

!insertmacro  GetFileVersion
!insertmacro  VersionCompare

!insertmacro  un.TrimNewLines
!insertmacro  un.FileReadFromEnd
!insertmacro  un.GetFileAttributes

/*-------------------------------------------------------------------------*/
/*                     Support macros/functions                            */
/*-------------------------------------------------------------------------*/
/*   PROGRAMMING NOTE: presumed default mode is "SetOverwrite on"          */
/*-------------------------------------------------------------------------*/

!ifdef  WEB_INSTALLER
  var     WebInstallURL
  var     WebInstall_URLBase
  !macro  File                 FilePath    FileName
    FileWrite  $hUninstLog      "$OUTDIR\${FileName}$\r$\n"
    Push $R0
    /* PROGRAMMING NOTE: the case of the Filename *MUST* match *EXACTLY*!! */
    inetc::get  /noproxy  "$WebInstallURL/${FileName}"  "${FileName}"  /end
    Pop $R0
    ${If} "$R0" != "OK"
        DetailPrint                    "Download of '${FileName}' failed: $R0"
        MessageBox  MB_ICONSTOP|MB_OK  "Download of '${FileName}' failed: $R0"
        Abort
    ${Endif}
    Pop $R0
  !macroend
!else /* (other installer) */
  !macro  File                 FilePath    FileName
    File                    "${FilePath}\${FileName}"
    FileWrite  $hUninstLog      "$OUTDIR\${FileName}$\r$\n"
  !macroend
!endif /* WEB_INSTALLER */

!macro CopyFiles  SourcePath       DestPath
    FileWrite  $hUninstLog      "${DestPath}$\r$\n"
    CopyFiles  "${SourcePath}"  "${DestPath}"
!macroend

!macro CreateDirectory            Dir
    /*
        PROGRAMMING NOTE: must be careful to log the directory WITHOUT an
        ending backslash, since that's the way $INSTDIR is formatted. Else
        our string comparison in "un.ProcessLogFileInReverse" fails, causing
        us to prematurely delete the $INSTDIR directory during uninstallation,
        thereby causing all remaining installed files (which got installed
        to $INSTDIR) to not get deleted!
    */
    CreateDirectory            "${Dir}"
    SetOutPath                 "${Dir}"
    FileWrite  $hUninstLog     "${Dir}$\r$\n"
!macroend

!macro CreateShortCut             ShortcutPathname             TargetPathname
    CreateShortCut             "${ShortcutPathname}"        "${TargetPathname}"
    FileWrite  $hUninstLog     "${ShortcutPathname}$\r$\n"
!macroend

!macro CreateURLShortCut          ShortcutPathname                                        TargetURL
    WriteIniStr                "${ShortcutPathname}"        "InternetShortcut"  "URL"  "${TargetURL}"
    FileWrite  $hUninstLog     "${ShortcutPathname}$\r$\n"
!macroend

/***************************************************************************/
/*                      User Interface layout                              */
/***************************************************************************/
!define       OMUI_THEME_PATH  "${NSISDIR}\Contrib\MUI Orange Vista Theme\CD-Clean"
/*-------------------------------------------------------------------------*/
!define       MUI_ICON                        "${OMUI_THEME_PATH}\installer-nopng.ico"
!define       MUI_UNICON                      "${OMUI_THEME_PATH}\uninstaller-nopng.ico"
/*-------------------------------------------------------------------------*/
!define       MUI_HEADERIMAGE
!define       MUI_HEADERIMAGE_RIGHT
!define       MUI_HEADERIMAGE_BITMAP          "${OMUI_THEME_PATH}\header-r.bmp"
!define       MUI_HEADERIMAGE_UNBITMAP        "${OMUI_THEME_PATH}\header-r-un.bmp"
/*-------------------------------------------------------------------------*/
!define       MUI_WELCOMEFINISHPAGE_BITMAP    "${OMUI_THEME_PATH}\wizard.bmp"
!define       MUI_UNWELCOMEFINISHPAGE_BITMAP  "${OMUI_THEME_PATH}\wizard-un.bmp"
/*-------------------------------------------------------------------------*/
!define       MUI_ABORTWARNING
/*-------------------------------------------------------------------------*/
!insertmacro  MUI_PAGE_WELCOME
/*-------------------------------------------------------------------------*/
!insertmacro  MUI_PAGE_LICENSE      "${PRODUCT_DIST_INDIR}\LICENSE.rtf"
/*-------------------------------------------------------------------------*/
!ifdef  WEB_INSTALLER
Page  custom  ChooseMirrorPage      ; (custom "Choose download mirror" page)
!endif
/*-------------------------------------------------------------------------*/
!insertmacro  MUI_PAGE_COMPONENTS
/*-------------------------------------------------------------------------*/
!insertmacro  MUI_PAGE_DIRECTORY
/*-------------------------------------------------------------------------*/
!define       MUI_STARTMENUPAGE_NODISABLE
!define       MUI_STARTMENUPAGE_DEFAULTFOLDER           "${PRODUCT_DEFAULT_SMGROUP}"
!define       MUI_STARTMENUPAGE_REGISTRY_VALUENAME      "${PRODUCT_STARTMENU_REGVAL}"
!define       MUI_STARTMENUPAGE_REGISTRY_ROOT           "${PRODUCT_UNINST_ROOT_KEY}"
!define       MUI_STARTMENUPAGE_REGISTRY_KEY            "${PRODUCT_UNINST_KEY}"
 var                                                      SMGROUP
!insertmacro  MUI_PAGE_STARTMENU    StartMenuPageId      $SMGROUP
/*-------------------------------------------------------------------------*/
!insertmacro  MUI_PAGE_INSTFILES
/*-------------------------------------------------------------------------*/
!define       MUI_FINISHPAGE_NOAUTOCLOSE
!define       MUI_FINISHPAGE_SHOWREADME     "$INSTDIR\README.rtf"
!insertmacro  MUI_PAGE_FINISH
/*-------------------------------------------------------------------------*/
!insertmacro  MUI_UNPAGE_WELCOME
/*-------------------------------------------------------------------------*/
UninstPage  custom  un.RemoveRegEntriesPage      ; (custom uninstall page)
/*-------------------------------------------------------------------------*/
!insertmacro  MUI_UNPAGE_CONFIRM
/*-------------------------------------------------------------------------*/
!insertmacro  MUI_UNPAGE_INSTFILES
/*-------------------------------------------------------------------------*/
!define       MUI_UNFINISHPAGE_NOAUTOCLOSE
!insertmacro  MUI_UNPAGE_FINISH
/*-------------------------------------------------------------------------*/
!insertmacro  MUI_LANGUAGE              "English"       ; (must come last)

/***************************************************************************/
/*  Reserve room up-front for any file(s) needed by any custom MUI pages   */
/***************************************************************************/

!ifdef  WEB_INSTALLER
!insertmacro    MUI_RESERVEFILE_INSTALLOPTIONS
ReserveFile     "${ChooseMirrorPageINIFilename}"
!endif /* WEB_INSTALLER */
ReserveFile     "${DelRegEntsPageINIFilename}"

/***************************************************************************/
/*                          Begin install logic...                         */
/***************************************************************************/

Function .onInit

    InitPluginsDir

    /* Verify we're running the correct platform installer... */

  !ifdef  WIN64
    ${Unless} ${RunningX64}
        MessageBox  MB_ICONSTOP|MB_OK "$(PlatformError)"
        Abort
    ${EndUnless}
  !endif

    /* Verify the product is not currently being used... */

    Call IsProductInUse
    ${If} $0 <> 0
        MessageBox  MB_ICONSTOP|MB_OK "$(InstallInUseError)"
        Abort
    ${EndIf}

    /*  Check to see if we're already installed and if we are  */
    /*  whether the version that's already installed is newer  */
    /*  or older than the version we're about to install...    */

    ${If} ${AWSBrowseIsInstalled}

        /* Compare the version of the already installed version against */
        /* our own version to see if they're installing a newer version */

        ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation"
        ${GetFileVersion} "$0\${PRODUCT_NAME}32.exe" $1
        strcpy $2 "${PRODUCT_VERSION_LONG}"

        /*   $1   installed version       */
        /*   $2   our version             */
        /*   $0   VersionCompare results  */

        ${VersionCompare} $1 $2 $0

        ; $0 == 0  both versions are equal to one another
        ; $0 == 1  first version (installed version) is newer
        ; $0 == 2  second version (our version)      is newer

        ${If} $0 == "1"      /* (trying to install an OLDER version?!) */

            /*  "Are you really, REALLY sure you want to do that??!!"  */

            ${If} ${Cmd} `MessageBox MB_ICONSTOP|MB_YESNO|MB_DEFBUTTON2 \
                                "$(BackLevelError)" /SD IDNO IDNO`

                /* They changed their mind...   Abort the install...     */

                /* PROGRAMMING NOTE: For silent installs we must be sure  */
                /* to exit silently (without error). Otherwise the caller */
                /* might mistakenly think the install failed when in fact */
                /* it actually didn't (since it is after all installed!)  */

                ${Unless} ${Silent}
                    Abort             /* (normal Abort if normal install) */
                ${Else}
                    SetErrorLevel 0   /* (RC=0 == successful completion)  */
                    Quit              /* (Successful Completion Exit...)  */
                ${EndUnless}

            ${EndIf}

        ${EndIf}

    ${EndIf} /* ${AWSBrowseIsInstalled} */

  !ifdef  WEB_INSTALLER

    /* Make sure we have an Internet connection to download our files... */

    Push $R0
    Call  ConnectToInternet
    ${If} "$R0" != "online"
        DetailPrint        "$(NoConnection)"
        MessageBox  MB_OK  "$(NoConnection)"
        Abort
    ${EndIf}
    Pop $R0

    /* Do normal web-install, or silent web-install... */

    ${Unless} ${Silent}

        /* Extract the "Choose Mirror" dialog user-interface layout... */

        !insertmacro  MUI_INSTALLOPTIONS_EXTRACT  "${ChooseMirrorPageINIFilename}"

        /* (see comments in function itself) */

        Call  SetSectionSizes

    ${Else}  /* (silent web-install..) */

        /* SILENT: Retrieve which mirror to use from the command-line... */

        Push  "MIRROR"                  ; name of desired argument
        Push  ""                        ; default value if not found
        Call  GetParameterValue         ; get parameter value
        Pop   $WebInstall_URLBase       ; retrieve value from stack

        ${If} $WebInstall_URLBase == ""
            DetailPrint                     "$(NoCmdlineMirror)"
            strcpy  $WebInstall_URLBase     "${DEFAULT_URLBASE}"
        ${EndIf}

        /* Set needed variables based on chosen value... */ 

        Call  SetMirror

    ${EndUnless}

  !endif  /* WEB_INSTALLER */

FunctionEnd

/*-------------------------------------------------------------------------*/
/*                          (open logfile)                                 */
/*-------------------------------------------------------------------------*/

Section  -OpenLogFile
    CreateDirectory            "$INSTDIR"
    ${If} ${FileExists}        "$INSTDIR\${UninstLogFilename}"
        SetFileAttributes      "$INSTDIR\${UninstLogFilename}"  NORMAL
        FileOpen  $hUninstLog  "$INSTDIR\${UninstLogFilename}"  a
        FileSeek  $hUninstLog                                   0 END
    ${Else}
        FileOpen  $hUninstLog  "$INSTDIR\${UninstLogFilename}"  w
    ${EndIf}
SectionEnd

/*-------------------------------------------------------------------------*/
/*                   Choose mirror for web-install                         */
/*-------------------------------------------------------------------------*/

!ifdef  WEB_INSTALLER

Function  ChooseMirrorPage

    ${Unless} ${Silent}

        /* Display the "Choose Mirror" dialog page... */

        !insertmacro  MUI_HEADER_TEXT  $(ChooseMirrorPageTitle)  $(ChooseMirrorPageSubtitle)
        !insertmacro  MUI_INSTALLOPTIONS_DISPLAY                "${ChooseMirrorPageINIFilename}"

        /* Retrieve user's dialog selection... */

        strcpy  $0   "${ChooseMirrorPageINIFilename}"   ; (dialog .INI filename)
        intop   $1   1 + 0                              ; 1st button's field#
        intop   $2   2 + 0                              ; #of buttons
        call    GetChosenRadioButtonNum                 ; call helper function
                                                        ; chosen button# now in $0
        /* Select the chosen mirror... */

        ${Switch} $0
            /*
            **  NOTE! The following 'cases'         MUST BE
            **  in the                        **>> SAME ORDER <<**
            **  as they appear on our           ChooseMirrorPage.ini
            **  dialog!
            */
            ${Default}

            ${Case} 0
                strcpy    $WebInstall_URLBase   "${NORTH_AMERICAN_URLBASE}"
                ${Break}

            ${Case} 1
                strcpy    $WebInstall_URLBase   "${EUROPEAN_URLBASE}"
                ${Break}

        ${EndSwitch}

        /* Set needed variables based on chosen values... */ 

        Call  SetMirror

    ${EndUnless}

FunctionEnd

Function  SetMirror  /*----------------------------------------------------*/

    /* Log chosen mirror... */

    DetailPrint  "/MIRROR=$WebInstall_URLBase"

    /* Create needed web-install URL dir variables based on chosen mirror  */

    strcpy  $VCRedistURL       "$WebInstall_URLBase/vcredist/${VCRedistVersion}"
    strcpy  $WebInstallURL     "$WebInstall_URLBase/${PRODUCT_NAME}/${PRODUCT_VERSION_SHORT}"

FunctionEnd

!endif  /* WEB_INSTALLER */

/*-------------------------------------------------------------------------*/
/*                          Main Binaries                                  */
/*-------------------------------------------------------------------------*/

Section    "!Main Binaries"      SEC_MainBinaries

    SectionIn       RO
    SetOverwrite    on

    ${CreateDirectory}                      "$INSTDIR"
    ${CopyFiles}  "$EXEPATH"                "$INSTDIR\${PRODUCT_SETUP_EXENAME}"
    ${File}       "${PRODUCT_DIST_INDIR}"   "${PRODUCT_NAME}32.exe"
    ${File}       "${PRODUCT_DIST_INDIR}"   "zlib1_32.dll"
    ${File}       "${PRODUCT_DIST_INDIR}"   "libbz2_32.dll"
  !ifdef WIN64
    ${File}       "${PRODUCT_DIST_INDIR}"   "${PRODUCT_NAME}64.exe"
    ${File}       "${PRODUCT_DIST_INDIR}"   "zlib1_64.dll"
    ${File}       "${PRODUCT_DIST_INDIR}"   "libbz2_64.dll"
  !endif
    ${File}       "${PRODUCT_DIST_INDIR}"   "LICENSE.rtf"
    ${File}       "${PRODUCT_DIST_INDIR}"   "README.rtf"

    Push  $INSTDIR          ; directory to add
    Call  AddToPath         ; append to Windows search PATH

SectionEnd

/*-------------------------------------------------------------------------*/
/*                          Source Code                                    */
/*-------------------------------------------------------------------------*/

Section  /o   "Source Code"     SEC_SourceCode

    SetOverwrite   on

    ${CreateDirectory}                      "$INSTDIR"
    ${File}     "${PRODUCT_DIST_INDIR}"     "${PRODUCT_SOURCE_ZIPNAME}"

SectionEnd

/*-------------------------------------------------------------------------*/
/*               Selection (Section) description text                      */
/*-------------------------------------------------------------------------*/

!insertmacro  MUI_FUNCTION_DESCRIPTION_BEGIN

    !insertmacro  MUI_DESCRIPTION_TEXT  ${SEC_MainBinaries}    "$(MainBinaries_SEC_Desc)"
    !insertmacro  MUI_DESCRIPTION_TEXT  ${SEC_SourceCode}      "$(SourceCode_SEC_Desc)"

!insertmacro  MUI_FUNCTION_DESCRIPTION_END

/*-------------------------------------------------------------------------*/
/*                      Finalize installation                              */
/*-------------------------------------------------------------------------*/

Section  -FinishUp

    WriteUninstaller  "$INSTDIR\${UninstEXEName}"

    /* Add entries to Start/Programs menu... */

    !insertmacro  MUI_STARTMENU_WRITE_BEGIN     StartMenuPageId

        ${CreateDirectory}    "$SMPROGRAMS\$SMGROUP"
        ${CreateShortCut}     "$SMPROGRAMS\$SMGROUP\${PRODUCT_NAME}32.lnk"      "$INSTDIR\${PRODUCT_NAME}32.exe"
      !ifdef WIN64
        ${CreateShortCut}     "$SMPROGRAMS\$SMGROUP\${PRODUCT_NAME}64.lnk"      "$INSTDIR\${PRODUCT_NAME}64.exe"
      !endif

        ${CreateShortCut}     "$SMPROGRAMS\$SMGROUP\${PRODUCT_NAME} Home.lnk"   "$INSTDIR\${PRODUCT_NAME} Home.url"
        ${CreateURLShortCut}  "$INSTDIR\${PRODUCT_NAME} Home.url"               "http://${PRODUCT_URL}"

        ${CreateShortCut}     "$SMPROGRAMS\$SMGROUP\${PRODUCT_NAME} Help.lnk"   "$INSTDIR\${PRODUCT_NAME} Help.url"
        ${CreateURLShortCut}  "$INSTDIR\${PRODUCT_NAME} Help.url"               "http://${PRODUCT_URL}/help"

        ${CreateShortCut}     "$SMPROGRAMS\$SMGROUP\Product directory.lnk"      "$INSTDIR"
        ${CreateShortCut}     "$SMPROGRAMS\$SMGROUP\README.lnk"                 "$INSTDIR\README.rtf"
        ${CreateShortCut}     "$SMPROGRAMS\$SMGROUP\LICENSE.lnk"                "$INSTDIR\LICENSE.rtf"
        ${CreateShortCut}     "$SMPROGRAMS\$SMGROUP\Uninstall.lnk"              "$INSTDIR\${UninstEXEName}"

    !insertmacro  MUI_STARTMENU_WRITE_END

    /* Add entries to "Add/Remove Programs" registry... */

    WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "ProductID"         "${PRODUCT_NAME}"
    WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "DisplayName"       "$(^Name)"
    WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "DisplayVersion"    "${PRODUCT_VERSION_SHORT}"
/*  WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "DisplayIcon"       "foobar.exe,0"  */
    WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "Publisher"         "${PRODUCT_COMPANY_LONG}"
    WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "URLInfoAbout"      "http://${PRODUCT_URL}"
    WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "URLUpdateInfo"     "http://${PRODUCT_URL}"
    WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "HelpLink"          "http://${PRODUCT_URL}/help"
    WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "InstallLocation"   "$INSTDIR"
    WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "UninstallString"   "$INSTDIR\${UninstEXEName}"
    WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "Readme"            "$INSTDIR\README.rtf"
    WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "Comments"          "${PRODUCT_DESCRIPTION}"
    WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "Contact"           "${PRODUCT_EMAIL}"
    WriteRegStr    ${PRODUCT_UNINST_ROOT_KEY}  "${PRODUCT_UNINST_KEY}"  "ModifyPath"        "$INSTDIR\${PRODUCT_SETUP_EXENAME}"

    /* Add any needed additional product registry entries... */

    Call  AddProductRegistryEntries

!ifndef  MIN_INSTALLER

    /* Handle additional dependencies/prerequisites... */

    Call  Install_FishLib_IfNeeded              /* FishLib */
    Call  Install_VCRedist_IfNeeded             /* VC++    */

!endif /* !MIN_INSTALLER */

SectionEnd

/*-------------------------------------------------------------------------*/
/*                   AddProductRegistryEntries                             */
/*-------------------------------------------------------------------------*/
/*        (Function to ADD any needed product registry entries)            */
/*-------------------------------------------------------------------------*/

Function  AddProductRegistryEntries

    /*---------------------------------------------------*/
    /*   Associate ".AWS" file-type to our program       */
    /*---------------------------------------------------*/

    WriteRegStr         HKEY_CLASSES_ROOT                 \
        ".aws"                                            \
        ""                                                \
        "AWS.Tape.File"

    WriteRegStr         HKEY_CLASSES_ROOT                 \
        "AWS.Tape.File"                                   \
        ""                                                \
        "AWS.Tape.File"

    WriteRegDWORD       HKEY_CLASSES_ROOT                 \
        "AWS.Tape.File"                                   \
        "EditFlags"                                       \
        0x00000000

    WriteRegStr         HKEY_CLASSES_ROOT                 \
        "AWS.Tape.File\DefaultIcon"                       \
        ""                                                \
        "$INSTDIR\${PRODUCT_NAME}32.exe,1"

  !ifdef WIN64
    WriteRegStr         HKEY_CLASSES_ROOT                 \
        "AWS.Tape.File\shell\open\command"                \
        ""                                                \
        '"$INSTDIR\${PRODUCT_NAME}64.exe" "%1"'
  !else
    WriteRegStr         HKEY_CLASSES_ROOT                 \
        "AWS.Tape.File\shell\open\command"                \
        ""                                                \
        '"$INSTDIR\${PRODUCT_NAME}32.exe" "%1"'
  !endif

    /*---------------------------------------------------*/
    /*   Associate ".HET" file-type to our program       */
    /*---------------------------------------------------*/

    WriteRegStr         HKEY_CLASSES_ROOT                 \
        ".het"                                            \
        ""                                                \
        "Hercules.Emulated.Tape.File"

    WriteRegStr         HKEY_CLASSES_ROOT                 \
        "Hercules.Emulated.Tape.File"                     \
        ""                                                \
        "Hercules.Emulated.Tape.File"

    WriteRegDWORD       HKEY_CLASSES_ROOT                 \
        "Hercules.Emulated.Tape.File"                     \
        "EditFlags"                                       \
        0x00000000

    WriteRegStr         HKEY_CLASSES_ROOT                 \
        "Hercules.Emulated.Tape.File\DefaultIcon"         \
        ""                                                \
        "$INSTDIR\${PRODUCT_NAME}32.exe,4"

  !ifdef WIN64
    WriteRegStr         HKEY_CLASSES_ROOT                 \
        "Hercules.Emulated.Tape.File\shell\open\command"  \
        ""                                                \
        '"$INSTDIR\${PRODUCT_NAME}64.exe" "%1"'
  !else
    WriteRegStr         HKEY_CLASSES_ROOT                 \
        "Hercules.Emulated.Tape.File\shell\open\command"  \
        ""                                                \
        '"$INSTDIR\${PRODUCT_NAME}32.exe" "%1"'
  !endif

    /*---------------------------------------------------*/
    /*      Associate ".TDF" file-type to notepad        */
    /*---------------------------------------------------*/

    WriteRegStr         HKEY_CLASSES_ROOT                 \
        ".tdf"                                            \
        ""                                                \
        "Tape.Descriptor.File"

    WriteRegStr         HKEY_CLASSES_ROOT                 \
        "Tape.Descriptor.File"                            \
        ""                                                \
        "Tape.Descriptor.File"

    WriteRegDWORD       HKEY_CLASSES_ROOT                 \
        "Tape.Descriptor.File"                            \
        "EditFlags"                                       \
        0x00000000

    WriteRegStr         HKEY_CLASSES_ROOT                 \
        "Tape.Descriptor.File\DefaultIcon"                \
        ""                                                \
        "$INSTDIR\${PRODUCT_NAME}32.exe,2"

    WriteRegExpandStr   HKEY_CLASSES_ROOT                 \
        "Tape.Descriptor.File\shell\open\command"         \
        ""                                                \
        '"%SystemRoot%\system32\NOTEPAD.EXE" "%1"'

FunctionEnd

/*-------------------------------------------------------------------------*/
/*                      (close logfile)                                    */
/*-------------------------------------------------------------------------*/

Section  -CloseLogFile
    FileClose                    $hUninstLog
    SetFileAttributes  "$INSTDIR\${UninstLogFilename}"  READONLY|SYSTEM|HIDDEN
    SetDetailsView  show
SectionEnd

/***************************************************************************/
/***************************************************************************/
/**                                                                       **/
/**                     Begin uninstall logic...                          **/
/**                                                                       **/
/***************************************************************************/
/***************************************************************************/

Function un.onInit

    /* Verify the product is not currently being used... */

    Call un.IsProductInUse
    ${If} $0 <> 0
        MessageBox  MB_ICONSTOP|MB_OK "$(UninstallInUseError)"
        Abort
    ${EndIf}

FunctionEnd

Section  Uninstall

    /* Save work registers on the stack... */

    Push $R0        ; File line       (data read from the file)
    Push $R1        ; Line count      (total #of lines in file)
    Push $R2        ; Current line#   (where we're at in the file)

    /* Retrieve value of $SMGROUP variable (Start Menu group) */

    !insertmacro  MUI_STARTMENU_GETFOLDER  StartMenuPageId  $SMGROUP

    /* Verify uninstall logfile exists. We cannot proceed without it. */

    ${Unless} ${FileExists}  "$INSTDIR\${UninstLogFilename}"
        MessageBox  MB_ICONSTOP|MB_OK "$(UninstLogMissing)"
        Abort
    ${EndUnless}

    /* Remove installation directory from Windows search PATH    */
    /* Remove other directories from other environment variables */
    /* We do each many times just in case they exist many times. */

    Push $R0
    ${For} $R0 1 3

        Push  $INSTDIR
        Call  un.RemoveFromPath

    ${Next}
    Pop $R0

    /* Delete any product registry entries that were added...   */

    Call  un.RemoveProductRegistryEntries

    /*  Change the uninstall logfile attributes back to normal  */
    /*  so we can delete it when we're done with it. (It was    */
    /*  originally created "hidden" when we did the install).   */

    SetFileAttributes  "$INSTDIR\${UninstLogFilename}"  NORMAL

    /* Now read the logfile in reverse order,  */
    /* and delete all files that we installed. */

    ${un.FileReadFromEnd}  "$INSTDIR\${UninstLogFilename}"  un.ProcessLogFileInReverse

    /* Now delete the logfile */
    /* and the uninstall program itself */
    /* now that we're done with them */

    SetOutPath $TEMP
    Delete  "$INSTDIR\${UninstLogFilename}"
    Delete  "$INSTDIR\${UninstEXEName}"

    /* Now delete installation directory itself..
      (and parent directories (IF THEY'RE EMPTY))... */

    RMDir   "$INSTDIR"
    RMDir   "$INSTDIR\.."
    RMDir   "$INSTDIR\..\.."
    RMDir   "$INSTDIR\..\..\.."

    /* ... and all of our Start Menu entries... */
    /* ...(and parent directories (IF THEY'RE EMPTY))... */

    RMDir   "$SMPROGRAMS\$SMGROUP"
    RMDir   "$SMPROGRAMS\$SMGROUP\.."
    RMDir   "$SMPROGRAMS\$SMGROUP\..\.."
    RMDir   "$SMPROGRAMS\$SMGROUP\..\..\.."

    /* Finally, delete the 'Add/Remove Programs' registry entries */

    DeleteRegKey   ${PRODUCT_UNINST_ROOT_KEY}   "${PRODUCT_UNINST_KEY}"

    /* We're done! Pop work registers off the stack and exit... */

    Pop $R2
    Pop $R1
    Pop $R0

SectionEnd

/*-------------------------------------------------------------------------*/
/*           Remove Registry Entries CUSTOM UNINSTALL PAGE                 */
/*-------------------------------------------------------------------------*/

var          RemoveRegEntries

Function  un.RemoveRegEntriesPage

    strcpy  $RemoveRegEntries   "0"         ; (safe default)

    !insertmacro  MUI_HEADER_TEXT  $(DelRegEntsPageTitle)  $(DelRegEntsPageSubtitle)
    !insertmacro  MUI_INSTALLOPTIONS_EXTRACT              "${DelRegEntsPageINIFilename}"
    !insertmacro  MUI_INSTALLOPTIONS_DISPLAY              "${DelRegEntsPageINIFilename}"

    !insertmacro  INSTALLOPTIONS_READ  $RemoveRegEntries  \
        "${DelRegEntsPageINIFilename}"  "Field 2"  "State"

FunctionEnd

/*-------------------------------------------------------------------------*/
/*                   RemoveProductRegistryEntries                          */
/*-------------------------------------------------------------------------*/
/*        (Function to REMOVE any needed product registry entries)         */
/*-------------------------------------------------------------------------*/

Function  un.RemoveProductRegistryEntries

    /* Only remove them if they SPECIFICALLY requested that we do so... */

    ${If} "$RemoveRegEntries" == "1"

	    DeleteRegKey  HKEY_CLASSES_ROOT  ".aws"
	    DeleteRegKey  HKEY_CLASSES_ROOT  ".het"
	    DeleteRegKey  HKEY_CLASSES_ROOT  ".tdf"
	
	    DeleteRegKey  HKEY_CLASSES_ROOT  "AWS.Tape.File"
	    DeleteRegKey  HKEY_CLASSES_ROOT  "Hercules.Emulated.Tape.File"
	    DeleteRegKey  HKEY_CLASSES_ROOT  "Tape.Descriptor.File"
	
	    DeleteRegKey            HKEY_CURRENT_USER  "Software\${PRODUCT_COMPANY_LONG}\${PRODUCT_NAME}"
	    DeleteRegKey  /ifempty  HKEY_CURRENT_USER  "Software\${PRODUCT_COMPANY_LONG}"

    ${Endif}

FunctionEnd

/***************************************************************************/
/***************************************************************************/
/**                                                                       **/
/**                        Helper functions...                            **/
/**                                                                       **/
/***************************************************************************/
/***************************************************************************/

/*-------------------------------------------------------------------------*/
/*                          IsProductInUse                                 */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  Output:                                                                */
/*                                                                         */
/*      $0        true / false:    1 == in use,    0 == NOT in use         */
/*                                                                         */
/*-------------------------------------------------------------------------*/

!macro         IsProductInUse  un
Function  ${un}IsProductInUse

    /* Retrieve the product's installation directory... */

    ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation"

    ${If} ${Errors}             /* Product is not installed... */
        push 0                  /* (push results = NOT in use) */
        pop $0                  /* (result expected in $0 var) */
        return                  /* (return to caller) */
    ${EndIf}

    /* Switch to it... */

    push $1                     /* (save original $1) */
    GetFullPathName $1 .        /* (get current directory) */
    exch $1                     /* (restore $1, push curr dir on stack) */
    SetOutPath $0               /* (switch to install directory) */

    /* Now check whether the product binaries are in use... */

    push "${PRODUCT_NAME}32.exe"
    Call ${un}IsFileInUse
    pop  $0
    ${IfThen} $0 <> 0 ${|} goto Exit ${|}

    push "zlib1_32.dll"
    Call ${un}IsFileInUse
    pop  $0
    ${IfThen} $0 <> 0 ${|} goto Exit ${|}

    push "libbz2_32.dll"
    Call ${un}IsFileInUse
    pop  $0
    ${IfThen} $0 <> 0 ${|} goto Exit ${|}

  !ifdef  WIN64

    push "${PRODUCT_NAME}64.exe"
    Call ${un}IsFileInUse
    pop  $0
    ${IfThen} $0 <> 0 ${|} goto Exit ${|}

    push "zlib1_64.dll"
    Call ${un}IsFileInUse
    pop  $0
    ${IfThen} $0 <> 0 ${|} goto Exit ${|}

    push "libbz2_64.dll"
    Call ${un}IsFileInUse
    pop  $0
    ${IfThen} $0 <> 0 ${|} goto Exit ${|}

  !endif /* WIN64 */

  /* Restore original current directory and exit with results... */

  Exit:
    exch $0                     /* (get curr dir back, push results) */
    SetOutPath $0               /* (go back to original directory) */
    pop $0                      /* (get $0 return code back) */

FunctionEnd
!macroend

!insertmacro  IsProductInUse  ""
!insertmacro  IsProductInUse  "un."

!ifdef  WEB_INSTALLER

/*-------------------------------------------------------------------------*/
/*                         ConnectToInternet                               */
/*-------------------------------------------------------------------------*/

Function  ConnectToInternet
    /* (returns $R0 == "online" if connected, else error not connected) */
    ClearErrors
    Dialer::GetConnectedState
    Pop $R0
    ${If} "$R0" != "online"
        ${If} ${Cmd} `MessageBox MB_YESNO|MB_ICONQUESTION "$(AskConnect)" /SD IDNO IDYES`
            ClearErrors
            Dialer::AutodialUnattended
            Pop $R0
            ${If} "$R0" != "online"
                DetailPrint                    "$(ConnectError): $R0"
                MessageBox  MB_ICONSTOP|MB_OK  "$(ConnectError): $R0"
            ${EndIf}
        ${EndIf}
    ${EndIf}
FunctionEnd

/*-------------------------------------------------------------------------*/
/*                      GetChosenRadioButtonNum                            */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  Input:                                                                 */
/*                                                                         */
/*      $0      Name of dialog .INI file                                   */
/*      $1      Field number of FIRST radio button in the group            */
/*      $2      Number of radio buttons in the group                       */
/*                                                                         */
/*  Preserved (saved/restored) work variables:                             */
/*                                                                         */
/*      $3      field number of LAST radio button in the group             */
/*      $4      current field number                                       */
/*      $5      button state                                               */
/*      $6      chosen button number (temp)                                */
/*                                                                         */
/*  Requirements:                                                          */
/*                                                                         */
/*      The field numbers for all radio buttons in the group               */
/*      must be consecutively numbered.                                    */
/*                                                                         */
/*  Output:                                                                */
/*                                                                         */
/*      $0      0-based relative chosen button number  (-1 == none)        */
/*                                                                         */
/*-------------------------------------------------------------------------*/

Function  GetChosenRadioButtonNum

    ; (save work variables)

    push  $3
    push  $4
    push  $5
    push  $6

        push   -1                   ; -1 == "none"
        pop    $6                   ; init chosen button# to 'none'

        intop  $3  $1 + $2          ; calculate...
        intop  $3  $3 - 1           ; ...field# of LAST button in group

        /* Loop through buttons... */

        ${For}  $4  $1  $3          ; (from first field# to last field#)

            /* Retrieve next field's (button's) state... */

            !insertmacro  INSTALLOPTIONS_READ  $5  "$0"  "Field $4"  "State"

            /* Was this field (button) checked? */

            ${If}  $5  <>  0
                intop  $6  $4 - $1      ; (get 0-based relative button#)
                ${Break}                ; (we found our selected button)
            ${EndIf}

        ${Next}

        push  $6                ; (might still be -1 if none chosen)
        pop   $0                ; (move chosen 0-based button# to $0)

    ; (restore work variables)

    pop   $6
    pop   $5
    pop   $4
    pop   $3

FunctionEnd

!endif /* WEB_INSTALLER */

/*-------------------------------------------------------------------------*/
/*                      ProcessLogFileInReverse                            */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/*   'FileReadFromEnd' callback function...    (used during uninstall)     */
/*                                                                         */
/*    Register usage:                                                      */
/*                                                                         */
/*      $9         Current line data         ("string\r\n")                */
/*      $8         BOF-relative line number  (positive +number)            */
/*      $7         EOF-relative line number  (negative -number)            */
/*      $6-0       Not used                                                */
/*                                                                         */
/*   File is automatically opened before callback begins. Line numbers     */
/*   are 1-based relative to their origin. For a file with five lines      */
/*   in it, the first retrieved line would be +5 and -1. The next line     */
/*   would be +4 and -2, etc, all the way to +1 and -5. The returned       */
/*   string includes the terminating CR and/or LF. Remove them yourself    */
/*   if you don't need/want them. The callback ends whenever the BOF is    */
/*   reached (i.e. AFTER the first line of the file is returned). For      */
/*   each call, A VALUE *MUST* BE PUSHED ONTO THE STACK OR ELSE THE        */
/*   FUNCTION HANGS! Push "StopFileReadFromEnd" onto the stack to cause    */
/*   early termination. File is automatically closed when callback ends.   */
/*                                                                         */
/*-------------------------------------------------------------------------*/

Function  un.ProcessLogFileInReverse

    ${un.TrimNewlines}   $9   $9            ; (remove trailing CRLF..)
    /*
        PROGRAMMING NOTE: must be careful to not prematurely delete our
        $INSTDIR installation directory during uninstallation! Otherwise
        all remaining installed files (which happened to be installed
        there) end up never getting deleted! (because the directory they
        were in has just been deleted!). Our main 'Uninstall' section
        manually deletes the $INSTDIR directory itself before exiting,
        so we must be sure to NOT do so here!
    */
    ${If} "$INSTDIR" != "$9"                ; (if NOT $INSTDIR..)

        /* Delete the file or directory that we previously installed... */

        ${un.GetFileAttributes}  "$9"   "DIRECTORY"   $0

        ${If}  "$0"  ==  "1"                ; (if directory)
            RMDir                "$9"       ; (delete directory)
        ${Else}                             ; (else)
            Delete  /REBOOTOK    "$9"       ; (delete file)
        ${EndIf}

        ClearErrors

    ${EndIf}

    /* (see documentation; the following is unfortunately required..) */
    push    0

FunctionEnd

!ifdef  WEB_INSTALLER

/*-------------------------------------------------------------------------*/
/*                         SetSectionSizes                                 */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/*   The following function is needed for web installs so that NSIS can    */
/*   provide an approximate "size" feedback to the user for each section   */
/*   the user choses to install. Normally NSIS gets this information for   */
/*   itself whenever it builds the installer (based on the "Files" and     */
/*   "CopyFiles" statements), but for web installs, because the files      */
/*   are downloaded and installed at runtime, it thus cannot determine     */
/*   the values at compile time so you must tell it. That's what this      */
/*   function does: it tells NSIS the approximate size of each section     */
/*   so it can tell the user how much disk space their given selection     */
/*   is going to consume. Our values are only approximate for now since    */
/*   given the cost of disk drives these days ($$$ per GB), I don't see    */
/*   the need to be all that accurate. <shrug>  Your mileage may vary.     */
/*                                                                         */
/*-------------------------------------------------------------------------*/

  !ifndef  WIN64

    !define  Size_MainBinaries      "3100"          ; Win32 size
    !define  Size_SourceCode        "440"           ; Win32 size

  !else

    !define  Size_MainBinaries      "5600"          ; Win64 size
    !define  Size_SourceCode        "440"           ; Win64 size

  !endif

Function  SetSectionSizes

    SectionSetSize  ${SEC_MainBinaries}     ${Size_MainBinaries}
    SectionSetSize  ${SEC_SourceCode}       ${Size_SourceCode}

FunctionEnd

!endif /* WEB_INSTALLER */

!ifndef  MIN_INSTALLER

/*-------------------------------------------------------------------------*/
/*                         NormalizePath                                   */
/*-------------------------------------------------------------------------*/

Function  NormalizePath

    Exch  $0
    push  $1

    ClearErrors

    ${Do}
        ${WordReplace}  "$0"   "/"   "\"   "E+"   $1
        ${If} ${Errors}
            ${Break}
        ${Else}
            strcpy  $0  $1
        ${EndIf}
    ${Loop}

    ClearErrors

    ${Do}
        ${WordReplace}  "$0"   "\\"   "\"   "E+"   $1
        ${If} ${Errors}
            ${Break}
        ${Else}
            strcpy  $0  $1
        ${EndIf}
    ${Loop}

    ClearErrors

    pop  $1
    Exch $0

FunctionEnd

/*-------------------------------------------------------------------------*/
/*                            ParentDir                                    */
/*-------------------------------------------------------------------------*/

Function  ParentDir

    Exch  $0

    Push  $0
    Call  NormalizePath
    Pop   $0

    ${WordFind}  "$0"  "\"  "-2{*"  $0

    Exch  $0

FunctionEnd

/*-------------------------------------------------------------------------*/
/*                Install VC++ Runtime if needed                           */
/*-------------------------------------------------------------------------*/

Function  Install_VCRedist_IfNeeded

    ${Install_SP1_X86_VC80Redistributables_IfNeeded}

  !ifdef WIN64
    ${Install_SP1_X64_VC80Redistributables_IfNeeded}
  !endif

FunctionEnd

/*-------------------------------------------------------------------------*/
/*                   Install FishLib if needed                             */
/*-------------------------------------------------------------------------*/

Function  Install_FishLib_IfNeeded

    DetailPrint  "$(InitFishLibInstall)"

    /* Determine proper INSTDIR value to use for FishLib... */

    var  /global  FISHLIB_INSTDIR

    ${If} ${FishLibIsInstalled}
  
        ReadRegStr $FISHLIB_INSTDIR HKLM "${FISHLIB_UNINST_KEY}" "InstallLocation"

    ${Else}

        push  $INSTDIR      ; (e.g. "%ProgramFiles%\SoftDevLabs\CTCI-W32")
        Call  ParentDir     ; (e.g. ".." (backup to parent directory))
        pop   $0            ; (e.g. "%ProgramFiles%\SoftDevLabs")
        strcpy $FISHLIB_INSTDIR "$0\${FISHLIB_PRODUCT_NAME}"

    ${EndIf}

    /* Now do the (re-)install... */

  !ifdef  WEB_INSTALLER

    DetailPrint  "$(DownloadingFishLib)"

    SetOutPath  "$PLUGINSDIR"
    inetc::get    /noproxy                                                                              \
        "$WebInstall_URLBase/${FISHLIB_PRODUCT_NAME}/${FISHLIB_VERSION_SHORT}/${FISHLIB_SETUP_EXENAME}"  \
                                                                             "${FISHLIB_SETUP_EXENAME}"  /end
    Pop $R0

    ${If} "$R0" != "OK"
        DetailPrint                    "$(FishLibDownloadFailed) : $R0"
        MessageBox  MB_ICONSTOP|MB_OK  "$(FishLibDownloadFailed) : $R0"
        SetErrors
    ${Else}
        DetailPrint  "$(LaunchingFishLib)"
        ClearErrors
        /* (NOTE: NSIS requires that '/D=' option MUST be the LAST option on the command line!) */
        ExecWait  '"$PLUGINSDIR\${FISHLIB_SETUP_EXENAME}"  /S  /MIRROR=$WebInstall_URLBase  /D=$FISHLIB_INSTDIR'
    ${Endif}

  !else  /* (STD/DEV installer) */

    File  "/oname=$PLUGINSDIR\${FISHLIB_SETUP_EXENAME}"  \
        "${FISHLIB_SETUP_DIR}\${FISHLIB_SETUP_EXENAME}"

    DetailPrint  "$(LaunchingFishLib)"
    ClearErrors
    /* (NOTE: NSIS requires that '/D=' option MUST be the LAST option on the command line!) */
    ExecWait  '"$PLUGINSDIR\${FISHLIB_SETUP_EXENAME}"  /S  /D=$FISHLIB_INSTDIR'

  !endif /* WEB_INSTALLER */

    ${If} ${Errors}
        ClearErrors
        DetailPrint                            "$(AutoFishLibInstallFailed)"
        MessageBox   MB_ICONEXCLAMATION|MB_OK  "$(AutoFishLibFailedMsgBox)"
    ${Else}
        DetailPrint  "$(FishLibInstallSuccess)"
    ${EndIf}

    Delete "$PLUGINSDIR\${FISHLIB_SETUP_EXENAME}"

FunctionEnd

!endif /* !MIN_INSTALLER */
