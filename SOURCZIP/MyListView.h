// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// MyListView.h : interface of the CMyListView class
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  03/19/06    1.3.0   Fish    UNICODE fixes
//  03/20/06    1.3.0   Fish    Make dialog resizeable
//  08/05/06    1.4.0   Fish    Support for searching within block, file or entire tape.
//  12/07/06    1.5.0   Fish    Maintain list of HDR1 names for new "Go To..." command
//  01/14/07    1.5.0   Fish    Changes needed for new FishLib
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

class CAWSBrowseDoc;        // (forward reference)

//////////////////////////////////////////////////////////////////////////////////////////

class CMyListView : public CListView
{
protected:

    CMyListView();

    DECLARE_DYNCREATE(CMyListView)

public:

    CAWSBrowseDoc*  GetDocument();
    bool            HaveSearchString()  { return !m_strSearchString.IsEmpty(); };
    bool            HaveGoto();

    void SaveProfileSettings();

    //{{AFX_VIRTUAL(CMyListView)
    public:
    virtual void OnDraw(CDC* pDC);  // overridden to draw this view
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
    protected:
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
    //}}AFX_VIRTUAL

public:

    virtual ~CMyListView();

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

    void LoadProfileSettings();

    CListCtrl*      m_pListCtrl;            // (ptr to actual CListCtrl object)
    CAWSBrowseDoc*  m_pDoc;                 // (ptr to associated CDocument object)

    void FillListControlWithData();         // (intialization)
                                            // (init helper)
    void SaveStdLabels( int nChunkNum, CAWSFileStats& file_stats, BYTE* pExpandedData );

    void SelectItem( int nItemNum );        // (updates below variables)
    void DisplaySelectedItem();             // (notifies hexedit control)

    int  m_nSelectedItemNum;                // (points to currently selected item)
    int  m_nSelectedChunkNum;               // (currently selected/displayed chunk)

    CStringArray  m_arHDR1Names;            // (array of HDR1 file names)
    CArray<int>   m_arHDR1ChunkNums;        // (corresponding chunk nums)
    int           m_nGoToEntry;             // (last selected GoTo entry)

    /////////////////////////////////////////////////////////////////
    // Find/Replace support...

    void UpdateSearchString();              // (updates m_strSearchString et al.)
    void UpdateSearchParms();               // (updates m_strPrevSearchString et al.)

    // (the following variables remain for the life of the program)

    CList< CString, LPCTSTR >  m_FindWhatList;

    CString  m_strSearchString;             // string to find (always ASCII)
    BOOL     m_bForwSearch;                 // forw/back search direction
    BOOL     m_bWholeWord;                  // (match whole words only for text searches)
    BOOL     m_bMatchCase;                  // (respect case for text searches)
    BOOL     m_bHexSearch;                  // (do hex search instead of text)
    int      m_nSearchWhere;                // (0==block, 1==file, 2==tape)

    CString  m_strPrevSearchString;         // (used to init below data)
    BOOL     m_bPrevEBCDICMode;             // (used to init below data)
    BOOL     m_bPrevTextArea;               // (used to init below data)

    BYTE*    m_pSearchData;                 // ASCII/EBCDIC search data   (NOTE: always SBCS!)
    SIZE_T   m_nSearchDataLen;              // length of search data      (NOTE: always SBCS!)
    SIZE_T   m_nForwShiftTable[256];        // Pratt-Boyer-Moore shift table
    SIZE_T   m_nBackShiftTable[256];        // Pratt-Boyer-Moore shift table

    // (helper functions)

    CString BuildHexSearchString ( BYTE* pData, SSIZE_T nDataLen );
    CString GetBlockDataAsCString( BYTE* pData, SSIZE_T nDataLen, BOOL bEBCDIC );

    // (the following variables are reinitialized for each new document loaded
    // and updated as the selection changes...)

    bool LoadItemData( int nItemNum );      // (loads requested data block)

    BYTE*    m_pDataBlock;                  // ptr --> loaded data block
    SSIZE_T  m_nBlockLen;                   // length of loaded data block
    SSIZE_T  m_nBlockPos;                   // current location within data block

    bool   FindNext();                      // (find next occurrence)
    bool   FindPrev();                      // (find previous occurrence)
    bool   DoSearch();                      // (performs the actual search)
    BYTE*  DoTextSearch();                  // (DoSearch helper function for text searches)
    bool   NotFound();                      // (issues "Not Found" message)

    SSIZE_T  GetNextSearchBlock( int& nItemNum );       // (DoSearch helper)

    /////////////////////////////////////////////////////////////////
    // Printing/Print Preview support...

    CRect   CalcPrintRect   ( CDC* pDC = NULL );                // (calculate 'm_rectDraw' ourselves! Also
                                                                //  initializes minimum margin values for
                                                                //  default printer)
    bool    DoPageSetup     ();                                 // (set margins, select printer, etc)

    size_t  BytesPerLine    ( CRect rectPrint );                // (calc #of bytes per line that will fit on a page)
    size_t  m_nBytesPerLine;                                    // (#of bytes per line will fit on a page)

    UINT    LinesPerPage    ( CRect rectPrint );                // (calc #of print lines that will fit on a page)
    UINT    NumPrintPages   ( CRect rectPrint );                // (calc #of pages needed to print current data block)
    size_t  CalcPrintPos    ( CRect rectPrint, UINT nPageNum ); // (calc current block location for this page#)
    CRect   AdjustPrintRect ( CRect rectPrint );                // (apply user specified margins to print formatting rectangle)

    CString m_strPrinterFontName;
    UINT    m_nPrinterFontSize;
    CFont*  m_pPrinterFont; // (fixed for now)
    UINT    m_nLineHeight;  // (MM_LOENGLISH = 1000ths/inch)
    UINT    m_nCharWidth;   // (MM_LOENGLISH = 1000ths/inch)

    // Note: margins are always POSITIVE values in 1000ths/inch. The minimum margin
    // values are initialized by the CalcPrintRect function. The other margin values
    // are specified by the user via the DoPageSetup function...

    int     m_nLeftMargin,    m_nRightMargin,    m_nTopMargin,    m_nBottomMargin;
    int     m_nMinLeftMargin, m_nMinRightMargin, m_nMinTopMargin, m_nMinBottomMargin;

    /////////////////////////////////////////////////////////////////

    //{{AFX_MSG(CMyListView)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnEditFind();
    afx_msg void OnEditFindNext();
    afx_msg void OnEditFindPrevious();
    afx_msg void OnEditNextBlock();
    afx_msg void OnEditPreviousBlock();
    afx_msg void OnEditNexFile();
    afx_msg void OnEditPreviousFile();
    afx_msg void OnFilePrint();
    afx_msg void OnFilePageSetup();
    afx_msg void OnFilePrintPreview();
    afx_msg void OnViewPrinterFont();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptScreen);
    afx_msg void OnListViewRightClickInfo();
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnViewStatistics();
    afx_msg void OnUpdateEditGoto(CCmdUI *pCmdUI);
    afx_msg void OnEditGoto();
    //}}AFX_MSG

    void OnItemStateChange( NMHDR* pNotifyStruct, LRESULT* pResult );

    DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DEBUG
inline CAWSBrowseDoc* CMyListView::GetDocument() { return (CAWSBrowseDoc*) m_pDocument; }
#endif

//////////////////////////////////////////////////////////////////////////////////////////
