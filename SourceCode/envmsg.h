#pragma once

void ReportProc( LPCTSTR msg, LPCTSTR hdr, int type );
void StatusMsgProc( LPCTSTR msg );
int ReportBoxProc(  LPCTSTR hdr, LPCTSTR msg, int flags );


enum ENV_REPORTING_LEVEL
   {
   ERL_ERRORS   = 1,
   ERL_WARNINGS = 2,
   ERL_INFOS    = 4,
   ERL_ALL      = 7
   };


//int EnvWarningMsg( LPCSTR msg, int flags=MB_OK );
//int EnvErrorMsg( LPCSTR msg, int flags=MB_OK );
//int EnvInfoMsg( LPCSTR msg, int flags=MB_OK );
//void EnvMsg( LPCSTR msg );
