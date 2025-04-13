// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// SettingsDialog.cpp : implementation file
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  03/19/06    1.3.0   Fish    Text-only record length
//  06/15/06    1.5.0   Fish    VS2005, x64
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SettingsDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (embed copyright into object code)
#pragma comment ( user, COPYRIGHT_EXESTR )

//////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CSettingsDialog, CDialog )

    //{{AFX_MSG_MAP(CSettingsDialog)
    ON_BN_CLICKED(IDC_DYN_CHECKBOX, OnDynCheckbox)
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP

END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////

CSettingsDialog::CSettingsDialog( CWnd* pParent /* =NULL */ )

    : CDialog( CSettingsDialog::IDD, pParent )
{
    //{{AFX_DATA_INIT(CSettingsDialog)
    m_nBPG = 0;
    m_nBPR = 0;
    m_bDynBPR = FALSE;
    m_nFormat = -1;
    m_nTextRecLen = 0;
    //}}AFX_DATA_INIT

    m_nBPG        = DEF_HEXEDIT_BPG;
    m_nBPR        = DEF_HEXEDIT_BPR;
    m_nFormat     = ASCII_FORMAT;
    m_nTextRecLen = DEF_HEXEDIT_TEXTONLY_RECLEN;
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CSettingsDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_BPGSpinner    . SetRange32( 1, MAX_BYTES_PER_GROUP );
    m_BPRSpinner    . SetRange32( 1, MAX_BYTES_PER_LINE  );
    m_RecLenSpinner . SetRange32( 1, MAX_BYTES_PER_LINE  );

    OnDynCheckbox();

    CWindowPlacement wp; wp.Restore( _T( "SettingsDlg" ), this, FALSE );

    return TRUE;    // return TRUE unless you set the focus to a control
                    // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////////////////////

void CSettingsDialog::OnDynCheckbox()
{
    m_bDynBPR = m_DynBPRCheckbox.GetCheck();

    m_BPREdit    . EnableWindow( !m_bDynBPR );
    m_BPRSpinner . EnableWindow( !m_bDynBPR );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CSettingsDialog::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );

    //{{AFX_DATA_MAP(CSettingsDialog)
    DDX_Control(pDX, IDC_RECLEN_SPINNER, m_RecLenSpinner);
    DDX_Control(pDX, IDC_BPG_SPINNER, m_BPGSpinner);
    DDX_Text(pDX, IDC_BPG_EDIT, m_nBPG);
    DDX_Control(pDX, IDC_BPR_EDIT, m_BPREdit);
    DDX_Control(pDX, IDC_BPR_SPINNER, m_BPRSpinner);
    DDX_Text(pDX, IDC_BPR_EDIT, m_nBPR);
    DDX_Control(pDX, IDC_DYN_CHECKBOX, m_DynBPRCheckbox);
    DDX_Check(pDX, IDC_DYN_CHECKBOX, m_bDynBPR);
    DDX_Control(pDX, IDC_TEXT_RECLEN_EDIT, m_RecLenEdit);
    DDX_Text(pDX, IDC_TEXT_RECLEN_EDIT, m_nTextRecLen);
    DDX_Radio(pDX, IDC_FORMAT_RADIOBUTTON_GROUP, m_nFormat);
    //}}AFX_DATA_MAP

    DDV_MinMaxUInt( pDX, m_nBPG,        1, MAX_BYTES_PER_GROUP );
    DDV_MinMaxUInt( pDX, m_nBPR,        1, MAX_BYTES_PER_LINE  );
    DDV_MinMaxUInt( pDX, m_nTextRecLen, 1, MAX_BYTES_PER_LINE  );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CSettingsDialog::OnDestroy()
{
    CWindowPlacement wp; wp.Save( _T( "SettingsDlg" ), this );
    CDialog ::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////////////////////
