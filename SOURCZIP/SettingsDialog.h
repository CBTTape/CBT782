// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// SettingsDialog.h : header file
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  03/19/06    1.3.0   Fish    Text-only record length
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

//////////////////////////////////////////////////////////////////////////////////////////

class CSettingsDialog : public CDialog
{
public:

    CSettingsDialog( CWnd* pParent = NULL );    // standard constructor

    //{{AFX_DATA(CSettingsDialog)
    enum { IDD = IDD_SETTINGS_DIALOG };
    CSpinButtonCtrl m_BPGSpinner;
    UINT            m_nBPG;
    CEdit           m_BPREdit;
    CSpinButtonCtrl m_BPRSpinner;
    UINT            m_nBPR;
    CButton         m_DynBPRCheckbox;
    BOOL            m_bDynBPR;
    CEdit           m_RecLenEdit;
    UINT            m_nTextRecLen;
    CSpinButtonCtrl m_RecLenSpinner;
    int             m_nFormat;
    //}}AFX_DATA

#define  ASCII_FORMAT    (0)        // (first radio button is "ASCII")
#define  EBCDIC_FORMAT   (1)        // (second radio button is "EBCDIC")

    //{{AFX_VIRTUAL(CSettingsDialog)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    //{{AFX_MSG(CSettingsDialog)
    virtual BOOL OnInitDialog();
    afx_msg void OnDynCheckbox();
    afx_msg void OnDestroy();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////////////////
