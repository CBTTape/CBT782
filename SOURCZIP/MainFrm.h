// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// MainFrm.h : interface of the CMainFrame class
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  02/06/07    1.5.0   Fish    BCMenu
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

class CAWSBrowseDoc;    // (forward reference)

//////////////////////////////////////////////////////////////////////////////////////////

class CMainFrame : public CFrameWnd
{
protected:      // create from serialization only

    DECLARE_DYNCREATE( CMainFrame )

    CMainFrame() {};

public:

    CAWSBrowseDoc*  GetDocument()   { return (CAWSBrowseDoc*)GetActiveDocument(); };

    HMENU NewMenu();                // BCMenu
    BCMenu m_menu;                  // BCMenu

    //{{AFX_VIRTUAL(CMainFrame)
    public:
    virtual BOOL OnCreateClient(CREATESTRUCT* pCS, CCreateContext* pContext);
    protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    //}}AFX_VIRTUAL

public:

    virtual ~CMainFrame() {};
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump( CDumpContext& dc ) const;
#endif

protected:

    CToolBar      m_wndToolBar;
    CStatusBar    m_wndStatusBar;
    CSplitterWnd  m_wndSplitter;

    //{{AFX_MSG(CMainFrame)
    afx_msg int OnCreate(CREATESTRUCT* pCS);
    afx_msg void OnUpdateEditFind(CCmdUI* pCmdUI);
    afx_msg void OnEditFind();
    afx_msg void OnUpdateViewAscii(CCmdUI* pCmdUI);
    afx_msg void OnViewAscii();
    afx_msg void OnUpdateViewEbcdic(CCmdUI* pCmdUI);
    afx_msg void OnViewEbcdic();
    afx_msg void OnUpdateEditFindNext(CCmdUI* pCmdUI);
    afx_msg void OnEditFindNext();
    afx_msg void OnUpdateEditFindPrevious(CCmdUI* pCmdUI);
    afx_msg void OnEditFindPrevious();
    afx_msg void OnUpdateEditHexArea(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditTextArea(CCmdUI* pCmdUI);
    afx_msg void OnClose();
    afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
    afx_msg void OnEditCopy();
    afx_msg void OnUpdateEditNextBlock(CCmdUI* pCmdUI);
    afx_msg void OnEditNextBlock();
    afx_msg void OnUpdateEditPreviousBlock(CCmdUI* pCmdUI);
    afx_msg void OnEditPreviousBlock();
    afx_msg void OnUpdateEditNextFile(CCmdUI* pCmdUI);
    afx_msg void OnEditNextFile();
    afx_msg void OnUpdateEditPreviousFile(CCmdUI* pCmdUI);
    afx_msg void OnEditPreviousFile();
    afx_msg void OnHelpDLLsVersionInformation();
    afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);
    afx_msg void OnEditTextArea();
    afx_msg void OnEditHexArea();
    afx_msg void OnViewSettings();
    afx_msg void OnUpdateFilePageSetup(CCmdUI* pCmdUI);
    afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
    afx_msg void OnFilePrint();
    afx_msg void OnFilePrintPreview();
    afx_msg void OnFilePageSetup();
    afx_msg void OnViewDisplayFont();
    afx_msg void OnUpdateViewDisplayFont(CCmdUI* pCmdUI);
    afx_msg void OnViewPrinterFont();
    afx_msg void OnUpdateViewPrinterFont(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewHex(CCmdUI* pCmdUI);
    afx_msg void OnViewHex();
    afx_msg void OnUpdateViewBoth(CCmdUI* pCmdUI);
    afx_msg void OnViewBoth();
    afx_msg void OnUpdateViewTextOnly(CCmdUI* pCmdUI);
    afx_msg void OnViewTextOnly();
    afx_msg void OnViewStatistics();
    afx_msg void OnUpdateViewStatistics(CCmdUI *pCmdUI);
    afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
    afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////////////////
