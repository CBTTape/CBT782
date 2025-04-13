// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are otherwise changed relatively infrequently...
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  06/15/06    1.5.0   Fish    VS2005, x64
//  08/11/06    1.5.0   Fish    Fix/comment SDK header #defines: _WIN32_WINNT, _WIN32_IE.
//  02/27/07    1.5.1   Fish    Fix for VS2005 SP1: #define _USE_RTM_VERSION.
//  08/12/07    1.5.2   Fish    Remove #define _USE_RTM_VERSION. Microsoft now
//                              thankfully distributes the SP1 version of vcredist.
//  11/25/07    1.5.2   Fish    Use different 32/64-bit zlib1/libbz2 DLL names.
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////
// Use new WinXP style skin/buttons/etc...

#if                                                                       defined _M_IX86
  #pragma comment( linker, "/manifestdependency:\"type='win32' processorArchitecture='x86'   name='Microsoft.Windows.Common-Controls' version='6.0.0.0' publicKeyToken='6595b64144ccf1df'\"" )
#elif                                                                      defined _M_IA64
  #pragma comment( linker, "/manifestdependency:\"type='win32' processorArchitecture='ia64'  name='Microsoft.Windows.Common-Controls' version='6.0.0.0' publicKeyToken='6595b64144ccf1df'\"" )
#elif                                                                        defined _M_X64
  #pragma comment( linker, "/manifestdependency:\"type='win32' processorArchitecture='amd64' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' publicKeyToken='6595b64144ccf1df'\"" )
#else                                                                          // (any/other)
  #pragma comment( linker, "/manifestdependency:\"type='win32' processorArchitecture='*'     name='Microsoft.Windows.Common-Controls' version='6.0.0.0' publicKeyToken='6595b64144ccf1df'\"" )
#endif

//////////////////////////////////////////////////////////////////////////////////////////

#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers

#define  _WIN32_WINNT    0x0500             // VS2005: (Windows 2000 or greater)
#define  WINVER          0x0500             // VS2005: (Windows 2000 or greater)

//////////////////////////////////////////////////////////////////////////////////////////
// (standard/default Windows/MFC headers)

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>       // MFC CCtrlView classes
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>       // MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxdlgs.h>        // MFC CPageSetupDialog
#include <afxtempl.h>       // MFC Template classes
#include <afxpriv.h>        // MFC UNICODE support
#include <winsock2.h>       // (need 'htonl', etc)
#include <winspool.h>       // (need DocumentProperties)
#pragma hdrstop             // (stop pre-compiled headers here...)

//////////////////////////////////////////////////////////////////////////////////////////
// Ensure all modules use same allocator...

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// #includes common to the entire project...

#include "resource.h"           // (project resource #defines)
#include "Product.h"            // (Product information)
#include "Version.h"            // (Version information)
#include "FishLib.h"            // (FishLib support functions and classes)
#include "AWS.h"                // (main AWS file class)
#include "AWSBrowse.h"          // (so everyone can get to g_App)

//////////////////////////////////////////////////////////////////////////////////////////
// Limits, etc...

#define  MAX_MRU                     ( 9 )              // (most recently used files)
#define  MAX_BLOCK_SIZE              ( 256 * 1024 )     // (overridden from Registry)
#define  MAX_BYTES_PER_GROUP         ( MAX_BPG )        // (see fishlib HexEdit.h)
#define  MAX_BYTES_PER_LINE          ( UCHAR_MAX )      // (see fishlib HexEdit.h)
#define  MAX_HEX_SEARCH_STRING_LEN   ( 64 )             // (just a reasonable limit)
#define  MAX_FIND_MRU_ENTRIES        ( 16 )             // (just a reasonable limit)

//////////////////////////////////////////////////////////////////////////////////////////
// Initial DEFAULT values

#define  DEF_HEXEDIT_INDENT          ( 3)       // #of pixels to indent hex-edit display...
#define  DEF_HEXEDIT_BPG             ( 4)       // #of bytes per group
#define  DEF_HEXEDIT_BPR             (16)       // #of bytes per row (bytes per line)
#define  DEF_HEXEDIT_DYNBPR          (TRUE)     // dynamic bytes-per-row option
#define  DEF_HEXEDIT_TEXTONLY_RECLEN (80)       // BPR value when only text area being shown

#define  DEF_HEXEDIT_DEF_CODEPAGE    ASCII_CODE_PAGE        // default code page
#define  DEF_HEXEDIT_TEXT_CODEPAGE   EBCDIC_CODE_PAGE       // text code page
#define  DEF_HEXEDIT_EBCDIC_MODE     (TRUE)                 // (MUST MATCH DEF_HEXEDIT_TEXT_CODEPAGE!!)
#define  DEF_HEXEDIT_VIEWHEX         (TRUE)                 // view hex data as well as text

#define  DEF_HEXEDIT_TEXT_COLOR      GetSysColor( COLOR_WINDOWTEXT )    // text color
#define  DEF_HEXEDIT_BG_COLOR        GetSysColor( COLOR_WINDOW )        // background color

#define  DEF_HEXEDIT_HI_TEXT_COLOR   GetSysColor( COLOR_HIGHLIGHTTEXT ) // highlighted text color
#define  DEF_HEXEDIT_HI_BG_COLOR     GetSysColor( COLOR_HIGHLIGHT )     // highlighted background color

#define  DEF_HEXEDIT_FONTFACE        _T( "Courier New" )    // text font
#define  DEF_HEXEDIT_FONTSIZE        (10)                   // text font point size

//////////////////////////////////////////////////////////////////////////////////////////
// Definition of our listview columns...

#define  COL_0_FILENUM_COLNUM      (0)      // "File: 999"   (or "---" if tapemark)
#define  COL_1_BLOCKNUM_COLNUM     (1)      // "Block: 999"  (or "---" if tapemark)
#define  COL_2_BYTES_COLNUM        (2)      // "9999 bytes"  (or "(tapemark)" if tapemark)

#define  COL_0_DESC_LIT           _T( "File" )
#define  COL_1_DESC_LIT           _T( "Block" )
#define  COL_2_DESC_LIT           _T( "Bytes" )

#define  COL_0_PIXEL_WIDTH         (50)
#define  COL_1_PIXEL_WIDTH         (80)
#define  COL_2_PIXEL_WIDTH         (85)

#define  TOTAL_PIXEL_WIDTH_OF_ALL_COLUMNS  \
    (0                                     \
        + COL_0_PIXEL_WIDTH                \
        + COL_1_PIXEL_WIDTH                \
        + COL_2_PIXEL_WIDTH                \
    )

#define  UNDERLINE_OFFSET( nLineHeight )    ( (int) ( (double) nLineHeight * 0.1 ) )

//////////////////////////////////////////////////////////////////////////////////////////
// Private window messages...

#define  MY_WM_REFRESH_HEXEDIT_VIEW  ( WM_USER + 0 )

//////////////////////////////////////////////////////////////////////////////////////////
// Compression/decompression...

#if defined( _WIN64 )
#define  ZLIB_DLLNAME                "zlib1_64.dll"
#else
#define  ZLIB_DLLNAME                "zlib1_32.dll"
#endif
#define  ZLIB_COMPRESS_FUNC_NAME     "compress2"
#define  ZLIB_UNCOMPRESS_FUNC_NAME   "uncompress"
#define  ZLIB_GETVERSION_FUNC_NAME   "zlibVersion"

#if defined( _WIN64 )
#define  BZIP2_DLLNAME               "libbz2_64.dll"
#else
#define  BZIP2_DLLNAME               "libbz2_32.dll"
#endif
#define  BZIP2_COMPRESS_FUNC_NAME    "BZ2_bzBuffToBuffCompress"
#define  BZIP2_UNCOMPRESS_FUNC_NAME  "BZ2_bzBuffToBuffDecompress"
#define  BZIP2_GETVERSION_FUNC_NAME  "BZ2_bzlibVersion"

///////////////////////////

extern  bool  Compress   ( BYTE*  pOutBuff,  DWORD&  dwOutLen,
                           BYTE*  pInBuff,   DWORD   dwInLen,
                           BYTE   nMethod,
                           BYTE   Level );

extern  bool  Decompress ( BYTE*  pOutBuff,  DWORD&  dwOutLen,
                           BYTE*  pInBuff,   DWORD   dwInLen,
                           BYTE   nMethod );

extern  int   g_nCompressDecompressRC;

extern  CString  Get_ZLIB_DLL_Version();
extern  CString  Get_BZIP2_DLL_Version();

///////////////////////////

#define  COMP_METHOD_ZLIB   (1)     // (choose one or the other)
#define  COMP_METHOD_BZIP2  (2)     // (choose one or the other)

#define  COMP_LEVEL_MIN     (1)     // (rarely used)
#define  COMP_LEVEL_MAX     (9)     // (almost always used)

///////////////////////////

extern  bool  Load_ZLIB_DLL();
extern  bool  Load_BZIP2_DLL();

//////////////////////////////////////////////////////////////////////////////////////////

extern DWORD  g_dwMaxBlockSize;         // (maximum supported block size (from Registry))
extern CLocale  g_Locale;               // (for internationalization)

//////////////////////////////////////////////////////////////////////////////////////////

extern bool IsWordChar( TCHAR ch );
extern bool IsWholeWord( LPCTSTR pszString );
extern bool ValidHexSearchString( LPCTSTR pszHexString );

extern HANDLE CopyHandle( HANDLE hFrom );

//////////////////////////////////////////////////////////////////////////////////////////
