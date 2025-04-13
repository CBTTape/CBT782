// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// AWSBrowse.h : main header file for the AWSBROWSE application
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

//////////////////////////////////////////////////////////////////////////////////////////

class CAWSBrowseApp : public CWinApp
{
public:

    CAWSBrowseApp() {};

    BOOL  SetMFCsDefaultPrinterToLandscapeMode();

    HANDLE  GetMFCsDefaultPrinterDEVMODEHandle()  { return m_hDevMode; };
    HANDLE  GetMFCsDefaultPrinterDEVNAMESHandle() { return m_hDevNames; };

    //{{AFX_VIRTUAL(CAWSBrowseApp)
    public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    //}}AFX_VIRTUAL

    //{{AFX_MSG(CAWSBrowseApp)
    afx_msg void OnAppAbout();
        // NOTE - the ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG

protected:

    void LoadMyStdProfileSettings();
    void SaveMyStdProfileSettings();

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////////////////
// The one and only application object...

extern CAWSBrowseApp  g_App;    // (this one object does it all; it's the entire program)

//////////////////////////////////////////////////////////////////////////////////////////
