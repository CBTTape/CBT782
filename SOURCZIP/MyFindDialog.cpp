// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// MyFindDialog.cpp : implementation file
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  03/19/06    1.3.0   Fish    Disable hex search option if hex area not visible
//  06/15/06    1.5.0   Fish    VS2005, x64
//  08/05/06    1.4.0   Fish    Support for searching within block, file or entire tape.
//  01/17/07    1.5.0   Fish    Fix bug when empty search string list passed
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyFindDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (embed copyright into object code)
#pragma comment ( user, COPYRIGHT_EXESTR )

//////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CMyFindDialog, CDialog )

    //{{AFX_MSG_MAP(CMyFindDialog)
    ON_CBN_EDITCHANGE(IDC_FINDWHAT_COMBOBOX, OnEditChangeFindWhatCombobox)
    ON_WM_DESTROY()
    ON_CBN_SELENDOK(IDC_FINDWHAT_COMBOBOX, OnSelendokFindwhatCombobox)
    //}}AFX_MSG_MAP

END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////

CMyFindDialog::CMyFindDialog( CWnd* pParent /* =NULL*/ )

    : CDialog( CMyFindDialog::IDD, pParent )

{
    //{{AFX_DATA_INIT(CMyFindDialog)
    m_bWholeWord = FALSE;
    m_bMatchCase = FALSE;
    m_bHex = FALSE;
    m_nWhere = 0;
    m_nDirection = 0;
    //}}AFX_DATA_INIT

    // Defaults...

    m_nWhere     = FIND_ONTAPE;
    m_nDirection = FIND_DOWN;
    m_bIsHexView = TRUE;            // (will be (should be) set properly later by caller)
}

//////////////////////////////////////////////////////////////////////////////////////////

BOOL CMyFindDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Fill the combo box with the CList items...

    m_FindWhatComboBox.ResetContent();
    m_FindWhatComboBox.LimitText( MAX_HEX_SEARCH_STRING_LEN );

    POSITION pos = m_FindWhatList.GetHeadPosition();

    while ( pos )
        m_FindWhatComboBox.AddString( m_FindWhatList.GetNext( pos ) );

    if ( !m_FindWhatList.IsEmpty() )
        m_FindWhatButton.EnableWindow( TRUE );

    m_FindWhatComboBox.SetCurSel(0);

    OnSelendokFindwhatCombobox();

    CWindowPlacement wp; wp.Restore( _T( "FindDlg" ), this, FALSE );

    if (!m_bIsHexView)
        m_HexCheckbox.EnableWindow( m_bHex = FALSE );

    return TRUE;    // return TRUE unless you set the focus to a control
                    // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyFindDialog::OnEditChangeFindWhatCombobox()
{
    CString strFindWhat;
    m_FindWhatComboBox.GetWindowText( strFindWhat );
    UpdateControlsBasedOnNewFindWhat( strFindWhat );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyFindDialog::OnSelendokFindwhatCombobox() 
{
    int nIndex = m_FindWhatComboBox.GetCurSel();
    if (nIndex >= 0)
    {
        CString strFindWhat;
        m_FindWhatComboBox.GetLBText( nIndex, strFindWhat );
        UpdateControlsBasedOnNewFindWhat( strFindWhat );
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyFindDialog::UpdateControlsBasedOnNewFindWhat( LPCTSTR pszFindWhat )
{
    m_bWholeWord  =                    IsWholeWord          ( pszFindWhat );
    m_bHex        =  m_bIsHexView  &&  ValidHexSearchString ( pszFindWhat );

    if ( !m_bWholeWord )
        m_WholeWordCheckbox . SetCheck ( FALSE  );
    m_HexCheckbox           . SetCheck ( m_bHex );

    m_FindWhatButton    . EnableWindow ( *pszFindWhat                 );
    m_MatchCaseCheckbox . EnableWindow ( *pszFindWhat                 );
    m_WholeWordCheckbox . EnableWindow ( *pszFindWhat && m_bWholeWord );
    m_HexCheckbox       . EnableWindow ( *pszFindWhat && m_bHex       );
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyFindDialog::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );

    //{{AFX_DATA_MAP(CMyFindDialog)
    DDX_Control(pDX, IDC_FINDWHAT_COMBOBOX, m_FindWhatComboBox);
    DDX_Control(pDX, IDC_WHOLEWORD_CHECKBOX, m_WholeWordCheckbox);
    DDX_Check(pDX, IDC_WHOLEWORD_CHECKBOX, m_bWholeWord);
    DDX_Control(pDX, IDC_MATCHCASE_CHECKBOX, m_MatchCaseCheckbox);
    DDX_Check(pDX, IDC_MATCHCASE_CHECKBOX, m_bMatchCase);
    DDX_Radio(pDX, IDC_UPDOWN_RADIO_BUTTON_GROUP, m_nDirection);
    DDX_Control(pDX, IDOK, m_FindWhatButton);
    DDX_Control(pDX, IDC_HEX_CHECKBOX, m_HexCheckbox);
    DDX_Check(pDX, IDC_HEX_CHECKBOX, m_bHex);
    DDX_Radio(pDX, IDC_WHERE_RADIO_BUTTON_GROUP, m_nWhere);
    //}}AFX_DATA_MAP

    if ( pDX->m_bSaveAndValidate )
    {
        if ( !ValidData( pDX ) )
            pDX->Fail();

        // Return all combo box items in CList object
        // in the same order as they're in...

        m_FindWhatList.RemoveAll();

        CString strFindWhat, str;

        m_FindWhatComboBox.GetWindowText( strFindWhat );

        if ( !strFindWhat.IsEmpty() )
            m_FindWhatList.AddTail( strFindWhat );

        for ( int i=0, k = m_FindWhatComboBox.GetCount(); i < k; i++ )
        {
            m_FindWhatComboBox.GetLBText( i, str );

            if ( !str.IsEmpty() && str.Compare( strFindWhat ) != 0 )
                m_FindWhatList.AddTail( str );
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

bool CMyFindDialog::ValidData( CDataExchange* pDX )
{
    CString strFindWhat; m_FindWhatComboBox.GetWindowText( strFindWhat );
    ASSERT(  ValidHexSearchString( strFindWhat ) || !m_bHex );
    if    ( !ValidHexSearchString( strFindWhat ) &&  m_bHex )
    {
        AfxMessageBox( _T( "Invalid hex string" ) );
        pDX->PrepareCtrl(IDC_FINDWHAT_COMBOBOX);
        return false;
    }
    if ( !m_bWholeWord ) return true;
    if ( IsWholeWord( strFindWhat ) ) return true;
    AfxMessageBox( _T( "Please enter a whole word" ) );
    pDX->PrepareCtrl(IDC_FINDWHAT_COMBOBOX);
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CMyFindDialog::OnDestroy()
{
    CWindowPlacement wp; wp.Save( _T( "FindDlg" ), this );
    CDialog ::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////////////////////
