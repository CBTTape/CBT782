/***************************************************************************/
/*      Product.nsh    --    defines the product strings                   */
/***************************************************************************/
/*                                                                         */
/*  NOTE: some values here (PRODUCT_VERSION_SHORT and PRODUCT_BUILDNUM)    */
/*  are defined via environment variables and thus are the responsibility  */
/*  of the calling batch file to set them appropriately before invoking    */
/*  the NSIS compiler to build the setup program.                          */
/*                                                                         */
/***************************************************************************/
/*                                                                         */
/*  Change History:                                                        */
/*                                                                         */
/*  MM/DD/YY   XXXXXXXXXXXXXXXX.........                                   */
/*                                                                         */
/***************************************************************************/

!ifndef _PRODUCT_NSH_
!define _PRODUCT_NSH_

!AddIncludeDir  "${NSISDIR}\include\local"  ; (so we can find Debug64.nsh)
!include        "Debug64.nsh"               ; (we need 'Platform' defined)
!include        "CkIfInst.nsh"              ; (need PRODUCT ID constants)

/*-------------------------------------------------------------------------*/
/*                         Installer types...                              */
/*-------------------------------------------------------------------------*/

/*  NOTE: The below constants MUST MATCH the ones set in 'makesetup.cmd'   */

!define _STD_INSTALLER  1  /* NORMAL installer WITHOUT any developer files */
!define _DEV_INSTALLER  2  /* With FishLib DEVELOPMENT FILES also embedded */
!define _MIN_INSTALLER  3  /* ONLY FishLib, to embed into other installers */
!define _WEB_INSTALLER  4  /* NO FILES embedded at all, downloaded instead */

!if ${_INSTALLER_TYPE} == ${_STD_INSTALLER}
  !define                    STD_INSTALLER      ; Standard installer
!endif
!if ${_INSTALLER_TYPE} == ${_DEV_INSTALLER}
  !define                    DEV_INSTALLER      ; Developer installer
!endif
!if ${_INSTALLER_TYPE} == ${_MIN_INSTALLER}
  !define                    MIN_INSTALLER      ; Minimal installer
!endif
!if ${_INSTALLER_TYPE} == ${_WEB_INSTALLER}
  !define                    WEB_INSTALLER      ; Web-based installer
!endif

/*-------------------------------------------------------------------------*/
/*       (For testing various combinations of Installer types...)         */
/*-------------------------------------------------------------------------*/

!ifdef    DEV_INSTALLER
  !define DEV_or_WEB_INSTALLER
!else ifdef      WEB_INSTALLER    ;(for testing if DEV -OR- WEB installer)
  !define DEV_or_WEB_INSTALLER
!endif

!ifdef    MIN_INSTALLER
  !define MIN_or_STD_INSTALLER
!else ifdef      STD_INSTALLER    ;(for testing if MIN -OR- STD installer)
  !define MIN_or_STD_INSTALLER
!endif

/*-------------------------------------------------------------------------*/
/*                        Web-install support...                           */
/*-------------------------------------------------------------------------*/

!ifdef WEB_INSTALLER

  !define  NORTH_AMERICAN_URLBASE     "http://www.cbttape.org/~fish/download"
  !define  EUROPEAN_URLBASE           "http://www.softdevlabs.com/download"
  !define  DEFAULT_URLBASE            "${NORTH_AMERICAN_URLBASE}"

  !define  VCRedistVersion  "8.0"   /* (used in URL path for web installs) */

!endif /* WEB_INSTALLER */

/*-------------------------------------------------------------------------*/
/*                         Product information                             */
/*-------------------------------------------------------------------------*/

!define  PRODUCT_NAME                 "AWSBrowse"
!define  PRODUCT_DESCRIPTION          "Fish's AWS/HET File Browser Utility"
!define  PRODUCT_COPYRIGHT_YEARS      "2004-2008"
!define  PRODUCT_COMPANY_SHORT        "SoftDevLabs"
!define  PRODUCT_COMPANY_LONG         "Software Development Laboratories"
!define  PRODUCT_URL                  "www.softdevlabs.com/awsbrowse"
!define  PRODUCT_EMAIL                "fish@softdevlabs.com"

/*         "$%XXXXXX%" are environ vars passed by 'makesetup.cmd'          */

!define  PRODUCT_VERSION_SHORT        "$%PRODUCT_VERSION_SHORT%"
!define  PRODUCT_BUILDNUM             "$%PRODUCT_BUILDNUM%"
!define  PRODUCT_DIST_INDIR           "$%PRODUCT_DIST_INDIR%"
!define  PRODUCT_SETUP_OUTDIR         "$%PRODUCT_SETUP_OUTDIR%"

!define  PRODUCT_SETUP_EXENAME        "${_OUTFILE}" ;(passed via command-line)
!define  PRODUCT_VERSION_LONG         "${PRODUCT_VERSION_SHORT}.${PRODUCT_BUILDNUM}"
!define  PRODUCT_COPYRIGHT            "Copyright (C) ${PRODUCT_COPYRIGHT_YEARS}"
!define  PRODUCT_DEFAULT_INSTALLDIR   "$PROGRAMFILES\${PRODUCT_COMPANY_SHORT}\${PRODUCT_NAME}"
!ifdef WEB_INSTALLER
!define  PRODUCT_SOURCE_ZIPNAME       "${PRODUCT_NAME}_${PRODUCT_VERSION_SHORT}_src.zip"
!else
!define  PRODUCT_SOURCE_ZIPNAME       "${PRODUCT_NAME}_${PRODUCT_VERSION_LONG}_src.zip"
!endif
!define  PRODUCT_STARTMENU_REGVAL     "StartMenuDir"
!define  PRODUCT_DEFAULT_SMGROUP      "${PRODUCT_COMPANY_SHORT}\${PRODUCT_NAME}"
!define  PRODUCT_UNINST_ROOT_KEY      "HKLM"
!define  PRODUCT_UNINST_BASE_KEY      "Software\Microsoft\Windows\CurrentVersion\Uninstall"
!define  PRODUCT_UNINST_KEY           "${PRODUCT_UNINST_BASE_KEY}\${AWSBROWSE_PRODID}"

/*-------------------------------------------------------------------------*/
/*                      Installer version information                      */
/*-------------------------------------------------------------------------*/

!ifdef WEB_INSTALLER
VIAddVersionKey   "FileDescription"    "${PRODUCT_DESCRIPTION} WEB Installer"
VIAddVersionKey   "ProductName"        "${PRODUCT_NAME} ${PRODUCT_VERSION_SHORT} WEB installer"
VIAddVersionKey   "SpecialBuild"       "${Platform} Web Installer"
!else ifdef MIN_INSTALLER
VIAddVersionKey   "FileDescription"    "${PRODUCT_DESCRIPTION} MIN Installer"
VIAddVersionKey   "ProductName"        "${PRODUCT_NAME} ${PRODUCT_VERSION_SHORT} MIN installer"
VIAddVersionKey   "SpecialBuild"       "${Platform} Minimal Installer"
!else ifdef STD_INSTALLER
VIAddVersionKey   "FileDescription"    "${PRODUCT_DESCRIPTION} STD Installer"
VIAddVersionKey   "ProductName"        "${PRODUCT_NAME} ${PRODUCT_VERSION_SHORT} STD installer"
VIAddVersionKey   "SpecialBuild"       "${Platform} Standard Installer"
!else ifdef DEV_INSTALLER
VIAddVersionKey   "FileDescription"    "${PRODUCT_DESCRIPTION} DEV Installer"
VIAddVersionKey   "ProductName"        "${PRODUCT_NAME} ${PRODUCT_VERSION_SHORT} DEV installer"
VIAddVersionKey   "SpecialBuild"       "${Platform} Developers Installer"
!else
  !error "Invalid _INSTALLER_TYPE"
!endif
VIAddVersionKey   "FileVersion"        "${PRODUCT_VERSION_LONG}"
VIAddVersionKey   "ProductVersion"     "${PRODUCT_VERSION_LONG}"
VIAddVersionKey   "OriginalFilename"   "${PRODUCT_SETUP_EXENAME}"
VIAddVersionKey   "CompanyName"        "${PRODUCT_COMPANY_LONG}"
VIAddVersionKey   "LegalCopyright"     "${PRODUCT_COPYRIGHT}, ${PRODUCT_COMPANY_SHORT}"
VIAddVersionKey   "Product URL"        "http://${PRODUCT_URL}"
VIAddVersionKey   "Product Email"      "${PRODUCT_EMAIL}"
VIProductVersion                       "${PRODUCT_VERSION_LONG}"

BrandingText  "Copyright (C) ${PRODUCT_COPYRIGHT_YEARS} ${PRODUCT_COMPANY_LONG}"

/*-------------------------------------------------------------------------*/
/*                    Product Dependency Information                       */
/*-------------------------------------------------------------------------*/

!define  FISHLIB_PRODUCT_NAME          "FishLib"
!define  FISHLIB_VERSION_SHORT         "2.8.0"
!define  FISHLIB_SETUP_DIR             "L:\MyProjects\_SDL Open Source\FishLibDLL\Setup\All"
!define  FISHLIB_SETUP_EXENAME         "${FISHLIB_PRODUCT_NAME}_${FISHLIB_VERSION_SHORT}_${Platform}_min_setup.exe"
!define  FISHLIB_UNINST_KEY            "${PRODUCT_UNINST_BASE_KEY}\${FISHLIB_PRODID}"

/*-------------------------------------------------------------------------*/

!endif # _PRODUCT_NSH_
