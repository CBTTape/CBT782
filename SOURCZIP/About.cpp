// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// About.cpp : implementation file
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  06/15/06    1.5.0   Fish    VS2005, x64
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "About.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (embed copyright into object code)
#pragma comment ( user, COPYRIGHT_EXESTR )

//////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CAboutDlg, CDialog )

    //{{AFX_MSG_MAP(CAboutDlg)
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP

END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////

CAboutDlg::CAboutDlg()

    : CDialog( CAboutDlg::IDD )
{
    //{{AFX_DATA_INIT(CAboutDlg)
    m_strProductVersion = _T("");
    m_strCopyrightCompany = _T("");
    m_strContactEmail = _T("");
    //}}AFX_DATA_INIT

    m_strProductVersion   = _T( PRODUCT ) _T( " -- Version " ) VERSION_STR;
    m_strCopyrightCompany = _T( COPYRIGHT ) _T( " " ) _T( COMPANY );
    m_strContactEmail     = _T( PRODUCT_EMAIL );
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CAboutDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Must set the url AFTER OnInitDialog so the window is sized properly

    m_ProductVersionStatic . SetURL( _T( PRODUCT_URL ) );
    m_ContactEmailStatic   . SetURL( _T( "mailto:" ) _T( PRODUCT_EMAIL ) _T( "?subject=" ) _T( PRODUCT ) _T( " -- Version " ) VERSION_STR );

    // Save default hover color

    COLORREF crDefaultHoverColor = m_ProductVersionStatic.GetHoverColor();

    // Try to use same colors as browser (if possible)

    m_ProductVersionStatic . SetIEColors();
    m_ContactEmailStatic   . SetIEColors();

    // Retrieve browser colors

    COLORREF crLinkColor    = m_ProductVersionStatic.GetLinkColor();
    COLORREF crVisitedColor = m_ProductVersionStatic.GetVisitedColor();
    COLORREF crHoverColor   = m_ProductVersionStatic.GetHoverColor();

    if ( crHoverColor == crLinkColor )  // Do they have a hover color specified?
    {
        // They don't have a hover color defined. Use the default hover color.

        m_ProductVersionStatic . SetColors( crLinkColor, crVisitedColor, crDefaultHoverColor );
        m_ContactEmailStatic   . SetColors( crLinkColor, crVisitedColor, crDefaultHoverColor );
    }

    CWindowPlacement wp; wp.Restore( _T( "About" ), this );

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////////////////////

void CAboutDlg::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );

    //{{AFX_DATA_MAP(CAboutDlg)
    DDX_Control(pDX, IDC_CONTACT_EMAIL_STATIC, m_ContactEmailStatic);
    DDX_Control(pDX, IDC_COPYRIGHT_COMPANY_STATIC, m_CopyrightCompanyStatic);
    DDX_Control(pDX, IDC_PRODUCT_VERSION_STATIC, m_ProductVersionStatic);
    DDX_Text(pDX, IDC_PRODUCT_VERSION_STATIC, m_strProductVersion);
    DDX_Text(pDX, IDC_COPYRIGHT_COMPANY_STATIC, m_strCopyrightCompany);
    DDX_Text(pDX, IDC_CONTACT_EMAIL_STATIC, m_strContactEmail);
    //}}AFX_DATA_MAP
}

//////////////////////////////////////////////////////////////////////////////////////////

void CAboutDlg::OnDestroy()
{
    CDialog ::OnDestroy();
    CWindowPlacement wp; wp.Save( _T( "About" ), this );
}

//////////////////////////////////////////////////////////////////////////////////////////
