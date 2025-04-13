/***********************************************************************************/
/*  GetParms.nsh            GetParameterValue                                      */
/***********************************************************************************/
/*                                                                                 */
/*  Ref:  http://nsis.sourceforge.net/Get_command_line_parameter_by_name           */
/*                                                                                 */
/*  A function that uses the GetParameters function to retrieve the value          */
/*  of a command line argument. I wanted an easy way to retrieve parameters        */
/*  and modified the example given in the NSIS documentation.                      */
/*                                                                                 */
/*  (C) Copyright 2004-2005, Chris Morgan  <cmorgan@alum.wpi.edu>                  */
/*                                                                                 */
/*  Usage:                                                                         */
/*                                                                                 */
/*       !include "GetParms.nsh"    ; Need "GetParameterValue" function            */
/*                                                                                 */
/*       Push "OUTPUT"              ; push the search string onto the stack        */
/*       Push "DefaultValue"        ; push a default value onto the stack          */
/*       Call GetParameterValue                                                    */
/*       Pop $2                                                                    */
/*       MessageBox  MB_OK  "The value of 'OUTPUT' parameter is '$2'"              */
/*                                                                                 */
/*  This retrieves VALUE from a command line of:    /OUTPUT=VALUE                  */
/*                                           or:   "/OUTPUT=VALUE WITH SPACES"     */
/*                                                                                 */
/*  Fish note: I didn't write this. Someone else did. I got it from the NSIS       */
/*  web site and created this header based on it, adding the above comments.       */
/*                                                                                 */
/***********************************************************************************/

!ifndef  _GetParms_NSH_
!define  _GetParms_NSH_

!verbose push
!verbose 3

!include "FileFunc.nsh"                 ; Need "GetParameters" macro...
!insertmacro  GetParameters             ; Need "GetParameters" function...

;------------------------------------------------------------------------------------
;                                                                                   ;
;                           GetParameterValue                                       ;
;                                                                                   ;
;  Chris Morgan  <cmorgan@alum.wpi.edu>  5/10/2004                                  ;
;                                                                                   ;
;   - Updated 4/7/2005 to add support for retrieving a command line switch          ;
;     and additional documentation                                                  ;
;                                                                                   ;
;  Searches the command line input, retrieved using GetParameters, for the          ;
;  value of an option given the option name.  If no option is found the             ;
;  default value is placed on the top of the stack upon function return.            ;
;                                                                                   ;
;  This function can also be used to detect the existence of just a                 ;
;  command line switch like /OUTPUT  Pass the default and "OUTPUT"                  ;
;  on the stack like normal.  An empty return string "" will indicate               ;
;  that the switch was found, the default value indicates that                      ;
;  neither a parameter or switch was found.                                         ;
;                                                                                   ;
;  Inputs - Top of stack is default if parameter isn't found,                       ;
;   second in stack is parameter to search for, ex. "OUTPUT"                        ;
;                                                                                   ;
;  Outputs - Top of the stack contains the value of this parameter                  ;
;   So if the command line contained /OUTPUT=somedirectory, "somedirectory"         ;
;   will be on the top of the stack when this function returns                      ;
;                                                                                   ;
;  Register usage:                                                                  ;
;                                                                                   ;
;   $R0 - default return value if the parameter isn't found                         ;
;   $R1 - input parameter, for example OUTPUT from the above example                ;
;   $R2 - the length of the search, this is the search parameter+2                  ;
;         as we have '/OUTPUT='                                                     ;
;   $R3 - the command line string                                                   ;
;   $R4 - result from StrStr calls                                                  ;
;   $R5 - search for ' ' or '"'                                                     ;
;                                                                                   ;
;------------------------------------------------------------------------------------

Function GetParameterValue

  Exch $R0              ; get the top of the stack (default parameter) into $R0
  Exch                  ; exchange the top of the stack(default) with
                        ; the second in the stack (parameter to search for)
  Exch $R1              ; get the top of the stack (search parameter) into $R1

  /*
  **  Preserve on the stack the registers used in this function
  */

  Push $R2
  Push $R3
  Push $R4
  Push $R5

  Strlen $R2 $R1+2      ; save the length of the search string in $R2

  Call GetParameters    ; get the command line parameters
  Pop $R3               ; save command line string in $R3

  /*
  **  Search for quoted search string
  */

  StrCpy $R5 '"'        ; later on we want to search for a open quote
  Push $R3              ; push the 'search in' string onto the stack
  Push '"/$R1='         ; push the 'search for'
  Call StrStr           ; search for the quoted parameter value
  Pop $R4
  StrCpy $R4 $R4 "" 1   ; skip over open quote character, "" means no maxlen
  StrCmp $R4 "" "" next ; if we didn't find an empty string go to next

  /*
  **  Search for non-quoted search string
  */

  StrCpy $R5 ' '        ; later on we want to search for a space since we
                        ; didn't start with an open quote '"' we shouldn't
                        ; look for a close quote '"'
  Push $R3              ; push the command line back on the stack for searching
  Push '/$R1='          ; search for the non-quoted search string
  Call StrStr
  Pop $R4

  /*
  **  $R4 now contains the parameter string starting at the search string,
  **  if it was found
  */

next:
  StrCmp $R4 "" check_for_switch    ; if we didn't find anything then look for
                                    ; usage as a command line switch
  /*
  **  Copy the value after /$R1= by using StrCpy with an offset of $R2,
  **  the length of '/OUTPUT='
  */
  StrCpy $R0 $R4 "" $R2  ; copy commandline text beyond parameter into $R0

  ; search for the next parameter so we can trim this extra text off
  Push $R0
  Push $R5              ; search for either the first space ' ',
                        ; or the first quote '"'
                        ; if we found '"/output' then we want to find
                        ; the ending, as in '"/output=somevalue"'
                        ; if we found '/output' then we want to find
                        ; the first space after '/output=somevalue'
  Call StrStr           ; search for the next parameter
  Pop $R4
  StrCmp $R4 "" done    ; if 'somevalue' is missing, we are done
  StrLen $R4 $R4        ; get the length of 'somevalue' so we can copy this
                        ; text into our output buffer
  StrCpy $R0 $R0 -$R4   ; using the length of the string beyond the value,
                        ; copy only the value into $R0
  goto done             ; if we are in the parameter retrieval path skip over
                        ; the check for a command line switch
  /*
  **  See if the parameter was specified as a command line switch,
  **  like '/output'
  */

check_for_switch:
  Push $R3              ; push the command line back on the stack for searching
  Push '/$R1'           ; search for the non-quoted search string
  Call StrStr
  Pop $R4
  StrCmp $R4 "" done    ; if we didn't find anything then use the default
  StrCpy $R0 ""         ; otherwise copy in an empty string since we found
                        ; the parameter, just didn't find a value

done:
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0              ; put the value in $R0 at the top of the stack

FunctionEnd

;------------------------------------------------------------------------------------

!verbose pop

!endif  #  _GetParms_NSH_
