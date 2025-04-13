// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// AWSBrowseDoc.h : interface of the CAWSBrowseDoc class
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  06/15/06    1.5.0   Fish    VS2005, x64
//  11/21/06    1.5.0   Fish    Load zlib/bzip2 DLLs at startup
//  11/21/06    1.5.0   Fish    Added "File -> Close" command.
//  11/21/06    1.5.0   Fish    HandleOpenDocumentError
//  12/05/06    1.5.0   Fish    Maintain general file statistics (CAWSFileStats)
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

class CProgressDlg;     // (forward reference)
class CMyListView;      // (forward reference)
class CMyHexEditView;   // (forward reference)

//////////////////////////////////////////////////////////////////////////////////////////

class CAWSBrowseDoc : public CDocument
{
protected:      // create from serialization only

    CAWSBrowseDoc();

    DECLARE_DYNCREATE( CAWSBrowseDoc )

public:

    CString   m_strVol1Label;           // (must be set manually because we don't)

    BYTE      m_bAWSFlag1;              // (combined from ALL chunks together)
    BYTE      m_bAWSFlag2;              // (combined from ALL chunks together)

    CMyListView*     m_pListView;       // (set by CMyListView::OnCreate)
    CMyHexEditView*  m_pHexEditView;    // (set by CMyHexEditView::OnCreate)

    /////////////////////////////////////////////////////////////////
    // The following utility function is used by the CMyHexEditView
    // object to retrieve the actual "tape" file data block in both
    // compressed and uncompressed format. The chunk# that is passed
    // identifies which data block is requested and must point to a
    // chunk info item of "NormalBlock" or "BeginningOfBlock" type.

    bool  GetBlockData              // (true/false success/failure)
    (
        int     nChunkNum,          // IN   where this block begins
        BYTE*&  pCompressedData,    // OUT  --> raw compressed data
        int&    nCompressedLen,     // OUT  ==  raw compressed len
        BYTE*&  pExpandedData,      // OUT  --> expanded data block
        int&    nExpandedLen        // OUT  ==  expanded block len
    );

    /////////////////////////////////////////////////////////////////
    // Function to retrieve a read-only handle to the document file...

    HANDLE  GetFileHandle()  { return m_hFile; };

    /////////////////////////////////////////////////////////////////
    // Function to return #of CAWSChunkInfo items there are...

    int   NumChunks()  { return (int)m_arChunkInfo.GetSize(); };
    bool  IsEmpty()    { return NumChunks() ? false : true; };

    /////////////////////////////////////////////////////////////////
    // Function to return a specific CAWSChunkInfo item...

    CAWSChunkInfo&  ChunkInfo( int nChunkNum );

    /////////////////////////////////////////////////////////////////
    // Functions to get/set a specific CAWSFileStats item...

    CAWSFileStats&  GetFileStats( int nFileNum );
    bool            SetFileStats( int nFileNum, const CAWSFileStats& rStats );
    int             NumFiles()  { return (int)m_arFileStats.GetSize(); };

    /////////////////////////////////////////////////////////////////
    // Ptr to progress dialog displayed while file is loading...

    CProgressDlg*  m_pProgressDlg;
    BOOL           m_bCancelled;    // ('Cancel' button was clicked)

    /////////////////////////////////////////////////////////////////


    //{{AFX_VIRTUAL(CAWSBrowseDoc)
    public:
    virtual BOOL OnNewDocument();
    virtual void DeleteContents();
    virtual BOOL OnOpenDocument( LPCTSTR pszPathName );
    //}}AFX_VIRTUAL

public:

    virtual ~CAWSBrowseDoc();

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

    BOOL   MyDoOpenDocument( LPCTSTR pszPathName );
    BOOL   ReturnOpenDocumentError( HANDLE& hFile, LPCTSTR pszPathName, DWORD dwLastError );
    bool   HandleOpenDocumentError( DWORD dwLastError, LPCTSTR pszPathName );

    bool   m_bZLibOK;       // (whether or not zlib decompression dll loaded ok or not)
    bool   m_bBZip2OK;      // (whether or not zlib decompression dll loaded ok or not)
    bool   m_bDidWarning;   // (whether or not we issued our missing dll warning message)

    BYTE*  m_pTempCompressedDataBuffer;     // (work)
    BYTE*  m_pTempExpandedDataBuffer;       // (work)

    // The following is a duplicate handle to the file we opened and
    // serialized (read) into our document object. It is purposely kept
    // open to prevent other processes from trying to open the same file
    // with write access in an attempt to update it while we're using it.

    HANDLE  m_hFile;        // (handle to document file)

    HANDLE  CreateReadOnlyDuplicateOfFileHandle();  // (helper function)

    // The following is an array of CAWSChunkInfo objects encompassing the
    // information pertaining to the list of AWS chunks on the 'tape'. It
    // allows us to directly access any chunk on the 'tape'...

    CArray< CAWSChunkInfo, CAWSChunkInfo& >  m_arChunkInfo;

    // The following array hold statistical information pertaining to each
    // tapemark-separated file on the tape. It is only used for informational
    // reporting purposes only, and nothing else.

    CArray< CAWSFileStats, CAWSFileStats& >  m_arFileStats;

    //{{AFX_MSG(CAWSBrowseDoc)
    //}}AFX_MSG

    afx_msg void OnUpdateMyFileClose(CCmdUI *pCmdUI);
    afx_msg void OnMyFileClose();

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////////////////
