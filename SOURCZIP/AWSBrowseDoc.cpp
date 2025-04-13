// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// AWSBrowseDoc.cpp : implementation of the CAWSBrowseDoc class
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  06/15/06    1.5.0   Fish    VS2005, x64
//  08/04/06    1.5.0   Fish    Use FishLib "SetFilePointer64()" API in place of Win32
//                              SetFilePointerEx() API for compiler version portability
//  11/21/06    1.5.0   Fish    Load zlib/bzip2 DLLs at startup
//  11/21/06    1.5.0   Fish    Added "File -> Close" command.
//  11/21/06    1.5.0   Fish    Created "HandleOpenDocumentError" function to try and 
//                              recover from bad data (trailing garbage), if possible.
//  11/23/06    1.5.0   Fish    Recognize Bus-Tech hardware compression flag.
//  12/05/06    1.5.0   Fish    Maintain general file statistics (CAWSFileStats)
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProgDlg.h"
#include "AWSBrowseDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (embed copyright into object code)
#pragma comment ( user, COPYRIGHT_EXESTR )

//////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE( CAWSBrowseDoc, CDocument )

BEGIN_MESSAGE_MAP( CAWSBrowseDoc, CDocument )

    //{{AFX_MSG_MAP(CAWSBrowseDoc)
    //}}AFX_MSG_MAP

    ON_UPDATE_COMMAND_UI(ID_FILE_MYCLOSE, &CAWSBrowseDoc::OnUpdateMyFileClose)
    ON_COMMAND(ID_FILE_MYCLOSE, &CAWSBrowseDoc::OnMyFileClose)

END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////

CAWSBrowseDoc::CAWSBrowseDoc()
{
    m_hFile = NULL;

    m_arChunkInfo.SetSize( 0, 10000 );
    m_arFileStats.SetSize( 0, 100   );

    m_pProgressDlg = NULL;
    m_bCancelled   = FALSE;
    m_pHexEditView = NULL;
    m_pListView    = NULL;

    m_pTempCompressedDataBuffer = new BYTE [ g_dwMaxBlockSize ];
    m_pTempExpandedDataBuffer   = new BYTE [ g_dwMaxBlockSize ];

    ASSERT( m_pTempCompressedDataBuffer );
    ASSERT( m_pTempExpandedDataBuffer );

    m_bZLibOK     = Load_ZLIB_DLL();
    m_bBZip2OK    = Load_BZIP2_DLL();
    m_bDidWarning = false;
}

//////////////////////////////////////////////////////////////////////////////////////////

CAWSBrowseDoc::~CAWSBrowseDoc()
{
    delete [] m_pTempCompressedDataBuffer;
    delete [] m_pTempExpandedDataBuffer;

    if ( m_pProgressDlg )
    {
        m_pProgressDlg->DestroyWindow();
        delete m_pProgressDlg; m_pProgressDlg = NULL;
    }
    if ( m_hFile )
        CloseHandle( m_hFile );
    m_hFile = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CAWSBrowseDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CAWSBrowseDoc::Dump( CDumpContext& dc ) const
{
    CDocument::Dump( dc );
}
#endif //_DEBUG

//////////////////////////////////////////////////////////////////////////////////////////

void CAWSBrowseDoc::OnUpdateMyFileClose(CCmdUI *pCmdUI)
{
    pCmdUI->Enable( !IsEmpty() );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CAWSBrowseDoc::OnMyFileClose()
{
    VERIFY( OnNewDocument() );
    UpdateAllViews(NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CAWSBrowseDoc::OnNewDocument()
{
    if ( m_hFile )
        CloseHandle( m_hFile );
    m_hFile = NULL;
    if ( !CDocument::OnNewDocument() )
        return FALSE;
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CAWSBrowseDoc::DeleteContents()
{
    if ( m_hFile )
        CloseHandle( m_hFile );

    m_hFile = NULL;

    m_strVol1Label = _T("");

    m_bAWSFlag1 = 0;
    m_bAWSFlag2 = 0;

    m_arChunkInfo.RemoveAll();
    m_arFileStats.RemoveAll();

    m_bZLibOK     = Load_ZLIB_DLL();
    m_bBZip2OK    = Load_BZIP2_DLL();
    m_bDidWarning = false;

    CDocument ::DeleteContents();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Called by the framework as part of the File Open command. The default implementation
// of this function opens the specified file, calls the DeleteContents member function
// to ensure that the document is empty, calls CObject::Serialize to read the file's
// contents, and then marks the document as clean. Override this function if you want to
// use something other than the archive mechanism or the file mechanism.
//
// Return Value:
//
//    TRUE          The document was successfully loaded.
//    FALSE         An error occured; document NOT loaded.
//
// Parameters:
//
//    pszPathName   Points to the path of the document to be opened.
//

BOOL CAWSBrowseDoc::OnOpenDocument( LPCTSTR pszPathName )
{
    DeleteContents();
    BOOL bSuccess = MyDoOpenDocument( pszPathName );
    SetModifiedFlag( FALSE );
    return bSuccess;
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CAWSBrowseDoc::ReturnOpenDocumentError( HANDLE& hFile, LPCTSTR pszPathName, DWORD dwLastError )
{
    CString strLastError = FormatLastError ( dwLastError );

    CString strErrMsg; strErrMsg.Format
    (
        _T( "An error occurred opening or accessing the file:\n\n" )

        _T( "   \"%s\"\n\n" )

        _T( "Message:\n\n" )

        _T( "   %s" )

        ,pszPathName
        ,strLastError
    );

    AfxGetMainWnd()->MessageBox( strErrMsg, _T( "  ERROR" ), MB_ICONERROR | MB_OK );

    if ( hFile )
        CloseHandle( hFile );
    hFile = NULL;

    if ( m_hFile )
        CloseHandle( m_hFile );
    m_hFile = NULL;

    return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Try to recover from bad data, if possible...

bool CAWSBrowseDoc::HandleOpenDocumentError( DWORD dwLastError, LPCTSTR pszPathName )
{
    // Attempt to recover from invalid data by discarding chunks in reverse order
    // until we find one that ISN'T either a BeginningOfBlock or IntermediateBlock
    // (i.e. until we find either a EndOfBlock, NormalBlock or TapeMark chunk).

    INT_PTR        nChunkNum;
    CAWSChunkInfo  chunk_info;
    EAWSChunkType  chunk_type;

    // Look for a spot where we can truncate our array at...

    for (;;)
    {
        if ( m_arChunkInfo.IsEmpty() )
            return false;

        nChunkNum   =  m_arChunkInfo.GetUpperBound();
        chunk_info  =  m_arChunkInfo[ nChunkNum ];
        chunk_type  =  chunk_info.ChunkType();

        if (0
            || EndOfBlock  == chunk_type
            || NormalBlock == chunk_type
            || TapeMark    == chunk_type
        )
            break;

        m_arChunkInfo.RemoveAt( nChunkNum );
    }

    // Let the user know what's going on...

    CString strLastError = FormatLastError ( dwLastError );

    CString strErrMsg; strErrMsg.Format
    (
        _T( "An error occurred opening or accessing the file:\n\n" )

        _T( "   \"%s\"\n\n" )

        _T( "Message:\n\n" )

        _T( "   %s\n\n" )

        _T( "File open processing aborted. The tape appears to be damaged.\n")
        _T( "Attempting to use this tape may produce unpredictable results.\n" )
        _T( "Not all data will be displayed. Only valid data will be shown." )

        ,pszPathName
        ,strLastError
    );

    AfxGetMainWnd()->MessageBox( strErrMsg, _T( "  *WARNING*" ), MB_ICONWARNING | MB_OK );

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CAWSBrowseDoc::MyDoOpenDocument( LPCTSTR pszPathName )
{
    // Close any previous file that may still be open...

    if ( m_hFile )
        CloseHandle( m_hFile );
    m_hFile = NULL;

    // Open the requested file...

    DWORD    dwLastError;
    CString  strFilePath = pszPathName;

    HANDLE hFile = CreateFile
    (
        strFilePath,
        GENERIC_READ,
        0,                      // (exclusive)
        NULL,                   // (security)
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL                    // (template)
    );

    if ( !hFile || INVALID_HANDLE_VALUE == hFile )
    {
        dwLastError = GetLastError();
        return ReturnOpenDocumentError( hFile, strFilePath, dwLastError );
    }

    // Perform the serialization...

    LARGE_INTEGER   n64FileSize;
    LARGE_INTEGER   n64FilePtr;
    LARGE_INTEGER   n64MoveDistance;
    DWORD           dwBytesRead;

    // --------------------------------------------------------------
    //   Create a [new] progress dialog...
    // --------------------------------------------------------------

    if ( m_pProgressDlg )   // (destroy old one if it still exists)
    {
        m_pProgressDlg->DestroyWindow();
        delete m_pProgressDlg; m_pProgressDlg = NULL;
    }

    m_pProgressDlg = new CProgressDlg; ASSERT( m_pProgressDlg );
    m_pProgressDlg->m_strTitle.Format( _T( "Reading: %s" ), strFilePath );
    VERIFY( m_pProgressDlg->Create() ); // (display it)
    m_bCancelled = FALSE;

    // --------------------------------------------------------------
    //   Retrieve the file size...   (for progress dialog purposes)
    // --------------------------------------------------------------

    n64MoveDistance.QuadPart = 0;
    VERIFY( -1 != (n64FileSize.QuadPart = SetFilePointer64( hFile, n64MoveDistance.QuadPart, FILE_END )) );

    // (reset file pointer back to the beginning of the file)

    n64MoveDistance.QuadPart = 0;
    VERIFY( -1 != (n64FilePtr.QuadPart = SetFilePointer64( hFile, n64MoveDistance.QuadPart, FILE_BEGIN )) );

    // (sanity check: should now be pointing to beginning of file)

    ASSERT( !n64FilePtr.QuadPart );

    // --------------------------------------------------------------
    //   Read the file and build an array of CAWSChunkInfo items
    //   based on the chunks and tapemarks we find...
    // --------------------------------------------------------------

    CAWSChunkHdr   chunk_hdr;       // AWS chunk header object
    CAWSChunkInfo  chunk_info;      // AWS chunk header information object
    CAWSFileStats  file_stats;      // file statistics
    WORD           wChunkLen;       // size of current chunk

    bool  bIsTapemark   = false;    // (work)
    int   nTapeMarkNum  = 0;        // (work)

    do
    {
        // Read the [next] AWS chunk header
        // directly into our CAWSChunkHdr object...

        if ( !ReadFile( hFile, &chunk_hdr, NUM_BYTES( chunk_hdr ), &dwBytesRead, NULL ) )
        {
            dwLastError = GetLastError();
            m_pProgressDlg->DestroyWindow();
            delete m_pProgressDlg; m_pProgressDlg = NULL;
            return ReturnOpenDocumentError( hFile, strFilePath, dwLastError );
        }

        file_stats.m_nTotalBytes += NUM_BYTES( chunk_hdr );

        m_bAWSFlag1 |= chunk_hdr.Flag1();   // (informational only)
        m_bAWSFlag2 |= chunk_hdr.Flag2();   // (informational only)

        // Check for end-of-file...

        if ( !dwBytesRead ) break; // (end-of-file)

        // Validate the AWS chunk header information...

        if ( dwBytesRead < NUM_BYTES( chunk_hdr ) || !chunk_hdr.IsValid() )
        {
            dwLastError = ERROR_INVALID_DATA;

            if (HandleOpenDocumentError( dwLastError, strFilePath ))
                break;

            m_pProgressDlg->DestroyWindow();
            delete m_pProgressDlg; m_pProgressDlg = NULL;
            return ReturnOpenDocumentError( hFile, strFilePath, dwLastError );
        }

        // Use the AWS chunk header information to initialize
        // our CAWSChunkInfo object with, and add the chunk_info
        // object to our document array...

        chunk_info.Init( n64FilePtr, &chunk_hdr, nTapeMarkNum );

        if ( (bIsTapemark = chunk_hdr.IsTapeMark()) )
            nTapeMarkNum++;

        m_arChunkInfo.Add( chunk_info );

        // Update statistics...

        if ( bIsTapemark )
        {
            ASSERT( file_stats.m_nTotalBytes );
            m_arFileStats.Add( file_stats );
            file_stats.Reinit();
        }
        else
        {
            wChunkLen = chunk_hdr.CurrChunkLen();
            file_stats.m_nTotalChunkBytes += wChunkLen;
            file_stats.m_nTotalBytes      += wChunkLen;
            file_stats.m_nTotalChunks     += 1;
        }

        // Determine how far we have to skip ahead
        // to reach the next AWS chunk header....

        if ( bIsTapemark )
            n64MoveDistance.QuadPart = 0;
        else
            n64MoveDistance.QuadPart = wChunkLen;

        // Skip ahead to the next AWS chunk header...

        VERIFY( -1 != (n64FilePtr.QuadPart = SetFilePointer64( hFile, n64MoveDistance.QuadPart, FILE_CURRENT )) );

        // Update our progress dialog...

        long double dPercent = (long double) n64FilePtr.QuadPart / (long double) n64FileSize.QuadPart;
        m_pProgressDlg->SetPos( (int) ( dPercent * 100.0 ) );
    }
    while ( !( m_bCancelled = m_pProgressDlg->CheckCancelButton() ) );

    if ( m_bCancelled )
    {
        dwLastError = ERROR_OPERATION_ABORTED;  // (or maybe 'ERROR_MORE_DATA' ???)
        m_pProgressDlg->DestroyWindow();
        delete m_pProgressDlg; m_pProgressDlg = NULL;
        return ReturnOpenDocumentError( hFile, strFilePath, dwLastError );
    }

    // (save stats for last file in case it didn't end with a tapemark)

    if ( !bIsTapemark )
    {
        ASSERT( file_stats.m_nTotalBytes );
        m_arFileStats.Add( file_stats );
    }

    // PROGRAMMING NOTE: since the list-control view may take a while
    // loading the list-control with information from our [potentially
    // quite large!] array of CAWSChunkInfo objects that we just read,
    // we purposely keep the progress dialog displayed and defer to
    // the view the responsibility for destroying it... (it may wish
    // to use it to let the user know of its own progress loading its
    // control with data, and will destroy it when IT'S done with it)...

    // Create a duplicate handle to the file we just serialized, but with only
    // read-only access... (this is to prevent other processes from trying to
    // open the same file with write access in an attempt to modify it while we
    // (our application/program) are reading and displaying it to the user)...

    ASSERT( !m_hFile );         // (sanity check)

    VERIFY( DuplicateHandle
    (
        GetCurrentProcess(),    // handle to source process owning handle to be duplicated
        hFile,                  // the source process's handle that is to be duplicated
        GetCurrentProcess(),    // handle to the process that is to receive the duplicate
        &m_hFile,               // pointer to the handle that is to receive the duplicate
        GENERIC_READ,           // desired access attribute for the duplicated handle
        FALSE,                  // whether the duplicated handle may be inherited or not
        0                       // other optional actions
    ));

    ASSERT( m_hFile && INVALID_HANDLE_VALUE != m_hFile );

    if ( !m_hFile || INVALID_HANDLE_VALUE == m_hFile )
    {
        dwLastError = GetLastError();
        m_pProgressDlg->DestroyWindow();
        delete m_pProgressDlg; m_pProgressDlg = NULL;
        return ReturnOpenDocumentError( hFile, strFilePath, dwLastError );
    }

    // Now that the original file handle has been duplicated,
    // close the original handle since we no longer need it...

    CloseHandle( hFile );

    SetModifiedFlag( FALSE );

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function to return a specific CAWSChunkInfo item...

CAWSChunkInfo&  CAWSBrowseDoc::ChunkInfo( int nChunkNum )
{
    return ( nChunkNum >= 0 && nChunkNum <= m_arChunkInfo.GetUpperBound() ) ?
        m_arChunkInfo[ nChunkNum ] : *((CAWSChunkInfo*)NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Functions to get/set a specific CAWSFileStats item...

CAWSFileStats&  CAWSBrowseDoc::GetFileStats( int nFileNum )
{
    return ( nFileNum >= 0 && nFileNum <= m_arFileStats.GetUpperBound() ) ?
        m_arFileStats[ nFileNum ] : *((CAWSFileStats*)NULL);
}

bool CAWSBrowseDoc::SetFileStats( int nFileNum, const CAWSFileStats& rStats )
{
    if ( nFileNum < 0 || nFileNum > m_arFileStats.GetUpperBound() )
    {
        ASSERT (false);
        return  false;
    }

    m_arFileStats [ nFileNum ] = rStats;
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool GetBlockDataError( BYTE*& p, int& n ) { p = NULL; n = 0; return false; }

// The following utility function is used by the CMyHexEditView object to retrieve the
// actual "tape" file data block in both compressed and uncompressed format. The chunk#
// that is passed identifies which data block is requested and must point to a chunk info
// item of "NormalBlock" or "BeginningOfBlock" type.

bool CAWSBrowseDoc::GetBlockData
(
    int     nChunkNum,          // IN   where this block begins
    BYTE*&  pCompressedData,    // OUT  --> raw compressed data
    int&    nCompressedLen,     // OUT  ==  raw compressed len
    BYTE*&  pExpandedData,      // OUT  --> expanded data block
    int&    nExpandedLen        // OUT  ==  expanded block len
)
{
    // Initialize return values...

    pCompressedData  =  pExpandedData  = NULL;
    nCompressedLen   =  nExpandedLen   = 0;

    // Validate passed chunk#...

    if ( nChunkNum < 0 || nChunkNum > m_arChunkInfo.GetUpperBound() )
        return GetBlockDataError( pCompressedData, nCompressedLen );

    // Retrieve the chunk info and validate...

    CAWSChunkInfo   chunk_info   =  m_arChunkInfo[ nChunkNum ];
    EAWSChunkType   chunk_type   =  chunk_info.ChunkType();

    if ( !( NormalBlock == chunk_type || BeginningOfBlock == chunk_type ) )
        return GetBlockDataError( pCompressedData, nCompressedLen );

    // Initialize starting flags. These must not change
    // as we gather all of this block's chunks together.
    // If they do, then the file is damaged and we cannot
    // proceed...

    bool bIsCompressed  = chunk_info.IsCompressed();
    bool bIsHCompresed  = chunk_info.IsHCompressed();
    bool bIsZCompresed  = chunk_info.IsZCompressed();
    bool bIsSpannedBlk  = chunk_info.ChunkType() != NormalBlock;

    // Gather all of this block's data chunks together
    // into one large contiguous data buffer...

    pCompressedData  =  m_pTempCompressedDataBuffer;
    nCompressedLen   =  chunk_info.CurrChunkLen();

    BYTE*           pChunkBuff   = pCompressedData;
    DWORD           dwChunkSize  = nCompressedLen;
    DWORD           dwBytesRead;
    LARGE_INTEGER   n64FilePtr;

    for (;;)
    {
        // Read the first/next/only data chunk into our buffer...

        n64FilePtr.QuadPart = chunk_info.FilePtr().QuadPart + NUM_BYTES( CAWSChunkHdr );
        dwBytesRead = 0;

        if (0
            || SetFilePointer64( m_hFile, n64FilePtr.QuadPart, FILE_BEGIN ) == -1
            || !ReadFile( m_hFile, pChunkBuff, dwChunkSize, &dwBytesRead, NULL )
            || dwBytesRead != dwChunkSize
        )
            return GetBlockDataError( pCompressedData, nCompressedLen );

        pChunkBuff += dwBytesRead; // (where next chunk will go if needed)

        // Are there any more chunks remaining for this data block?

        if ( ( !bIsSpannedBlk  &&  NormalBlock == chunk_type ) ||
             (  bIsSpannedBlk  &&  EndOfBlock  == chunk_type ) )
        {
            break;  // (that's all there is and there ain't no more)
        }

        // Go on to the next chunk...

        ASSERT( bIsSpannedBlk ); nChunkNum++;

        if ( nChunkNum < 0 || nChunkNum > m_arChunkInfo.GetUpperBound() )
            return GetBlockDataError( pCompressedData, nCompressedLen );

        chunk_info = m_arChunkInfo[ nChunkNum ];
        chunk_type = chunk_info.ChunkType();

        dwChunkSize = chunk_info.CurrChunkLen();
        nCompressedLen += dwChunkSize;

        // Check to make sure we're not about to overflow our i/o buffer
        // and that this next chunk is of the proper type we expect
        // (i.e. that the file is consistent and not damaged)...

        if (0
            || nCompressedLen < 0                       // (would overflow)
            || nCompressedLen > (int) g_dwMaxBlockSize  // (would overflow)
            || !( IntermediateBlock == chunk_type || EndOfBlock == chunk_type )
            || chunk_info.IsCompressed()  !=  bIsCompressed
            || chunk_info.IsHCompressed() !=  bIsHCompresed
            || chunk_info.IsZCompressed() !=  bIsZCompresed
        )
            return GetBlockDataError( pCompressedData, nCompressedLen );
    }

    // If this isn't compressed data, then we're done...

    if ( !bIsCompressed )
    {
        pExpandedData   = pCompressedData;
        nExpandedLen    = nCompressedLen;
        pCompressedData = NULL;
        nCompressedLen  = 0;
        return true;
    }

    // Otherwise we need to expand it...

    // If the decompression isn't possible
    // then there's no sense in even trying...

    if (0
        || ( !bIsHCompresed && !bIsZCompresed )     // Bus-Tech hardware compressed
        || (  bIsZCompresed && !m_bZLibOK     )     // ZLIB dll not available
        || ( !bIsZCompresed && !m_bBZip2OK    )     // BZIP2 dll not available
    )
    {
        if ( !m_bDidWarning )
        {
            m_bDidWarning = true;
            CString str;

            if ( !bIsHCompresed && !bIsZCompresed )     // Bus-Tech hardware compressed
            {
                str = _T( "ERROR: Non-Hercules or non-ZLIB compressed. Data decompression not possible." );
            }
            else if ( bIsZCompresed && !m_bZLibOK )     // ZLIB compressed but ZLIB not available
            {
                str.Format
                (
                    _T( "ERROR: '%s' or required entry points not found. Data decompression not possible." )
                    ,_T( ZLIB_DLLNAME )
                );
            }
            else if ( !bIsZCompresed && !m_bBZip2OK )   // BZIP2 compressed but BZIP2 not available
            {
                str.Format
                (
                    _T( "ERROR: '%s' or required entry points not found. Data decompression not possible." )
                    ,_T( BZIP2_DLLNAME )
                );
            }

            TRACE( _T( "** CAWSBrowseDoc::GetBlockData: %s\n" ), str );
            AfxMessageBox( str );
        }

        return false;   // (no sense in even trying)
    }

    // Everything looks okay. Attempt decompression...

    DWORD dwExpandedLen = g_dwMaxBlockSize;

    if ( !Decompress
    (
        m_pTempExpandedDataBuffer,
        dwExpandedLen,

        pCompressedData,
        nCompressedLen,

        bIsZCompresed ? COMP_METHOD_ZLIB : COMP_METHOD_BZIP2
    ))
    {
        // DECOMPRESSION ERROR: return the compressed data block
        // anyway in case they want to examine the data to try and
        // figure out what went wrong...

        return false;
    }

    pExpandedData = m_pTempExpandedDataBuffer;
    nExpandedLen  = dwExpandedLen;

    ASSERT( nExpandedLen > 0 );

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
