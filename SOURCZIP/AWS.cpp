// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
//////////////////////////////////////////////////////////////////////////////////////////
// AWS.cpp : implementation file for the the AWS classes
//////////////////////////////////////////////////////////////////////////////////////////
//
//  Change History:
//
//  03/12/06    1.3.0   Fish    Support for larger block sizes
//  03/20/06    1.3.0   Fish    Add support for Bus-Tech ZLIB compression
//  06/15/06    1.5.0   Fish    VS2005, x64
//  11/21/06    1.5.0   Fish    Add CAWSChunkHdr::Init( WORD,WORD,BYTE,BYTE ) overload.
//  11/24/06    1.5.0   Fish    Add support for Bus-Tech segmented-block flag.
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AWS.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (embed copyright into object code)
#pragma comment ( user, COPYRIGHT_EXESTR )

//////////////////////////////////////////////////////////////////////////////////////////

WORD  CAWSChunkHdr::CurrChunkLen()
{
    // Normalize little-endian chunk size to host format
    // (works on both little-endian and big-endian systems)

    return  ((((WORD)m_CurrChunkLen[0]) << 0) & 0x00FF)
            |
            ((((WORD)m_CurrChunkLen[1]) << 8) & 0xFF00)
            ;
}

//////////////////////////////////////////////////////////////////////////////////////////

WORD  CAWSChunkHdr::PrevChunkLen()
{
    // Normalize little-endian chunk size to host format
    // (works on both little-endian and big-endian systems)

    return  ((((WORD)m_PrevChunkLen[0]) << 0) & 0x00FF)
            |
            ((((WORD)m_PrevChunkLen[1]) << 8) & 0xFF00)
            ;
}

//////////////////////////////////////////////////////////////////////////////////////////

EAWSChunkType CAWSChunkHdr::ChunkType()
{
    // (must not have any resvered bits set)

    if (0
        || (m_Flag1 & AWSHDR_FLAG1_RESERVED)
        || (m_Flag2 & AWSHDR_FLAG2_RESERVED)
    )
        return Invalid;

    // (can't be both Herc ZLIB compressed *and* Herc BZIP2 compressed)

    if (1
        && (m_Flag1 & AWSHDR_FLAG1_HERC_ZLIB)
        && (m_Flag1 & AWSHDR_FLAG1_HERC_BZIP2)
    )
        return Invalid;

    // (can't be both Bus-Tech ZLIB compressed *and* Bus-Tech hardware compressed))

    if (1
        && (m_Flag2 & AWSHDR_FLAG2_BUSTECH_ZLIB)
        && (m_Flag2 & AWSHDR_FLAG2_BUSTECH_HW)
    )
        return Invalid;

    // (can't be both Herc compressed *and* Bus-Tech compressed)

    if (1
        && (m_Flag1 & AWSHDR_FLAG1_COMPRESSED)
        && (m_Flag2 & AWSHDR_FLAG2_COMPRESSED)
    )
        return Invalid;

    // (tapemark flag must be specified alone)

    if (m_Flag1 & AWSHDR_FLAG1_TAPEMARK)
    {
        if (0
            || (m_Flag1 & ~AWSHDR_FLAG1_TAPEMARK)
            ||  m_Flag2
        )
            return Invalid;
    }

    // Determine chunk type...

    switch ( m_Flag1 & ~AWSHDR_FLAG1_COMPRESSED )
    {
        // Normal AWS...

        case 0
             :
            return CurrChunkLen() ? IntermediateBlock : Invalid;

        case 0
             | AWSHDR_FLAG1_BEGBLK
             :
            return CurrChunkLen() ? BeginningOfBlock  : Invalid;

        case 0
             | AWSHDR_FLAG1_ENDBLK
             :
            return CurrChunkLen() ? EndOfBlock        : Invalid;

        case 0
             | AWSHDR_FLAG1_BEGBLK
             | AWSHDR_FLAG1_ENDBLK
             :
            return CurrChunkLen() ? NormalBlock       : Invalid;

        case 0
             | AWSHDR_FLAG1_TAPEMARK
             :
            return CurrChunkLen() ? Invalid           : TapeMark;

        // Bus-Tech extensions...

        case 0
             | AWSHDR_FLAG1_BUSTECH_MIDBLK
             :
            return CurrChunkLen() ? IntermediateBlock : Invalid;

        case 0
             | AWSHDR_FLAG1_BEGBLK
             | AWSHDR_FLAG1_BUSTECH_MIDBLK
             :
            return CurrChunkLen() ? BeginningOfBlock  : Invalid;

        case 0
             | AWSHDR_FLAG1_BUSTECH_MIDBLK
             | AWSHDR_FLAG1_ENDBLK
             :
            return CurrChunkLen() ? EndOfBlock        : Invalid;

        case 0
             | AWSHDR_FLAG1_BEGBLK
             | AWSHDR_FLAG1_BUSTECH_MIDBLK
             | AWSHDR_FLAG1_ENDBLK
             :
            return CurrChunkLen() ? NormalBlock       : Invalid;
    }

    return Invalid;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CAWSChunkHdr::Init( WORD wCurrChunkLen, WORD wPrevChunkLen, BYTE bFlag1, BYTE bFlag2 )
{
    m_CurrChunkLen[0] = LOBYTE( wCurrChunkLen );
    m_CurrChunkLen[1] = HIBYTE( wCurrChunkLen );

    m_PrevChunkLen[0] = LOBYTE( wPrevChunkLen );
    m_PrevChunkLen[1] = HIBYTE( wPrevChunkLen );

    m_Flag1 = bFlag1;
    m_Flag2 = bFlag2;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

CAWSChunkInfo::CAWSChunkInfo( LARGE_INTEGER n64FilePtr, void* pData, int nFileNum )
{
    m_n64FilePtr = n64FilePtr; m_AWSChunkHdr.Init( pData ); m_nFileNum = nFileNum;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CAWSChunkInfo::Init( LARGE_INTEGER n64FilePtr, void* pData, int nFileNum )
{
    m_n64FilePtr = n64FilePtr; m_AWSChunkHdr.Init( pData ); m_nFileNum = nFileNum;
}

//////////////////////////////////////////////////////////////////////////////////////////

CAWSChunkInfo& CAWSChunkInfo::operator = ( CAWSChunkInfo& info )
{
    m_n64FilePtr  = info.m_n64FilePtr;
    m_AWSChunkHdr = info.m_AWSChunkHdr;
    m_nFileNum    = info.m_nFileNum;
    return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
