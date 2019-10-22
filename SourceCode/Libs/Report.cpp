// Report.cpp : implementation file
//

#include "libs.h"
#pragma hdrstop

#include "Report.h"

int Report::reportFlag = RF_MESSAGEBOX | RF_CALLBACK;

REPORTMSG_PROC Report::reportMsgProc = NULL;
STATUSMSG_PROC Report::statusMsgProc = NULL;
MSGWND_PROC Report::msgWndProc = NULL;

FILE *Report::m_pFile = NULL;


int Report::Msg( LPCTSTR msg, int errorType, LPCTSTR hdr, int flags )
   {
   int retVal = 0;

   if ( reportFlag & RF_MESSAGEBOX && msgWndProc != NULL )
      {
      if ( hdr == NULL )
         hdr = "Report";

      int retVal = msgWndProc( hdr, msg, flags );
      }

//#ifdef _WINDOWS
//      ReportBox dlg;
//      dlg.m_msg = msg;
//      dlg.m_hdr = hdr;
//      dlg.m_noShow = 0;
//      dlg.m_output = ( reportFlag & RF_CALLBACK ) ? 1 : 0;
//      dlg.m_flags = flags;
//      retVal = (int) dlg.DoModal();
//
//      reportFlag = 0;
//      if ( ! dlg.m_noShow )
//         reportFlag += RF_MESSAGEBOX;
//
//      if ( dlg.m_output )
//         reportFlag += RF_CALLBACK;
//#endif
    // retVal = AfxGetMainWnd()->MessageBox( msg, hdr, flags );
      
   CString _msg = msg;

   if ( reportFlag & RF_CALLBACK && reportMsgProc != NULL )
      {
      if ( hdr == NULL )
         hdr = "Message";

      CString title( hdr );
      title += ": ";

      title.Replace( "%", "percent" );  // special formating character
      _msg.Replace( "%", "percent" );
      reportMsgProc( _msg, title, errorType );
      }

   if ( m_pFile != NULL )
      WriteFile( _msg );

   return retVal;
   }



int Report::OKMsg( LPCTSTR msg, LPCTSTR hdr )
   {
#ifdef _WINDOWS   
   CString title = ( hdr == NULL ) ? "Success" : hdr;
   MessageBox( GetActiveWindow(), msg, title, MB_OK | MB_ICONEXCLAMATION );
#endif

   return 0;
   }
   

 int Report::YesNoMsg( LPCTSTR msg,  LPCTSTR hdr)
   {
   int retVal = 0;
#ifdef _WINDOWS   
   CString title = ( hdr == NULL ) ? "Response Requested" : hdr;
   retVal = MessageBox( GetActiveWindow(), msg, title, MB_YESNO | MB_ICONQUESTION );
#endif

   return retVal;
    }


int Report::LogMsg( LPCTSTR msg, int type )
   {
   int oldReportFlag = reportFlag;
   reportFlag = RF_CALLBACK;
   int retVal = 0;
   switch ( type )
      {
      case RT_INFO:    retVal = Report::InfoMsg( msg );     break;
      case RT_WARNING: retVal = Report::WarningMsg( msg );  break;
      case RT_ERROR:   retVal = Report::ErrorMsg( msg );    break;
      case RT_FATAL:   retVal = Report::FatalMsg( msg );    break;
      case RT_SYSTEM:  retVal = Report::SystemMsg( msg );   break;
      }
   
   reportFlag = oldReportFlag;
   return retVal;
   }


int Report::BalloonMsg( LPCTSTR msg, int type, int duration /*=2000*/ )
   {
   int retVal = 0;

#ifdef _WINDOWS
   CString hdr;

   switch ( type )
      {
      case RT_INFO:    hdr = _T("Information Message");  break;
      case RT_WARNING: hdr = _T("Warning! ");     break;
      case RT_ERROR:   hdr = _T("Error! ");       break;
      case RT_FATAL:   hdr = _T("Fatal Error! "); break;
      case RT_SYSTEM:  hdr = _T("System Error! ");break;
      }

   CWnd *pMain = ::AfxGetMainWnd();
   CRect rect;
   pMain->GetClientRect( &rect );
      
   //CBalloonHelp::LaunchBalloon( hdr, msg, CPoint( 400, rect.bottom-40 ), IDI_WARNING, 
   //            CBalloonHelp::unCLOSE_ON_ANYTHING | CBalloonHelp::unSHOW_CLOSE_BUTTON /* | CBalloonHelp::unDELAY_CLOSE | CBalloonHelp::unSHOW_TOPMOST */,
   //            pMain, "", duration );

   CMFCDesktopAlertWnd *pPopup = new CMFCDesktopAlertWnd;
   pPopup->SetAutoCloseTime( duration );
   pPopup->SetAnimationType( CMFCPopupMenu::SLIDE );
   pPopup->SetSmallCaption( 0 );
   //pPopup->SetTransparency( 100 );
      
   CMFCDesktopAlertWndInfo params;

   switch( type )
      {
      case RT_INFO:    params.m_hIcon = LoadIcon( NULL, IDI_INFORMATION );        break;
      case RT_WARNING: params.m_hIcon = LoadIcon( NULL, IDI_WARNING );            break;
      case RT_ERROR:   
      case RT_FATAL:   
      case RT_SYSTEM:  params.m_hIcon = LoadIcon( NULL, IDI_ERROR );              break;
      default: params.m_hIcon = NULL;
      }

   params.m_strText = msg;
   params.m_strURL = _T( "" );
   //params.m_nURLCmdID = 101;   
   CPoint pos;    // screen coords
   pos.x = 400;
   pos.y = rect.bottom-40;
   pPopup->Create( pMain, params, NULL, pos );
   pPopup->SetWindowText( hdr );     

#endif
   return retVal;
   }


void Report::StatusMsg( LPCTSTR msg )
   {
   if ( statusMsgProc != NULL ) statusMsgProc( msg );
   LogMsg(msg);
   return;
   }


int Report::SystemErrorMsg  ( LONG s_err, LPCTSTR __msg, LPCTSTR hdr, int flags)
   {
   int retVal = 0;

#ifdef _WINDOWS
   CString title = ( hdr == NULL ) ? "Error" : hdr;

   LPVOID lpMsgBuf;
   FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, s_err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf,  0,  NULL  );   

   CString msg = ( __msg == NULL ) ? "" : __msg ;
   msg += (LPTSTR) lpMsgBuf;
   LocalFree( lpMsgBuf );

   retVal = MessageBox( GetActiveWindow(), msg, title, flags | MB_ICONEXCLAMATION );
#endif

   return retVal;
   }


int Report::OpenFile( LPCTSTR filename )
   {
   if ( m_pFile )
      CloseFile();

   int retVal = fopen_s( &m_pFile, filename, "wt");
   if ( retVal != 0 )
      {
      CString msg( "Report: Unable to open file " );
      msg += filename;
      Report::ErrorMsg( msg );
      }
      
   return retVal;
   }


int Report::CloseFile( void )
   {
   if ( m_pFile != NULL )
      {
      fclose( m_pFile );
      m_pFile = NULL;
      }

   return 0;
   }


int Report::WriteFile( LPCTSTR line )
   {
   if ( m_pFile == NULL )
      return -1;

   fputs( line, m_pFile );    // no newline, must add
   fputc( '\n', m_pFile );

   return 0;
   }
 
