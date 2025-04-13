// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// MainFrm.cpp : implementation of the CMainFrame class
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  03/19/06    1.3.0   Fish    Support toggling display of Hex view on or off
//  06/15/06    1.5.0   Fish    VS2005, x64
//  08/04/06    1.5.0   Fish    Report FishLib DLL version information too
//  12/07/06    1.5.0   Fish    UI bug fix: File->Print and File->Print Preview
//                              menu selections shouldn't be allowed for tapemarks
//  02/06/07    1.5.0   Fish    BCMenu
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainFrm.h"
#include "MyListView.h"
#include "MyHexEditView.h"
#include "AWSBrowseDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (embed copyright into object code)
#pragma comment ( user, COPYRIGHT_EXESTR )

//////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE( CMainFrame, CFrameWnd )

BEGIN_MESSAGE_MAP( CMainFrame, CFrameWnd )

    //{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
    ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateEditFind)
    ON_COMMAND(ID_EDIT_FIND, OnEditFind)
    ON_UPDATE_COMMAND_UI(ID_VIEW_ASCII, OnUpdateViewAscii)
    ON_COMMAND(ID_VIEW_ASCII, OnViewAscii)
    ON_UPDATE_COMMAND_UI(ID_VIEW_EBCDIC, OnUpdateViewEbcdic)
    ON_COMMAND(ID_VIEW_EBCDIC, OnViewEbcdic)
    ON_UPDATE_COMMAND_UI(ID_EDIT_FINDNEXT, OnUpdateEditFindNext)
    ON_COMMAND(ID_EDIT_FINDNEXT, OnEditFindNext)
    ON_UPDATE_COMMAND_UI(ID_EDIT_FINDPREVIOUS, OnUpdateEditFindPrevious)
    ON_COMMAND(ID_EDIT_FINDPREVIOUS, OnEditFindPrevious)
    ON_UPDATE_COMMAND_UI(ID_EDIT_HEXAREA, OnUpdateEditHexArea)
    ON_UPDATE_COMMAND_UI(ID_EDIT_TEXTAREA, OnUpdateEditTextArea)
    ON_WM_CLOSE()
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_UPDATE_COMMAND_UI(ID_EDIT_NEXTBLOCK, OnUpdateEditNextBlock)
    ON_COMMAND(ID_EDIT_NEXTBLOCK, OnEditNextBlock)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PREVIOUSBLOCK, OnUpdateEditPreviousBlock)
    ON_COMMAND(ID_EDIT_PREVIOUSBLOCK, OnEditPreviousBlock)
    ON_UPDATE_COMMAND_UI(ID_EDIT_NEXTFILE, OnUpdateEditNextFile)
    ON_COMMAND(ID_EDIT_NEXTFILE, OnEditNextFile)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PREVIOUSFILE, OnUpdateEditPreviousFile)
    ON_COMMAND(ID_EDIT_PREVIOUSFILE, OnEditPreviousFile)
    ON_COMMAND(ID_HELP_DLLVERSIONINFORMATION, OnHelpDLLsVersionInformation)
    ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
    ON_COMMAND(ID_EDIT_TEXTAREA, OnEditTextArea)
    ON_COMMAND(ID_EDIT_HEXAREA, OnEditHexArea)
    ON_COMMAND(ID_VIEW_SETTINGS, OnViewSettings)
    ON_UPDATE_COMMAND_UI(ID_FILE_PAGE_SETUP, OnUpdateFilePageSetup)
    ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrintPreview)
    ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
    ON_COMMAND(ID_FILE_PAGE_SETUP, OnFilePageSetup)
    ON_COMMAND(ID_VIEW_DISPLAYFONT, OnViewDisplayFont)
    ON_UPDATE_COMMAND_UI(ID_VIEW_DISPLAYFONT, OnUpdateViewDisplayFont)
    ON_COMMAND(ID_VIEW_PRINTERFONT, OnViewPrinterFont)
    ON_UPDATE_COMMAND_UI(ID_VIEW_PRINTERFONT, OnUpdateViewPrinterFont)
    ON_UPDATE_COMMAND_UI(ID_VIEW_HEX, OnUpdateViewHex)
    ON_COMMAND(ID_VIEW_HEX, OnViewHex)
    ON_UPDATE_COMMAND_UI(ID_VIEW_BOTH, OnUpdateViewBoth)
    ON_COMMAND(ID_VIEW_BOTH, OnViewBoth)
    ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTONLY, OnUpdateViewTextOnly)
    ON_COMMAND(ID_VIEW_TEXTONLY, OnViewTextOnly)
    ON_UPDATE_COMMAND_UI(ID_VIEW_STATISTICS, &CMainFrame::OnUpdateViewStatistics)
    ON_COMMAND(ID_VIEW_STATISTICS, &CMainFrame::OnViewStatistics)
    //}}AFX_MSG_MAP

    ON_WM_MEASUREITEM()
    ON_WM_INITMENUPOPUP()
    ON_WM_MENUCHAR()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void CMainFrame::Dump( CDumpContext& dc ) const
{
    CFrameWnd::Dump( dc );
}

#endif //_DEBUG

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CMainFrame::PreCreateWindow( CREATESTRUCT& cs )
{
    // Remove "horizontal/vertical-redraw" window style to prevent flickering in
    // our View windows whenever they're resized (since they're 'control' views)
    // by registering our own window class without CS_HREDRAW and CS_VREDRAW...
    // (THANK YOU to Paul DiLascia!!)

    // The only window classes that MFC registers with CS_HREDRAW and CS_VREDRAW
    // are as follows (ref: function "AfxEndDeferRegisterClass" in WINCORE.CPP):
    //
    //   AFX_WND_REG:               child windows
    //   AFX_WNDOLECONTROL_REG:     control windows
    //   AFX_WNDFRAMEORVIEW_REG:    SDI CMainFrame (CFrameWnd derived),
    //                              MDI CChildFrame (CMDIChildWnd derived),
    //                              and Views (CView derived)
    //
    // so those are the ONLY ones you ever MIGHT want to do the below for...
    // (but, of course, as fate would have it, just about EVERY frickin' window
    // in an MFC app happens to be one of the above types, so)

    cs.lpszClass = AfxRegisterWndClass
    (
        CS_DBLCLKS,                         // (need double-clicks)
        NULL,                               // (use default cursor)
        NULL,                               // (NO BACKGROUND BRUSH!)
        g_App.LoadIcon( IDR_MAINFRAME )     // (app icon)
    );

    ASSERT( cs.lpszClass );

    return CFrameWnd::PreCreateWindow( cs );
}

//////////////////////////////////////////////////////////////////////////////////////////

static UINT g_arUINTStatusbarPaneIndicators[] =
{
    ID_SEPARATOR,
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};

//////////////////////////////////////////////////////////////////////////////////////////

int CMainFrame::OnCreate( CREATESTRUCT* pCS )
{
    if ( CFrameWnd::OnCreate( pCS ) == -1 )
        return -1;

    if (0
        || !m_wndToolBar.CreateEx
        (
            this,
            TBSTYLE_FLAT,
            0
            | WS_CHILD
            | WS_VISIBLE
            | CBRS_TOP
            | CBRS_GRIPPER
            | CBRS_TOOLTIPS
            | CBRS_FLYBY
            | CBRS_SIZE_DYNAMIC
        )
        || !m_wndToolBar.LoadToolBar( IDR_MAINFRAME )
    )
    {
        TRACE( _T( "** CMainFrame::OnCreate: Failed to create m_wndToolBar!\n" ) );
        ASSERT( false );
        return -1;
    }

    m_wndToolBar.SetWindowText( _T( "Toolbar" ) );

    if (0
        || !m_wndStatusBar.Create( this )
        || !m_wndStatusBar.SetIndicators
            (
                             g_arUINTStatusbarPaneIndicators,
                NUM_ENTRIES( g_arUINTStatusbarPaneIndicators )
            )
    )
    {
        TRACE( _T( "** CMainFrame::OnCreate: Failed to create m_wndStatusBar!\n" ) );
        ASSERT(false);
        return -1;
    }

    m_wndToolBar.EnableDocking( CBRS_ALIGN_ANY );
                 EnableDocking( CBRS_ALIGN_ANY );

    DockControlBar( &m_wndToolBar );

    SafeLoadBarState ( this, _T( "BarsState" ) );

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CMainFrame::OnCreateClient( CREATESTRUCT* pCS, CCreateContext* pContext )
{
    // Create a splitter window consisting of two columns: first column is our
    // ListView pane that shows the list of File#s/Block#s on the tape; The 2nd
    // column (pane) [of our splitter window] is our HexEdit view which shows
    // them the data from the block they clicked on...

    //                                      num   num
    //                                      rows  cols
    if ( !m_wndSplitter.CreateStatic( this,  1,    2  ) )
    {
        TRACE( _T( "** CMainFrame::OnCreateClient: Failed to create m_wndSplitter!\n" ) );
        ASSERT(false);
        return FALSE;
    }

    // ("Size" doesn't matter for Create; actual column width is adjusted further below)

    VERIFY( m_wndSplitter.CreateView( 0, 0, RUNTIME_CLASS( CMyListView    ), CSize(0,0), pContext ) );
    VERIFY( m_wndSplitter.CreateView( 0, 1, RUNTIME_CLASS( CMyHexEditView ), CSize(0,0), pContext ) );

    // -----------------------------------------------------------------------
    //
    // "Column 0" of the  >>> SPLITTER <<<  window is our ListView
    // pane (which, because it is a ListView, has a number of columns
    // of its own (i.e. File#, Block#, etc)).

    // The below "TOTAL_PIXEL_WIDTH_OF_ALL_COLUMNS" value is the total
    // width of just the LISTVIEW "column" of our SPLITTER window.

    // "Column 1" of our splitter window is the HexEdit View that shows
    // the actual contents of the ListView item (tape block) they clicked
    // on. The [splitter window pane] "column width" value for it doesn't
    // really matter, since the last "column" (pane) in a splitter window
    // always gets however many pixels are left over...

    int nVertScrollbarWidth = GetSystemMetrics( SM_CXVSCROLL );
    int nPane0Width = TOTAL_PIXEL_WIDTH_OF_ALL_COLUMNS + nVertScrollbarWidth;

    //                           splitter   initial
    //                            window     width    minimum
    //                            column      of       pane
    //                         (i.e. pane)   pane      width
    // -----------------------------------------------------------------------

    m_wndSplitter.SetColumnInfo(    0,    nPane0Width,  15 );
    m_wndSplitter.SetColumnInfo(    1,         0,       15 );

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnClose()
{
    SaveBarState( _T( "BarsState" ) );
    CWindowPlacement wp; wp.Save( _T( "MainWnd" ), this );
    CFrameWnd ::OnClose();
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnHelpDLLsVersionInformation()
{
    CString  strFishLibVersInfo;
    CString  strZLIBVersInfo;
    CString  strBZIP2VersInfo;

    // Retrieve FishLib dll version information

    {
#define  FISHLIB_DLL_NAME   FISHLIB_NAME  _T(".dll")

        CModuleVersion  vers_info;

        vers_info . GetFileVersionInfo( FISHLIB_DLL_NAME );

        strFishLibVersInfo . Format
        (
            _T( "%s  version %d.%d.%d.%d" )

            ,FISHLIB_DLL_NAME

            ,HIWORD( vers_info . dwFileVersionMS )
            ,LOWORD( vers_info . dwFileVersionMS )
            ,HIWORD( vers_info . dwFileVersionLS )
            ,LOWORD( vers_info . dwFileVersionLS )
        );
    }

    // Retrieve ZLIB dll version information

    {
        CModuleVersion  vers_info;

        vers_info . GetFileVersionInfo( _T( ZLIB_DLLNAME ) );

        strZLIBVersInfo . Format
        (
            _T( "%s  version %s  (%d.%d.%d.%d)" )

            ,_T( ZLIB_DLLNAME )

            ,Get_ZLIB_DLL_Version()

            ,HIWORD( vers_info . dwFileVersionMS )
            ,LOWORD( vers_info . dwFileVersionMS )
            ,HIWORD( vers_info . dwFileVersionLS )
            ,LOWORD( vers_info . dwFileVersionLS )
        );
    }

    // Retrieve BZIP2 dll version information

    {
        CModuleVersion  vers_info;
        vers_info . GetFileVersionInfo( _T( BZIP2_DLLNAME ) );

        strBZIP2VersInfo . Format
        (
            _T( "%s  version %s  (%d.%d.%d.%d)" )

            ,_T( BZIP2_DLLNAME )

            ,Get_BZIP2_DLL_Version()

            ,HIWORD( vers_info . dwFileVersionMS )
            ,LOWORD( vers_info . dwFileVersionMS )
            ,HIWORD( vers_info . dwFileVersionLS )
            ,LOWORD( vers_info . dwFileVersionLS )
        );
    }

    // Display DLL version information

    CString strReport; strReport . Format
    (
        _T( "%s\n" )
        _T( "http://www.softdevlabs.com\n\n" )

        _T( "%s\n" )
        _T( "http://www.zlib.net\n\n" )

        _T( "%s\n" )
        _T( "http://sources.redhat.com/bzip2" )

        ,strFishLibVersInfo

        ,strZLIBVersInfo
        ,strBZIP2VersInfo
    );

    TRACE( _T( "** %s\n" ), strReport );

    AfxMessageBox( strReport, MB_ICONINFORMATION );
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//                         O n U p d a t e X X X ( ) . . .
//
//////////////////////////////////////////////////////////////////////////////////////////
//
// The following menu and toolbar OnUpdateXXX() functions are defined in the CMainFrame
// class because the CFrameWnd class (which CMainFrame is derived from) is the only one
// responsible for updating the user interface.
//
// Here we simply call into the appropriate splitter window view class function (either
// CMyListView or CMyHexEditView -- a pointer to each of which is kept in our CDocument
// CAWSBrowseDoc class object) to determine whether/how the command/toolbar should be
// enabled/disabled, checked/unchecked, etc.
//
// The actual menu/toolbar COMMAND functions however are defined within the appropriate
// view classes themselves however (CMyListView or CMyHexEditView) since they're the ones
// ultimately responsible for actually processing the command in question...
//
//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnUpdateFilePageSetup( CCmdUI* pCmdUI )
{
    pCmdUI->Enable( !GetDocument()->IsEmpty() );
}

void CMainFrame::OnUpdateFilePrintPreview( CCmdUI* pCmdUI )
{
    pCmdUI->Enable
    (1
        && !GetDocument()->IsEmpty()
        && !GetDocument()->m_pHexEditView->IsEmpty()
    );
}

void CMainFrame::OnUpdateFilePrint ( CCmdUI* pCmdUI )
{
    pCmdUI->Enable
    (1
        && !GetDocument()->IsEmpty()
        && !GetDocument()->m_pHexEditView->IsEmpty()
    );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnUpdateEditCopy( CCmdUI* pCmdUI )
{
    pCmdUI->Enable( GetDocument()->m_pHexEditView->IsDataSelected() );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnUpdateEditHexArea( CCmdUI* pCmdUI )
{
    CAWSBrowseDoc*   pDoc          = GetDocument();
    CMyHexEditView*  pHexEditView  = pDoc->m_pHexEditView;
    CWnd*            pFocusWnd     = GetFocus();

    BOOL bIsHexView = pHexEditView->IsHexView();

    BOOL bEnable = !pDoc->IsEmpty() && bIsHexView &&
    (0
        || pFocusWnd == pHexEditView
        || pFocusWnd == pHexEditView->GetHexEditCtrlPtr()
    );

    BOOL bCheck = FALSE;

    if ( bEnable )
    {
        if (!bIsHexView)
            ASSERT( false );    // (oops! got them out-of-sync with one another!)

        int nSelStart, nSelEnd; BOOL bTextArea;
        pHexEditView->GetHexEditCtrlPtr()->GetSelEx( nSelStart, nSelEnd, bTextArea );
        bCheck  = !bTextArea;

        if ( bCheck && !bIsHexView )
        {
            ASSERT( false );    // (oops! got them out-of-sync with one another!)
            bCheck  = FALSE;    // (can't switch to hex area if it's not visible!)
            bEnable = FALSE;    // (can't switch to hex area if it's not visible!)
        }
    }

    pCmdUI->Enable( bEnable );
    pCmdUI->SetCheck( bCheck );
}

void CMainFrame::OnUpdateEditTextArea( CCmdUI* pCmdUI )
{
    CAWSBrowseDoc* pDoc = GetDocument();
    CWnd* pFocusWnd = GetFocus();

    BOOL bEnable = !pDoc->IsEmpty() &&
    (0
        || pFocusWnd == pDoc->m_pHexEditView
        || pFocusWnd == pDoc->m_pHexEditView->GetHexEditCtrlPtr()
    );
    BOOL bCheck = FALSE;

    if ( bEnable )
    {
        int nSelStart, nSelEnd; BOOL bTextArea;
        pDoc->m_pHexEditView->GetHexEditCtrlPtr()->GetSelEx( nSelStart, nSelEnd, bTextArea );
        bCheck  = bTextArea;
    }

    pCmdUI->Enable( bEnable );
    pCmdUI->SetCheck( bCheck );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnUpdateEditFind( CCmdUI* pCmdUI )
{
    pCmdUI->Enable( !GetDocument()->IsEmpty() );
}

void CMainFrame::OnUpdateEditFindNext( CCmdUI* pCmdUI )
{
    CAWSBrowseDoc* pDoc = GetDocument();
    pCmdUI->Enable( !pDoc->IsEmpty() && pDoc->m_pListView->HaveSearchString() );
}

void CMainFrame::OnUpdateEditFindPrevious( CCmdUI* pCmdUI )
{
    CAWSBrowseDoc* pDoc = GetDocument();
    pCmdUI->Enable( !pDoc->IsEmpty() && pDoc->m_pListView->HaveSearchString() );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnUpdateEditNextBlock( CCmdUI* pCmdUI )
{
    pCmdUI->Enable( !GetDocument()->IsEmpty() );
}

void CMainFrame::OnUpdateEditPreviousBlock( CCmdUI* pCmdUI )
{
    pCmdUI->Enable( !GetDocument()->IsEmpty() );
}

void CMainFrame::OnUpdateEditNextFile( CCmdUI* pCmdUI )
{
    pCmdUI->Enable( !GetDocument()->IsEmpty() );
}

void CMainFrame::OnUpdateEditPreviousFile( CCmdUI* pCmdUI )
{
    pCmdUI->Enable( !GetDocument()->IsEmpty() );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnUpdateViewDisplayFont( CCmdUI* pCmdUI )
{
    pCmdUI->Enable( TRUE );
}

void CMainFrame::OnUpdateViewPrinterFont( CCmdUI* pCmdUI )
{
    pCmdUI->Enable( TRUE );
}

void CMainFrame::OnUpdateViewHex( CCmdUI* pCmdUI )
{
    CAWSBrowseDoc* pDoc = GetDocument();
    pCmdUI->Enable( !pDoc->IsEmpty() );
    pCmdUI->SetCheck( pDoc->m_pHexEditView->IsHexView() );
}

void CMainFrame::OnUpdateViewAscii( CCmdUI* pCmdUI )
{
    CAWSBrowseDoc* pDoc = GetDocument();
    pCmdUI->Enable( !pDoc->IsEmpty() );
    pCmdUI->SetCheck( pDoc->m_pHexEditView->IsASCIIMode() );
}

void CMainFrame::OnUpdateViewEbcdic( CCmdUI* pCmdUI )
{
    CAWSBrowseDoc* pDoc = GetDocument();
    pCmdUI->Enable( !pDoc->IsEmpty() );
    pCmdUI->SetCheck( pDoc->m_pHexEditView->IsEBCDICMode() );
}

void CMainFrame::OnUpdateViewBoth( CCmdUI* pCmdUI )
{
    CAWSBrowseDoc* pDoc = GetDocument();
    pCmdUI->Enable( !pDoc->IsEmpty() );
    pCmdUI->SetCheck( pDoc->m_pHexEditView->IsHexView() );
}

void CMainFrame::OnUpdateViewTextOnly( CCmdUI* pCmdUI )
{
    CAWSBrowseDoc* pDoc = GetDocument();
    pCmdUI->Enable( !pDoc->IsEmpty() );
    pCmdUI->SetCheck( !pDoc->m_pHexEditView->IsHexView() );
}

void CMainFrame::OnUpdateViewStatistics(CCmdUI *pCmdUI)
{
    CAWSBrowseDoc* pDoc = GetDocument();
    pCmdUI->Enable( !pDoc->IsEmpty() );
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
//          O n X X X x x x ( )   menu/toolbar command processing functions...
//
//  Here all we do is pass the command on to the appropriate view for processing...
//
//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnFilePageSetup()
{
    GetDocument()->m_pListView->SendMessage( WM_COMMAND, ID_FILE_PAGE_SETUP, 0 );
}

void CMainFrame::OnFilePrintPreview()
{
    GetDocument()->m_pListView->SendMessage( WM_COMMAND, ID_FILE_PRINT_PREVIEW, 0 );
}

void CMainFrame::OnFilePrint()
{
    GetDocument()->m_pListView->SendMessage( WM_COMMAND, ID_FILE_PRINT, 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnEditCopy()
{
    GetDocument()->m_pHexEditView->SendMessage( WM_COMMAND, ID_EDIT_COPY, 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnEditHexArea()
{
    CAWSBrowseDoc* pDoc = GetDocument();

    if ( !pDoc->m_pHexEditView->IsHexView() )
    {
        ASSERT( false );    // (oops! got them out-of-sync with one another!)
        return;             // (can't switch to hex area if it's not visible!)
    }

    pDoc->m_pHexEditView->SendMessage( WM_COMMAND, ID_EDIT_HEXAREA, 0 );
}

void CMainFrame::OnEditTextArea()
{
    GetDocument()->m_pHexEditView->SendMessage( WM_COMMAND, ID_EDIT_TEXTAREA, 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnEditFind()
{
    GetDocument()->m_pListView->SendMessage( WM_COMMAND, ID_EDIT_FIND, 0 );
}

void CMainFrame::OnEditFindNext()
{
    GetDocument()->m_pListView->SendMessage( WM_COMMAND, ID_EDIT_FINDNEXT, 0 );
}

void CMainFrame::OnEditFindPrevious()
{
    GetDocument()->m_pListView->SendMessage( WM_COMMAND, ID_EDIT_FINDPREVIOUS, 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnEditNextBlock()
{
    GetDocument()->m_pListView->SendMessage( WM_COMMAND, ID_EDIT_NEXTBLOCK, 0 );
}

void CMainFrame::OnEditPreviousBlock()
{
    GetDocument()->m_pListView->SendMessage( WM_COMMAND, ID_EDIT_PREVIOUSBLOCK, 0 );
}

void CMainFrame::OnEditNextFile()
{
    GetDocument()->m_pListView->SendMessage( WM_COMMAND, ID_EDIT_NEXTFILE, 0 );
}

void CMainFrame::OnEditPreviousFile()
{
    GetDocument()->m_pListView->SendMessage( WM_COMMAND, ID_EDIT_PREVIOUSFILE, 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnViewDisplayFont()
{
    GetDocument()->m_pHexEditView->SendMessage( WM_COMMAND, ID_VIEW_DISPLAYFONT, 0 );
}

void CMainFrame::OnViewPrinterFont()
{
    GetDocument()->m_pListView->SendMessage( WM_COMMAND, ID_VIEW_PRINTERFONT, 0 );
}

void CMainFrame::OnViewSettings()
{
    GetDocument()->m_pHexEditView->SendMessage( WM_COMMAND, ID_VIEW_SETTINGS, 0 );
}

void CMainFrame::OnViewHex()
{
    GetDocument()->m_pHexEditView->SendMessage( WM_COMMAND, ID_VIEW_HEX, 0 );
}

void CMainFrame::OnViewAscii()
{
    GetDocument()->m_pHexEditView->SendMessage( WM_COMMAND, ID_VIEW_ASCII, 0 );
}

void CMainFrame::OnViewEbcdic()
{
    GetDocument()->m_pHexEditView->SendMessage( WM_COMMAND, ID_VIEW_EBCDIC, 0 );
}

void CMainFrame::OnViewBoth()
{
    CAWSBrowseDoc* pDoc = GetDocument();
    if (!pDoc->m_pHexEditView->IsHexView())
        pDoc->m_pHexEditView->SendMessage( WM_COMMAND, ID_VIEW_HEX, 0 );
}

void CMainFrame::OnViewTextOnly()
{
    CAWSBrowseDoc* pDoc = GetDocument();
    if (pDoc->m_pHexEditView->IsHexView())
        pDoc->m_pHexEditView->SendMessage( WM_COMMAND, ID_VIEW_HEX, 0 );
}

void CMainFrame::OnViewStatistics()
{
    GetDocument()->m_pListView->SendMessage( WM_COMMAND, ID_VIEW_STATISTICS, 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////
// BCMenu

HMENU CMainFrame::NewMenu()
{
    // Load the menu from the resources
    // ****replace IDR_MAINFRAME with your menu ID****
    m_menu.LoadMenu(IDR_MAINFRAME);

    // Use ModifyODMenu to add a bitmap to a menu options.
    // The first parameter is the menu option text string.
    // If it's NULL, keep the current text string.The second
    // option is the ID of the menu option to change.
    // The third option is the resource ID of the bitmap.
    // This can also be a toolbar ID in which case the class
    // searches the toolbar for the appropriate bitmap.Only
    // Bitmap and Toolbar resources are supported.
//  m_menu.ModifyODMenu(NULL,ID_ZOOM,IDB_ZOOM);

    // Another method for adding bitmaps to menu options is
    // through the LoadToolbar member function.This method
    // allows you to add all the bitmaps in a toolbar object
    // to menu options (if they exist). The function parameter
    // is an the toolbar id. There is also a function called
    // LoadToolbars that takes an array of id's.
    m_menu.LoadToolbar( IDR_MAINFRAME );

    return m_menu.Detach();
}

//////////////////////////////////////////////////////////////////////////////////////////
// BCMenu: This handler ensures that the popup menu items are drawn correctly

void CMainFrame::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    if (ODT_MENU == lpMeasureItemStruct->CtlType)
    {
        HMENU hMenuID = (HMENU) (UINT_PTR) lpMeasureItemStruct->itemID;

        if (IsMenu(hMenuID))
        {
            CMenu* pMenu = CMenu::FromHandle(hMenuID);

            if (m_menu.IsMenu(pMenu))
            {
                m_menu.MeasureItem(lpMeasureItemStruct);
                return;
            }
        }
    }

    CFrameWnd::OnMeasureItem(nIDCtl,lpMeasureItemStruct);
}

//////////////////////////////////////////////////////////////////////////////////////////
// BCMenu: This handler ensures that keyboard shortcuts work

LRESULT CMainFrame::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu)
{
    if (m_menu.IsMenu(pMenu))
        return BCMenu::FindKeyboardShortcut(nChar, nFlags, pMenu);

    return CFrameWnd::OnMenuChar(nChar, nFlags, pMenu);
}

//////////////////////////////////////////////////////////////////////////////////////////
// BCMenu: This handler updates the menus from time to time

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
    CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

    if (bSysMenu) return;

    if (m_menu.IsMenu(pPopupMenu)) BCMenu::UpdateMenu(pPopupMenu);
}

//////////////////////////////////////////////////////////////////////////////////////////
