// Copyright (c) 2004-2008, Software Development Laboratories, "Fish" (David B. Trout)
///////////////////////////////////////////////////////////////////////////////////////
//
//      Version.h  --  defines the build version '#define' strings for the product
//
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////
#include "AutoBuildCount.h"     //  (where BUILDCOUNT_STR and BUILDCOUNT_NUM are defined)
//////////////////////////////////


#define VERMAJOR_NUM  1         // MAJOR Release (program radically changed)
#define VERMAJOR_STR "1"

#define VERINTER_NUM  5         // Minor Enhancements (new/modified feature, etc)
#define VERINTER_STR "5"

#define VERMINOR_NUM  2         // Bug Fix
#define VERMINOR_STR "2"

///////////////////////////////////////////////////////////////////////////////////////
// Construct VERSION string consisting of above MAJOR/INTER/MINOR strings + BUILDCOUNT
///////////////////////////////////////////////////////////////////////////////////////

#ifdef RC2
#undef _T
#define _T(x) x
#endif

#ifdef _UNICODE
  #ifdef _DEBUG
    #define VERSION_STR  _T( VERMAJOR_STR ) _T( "." ) _T( VERINTER_STR ) _T( "." ) _T( VERMINOR_STR ) _T( "." ) _T( BUILDCOUNT_STR ) _T( "-UD" )
  #else
    #define VERSION_STR  _T( VERMAJOR_STR ) _T( "." ) _T( VERINTER_STR ) _T( "." ) _T( VERMINOR_STR ) _T( "." ) _T( BUILDCOUNT_STR ) _T( "-U" )
  #endif
#else
  #ifdef _DEBUG
    #define VERSION_STR  _T( VERMAJOR_STR ) _T( "." ) _T( VERINTER_STR ) _T( "." ) _T( VERMINOR_STR ) _T( "." ) _T( BUILDCOUNT_STR ) _T( "-D" )
  #else
    #define VERSION_STR  _T( VERMAJOR_STR ) _T( "." ) _T( VERINTER_STR ) _T( "." ) _T( VERMINOR_STR ) _T( "." ) _T( BUILDCOUNT_STR )
  #endif
#endif

///////////////////////////////////////////////////////////////////////////////////////
