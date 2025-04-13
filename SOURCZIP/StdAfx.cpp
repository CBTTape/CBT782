// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// stdafx.cpp : source file that includes just the standard includes
// AWSBrowse.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  06/15/06    1.5.0   Fish    VS2005, x64
//  11/21/06    1.5.0   Fish    Load zlib/bzip2 DLLs at startup
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

// (embed copyright into object code)
#pragma comment ( user, COPYRIGHT_EXESTR )

//////////////////////////////////////////////////////////////////////////////////////////
// (for reference; from zlib.h and bzip.h)

#define Z_OK                   ( 0)
#define BZ_OK                  ( 0)

#define Z_BEST_SPEED           ( 1)
#define Z_BEST_COMPRESSION     ( 9)

#define BZ_BEST_SPEED          ( 1)
#define BZ_BEST_COMPRESSION    ( 9)

///////////////////////////

#define Z_STREAM_END           (+1)
#define BZ_RUN_OK              (+1)

#define Z_NEED_DICT            (+2)
#define BZ_FLUSH_OK            (+2)

#define BZ_FINISH_OK           (+3)
#define BZ_STREAM_END          (+4)

///////////////////////////

#define Z_ERRNO                (-1)
#define BZ_SEQUENCE_ERROR      (-1)

#define Z_STREAM_ERROR         (-2)
#define BZ_PARAM_ERROR         (-2)

#define Z_DATA_ERROR           (-3)
#define BZ_MEM_ERROR           (-3)

#define Z_MEM_ERROR            (-4)
#define BZ_DATA_ERROR          (-4)

#define Z_BUF_ERROR            (-5)
#define BZ_DATA_ERROR_MAGIC    (-5)

#define Z_VERSION_ERROR        (-6)
#define BZ_IO_ERROR            (-6)

#define BZ_UNEXPECTED_EOF      (-7)
#define BZ_OUTBUFF_FULL        (-8)
#define BZ_CONFIG_ERROR        (-9)

//////////////////////////////////////////////////////////////////////////////////////////

typedef  int ( __cdecl  ZLIB_COMPRESS2_FUNC )
(
          BYTE*   dest,
          ULONG*  destLen,
    const BYTE*   source,
          ULONG   sourceLen,
          int     level
);

typedef  int ( __cdecl  ZLIB_UNCOMPRESS_FUNC )
(
          BYTE*   dest,
          ULONG*  destLen,
    const BYTE*   source,
          ULONG   sourceLen
);

typedef  char* ( __cdecl  ZLIB_GETVERSION_FUNC )();

typedef  int ( WINAPI  BZIP2_COMPRESS_FUNC )
(
    char*          dest,
    unsigned int*  destLen,
    char*          source,
    unsigned int   sourceLen,
    int            level,           // Fish: just a better name IMO
    int            verbosity,
    int            workFactor
);

typedef  int ( WINAPI  BZIP2_UNCOMPRESS_FUNC )
(
    char*          dest,
    unsigned int*  destLen,
    char*          source,
    unsigned int   sourceLen,
    int            small_opt,       // Fish: 'small' is a reserved word in VC++
    int            verbosity
);

typedef  char* ( WINAPI  BZIP2_GETVERSION_FUNC )();

//////////////////////////////////////////////////////////////////////////////////////////
// (global vars...)

HMODULE                  g_h_ZLIB_DLL             = NULL;
HMODULE                  g_h_BZIP2_DLL            = NULL;

ZLIB_COMPRESS2_FUNC*     g_pfn_ZLIB_Compress      = NULL;
ZLIB_UNCOMPRESS_FUNC*    g_pfn_ZLIB_Decompress    = NULL;
ZLIB_GETVERSION_FUNC*    g_pfn_ZLIB_GetVersion    = NULL;

BZIP2_COMPRESS_FUNC*     g_pfn_BZIP2_Compress     = NULL;
BZIP2_UNCOMPRESS_FUNC*   g_pfn_BZIP2_Decompress   = NULL;
BZIP2_GETVERSION_FUNC*   g_pfn_BZIP2_GetVersion   = NULL;

int                      g_nCompressDecompressRC  = 0;

//////////////////////////////////////////////////////////////////////////////////////////

void Unload_ZLIB_DLL()
{
    if ( g_h_ZLIB_DLL )
        VERIFY( FreeLibrary( g_h_ZLIB_DLL ) );

    g_h_ZLIB_DLL          = NULL;
    g_pfn_ZLIB_Compress   = NULL;
    g_pfn_ZLIB_Decompress = NULL;
}

void Unload_BZIP2_DLL()
{
    if ( g_h_BZIP2_DLL )
        VERIFY( FreeLibrary( g_h_BZIP2_DLL ) );

    g_h_BZIP2_DLL          = NULL;
    g_pfn_BZIP2_Compress   = NULL;
    g_pfn_BZIP2_Decompress = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool Load_ZLIB_DLL()
{
    if ( g_h_ZLIB_DLL ) return true;

    Unload_ZLIB_DLL();

    g_h_ZLIB_DLL = LoadLibrary( _T( ZLIB_DLLNAME ) );

    if ( !g_h_ZLIB_DLL )
        return false;

    g_pfn_ZLIB_Compress   = (ZLIB_COMPRESS2_FUNC*)  GetProcAddress( g_h_ZLIB_DLL, ZLIB_COMPRESS_FUNC_NAME   );
    g_pfn_ZLIB_Decompress = (ZLIB_UNCOMPRESS_FUNC*) GetProcAddress( g_h_ZLIB_DLL, ZLIB_UNCOMPRESS_FUNC_NAME );
    g_pfn_ZLIB_GetVersion = (ZLIB_GETVERSION_FUNC*) GetProcAddress( g_h_ZLIB_DLL, ZLIB_GETVERSION_FUNC_NAME );

    if (1
        &&  g_pfn_ZLIB_Compress
        &&  g_pfn_ZLIB_Decompress
        &&  g_pfn_ZLIB_GetVersion
    )
        return true;

    Unload_ZLIB_DLL();
    return false;
}

bool Load_BZIP2_DLL()
{
    if ( g_h_BZIP2_DLL ) return true;

    Unload_BZIP2_DLL();

    g_h_BZIP2_DLL = LoadLibrary( _T( BZIP2_DLLNAME ) );

    if ( !g_h_BZIP2_DLL )
        return false;

    g_pfn_BZIP2_Compress   = (BZIP2_COMPRESS_FUNC*)   GetProcAddress( g_h_BZIP2_DLL, BZIP2_COMPRESS_FUNC_NAME   );
    g_pfn_BZIP2_Decompress = (BZIP2_UNCOMPRESS_FUNC*) GetProcAddress( g_h_BZIP2_DLL, BZIP2_UNCOMPRESS_FUNC_NAME );
    g_pfn_BZIP2_GetVersion = (BZIP2_GETVERSION_FUNC*) GetProcAddress( g_h_BZIP2_DLL, BZIP2_GETVERSION_FUNC_NAME );

    if (1
        &&  g_pfn_BZIP2_Compress
        &&  g_pfn_BZIP2_Decompress
        &&  g_pfn_BZIP2_GetVersion
    )
        return true;

    Unload_BZIP2_DLL();
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////

CString Get_ZLIB_DLL_Version()
{
    CString strVersion = _T( "n/a" );
    if ( g_pfn_ZLIB_GetVersion )
        strVersion = g_pfn_ZLIB_GetVersion();
    return strVersion;
}

CString Get_BZIP2_DLL_Version()
{
    CString strVersion = _T( "n/a" );
    if ( g_pfn_BZIP2_GetVersion )
        strVersion = g_pfn_BZIP2_GetVersion();
    return strVersion;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool Compress( BYTE* pOutBuff, DWORD& dwOutLen,
               BYTE* pInBuff,  DWORD  dwInLen,
               BYTE  nMethod,  BYTE   nLevel    )
{
    if ( nMethod != COMP_METHOD_ZLIB &&
         nMethod != COMP_METHOD_BZIP2 )
        return false;

    if ( nLevel < COMP_LEVEL_MIN || nLevel > COMP_LEVEL_MAX )
        return false;

    if ( nMethod == COMP_METHOD_ZLIB && !g_h_ZLIB_DLL )
        return false;

    if ( nMethod == COMP_METHOD_BZIP2 && !g_h_BZIP2_DLL )
        return false;

    ASSERT(( nMethod == COMP_METHOD_ZLIB  && g_pfn_ZLIB_Compress  )
        || ( nMethod == COMP_METHOD_BZIP2 && g_pfn_BZIP2_Compress ));

    return ( nMethod == COMP_METHOD_ZLIB ) ?
    (
        ( g_nCompressDecompressRC = g_pfn_ZLIB_Compress
        (
            (BYTE*)          pOutBuff,
            (ULONG*)        &dwOutLen,

            (const BYTE*)    pInBuff,
            (ULONG)          dwInLen,

            (int)            nLevel
        ))
        == Z_OK ? true : false
    )
    :
    (
        ( g_nCompressDecompressRC = g_pfn_BZIP2_Compress
        (
            (char*)           pOutBuff,
            (unsigned int*)  &dwOutLen,

            (char*)           pInBuff,
            (unsigned int)    dwInLen,

            (int)             nLevel

            ,0,0
        ))
        == BZ_OK ? true : false
    )
    ;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool Decompress( BYTE* pOutBuff, DWORD& dwOutLen,
                 BYTE* pInBuff,  DWORD  dwInLen,
                 BYTE  nMethod                     )
{
    if ( nMethod != COMP_METHOD_ZLIB &&
         nMethod != COMP_METHOD_BZIP2 )
        return false;

    if ( nMethod == COMP_METHOD_ZLIB && !g_h_ZLIB_DLL )
        return false;

    if ( nMethod == COMP_METHOD_BZIP2 && !g_h_BZIP2_DLL )
        return false;

    ASSERT(( nMethod == COMP_METHOD_ZLIB  && g_pfn_ZLIB_Decompress  )
        || ( nMethod == COMP_METHOD_BZIP2 && g_pfn_BZIP2_Decompress ));

    // (the following makes it easier to set debugging breakpoints)

    bool bSuccess;

    if ( nMethod == COMP_METHOD_ZLIB )
    {
        bSuccess =
        (
            g_nCompressDecompressRC = g_pfn_ZLIB_Decompress
            (
                (BYTE*)          pOutBuff,
                (ULONG*)        &dwOutLen,

                (const BYTE*)    pInBuff,
                (ULONG)          dwInLen
            )
        )
        == Z_OK ? true : false;
    }
    else // ( nMethod == COMP_METHOD_BZIP2 )
    {
        bSuccess =
        (
            g_nCompressDecompressRC = g_pfn_BZIP2_Decompress
            (
                (char*)           pOutBuff,
                (unsigned int*)  &dwOutLen,

                (char*)           pInBuff,
                (unsigned int)    dwInLen

                ,0,0
            )
        )
        == BZ_OK ? true : false;
    }

    if ( bSuccess )
        return true;
    else
        return false;
}

//////////////////////////////////////////////////////////////////////////////////////////

DWORD g_dwMaxBlockSize = MAX_BLOCK_SIZE;        // (overridden from Registry)
CLocale  g_Locale;                              // (for internationalization)

//////////////////////////////////////////////////////////////////////////////////////////

bool IsWordChar( TCHAR ch )
{
    return ( _T( '_' ) == ch || _istalnum( ch ) );
}

//////////////////////////////////////////////////////////////////////////////////////////

bool IsWholeWord( LPCTSTR pszString )
{
    do
        if ( !IsWordChar( *pszString ) )
            return false; 
    while ( *++pszString );
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool ValidHexSearchString( LPCTSTR pszHexString )
{
    CString strTemp = pszHexString;

    return (1
        &&  IsHexString( strTemp )
        &&  strTemp.GetLength() <= MAX_HEX_SEARCH_STRING_LEN
        &&  !( strTemp.GetLength() % 2 )
    );
}

//////////////////////////////////////////////////////////////////////////////////////////

HANDLE CopyHandle( HANDLE hFrom )
{
    ASSERT( hFrom ); if ( !hFrom ) return NULL;
    SIZE_T nDataLen = GlobalSize( hFrom );
    ASSERT( nDataLen ); if ( !nDataLen ) return NULL;

    HANDLE  hCopy = NULL;
    BYTE*   pCopy = NULL;
    BYTE*   pFrom = NULL;

    if ( ( hCopy = GlobalAlloc( GHND, nDataLen ) ) )
    {
        pCopy = (BYTE*) GlobalLock( hCopy ); ASSERT( pCopy );
        pFrom = (BYTE*) GlobalLock( hFrom ); ASSERT( pFrom );
        CopyMemory( pCopy, pFrom, nDataLen );
        GlobalUnlock( hFrom ); pFrom = NULL;
        GlobalUnlock( hCopy ); pCopy = NULL;
    }

    return hCopy;
}

//////////////////////////////////////////////////////////////////////////////////////////
