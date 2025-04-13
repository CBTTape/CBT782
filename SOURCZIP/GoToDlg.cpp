// Copyright (c) 2006-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// GoToDlg.cpp : implementation of the CGoToDlg class
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  12/07/06    1.5.0   Fish    Created
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GoToDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGoToDlg, CDialog)
    ON_WM_DESTROY()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////

CGoToDlg::CGoToDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGoToDlg::IDD, pParent)
    , m_nSelectedEntry(0)
{
}

//////////////////////////////////////////////////////////////////////////////////////////

CGoToDlg::~CGoToDlg()
{
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CGoToDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT( !m_arFileNames.IsEmpty() ); // (sanity check; why else would we be called?!)

    for (int i=0, k=(int)m_arFileNames.GetCount(); i < k; i++)
        m_FileNamesCombo.AddString( m_arFileNames[i] );

    m_FileNamesCombo.SetCurSel( m_nSelectedEntry );

    CWindowPlacement wp; wp.Restore( _T( "GoToDlg" ), this, FALSE );

    return FALSE;   // return TRUE unless you set the focus to a control
                    // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////////////////////

void CGoToDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_FILENAMES_COMBOBOX, m_FileNamesCombo);

    if (!pDX->m_bSaveAndValidate) return;

    VERIFY( CB_ERR != (m_nSelectedEntry = m_FileNamesCombo.GetCurSel()) );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CGoToDlg::OnDestroy()
{
    CWindowPlacement wp; wp.Save( _T( "GoToDlg" ), this );
    CDialog::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////////////////////
