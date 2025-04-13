// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// ProgDlg.h : header file
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////

class CProgressDlg : public CDialog
{
public:

    CProgressDlg( UINT nCaptionID = IDS_PROGRESS_CAPTION );
    ~CProgressDlg();

    CString  m_strTitle;    // (optional title used if non-empty)

    BOOL  Create( CWnd *pParent = NULL );

    BOOL  CheckCancelButton();

    void  SetRange32 ( int nLower, int nUpper );
    int   SetStep    ( int nStep );
    int   SetPos     ( int nPos  );
    int   OffsetPos  ( int nPos  );
    int   StepIt     ();

    //{{AFX_DATA(CProgressDlg)
    enum { IDD = IDD_PROGRESS };
    CProgressCtrl   m_Progress;
    //}}AFX_DATA

    //{{AFX_VIRTUAL(CProgressDlg)
    public:
    virtual BOOL DestroyWindow();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    UINT  m_nCaptionID;
    int   m_nLower, m_nUpper, m_nStep;
    BOOL  m_bCancelled;
    BOOL  m_bParentDisabled;

    void  ReEnableParent();
    void  UpdatePercent( int nCurrent );
    void  PumpMessages();

    //{{AFX_MSG(CProgressDlg)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG

    virtual void OnOK() {};
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////////////////
