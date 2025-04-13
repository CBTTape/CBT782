// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// MyListView.cpp : implementation of the CMyListView class
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  03/19/06    1.3.0   Fish    UNICODE fixes
//  03/19/06    1.3.0   Fish    Support toggling display of Hex view on or off
//  03/20/06    1.3.0   Fish    Remove hex len from summary
//  03/20/06    1.3.0   Fish    Make dialog resizeable
//  06/15/06    1.5.0   Fish    VS2005, x64
//  08/05/06    1.4.0   Fish    Support for searching within block, file or entire tape.
//  11/23/06    1.5.0   Fish    Recognize Bus-Tech hardware compression flag.
//  12/05/06    1.5.0   Fish    Maintain/repirt general file statistics (CAWSFileStats)
//  12/07/06    1.5.0   Fish    Maintain list of HDR1 names for new "Go To..." command
//  12/09/06    1.5.0   Fish    Use new CMyFontDialog
//  01/14/07    1.5.0   Fish    Changes needed for new FishLib
//  01/17/07    1.5.0   Fish    Fix bug in UpdateSearchParms when empty search string.
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AWSBrowseDoc.h"
#include "MyListView.h"
#include "ProgDlg.h"
#include "MyHexEditView.h"
#include "MyFindDialog.h"
#include "BlockDetailDlg.h"
#include "GoToDlg.h"
#include "MyFontDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE( CMyListView, CListView )

BEGIN_MESSAGE_MAP( CMyListView, CListView )

    //{{AFX_MSG_MAP(CMyListView)
    ON_WM_CREATE()
    ON_COMMAND(ID_EDIT_FIND, OnEditFind)
    ON_COMMAND(ID_EDIT_FINDNEXT, OnEditFindNext)
    ON_COMMAND(ID_EDIT_FINDPREVIOUS, OnEditFindPrevious)
    ON_COMMAND(ID_EDIT_NEXTBLOCK, OnEditNextBlock)
    ON_COMMAND(ID_EDIT_PREVIOUSBLOCK, OnEditPreviousBlock)
    ON_COMMAND(ID_EDIT_NEXTFILE, OnEditNexFile)
    ON_COMMAND(ID_EDIT_PREVIOUSFILE, OnEditPreviousFile)
    ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
    ON_COMMAND(ID_FILE_PAGE_SETUP, OnFilePageSetup)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
    ON_COMMAND(ID_VIEW_PRINTERFONT, OnViewPrinterFont)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_LVRCLICK_INFO, OnListViewRightClickInfo)
    ON_WM_LBUTTONDBLCLK()
    ON_COMMAND(ID_VIEW_STATISTICS, &CMyListView::OnViewStatistics)
    ON_UPDATE_COMMAND_UI(ID_EDIT_GOTO, &CMyListView::OnUpdateEditGoto)
    ON_COMMAND(ID_EDIT_GOTO, &CMyListView::OnEditGoto)
    //}}AFX_MSG_MAP

    // Print/Print Preview support...

    ON_COMMAND ( ID_FILE_PAGE_SETUP,    OnFilePageSetup    )
    ON_COMMAND ( ID_FILE_PRINT_PREVIEW, OnFilePrintPreview )
    ON_COMMAND ( ID_FILE_PRINT_DIRECT,  OnFilePrint        )
//  ON_COMMAND ( ID_FILE_PRINT,         OnFilePrint        )   // (already in above map)

    // Programming Note: ON_NOTIFY_REFLECT( LVN_ITEMCHANGED ) catches both clicks
    // and double-clicks too, as well as keyboard selections and de-selections...

    ON_NOTIFY_REFLECT ( LVN_ITEMCHANGED, OnItemStateChange )

END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////

CMyListView::CMyListView()
{
    m_pDoc      = NULL;
    m_pListCtrl = NULL;

    m_nSelectedItemNum  = -1;
    m_nSelectedChunkNum = -1;

    // Find/Replace support...

    m_pSearchData     = NULL;
    m_nSearchDataLen  = 0;
    m_pDataBlock      = NULL;
    m_nBlockLen       = 0;
    m_bForwSearch     = TRUE;
    m_nBlockPos       = 0;

    m_nSearchWhere    = FIND_ONTAPE;

    memset( m_nForwShiftTable, 0, NUM_BYTES( m_nForwShiftTable ) );
    memset( m_nBackShiftTable, 0, NUM_BYTES( m_nBackShiftTable ) );

    m_pPrinterFont = NULL;

    LoadProfileSettings();

    VERIFY( g_App.SetMFCsDefaultPrinterToLandscapeMode() );
}

//////////////////////////////////////////////////////////////////////////////////////////

CMyListView::~CMyListView()
{
    delete [] m_pDataBlock;
    delete [] m_pSearchData;
    delete m_pPrinterFont;

    SaveProfileSettings();
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::LoadProfileSettings()
{
    UINT nMRUCount = g_App.GetProfileInt( _T( "FindMRU" ), _T( "Count" ), 0 );

    CString strMRUEntryNum, strMRUValue;

    for ( UINT nMRUEntryNum = 0; nMRUEntryNum < nMRUCount; nMRUEntryNum++ )
    {
        strMRUEntryNum.Format( _T( "%d" ), nMRUEntryNum );
        strMRUValue = g_App.GetProfileString( _T( "FindMRU" ), strMRUEntryNum );
        m_FindWhatList.AddTail( strMRUValue );
    }

    if ( !m_FindWhatList.IsEmpty() )
        m_strSearchString = m_FindWhatList.GetHead();

    m_bWholeWord    = g_App.GetProfileInt( _T( "FindMRU" ), _T( "MatchWord" ), FALSE );
    m_bMatchCase    = g_App.GetProfileInt( _T( "FindMRU" ), _T( "MatchCase" ), FALSE );
    m_bHexSearch    = g_App.GetProfileInt( _T( "FindMRU" ), _T( "HexSeach"  ), FALSE );

    m_nSearchWhere  = g_App.GetProfileInt( _T( "FindMRU" ), _T( "SearchWhere" ), m_nSearchWhere );

    m_nLeftMargin   = g_App.GetProfileInt( _T( "Margins" ), _T( "Left"      ),   -1  );
    m_nRightMargin  = g_App.GetProfileInt( _T( "Margins" ), _T( "Right"     ),   -1  );
    m_nTopMargin    = g_App.GetProfileInt( _T( "Margins" ), _T( "Top"       ),   -1  );
    m_nBottomMargin = g_App.GetProfileInt( _T( "Margins" ), _T( "Bottom"    ),   -1  );

    m_strPrinterFontName = g_App.GetProfileString( _T( "Settings" ), _T( "PrinterFontName" ), DEF_HEXEDIT_FONTFACE );
    m_nPrinterFontSize   = g_App.GetProfileInt   ( _T( "Settings" ), _T( "PrinterFontSize" ), DEF_HEXEDIT_FONTSIZE );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::SaveProfileSettings()
{
    g_App.WriteProfileInt( _T( "FindMRU" ), _T( "Count" ), (int)m_FindWhatList.GetCount() );

    CString strMRUEntryNum, strMRUValue;
    UINT nMRUEntryNum = 0;

    POSITION pos = m_FindWhatList.GetHeadPosition();

    while ( pos )
    {
        strMRUValue = m_FindWhatList.GetNext( pos );
        strMRUEntryNum.Format( _T( "%d" ), nMRUEntryNum++ );
        g_App.WriteProfileString( _T( "FindMRU" ), strMRUEntryNum, strMRUValue );
    }

    g_App.WriteProfileInt( _T( "FindMRU"  ), _T( "MatchWord" ), m_bWholeWord    );
    g_App.WriteProfileInt( _T( "FindMRU"  ), _T( "MatchCase" ), m_bMatchCase    );
    g_App.WriteProfileInt( _T( "FindMRU"  ), _T( "HexSeach"  ), m_bHexSearch    );

    g_App.WriteProfileInt( _T( "FindMRU"  ), _T( "SearchWhere" ), m_nSearchWhere );

    g_App.WriteProfileInt( _T( "Margins"  ), _T( "Left"      ), m_nLeftMargin   );
    g_App.WriteProfileInt( _T( "Margins"  ), _T( "Right"     ), m_nRightMargin  );
    g_App.WriteProfileInt( _T( "Margins"  ), _T( "Top"       ), m_nTopMargin    );
    g_App.WriteProfileInt( _T( "Margins"  ), _T( "Bottom"    ), m_nBottomMargin );

    g_App.WriteProfileString( _T( "Settings" ), _T( "PrinterFontName" ), m_strPrinterFontName );
    g_App.WriteProfileInt   ( _T( "Settings" ), _T( "PrinterFontSize" ), m_nPrinterFontSize );
}

//////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CMyListView::AssertValid() const
{
    CListView::AssertValid();
}

void CMyListView::Dump( CDumpContext& dc ) const
{
    CListView::Dump( dc );
}

CAWSBrowseDoc* CMyListView::GetDocument() // (non-debug version is inline)
{
    ASSERT( m_pDocument->IsKindOf( RUNTIME_CLASS( CAWSBrowseDoc ) ) );
    return (CAWSBrowseDoc*) m_pDocument;
}
#endif //_DEBUG

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CMyListView::PreCreateWindow( CREATESTRUCT& cs )
{
    // Turn OFF styles we DON'T want

    cs.style &= ~(
        0
        | LVS_EDITLABELS
        | LVS_ICON
        | LVS_LIST
//      | LVS_NOCOLUMNHEADER
        | LVS_NOSCROLL
        | LVS_OWNERDRAWFIXED
        | LVS_SHAREIMAGELISTS
        | LVS_SMALLICON
        | LVS_SORTASCENDING
        | LVS_SORTDESCENDING
    );

    // Turn ON styles we DO want

    cs.style |= (
        0
        | LVS_REPORT
        | LVS_SHOWSELALWAYS
        | LVS_SINGLESEL
    );

    return CListView::PreCreateWindow( cs );
}

//////////////////////////////////////////////////////////////////////////////////////////

int CMyListView::OnCreate( CREATESTRUCT* pCreateStruct )
{
    if ( CListView ::OnCreate( pCreateStruct ) == -1 )
        return -1;

    // Perform one-time-only initialization...

    // Save ptr to list control so we don't have
    // to keep retrieving it each time...

    m_pListCtrl = &GetListCtrl();

    // Set our list control options...

    m_pListCtrl->SetExtendedStyle
    (0
        | LVS_EX_FULLROWSELECT
        | LVS_EX_LABELTIP
        | LVS_EX_HEADERDRAGDROP
    );

    // Define our columns...

    VERIFY( -1 != m_pListCtrl->InsertColumn( COL_0_FILENUM_COLNUM,  COL_0_DESC_LIT, LVCFMT_LEFT, COL_0_PIXEL_WIDTH ) );
    VERIFY( -1 != m_pListCtrl->InsertColumn( COL_1_BLOCKNUM_COLNUM, COL_1_DESC_LIT, LVCFMT_LEFT, COL_1_PIXEL_WIDTH ) );
    VERIFY( -1 != m_pListCtrl->InsertColumn( COL_2_BYTES_COLNUM   , COL_2_DESC_LIT, LVCFMT_LEFT, COL_2_PIXEL_WIDTH ) );

    m_pDoc = GetDocument();
    m_pDoc->m_pListView = this; // (we exist now)

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint )
{
    VERIFY( m_pListCtrl->DeleteAllItems() );

    if (m_pDoc->m_pHexEditView)
        m_pDoc->m_pHexEditView->SendMessage( MY_WM_REFRESH_HEXEDIT_VIEW, -1, 0 );

    m_nSelectedItemNum  = -1;
    m_nSelectedChunkNum = -1;

    delete [] m_pDataBlock;
    m_pDataBlock = NULL;
    m_nBlockLen  = 0;
    m_nBlockPos  = 0;

    if ( m_pDoc->IsEmpty() )
        return;

    FillListControlWithData();

    if ( !m_strSearchString.IsEmpty() )
    {
        // Switch the hexedit control over to match whatever format
        // our initial search string value is in...

        if (1
            && m_pDoc->m_pHexEditView->IsHexView()  // (hex area must be visible to switch to it!)
            && ValidHexSearchString( m_strSearchString )
        )
            m_pDoc->m_pHexEditView->SetTextArea( FALSE );
        else
            m_pDoc->m_pHexEditView->SetTextArea( TRUE );

        UpdateSearchParms();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::FillListControlWithData()
{
    ASSERT( !m_pDoc->IsEmpty() );           // (sanity check: document must contain data)
    ASSERT( !m_pListCtrl->GetItemCount() ); // (sanity check: list control must be empty)

    // (re-init to empty)

    m_arHDR1Names.RemoveAll();              // (array of HDR1 file names)
    m_arHDR1ChunkNums.RemoveAll();          // (corresponding chunk nums)
    m_nGoToEntry = 0;                       // (reinit each time)

    // Tell the list control of approximately how many items we're
    // going to be adding... (this helps to speed things up a bit)

    m_pListCtrl->SetItemCount( m_pDoc->NumChunks() );   // (not exact, but close enough)

    // The document creates/displays a progress dialog whenever
    // it loads a file (since doing so may take a while depending
    // on how big the file is) but it doesn't destroy it once the
    // file is loaded since, if the file IS a large one, then it
    // logically follows that WE TOO are probably going to take
    // a short while to "process" the data too (and thus having
    // a progress dialog to let the user know the file is still
    // in the process of being loaded is probably a good idea).

    // Thus, the progress dialog "sticks around" until we're ALSO
    // done with it too. It only gets destroyed (BY US!) whenever
    // WE'RE done with OUR processing...

    ASSERT( m_pDoc->m_pProgressDlg );

    m_pDoc->m_pProgressDlg->SetWindowText( _T( " Calculating block sizes..." ) );
    m_pDoc->m_pProgressDlg->SetPos(0);
    m_pDoc->m_pProgressDlg->SetRange32( 0, m_pDoc->NumChunks() - 1 );

    // Now start inserting our items into the listview control...

    CAWSChunkInfo  chunk_info;
    EAWSChunkType  chunk_type;
    CAWSFileStats  file_stats;
    int            nTapeMarkNum   = 0;      // (physical file#)
    int            nFileNum       = 1;      // (logical file#)
    int            nFileBlockNum  = 0;
    int            nFileBlockLen  = 0;
    int            nBlockChunkNum = 0;
    int            nItemNum;
    CString        strFileNum, strBlockNum, strNumBytes;
    BOOL           bCancelled     = FALSE;
    bool           bDecompressOK, bBlockOK, bBlockErrors = false;

    BYTE*  pCompressedData;     // raw compressed data
    int    nCompressedLen;      // raw compressed len
    BYTE*  pExpandedData;       // expanded data block
    int    nExpandedLen;        // expanded block len

    // (turn off redraw while the list control is being filled with items)

    m_pListCtrl->SetRedraw( FALSE );

    // Process each chunk in the document...

    file_stats = m_pDoc->GetFileStats( 0 );     // (starting file stats)

    for
    (
        int  nChunkNum = 0,   nNumChunks = m_pDoc->NumChunks()
        ;
        nChunkNum < nNumChunks  &&  !( bCancelled = m_pDoc->m_pProgressDlg->CheckCancelButton() )
        ;
        nChunkNum++
    )
    {
        if (m_pDoc->m_pProgressDlg)
            m_pDoc->m_pProgressDlg->SetPos( nChunkNum );

        chunk_info = m_pDoc->ChunkInfo( nChunkNum );
        chunk_type = chunk_info.ChunkType();

        if ( TapeMark == chunk_type )
        {
            // Save the stats for this [physical] file,
            // and retrieve the ones for the next file...

            VERIFY( m_pDoc->SetFileStats( nTapeMarkNum++, file_stats ) );

            if ( m_pDoc->NumFiles() > nTapeMarkNum )
                file_stats = m_pDoc->GetFileStats( nTapeMarkNum );

            // Finish this logical file and start a new one...

            nBlockChunkNum = nChunkNum;

            strFileNum  = _T(  "*******"   );
            strBlockNum = _T( "(tapemark)" );
            strNumBytes = _T(  "*******"   );

            if ( nFileBlockNum )
                nFileNum++;

            nFileBlockNum = 0;
            nFileBlockLen = 0;
        }
        else
        {
            if (!(bDecompressOK = m_pDoc->GetBlockData
            (
                nChunkNum,
                pCompressedData,
                nCompressedLen,
                pExpandedData,
                nExpandedLen
            )))
            {
                CString str; str.Format
                (
                    _T( "Decompression error: rc=%d; chunk# %s, file position: %s (0x%I64X)" )

                    ,g_nCompressDecompressRC
                    ,g_Locale.FormatNumber( nChunkNum )
                    ,g_Locale.FormatNumber( chunk_info.FilePtr().QuadPart )
                    ,                       chunk_info.FilePtr().QuadPart
                );
                TRACE( _T( "** CMyListView::FillListControlWithData: %s\n" ), str );
                nExpandedLen = 0;
            }

            if ( !chunk_info.IsCompressed() )
            {
                ASSERT( bDecompressOK );
                ASSERT( nExpandedLen == chunk_info.CurrChunkLen() );
            }

            switch ( chunk_type )
            {
                case NormalBlock:
                {
                    nBlockChunkNum = nChunkNum;
                    bBlockOK = bDecompressOK;
                    nFileBlockLen = nExpandedLen;
                    nFileBlockNum++;
                    break;
                }

                case BeginningOfBlock:
                {
                    nBlockChunkNum = nChunkNum;
                    bBlockOK = bDecompressOK;
                    nFileBlockLen = nExpandedLen;
                    nFileBlockNum++;
                    continue;
                }

                case IntermediateBlock:
                {
                    bBlockOK = bDecompressOK && bBlockOK;
                    nFileBlockLen += nExpandedLen;
                    continue;
                }

                default:
                case Invalid:
                {
                    CString str; str.Format
                    (
                        _T( "Invalid block flags: chunk# %s, file position: %s (0x%I64X)" )
                        ,g_Locale.FormatNumber( nChunkNum )
                        ,g_Locale.FormatNumber( chunk_info.FilePtr().QuadPart )
                        ,                       chunk_info.FilePtr().QuadPart
                    );
                    TRACE( _T( "** CMyListView::FillListControlWithData: %s\n" ), str );
                    AfxMessageBox( str, MB_ICONWARNING );
                    bBlockOK = false;
                }
                // (purposely fall thru to EndOfBlock case)
                case EndOfBlock:
                {
                    bBlockOK = bDecompressOK && bBlockOK;
                    nFileBlockLen += nExpandedLen;
                    break;
                }
            }

            // Build the list-control strings describing the block...

            strFileNum  . Format( _T( "File: %s"   ), g_Locale.FormatNumber( nFileNum      ) );
            strBlockNum . Format( _T( "Block: %s"  ), g_Locale.FormatNumber( nFileBlockNum ) );

            if ( bBlockOK )
                strNumBytes.Format( _T( "%s bytes" ), g_Locale.FormatNumber( nFileBlockLen ) );
            else
            {
                strNumBytes = _T( "(error)" );
                bBlockErrors = true;
            }

            // Save any Standard Labels we happen to come across...

            if (1
                && nFileBlockNum >= 1
                && nFileBlockNum <= 3
                && NormalBlock == chunk_type
                && 80 == nExpandedLen
            )
                SaveStdLabels( nChunkNum, file_stats, pExpandedData );
        }

        // Update this [physical] file's stats...

        file_stats.m_nTotalDataBytes += nFileBlockLen;

        // Insert the list control item itself...

        VERIFY( ( nItemNum = m_pListCtrl->InsertItem( nChunkNum, strFileNum ) ) != -1 );

        // Define additional columns for the list control item...

        VERIFY( m_pListCtrl->SetItemText( nItemNum, COL_1_BLOCKNUM_COLNUM, strBlockNum ) );
        VERIFY( m_pListCtrl->SetItemText( nItemNum, COL_2_BYTES_COLNUM,    strNumBytes ) );

        // Save the chunk# this list control item is for...

        VERIFY( m_pListCtrl->SetItemData( nItemNum, nBlockChunkNum ) );
    }

    // (re-enable drawing so the list control shows the items we just inserted)

    m_pListCtrl->SetRedraw( TRUE );
    m_pListCtrl->UpdateWindow();

    // Destroy the progress dialog...

    m_pDoc->m_pProgressDlg->DestroyWindow();
    delete m_pDoc->m_pProgressDlg;
    m_pDoc->m_pProgressDlg = NULL;

    // Auto-select the first item...

    SelectItem(0);              // (now select the first item)

    // Issue a warning if there were any errors during processing...

    if ( bBlockErrors )
        AfxMessageBox
        (
            _T( "Warning: Decompression error encountered on one or more blocks." )
        );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::SaveStdLabels( int nChunkNum, CAWSFileStats& file_stats, BYTE* pExpandedData )
{
    // ASCII
    static const BYTE  bVOL1A[4] = {0x56,0x4F,0x4C,0x31};
    static const BYTE  bHDR1A[4] = {0x48,0x44,0x52,0x31};
    static const BYTE  bHDR2A[4] = {0x48,0x44,0x52,0x32};
    static const BYTE  bEOF1A[4] = {0x45,0x4F,0x46,0x31};
    static const BYTE  bEOF2A[4] = {0x45,0x4F,0x46,0x32};
    static const BYTE  bEOV1A[4] = {0x45,0x4F,0x56,0x31};
    static const BYTE  bEOV2A[4] = {0x45,0x4F,0x56,0x32};

    // EBCDIC
    static const BYTE  bVOL1E[4] = {0xE5,0xD6,0xD3,0xF1};
    static const BYTE  bHDR1E[4] = {0xC8,0xC4,0xD9,0xF1};
    static const BYTE  bHDR2E[4] = {0xC8,0xC4,0xD9,0xF2};
    static const BYTE  bEOF1E[4] = {0xC5,0xD6,0xC6,0xF1};
    static const BYTE  bEOF2E[4] = {0xC5,0xD6,0xC6,0xF2};
    static const BYTE  bEOV1E[4] = {0xC5,0xD6,0xE5,0xF1};
    static const BYTE  bEOV2E[4] = {0xC5,0xD6,0xE5,0xF2};

    CString  strStdLabel;   // (work)

    // ASCII Standard Label?
    if (0
        || memcmp( bHDR1A, pExpandedData, 4 ) == 0
        || memcmp( bEOF1A, pExpandedData, 4 ) == 0
        || memcmp( bHDR2A, pExpandedData, 4 ) == 0
        || memcmp( bEOF2A, pExpandedData, 4 ) == 0
        || memcmp( bVOL1A, pExpandedData, 4 ) == 0
        || memcmp( bEOV1A, pExpandedData, 4 ) == 0
        || memcmp( bEOV2A, pExpandedData, 4 ) == 0
    )
    {
        // PROGRAMMING NOTE: we know for a fact that our buffer
        // is longer than 80-bytes!, so the following should be
        // safe to do...
        *(pExpandedData + 80) = 0;
        strStdLabel = (LPSTR) pExpandedData;
    }
    else  // EBCDIC Standard Label?
    if (0
        || memcmp( bHDR1E, pExpandedData, 4 ) == 0
        || memcmp( bEOF1E, pExpandedData, 4 ) == 0
        || memcmp( bHDR2E, pExpandedData, 4 ) == 0
        || memcmp( bEOF2E, pExpandedData, 4 ) == 0
        || memcmp( bVOL1E, pExpandedData, 4 ) == 0
        || memcmp( bEOV1E, pExpandedData, 4 ) == 0
        || memcmp( bEOV2E, pExpandedData, 4 ) == 0
    )
    {
        // Convert to ASCII...

        BYTE  bASCIILabel [80 + 1];

        VERIFY( TranslateString
        (
            bASCIILabel,   ASCII_CODE_PAGE,     // (o/p buffer & code page)
            pExpandedData, EBCDIC_CODE_PAGE,    // (i/p buffer & code page)
            80                                  // (length in bytes)
        ));

        *(bASCIILabel + 80) = 0;        // (NULL terminate string)
        strStdLabel = (LPSTR) bASCIILabel;
    }
    else
        return; // (not anything we're interested in)

    VERIFY( strStdLabel.Trim().GetLength() >= 4 );

    // Save Standard Label...

    if ( strStdLabel[0] == _T('V') )
    {
        m_pDoc->m_strVol1Label = strStdLabel;
    }
    else if ( strStdLabel[3] == _T('1') )
    {
        file_stats.m_strStdLabel1 = strStdLabel;

        if ( strStdLabel.Left(3) == _T("HDR") )
        {
            m_arHDR1Names.Add( strStdLabel.Mid(4,17).Trim() );
            m_arHDR1ChunkNums.Add( nChunkNum );
        }
    }
    else if ( strStdLabel[3] == _T('2') )
    {
        file_stats.m_strStdLabel2 = strStdLabel;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// They left/right clicked (or left/right double-clicked) on an item...

void CMyListView::SelectItem( int nItemNum )
{
    UINT  nState  = LVIS_SELECTED | LVIS_FOCUSED;
    UINT  nMask   = LVIS_SELECTED | LVIS_FOCUSED;

    // (Note: should cause below 'OnItemStateChange' to be called too)

    m_pListCtrl->SetItemState( nItemNum, nState, nMask );
    m_pListCtrl->EnsureVisible( nItemNum, FALSE );
    m_pListCtrl->UpdateWindow();

    // Keep our variables current...

    m_nSelectedItemNum   =  nItemNum;
    m_nSelectedChunkNum  =  (int)m_pListCtrl->GetItemData( nItemNum );
}

//////////////////////////////////////////////////////////////////////////////////////////
// They selected a new (different) item...  (called by the above function
// whenever they left/right click or left/right double-click on an item...)

void CMyListView::OnItemStateChange( NMHDR* pNotifyStruct, LRESULT* pResult )
{
    NMITEMACTIVATE*  pItemActivate   = (NMITEMACTIVATE*) pNotifyStruct;

    // If the document is empty,
    // or an item isn't being selected,
    // then ignore the event...

    int nItemNum = pItemActivate->iItem;

    if (0
        || m_pDoc->IsEmpty()
        || !( pItemActivate->uNewState & LVIS_SELECTED )
        || nItemNum < 0
    )
    {
        return;     // (not a notification we're interested in)
    }

    // Keep our variables current...

    m_nSelectedItemNum   =  nItemNum;
    m_nSelectedChunkNum  =  (int)m_pListCtrl->GetItemData( m_nSelectedItemNum );

    DisplaySelectedItem();      // (notify hexedit control)
}

//////////////////////////////////////////////////////////////////////////////////////////
// Tell the hex-edit view the chunk# of the data block that was just selected
// so it can load it into its hexedit control and display it for the user...

void CMyListView::DisplaySelectedItem()
{
    if (m_pDoc->m_pHexEditView)
        m_pDoc->m_pHexEditView->SendMessage( MY_WM_REFRESH_HEXEDIT_VIEW, m_nSelectedChunkNum, 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////
// The "Find" menu item was selected. Display the Find/Replace dialog box...

void CMyListView::OnEditFind()
{
    CString strTemp;

    UpdateSearchString();   // (retrieve current/previous search string value)

    // Initialize and display the "Find" dialog
    // and then retrieve the search parameters...

    {
        CMyFindDialog  dlg( this );

        // Remove old list of search strings...

        dlg.m_FindWhatList.RemoveAll();

        // Populate dialog with new (current) list...
        // (this also removes redundant/identical entries)

        if ( !m_strSearchString.IsEmpty() )
            dlg.m_FindWhatList.AddHead( m_strSearchString );

        POSITION pos = m_FindWhatList.GetHeadPosition();

        while ( pos && m_FindWhatList.GetCount() < MAX_FIND_MRU_ENTRIES )
        {
            strTemp = m_FindWhatList.GetNext( pos );

            if ( !strTemp.IsEmpty() && strTemp.Compare( m_strSearchString ) != 0 )
                dlg.m_FindWhatList.AddTail( strTemp );
        }

        // Set default search direction...

        if ( !m_nSelectedItemNum && m_nBlockPos < 0 )
        {
            // (already at beg-of-file; backward search
            //  not possible; default to forward search)

            m_bForwSearch = TRUE;  // (sensible default)
        }
        else if ( m_nSelectedItemNum >= ( m_pListCtrl->GetItemCount() - 1 )
            && m_nBlockPos >= m_nBlockLen )
        {
            // (already at end-of-file; forward search
            //  not possible; default to backward search)

            m_bForwSearch = FALSE;  // (sensible default)
        }

        // Fix our "do-hex-search" flag if we haven't yet...

        BOOL bIsHexView  = m_pDoc->m_pHexEditView->IsHexView();
        m_bHexSearch     = bIsHexView ? m_bHexSearch : FALSE;

        // Pass values to dialog...

        dlg.m_bWholeWord = m_bWholeWord;
        dlg.m_bMatchCase = m_bMatchCase;
        dlg.m_bHex       = m_bHexSearch;
        dlg.m_bIsHexView = bIsHexView;
        dlg.m_nWhere     = m_nSearchWhere; // (FIND_INBLOCK, FIND_INFILE, FIND_ONTAPE)
        dlg.m_nDirection = m_bForwSearch ? FIND_DOWN : FIND_UP;

        // Display dialog, retrieve results...

        if ( IDOK != dlg.DoModal() )
            return;

        m_bWholeWord   = dlg.m_bWholeWord;
        m_bMatchCase   = dlg.m_bMatchCase;
        m_bHexSearch   = dlg.m_bHex;
        m_nSearchWhere = dlg.m_nWhere;

        if ( m_bHexSearch && !bIsHexView )
        {
            ASSERT( false );        // (CMyFindDialog fucked up!)
            m_bHexSearch = FALSE;   // (fix it)
        }

        m_pDoc->m_pHexEditView->SetTextArea( !m_bHexSearch );

        // (update our saved list of search strings)

        m_FindWhatList.RemoveAll();
        m_FindWhatList.AddTail( &dlg.m_FindWhatList );

        while ( m_FindWhatList.GetCount() > MAX_FIND_MRU_ENTRIES )
            m_FindWhatList.RemoveTail();

        strTemp        =  m_FindWhatList.GetHead();
        m_bForwSearch  =  FIND_DOWN == dlg.m_nDirection ? TRUE : FALSE;
    }

    // Now perform the requested search...

    // (we must have a search string to work with,
    // otherwise why are we trying to do a search?!)

    if ( strTemp.IsEmpty() )
        return;

    // Validate their new search string....

    if ( m_pDoc->m_pHexEditView->IsTextArea() )
    {
        // They're in the text area. The search string they enter should
        // be a regular text string (which can be pretty much anything at
        // all). If it looks like it's a valid hexadecimal string though,
        // then presume they're actually entering a hex search string so
        // switch the the hex edit control over to the hex area so that
        // it matches the format of the type of search string we presume
        // they're entering...

        if (1
            && m_pDoc->m_pHexEditView->IsHexView()  // (hex area must be visible to switch to it!)
            && ValidHexSearchString( strTemp )
        )
        {
            // Looks like they did indeed enter a hex search string,
            // so switch over to the hex area so that it matches the
            // format of the search string we presume they've entered.

            m_pDoc->m_pHexEditView->SetTextArea( FALSE );
            ( (CFrameWnd*) AfxGetMainWnd() )->SetActiveView( m_pDoc->m_pHexEditView );
        }
    }
    else
    {
        ASSERT( m_pDoc->m_pHexEditView->IsHexView() ); // (if they're positioned in the hex area it better be visible!!)

        // They're positioned in the hex area, so the search string
        // SHOULD be a hex string. If it's not however, then accept
        // their text search string and switch the hexedit control
        // over to the text area to match the format of the search
        // string they entered...

        if (0
            || !m_pDoc->m_pHexEditView->IsHexView() // (MUST switch to text area if hex area not visible!)
            || !ValidHexSearchString( strTemp )
        )
        {
            // They're in the hex area but they didn't enter a hex
            // search string (they entered a text search string).
            // Switch the hexedit control over to the text area so
            // that the search string matches the area they're in...

            m_pDoc->m_pHexEditView->SetTextArea( TRUE );
            ( (CFrameWnd*) AfxGetMainWnd() )->SetActiveView( m_pDoc->m_pHexEditView );
        }
    }

    // We have a valid search string that matches the area they're in.
    // We can now safely proceed to perform their requested search...

    m_strSearchString = strTemp;    // (what they wish to find)
    UpdateSearchParms();            // (binary data search parms)

    // Perform the requested search...

    if ( m_bForwSearch )
        FindNext();                 // (FindNext = forward search)
    else
        FindPrev();                 // (FindPrev = backward search)
}

//////////////////////////////////////////////////////////////////////////////////////////
// This function is only ever called by the OnEditFind() function whenever they start
// a brand new search before the search dialog is displayed (so the m_strSearchString
// value that gets displayed in the dialog is "current" whenever the Find dialog is
// displayed). It update the 'm_strSearchString' value to match whatever data they
// currently have selected in the hexedit control (if anything at all). If nothing
// is selected, then nothing is changed. Updates the following variable as appropriate:
//
//   m_strSearchString;    // string to find (always ASCII, and also ALWAYS
//                         // matches the format of the edit area we're in)

void CMyListView::UpdateSearchString()
{
    // Retrieve a copy of whatever data they currently have selected
    // in the hexedit control (if anything is selected at all)...

    BYTE*    pSelectedData     = NULL;
    SSIZE_T  dwSelectedDataLen = 0;

    // CAUTION! The following function does a 'new'. Caller is
    // responsible for doing the 'delete' to prevent memory leak!

    if ( m_pDoc->m_pHexEditView->GetSelectedData( pSelectedData, dwSelectedDataLen ) )
    {
        // They currently [still] have something selected. Build a new search
        // string based on whatever data they currently [still] have selected
        // according to whatever edit area (hex/text) they're curently in...

        ASSERT( pSelectedData && dwSelectedDataLen );

        if ( m_pDoc->m_pHexEditView->IsTextArea() )
            m_strSearchString = GetBlockDataAsCString( pSelectedData, dwSelectedDataLen,
                m_pDoc->m_pHexEditView->IsEBCDICMode() );
        else
            m_strSearchString = BuildHexSearchString ( pSelectedData, dwSelectedDataLen );

        delete [] pSelectedData;    // (prevent memory leak)

        // (discard the existing search parms)

        m_strPrevSearchString.Empty();
        delete [] m_pSearchData;
        m_pSearchData = NULL;
        m_nSearchDataLen = 0;

        // (build new ASCII/EBCDIC search parms based on the new
        // m_strSearchString value according to the area they're in)

        UpdateSearchParms();    // (update binary data search parms)
        return;
    }

    // Nothing is selected, but our current binary data search parms could
    // still be obsolete since they could have either switched edit areas
    // (from the text area to hex area or vice-versa) or switched ASCII/EBCDIC
    // modes. Thus, to be safe, we will discard our current search string
    // (and associated binary data search parms) and build a new one (and
    // associated binary data search parms) based on current values...

    if ( !m_strSearchString.IsEmpty() )
    {
        if ( !m_strPrevSearchString.IsEmpty() )
        {
            ASSERT( m_pSearchData && m_nSearchDataLen );

            if ( m_pDoc->m_pHexEditView->IsTextArea() )
                m_strSearchString = GetBlockDataAsCString( m_pSearchData, m_nSearchDataLen,
                    m_pDoc->m_pHexEditView->IsEBCDICMode() );
            else
                m_strSearchString = BuildHexSearchString ( m_pSearchData, m_nSearchDataLen );

            // (force new binary data search parms to be built)

            m_strPrevSearchString.Empty();
            delete [] m_pSearchData;
            m_pSearchData = NULL;
            m_nSearchDataLen = 0;
        }
    }

    // (build new ASCII/EBCDIC search parms based on the new
    // m_strSearchString value according to the area they're in)

    UpdateSearchParms();
}

//////////////////////////////////////////////////////////////////////////////////////////

CString CMyListView::BuildHexSearchString( BYTE* pData, SSIZE_T nDataLen )
{
    CString strHexString, strHexByte;

    for ( SSIZE_T i=0; i < ( MAX_HEX_SEARCH_STRING_LEN / 2 ) && i < nDataLen; i++ )
    {
        strHexByte.Format( _T( "%02.2X" ), *pData++ );
        strHexString += strHexByte;
    }

    return strHexString;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Update our search parms based on a new m_strSearchString value.
// Updates the following variables as appropriate:
//
//   m_strPrevSearchString;         // (previous value)
//   m_bPrevEBCDICMode;             // (previous value)
//   m_bPrevTextArea;               // (previous value)
//
//   m_pSearchData;                 // ASCII/EBCDIC search data  (NOTE: always SBCS!)
//   m_nSearchDataLen;              // length of search data     (NOTE: always SBCS!)
//
//   m_nForwShiftTable[256];        // Pratt-Boyer-Moore shift table
//   m_nBackShiftTable[256];        // Pratt-Boyer-Moore shift table

void CMyListView::UpdateSearchParms()
{
    // Check if the parms we may already still have are still good or not...

    if (0
        || m_strSearchString.IsEmpty()      // (a search string is required!)
        || (1
            &&  m_pSearchData
            &&  m_nSearchDataLen
            &&  m_strPrevSearchString == m_strSearchString
            &&  m_bPrevTextArea       == m_pDoc->m_pHexEditView->IsTextArea()
            &&  m_bPrevEBCDICMode     == m_pDoc->m_pHexEditView->IsEBCDICMode()
           )
    )
        return;  // (no search string given or existing parms are still good)

    // Our search parms are out of date; update them...

    ASSERT(!m_strSearchString.IsEmpty());   // (a search string is required!)

    delete [] m_pSearchData;
    m_pSearchData = NULL;
    m_nSearchDataLen = 0;

    m_strPrevSearchString = m_strSearchString;
    m_bPrevTextArea       = m_pDoc->m_pHexEditView->IsTextArea();
    m_bPrevEBCDICMode     = m_pDoc->m_pHexEditView->IsEBCDICMode();

    if ( m_pDoc->m_pHexEditView->IsTextArea() )
    {
        // We're in the text area, so the search string is a text search string...

        USES_CONVERSION;

        m_nSearchDataLen = m_strSearchString.GetLength();
        m_pSearchData = new BYTE [ m_nSearchDataLen ]; ASSERT( m_pSearchData );
        memcpy( m_pSearchData, T2CA((LPCTSTR)m_strSearchString), m_nSearchDataLen );

        if ( m_pDoc->m_pHexEditView->IsEBCDICMode() )
        {
            VERIFY
            (
                // PROGRAMMING NOTE: the FishLib 'TranslateString' function
                // works with single-byte character string data and NOT UNICODE.

                TranslateString
                (
                    m_pSearchData, EBCDIC_CODE_PAGE,    // to
                    m_pSearchData, ASCII_CODE_PAGE,     // from
                    (int)m_nSearchDataLen
                )
            );
        }
    }
    else
    {
        // We're in the hex area, so the search string is a hex search string...

        ASSERT( ValidHexSearchString( m_strSearchString ) );

        CString strUpper = m_strSearchString; // (working copy)
        strUpper.MakeUpper();                 // (upper case)

        m_nSearchDataLen = strUpper.GetLength() / 2; ASSERT( m_nSearchDataLen );
        m_pSearchData = new BYTE [ m_nSearchDataLen ]; ASSERT( m_pSearchData );

        BYTE*  pByte = m_pSearchData;
        BYTE   b1, b2;
        TCHAR  ch;

        for ( SIZE_T i=0; i < m_nSearchDataLen; i++ )
        {
            ch = strUpper.GetAt((int)(i*2));
            {
                if ( ch <= _T( '9' ) ) ch =  0 + ( ch - _T( '0' ) );
                else                   ch = 10 + ( ch - _T( 'A' ) );
            }
            b1 = ( ( ch << 4 ) & 0xF0 );

            ch = strUpper.GetAt((int)((i*2) + 1));
            {
                if ( ch <= _T( '9' ) ) ch =  0 + ( ch - _T( '0' ) );
                else                   ch = 10 + ( ch - _T( 'A' ) );
            }
            b2 = ( ch & 0x0F );

            *pByte++ = b1 | b2;
        }
    }

    InitQuickForwardMemSearch
    (
        m_nForwShiftTable,
        m_pSearchData,
        m_nSearchDataLen
    );

    InitQuickBackwardMemSearch
    (
        m_nBackShiftTable,
        m_pSearchData,
        m_nSearchDataLen
    );
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//   MEANING AND INTERPRETATION OF START AND END OF "SELECTION"...
//
//
//   No active selection:
//
//      +---+---+---+---+---+---+---+---+---+---+
//      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |...
//      +---+---+---+---+---+---+---+---+---+---+
//              A
//              |
//              +---- Cursor position = 2
//
//            Selection start = 2
//            Selection end   = 2
//
//
//
//   FORWARD selection:
//
//      +---+---+---+---+---+---+---+---+---+---+
//      | 0 | 1 | SSSSSSSSS | 5 | 6 | 7 | 8 | 9 |...
//      +---+---+---+---+---+---+---+---+---+---+
//                          A
//                          |
//                          +---- Cursor position = 5
//
//            Selection start = 2
//            Selection end   = 5
//
//
//
//   BACKWARD selection:
//
//      +---+---+---+---+---+---+---+---+---+---+
//      | 0 | 1 | SSSSSSSSS | 5 | 6 | 7 | 8 | 9 |...
//      +---+---+---+---+---+---+---+---+---+---+
//              A
//              |
//              +---- Cursor position = 2
//
//            Selection start = 4
//            Selection end   = 1
//
//
//////////////////////////////////////////////////////////////////////////////////////////
// They pressed 'F3' to find the NEXT occurrence
// of the previously entered search string...

bool CMyListView::FindNext()
{
    m_bForwSearch = TRUE;   // (keep this up to date)

    LoadItemData( m_nSelectedItemNum );

    // FORWARD search: start the search at either the current position,
    // or, if there's a selection,
    // at the first byte of the selected data + 1.

    int nSelStart, nSelEnd;

    m_pDoc->m_pHexEditView->GetHexEditCtrlPtr()->GetSel( nSelStart, nSelEnd );

    // NOTE: a starting search block position of -1 (minus 1) IS
    // valid, and simply causes the search to start at either the
    // end of the previous block (if there is one) for backward
    // searches, or at the start of the current block (byte 0)
    // for forward searches. The same concept applies for the case
    // of a start position that is [one byte] past the end of the
    // block too: the search simply starts on the very first byte
    // (byte 0) of the next block (if it exists) for forward searches,
    // or on the very last byte of the current block for backward
    // searches...

    if ( nSelStart == nSelEnd )
        // (no selection)
        m_nBlockPos = nSelStart;
    else
    {
        if ( nSelStart < nSelEnd )
            // (forward selection)
            m_nBlockPos = nSelStart + 1;
        else
            // (backward selection)
            m_nBlockPos = nSelEnd + 2;
    }

    return DoSearch();
}

//////////////////////////////////////////////////////////////////////////////////////////
// They pressed 'Shift+F3' to find the PREVIOUS occurrence
// of the previously entered search string...

bool CMyListView::FindPrev()
{
    m_bForwSearch = FALSE;  // (keep this up to date)

    LoadItemData( m_nSelectedItemNum );

    // BACKWARD search: start the search at either the current position - 1,
    // or, if there's a selection,
    // at the first byte of the selected data - 1.

    int nSelStart, nSelEnd;

    m_pDoc->m_pHexEditView->GetHexEditCtrlPtr()->GetSel( nSelStart, nSelEnd );

    // NOTE: a starting search block position of -1 (minus 1) IS
    // valid, and simply causes the search to start at either the
    // end of the previous block (if there is one) for backward
    // searches, or at the start of the current block (byte 0)
    // for forward searches. The same concept applies for the case
    // of a start position that is [one byte] past the end of the
    // block too: the search simply starts on the very first byte
    // (byte 0) of the next block (if it exists) for forward searches,
    // or on the very last byte of the current block for backward
    // searches...

    if ( nSelStart == nSelEnd )
        // (no selection)
        m_nBlockPos = nSelStart - 1;
    else
    {
        if ( nSelStart < nSelEnd )
            // (forward selection)
            m_nBlockPos = nSelStart - 1;
        else
            // (backward selection)
            m_nBlockPos = nSelEnd;
    }

    return DoSearch();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Perform the requested search...

BYTE* CMyListView::DoTextSearch()
{
    // NOTE: a starting search block position of -1 (minus 1) IS
    // valid, and simply causes the search to start at either the
    // end of the previous block (if there is one) for backward
    // searches, or at the start of the current block (byte 0)
    // for forward searches. The same concept applies for the case
    // of a start position that is [one byte] past the end of the
    // block too: the search simply starts on the very first byte
    // (byte 0) of the next block (if it exists) for forward searches,
    // or on the very last byte of the current block for backward
    // searches. Thus, if the start position is either -1 or past
    // the end of the current block, we simply return an immediate
    // 'not found' condition and the DoSearch simply transparently
    // moves on to the next/previous block...

    SSIZE_T nStartPos = m_nBlockPos;

    if (0
        || !m_nBlockLen                // (tapemark block)
        || ( nStartPos <       0      && !m_bForwSearch )
        || ( nStartPos >= m_nBlockLen &&  m_bForwSearch )
        || m_strSearchString.IsEmpty()  // (empty search arg)
    )
        return NULL;

    if ( nStartPos < 0 )
    {
        ASSERT( m_bForwSearch );
        nStartPos = 0;
    }
    else if ( nStartPos >= m_nBlockLen )
    {
        ASSERT( !m_bForwSearch );
        nStartPos = ( m_nBlockLen - 1 );
    }

    ASSERT( nStartPos >= 0 && nStartPos <= ( m_nBlockLen - 1 ) );

    CString strFindWhat = m_strSearchString;

    if ( !m_bMatchCase )
        strFindWhat.MakeLower();

    // Convert block data to CString...

    CString  strDataBlockAsCString = GetBlockDataAsCString
    (
        m_pDataBlock,
        m_nBlockLen,
        m_pDoc->m_pHexEditView->IsEBCDICMode()
    );

    // Convert to lowercase if case isn't important...

    if ( !m_bMatchCase )
        strDataBlockAsCString.MakeLower();

    // Reverse the two strings if we're doing a backward search...

    if ( !m_bForwSearch )
    {
        strFindWhat           . MakeReverse();
        strDataBlockAsCString . MakeReverse();

        // (and adjust the start position appropriately...)

        nStartPos = ( m_nBlockLen - 1 ) - nStartPos;
    }

    ASSERT( nStartPos >= 0 && nStartPos <= ( m_nBlockLen - 1 ) );

    // Here's where we actually do the search...

    SSIZE_T nFoundPos = strDataBlockAsCString.Find( strFindWhat, (int)nStartPos );

    // Return ptr to where string was found...

    if ( nFoundPos < 0 ) return NULL;  // (not found)

    // (handle "whole word only" searches)

    if ( m_bWholeWord )
    {
        SSIZE_T nCharBeforeMatchPos  =  nFoundPos  -             1;
        SSIZE_T nCharAfterMatchPos   =  nFoundPos  +  strFindWhat.GetLength();

        TCHAR chCharBeforeMatch = nCharBeforeMatchPos >=                    0             ? strDataBlockAsCString[ (int)nCharBeforeMatchPos ] : _T( ' ' );
        TCHAR chCharAfterMatch  = nCharAfterMatchPos  < strDataBlockAsCString.GetLength() ? strDataBlockAsCString[ (int)nCharAfterMatchPos  ] : _T( ' ' );

        if (0
            || IsWordChar( chCharBeforeMatch )
            || IsWordChar( chCharAfterMatch  )
        )
            return NULL; // (not a whole word)
    }

    // (fix found position if needed)

    if ( !m_bForwSearch )
        nFoundPos = ( m_nBlockLen - 1 ) - ( nFoundPos + ( strFindWhat.GetLength() - 1 ) );

    ASSERT( nFoundPos >= 0 && nFoundPos <= ( m_nBlockLen - 1 ) );

    return ( m_pDataBlock + nFoundPos );
}

//////////////////////////////////////////////////////////////////////////////////////////

bool CMyListView::DoSearch()
{
    int nItemNum = m_nSelectedItemNum;  // (starting item#)

    // Load the starting block's data (if needed)...

    if ( !m_pDataBlock )
    {
        if ( !LoadItemData( nItemNum ) && !( m_pDataBlock && m_nBlockLen ) )
        {
            // Probably a tapemark; skip past it...

            if ( ( m_nBlockPos = GetNextSearchBlock( nItemNum ) ) < 0 )
                return NotFound();
        }
    }

    ASSERT( m_pDataBlock  && m_nBlockLen      );
    ASSERT( m_pSearchData && m_nSearchDataLen );

    BYTE* pFoundLoc = NULL;
    {
        CWaitCursor  wait;  // (this may take a while)

        do
        {
            // NOTE: a starting search block position of -1 (minus 1) IS
            // valid, and simply causes the search to start at either the
            // end of the previous block (if there is one) for backward
            // searches, or at the start of the current block (byte 0)
            // for forward searches. The same concept applies for the case
            // of a start position that is [one byte] past the end of the
            // block too: the search simply starts on the very first byte
            // (byte 0) of the next block (if it exists) for forward searches,
            // or on the very last byte of the current block for backward
            // searches. Thus, if the start position is either -1 or past
            // the end of the current block, the search functions simply
            // return an immediate 'not found' and we simply transparently
            // move on to the next/previous block...

            if ( m_pDoc->m_pHexEditView->IsTextArea() )
                pFoundLoc = DoTextSearch();
            else
            {
                if ( m_bForwSearch )
                {
                    pFoundLoc = (BYTE*) QuickForwardMemSearch
                    (
                        m_pDataBlock,  m_nBlockLen,
                        m_pSearchData, m_nSearchDataLen,
                        m_pDataBlock + m_nBlockPos,
                        m_nForwShiftTable
                    );
                }
                else
                {
                    pFoundLoc = (BYTE*) QuickBackwardMemSearch
                    (
                        m_pDataBlock,  m_nBlockLen,
                        m_pSearchData, m_nSearchDataLen,
                        m_pDataBlock + m_nBlockPos,
                        m_nBackShiftTable
                    );
                }
            }
        }
        while ( !pFoundLoc && ( m_nBlockPos = GetNextSearchBlock( nItemNum ) ) >= 0 );
    }

    if ( !pFoundLoc )
        return NotFound();

    ASSERT( pFoundLoc >= m_pDataBlock && pFoundLoc < ( m_pDataBlock + m_nBlockLen ) );

    // We found what we were looking for. Select the list control item
    // the data was found in (which should cause the hexedit control to
    // load and display the data to the user as well) and then afterwards,
    // ask the hexedit control to select the data we found so that they
    // can see exactly where within the data block it was found at...

    SelectItem( nItemNum );     // (show them the data block)

    // Auto-select the found data...

    SSIZE_T dwFoundDisp = (pFoundLoc - m_pDataBlock);
    ASSERT( dwFoundDisp < m_nBlockLen );

    SSIZE_T nSelStart, nSelEnd;

    if ( m_bForwSearch || !dwFoundDisp )
    {
        nSelStart = dwFoundDisp;
        nSelEnd   = dwFoundDisp + m_nSearchDataLen;
    }
    else
    {
        nSelStart = dwFoundDisp + m_nSearchDataLen;
        nSelEnd   = dwFoundDisp;

        nSelStart--;
        nSelEnd--;
    }

    VERIFY( m_pDoc->m_pHexEditView->GetHexEditCtrlPtr()->SetSelEx( nSelStart, nSelEnd, m_pDoc->m_pHexEditView->IsTextArea() ) );
    ( (CFrameWnd*) AfxGetMainWnd() )->SetActiveView( m_pDoc->m_pHexEditView );

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool CMyListView::NotFound()    // (issues "Not Found" message)
{
    AfxMessageBox( _T( "Not found" ) );
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// (DoSearch helper: returns new block position or -1 @ EOF)

SSIZE_T CMyListView::GetNextSearchBlock( int& nItemNum )
{
    // Return EOF if we're only supposed to search within the current block.

    if ( FIND_INBLOCK == m_nSearchWhere )
        return -1;

    // Return EOF if we've reached the end or beginning of the tape.

    if (0
        || (  m_bForwSearch  &&  ++nItemNum  >=  m_pListCtrl->GetItemCount() )
        || ( !m_bForwSearch  &&  --nItemNum  <               0               )
    )
        return -1;

    // Skip past tapemark chunks to next/previous data block...
    // (unless we're only searching within a single file of course)

    int            nChunkNum  = (int)m_pListCtrl->GetItemData( nItemNum );
    EAWSChunkType  chunk_type = m_pDoc->ChunkInfo( nChunkNum ).ChunkType();

    while
    (1
        && NormalBlock      != chunk_type
        && BeginningOfBlock != chunk_type
    )
    {
        // Return EOF if we've reached the end or beginning of the file.

        if (1
            &&  FIND_INFILE  ==  m_nSearchWhere
            &&  TapeMark     ==  chunk_type
        )
            return -1;

        // Else continue looking for the beginning of the next/previous block.

        if (0
            || (  m_bForwSearch  &&  ++nItemNum  >=  m_pListCtrl->GetItemCount() )
            || ( !m_bForwSearch  &&  --nItemNum  <               0               )
        )
            return -1;

        nChunkNum  = (int)m_pListCtrl->GetItemData( nItemNum );
        chunk_type = m_pDoc->ChunkInfo( nChunkNum ).ChunkType();
    }

    if ( !LoadItemData( nItemNum ) && !( m_pDataBlock && m_nBlockLen ) )
    {
        ASSERT( false );    // (something is VERY wrong!)
        return -1;
    }

    return m_bForwSearch ? 0 : m_nBlockLen;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Load the requested data block data...
// Updates the following variables as appropriate:
//
//   m_pDataBlock;      // ptr --> current data block
//   m_nBlockLen;       // length of current data block

bool CMyListView::LoadItemData( int nItemNum )
{
    ASSERT( nItemNum >= 0 && nItemNum < m_pListCtrl->GetItemCount() );
    int nChunkNum = (int)m_pListCtrl->GetItemData( nItemNum );
    ASSERT( nChunkNum >= 0 );

    delete [] m_pDataBlock;
    m_pDataBlock = NULL;
    m_nBlockLen  = 0;

    BYTE*  pCompressedData;
    int    nCompressedLen;
    BYTE*  pExpandedData;
    int    nExpandedLen;

    bool bSuccess = true;   // (let's be optimistic here)

    if ( !m_pDoc->GetBlockData
    (
        nChunkNum,
        pCompressedData,
        nCompressedLen,
        pExpandedData,
        nExpandedLen
    ))
    {
        bSuccess = false;   // (forced because of error)

        if ( !( pCompressedData && nCompressedLen ) )
        {
            // Tapemark (0-length block) or other error
            return false;
        }

        // Decompression error; use compressed values

        ASSERT( !pExpandedData   && !nExpandedLen   );  // (sanity check)
        ASSERT(  pCompressedData &&  nCompressedLen );  // (sanity check)

        pExpandedData = pCompressedData;
        nExpandedLen  = nCompressedLen;
    }

    ASSERT( pExpandedData && nExpandedLen > 0 );
    m_nBlockLen = nExpandedLen;
    m_pDataBlock = new BYTE [ m_nBlockLen ]; ASSERT( m_pDataBlock );
    memcpy( m_pDataBlock, pExpandedData, m_nBlockLen );

    return bSuccess;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnDraw( CDC* pDC )
{
    //  PROGRAMMING NOTE: no drawing code is actually needed
    //  here (since we let the list control itself do all of
    //  our drawing), BUT...

    //  The OnDraw function is nonetheless still required
    //  for Views so while there's nothing for us to do here,
    //  we still need to keep this function here and simply
    //  not do anything. It's dumb, I know, but there you
    //  have it... <shrug>
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnFilePageSetup()
{
    DoPageSetup();      // (set page margins, etc)
}

//////////////////////////////////////////////////////////////////////////////////////////
// (set page margins, etc)

bool CMyListView::DoPageSetup()
{
    CPageSetupDialog  dlg( PSD_MARGINS | PSD_MINMARGINS | PSD_INTHOUSANDTHSOFINCHES, this );

    CalcPrintRect();    // (also initializes the minimum margin values)

    dlg.m_psd.rtMinMargin.left   = m_nMinLeftMargin;
    dlg.m_psd.rtMinMargin.right  = m_nMinRightMargin;
    dlg.m_psd.rtMinMargin.top    = m_nMinTopMargin;
    dlg.m_psd.rtMinMargin.bottom = m_nMinBottomMargin;

    // Set the default margins...

    if ( m_nLeftMargin < dlg.m_psd.rtMinMargin.left )
    {
        dlg.m_psd.rtMargin = dlg.m_psd.rtMinMargin; // (too small; use minimum)
    }
    else // (use the values they specified)
    {
        dlg.m_psd.rtMargin.left   = m_nLeftMargin;
        dlg.m_psd.rtMargin.right  = m_nRightMargin;
        dlg.m_psd.rtMargin.top    = m_nTopMargin;
        dlg.m_psd.rtMargin.bottom = m_nBottomMargin;
    }

    dlg.m_psd.hDevMode  = g_App.GetMFCsDefaultPrinterDEVMODEHandle();
    dlg.m_psd.hDevNames = g_App.GetMFCsDefaultPrinterDEVNAMESHandle();

    if ( IDOK != dlg.DoModal() )
        return false; // (they changed their mind)

    // (tell MFC to use the printer they specified)

    g_App.SelectPrinter( dlg.m_psd.hDevNames, dlg.m_psd.hDevMode );

    // Save the new margin values... (Note that the minimum margin values for the
    // selected printer may have changed if they specified a different printer)...

    m_nMinLeftMargin    =        dlg.m_psd.rtMinMargin . left;
    m_nMinRightMargin   =        dlg.m_psd.rtMinMargin . right;
    m_nMinTopMargin     =        dlg.m_psd.rtMinMargin . top;
    m_nMinBottomMargin  =        dlg.m_psd.rtMinMargin . bottom;

    m_nLeftMargin       =  max ( dlg.m_psd.rtMargin    . left,   m_nMinLeftMargin   );
    m_nRightMargin      =  max ( dlg.m_psd.rtMargin    . right,  m_nMinRightMargin  );
    m_nTopMargin        =  max ( dlg.m_psd.rtMargin    . top,    m_nMinTopMargin    );
    m_nBottomMargin     =  max ( dlg.m_psd.rtMargin    . bottom, m_nMinBottomMargin );

    CSize szPaper = dlg.GetPaperSize();

    m_nLeftMargin       =  min ( m_nLeftMargin,     szPaper.cx                   );
    m_nRightMargin      =  min ( m_nRightMargin,  ( szPaper.cx - m_nLeftMargin ) );
    m_nTopMargin        =  min ( m_nTopMargin,      szPaper.cy                   );
    m_nBottomMargin     =  min ( m_nBottomMargin, ( szPaper.cy - m_nTopMargin  ) );

    SaveProfileSettings();

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnFilePrintPreview()
{
    if ( m_nLeftMargin < 0 )
        if ( !DoPageSetup() )
            return;

    CListView::OnFilePrintPreview();
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnFilePrint()
{
    if ( m_nLeftMargin < 0 )
        if ( !DoPageSetup() )
            return;

    CListView::OnFilePrint();
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//                            OnPreparePrinting
//
// You must override this function to enable printing and print preview. Call the
// DoPreparePrinting member function, passing it the pInfo parameter, and then return
// its return value; DoPreparePrinting displays the Print dialog box and creates a
// printer device context. If you want to initialize the Print dialog box with values
// other than the defaults, assign values to the members of pInfo. For example, if
// you know the length of the document, pass the value to the SetMaxPage member function
// of pInfo before calling DoPreparePrinting. This value is displayed in the To: box
// in the Range portion of the Print dialog box.
//
// DoPreparePrinting does not display the Print dialog box for a preview job. If you
// want to bypass the Print dialog box for a print job, check that the m_bPreview member
// of pInfo is FALSE and then set it to TRUE before passing it to DoPreparePrinting;
// reset it to FALSE afterwards
//
// If you need to perform initializations that require access to the CDC object
// representing the printer device context (for example, if you need to know the page
// size before specifying the length of the document), override the OnBeginPrinting
// member function.
//
// If you want to set the value of the m_nNumPreviewPages or m_strPageDesc members of
// the pInfo parameter, do so after calling DoPreparePrinting. The DoPreparePrinting
// member function sets m_nNumPreviewPages to the value found in the application's .INI
// file and sets m_strPageDesc to its default value.

BOOL CMyListView::OnPreparePrinting( CPrintInfo* pInfo )
{
    // Load the block to be printed...

    if ( !LoadItemData( m_nSelectedItemNum ) && !( m_pDataBlock && m_nBlockLen ) )
        return FALSE;       // (can't print tapemarks)

    return DoPreparePrinting( pInfo );
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//                              OnBeginPrinting
//
// Called by the framework at the beginning of a print or print preview job, after
// OnPreparePrinting has been called. The default implementation of this function does
// nothing. Override this function to allocate any GDI resources, such as pens or fonts,
// needed specifically for printing. Select the GDI objects into the device context
// from within the OnPrint member function for each page that uses them. If you are using
// the same view object to perform both screen display and printing, use separate
// variables for the GDI resources needed for each display; this allows you to update
// the screen during printing.
//
// You can also use this function to perform initializations that depend on properties
// of the printer device context. For example, the number of pages needed to print the
// document may depend on settings that the user specified from the Print dialog box
// (such as page length). In such a situation, you cannot specify the document length
// in the OnPreparePrinting member function, where you would normally do so; you must
// wait until the printer device context has been created based on the dialog box settings.
// OnBeginPrinting is the first overridable function that gives you access to the CDC
// object representing the printer device context, so you can set the document length
// from this function. Note that if the document length is not specified by this time,
// a scroll bar is not displayed during print preview.
//
//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo )
{
    // Load block to be printed...

    if ( !LoadItemData( m_nSelectedItemNum ) && !( m_pDataBlock && m_nBlockLen ) )
        pInfo->m_bContinuePrinting = false;     // (can't print tapemarks)

    // Create printer font...  (Note: proper mapping mode must be set first!)

    int nOrigMapMode = pDC->SetMapMode( MM_HIENGLISH );
    if ( m_pPrinterFont ) delete m_pPrinterFont; m_pPrinterFont = new CFont;
    m_pPrinterFont->CreatePointFont( m_nPrinterFontSize * 10, m_strPrinterFontName, pDC );

    // Save this font's width and height (in logical units)...

    CFont* pOrigFont = pDC->SelectObject( m_pPrinterFont );
    {
        TEXTMETRIC tm; VERIFY( pDC->GetTextMetrics( &tm ) );
        m_nLineHeight = tm.tmHeight;
        m_nCharWidth  = tm.tmAveCharWidth;
    }

    // Calculate #of pages needed to print this data block...

    //                 ** IMPORTANT PROGRAMMING NOTE! **

    // Unfortunately, MFC doesn't initialize the 'm_rectDraw' member of CPrintInfo
    // until after the call to OnPrepareDC() returns (and we're in OnBeginPrinting
    // at the moment; OnPrepareDC won't be called until sometime AFTER we return).
    // Therefore we need to calculate the rectangle ourselves based on the current
    // printer device context...

    CRect rectAdjustedPrintRect = AdjustPrintRect( CalcPrintRect( pDC ) );

    bool bIsHexView = m_pDoc->m_pHexEditView->IsHexView() ? true : false;

    if ( bIsHexView && m_pDoc->m_pHexEditView->IsDynBPR() )     // (dynamic bytes-per-row?)
    {
        // Dynamic print formatting (i.e. they didn't specify a specific, fixed,
        // bytes-per-row (bytes-per-line) value). Thus we need to calculate our-
        // selves how many bytes will fit on each print line...

        m_nBytesPerLine = BytesPerLine( rectAdjustedPrintRect );

        TRACE( _T( "** OnBeginPrinting: dynbpr = true; m_nBytesPerLine = %d\n" ), m_nBytesPerLine );
    }
    else
    {
        // They specified a specific bytes-per-row (bytes-per-line) value.
        // Use the value they specified. It's up to them to make sure the
        // page is wide enough to print each line...

        if (bIsHexView)
            m_nBytesPerLine = m_pDoc->m_pHexEditView->GetBPR();
        else
            m_nBytesPerLine = m_pDoc->m_pHexEditView->GetTextRecLen();

        TRACE( _T( "** OnBeginPrinting: DYNBPR = FALSE; m_nBytesPerLine = %d  (fmt=%s)\n" ),
            m_nBytesPerLine, bIsHexView ? _T( "hex & text" ) : _T("text-only") );
    }

    pInfo->SetMinPage( 1 );
    pInfo->SetMaxPage( NumPrintPages( rectAdjustedPrintRect ) );

    pDC->SelectObject( pOrigFont );
    pDC->SetMapMode( nOrigMapMode );
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//                                 OnPrepareDC
//
// Called by the framework before the OnDraw member function is called for screen
// display and before the OnPrint member function is called for each page during
// printing or print preview.
//
// Override OnPrepareDC for any of the following reasons:
//
//      o   To adjust attributes of the device context as needed for the
//          specified page. For example, if you need to set the mapping
//          mode or other characteristics of the device context, do so in
//          this function.
//
// Call the base class version of OnPrepareDC at the beginning of your override.
//
//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnPrepareDC( CDC* pDC, CPrintInfo* pInfo )
{
    if ( pDC->IsPrinting() )
        pDC->SetMapMode( MM_HIENGLISH );    // (so base class sees it)

    CListView::OnPrepareDC( pDC, pInfo );   // (call bse class)

    if ( pDC->IsPrinting() )
        pDC->SetMapMode( MM_HIENGLISH );    // (so WE see it!)
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//                                  OnPrint
//
// Called by the framework to print or preview a page of the document. For each page
// being printed, the framework calls this function immediately after calling the
// OnPrepareDC member function. The page being printed is specified by the m_nCurPage
// member of the CPrintInfo structure that pInfo points to. The default implementation
// calls the OnDraw member function and passes it the printer device context.
//
// Override this function for any of the following reasons:
//
//     o  To allow printing of multipage documents. Render only the portion
//        of the document that corresponds to the page currently being printed.
//        If you're using OnDraw to perform the rendering, you can adjust the
//        viewport origin so that only the appropriate portion of the document
//        is printed.
//
//     o  To make the printed image look different from the screen image (that
//        is, if your application is not WYSIWYG). Instead of passing the printer
//        device context to OnDraw, use the device context to render an image
//        using attributes not shown on the screen.
//
//     o  If you need GDI resources for printing that you don't use for screen
//        display, select them into the device context before drawing and deselect
//        them afterwards. These GDI resources should be allocated in OnBeginPrinting
//        and released in OnEndPrinting.
//
// To implement headers or footers. You can still use OnDraw to do the rendering by
// restricting the area that it can print on.
//
// Note that the m_rectDraw member of the pInfo parameter describes the printable area
// of the page in logical units.
//
// Do not call OnPrepareDC in your override of OnPrint; the framework calls OnPrepareDC
// automatically before calling OnPrint.
//
//////////////////////////////////////////////////////////////////////////////////////////

LPCTSTR FormatDumpLine( BYTE*   pData,          size_t  nDataLen,
                        LPTSTR  pszPrtBuff,     size_t& nPrtBuffLen,
                        size_t  nBytesPerLine,  size_t  nBytesPerGroup,
                        bool    bEBCDIC,        bool    bCalcPrintLineLenOnly, bool bIsHexView );

void CMyListView::OnPrint( CDC* pDC, CPrintInfo* pInfo )
{
    ///////////////////////////////////////////////////////////////////////////////
    //
    //
    //                    Mapping Mode == MM_HIENGLISH
    //
    //
    //            Each logical unit is converted to 0.001 inch.
    //            Positive x is to the right; positive y is UP!
    //                                        ^^^^^^^^^^^^^^^^^
    //
    //
    //                            -- EXAMPLE --
    //
    //
    //                     8.5" x 11.0" landscape mode
    //
    //                  1000 units per inch (MM_HIENGLISH)
    //
    //
    //
    //                 left (0)                   right (11000)
    //                     |                           |
    //                     V                           V
    //
    //       top (0) -->   +---------------------------+
    //                     | line 1...                 |
    //                     | line 2...                 |
    //                     |      .                    |
    //                     |      .                    |
    //                     |      .                    | <--- 'Y' axis (vert)
    //                     |      .    PAGE            |
    //                     |      .                    |
    //                     |      .                    |
    //                     |      .                    |
    //                     |      .                    |
    //                     | line n...                 |
    // bottom (-8500) -->  +---------------------------+
    //
    //            A                  A
    //            |                  |
    //            |                  +----- 'X' axis (horiz)
    //            |
    //            |
    //            |
    //
    //        ** NOTE! **
    //
    //  Negative value here! Thus,
    //  you must remember to SUBTRACT
    //  from the 'Y' coordinate in
    //  order to move DOWN the page!
    //
    ///////////////////////////////////////////////////////////////////////////////

    CRect rectPrint = AdjustPrintRect( pInfo->m_rectDraw );  // (our print rect)

    // Set the mapping mode first, THEN select the font into the device context...

    int nOrigMapMode = pDC->SetMapMode( MM_HIENGLISH );
    CFont* pOrigFont = pDC->SelectObject( m_pPrinterFont );

    // Allocate a print-line buffer large enough for our needs...

    bool    bIsHexView             = m_pDoc->m_pHexEditView->IsHexView() ? true : false;
    size_t  nPrtBuffLen            = 0;  // (to be calculated)
    size_t  nBytesPerGroup         = m_pDoc->m_pHexEditView->GetBPG();
    bool    bEBCDIC                = m_pDoc->m_pHexEditView->IsEBCDICMode() ? true : false;
    bool    bCalcPrintLineLenOnly  = true; // (work flag)

    FormatDumpLine( NULL, 0, NULL, nPrtBuffLen, m_nBytesPerLine, nBytesPerGroup, false, bCalcPrintLineLenOnly, bIsHexView );
    ASSERT(                        nPrtBuffLen );

    LPTSTR  pszPrintLineBuff = new TCHAR [ nPrtBuffLen ];
    ASSERT( pszPrintLineBuff );

    // Now [continue] print[ing] this block of data...

    // PROGRAMMING NOTE: if you change the format of the below print page (i.e. by
    // adding additional blank lines, etc) then you need to make sure you make the
    // same corresponding changes to 'NumPrintPages()', 'CalcPrintPos()', etc, too!

    CString  str;  // (work)

    // First, print the page header...
    {
        str.Format( _T( "   Page %d of %d" ), pInfo->m_nCurPage, pInfo->GetMaxPage() );
        CSize szPageNumRect = pDC->GetTextExtent( str );

        // (file path...)

        CRect rectDraw = rectPrint;
        rectDraw.right -= szPageNumRect.cx;
        rectDraw.bottom = rectDraw.top - m_nLineHeight;

        pDC->DrawText( m_pDoc->GetPathName(), &rectDraw, DT_LEFT | DT_TOP | DT_PATH_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE );

        // (page number...)

        rectDraw = rectPrint;
        rectDraw.left = rectDraw.right - szPageNumRect.cx;
        rectDraw.bottom = rectDraw.top - m_nLineHeight;

        pDC->DrawText( str, &rectDraw, DT_LEFT | DT_TOP | DT_PATH_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE );

        // (underline...)

        pDC->MoveTo( rectPrint.left,  rectPrint.top - ( m_nLineHeight + UNDERLINE_OFFSET( m_nLineHeight ) ) );
        pDC->LineTo( rectPrint.right, rectPrint.top - ( m_nLineHeight + UNDERLINE_OFFSET( m_nLineHeight ) ) );
    }

    // PROGRAMMING NOTE: if you change the format of the below print page (i.e. by
    // adding additional blank lines, etc) then you need to make sure you make the
    // same corresponding changes to 'NumPrintPages()', 'CalcPrintPos()', etc, too!

    // Skip a line, then print the file#, block#, and block size...

    int x = rectPrint.left;
    int y = rectPrint.top - ( m_nLineHeight + UNDERLINE_OFFSET( m_nLineHeight ) + m_nLineHeight );

    {
        CAWSChunkHdr hdr = m_pDoc->ChunkInfo( m_nSelectedChunkNum ).Header();

        CString strFileNum  = m_pListCtrl->GetItemText( m_nSelectedItemNum, COL_0_FILENUM_COLNUM  );
        CString strBlockNum = m_pListCtrl->GetItemText( m_nSelectedItemNum, COL_1_BLOCKNUM_COLNUM );
        CString strBytes    = m_pListCtrl->GetItemText( m_nSelectedItemNum, COL_2_BYTES_COLNUM    );

        CString strCompFmt1;    // (compression format)
        CString strCompFmt2;    // (compression format)

        if      ( !hdr.IsCompressed()    ) strCompFmt1 = _T( "none"  );
        else if (  hdr.IsZCompressed()   ) strCompFmt1 = _T( "ZLIB"  );
        else if (  hdr.IsBZ2Compressed() ) strCompFmt1 = _T( "BZIP2" );
        else                               strCompFmt1 = _T( "HW"    );

        strCompFmt2 = (hdr.IsCompressed() && !hdr.IsHCompressed()) ? _T( " (Bus-Tech)" ) : _T("");

        str.Format
        (
            _T( "%s   %s    Comp: %s%s    Len: %s   (%X hex)" )

            ,strFileNum
            ,strBlockNum
            ,strCompFmt1,strCompFmt2
            ,strBytes
            ,m_nBlockLen
        );

        VERIFY( pDC->TextOut( x, y, str ) );

        CString strTextFormat = bEBCDIC ? _T( "-- EBCDIC --" ) : _T( "-- ASCII --" );

        int x_TextFormat;

        if (bIsHexView)
        {
            x_TextFormat = rectPrint.left
                +  (int)( ( 6 + 3 + nPrtBuffLen ) * m_nCharWidth )
                -  (int)( ( m_nBytesPerLine / 2 ) * m_nCharWidth )
                -       (           7             * m_nCharWidth )
                ;
        }
        else
        {
            int nPageWidthInChars = rectPrint.Width() / m_nCharWidth;

            x_TextFormat = rectPrint.right -
            (
                (
                    ((nPageWidthInChars - str.GetLength()) / 2)
                    +
                    (strTextFormat.GetLength() / 2)
                )

                * m_nCharWidth
            );
        }

        VERIFY( pDC->TextOut( x_TextFormat, y, strTextFormat ) );

        // PROGRAMMING NOTE: if you change the format of the below print page (i.e. by
        // adding additional blank lines, etc) then you need to make sure you make the
        // same corresponding changes to 'NumPrintPages()', 'CalcPrintPos()', etc, too!

        y -= ( 2 * m_nLineHeight );     // (skip a line)
    }

    // Now fill the rest of the page with dump lines for the [remaining] block data...

    ASSERT( m_pDataBlock && m_nBlockLen );     // (sanity check)

    // Now format dump lines, one by one, from our block data,
    // into that print line buffer and then print them (one by one)...

    SSIZE_T nPrintPos  = CalcPrintPos( rectPrint, pInfo->m_nCurPage );
    ASSERT( nPrintPos < m_nBlockLen );     // (sanity check)

    BYTE*   pThisLinesData;
    size_t  nBytesThisLine;
    CString strDumpLine;

    CRect rectDraw;

    rectDraw.left   = x;
    rectDraw.right  = x + rectPrint.Width();

    do
    {
        rectDraw.top    = y;
        rectDraw.bottom = y - m_nLineHeight;

        pThisLinesData  =  m_pDataBlock + nPrintPos;
        nBytesThisLine  =  m_nBlockLen  - nPrintPos;

        strDumpLine.Format
        (
            _T( "%06.6X   %s" )

            ,nPrintPos
            ,FormatDumpLine( pThisLinesData,   nBytesThisLine,
                             pszPrintLineBuff, nPrtBuffLen,
                             m_nBytesPerLine,  nBytesPerGroup,
                             bEBCDIC,          false,           bIsHexView )
        );

        UINT nFormat =
            0
            | DT_LEFT // Aligns text to the left.
            | DT_TOP // Justifies the text to the top of the rectangle.
            | DT_NOPREFIX // Print "A&bc&&d" as: "A&bc&&d"
            | DT_SINGLELINE // Displays text on a single line only. Carriage returns and line feeds do not break the line.
            ;

        VERIFY( pDC->DrawText( strDumpLine, &rectDraw, nFormat ) );

        y         -= m_nLineHeight;         // (move down the page...)
        nPrintPos += m_nBytesPerLine;       // (next block position...)
    }
    while ( nPrintPos < m_nBlockLen && y >= ( rectPrint.bottom + (LONG)m_nLineHeight ) );

    delete [] pszPrintLineBuff;

    pDC->SetMapMode( nOrigMapMode );
    pDC->SelectObject( pOrigFont );
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//                                OnEndPrinting
//
// Called by the framework after a document has been printed or previewed. The default
// implementation of this function does nothing. Override this function to free any GDI
// resources you allocated in the OnBeginPrinting member function.
//
//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnEndPrinting( CDC* /*pDC*/, CPrintInfo* /*pInfo*/ )
{
    delete m_pPrinterFont;
    m_pPrinterFont = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////

LPCTSTR FormatDumpLine( BYTE*   pData,          size_t  nDataLen,
                        LPTSTR  pszPrtBuff,     size_t& nPrtBuffLen,
                        size_t  nBytesPerLine,  size_t  nBytesPerGroup,
                        bool    bEBCDIC,        bool    bCalcPrintLineLenOnly, bool bIsHexView )
{
    ASSERT( nBytesPerLine );

    if ( !nBytesPerLine || !nBytesPerGroup )
    {
        nPrtBuffLen = 0;
        return NULL;
    }

    // If we're supposed to only print the text area and not the hex area
    // and they're only interested in how long the printline is going to be,
    // then our calculation is real simple: our printline length will be
    // exactly equal to their #of [text] bytes per line value...

    if (!bIsHexView && bCalcPrintLineLenOnly)
    {
        nPrtBuffLen = nBytesPerLine + 1;    // (plus+1 for NULL terminator)
        return NULL;
    }

    // Calculate width (in print chars) of each area of the print line (hex / text)...

    size_t  nTextAreaWidthInChars = nBytesPerLine;
    size_t  nFullGroupsPerLine    = nBytesPerLine /   nBytesPerGroup;
    size_t  nBytesInLastGroup     = nBytesPerLine - ( nBytesPerGroup * nFullGroupsPerLine );
    size_t  nHexAreaWidthInChars  =
        0
        +  ( nFullGroupsPerLine * nBytesPerGroup * 2 )  // (1 byte = 2 hex chars)
        +    nFullGroupsPerLine                         // (blank between groups)
        +  ( nBytesInLastGroup * 2 )                    // (n bytes = 2n hex chars)
        ;

    if ( !nBytesInLastGroup )
        nHexAreaWidthInChars--;

    // Print line format is: hex area, 3 blanks, text area...

    size_t  nPrintLineLenInChars = nHexAreaWidthInChars + 3 + nTextAreaWidthInChars;

    if ( bCalcPrintLineLenOnly )
    {
        nPrtBuffLen = nPrintLineLenInChars + 1;     // (plus+1 for NULL terminator)
        return NULL;
    }

    if (!bIsHexView)
    {
        // (we will be printing ONLY text data and NOT any hex data)
        nHexAreaWidthInChars = 0;
        nPrintLineLenInChars = nBytesPerLine;
    }

    ASSERT( pData &&  nDataLen &&  pszPrtBuff && nPrtBuffLen >  nPrintLineLenInChars );
    if (   !pData || !nDataLen || !pszPrtBuff || nPrtBuffLen <= nPrintLineLenInChars )
        return NULL;

    static const TCHAR  chBlank  = _T( ' ' );

    // Initialize the entire print line to blanks...
    {
        register LPTSTR p = pszPrtBuff; register size_t i;
        for (i=0; i < nPrintLineLenInChars; i++) *p++ = chBlank;
        *p = 0; // (NULL terminate)
    }

    BYTE*   pHexData     = pData;
    size_t  nDataLenWrk  = min( nDataLen, nBytesPerLine );
    LPTSTR  pszHexArea   = bIsHexView ?   pszPrtBuff                              : NULL;
    LPTSTR  pszTextArea  = bIsHexView ? ( pszPrtBuff + nHexAreaWidthInChars + 3 ) : pszPrtBuff;
    size_t  nGroups       = 0;
    size_t  nGroupLen     = 0;

    // Format the hex area portion of the print line one fullword at a time...

    if (bIsHexView)
    {
        for (;;)
        {
            nGroupLen = min( nBytesPerGroup, nDataLenWrk );

            MakeHex( pHexData, nGroupLen, pszHexArea );
            pszHexArea += ( 2 * nGroupLen );
            *pszHexArea = chBlank;

            pHexData     +=  nGroupLen;
            nDataLenWrk  -=  nGroupLen;

            if ( !nDataLenWrk ) break;

            nGroups++;
            pszHexArea++;   // (blank between groups)
        }
    }

    // Now do the text area...

    nDataLenWrk = min( nDataLen, nBytesPerLine );
    MakePrt( pData, nDataLenWrk, pszTextArea, bEBCDIC );

    *( pszTextArea + nDataLenWrk ) = chBlank;
    *( pszPrtBuff + nPrintLineLenInChars ) = 0;

    return pszPrtBuff;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
//     O t h e r   M i s c   C o m m a n d   P r o c e s s i n g   F u n c t i o n s
//
//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnEditFindNext()
{
    FindNext();
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnEditFindPrevious()
{
    FindPrev();
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnEditNextBlock()
{
    int nItemNum = m_nSelectedItemNum;
    if ( ++nItemNum >= m_pListCtrl->GetItemCount() )
    {
        MessageBeep( MB_ICONEXCLAMATION );
        return;
    }
    SelectItem( nItemNum );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnEditPreviousBlock()
{
    int nItemNum = m_nSelectedItemNum;
    if ( --nItemNum < 0 )
    {
        MessageBeep( MB_ICONEXCLAMATION );
        return;
    }
    SelectItem( nItemNum );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnEditNexFile()
{
    int nItemNum = m_nSelectedItemNum;
    int nChunkNum;
    EAWSChunkType  chunk_type;

    do
    {
        if ( ++nItemNum >= m_pListCtrl->GetItemCount() )
        {
            SelectItem( --nItemNum );
            MessageBeep( MB_ICONEXCLAMATION );
            return;
        }

        nChunkNum = (int)m_pListCtrl->GetItemData( nItemNum );
        chunk_type = m_pDoc->ChunkInfo( nChunkNum ).ChunkType();
    }
    while ( TapeMark != chunk_type );

    SelectItem( nItemNum );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnEditPreviousFile()
{
    int nItemNum = m_nSelectedItemNum;
    int nChunkNum;
    EAWSChunkType  chunk_type;

    do
    {
        if ( --nItemNum < 0 )
        {
            SelectItem( 0 );
            MessageBeep( MB_ICONEXCLAMATION );
            return;
        }

        nChunkNum = (int)m_pListCtrl->GetItemData( nItemNum );
        chunk_type = m_pDoc->ChunkInfo( nChunkNum ).ChunkType();
    }
    while ( TapeMark != chunk_type );

    SelectItem( nItemNum );
}

//////////////////////////////////////////////////////////////////////////////////////////
// (calculate 'm_rectDraw' ourselves! Also initializes
// minimum margin values for default printer)...

CRect CMyListView::CalcPrintRect( CDC* pDC /* =NULL */ )
{
    CDC dcPrinter;

    // If no printer DC was passed, then create a temporary one for us to use...

    if ( !pDC )
    {
        VERIFY( g_App.CreatePrinterDC( dcPrinter ) );
        pDC = &dcPrinter;
    }

    // PHYSICALWIDTH:     For printing devices: the width of the physical page, in
    //                    device units. For example, a printer set to print at 600 dpi
    //                    on 8.5 x 11-inch paper has a physical width value of 5100
    //                    device units. Note that the physical page is almost always
    //                    greater than the printable area of the page, and never smaller.

    // PHYSICALHEIGHT:    For printing devices: the height of the physical page, in
    //                    device units. For example, a printer set to print at 600 dpi
    //                    on 8.5 x 11-inch paper has a physical height value of 6600
    //                    device units. Note that the physical page is almost always
    //                    greater than the printable area of the page, and never smaller.

    double dPHYSICALWIDTHInDeviceUnits   = (double) pDC -> GetDeviceCaps ( PHYSICALWIDTH   );
    double dPHYSICALHEIGHTInDeviceUnits  = (double) pDC -> GetDeviceCaps ( PHYSICALHEIGHT  );

    // PHYSICALOFFSETX:   For printing devices: the distance from the left edge of the
    //                    physical page to the left edge of the printable area, in device
    //                    units. For example, a printer set to print at 600 dpi on 8.5-
    //                    x 11-inch paper, that cannot print on the leftmost 0.25-inch
    //                    of paper, has a horizontal physical offset of 150 device units.

    // PHYSICALOFFSETY:   For printing devices: the distance from the top edge of the
    //                    physical page to the top edge of the printable area, in device
    //                    units. For example, a printer set to print at 600 dpi on 8.5-
    //                    x 11-inch paper, that cannot print on the topmost 0.5-inch of
    //                    paper, has a vertical physical offset of 300 device units.

    double dPHYSICALOFFSETXInDeviceUnits = (double) pDC -> GetDeviceCaps ( PHYSICALOFFSETX );
    double dPHYSICALOFFSETYInDeviceUnits = (double) pDC -> GetDeviceCaps ( PHYSICALOFFSETY );

    //  LOGPIXELSX:  Number of pixels per logical inch along the width of the page.
    //  LOGPIXELSY:  Number of pixels per logical inch along the height of the page.

    double dHorzDeviceUnitsPerInch = (double) pDC -> GetDeviceCaps ( LOGPIXELSX );
    double dVertDeviceUnitsPerInch = (double) pDC -> GetDeviceCaps ( LOGPIXELSY );

    // Now since our mapping mode is MM_HIENGLISH, we need to convert
    // the above physical unit values (pixels) to logical units (0.001")...

    double dPageWidthInInches   = dPHYSICALWIDTHInDeviceUnits  / dHorzDeviceUnitsPerInch;
    double dPageHeightInInches  = dPHYSICALHEIGHTInDeviceUnits / dVertDeviceUnitsPerInch;

    double dHorzOffsetInInches  =  dPHYSICALOFFSETXInDeviceUnits / dHorzDeviceUnitsPerInch;
    double dVertOffsetInInches  =  dPHYSICALOFFSETYInDeviceUnits / dVertDeviceUnitsPerInch;

    double dPageWidthInLogicalUnits  = dPageWidthInInches  * 1000.0;
    double dPageHeightInLogicalUnits = dPageHeightInInches * 1000.0;

    dPageWidthInLogicalUnits  += 0.5;
    dPageHeightInLogicalUnits += 0.5;

    double dHorzOffsetInLogicalUnits = dHorzOffsetInInches * 1000.0;
    double dVertOffsetInLogicalUnits = dVertOffsetInInches * 1000.0;

    dHorzOffsetInLogicalUnits += 0.5;
    dVertOffsetInLogicalUnits += 0.5;

    // (save calculated minimum margin values)

    m_nMinLeftMargin   = (int) dHorzOffsetInLogicalUnits;
    m_nMinRightMargin  = (int) dHorzOffsetInLogicalUnits;

    m_nMinTopMargin    = (int) dVertOffsetInLogicalUnits;
    m_nMinBottomMargin = (int) dVertOffsetInLogicalUnits;

    // Now we get to calculate our print rectangle in #of logical units...

    // (remember our mapping mode is MM_HIENGLISH, so positive x
    //  is to the right, POSITIVE y is UP, and NEGATIVE y is DOWN)...

    CRect rectPrintableAreaOfPage;  rectPrintableAreaOfPage.SetRectEmpty();

    rectPrintableAreaOfPage.right  += (int) dPageWidthInLogicalUnits;
    rectPrintableAreaOfPage.bottom -= (int) dPageHeightInLogicalUnits;

    // The rectangle should now encompass the entire physical page.
    // Now all we have to do is shrink it (make it a little bit smaller)
    // by the offset amount to yield a rectangle encompassing only the
    // PRINTABLE portion of the physical page (in logical units)...

    rectPrintableAreaOfPage.top    -= m_nMinTopMargin;    // (because negative is down)
    rectPrintableAreaOfPage.bottom += m_nMinBottomMargin; // (because positive is up)

    rectPrintableAreaOfPage.left   += m_nMinLeftMargin;
    rectPrintableAreaOfPage.right  -= m_nMinRightMargin;

    // Done! Return our calculated 'm_rectDraw' value...

    return rectPrintableAreaOfPage;
}

//////////////////////////////////////////////////////////////////////////////////////////
// (apply margins to MFC's page rectangle)

CRect CMyListView::AdjustPrintRect( CRect rectPrint )
{
    // PROGRAMMING NOTE: the m_rectDraw rectangle that MFC passes in CPrintInfo
    // is the printable area of the page. Therefore, in order to properly apply
    // the print margins that the user requested, we must first expand our print
    // rectangle by our saved minimum margin value to obtain a rectangle encom-
    // passing the actual entire physical page, and then afterwards shrink that
    // rectangle by the user's specified margin settings to yield a rectangle
    // eccompassing the actual portion of the page that we're to print within.

    // (See also comments regarding MM_HIENGLISH mapping mode in OnPrint function)

    CRect rectAdjusted = rectPrint;         // (start with printable area of page)

    rectAdjusted.left    -=  m_nMinLeftMargin;  // (expand to get actual page rectangle)
    rectAdjusted.right   +=  m_nMinRightMargin; // (expand to get actual page rectangle)
    rectAdjusted.top     +=  m_nMinTopMargin;   // (   ADD    because positive is UP!   )
    rectAdjusted.bottom  -=  m_nMinBottomMargin;// ( SUBTRACT because negative is DOWN! )

    rectAdjusted.left    +=  m_nLeftMargin;     // (now shrink it by the specified amount)
    rectAdjusted.right   -=  m_nRightMargin;    // (now shrink it by the specified amount)
    rectAdjusted.top     -=  m_nTopMargin;      // ( SUBTRACT because negative is DOWN! )
    rectAdjusted.bottom  +=  m_nBottomMargin;   // (   ADD    because positive is UP!   )

    return rectAdjusted;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Calculate #of bytes per line that will fit on a page...  (only used for dynamic BPR)

size_t CMyListView::BytesPerLine( CRect rectPrint )
{
    // PROGRAMMING NOTE: since this function is only ever called whenever dynamic BPR
    // is active/enabled and since dynamic BPR is only ever enabled/active whenever
    // the hex area is being displayed/shown, then 'HexView' BETTER be 'true' or else
    // something is very wrong somwhere!

    bool bIsHexView = m_pDoc->m_pHexEditView->IsHexView() ? true : false;

    if (!bIsHexView)
    {
        ASSERT( false );        // (oops! we fucked up somewhere!)
        bIsHexView = true;      // (not sure what else can we do!)
    }

    // PROGRAMMING NOTE: the passed 'rectPrint' value has already been adjusted
    //                   to allow for the user's specified print margins...

    // Calculate the maximum #of characters than will fit on the page...

    ASSERT( m_nCharWidth );         // (we'll crash if this isn't true)
    rectPrint.NormalizeRect();      // (so Width() comes out positive)

    size_t nPageWidthInChars = rectPrint.Width() / m_nCharWidth;

    if (nPageWidthInChars < 1)  // Allow for possibility of zero-width print area...
        nPageWidthInChars = 1;  // (in case the left/right margins are too big)

    TRACE( _T( "*** starting nPageWidthInChars = %d\n" ), nPageWidthInChars );

    // TECHNIQUE: keep calling 'FormatDumpLine', decreasing the 'bytes-per-line'
    // value we pass it each time, until the string it returns is short enough
    // to fit on the given page...

    // Note that while this may be inefficient, we don't really care since we're
    // only ever called ONCE at the start of a given print / print-preview job,
    // and not repeatedly for each print line (and besides, the FormatDumpLine
    // function is coded to not even bother building an actual properly formatted
    // print line in the first place when called with the bCalcPrintLineLenOnly
    // option, so it's not really that inefficient in the first darn place. Our
    // main concern here is *accuracy*, not efficiency)...

    size_t  nBytesPerLine  = nPageWidthInChars;     // (starting value known to be too big)
    size_t  nBytesPerGroup = m_pDoc->m_pHexEditView->GetBPG();
    size_t  nPrtBuffLen    = nPageWidthInChars + 1; // (ditto)

    bool bEBCDIC               = false;
    bool bCalcPrintLineLenOnly = true;

    while ( nBytesPerLine > 1 && ( 9 + nPrtBuffLen - 1 ) > nPageWidthInChars )
        FormatDumpLine( NULL, 0, NULL, nPrtBuffLen, --nBytesPerLine, nBytesPerGroup, bEBCDIC,
            bCalcPrintLineLenOnly, bIsHexView );

    TRACE( _T( "*** nBytesPerLine = %d\n" ), nBytesPerLine );

    return nBytesPerLine;
}

//////////////////////////////////////////////////////////////////////////////////////////
// (calc #of pages needed to print current data block)

UINT CMyListView::NumPrintPages( CRect rectPrint )
{
    // PROGRAMMING NOTE: the passed 'rectPrint' value has already been adjusted
    //                   to allow for the user's specified print margins...

    size_t nBytesPerPage = LinesPerPage( rectPrint ) * m_nBytesPerLine;

    ASSERT( m_nBlockLen ); // (we don't support printing tapemarks)

    return (UINT)( ( m_nBlockLen + ( nBytesPerPage - 1 ) ) / nBytesPerPage );
}

//////////////////////////////////////////////////////////////////////////////////////////
// (calc current block location for this page#)

size_t CMyListView::CalcPrintPos( CRect rectPrint, UINT nPageNum )
{
    // PROGRAMMING NOTE: the passed 'rectPrint' value has already been adjusted
    //                   to allow for the user's specified print margins...

    return ( ( nPageNum - 1 ) * ( LinesPerPage( rectPrint ) * m_nBytesPerLine ) );
}

//////////////////////////////////////////////////////////////////////////////////////////
// (calc #of print lines that will fit on a page)

UINT CMyListView::LinesPerPage( CRect rectPrint )
{
    // PROGRAMMING NOTE: the passed 'rectPrint' value has already been adjusted
    //                   to allow for the user's specified print margins...

    // PROGRAMMING NOTE: we assume a non-MM_TEXT mapping mode such that in order to
    //                   move further down the page, we need to make our y-coordinate
    //                   (i.e. the bottom of the print rectangle) more negative...

    // Move top of print rectangle down by the height of our page header lines...

    rectPrint.top -= ( m_nLineHeight + UNDERLINE_OFFSET( m_nLineHeight ) + m_nLineHeight );
    rectPrint.top -= ( 2 * m_nLineHeight );

    // Allow for possibility of zero-height remaining print area...
    // (i.e. in case the specified top/bottom margins are too big)

    if (rectPrint.bottom > ( rectPrint.top - (LONG)m_nLineHeight ) )
        rectPrint.bottom =   rectPrint.top - (LONG)m_nLineHeight;

    ASSERT( m_nLineHeight );        // (we'll crash if this isn't true)
    rectPrint.NormalizeRect();      // (so Height() comes out positive)
    UINT nFullLinesPerPage = rectPrint.Height() / m_nLineHeight;

    if (nFullLinesPerPage < 1)      // (less than one print line per page?!)
        nFullLinesPerPage = 1;      // (force minimum of at least one line per page)

    return nFullLinesPerPage;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnViewPrinterFont()
{
    CDC dcPrinter; VERIFY( g_App.CreatePrinterDC( dcPrinter ) );
    if ( m_pPrinterFont ) delete m_pPrinterFont; m_pPrinterFont = new CFont;
    m_pPrinterFont->CreatePointFont( m_nPrinterFontSize * 10, m_strPrinterFontName );
    LOGFONT lf; m_pPrinterFont->GetLogFont( &lf );

    DWORD dwFlags =
        0
        | CF_INITTOLOGFONTSTRUCT
        | CF_PRINTERFONTS
        | CF_FIXEDPITCHONLY
        ;

    CMyFontDialog  dlg( _T("Printer Font"), &lf, dwFlags, &dcPrinter, this );

    if ( IDOK != dlg.DoModal() ) return;

    m_strPrinterFontName = dlg.GetFaceName();
    m_nPrinterFontSize   = dlg.GetSize() / 10;

    if ( m_pPrinterFont ) delete m_pPrinterFont; m_pPrinterFont = new CFont;
    m_pPrinterFont->CreatePointFont( m_nPrinterFontSize * 10, m_strPrinterFontName, &dcPrinter );

    SaveProfileSettings();
}

//////////////////////////////////////////////////////////////////////////////////////////
// (they right-clicked on a ListView item)

void CMyListView::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
    // Check to see if they actually clicked on a ListView item,
    // or if they clicked somewhere else...

    LVHITTESTINFO  hti;  memset( &hti, 0, NUM_BYTES( hti ) );

    hti.pt = ptScreen; m_pListCtrl->ScreenToClient( &hti.pt );

    m_pListCtrl->HitTest( &hti );

    if ( !(hti.flags & LVHT_ONITEM) )   // (clicked on item?)
        return;                         // (no, ignore click)

    // Display our Right-Click context menu...

    CMenu  menu; VERIFY( menu.LoadMenu( IDR_LVRCLICK_MENU ) );
    CMenu* pPopup = menu.GetSubMenu(0); ASSERT( pPopup );

    SetForegroundWindow();          // (required so that popup menu disappears)
    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, this);
    PostMessage(WM_NULL, 0, 0);     // (to correct TrackPopupMenu problem; see Q135788)
}

//////////////////////////////////////////////////////////////////////////////////////////
// (double-clicking same as right-clicking and selecting "Block Details")

void CMyListView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    CListView ::OnLButtonDblClk(nFlags, point);

    if (m_pListCtrl && m_pListCtrl->HitTest(point) >= 0)
        OnListViewRightClickInfo();
}

//////////////////////////////////////////////////////////////////////////////////////////
// (we presume m_nSelectedItemNum & m_nSelectedChunkNum are correct/valid)

void CMyListView::OnListViewRightClickInfo()
{
    // Gather the information...

    CAWSChunkInfo  chunk_info  = m_pDoc->ChunkInfo( m_nSelectedChunkNum );

    WORD           wCurrChunkLen  = chunk_info.CurrChunkLen();
    WORD           wPrevChunkLen  = chunk_info.PrevChunkLen();
    bool           bIsBegBlk      = chunk_info.IsBegBlk();
    bool           bIsTapeMark    = chunk_info.IsTapeMark();
    bool           bIsEndBlk      = chunk_info.IsEndBlk();
    bool           bIsCompressed  = chunk_info.IsCompressed();
    bool           bIsZCompressed = chunk_info.IsZCompressed();
    bool           bIsBZ2Compress = chunk_info.IsBZ2Compressed();
    bool           bIsHCompressed = chunk_info.IsHCompressed();
    bool           bIsValid       = chunk_info.IsValid();
    CAWSChunkHdr   AWSChunkHdr    = chunk_info.Header();

    BYTE*    pAWSChunkHdrBytes    = (BYTE*) &AWSChunkHdr;

    // Format it...

    CString  strFileNum   =  m_pListCtrl->GetItemText( m_nSelectedItemNum, COL_0_FILENUM_COLNUM  );
    CString  strBlockNum  =  m_pListCtrl->GetItemText( m_nSelectedItemNum, COL_1_BLOCKNUM_COLNUM );
    CString  strBlockSize =  m_pListCtrl->GetItemText( m_nSelectedItemNum, COL_2_BYTES_COLNUM    );

    CString strCompFmt1;    // (compression format)
    CString strCompFmt2;    // (compression format)

    CString  strCurrChunkLen; strCurrChunkLen . Format( _T( "%s" ), g_Locale.FormatNumber( wCurrChunkLen ) );
    CString  strPrevChunkLen; strPrevChunkLen . Format( _T( "%s" ), g_Locale.FormatNumber( wPrevChunkLen ) );
    CString  strFlags;        strFlags        . Format
    (
        _T( "%s%s%s%s%s%s%s%s" )

        ,bIsBegBlk                                           ?  _T( "BeginningOfBlock "  )  :  _T("")
        ,bIsTapeMark                                         ?  _T( "TapeMark "          )  :  _T("")
        ,bIsEndBlk                                           ?  _T( "EndOfBlock "        )  :  _T("")
        ,bIsZCompressed                                      ?  _T( "ZLIB "              )  :  _T("")
        ,bIsBZ2Compress                                      ?  _T( "BZIP2 "             )  :  _T("")
        ,bIsCompressed && !bIsHCompressed && !bIsZCompressed ?  _T( "HW "                )  :  _T("")
        ,bIsCompressed && !bIsHCompressed                    ?  _T( "(Bus-Tech) "        )  :  _T("")
        ,bIsValid                                            ?  _T( ""                   )  :  _T("Invalid")
    );
    if (strFlags.IsEmpty()) strFlags = _T( "(none)" );

    CString  strSummary;

    if (bIsTapeMark) strSummary = _T( "    *** TAPEMARK ***" );
    else strSummary.Format
    (
        _T( "%s   %s    Len: %s" )

        ,strFileNum
        ,strBlockNum
        ,strBlockSize
    );

    CString strFileDisp = g_DefaultLocale.FormatNumber( chunk_info.FilePtr().QuadPart );

#define  CRLF  _T( "\r\n" )

    CBlockDetailDlg  dlg(this);  dlg.m_strDetails.Format
    (
        _T( "%s" )                                      CRLF
                                                        CRLF
        _T( "Displacement: %s dec, %08.8X:%08.8X hex" ) CRLF
                                                        CRLF
        _T( "%s" )                                      CRLF
                                                        CRLF
        _T( " -- AWS Control Bytes --" )                CRLF
                                                        CRLF
        _T( "    %2.2X%2.2X  %2.2X%2.2X  %2.2X%2.2X" )  CRLF
                                                        CRLF
        _T( "  Current:  %s bytes" )                    CRLF
        _T( "  Previous: %s bytes" )                    CRLF
        _T( "  Flags:    %s"       )                    CRLF

        ,m_pDoc->GetPathName()
        ,strFileDisp
        ,chunk_info.FilePtr().HighPart
        ,chunk_info.FilePtr().LowPart
        ,strSummary
        ,*(pAWSChunkHdrBytes + 0)
        ,*(pAWSChunkHdrBytes + 1)
        ,*(pAWSChunkHdrBytes + 2)
        ,*(pAWSChunkHdrBytes + 3)
        ,*(pAWSChunkHdrBytes + 4)
        ,*(pAWSChunkHdrBytes + 5)
        ,strCurrChunkLen
        ,strPrevChunkLen
        ,strFlags
    );

    // Display it...

    dlg.m_strTitle = _T(" Block Details");

    dlg.m_strWindowPlacementKeyName = _T( "BlockDetailDlg" );

    dlg.DoModal();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Search helper function that properly converts raw block data
// into an appropriate CString variable, being careful to take both
// ASCII/EBCDIC and MBCS/UNICODE build settings into consideration...

CString CMyListView::GetBlockDataAsCString( BYTE* pData, SSIZE_T nDataLen, BOOL bEBCDIC )
{
    CString  strDataBlockAsCString;

    // First, copy the data to a temporary work buffer where we can modify it...

    BYTE* pBlockData = new BYTE [ nDataLen + 1 ];  // (plus+1 for NULL terminator (added later))

    if (!pBlockData)
        return strDataBlockAsCString;

    memcpy( pBlockData, pData, nDataLen );

    // Now convert all NULLs to non-NULLs...

    for ( SSIZE_T i=0; i < nDataLen; i++ )
    {
        // (convert embedded NULLs to non-NULLs)
        if ( !(*(pBlockData + i)) )
               *(pBlockData + i) = (BYTE)(~NULL);
    }

    // NULL terminate it...
    *( pBlockData + nDataLen ) = NULL;

    // Convert it to ASCII if it's currently considered to be EBCDIC data...

    if ( bEBCDIC )
    {
        VERIFY
        (
            // PROGRAMMING NOTE: the FishLib 'TranslateString' function
            // works with single-byte character string data and NOT UNICODE.

            TranslateString
            (
                pBlockData, ASCII_CODE_PAGE,    // to
                pBlockData, EBCDIC_CODE_PAGE,   // from
                (int)nDataLen
            )
        );
    }

    // NULL terminate it again, just to be safe...
    *( pBlockData + nDataLen ) = NULL;

    // Finally, move it into a CString variable...

    strDataBlockAsCString = pBlockData;

    delete [] pBlockData;   // (discard work buffer)

    // Verify we did that correctly and return the result...

    ASSERT( strDataBlockAsCString.GetLength() == nDataLen );

    return strDataBlockAsCString;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnViewStatistics()
{
    CBlockDetailDlg  dlg(this);

    dlg.m_strWindowPlacementKeyName = _T( "TapeSummaryDlg" );

    // Identify whether tape is compressed or not...

    // (luckily Herc & Bus-Tech use completely different
    //  flags bits to indicate compression, so the below
    //  switch statement should tell us what we want)

    CString  strCompressionType;

    switch
    (0
        | (m_pDoc->m_bAWSFlag1 & AWSHDR_FLAG1_COMPRESSED)
        | (m_pDoc->m_bAWSFlag2 & AWSHDR_FLAG2_COMPRESSED)
    )
    {
        case AWSHDR_FLAG1_HERC_ZLIB:    strCompressionType = _T( "ZLIB"          ); break;
        case AWSHDR_FLAG1_HERC_BZIP2:   strCompressionType = _T( "BZIP2"         ); break;
        case AWSHDR_FLAG2_BUSTECH_ZLIB: strCompressionType = _T( "Bus-Tech ZLIB" ); break;
        case AWSHDR_FLAG2_BUSTECH_HW:   strCompressionType = _T( "Bus-Tech HW"   ); break;

        default: strCompressionType = _T( "mixed" ); break;
        case 0: break;
    }

    CString  strCompressionString;

    if (strCompressionType.IsEmpty())
        strCompressionString = _T( "uncompressed" );
    else
        strCompressionString = strCompressionType + _T( " compressed" );

    // Build summary header line...

    // (compressed status and filename)

    dlg.m_strDetails.Format
    (
        _T( "Tape Summary for %s file%c%s\"%s\"..." )

        ,strCompressionString
        ,_T(':')
        ,m_pDoc->GetPathName().GetLength() <= 40 ? _T(" ") : CRLF CRLF _T("  ")
        ,m_pDoc->GetPathName()
    );

    // Identify if tape is stdlbl'ed (has a VOL1 label)

    CString  strVolSer;
    CString  strDash = _T( "  --------------------------------------------------------------------------------" );

    if ( !m_pDoc->m_strVol1Label.IsEmpty() )
    {
        strVolSer = m_pDoc->m_strVol1Label.Mid( 4, 6 );

        CString str; str.Format
        (
            CRLF CRLF _T("Standard Labeled Volume: \"%s\"" )

            ,strVolSer
        );

        dlg.m_strDetails += str;

        strDash += _T( "---------" );
    }

    dlg.m_strDetails += CRLF CRLF + strDash + CRLF CRLF;

    // Now gather/total the actual tape's statistics...

    // Iterate through the document's stats objects (CAWSFileStats)
    // that it saved for each physical tapemark separated file on
    // the tape, and print them, accumulating them all as we go...

    CAWSFileStats  tape_stats;
    CAWSFileStats  file_stats, prev_file_stats;
    CString        strFileStats;

    CString  strFileNum;
    CString  strTotalBytes;
    CString  strOverheadBytes;
    CString  strTotalChunkBytes;
    CString  strTotalChunks;
    CString  strTotalDataBytes;
    CString  strOverheadPercent;
    CString  strPercentOriginalSize;

    INT64   nOverheadBytes;             // total bytes - compressed bytes
    double  dOverheadPercent;           // overhead bytes / total bytes
    double  dPercentOriginalSize;       // total bytes / expanded bytes
    int     nFileCount = 0;             // how many "files" we displayed

    int nNumFiles = m_pDoc->NumFiles();

    for (int i=0; i < nNumFiles; i++)
    {
        prev_file_stats   =  file_stats;
        file_stats        =  m_pDoc->GetFileStats(i);
        tape_stats       +=  file_stats;

        // (don't bother reporting lone tapemarks)

        if (1
            && file_stats.m_strStdLabel1.IsEmpty()
            && file_stats.m_strStdLabel2.IsEmpty()
            &&                 0          ==  file_stats.m_nTotalChunkBytes
            &&                 0          ==  file_stats.m_nTotalChunks
            &&                 0          ==  file_stats.m_nTotalDataBytes
            && NUM_BYTES( CAWSChunkHdr )  ==  file_stats.m_nTotalBytes
        )
            continue;   // (skip tapemarks)

        nFileCount++;   // (count #of "files")

        // (physical file#)

        strFileNum.Format( _T( "%s" ), g_Locale.FormatNumber( nFileCount ) );

        // Is this the [tapemark-separated] "file" containing
        // the actual logical stdlbl'ed file's HDR1/EOF1 labels?
        // Or is it the actual [possibly stdlbl'ed] file itself?

        if (0
            || !file_stats.m_strStdLabel1.IsEmpty()
            || !file_stats.m_strStdLabel2.IsEmpty()
        )
        {
            // Show the raw stdlbl's to them as-is...

            // Note: could be either HDR1, EOF1 or EOV1;
            // we don't (nor care) which ones they are.
            // We just show them as-is (in "raw" format).
            // We'll extract the info from them when we
            // process the next "file"...

            strFileStats.Format
            (
                _T( "  File %s" )  CRLF
                                   CRLF
                _T( "    %s"    )  CRLF
                _T( "    %s"    )  CRLF
                                   CRLF
                ,strFileNum

                ,file_stats.m_strStdLabel1  // (as-is = raw format)
                ,file_stats.m_strStdLabel2  // (as-is = raw format)
            );
        }
        else
        {
            // It's the actual [possibly stdlbl'ed] file itself...

            // Check the stats object for the PREVIOUS "file" to
            // see if this is the file data for a normal stdlbl'ed
            // file or an ordinary non-labeled file (in which case
            // there wouldn't be any HDR1 labels in the previous
            // file's stats object)...

            if (prev_file_stats.m_strStdLabel1.Left(4) == _T("HDR1"))
            {
                // The previous "file's" stats object has a HDR1 label,
                // so the file we're currently processing is a stdlbl'ed
                // file. Extract the actual dataset name, job, and step
                // name from the HDR1/HDR2 labels and display that info...

                CString  strFileId, strJob, strStep;

                strFileId = prev_file_stats.m_strStdLabel1.Mid(4,17).Trim();

                if (prev_file_stats.m_strStdLabel2.Left(4) == _T("HDR2"))
                {
                    strJob  = prev_file_stats.m_strStdLabel2.Mid(17,8).Trim();
                    strStep = prev_file_stats.m_strStdLabel2.Mid(26,8).Trim();

                    strFileStats.Format
                    (
                        _T( "  File %s"                   )  CRLF
                                                             CRLF
                        _T( "    File Id:           %17s" )  CRLF
                        _T( "    Job Name:          %17s" )  CRLF
                        _T( "    Step Name:         %17s" )  CRLF
                                                             CRLF
                        ,strFileNum

                        ,strFileId
                        ,strJob
                        ,strStep
                    );
                }
                else
                {
                    strFileStats.Format
                    (
                        _T( "  File %s"                   )  CRLF
                                                             CRLF
                        _T( "    File Id:           %17s" )  CRLF
                                                             CRLF
                        ,strFileNum

                        ,strFileId
                    );
                }
            }
            else
            {
                strFileStats.Format
                (
                    _T( "  File %s" )  CRLF
                                       CRLF
                    ,strFileNum
                );
            }
        }

        dlg.m_strDetails += strFileStats;

        // Calculate/format this file's statistics...

        nOverheadBytes       = file_stats.m_nTotalBytes - file_stats.m_nTotalChunkBytes;
        dOverheadPercent     = (double)nOverheadBytes / (double)file_stats.m_nTotalBytes;
        dPercentOriginalSize = (double)file_stats.m_nTotalBytes / (double)file_stats.m_nTotalDataBytes;

        strTotalBytes          . Format( _T( "%s" ), g_Locale.FormatNumber( file_stats.m_nTotalBytes       ) );
        strOverheadBytes       . Format( _T( "%s" ), g_Locale.FormatNumber(            nOverheadBytes      ) );
        strTotalChunkBytes     . Format( _T( "%s" ), g_Locale.FormatNumber( file_stats.m_nTotalChunkBytes  ) );
        strTotalChunks         . Format( _T( "%s" ), g_Locale.FormatNumber( file_stats.m_nTotalChunks      ) );
        strTotalDataBytes      . Format( _T( "%s" ), g_Locale.FormatNumber( file_stats.m_nTotalDataBytes   ) );
        strOverheadPercent     . Format( _T( "%s" ), g_Locale.FormatNumber(   100.0 * dOverheadPercent     ) );
        strPercentOriginalSize . Format( _T( "%s" ), g_Locale.FormatNumber(   100.0 * dPercentOriginalSize ) );

        strFileStats.Format
        (
            _T( "    Chunk count:       %17s"          )  CRLF
            _T( "    Uncompressed Size: %17s"          )  CRLF
            _T( "    Compressed Size:   %17s"          )  CRLF
            _T( "    Overhead Amount:   %17s  (%6s%%)" )  CRLF
            _T( "    Total File Size:   %17s  (%6s%%)" )  CRLF
                                                          CRLF
            ,strTotalChunks
            ,strTotalDataBytes
            ,strTotalChunkBytes
            ,strOverheadBytes    ,strOverheadPercent
            ,strTotalBytes       ,strPercentOriginalSize
        );

        dlg.m_strDetails += strFileStats;
    }

    // Format the accumulated totals for the entire tape volume...

    file_stats = tape_stats;

    nOverheadBytes       = file_stats.m_nTotalBytes - file_stats.m_nTotalChunkBytes;
    dOverheadPercent     = (double)nOverheadBytes / (double)file_stats.m_nTotalBytes;
    dPercentOriginalSize = (double)file_stats.m_nTotalBytes / (double)file_stats.m_nTotalDataBytes;

    strTotalBytes          . Format( _T( "%s" ), g_Locale.FormatNumber( file_stats.m_nTotalBytes       ) );
    strOverheadBytes       . Format( _T( "%s" ), g_Locale.FormatNumber(            nOverheadBytes      ) );
    strTotalChunkBytes     . Format( _T( "%s" ), g_Locale.FormatNumber( file_stats.m_nTotalChunkBytes  ) );
    strTotalChunks         . Format( _T( "%s" ), g_Locale.FormatNumber( file_stats.m_nTotalChunks      ) );
    strTotalDataBytes      . Format( _T( "%s" ), g_Locale.FormatNumber( file_stats.m_nTotalDataBytes   ) );
    strOverheadPercent     . Format( _T( "%s" ), g_Locale.FormatNumber(   100.0 * dOverheadPercent     ) );
    strPercentOriginalSize . Format( _T( "%s" ), g_Locale.FormatNumber(   100.0 * dPercentOriginalSize ) );

    CString  str_TOTALS_literal =
            _T( "  ---------------------------- Volume Totals --------------------------------------------" );

    if (!strVolSer.IsEmpty())
    {
        str_TOTALS_literal.Format
        (
            _T( "  ------------------------- Volume \"%s\" Totals ----------------------------------------" )

            ,strVolSer
        );
    }

    // (format file count...)

    CString  strFileCount;

    if (!m_arHDR1Names.IsEmpty())
    {
        // Standard Labeled tapes (containing stdlbl'ed files)
        // SHOULD consist of only stdlbl'ed files, and thus the
        // total number of tapemark-separated files SHOULD be
        // three (3) times the number of detected stdlbl'ed
        // (HDR1/EOF1-separated) files. Some tapes however are
        // mixed (containing some stdlbl'ed files and some non-
        // labeled files). Thus the following check only so we
        // can print a non-confusing "file count" for them...

        if (nFileCount == (3 * (int)m_arHDR1Names.GetCount()))
        {
            // As expected, the tape contains only stdlbl'ed files,
            // so use the #of HDR1 labels we saw as the file count...

            strFileCount.Format( _T( "%s" ), g_Locale.FormatNumber( m_arHDR1Names.GetCount() ) );
        }
        else
        {
            // Hmmm. File appears to contain non-labeled
            // files AND stdlbl'ed files together on one
            // tape. In order to prevent confusion we'll
            // say there's "x+y" files on the tape where
            // x is the number of stdlbl'ed files and y
            // is the number of other non-labeled files...

            int  nStdLbledFileCount     = (int)m_arHDR1Names.GetCount();
            int  nNONStdLbledFileCount  = nFileCount - (3 * nStdLbledFileCount);

            strFileCount.Format
            (
                _T( "%d+%d" )

                ,nStdLbledFileCount
                ,nNONStdLbledFileCount
            );
        }
    }
    else
    {
        strFileCount.Format( _T( "%s" ), g_Locale.FormatNumber( nFileCount ) );
    }

    // (print accumulated tape volume totals...)

    if (strCompressionType.IsEmpty())
        strCompressionString.Empty();
    else
        strCompressionString = _T("  ( ") + strCompressionType + _T(" )");

    strFileStats.Format
    (
                                                      CRLF
        _T( "%s"                                   )  CRLF
                                                      CRLF
        _T( "    Total Files:       %17s"          )  CRLF
        _T( "    Total Chunks:      %17s"          )  CRLF
        _T( "    Uncompressed Size: %17s"          )  CRLF
        _T( "    Compressed Size:   %17s%s"        )  CRLF
        _T( "    Overhead Amount:   %17s  (%6s%%)" )  CRLF
        _T( "    Total File Size:   %17s  (%6s%%)" )  CRLF

        ,str_TOTALS_literal

        ,strFileCount
        ,strTotalChunks
        ,strTotalDataBytes
        ,strTotalChunkBytes ,strCompressionString
        ,strOverheadBytes   ,strOverheadPercent
        ,strTotalBytes      ,strPercentOriginalSize
    );

    dlg.m_strDetails += strFileStats;

    // (format a reasonable window title for the dialog)

    if (strVolSer.IsEmpty())
    {
        CSplitPath  sp( m_pDoc->GetPathName() );
        CString  strFileNameAndExtOnly = sp.GetFileNameAndExt();

        dlg.m_strTitle.Format( _T( " File \"%s\" Summary" ), strFileNameAndExtOnly );
    }
    else
    {
        dlg.m_strTitle.Format( _T( " Volume \"%s\" Summary" ), strVolSer );
    }

    // FINALLY, display the dialog!  :)

    dlg.DoModal();
}

//////////////////////////////////////////////////////////////////////////////////////////

bool CMyListView::HaveGoto()
{
    return
    (1
        && m_pDoc
        && !m_pDoc->IsEmpty()
        && !m_arHDR1Names.IsEmpty()
    );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnUpdateEditGoto(CCmdUI *pCmdUI)
{
    pCmdUI->Enable( HaveGoto() );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyListView::OnEditGoto()
{
    if (!HaveGoto()) return;

    CGoToDlg  dlg(this);

    // Pass our HDR1 array values to the dialog...

    ASSERT( !m_arHDR1Names.IsEmpty() );
    ASSERT( m_nGoToEntry >= 0 && m_nGoToEntry <= m_arHDR1Names.GetUpperBound() );

    dlg.m_arFileNames.Append( m_arHDR1Names );
    dlg.m_nSelectedEntry = m_nGoToEntry;

    // Display dialog, wait for response...

    if (dlg.DoModal() != IDOK)
        return;

    // Retrieve results...

    m_nGoToEntry = dlg.m_nSelectedEntry;

    ASSERT( m_nGoToEntry >= 0 && m_nGoToEntry <= m_arHDR1Names.GetUpperBound() );

    CString  strHDR1Name  = m_arHDR1Names     [ m_nGoToEntry ];
    int      nChunkNum    = m_arHDR1ChunkNums [ m_nGoToEntry ];

    TRACE( _T( "** OnEditGoto: m_nGoToEntry %d == chunk %d == \"%s\"\n" ),
        m_nGoToEntry, nChunkNum, strHDR1Name );

    // Since 'nChunkNum' at this point is actually just
    // the tape chunk# of the chunk that the HDR1 label
    // was found in, we need to search forward until we
    // find the tapemark chunk that separates the stdlbls
    // from the actual file itself, and then go on to
    // the next chunk (immediately following the tapemark)
    // in order to reach the first chunk of the actual
    // stdlbl file they're actually interested in...

    int  nOrigChunkNum  = nChunkNum;   // (in case things go wrong)
    int  nNumChunks     = m_pDoc->NumChunks();

    CAWSChunkInfo  chunk_info;

    bool bFound = false;

    while (++nChunkNum < nNumChunks)
    {
        chunk_info = m_pDoc->ChunkInfo( nChunkNum );

        if (chunk_info.IsTapeMark())
        {
            if ((nChunkNum+1) < nNumChunks)
            {
                nChunkNum++;
                bFound = true;
            }
            break;
        }
    }

    if (!bFound)
        nChunkNum = nOrigChunkNum;

    // Locate list control item with that chunk#...

    LVFINDINFO  info;
    int         nItemNum;

    info.flags  = LVFI_PARAM;
    info.lParam = nChunkNum;

    nItemNum = m_pListCtrl->FindItem( &info );

    TRACE( _T( "** OnEditGoto: nItemNum %d == chunk %d == \"%s\"\n" ),
        nItemNum, nChunkNum, strHDR1Name );

    ASSERT( -1 != nItemNum );

    // Now select it into focus to show it to them...

    // For user friendliness, we'll make an attempt
    // to position the desired listview control item
    // approximately 1/4 of the way down in the
    // listview's list of items...  (I hate it when
    // the item I jump to is the very last line or
    // very first line on the screen! It makes more
    // sense to have it somewhere in the middle of
    // the screen. Thus 1/4 of the way down seems
    // to be a nice user-friendly position to me).

    if ( nItemNum >= 0 )
    {
        m_pListCtrl->LockWindowUpdate();
        {
            // First, select the item to bring it into focus
            // and to scroll it into view...

            SelectItem( nItemNum );

            // Next, retrieve the index of the first visible
            // item on the screen as well as how many items
            // are visible on the screen. These two values
            // together tell us where our particular item is
            // currently positioned and thus how far we'll
            // have to scroll the view...

            int  nCurrTopIndex  = m_pListCtrl->GetTopIndex();
            int  nItemsPerPage  = m_pListCtrl->GetCountPerPage();
            int  nNewTopIndex   = nItemNum - (nItemsPerPage / 4);

            if (nNewTopIndex < 0)
                nNewTopIndex = 0;

            int nNumLinesToScroll = nNewTopIndex - nCurrTopIndex;

            // Since the amount we need to ask the listctrl
            // to scroll must be specified in pixels (sigh!),
            // we need to convert the the #of lines into #of
            // pixels... (stupid, yes, but that's Microsoft)

            CRect rectItem;
            VERIFY( m_pListCtrl->GetItemRect( nItemNum, &rectItem, LVIR_BOUNDS ) );

            // FINALLY we get to do the scroll...

            CSize  szScrollAmt( 0, rectItem.Height() * nNumLinesToScroll );
            VERIFY( m_pListCtrl->Scroll( szScrollAmt ) );
        }
        m_pListCtrl->UnlockWindowUpdate();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
