// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
//  ProgDlg.cpp : implementation file
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  06/01/06    1.3.1   Fish    Fix divide-by-zero bug in UpdatePercent (sigh)
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProgDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CProgressDlg, CDialog )

    //{{AFX_MSG_MAP(CProgressDlg)
    //}}AFX_MSG_MAP

END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////

CProgressDlg::CProgressDlg( UINT nCaptionID /* =IDS_PROGRESS_CAPTION */ )
{
    //{{AFX_DATA_INIT(CProgressDlg)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    m_nCaptionID = nCaptionID;

    m_nLower  = 0;
    m_nUpper  = 100;
    m_nStep   = 10;

    m_bCancelled          = FALSE;
    m_bParentDisabled = FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////

CProgressDlg::~CProgressDlg()
{
    if ( m_hWnd )
        DestroyWindow();
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CProgressDlg::Create( CWnd *pParent )
{
    // Get the true parent of the dialog

    m_pParentWnd = CWnd::GetSafeOwner( pParent );

    // m_bParentDisabled is used to re-enable the parent window
    // when the dialog is destroyed. So we don't want to set it
    // to TRUE unless the parent was already enabled...

    if ( m_pParentWnd && m_pParentWnd->IsWindowEnabled() )
    {
        m_pParentWnd->EnableWindow( FALSE );
        m_bParentDisabled = TRUE;
    }

    if ( CDialog::Create( CProgressDlg::IDD, pParent ) )
        return TRUE;

    ReEnableParent();
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CProgressDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_Progress . SetRange32 ( m_nLower, m_nUpper );
    m_Progress . SetStep    ( m_nStep );
    m_Progress . SetPos     ( m_nLower );

    CString strCaption;

    if ( !m_strTitle.IsEmpty() )
        strCaption = m_strTitle;
    else
        VERIFY( strCaption.LoadString( m_nCaptionID ) );

    SetWindowText( strCaption );

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CProgressDlg::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );

    //{{AFX_DATA_MAP(CProgressDlg)
    DDX_Control(pDX, IDC_PROGDLG_PROGRESS, m_Progress);
    //}}AFX_DATA_MAP
}

//////////////////////////////////////////////////////////////////////////////////////////

void CProgressDlg::SetRange32( int nLower, int nUpper )
{
    m_nLower = nLower;
    m_nUpper = nUpper;
    m_Progress.SetRange32( nLower, nUpper );
}

//////////////////////////////////////////////////////////////////////////////////////////

int CProgressDlg::SetPos( int nPos )
{
    PumpMessages();
    int iResult = m_Progress.SetPos( nPos );
    UpdatePercent( m_Progress.GetPos() );
    return iResult;
}

//////////////////////////////////////////////////////////////////////////////////////////

int CProgressDlg::OffsetPos( int nPos )
{
    PumpMessages();
    int nResult = m_Progress.OffsetPos( nPos );
    UpdatePercent( m_Progress.GetPos() );
    return nResult;
}

//////////////////////////////////////////////////////////////////////////////////////////

int CProgressDlg::SetStep( int nStep )
{
    m_nStep = nStep; // (store for later use in calculating percentage)
    return m_Progress.SetStep( nStep );
}

//////////////////////////////////////////////////////////////////////////////////////////

int CProgressDlg::StepIt()
{
    PumpMessages();
    int nResult = m_Progress.StepIt();
    UpdatePercent( m_Progress.GetPos() );
    return nResult;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CProgressDlg::PumpMessages()
{
    ASSERT( m_hWnd );   // (must call Create() before using the dialog)

    // Handle dialog messages

    MSG msg; while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
    {
        if ( !IsDialogMessage( &msg ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

void CProgressDlg::UpdatePercent( int nNewPos )
{
    CWnd* pWndPercent = GetDlgItem( IDC_PROGDLG_PERCENT );

    int nDivisor  =  m_nUpper - m_nLower;
    int nDividend = ( nNewPos - m_nLower );

//  int nPercent =             ( nDividend * 100 ) / nDivisor;
    int nPercent = nDivisor ? (( nDividend * 100 ) / nDivisor) : 0;

    // Since the Progress Control wraps, we will wrap the percentage
    // along with it. However, don't reset 100% back to 0%...

    if ( nPercent != 100 )
         nPercent %= 100;

    // Display the percentage

    CString strBuf; strBuf.Format( _T( "%d%c" ), nPercent, _T( '%' ) );
    CString strCur; pWndPercent->GetWindowText( strCur );

    if ( strCur != strBuf )
        pWndPercent->SetWindowText( strBuf );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CProgressDlg::OnCancel()
{
    m_bCancelled = TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CProgressDlg::CheckCancelButton()
{
    PumpMessages();     // (process all pending messages first)

    // Reset m_bCancelled to FALSE so that CheckCancelButton keeps
    // returning FALSE until the user clicks the Cancel button again.

    // This will allow you to call CheckCancelButton and still continue
    // with whatever you were doing.

    // If m_bCancelled stayed TRUE, then the next call to CheckCancelButton
    // would always return TRUE.

    BOOL bResult = m_bCancelled;
    m_bCancelled = FALSE;

    return bResult;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CProgressDlg::ReEnableParent()
{
    if ( m_bParentDisabled && m_pParentWnd )
        m_pParentWnd->EnableWindow( TRUE );
    m_bParentDisabled = FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CProgressDlg::DestroyWindow()
{
    ReEnableParent();
    return CDialog::DestroyWindow();
}

//////////////////////////////////////////////////////////////////////////////////////////
