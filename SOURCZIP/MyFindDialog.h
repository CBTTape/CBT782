// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// MyFindDialog.h : header file
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  03/19/06    1.3.0   Fish    Disable hex search option if hex area not visible
//  08/05/06    1.5.0   Fish    Support for searching within block, file or entire tape.
//  02/26/08    1.5.2   Fish    CFLComboBoxEx.
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////

class CMyFindDialog : public CDialog
{
public:

    CMyFindDialog( CWnd* pParent = NULL );  // standard constructor

    CList< CString, LPCTSTR >  m_FindWhatList;  // (must be set by caller)
    BOOL                       m_bIsHexView;    // (must be set by caller)

    //{{AFX_DATA(CMyFindDialog)
    enum { IDD = IDD_MY_FIND_DIALOG };
    CFLComboBoxEx  m_FindWhatComboBox;
    CButton        m_WholeWordCheckbox;
    BOOL           m_bWholeWord;
    CButton        m_MatchCaseCheckbox;
    BOOL           m_bMatchCase;
    CButton        m_HexCheckbox;
    BOOL           m_bHex;
    int            m_nWhere;
    int            m_nDirection;
    CButton        m_FindWhatButton;
    //}}AFX_DATA

    // Radio button group values

#define  FIND_INBLOCK  (0)      // m_nWhere     (first  radio button is "Block")
#define  FIND_INFILE   (1)      // m_nWhere     (second radio button is "File")
#define  FIND_ONTAPE   (2)      // m_nWhere     (third  radio button is "Tape")

#define  FIND_UP       (0)      // m_nDirection (first  radio button is "Up")
#define  FIND_DOWN     (1)      // m_nDirection (second radio button is "Down")

    //{{AFX_VIRTUAL(CMyFindDialog)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    void UpdateControlsBasedOnNewFindWhat( LPCTSTR pszFindWhat );

    bool ValidData( CDataExchange* pDX );

    //{{AFX_MSG(CMyFindDialog)
    virtual BOOL OnInitDialog();
    afx_msg void OnEditChangeFindWhatCombobox();
    afx_msg void OnDestroy();
    afx_msg void OnSelendokFindwhatCombobox();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////////////////
