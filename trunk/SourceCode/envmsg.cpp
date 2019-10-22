#include "stdafx.h"
#pragma hdrstop

#include "envmsg.h"
#include "envview.h"
#include "envdoc.h"
#include "mainfrm.h"
#include <colors.hpp>
#include <Report.h>
#include <EnvEngine\EnvModel.h>
#include <ReportDlg.h>

extern CEnvView   *gpView;
extern CEnvDoc    *gpDoc;
extern CMainFrame *gpMain;
extern EnvModel   *gpModel;

// called whenever a Report message is invoked (e.g. Report::LogMsg();
void ReportProc( LPCTSTR msg, LPCTSTR hdr, int type )
   {
   COLORREF color;

   switch( type )
      {
      case RT_WARNING: 
         color = BLUE;
         gpDoc->m_warnings++;

         if ( gpModel->m_logMsgLevel > 0 && ( ! (gpModel->m_logMsgLevel & ERL_WARNINGS ) ) )
            return;
         break;

      case RT_ERROR:
      case RT_FATAL:
      case RT_SYSTEM:
         color = RED;
         gpDoc->m_errors++;
         if ( gpModel->m_logMsgLevel > 0 && ( ! (gpModel->m_logMsgLevel & ERL_ERRORS ) ) )
            return;
         break;

      default:
         color = BLACK;
         gpDoc->m_infos++;

         if ( gpModel->m_logMsgLevel > 0 && ( ! (gpModel->m_logMsgLevel & ERL_INFOS ) ) )
            return;

         break;
      }

   gpMain->AddLogLine( msg, (LOGBOOK_TEXTTYPES)type, color );
   }


void StatusMsgProc( LPCTSTR msg )
   {
   gpMain->SetStatusMsg( msg );
   }


// called whenever a Report message is invoked (e.g. Report::LogMsg();
int ReportBoxProc(  LPCTSTR hdr, LPCTSTR msg, int flags )
   {
   ReportBox dlg;
   dlg.m_msg = msg;
   dlg.m_hdr = hdr;
   dlg.m_noShow = 0;
   dlg.m_output = ( Report::reportFlag & RF_CALLBACK ) ? 1 : 0;
   dlg.m_flags = flags;
   int retVal = (int) dlg.DoModal();

   Report::reportFlag = 0;
   if ( ! dlg.m_noShow )
      Report::reportFlag += RF_MESSAGEBOX;

   if ( dlg.m_output )
      Report::reportFlag += RF_CALLBACK;
   
   retVal = AfxGetMainWnd()->MessageBox( msg, hdr, flags );
   
   return retVal;
   }


