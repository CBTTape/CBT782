// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// BlockDetailDlg.h : interface of the CBlockDetailDlg class
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  12/05/06    1.5.0   Fish    Alternate window title support.
//  12/06/06    1.5.0   Fish    Make read-only! (oops!)
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////

class CBlockDetailDlg : public CDialog
{
public:

    CBlockDetailDlg( CWnd* pParent = NULL );        // standard constructor

    CString  m_strTitle;                            // (optional window title)
    CString  m_strWindowPlacementKeyName;           // (since we're multi-purpose)

    //{{AFX_DATA(CBlockDetailDlg)
    enum { IDD = IDD_BLOCKINFO_DIALOG };
    CEdit   m_DetailsEdit;
    CString m_strDetails;
    CButton m_OKButton;
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CBlockDetailDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

#define  RGB_BlockDetailDlgBkColor   RGB(255,255,255)   // (solid white)

    HBRUSH m_hBrush;    // (for OnCtlColor background painting)
    CFont  m_Font;
    CSize  m_sizeMin;
    CRect  m_rectDlg;
    bool   m_bInitDialogDone;

    //{{AFX_MSG(CBlockDetailDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnWindowPosChanging(WINDOWPOS* pwndPos);
    afx_msg void OnDestroy();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////////////////
