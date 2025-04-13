/***********************************************************************************/
/*                             VCRedist.nsh                                        */
/***********************************************************************************/
/*                                                                                 */
/*  Defines macros to make it easier to have your installation program also        */
/*  install the VC++ redistributables if your product needs them.                  */
/*                                                                                 */
/*  (C) Copyright 2007-2008 Software Development Laboratories                      */
/*                                                                                 */
/*  Usage:                                                                         */
/*                                                                                 */
/*    !AddIncludeDir  "${NSISDIR}\include\local"                                   */
/*                                                                                 */
/*    !define   WEB_INSTALLER               ;(indicate web install desired)        */
/*                                                                                 */
/*    ; !ifndef   WEB_INSTALLER                                                    */
/*    ; !define   VC80Redist_PACKAGES_DIR   "C:\some\non-standard\location"        */
/*    ; !endif   ;WEB_INSTALLER                                                    */
/*                                                                                 */
/*    !include "VCRedist.nsh"               ;(Install VC Redistributables logic)   */
/*                                                                                 */
/*    !ifdef    WEB_INSTALLER                                                      */
/*    call      ConnectToInternet           ;(need connection to download!)        */
/*    strcpy   $VCRedistURL                 "http://www.somewhere.com/somedir"     */
/*    !endif   ;WEB_INSTALLER                                                      */
/*                                                                                 */
/*    ${Install_SP1_X86_VC80Redistributables_IfNeeded}      ;(8.0 32-bit all)      */
/*    ; ${Install_X86_VC90Redistributables_IfNeeded}        ;(9.0 32-bit all)      */
/*    ; ${Install_SP1_X64_VC80Redistributables_IfNeeded}    ;(8.0 64-bit x64)      */
/*    ; ${Install_SP1_IA64_VC80Redistributables_IfNeeded}   ;(8.0 64-bit Itanium)  */
/*                                                                                 */
/*                                                                                 */
/*  NOTE: For normal installs, if your "vcredist_x86.exe" (or "vcredist_x64.exe")  */
/*  package executables are not in the standard/default location then you need to  */
/*  !define the VC80Redist_PACKAGES_DIR (or VC90Redist_PACKAGES_DIR) constant as   */
/*  appropriate before !including this header. For web installs, you need to copy  */
/*  the location of where "vcredist_x86.exe" (or "vcredist_x64.exe") executable    */
/*  is on your web site to the '$VCRedistURL' variable before invoking the macro.  */
/*                                                                                 */
/***********************************************************************************/

!verbose push
!verbose 3

!ifndef  _VCRedist_NSH_
!define  _VCRedist_NSH_

!include "CkIfInst.nsh"                 ; Need "IsProductInstalled" macro...

;------------------------------------------------------------------------------
; Local VStudio directory where the VC redistributables packages live...

!ifndef    VC80Redist_PACKAGES_DIR
  !define  VC80Redist_PACKAGES_DIR  "$%ProgramFiles%\Microsoft Visual Studio 8\SDK\v2.0\Bootstrapper\Packages"
!endif
!ifndef    VC90Redist_PACKAGES_DIR
  !define  VC90Redist_PACKAGES_DIR  "$%ProgramFiles%\Microsoft SDKs\Windows\v6.0A\Bootstrapper\Packages"
!endif

!verbose push
!verbose 4
!echo "VC80Redist_PACKAGES_DIR = ${VC80Redist_PACKAGES_DIR}"
!echo "VC90Redist_PACKAGES_DIR = ${VC90Redist_PACKAGES_DIR}"
!verbose pop

;------------------------------------------------------------------------------
; The following macro, when inserted, will install the redistributables

!ifndef  WEB_INSTALLER

  /* Normal installer */

  !macro   Install_VC80Redistributables       Arch       ; ("x86", "x64" or "ia64")
    InitPluginsDir
    File       "/oname=$PLUGINSDIR\vcredist_${Arch}.exe"  "${VC80Redist_PACKAGES_DIR}\vcredist_${Arch}\vcredist_${Arch}.exe"
    ExecWait         '"$PLUGINSDIR\vcredist_${Arch}.exe"  /Q'
  !macroend

  !macro   Install_VC90Redistributables       Arch       ; ("x86", "x64" or "ia64")
    InitPluginsDir
    File       "/oname=$PLUGINSDIR\vcredist_${Arch}.exe"  "${VC90Redist_PACKAGES_DIR}\vcredist_${Arch}\vcredist_${Arch}.exe"
    ExecWait         '"$PLUGINSDIR\vcredist_${Arch}.exe"  /Q'
  !macroend

!else

  /* Web installer */

  var   VCRedistURL
  var   VCRedistname

  !macro    Install_VC80Redistributables      Arch       ; ("x86", "x64" or "ia64")
    strcpy         $VCRedistname  "vcredist_${Arch}.exe"
    InitPluginsDir
    inetc::get  /noproxy  "$VCRedistURL/$VCRedistname"  "$PLUGINSDIR\$VCRedistname"  /end
    Pop $R0
    ${If} "$R0" != "OK"
        SetDetailsView  show
        DetailPrint                    "Download '$VCRedistURL/$VCRedistname' failed: $R0"
        MessageBox  MB_ICONSTOP|MB_OK  "Download '$VCRedistURL/$VCRedistname' failed: $R0"
        Abort
    ${Endif}
    ExecWait        '"$PLUGINSDIR\$VCRedistname"  /Q'
  !macroend

  !macro    Install_VC90Redistributables      Arch       ; ("x86", "x64" or "ia64")
    strcpy         $VCRedistname  "vcredist_${Arch}.exe"
    InitPluginsDir
    inetc::get  /noproxy  "$VCRedistURL/$VCRedistname"  "$PLUGINSDIR\$VCRedistname"  /end
    Pop $R0
    ${If} "$R0" != "OK"
        SetDetailsView  show
        DetailPrint                    "Download '$VCRedistURL/$VCRedistname' failed: $R0"
        MessageBox  MB_ICONSTOP|MB_OK  "Download '$VCRedistURL/$VCRedistname' failed: $R0"
        Abort
    ${Endif}
    ExecWait        '"$PLUGINSDIR\$VCRedistname"  /Q'
  !macroend

!endif /* WEB_INSTALLER */

;------------------------------------------------------------------------------
; Now define the !defines that !insert and invoke the above macro with the
; appropriate parameter as part of their name (since it's easier that way).

!define  Install_SP1_X86_VC80Redistributables   '!insertmacro Install_VC80Redistributables  "x86"'
!define  Install_SP1_X64_VC80Redistributables   '!insertmacro Install_VC80Redistributables  "x64"'
!define  Install_SP1_IA64_VC80Redistributables  '!insertmacro Install_VC80Redistributables  "ia64"'

!define  Install_X86_VC90Redistributables       '!insertmacro Install_VC90Redistributables  "x86"'
!define  Install_X64_VC90Redistributables       '!insertmacro Install_VC90Redistributables  "x64"'
!define  Install_IA64_VC90Redistributables      '!insertmacro Install_VC90Redistributables  "ia64"'

;------------------------------------------------------------------------------
; Now define macros to do everything needed in one simple invokation...

!macro   Install_SP1_X86_VC80Redistributables_IfNeeded
    ${Unless} ${SP1_X86_VC80RedistributablesAreInstalled}
        DetailPrint "Installing x86 (32-bit) VC++ 8.0 SP1 redistributables..."
        ${Install_SP1_X86_VC80Redistributables}
    ${EndUnless}
!macroend

!macro   Install_SP1_X64_VC80Redistributables_IfNeeded
    ${Unless} ${SP1_X64_VC80RedistributablesAreInstalled}
        DetailPrint "Installing x64 (64-bit) VC++ 8.0 SP1 redistributables..."
        ${Install_SP1_X64_VC80Redistributables}
    ${EndUnless}
!macroend

!macro   Install_SP1_IA64_VC80Redistributables_IfNeeded
    ${Unless} ${SP1_IA64_VC80RedistributablesAreInstalled}
        DetailPrint "Installing ia64 (Itanium 64-bit) VC++ 8.0 SP1 redistributables..."
        ${Install_SP1_IA64_VC80Redistributables}
    ${EndUnless}
!macroend

!macro   Install_X86_VC90Redistributables_IfNeeded
    ${Unless} ${X86_VC90RedistributablesAreInstalled}
        DetailPrint "Installing x86 (32-bit) VC++ 9.0 redistributables..."
        ${Install_X86_VC90Redistributables}
    ${EndUnless}
!macroend

!macro   Install_X64_VC90Redistributables_IfNeeded
    ${Unless} ${X64_VC90RedistributablesAreInstalled}
        DetailPrint "Installing x64 (64-bit) VC++ 9.0 redistributables..."
        ${Install_X64_VC90Redistributables}
    ${EndUnless}
!macroend

!macro   Install_IA64_VC90Redistributables_IfNeeded
    ${Unless} ${IA64_VC90RedistributablesAreInstalled}
        DetailPrint "Installing ia64 (Itanium 64-bit) VC++ 9.0 redistributables..."
        ${Install_IA64_VC90Redistributables}
    ${EndUnless}
!macroend

;------------------------------------------------------------------------------
; Finally, define the !defines that !insert and invoke the above macros...

!define  Install_SP1_X86_VC80Redistributables_IfNeeded    "!insertmacro Install_SP1_X86_VC80Redistributables_IfNeeded"
!define  Install_SP1_X64_VC80Redistributables_IfNeeded    "!insertmacro Install_SP1_X64_VC80Redistributables_IfNeeded"
!define  Install_SP1_IA64_VC80Redistributables_IfNeeded   "!insertmacro Install_SP1_IA64_VC80Redistributables_IfNeeded"

!define  Install_X86_VC90Redistributables_IfNeeded        "!insertmacro Install_X86_VC90Redistributables_IfNeeded"
!define  Install_X64_VC90Redistributables_IfNeeded        "!insertmacro Install_X64_VC90Redistributables_IfNeeded"
!define  Install_IA64_VC90Redistributables_IfNeeded       "!insertmacro Install_IA64_VC90Redistributables_IfNeeded"

;------------------------------------------------------------------------------

!endif  #  _VCRedist_NSH_

!verbose pop

;------------------------------------------------------------------------------
