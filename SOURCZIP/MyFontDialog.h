// Copyright (c) 2006-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// MyFontDialog.h : interface of the CMyFontDialog class
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  12/09/06    1.5.0   Fish    Created
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////
// The below class is simply so I can give a window title of my own choosing to
// the standard system dialog which otherwise has generic non-informative title.

class CMyFontDialog : public CFontDialog
{
public:

    CMyFontDialog
    (
        LPCTSTR    pszTitle     = _T( "Choose Font" ),
        LPLOGFONT  lplfInitial  = NULL,
        DWORD      dwFlags      =
        0
        | CF_EFFECTS
        | CF_SCREENFONTS
        ,
        CDC*       pdcPrinter   = NULL,
        CWnd*      pParentWnd   = NULL
    )
        : CFontDialog ( lplfInitial, dwFlags, pdcPrinter, pParentWnd )
        , m_strTitle  ( pszTitle )
    {
    };

protected:

    CString  m_strTitle;

    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////////////////
