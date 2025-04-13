// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// AWSBrowse.cpp : Defines the class behaviors for the application.
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  06/15/06    1.5.0   Fish    VS2005, x64
//  11/21/06    1.5.0   Fish    Load zlib/bzip2 DLLs at startup
//  02/06/07    1.5.0   Fish    BCMenu
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AWSBrowseDoc.h"
#include "MyHexEditView.h"
#include "MainFrm.h"
#include "HEPARMS.h"
#include "About.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (embed copyright into object code)
#pragma comment ( user, COPYRIGHT_EXESTR )

//////////////////////////////////////////////////////////////////////////////////////////
// The one and only application object...

CAWSBrowseApp  g_App;       // (this one object does it all; it's the entire program)

//////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CAWSBrowseApp, CWinApp )

    //{{AFX_MSG_MAP(CAWSBrowseApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    //}}AFX_MSG_MAP

    // Standard file based document commands

    ON_COMMAND( ID_FILE_NEW,  CWinApp::OnFileNew  )
    ON_COMMAND( ID_FILE_OPEN, CWinApp::OnFileOpen )

END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////

void CAWSBrowseApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CAWSBrowseApp::InitInstance()
{
    AfxEnableControlContainer();

    // Change the registry key under which our settings are stored.

    SetRegistryKey( _T( COMPANY ) );

    LoadStdProfileSettings( MAX_MRU );
    LoadMyStdProfileSettings();

    // Load support DLLs...

    if (!Load_ZLIB_DLL())
        TRACE( _T("** WARNING: %s load failed!\n"), _T( ZLIB_DLLNAME ) );

    if (!Load_BZIP2_DLL())
        TRACE( _T("** WARNING: %s load failed!\n"), _T( BZIP2_DLLNAME ) );

    // Register the application's document templates.  Document templates
    // serve as the connection between documents, frame windows and views.

    CSingleDocTemplate* pDocTemplate;

    pDocTemplate = new CSingleDocTemplate
    (
        IDR_MAINFRAME,
        RUNTIME_CLASS( CAWSBrowseDoc  ),
        RUNTIME_CLASS( CMainFrame     ),    // main SDI frame window
        RUNTIME_CLASS( CMyHexEditView )
    );
    AddDocTemplate( pDocTemplate );

    // Parse command line for standard shell commands, DDE, file open

    CCommandLineInfo  cmdInfo;
    ParseCommandLine( cmdInfo );

    // Dispatch commands specified on the command line

    if ( !ProcessShellCommand( cmdInfo ) )
        return FALSE;

    // BCMenu...
    {
        CMenu* pMenu = m_pMainWnd->GetMenu();

        if (pMenu) pMenu->DestroyMenu();

        HMENU hMenu = ((CMainFrame*)m_pMainWnd)->NewMenu();

        pMenu = CMenu::FromHandle(hMenu);

        m_pMainWnd->SetMenu(pMenu);

        ((CMainFrame*)m_pMainWnd)->m_hMenuDefault = hMenu;
    }

    // The one and only window has been initialized, so show and update it.

    m_pMainWnd->ShowWindow( SW_SHOW );
    m_pMainWnd->UpdateWindow();

    CWindowPlacement wp; wp.Restore( _T( "MainWnd" ), m_pMainWnd );

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////

int CAWSBrowseApp::ExitInstance()
{
    SaveMyStdProfileSettings();
    return CWinApp ::ExitInstance();
}

//////////////////////////////////////////////////////////////////////////////////////////
// (set landscape mode on default printer)

BOOL CAWSBrowseApp::SetMFCsDefaultPrinterToLandscapeMode()
{
    PRINTDLG pd;  pd.lStructSize = NUM_BYTES( pd );

    if ( !GetPrinterDeviceDefaults( &pd ) )
        return FALSE;

    DEVMODE* pDevMode = (DEVMODE*) GlobalLock( m_hDevMode );

    if ( !pDevMode )
        return FALSE;

    pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
    GlobalUnlock( m_hDevMode );
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CAWSBrowseApp::LoadMyStdProfileSettings()
{
    g_dwMaxBlockSize = g_App.GetProfileInt( _T( "Limits" ), _T( "dwMaxBlockSize" ), g_dwMaxBlockSize );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CAWSBrowseApp::SaveMyStdProfileSettings()
{
    // There are no application-wide settings to be saved at the moment.

    // Most settings are saved/restored within the CMyHexEditView and
    // CMyListView classes.
}

//////////////////////////////////////////////////////////////////////////////////////////
