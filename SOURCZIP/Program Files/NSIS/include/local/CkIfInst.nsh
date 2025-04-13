/***********************************************************************************/
/*                             CkIfInst.nsh                                        */
/***********************************************************************************/
/*                                                                                 */
/*  Defines macros to make it easier to check if a given product is installed.     */
/*                                                                                 */
/*  (C) Copyright 2007-2008 Software Development Laboratories                      */
/*                                                                                 */
/*  Usage:                                                                         */
/*                                                                                 */
/*    !AddIncludeDir  "${NSISDIR}\include\local"                                   */
/*                                                                                 */
/*    !include "CkIfInst.nsh"          ; Check if product-id is installed          */
/*                                                                                 */
/*    ${Unless} ${MyProductDependencyIsInstalled}                                  */
/*       ${InstallMyProductDependency}                                             */
/*    ${EndUnless}                                                                 */
/*                                                                                 */
/*  NOTE: For another possible usage example, refer to the "VCRedist.nsh" header   */
/*  where the ${Install_SP1_X86_VC80Redistributables_IfNeeded} macro is defined.   */
/*                                                                                 */
/***********************************************************************************/

!verbose push
!verbose 3

!ifndef  _CkIfInst_NSH_
!define  _CkIfInst_NSH_

!include "LogicLib.nsh"                 ; Need some of LogicLib's logic
!include "FileFunc.nsh"                 ; need "${GetFileName}", etc

;------------------------------------------------------------------------------
; Publicly known product IDs...

!define  SP1_X86_VC80Redist_PRODID  "{7299052b-02a4-4627-81f2-1818da5d550d}"  ;(from Microsoft)
!define  SP1_X64_VC80Redist_PRODID  "{071c9b48-7c32-4621-a0ac-3f809523288f}"  ;(from Microsoft)
!define  SP1_IA64_VC80Redist_PRODID "{0f8fb34e-675e-42ed-850b-29d98c2ece08}"  ;(from Microsoft)

!define  X86_VC90Redist_PRODID      "{FF66E9F6-83E7-3A3E-AF14-8DE9A809A6A4}"  ;(determined empirically)
!define  X64_VC90Redist_PRODID      "{350AA351-21FA-3270-8B7A-835434E766AD}"  ;(determined empirically)
!define  IA64_VC90Redist_PRODID     "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}"  ;(unknown as yet)

;------------------------------------------------------------------------------
; Private product IDs...

!define  WINPCAP_PRODID             "WinPcapInst"
!define  FISHLIB_PRODID             "FishLib"
!define  AWSBROWSE_PRODID           "AWSBrowse"
!define  FTAPE_PRODID               "ftape"
!define  CTCIW32_PRODID             "CTCI-W32"
!define  HERCGUI_PRODID             "HercGUI"

;------------------------------------------------------------------------------
; The following macro checks to see if a specific "Uninstall" key value exists
; or not (specifically, the "UninstallString" value). If it does, then chances
; are good that the product in question has indeed been installed...

!macro  _IsProductInstalled  PRODID
    !insertmacro _LOGICLIB_TEMP
    ClearErrors
    StrCpy $_LOGICLIB_TEMP "0"          ; (default to "not installed")
    Push $R0
    ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODID}" "UninstallString"
    Pop $R0
    IfErrors +2 0
    StrCpy $_LOGICLIB_TEMP "1"          ; (indicate it HAS been installed)
    ClearErrors
!macroend

;------------------------------------------------------------------------------
; LogicLib extension macros to check if a specific product is installed...

!macro  _SP1_X86_VC80RedistributablesAreInstalled  _a  _b  _t  _f
    !insertmacro  _IsProductInstalled  "${SP1_X86_VC80Redist_PRODID}"
    !insertmacro  _==  $_LOGICLIB_TEMP 1  `${_t}`  `${_f}`
!macroend

!macro  _SP1_X64_VC80RedistributablesAreInstalled  _a  _b  _t  _f
    !insertmacro  _IsProductInstalled  "${SP1_X64_VC80Redist_PRODID}"
    !insertmacro  _==  $_LOGICLIB_TEMP 1  `${_t}`  `${_f}`
!macroend

!macro  _SP1_IA64_VC80RedistributablesAreInstalled  _a  _b  _t  _f
    !insertmacro  _IsProductInstalled  "${SP1_IA64_VC80Redist_PRODID}"
    !insertmacro  _==  $_LOGICLIB_TEMP 1  `${_t}`  `${_f}`
!macroend

!macro  _X86_VC90RedistributablesAreInstalled  _a  _b  _t  _f
    !insertmacro  _IsProductInstalled  "${X86_VC90Redist_PRODID}"
    !insertmacro  _==  $_LOGICLIB_TEMP 1  `${_t}`  `${_f}`
!macroend

!macro  _X64_VC90RedistributablesAreInstalled  _a  _b  _t  _f
    !insertmacro  _IsProductInstalled  "${X64_VC90Redist_PRODID}"
    !insertmacro  _==  $_LOGICLIB_TEMP 1  `${_t}`  `${_f}`
!macroend

!macro  _IA64_VC90RedistributablesAreInstalled  _a  _b  _t  _f
    !insertmacro  _IsProductInstalled  "${IA64_VC90Redist_PRODID}"
    !insertmacro  _==  $_LOGICLIB_TEMP 1  `${_t}`  `${_f}`
!macroend

;------------------------------------------------------------------------------
; (same thing but for private product ids...)

!macro  _WinPCapIsInstalled  _a  _b  _t  _f
    !insertmacro  _IsProductInstalled  "${WINPCAP_PRODID}"
    !insertmacro  _==  $_LOGICLIB_TEMP 1  `${_t}`  `${_f}`
!macroend

!macro  _FishLibIsInstalled  _a  _b  _t  _f
    !insertmacro  _IsProductInstalled  "${FISHLIB_PRODID}"
    !insertmacro  _==  $_LOGICLIB_TEMP 1  `${_t}`  `${_f}`
!macroend

!macro  _AWSBrowseIsInstalled  _a  _b  _t  _f
    !insertmacro  _IsProductInstalled  "${AWSBROWSE_PRODID}"
    !insertmacro  _==  $_LOGICLIB_TEMP 1  `${_t}`  `${_f}`
!macroend

!macro  _FTapeIsInstalled  _a  _b  _t  _f
    !insertmacro  _IsProductInstalled  "${FTAPE_PRODID}"
    !insertmacro  _==  $_LOGICLIB_TEMP 1  `${_t}`  `${_f}`
!macroend

!macro  _CTCIW32IsInstalled  _a  _b  _t  _f
    !insertmacro  _IsProductInstalled  "${CTCIW32_PRODID}"
    !insertmacro  _==  $_LOGICLIB_TEMP 1  `${_t}`  `${_f}`
!macroend

!macro  _HercGUIIsInstalled  _a  _b  _t  _f
    !insertmacro  _IsProductInstalled  "${HERCGUI_PRODID}"
    !insertmacro  _==  $_LOGICLIB_TEMP 1  `${_t}`  `${_f}`
!macroend

;------------------------------------------------------------------------------
; LogicLib extension comparison condition boolean variables...

!define  SP1_X86_VC80RedistributablesAreInstalled    ` ""  SP1_X86_VC80RedistributablesAreInstalled  "" `
!define  SP1_X64_VC80RedistributablesAreInstalled    ` ""  SP1_X64_VC80RedistributablesAreInstalled  "" `
!define  SP1_IA64_VC80RedistributablesAreInstalled   ` ""  SP1_IA64_VC80RedistributablesAreInstalled "" `

!define  X86_VC90RedistributablesAreInstalled        ` ""  X86_VC90RedistributablesAreInstalled  "" `
!define  X64_VC90RedistributablesAreInstalled        ` ""  X64_VC90RedistributablesAreInstalled  "" `
!define  IA64_VC90RedistributablesAreInstalled       ` ""  IA64_VC90RedistributablesAreInstalled "" `

;------------------------------------------------------------------------------
; (same thing but for private product ids...)

!define  WinPCapIsInstalled                         ` ""  WinPCapIsInstalled    "" `
!define  FishLibIsInstalled                         ` ""  FishLibIsInstalled    "" `
!define  AWSBrowseIsInstalled                       ` ""  AWSBrowseIsInstalled  "" `
!define  FTapeIsInstalled                           ` ""  FTapeIsInstalled      "" `
!define  CTCIW32IsInstalled                         ` ""  CTCIW32IsInstalled    "" `
!define  HercGUIIsInstalled                         ` ""  HercGUIIsInstalled    "" `

/*-------------------------------------------------------------------------*/
/*                             IsFileInUse                                 */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  Input:                                                                 */
/*                                                                         */
/*      push    name/path of file to be checked                            */
/*                                                                         */
/*  Requirements:                                                          */
/*                                                                         */
/*      If pushed filename is not a fullpath filename, then current        */
/*      directory must obviously be set to where the file exists.          */
/*                                                                         */
/*  Output:                                                                */
/*                                                                         */
/*      pop     1 or 0 (true/false):  1 == in use,  0 == not in use        */
/*                                                                         */
/*-------------------------------------------------------------------------*/

!insertmacro     GetFileName        /* (helper macro that we need/use) */
!insertmacro  un.GetFileName        /* (helper macro that we need/use) */

!macro        IsFileInUse  un
Function ${un}IsFileInUse

    exch  $0        /* (save $0 and retrieve original passed stack value) */
    push  $1        /* (save $1; use $1 = just filename)  */

    GetFullPathName      $0  $0     /* (FULLPATH REQUIRED for 'Delete'!) */
    ${${un}GetFileName}  $0  $1     /* (retrieve just the filename only) */

    Delete        "$0.IsFileInUse.tmp"
    Delete  "$TEMP\$1.IsFileInUse.tmp"

    ClearErrors

    ${If} ${FileExists} "$0"
        CopyFiles "$0" "$TEMP\$1.IsFileInUse.tmp"
        ${Unless} ${Errors}
            Rename "$0" "$0.IsFileInUse.tmp"
            ${Unless} ${Errors}
                Delete "$0.IsFileInUse.tmp"
                ${Unless} ${Errors}
                    /* Delete succeeded; restore */
                    push  0
                    CopyFiles "$TEMP\$1.IsFileInUse.tmp" "$0"
                ${Else}
                    /* Delete failed; restore original  */
                    /* and delete temporary backup copy */
                    push  1
                    Rename "$0.IsFileInUse.tmp" "$0"
                    Delete "$TEMP\$1.IsFileInUse.tmp"
                ${EndUnless}
            ${Else}
                /* Rename failed; delete backup */
                push  1
                Delete "$TEMP\$1.IsFileInUse.tmp"
            ${EndUnless}
        ${Else}
            /* CopyFiles failed */
            push  1
        ${EndUnless}
    ${Else}
        /* File doesn't even exist */
        push  0
    ${EndIf}

    pop  $0         /* (save results in $0) */
    pop  $1         /* (restore original $1) */
    exch $0         /* (restore original $0 and push results) */

FunctionEnd
!macroend

!insertmacro  IsFileInUse  ""
!insertmacro  IsFileInUse  "un."

;------------------------------------------------------------------------------

!endif  #  _CkIfInst_NSH_

!verbose pop

;------------------------------------------------------------------------------
