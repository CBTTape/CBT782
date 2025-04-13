/*-------------------------------------------------------------------------*/
/*                                                                         */
/*      Debug64.nsh    --    defines build configuration debug flags       */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*          !define  DEBUG,  WIN64,  Platform,  Configuration              */
/*-------------------------------------------------------------------------*/

!ifndef  _Debug64_NSH_
!define  _Debug64_NSH_



!ifdef DEBUG
  !undef DEBUG
!endif


!ifdef WIN64
  !undef WIN64
!endif


/*   The "_DEBUG" setting is passed on the command-line via "/D_DEBUG=1"   */


!ifdef _DEBUG
  !if "${_DEBUG}" != "0"
    !define DEBUG
  !endif
!endif


/*   The "_WIN64" setting is passed on the command-line via "/D_WIN64=1"   */


!ifdef _WIN64
  !if "${_WIN64}" != "0"
    !define WIN64
  !endif
!endif


!ifdef WIN64
  !define  Platform       "x64"
!else
  !define  Platform       "x86"
!endif


!ifdef DEBUG
  !define  Configuration  "Debug"
!else
  !define  Configuration  "Release"
!endif


!endif  #  _Debug64_NSH_
