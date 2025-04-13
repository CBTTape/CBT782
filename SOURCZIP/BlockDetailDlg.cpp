// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// BlockDetailDlg.cpp : implementation of the BlockDetailDlg class
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  12/05/06    1.5.0   Fish    Alternate window title support.
//  12/05/06    1.5.0   Fish    Set window icon same as main app's.
//  12/06/06    1.5.0   Fish    Make read-only! (oops!)
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "awsbrowse.h"
#include "BlockDetailDlg.h"
#include "BlockDetailDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CBlockDetailDlg, CDialog)

    //{{AFX_MSG_MAP(CBlockDetailDlg)
    ON_WM_SIZE()
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP

    ON_WM_CTLCOLOR()

END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////

CBlockDetailDlg::CBlockDetailDlg( CWnd* pParent /*=NULL*/ )

    : CDialog( CBlockDetailDlg::IDD, pParent )
{
    //{{AFX_DATA_INIT(CBlockDetailDlg)
    m_strDetails = _T("");
    //}}AFX_DATA_INIT

    m_bInitDialogDone = false;
    m_hBrush = CreateSolidBrush( RGB_BlockDetailDlgBkColor );
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CBlockDetailDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (!m_strTitle.IsEmpty())
        SetWindowText( m_strTitle );

    SetIcon( g_App.LoadIcon( IDR_MAINFRAME ), FALSE );

    GetWindowRect(&m_rectDlg);
    m_sizeMin = m_rectDlg.Size();

    VERIFY( m_Font.CreatePointFont( 10 * 10, _T( "Courier" ) ) );

    m_DetailsEdit.SetFont( &m_Font );

    m_bInitDialogDone = true;

    if (m_strWindowPlacementKeyName.IsEmpty())
        m_strWindowPlacementKeyName = _T( "BlockDetailDlg" );

    CWindowPlacement wp; wp.Restore( m_strWindowPlacementKeyName, this, TRUE );

    m_DetailsEdit.SetSel(0,0);

    return FALSE;   // return TRUE unless you set the focus to a control
                    // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////////////////////

void CBlockDetailDlg::OnWindowPosChanging(WINDOWPOS* pwndPos)
{
    if (m_bInitDialogDone)
    {
        pwndPos->cx = max(pwndPos->cx, m_sizeMin.cx);
        pwndPos->cy = max(pwndPos->cy, m_sizeMin.cy);
    }

    CDialog::OnWindowPosChanging(pwndPos);
}

//////////////////////////////////////////////////////////////////////////////////////////

void CBlockDetailDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    if (!m_bInitDialogDone) return;

    // Get the new size of the dialog

    CRect rectDlg;

    GetWindowRect(&rectDlg);
    ScreenToClient(&rectDlg);

    // Calculate how much the width/height has changed

    int  nDeltaX  =  rectDlg.Width()   -  m_rectDlg.Width();
    int  nDeltaY  =  rectDlg.Height()  -  m_rectDlg.Height();

    // Save the new dialog size for next time

    m_rectDlg = rectDlg;

    // Resize the dialog

    CRect rect; // (work)

    // Edit box -- make bigger, but don't move

    m_DetailsEdit.GetWindowRect(&rect); ScreenToClient(&rect);
    rect.BottomRight() += CSize(nDeltaX,nDeltaY);
    m_DetailsEdit.MoveWindow(&rect);
    m_DetailsEdit.Invalidate();

    // OK button -- move, but don't change the size

    // (and try to keep it in the middle of the screen)

    m_OKButton.GetWindowRect(&rect); ScreenToClient(&rect);

    int nButtonWidth = rect.Width();

    rect.left  = m_rectDlg.left + (m_rectDlg.Width() / 2) - (nButtonWidth / 2);
    rect.right = rect.left + nButtonWidth;
    rect.top    += nDeltaY;
    rect.bottom += nDeltaY;

    m_OKButton.MoveWindow(&rect);
    m_OKButton.Invalidate();
}

//////////////////////////////////////////////////////////////////////////////////////////

void CBlockDetailDlg::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );

    //{{AFX_DATA_MAP(CBlockDetailDlg)
    DDX_Control(pDX, IDOK, m_OKButton);
    DDX_Control(pDX, IDC_DETAILS_EDIT, m_DetailsEdit);
    DDX_Text(pDX, IDC_DETAILS_EDIT, m_strDetails);
    //}}AFX_DATA_MAP
}

//////////////////////////////////////////////////////////////////////////////////////////

void CBlockDetailDlg::OnDestroy()
{
    ASSERT( !m_strWindowPlacementKeyName.IsEmpty() );   // (set in OnInitDialog)

    CWindowPlacement wp; wp.Save( m_strWindowPlacementKeyName, this );
    CDialog::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////////////////////

HBRUSH CBlockDetailDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (pWnd->GetDlgCtrlID() == IDC_DETAILS_EDIT)
    {
        pDC->SetBkColor( RGB_BlockDetailDlgBkColor );
        hbr = m_hBrush;
    }

    return hbr;
}

//////////////////////////////////////////////////////////////////////////////////////////
