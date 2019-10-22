// EnvEngine.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "EnvEngine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <Maplayer.h>
#include <QueryEngine.h>

#include "EnvModel.h"
#include "Actor.h"
#include "Policy.h"
#include "Scenario.h"
#include "EnvLoader.h"
#include "EnvMisc.h"

EnvModel        *gpModel;
PolicyManager   *gpPolicyManager;
ActorManager    *gpActorManager;
ScenarioManager *gpScenarioManager;
QueryEngine     *gpQueryEngine;

#ifndef NO_MFC
//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CEnvEngineApp

BEGIN_MESSAGE_MAP(CEnvEngineApp, CWinApp)
END_MESSAGE_MAP()


// CEnvEngineApp construction

CEnvEngineApp::CEnvEngineApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CEnvEngineApp object

CEnvEngineApp theApp;


// CEnvEngineApp initialization

BOOL CEnvEngineApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

extern "C" int PASCAL InitEngine( LPCTSTR envxFile, Map *pMap, int initFlags )
   {
   gpModel           = new EnvModel;
   gpPolicyManager   = new PolicyManager;
   gpActorManager    = new ActorManager;
   gpScenarioManager = new ScenarioManager;
   
   EnvLoader loader;
   int result = loader.LoadProject( envxFile, pMap, gpModel, gpPolicyManager, gpActorManager, gpScenarioManager );

   if ( result < 0  )
     return result;

   // set up bins for loaded layer(s)
   for ( int i=0; i < pMap->GetLayerCount(); i++ )
      {
      if ( pMap->GetLayer( i )->m_pData != NULL )
         Map::SetupBins( pMap, i, -1 );
      }

   Report::StatusMsg( _T("") );

   // add a field for each actor value; cf CEnvDoc::CEnvDoc()
   for ( int i=0; i < EnvModel::GetActorValueCount(); i++ )
      {
      METAGOAL *pInfo =  EnvModel::GetActorValueMetagoalInfo( i );
      CString label( pInfo->name );
      label += " Actor Value";

      CString field;
      field.Format( "ACTOR%s", (PCTSTR) pInfo->name );
      if (field.GetLength() > 10)
         field.Truncate(10);
      field.Replace( ' ', '_' );

      EnvModel::m_pIDULayer->AddFieldInfo( field, 0, label, _T(""), _T(""), TYPE_FLOAT, MFIT_QUANTITYBINS, BCF_GREEN_INCR , 0, true );
      }
    
   int defaultScenario = gpScenarioManager->GetDefaultScenario();
   gpModel->SetScenario( gpScenarioManager->GetScenario( defaultScenario ) );  /* -- */

   //LoadExtensions();

   // reset EnvModel
   gpModel->Reset();

   int levels = EnvModel::m_lulcTree.GetLevels();

   switch( levels )
      {
      case 1:  EnvModel::m_pIDULayer->CopyColData( EnvModel::m_colStartLulc, EnvModel::m_colLulcA );   break;
      case 2:  EnvModel::m_pIDULayer->CopyColData( EnvModel::m_colStartLulc, EnvModel::m_colLulcB );   break;
      case 3:  EnvModel::m_pIDULayer->CopyColData( EnvModel::m_colStartLulc, EnvModel::m_colLulcC );   break;
      case 4:  EnvModel::m_pIDULayer->CopyColData( EnvModel::m_colStartLulc, EnvModel::m_colLulcD );   break;
      }

   Report::reportFlag = RF_CALLBACK | RF_MESSAGEBOX;      // disable onscreen messageboxes while loading

   // any errors during load?
   CString msg;
   //CString error;
   //CString warning;
   //if ( m_errors > 0 )
   //   {
   //   if (m_errors == 1 )
   //      error = "1 Error ";
   //   else
   //      error.Format( "%i Errors ", m_errors );
   //   }
   //
   //if ( m_warnings > 0 )
   //   {
   //   if (m_warnings == 1 )
   //      warning = "1 Warning ";
   //   else
   //      warning.Format( "%i Warnings ", m_warnings );
   //   }
   //     
   //if ( m_errors > 0 && m_warnings > 0 )
   //   msg.Format( "%s and %s \nwere encountered during startup.", (LPCTSTR) error, (LPCTSTR) warning );
   //else if ( m_errors > 0 )
   //   {
   //   if ( m_errors == 1 )
   //      msg = "1 Error was \nencountered during startup.";
   //   else
   //      msg.Format( "%i Errors were \nencountered during startup.", m_errors );
   //   }
   //else if ( m_warnings > 0 )
   //   {
   //   if ( m_warnings == 1 )
   //      msg = "1 Warning was encountered during startup";
   //   else
   //      msg.Format( "%i Warnings were \nencountered during startup.", m_warnings );
   //   }
   //
   //if ( ! msg.IsEmpty() )
   //   {
   //   msg += " Check the Output Panel for details...";
   //
   //   Report::BalloonMsg( msg, RT_WARNING );
   //   gpMain->SetStatusMsg( msg );
   //   }

   // if there are any relevant command line cmds, process them
   //if ( this->m_cmdRunScenario >= 0 )
   //   {
   //   // load the specified scenario
   //   if ( this->m_cmdRunScenario == 0 ) // run all scenarios
   //      {
   //      int count = (int) gpMain->m_pScenariosCombo->GetCount();
   //      gpMain->m_pScenariosCombo->SelectItem( count-1 );  // run all is last
   //      }
   //   else
   //      gpMain->m_pScenariosCombo->SelectItem( this->m_cmdRunScenario-1 );  // m_cmdRunScenario is 1-based
   //
   //   OnAnalysisRun(); 
   //
   //   return ( m_exitCode = -10 );
   //   }

   return 1;
   }

#endif
