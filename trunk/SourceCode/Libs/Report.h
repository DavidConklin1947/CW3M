/*
This file is part of Envision.

Envision is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Envision is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Envision.  If not, see <http://www.gnu.org/licenses/>

Copywrite 2012 - Oregon State University

*/
#pragma once

#include "libs.h"

// class-based version

 // report flags
const int RF_MESSAGEBOX = 1;    // msgs displayed in a MessageBox
const int RF_CALLBACK   = 2;    // msgs reported to the callback funtion (if defined)
const int RF_STATUSMSG  = 3;   

// reporting types
const int RT_INFO       = 1;    // general info msgs
const int RT_WARNING    = 2;    // warning msgs
const int RT_ERROR      = 3;    // error msgs
const int RT_FATAL      = 4;    // fatal error msgs
const int RT_SYSTEM     = 5;    // system error msgs


typedef void (*REPORTMSG_PROC)( LPCTSTR msg, LPCTSTR hdr, int type );
typedef void (*STATUSMSG_PROC)( LPCTSTR msg );
typedef int  (*MSGWND_PROC   )( LPCTSTR header, LPCTSTR msg, int flags );


class LIBSAPI Report
{
public:
   static int            reportFlag; 
   static REPORTMSG_PROC reportMsgProc;
   static STATUSMSG_PROC statusMsgProc;
   static MSGWND_PROC    msgWndProc;
   static FILE           *m_pFile;

   static int  ErrorMsg  ( LPCTSTR msg, LPCTSTR hdr="Error",   int flags=MB_OK ) { return Msg( msg, RT_ERROR, hdr, flags ); }
   static int  ErrorMsgEx( LPCTSTR msg, LPCTSTR hdr="Error",   int flags=MB_OK ) { return Msg( msg, RT_ERROR, hdr, flags ); }
   static int  WarningMsg( LPCTSTR msg, LPCTSTR hdr="Warning", int flags=MB_OK ) { return Msg( msg, RT_WARNING, hdr, flags ); }
   static int  InfoMsg   ( LPCTSTR msg, LPCTSTR hdr="Info",    int flags=MB_OK ) { return Msg( msg, RT_INFO, hdr, flags ); }
   static int  FatalMsg  ( LPCTSTR msg, LPCTSTR hdr="Fatal Error", int flags=MB_OK ) { return Msg( msg, RT_FATAL, hdr, flags ); }
   static int  SystemMsg ( LPCTSTR msg, LPCTSTR hdr="System", int flags=MB_OK )  { return Msg( msg, RT_SYSTEM, hdr, flags ); }
   static int  OKMsg     ( LPCTSTR msg, LPCTSTR hdr="\0" );
   static int  YesNoMsg  ( LPCTSTR msg, LPCTSTR hdr="\0" );
   static int  LogMsg    ( LPCTSTR msg, int type=RT_INFO );
   static int  LogWarning( LPCTSTR msg ) { return LogMsg( msg, RT_WARNING ); }
   static int  LogError  ( LPCTSTR msg ) { return LogMsg( msg, RT_ERROR ); }
   static int  BalloonMsg( LPCTSTR msg, int type=RT_INFO, int duration=5000 );
   static void StatusMsg ( LPCTSTR msg );
   static int  SystemErrorMsg( LONG s_err, LPCTSTR __msg= "\0",  LPCTSTR hdr="\0", int flags=MB_OK );

   static int OpenFile( LPCTSTR filename );
   static int CloseFile( void );
   static int WriteFile( LPCTSTR );

protected:
   static int Msg( LPCTSTR msg, int type, LPCTSTR hdr="\0", int flags=MB_OK );      
};

