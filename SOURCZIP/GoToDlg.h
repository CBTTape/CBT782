// Copyright (c) 2006-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// GoToDlg.h : interface of the CGoToDlg class
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  12/07/06    1.5.0   Fish    Created
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////

class CGoToDlg : public CDialog
{
public:

    CStringArray  m_arFileNames;        // (passed by caller)
    int           m_nSelectedEntry;     // (passed to caller)

	CGoToDlg(CWnd* pParent = NULL);     // standard constructor
	virtual ~CGoToDlg();

	enum { IDD = IDD_GOTO_DIALOG };

protected:

    CComboBox  m_FileNamesCombo;

    virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDestroy();
};

//////////////////////////////////////////////////////////////////////////////////////////
