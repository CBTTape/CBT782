// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// MyHexEditView.cpp : implementation file
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  03/19/06    1.3.0   Fish    Support toggling display of Hex view on or off
//  06/15/06    1.5.0   Fish    VS2005, x64
//  08/04/06    1.4.0   Fish    SetInitialHexEditParms: use HEM_BASE32 rather than
//                              HEM_BASE16 in m_HexEditParms for block size > 64K.
//  12/09/06    1.5.0   Fish    Use new CMyFontDialog
//  01/14/07    1.5.0   Fish    Changes needed for new FishLib
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyHexEditView.h"
#include "HEPARMS.h"
#include "AWSBrowseDoc.h"
#include "SettingsDialog.h"
#include "ClipBoard.h"
#include "MyFontDialog.h"
#include "MyListView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (embed copyright into object code)
#pragma comment ( user, COPYRIGHT_EXESTR )

//////////////////////////////////////////////////////////////////////////////////////////
// Programming Note: Scroll bars don't work properly unless you first create the both
// the "edit" and hex-edit controls WITHOUT them, and then AFTERWARDS call the hex-edit
// control's "EnableScrollBars" function...

#define  DEF_HEXEDIT_STYLE   \
(0                       \
    |  WS_CHILD          \
    |  WS_VISIBLE        \
    |  WS_GROUP          \
    |  WS_TABSTOP        \
    |  ES_MULTILINE      \
    |  ES_UPPERCASE      \
    |  ES_READONLY       \
    |  ES_NOHIDESEL      \
    |  ES_OEMCONVERT     \
)

//////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE( CMyHexEditView, CView )

BEGIN_MESSAGE_MAP( CMyHexEditView, CView )

    //{{AFX_MSG_MAP(CMyHexEditView)
    ON_WM_CREATE()
    ON_WM_ERASEBKGND()
    ON_WM_SETFOCUS()
    ON_WM_SIZE()
    ON_COMMAND(ID_EDIT_HEXAREA, OnEditHexArea)
    ON_COMMAND(ID_EDIT_TEXTAREA, OnEditTextArea)
    ON_COMMAND(ID_VIEW_SETTINGS, OnViewSettings)
    ON_COMMAND(ID_VIEW_ASCII, OnViewAscii)
    ON_COMMAND(ID_VIEW_EBCDIC, OnViewEbcdic)
    ON_COMMAND(ID_VIEW_DISPLAYFONT, OnViewDisplayFont)
    ON_COMMAND(ID_VIEW_HEX, OnViewHex)
    ON_UPDATE_COMMAND_UI(ID_EDIT_GOTO, OnUpdateEditGoto)
    ON_COMMAND(ID_EDIT_GOTO, OnEditGoto)
    //}}AFX_MSG_MAP

    ON_COMMAND ( ID_EDIT_COPY,       OnEditCopy      )
    ON_COMMAND ( ID_EDIT_SELECT_ALL, OnEditSelectAll )

    ON_MESSAGE( MY_WM_REFRESH_HEXEDIT_VIEW, OnMyRefreshViewMsg )

END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::OnUpdateEditGoto(CCmdUI *pCmdUI)
{
    pCmdUI->Enable( GetDocument()->m_pListView->HaveGoto() );
}

//////////////////////////////////////////////////////////////////////////////////////////
// (admittedly poor form but who cares... whatever works...)

void CMyHexEditView::OnEditGoto()
{
    // (pass "GoTo.." command to our list view for processing...)
    CWnd* pWnd = (CWnd*) GetDocument()->m_pListView;
    pWnd->SendMessage( WM_COMMAND, ID_EDIT_GOTO, 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::OnEditSelectAll() { m_HexEditCtrl.SetSel(0, -1); }

//////////////////////////////////////////////////////////////////////////////////////////

CMyHexEditView::CMyHexEditView()
{
    m_cpDefault      = DEF_HEXEDIT_DEF_CODEPAGE;
    m_cpEBCDIC       = DEF_HEXEDIT_TEXT_CODEPAGE;
    m_bEBCDICMode    = DEF_HEXEDIT_EBCDIC_MODE;
    m_bViewHex       = DEF_HEXEDIT_VIEWHEX;

    m_nBytesPerGroup = DEF_HEXEDIT_BPG;
    m_nBytesPerLine  = DEF_HEXEDIT_BPR;
    m_bDynBPR        = DEF_HEXEDIT_DYNBPR;
    m_nIndent        = DEF_HEXEDIT_INDENT;

    m_strFontName    = DEF_HEXEDIT_FONTFACE;
    m_nFontSize      = DEF_HEXEDIT_FONTSIZE;

    m_crTextColor    = DEF_HEXEDIT_TEXT_COLOR;
    m_crBGColor      = DEF_HEXEDIT_BG_COLOR;

    m_crHiTextColor  = DEF_HEXEDIT_HI_TEXT_COLOR;
    m_crHiBGColor    = DEF_HEXEDIT_HI_BG_COLOR;

    m_nTextRecLen    = DEF_HEXEDIT_TEXTONLY_RECLEN;

    LoadProfileSettings();      // (defaults must be set before calling!)

    CreateLogFont();            // (does NOT update hexedit control
                                //  since it hasn't been created yet)
}

//////////////////////////////////////////////////////////////////////////////////////////

CMyHexEditView::~CMyHexEditView()
{
    SaveProfileSettings();
}

//////////////////////////////////////////////////////////////////////////////////////////
// (defaults must be set before calling!)

void CMyHexEditView::LoadProfileSettings()
{
    m_nBytesPerGroup = g_App.GetProfileInt( _T( "Settings" ), _T( "BytesPerGroup" ), m_nBytesPerGroup );
    m_nBytesPerLine  = g_App.GetProfileInt( _T( "Settings" ), _T( "BytesPerLine"  ), m_nBytesPerLine  );
    m_bDynBPR        = g_App.GetProfileInt( _T( "Settings" ), _T( "DynamicBPR"    ), m_bDynBPR        );
    m_bEBCDICMode    = g_App.GetProfileInt( _T( "Settings" ), _T( "EBCDICMode"    ), m_bEBCDICMode    );
    m_bViewHex       = g_App.GetProfileInt( _T( "Settings" ), _T( "ViewHex"       ), m_bViewHex       );
    m_nTextRecLen    = g_App.GetProfileInt( _T( "Settings" ), _T( "TextRecLen"    ), m_nTextRecLen    );

    m_strFontName  =  g_App . GetProfileString ( _T( "Settings" ), _T( "DisplayFontName" ), m_strFontName );
    m_nFontSize    =  g_App . GetProfileInt    ( _T( "Settings" ), _T( "DisplayFontSize" ), m_nFontSize   );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::SaveProfileSettings()
{
    g_App.WriteProfileInt( _T( "Settings" ), _T( "BytesPerGroup" ), m_nBytesPerGroup );
    g_App.WriteProfileInt( _T( "Settings" ), _T( "BytesPerLine"  ), m_nBytesPerLine  );
    g_App.WriteProfileInt( _T( "Settings" ), _T( "DynamicBPR"    ), m_bDynBPR        );
    g_App.WriteProfileInt( _T( "Settings" ), _T( "EBCDICMode"    ), m_bEBCDICMode    );
    g_App.WriteProfileInt( _T( "Settings" ), _T( "ViewHex"       ), m_bViewHex       );
    g_App.WriteProfileInt( _T( "Settings" ), _T( "TextRecLen"    ), m_nTextRecLen    );

    g_App . WriteProfileString ( _T( "Settings" ), _T( "DisplayFontName" ), m_strFontName );
    g_App . WriteProfileInt    ( _T( "Settings" ), _T( "DisplayFontSize" ), m_nFontSize   );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::SetHexView( BOOL bEnable /* =TRUE */ )
{
    GetHexEditParms();          // (retrieve current settings)

    m_bViewHex             = bEnable;
    m_HexEditParms.dwMask  = HEM_OPTIONS;

    if (m_bViewHex)
    {
        m_HexEditParms.dwOptions      |=   (HEO_HEXAREA | HEO_ADRAREA);

        if (m_bDynBPR)
            m_HexEditParms.dwOptions  |=   HEO_DYNBPR;

        m_HexEditParms.dwMask         |=   HEM_BPR;
        m_HexEditParms.nBytesPerRow    =   m_nBytesPerLine;
    }
    else
    {
        m_HexEditParms.dwOptions      &=  ~(HEO_HEXAREA | HEO_ADRAREA);
        m_HexEditParms.dwOptions      &=  ~HEO_DYNBPR;

        m_HexEditParms.dwMask         |=   HEM_BPR;
        m_HexEditParms.nBytesPerRow    =   m_nTextRecLen;
    }

    m_HexEditCtrl.SetRedraw( FALSE );
    {
        VERIFY( m_HexEditCtrl.SetParms( &m_HexEditParms ) );
        if ( m_bEBCDICMode )  SetEBCDICMode();
        else                  SetASCIIMode();
    }
    m_HexEditCtrl.SetRedraw( TRUE );

    m_HexEditCtrl.Invalidate(); // (refresh screen)
    GetHexEditParms();          // (keeps our parms in sync with its)
    SaveProfileSettings();      // (save our new settings)
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::SetASCIIMode()
{
    GetHexEditParms();          // (retrieve current settings)

    m_bEBCDICMode         = FALSE;
    m_HexEditParms.dwMask = HEM_CODEPAGES;
    m_HexEditParms.cpText = ASCII_CODE_PAGE;

    VERIFY( m_HexEditCtrl.SetParms( &m_HexEditParms ) );

    m_HexEditCtrl.Invalidate(); // (refresh screen)
    GetHexEditParms();          // (keeps our parms in sync with its)
    SaveProfileSettings();      // (save our new settings)
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::SetEBCDICMode()
{
    GetHexEditParms();          // (retrieve current settings)

    m_bEBCDICMode         = TRUE;
    m_HexEditParms.dwMask = HEM_CODEPAGES;
    m_HexEditParms.cpText = EBCDIC_CODE_PAGE;

    VERIFY( m_HexEditCtrl.SetParms( &m_HexEditParms ) );

    m_HexEditCtrl.Invalidate(); // (refresh screen)
    GetHexEditParms();          // (keeps our parms in sync with its)
    SaveProfileSettings();      // (save our new settings)
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::SetBPG( BYTE nBPG )
{
    GetHexEditParms();          // (retrieve current settings)

    m_nBytesPerGroup              = nBPG;
    m_HexEditParms.dwMask         = HEM_BPG;
    m_HexEditParms.nBytesPerGroup = m_nBytesPerGroup;

    VERIFY( m_HexEditCtrl.SetParms( &m_HexEditParms ) );

    m_HexEditCtrl.Invalidate(); // (refresh screen)
    GetHexEditParms();          // (keeps our parms in sync with its)
    SaveProfileSettings();      // (save our new settings)
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::SetBPR( BYTE nBPR )
{
    ASSERT( !m_bDynBPR );       // (sanity check; setting BPR insane if Dynamic BPR enabled)

    GetHexEditParms();          // (retrieve current settings)

    ASSERT( ( m_HexEditParms.dwMask & HEM_OPTIONS ) && !( m_HexEditParms.dwOptions & HEO_DYNBPR  ) );

    SetDynBPR( m_bDynBPR = FALSE ); // (since we're setting a specific BPR setting)

    m_nBytesPerLine               =   nBPR;
    m_HexEditParms.dwMask         =   HEM_BPR;
    m_HexEditParms.nBytesPerRow   =   m_nBytesPerLine;

    VERIFY( m_HexEditCtrl.SetParms( &m_HexEditParms ) );

    m_HexEditCtrl.Invalidate(); // (refresh screen)
    GetHexEditParms();          // (keeps our parms in sync with its)
    SaveProfileSettings();      // (save our new settings)
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::SetDynBPR( BOOL bDynBPR /* =TRUE */ )
{
    // PROGRAMMING NOTE: flip-flopping the Dynamic BPR setting is tricky. The Dynamic
    // BPR option is an "options" flag setting, and thus we must set HEM_OPTIONS to
    // indicate we wish to change one of our options flags, but there's currently no
    // way to change just one specific options flag. When the HEM_OPTIONS mask flag
    // is set, it indicates ALL of the options flags are being changed. Thus we must
    // make darn sure that none of our OTHER options get accidentally changed/reset
    // by first retrieving all of our current options flags and then clearing and/or
    // setting the only one(s) we're interested in changing...

    GetHexEditParms();  // (retrieve current options flags)

    m_bDynBPR = bDynBPR;

    if ( m_bDynBPR )
    {
        // Turn the Dynamic BPR option flag back ON,
        // and leave all other options flags alone.

        m_HexEditParms.dwMask      =   HEM_OPTIONS;     // (we're changing our options flags)
        m_HexEditParms.dwOptions  |=   HEO_DYNBPR;      // (turn this option ON)
    }
    else
    {
        // Turn the Dynamic BPR option OFF (and leave all other options flags alone).
        // Also make sure the normal BPR (byte-per-row (i.e. bytes-per-line)) SETTING
        // gets set back to what it was before too by making sure the HEM_BPR settings
        // mask flag is set (i.e. the BPR setting is not an option flag, but rather is
        // a SETTING mask flag instead, so we need to make sure the HEM_BPR *mask* flag
        // gets set in our request so the control will accept our 'new' BPR value...)

        m_HexEditParms.dwMask         =   HEM_OPTIONS | HEM_BPR;    // (need both!)
        m_HexEditParms.dwOptions     &=  ~HEO_DYNBPR;               // (turn this option OFF)
        m_HexEditParms.nBytesPerRow   =   m_nBytesPerLine;          // (new SETTINGS value)
    }

    VERIFY( m_HexEditCtrl.SetParms( &m_HexEditParms ) );

    m_HexEditCtrl.Invalidate(); // (refresh screen)
    GetHexEditParms();          // (keeps our parms in sync with its)
    SaveProfileSettings();      // (save our new settings)
}

//////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CMyHexEditView::AssertValid() const
{
    CView::AssertValid();
}

void CMyHexEditView::Dump( CDumpContext& dc ) const
{
    CView::Dump( dc );
}

CAWSBrowseDoc* CMyHexEditView::GetDocument() // (non-debug version is inline)
{
    ASSERT( m_pDocument->IsKindOf( RUNTIME_CLASS( CAWSBrowseDoc ) ) );
    return (CAWSBrowseDoc*) m_pDocument;
}
#endif //_DEBUG

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CMyHexEditView::PreCreateWindow( CREATESTRUCT& cs )
{
    cs.style |= DEF_HEXEDIT_STYLE;
    return CView ::PreCreateWindow( cs );
}

//////////////////////////////////////////////////////////////////////////////////////////

int CMyHexEditView::OnCreate( CREATESTRUCT* pCS)
{
    if ( CView::OnCreate( pCS ) == -1 )
        return -1;

    if ( !m_HexEditCtrl.Create
    (
        _T( "edit" ),               // (window CLASS name for std edit control)
        pCS->lpszName,              // (the name of this specific window)
        DEF_HEXEDIT_STYLE,
        CRect( CPoint( pCS->x, pCS->y ), CSize( pCS->cx, pCS->cy ) ),
        this,
        IDC_MY_HEX_EDIT_CONTROL,
        (CCreateContext*) pCS->lpCreateParams   // CCreateContext* pContext (HOPEFULLY anyway..)
    ))
    {
        TRACE( _T( "** CMyHexEditView::OnCreate: Failed to create m_HexEditCtrl!\n" ) );
        ASSERT(false);
        return -1;
    }

    GetDocument()->m_pHexEditView = this;   // (we exist now)

    SetInitialHexEditParms();

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CMyHexEditView::OnEraseBkgnd( CDC* pDC )
{
    // Since our control covers our entire client area,
    // there's really nothing for us to erase...
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::OnDraw( CDC* pDC )
{
    CRect rectVisible; pDC->RectVisible( &rectVisible );
    m_HexEditCtrl.InvalidateRect( &rectVisible );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::OnSetFocus( CWnd* pOldWnd )
{
    CView::OnSetFocus( pOldWnd );
    m_HexEditCtrl.SetFocus();
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::OnSize( UINT nType, int cx, int cy )
{
    CView::OnSize( nType, cx, cy );
    CRect rectClient;
    GetClientRect( rectClient );
    m_HexEditCtrl.MoveWindow( rectClient );

    // If the Dynamic BPR option is enabled, then if the window was previously
    // resized skinnier than what would otherwise be needed to display a line
    // containing a full 'group' (GROUP, not line!) of data, then the control
    // automatically adjusts the BPG (bytes-per-group) setting to a smaller value
    // such that a full "group" can thus fit on the new [skinnier] line. If that
    // happened, then the current BPG (bytes-per-group) setting the control is
    // currently using might not match what we ultimately want it to be set at.
    // Thus we always reset the BPG setting after a resize...

    m_HexEditCtrl.SetBPG( m_nBytesPerGroup );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Refresh current hexedit control settings...

void CMyHexEditView::GetHexEditParms()
{
    ZeroMemory( &m_HexEditParms, NUM_BYTES( m_HexEditParms ) );
    m_HexEditParms.dwSize = NUM_BYTES( HEPARMS );
    VERIFY( m_HexEditCtrl.GetParms( &m_HexEditParms ) );

    // PROGRAMMING NOTE: we keep our BPR (bytes-per-line) setting in sync with
    // whatever the hexedit control is currently set to if the Dynamic BPR option
    // is enabled. That way whenever they go to turn Dynamic BPR off, they can
    // start with whatever BPR value the control WAS using when it was on...

    if ( m_bDynBPR )
        m_nBytesPerLine = m_HexEditParms.nBytesPerRow;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::SetInitialHexEditParms()
{
    m_HexEditCtrl.EnableScrollBars();

    GetHexEditParms();  // (retrieve current settings)

    m_HexEditParms.dwMask  &=  ~( HEM_BASE16 | HEM_BASE32 | HEM_BASE64 );

    m_HexEditParms.dwMask   =
        0
        | HEM_OPTIONS
        | HEM_COLORS
        | HEM_HICOLORS
//      | HEM_DISABCLRS         // (use default)
//      | HEM_DISABHICLRS       // (use default)
        | HEM_LOGFONT
//      | HEM_DATA              // (no data yet)
//      | HEM_BASE16            // (because HEM_BASE32)
        | HEM_BASE32            // (for support of block sizes > 64K)
//      | HEM_BASE64            // (because HEM_BASE32)
//      | HEM_BPR               // (set further below)
        | HEM_BPG
        | HEM_CODEPAGES
//      | HEM_UNDOLIMIT         // (not needed since read-only)
        | HEM_INDENT
        ;

    m_HexEditParms.dwOptions  =
        0
        | HEO_ADRAREA
//      | HEO_HEXAREA           // (set further below)
        | HEO_TXTAREA
//      | HEO_DYNBPR            // (set further below)
        | HEO_VSCROLL
        | HEO_HSCROLL
        | HEO_CARET
        | HEO_ESCKEY
//      | HEO_CTRLHSCROLL       // (we don't want this)
        | HEO_READONLY
        | HEO_INSMODE
        | HEO_NOHIDESEL
        | HEO_NOFINDREP
        | HEO_NOREPLACE
        | HEO_NONOTFOUND
        ;

    m_HexEditParms.clrText                =   m_crTextColor;
    m_HexEditParms.clrBack                =   m_crBGColor;
    m_HexEditParms.clrHighlightText       =   m_crHiTextColor;
    m_HexEditParms.clrHighlightBack       =   m_crHiBGColor;

    if (m_bViewHex)
    {
        m_HexEditParms.dwOptions         |=   (HEO_HEXAREA | HEO_ADRAREA);

        if ( m_bDynBPR )
        {
            m_HexEditParms.dwOptions     |=   HEO_DYNBPR;
            m_HexEditParms.dwMask        &=  ~HEM_BPR;
            m_HexEditParms.nBytesPerRow   =   0;
        }
        else
        {
            m_HexEditParms.dwOptions     &=  ~HEO_DYNBPR;
            m_HexEditParms.dwMask        |=   HEM_BPR;
            m_HexEditParms.nBytesPerRow   =   m_nBytesPerLine;
        }
    }
    else
    {
        m_HexEditParms.dwOptions         &=  ~(HEO_HEXAREA | HEO_ADRAREA);
        m_HexEditParms.dwOptions         &=  ~HEO_DYNBPR;
        m_HexEditParms.dwMask            |=   HEM_BPR;
        m_HexEditParms.nBytesPerRow       =   m_nTextRecLen;
    }

    m_HexEditParms.nBase                  =   0;
    m_HexEditParms.nMaxLength             =   0;
    m_HexEditParms.nBytesPerGroup         =   m_nBytesPerGroup;
    m_HexEditParms.cpDefault              =   m_cpDefault;

    if ( m_bEBCDICMode )
        m_HexEditParms.cpText             =   m_cpEBCDIC;
    else
        m_HexEditParms.cpText             =   m_cpDefault;

    m_HexEditParms.nIndent                =   m_nIndent;
    m_HexEditParms.pLogFont               =  &m_LogFont;

    VERIFY( m_HexEditCtrl.SetParms( &m_HexEditParms ) );

    CreateLogFont();    // (also updates hexedit control)
    GetHexEditParms();  // (keeps our parms in sync with its)
}

//////////////////////////////////////////////////////////////////////////////////////////
// The list view is notifying us that the user clicked on a new (different) list entry
// corresponding to a different AWS/HET "tape" file block. We need to retrieve the data
// for this block and present it to the user...

LRESULT CMyHexEditView::OnMyRefreshViewMsg( WPARAM nChunkNum, LPARAM )
{
    BYTE*   pCompressedData  = NULL;    // raw compressed data
    int     nCompressedLen   = 0;       // raw compressed len
    BYTE*   pExpandedData    = NULL;    // expanded data block
    int     nExpandedLen     = 0;       // expanded block len

    CAWSBrowseDoc*  pDoc  =  GetDocument();

    if ( nChunkNum != (WPARAM) -1 && !pDoc->IsEmpty() )
    {
        // (note: expansion errors transparently handled further below)

        pDoc->GetBlockData
        (
            (int)nChunkNum,
            pCompressedData,
            nCompressedLen,
            pExpandedData,      // (could be returned as NULL if expansion error)
            nExpandedLen        // (could be returned as zero if expansion error)
        );
    }

    BOOL bTextArea = IsTextArea();

    if ( !bTextArea && !m_bViewHex )
    {
        if (!pDoc->IsEmpty())
            ASSERT( false );    // (oops! got them out-of-sync with one another!)
        bTextArea = TRUE;       // (forced since hex area not visible)
    }

    GetHexEditParms();          // (retrieve current settings)

    // (note: use compressed data & len if any expansion error occurred)

    m_HexEditParms.dwMask      =  HEM_DATA;
    m_HexEditParms.pData       =  pExpandedData ? pExpandedData : pCompressedData;
    m_HexEditParms.nLength     =  nExpandedLen  ? nExpandedLen  : nCompressedLen;
    m_HexEditParms.nMaxLength  =  -1;

    m_HexEditCtrl.SetRedraw( FALSE );
    {
        VERIFY( m_HexEditCtrl.SetParms( &m_HexEditParms ) );
        VERIFY( m_HexEditCtrl.SetSelEx( 0, 0, bTextArea ) );
    }
    m_HexEditCtrl.SetRedraw( TRUE );

    m_HexEditCtrl.Invalidate(); // (refresh screen)
    GetHexEditParms();          // (keeps our parms in sync with its)

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CMyHexEditView::IsTextArea()
{
    BOOL bTextArea; int nSelStart, nSelEnd;
    m_HexEditCtrl.GetSelEx( nSelStart, nSelEnd, bTextArea );
    return bTextArea;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::SetTextArea( BOOL bTextArea /* =TRUE */ )
{
    // (sanity check: if not text area then hex area must be visible)

    if ( !bTextArea && !m_bViewHex )
    {
        ASSERT( false );    // (oops! got them out-of-sync with one another!)
        bTextArea = TRUE;
    }

    int nSelStart, nSelEnd; m_HexEditCtrl.GetSel( nSelStart, nSelEnd );
    m_HexEditCtrl.SetSelEx( nSelStart, nSelEnd, bTextArea, TRUE );
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CMyHexEditView::IsDataSelected()
{
    int nSelStart, nSelEnd;
    m_HexEditCtrl.GetSel( nSelStart, nSelEnd );
    return ( nSelStart != nSelEnd ) ? TRUE : FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CAUTION! The following function does a 'new'. Caller is
// responsible for doing the 'delete' to prevent memory leak!

BOOL CMyHexEditView::GetSelectedData( BYTE*& pSelectedData, SSIZE_T& nSelectedDataLen )
{
    pSelectedData = NULL; nSelectedDataLen = 0;

    // Retrieve the current selection

    SSIZE_T nSelStart, nSelEnd; m_HexEditCtrl.GetSel( nSelStart, nSelEnd );

    if ( nSelStart == nSelEnd )
        return FALSE;   // (nothing selected)

    m_HexEditCtrl.NormalizeSel( nSelStart, nSelEnd );

    // Retrieve the current data block data...

    BYTE* pBlockData = new BYTE [ g_dwMaxBlockSize ];   // (temp buffer)

    SSIZE_T nBlockLen = m_HexEditCtrl.GetData( pBlockData, g_dwMaxBlockSize );
    ASSERT( nBlockLen >= 0 && nBlockLen <= (int)g_dwMaxBlockSize );
    if ( nBlockLen < 0 ) nBlockLen = 0;
    if ( nBlockLen > (int)g_dwMaxBlockSize ) nBlockLen = (int)g_dwMaxBlockSize;

    // NOTE: nSelEnd MAY point PAST the last byte of data
    // if the cursor is currently positioned "at the end"

    if ( nSelStart <     0      ) nSelStart =     0;
    if ( nSelStart > nBlockLen  ) nSelStart = nBlockLen;

    if ( nSelEnd   <     0      ) nSelEnd   =     0;
    if ( nSelEnd   > nBlockLen  ) nSelEnd   = nBlockLen;

    ASSERT
    (1
        &&  nSelStart >= 0  &&  nSelStart <= nBlockLen
        &&  nSelEnd   >= 0  &&  nSelEnd   <= nBlockLen
        &&  (nSelEnd - nSelStart) > 0
    );

    nSelectedDataLen = nSelEnd - nSelStart;
    ASSERT( nSelectedDataLen && nSelectedDataLen <= nBlockLen );

    // Allocate a data buffer for the caller
    // and then copy the retrieved data into it...

    // NOTE: CALLER RESPONSIBLE FOR DOING 'delete' TO PREVENT MEMORY LEAK!

    pSelectedData = new BYTE [ nSelectedDataLen ];
    memcpy( pSelectedData, pBlockData + nSelStart, nSelectedDataLen );

    delete [] pBlockData;   // (discard temp buffer)

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//           C o m m a n d   P r o c e s s i n g   F u n c t i o n s
//
//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::OnEditCopy()
{
    if ( !IsDataSelected() ) return;
    m_HexEditCtrl.Copy();
    if (m_bViewHex) return;

    // Format the text into multiple fixed-length strings
    // since the hexedit control doesn't do that for them...

    CClipBoard  clipBoard;
    CString     strOldClipText, strNewClipText;

    strOldClipText << clipBoard;        // (get copy of clipboard data)

    while (!strOldClipText.IsEmpty())
    {
        UINT nLen = strOldClipText.GetLength();

        if (nLen > m_nTextRecLen)
            nLen = m_nTextRecLen;

        strNewClipText += strOldClipText.Left( nLen );

        if (nLen >= m_nTextRecLen)
            strNewClipText += _T("\r\n");

        strOldClipText = strOldClipText.Mid( nLen );
    }

    clipBoard << strNewClipText;        // (put string back onto clipboard)
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::OnEditHexArea()
{
    SetTextArea( FALSE );
}

void CMyHexEditView::OnEditTextArea()
{
    SetTextArea( TRUE );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::OnViewHex()
{
    SetHexView( !m_bViewHex );
}

void CMyHexEditView::OnViewAscii()
{
    SetASCIIMode();
}

void CMyHexEditView::OnViewEbcdic()
{
    SetEBCDICMode();
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::OnViewSettings()
{
    GetHexEditParms();  // (retrieve current settings)

    CSettingsDialog  dlg;

    dlg.m_nBPG        = m_nBytesPerGroup;
    dlg.m_nBPR        = m_nBytesPerLine;
    dlg.m_bDynBPR     = m_bDynBPR;
    dlg.m_nFormat     = m_bEBCDICMode ? EBCDIC_FORMAT : ASCII_FORMAT;
    dlg.m_nTextRecLen = m_nTextRecLen;

    if ( IDOK != dlg.DoModal() )
        return;

    m_nBytesPerGroup = dlg.m_nBPG;
    m_nBytesPerLine  = dlg.m_nBPR;
    m_bDynBPR        = dlg.m_bDynBPR;
    m_bEBCDICMode    = ( EBCDIC_FORMAT == dlg.m_nFormat ) ? true : false;
    m_nTextRecLen    = dlg.m_nTextRecLen;

    m_HexEditCtrl.SetRedraw( FALSE );
    {
        SetDynBPR( m_bDynBPR );                 // (turn this option on or off as needed)
        if ( !m_bDynBPR)                        // (if the dynamic BPR option was turned off,
            SetBPR( m_nBytesPerLine );          //  then we also need to (re-)set value too)
        SetBPG( m_nBytesPerGroup );             // (we always do this just in case...)
        if ( m_bEBCDICMode )  SetEBCDICMode();  // (and this...
        else                  SetASCIIMode();   //  ...or this too)
        SetHexView( m_bViewHex );               // (as well as this)
    }
    m_HexEditCtrl.SetRedraw( TRUE );

    m_HexEditCtrl.Invalidate(); // (refresh screen)
    GetHexEditParms();          // (keeps our parms in sync with its)
    SaveProfileSettings();      // (save our new settings)
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::OnViewDisplayFont()
{
    LOGFONT  lfFont;

    memcpy( &lfFont, &m_LogFont, NUM_BYTES( LOGFONT ) );

    DWORD dwFlags =
        0
        | CF_INITTOLOGFONTSTRUCT
        | CF_SCREENFONTS
        | CF_FIXEDPITCHONLY
        ;

    CMyFontDialog  dlg( _T("Display Font"), &lfFont, dwFlags, NULL, this );

    if ( IDOK != dlg.DoModal() ) return;

    memcpy( &m_LogFont, &lfFont, NUM_BYTES( LOGFONT ) );

    m_strFontName = dlg.GetFaceName();
    m_nFontSize   = dlg.GetSize() / 10;

    CreateLogFont();            // (also updates hexedit control)
    m_HexEditCtrl.Invalidate(); // (refresh screen)
    GetHexEditParms();          // (keeps our parms in sync with its)
    SaveProfileSettings();      // (save our new settings)
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyHexEditView::CreateLogFont()
{
    CFont   font;
    VERIFY( font.CreatePointFont( m_nFontSize * 10, m_strFontName ) );
    VERIFY( font.GetLogFont( &m_LogFont ) );

    if ( !IsWindow( m_HexEditCtrl.GetSafeHwnd() ) ) return;

    m_HexEditCtrl.SetHexFont( &m_LogFont );
}

//////////////////////////////////////////////////////////////////////////////////////////
