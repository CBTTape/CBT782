// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// About.h : header file
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////

class CAboutDlg : public CDialog
{
public:

    CAboutDlg();

    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    CHyperLink  m_ProductVersionStatic;
    CString     m_strProductVersion;
    CStatic     m_CopyrightCompanyStatic;
    CString     m_strCopyrightCompany;
    CHyperLink  m_ContactEmailStatic;
    CString     m_strContactEmail;
    //}}AFX_DATA

    //{{AFX_VIRTUAL(CAboutDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    //{{AFX_MSG(CAboutDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////////////////
