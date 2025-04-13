// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// AWS.h : interface file for the AWS classes
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  03/20/06    1.3.0   Fish    Add support for Bus-Tech ZLIB compression
//  11/21/06    1.5.0   Fish    Add CAWSChunkHdr::Init( WORD,WORD,BYTE,BYTE ) overload.
//  11/23/06    1.5.0   Fish    Recognize Bus-Tech hardware compression flag.
//  11/24/06    1.5.0   Fish    Add support for Bus-Tech segmented-block flag.
//  12/05/06    1.5.0   Fish    CAWSFileStats
//  12/05/06    1.5.0   Fish    Maintain file# in CAWSChunkInfo
//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////

#define  MAX_AWSCHUNKSIZE   ( 0xFFFF )

//////////////////////////////////////////////////////////////////////////////////////////

typedef enum EAWSChunkType
{
    Invalid,                // Invalid block (one/more reserved flags or zero length)
    BeginningOfBlock,       // First chunk of large block
    IntermediateBlock,      // Intermediate chunk of large block
    EndOfBlock,             // Last chunk of large block
    NormalBlock,            // Normal block (block is both first and last chunk)
    TapeMark,               // Tapemark
}
EAWSChunkType;

//////////////////////////////////////////////////////////////////////////////////////////
// Information pertaining to a chunk of data in an AWS/HET "tape" file...

class CAWSChunkHdr
{
protected:

    BYTE   m_CurrChunkLen[2];   // length of this chunk (LITTLE ENDIAN!)
    BYTE   m_PrevChunkLen[2];   // length of prev chunk (LITTLE ENDIAN!)
    BYTE   m_Flag1;             // (see #defines below)
    BYTE   m_Flag2;             // (see #defines below)

    //         Definitions for flag byte 1

    #define AWSHDR_FLAG1_BEGBLK         0x80    // Start of new block
    #define AWSHDR_FLAG1_TAPEMARK       0x40    // Tapemark
    #define AWSHDR_FLAG1_ENDBLK         0x20    // End of current block
    #define AWSHDR_FLAG1_BUSTECH_MIDBLK 0x10    // Bus-Tech: intermediate block
    #define AWSHDR_FLAG1_RESERVED       0x0C    // (reserved)
    #define AWSHDR_FLAG1_HERC_BZIP2     0x02    // Hercules: BZIP2 compressed
    #define AWSHDR_FLAG1_HERC_ZLIB      0x01    // Hercules: ZLIB compressed

    #define AWSHDR_FLAG1_COMPRESSED   (AWSHDR_FLAG1_HERC_ZLIB | AWSHDR_FLAG1_HERC_BZIP2)

    //         Definitions for flag byte 2

    #define AWSHDR_FLAG2_BUSTECH_ZLIB   0x80    // Bus-Tech: ZLIB compressed
    #define AWSHDR_FLAG2_BUSTECH_HW     0x40    // Bus-Tech: hardware compresed
    #define AWSHDR_FLAG2_RESERVED       0x3F    // (reserved)

    #define AWSHDR_FLAG2_COMPRESSED   (AWSHDR_FLAG2_BUSTECH_ZLIB | AWSHDR_FLAG2_BUSTECH_HW)

public:

    EAWSChunkType  ChunkType();
    WORD           CurrChunkLen();
    WORD           PrevChunkLen();
    BYTE           Flag1()           { return m_Flag1; };
    BYTE           Flag2()           { return m_Flag2; };
    bool           IsValid()         { return Invalid           != ChunkType(); };
    bool           IsBegBlk()        { return BeginningOfBlock  == ChunkType(); };
    bool           IsMidBlk()        { return IntermediateBlock == ChunkType(); };
    bool           IsEndBlk()        { return EndOfBlock        == ChunkType(); };
    bool           IsNormalBlk()     { return NormalBlock       == ChunkType(); };
    bool           IsTapeMark()      { return TapeMark          == ChunkType(); };

    //             Is compressed? (any)
    bool           IsCompressed()    { return ((m_Flag1 & AWSHDR_FLAG1_COMPRESSED) ||
                                               (m_Flag2 & AWSHDR_FLAG2_COMPRESSED)); };

    //             Is "Hercules" compressed?
    bool           IsHCompressed()   { return  ( m_Flag1 & AWSHDR_FLAG1_COMPRESSED   )  ? true : false; };

    //             Is "ZLIB" compressed?
    bool           IsZCompressed()   { return (( m_Flag1 & AWSHDR_FLAG1_HERC_ZLIB    )
                                         ||    ( m_Flag2 & AWSHDR_FLAG2_BUSTECH_ZLIB )) ? true : false; };

    //             Is "BZIP2" compressed?
    bool           IsBZ2Compressed() { return  ( m_Flag1 & AWSHDR_FLAG1_HERC_BZIP2   )  ? true : false; };

    void           Init( void* pData )              { memcpy( this, pData, NUM_BYTES(*this) ); };
    CAWSChunkHdr()                                  { memset( this,   0,   NUM_BYTES(*this) ); };
    CAWSChunkHdr( void* pData )                     { memcpy( this, pData, NUM_BYTES(*this) ); };
    CAWSChunkHdr( CAWSChunkHdr& hdr )               { memcpy( this, &hdr,  NUM_BYTES(*this) ); };
    CAWSChunkHdr& operator = ( CAWSChunkHdr& hdr )  { memcpy( this, &hdr,  NUM_BYTES(*this) ); return *this; };

    void           Init( WORD wCurrChunkLen, WORD wPrevChunkLen, BYTE bFlag1, BYTE bFlag2 );
};

//////////////////////////////////////////////////////////////////////////////////////////
// Information pertaining to a chunk of data in an AWS/HET "tape" file...

class CAWSChunkInfo
{
protected:

    LARGE_INTEGER  m_n64FilePtr;    // (file ptr where this AWS chunk came from)
    CAWSChunkHdr   m_AWSChunkHdr;   // (see previous class above)
    int            m_nFileNum;      // (physical file number this chunk came from)

public:

    EAWSChunkType  ChunkType()       { return m_AWSChunkHdr.ChunkType();       };
    WORD           CurrChunkLen()    { return m_AWSChunkHdr.CurrChunkLen();    };
    WORD           PrevChunkLen()    { return m_AWSChunkHdr.PrevChunkLen();    };
    BYTE           Flag1()           { return m_AWSChunkHdr.Flag1();           };
    BYTE           Flag2()           { return m_AWSChunkHdr.Flag2();           };
    bool           IsValid()         { return m_AWSChunkHdr.IsValid();         };
    bool           IsBegBlk()        { return m_AWSChunkHdr.IsBegBlk();        };
    bool           IsMidBlk()        { return m_AWSChunkHdr.IsMidBlk();        };
    bool           IsEndBlk()        { return m_AWSChunkHdr.IsEndBlk();        };
    bool           IsNormalBlk()     { return m_AWSChunkHdr.IsNormalBlk();     };
    bool           IsTapeMark()      { return m_AWSChunkHdr.IsTapeMark();      };
    bool           IsCompressed()    { return m_AWSChunkHdr.IsCompressed();    };
    bool           IsHCompressed()   { return m_AWSChunkHdr.IsHCompressed();   };
    bool           IsZCompressed()   { return m_AWSChunkHdr.IsZCompressed();   };
    bool           IsBZ2Compressed() { return m_AWSChunkHdr.IsBZ2Compressed(); };
    CAWSChunkHdr   Header()          { return m_AWSChunkHdr;                   };
    LARGE_INTEGER  FilePtr()         { return m_n64FilePtr;                    };
    int            FileNum()         { return m_nFileNum;                      };

    CAWSChunkInfo() { m_n64FilePtr.QuadPart = m_nFileNum = 0; };
    CAWSChunkInfo( CAWSChunkInfo& info ) { *this = info; };
    CAWSChunkInfo( LARGE_INTEGER n64FilePtr, void* pData, int nFileNum );
    CAWSChunkInfo& operator = ( CAWSChunkInfo& info );
    void  Init( LARGE_INTEGER n64FilePtr, void* pData, int nFileNum );
};

//////////////////////////////////////////////////////////////////////////////////////////
// Just some boring statistics...

class CAWSFileStats
{
public:

    CString  m_strStdLabel1;        // First  Standard Label for this file
    CString  m_strStdLabel2;        // Second Standard Label for this file

    INT64    m_nTotalBytes;         // total #of bytes in this file,
                                    // including all overhead bytes
                                    // (includes trailing tapemark, if any)

    INT64    m_nTotalChunks;        // total #of chunks in this file

    INT64    m_nTotalChunkBytes;    // total #of chunk bytes  (compressed bytes)
    INT64    m_nTotalDataBytes;     // total #of data bytes   (uncompressed bytes)

    void Reinit()
    {
        m_strStdLabel1     = _T("");
        m_strStdLabel2     = _T("");

        m_nTotalBytes      = 0;
        m_nTotalChunks     = 0;
        m_nTotalChunkBytes = 0;
        m_nTotalDataBytes  = 0;
    };

    CAWSFileStats()  { Reinit(); };

    CAWSFileStats& operator = ( const CAWSFileStats& rStats )
    {
        m_strStdLabel1     = rStats.m_strStdLabel1;
        m_strStdLabel2     = rStats.m_strStdLabel2;

        m_nTotalBytes      = rStats.m_nTotalBytes;
        m_nTotalChunks     = rStats.m_nTotalChunks;
        m_nTotalChunkBytes = rStats.m_nTotalChunkBytes;
        m_nTotalDataBytes  = rStats.m_nTotalDataBytes;
        return *this;
    };

    CAWSFileStats& operator += ( const CAWSFileStats& rStats )
    {
        m_strStdLabel1      =   _T( "" );
        m_strStdLabel2      =   _T( "" );

        m_nTotalBytes       +=  rStats.m_nTotalBytes;
        m_nTotalChunks      +=  rStats.m_nTotalChunks;
        m_nTotalChunkBytes  +=  rStats.m_nTotalChunkBytes;
        m_nTotalDataBytes   +=  rStats.m_nTotalDataBytes;
        return *this;
    };
};

//////////////////////////////////////////////////////////////////////////////////////////
