// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// MyHexEditView.h : header file
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  12/07/06    1.5.0   Fish    IsEmpty()
//  01/14/07    1.5.0   Fish    Changes needed for new FishLib
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "HexEdit.h"        // (need class CHexEdit)
class CAWSBrowseDoc;        // (forward reference)

//////////////////////////////////////////////////////////////////////////////////////////

class CMyHexEditView : public CView
{
protected:

    CMyHexEditView();       // protected constructor used by dynamic creation

    DECLARE_DYNCREATE( CMyHexEditView )

public:

    CAWSBrowseDoc*  GetDocument();
    CHexEdit*       GetHexEditCtrlPtr() { return &m_HexEditCtrl; };

    void SaveProfileSettings();

    void SetBPG( BYTE nBPG );
    void SetBPR( BYTE nBPR );
    void SetDynBPR( BOOL bDynBPR = TRUE );
    void SetASCIIMode();
    void SetEBCDICMode();
    void SetHexView( BOOL bEnable = TRUE );
    void SetTextArea( BOOL bTextArea = TRUE );

    // CAUTION! The following function does a 'new'. Caller is
    // responsible for doing the 'delete' to prevent memory leak!

    BOOL GetSelectedData( BYTE*& pSelectedData, SSIZE_T& nSelectedDataLen );

    BYTE GetBPG()        { return  m_nBytesPerGroup; };
    BYTE GetBPR()        { return  m_nBytesPerLine;  };
    BOOL IsDynBPR()      { return  m_bDynBPR;        };
    BOOL IsASCIIMode()   { return !m_bEBCDICMode;    };
    BOOL IsEBCDICMode()  { return  m_bEBCDICMode;    };
    BOOL IsHexView()     { return  m_bViewHex;       };
    UINT GetTextRecLen() { return  m_nTextRecLen;    };
    BOOL IsTextArea();
    BOOL IsDataSelected();
    BOOL IsEmpty()       { return m_HexEditCtrl.IsEmpty(); };

    //{{AFX_VIRTUAL(CMyHexEditView)
    protected:
    virtual void OnDraw(CDC* pDC);      // overridden to draw this view
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    //}}AFX_VIRTUAL

protected:

    virtual ~CMyHexEditView();

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

    void  LoadProfileSettings();
    void  CreateLogFont();

    CString   m_strFontName;
    int       m_nFontSize;      // (in points)
    LOGFONT   m_LogFont;

    CHexEdit  m_HexEditCtrl;
    HEPARMS   m_HexEditParms;

    UINT      m_cpDefault;
    UINT      m_cpEBCDIC;

    BOOL      m_bEBCDICMode;
    BOOL      m_bViewHex;

    int       m_nBytesPerGroup;
    int       m_nBytesPerLine;
    BOOL      m_bDynBPR;
    int       m_nIndent;
    UINT      m_nTextRecLen;

    COLORREF  m_crTextColor;
    COLORREF  m_crBGColor;

    COLORREF  m_crHiTextColor;
    COLORREF  m_crHiBGColor;

    void  SetInitialHexEditParms();
    void  GetHexEditParms();

    //{{AFX_MSG(CMyHexEditView)
    afx_msg int OnCreate(CREATESTRUCT* pCS);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnEditHexArea();
    afx_msg void OnEditTextArea();
    afx_msg void OnViewSettings();
    afx_msg void OnViewAscii();
    afx_msg void OnViewEbcdic();
    afx_msg void OnViewDisplayFont();
    afx_msg void OnViewHex();
    afx_msg void OnUpdateEditGoto(CCmdUI *pCmdUI);
    afx_msg void OnEditGoto();
    //}}AFX_MSG

    afx_msg void OnEditCopy();
    afx_msg void OnEditSelectAll();

    LRESULT OnMyRefreshViewMsg( WPARAM nChunkNum, LPARAM );

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DEBUG
inline CAWSBrowseDoc* CMyHexEditView::GetDocument() { return (CAWSBrowseDoc*) m_pDocument; }
#endif

//////////////////////////////////////////////////////////////////////////////////////////
