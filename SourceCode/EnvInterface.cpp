#include "StdAfx.h"
#include "EnvInterface.h"
#include "EnvEngine\EnvModel.h"
#include <Maplayer.h>
#include "EnvEngine\Policy.h"
#include "EnvEngine\Scenario.h"
#include "EnvEngine\Actor.h"
#include "MapPanel.h"
#include "EnvView.h"
//#include "PathManager.h"

#include "PolQueryDlg.h"
#include <VideoRecorder.h>

extern MapPanel      *gpMapPanel; 
extern EnvModel      *gpModel;
extern MapLayer      *gpCellLayer;
extern PolicyManager *gpPolicyManager;
extern ActorManager  *gpActorManager;
extern ScenarioManager *gpScenarioManager;
extern CEnvView      *gpView;

DELTA & GetDelta( DeltaArray *da, INT_PTR index )  { return da->GetAt( index ); }

int ApplyDeltaArray( void ) { return gpModel->ApplyDeltaArray( gpCellLayer ); }

int GetModelCount() { return EnvModel::GetModelCount(); }
ENV_EVAL_MODEL* GetModelInfo( int i ) { return EnvModel::GetModelInfo( i ); }
ENV_EVAL_MODEL* FindModelInfo( LPCTSTR name ) { return EnvModel::FindModelInfo( name ); }

int GetAutoProcessCount() { return EnvModel::GetAutonomousProcessCount(); }
ENV_AUTO_PROCESS* GetAutoProcessInfo( int i ) { return EnvModel::GetAutonomousProcessInfo( i ); }

int ChangeIDUActor( EnvContext *pContext, int idu, int groupID, bool randomize ) { return gpModel->ChangeIDUActor( pContext, idu, groupID, randomize ); }

// policy related methods
int     GetPolicyCount()          { return gpPolicyManager->GetPolicyCount(); }
int     GetUsedPolicyCount()      { return gpPolicyManager->GetUsedPolicyCount(); }
Policy *GetPolicy( int i )        { return gpPolicyManager->GetPolicy( i ); }
Policy *GetPolicyFromID( int id ) { return gpPolicyManager->GetPolicyFromID( id ); }


// Scenario related methods
int       EnvGetScenarioCount()            { return gpScenarioManager->GetCount(); }
Scenario *EnvGetScenario( int i )          { return gpScenarioManager->GetScenario( i ); }
Scenario *EnvGetScenarioFromName( LPCTSTR name, int *index )
   {
   Scenario *pScenario = gpScenarioManager->GetScenario( name );
   
   if ( index != NULL && pScenario != NULL )
      {
      *index = gpScenarioManager->GetScenarioIndex( pScenario );
      }

   return pScenario;
   }


int EnvAddVideoRecorder( int type, LPCTSTR name, LPCTSTR path, int frameRate, int method, int extra )
   {
   VideoRecorder *pVR = new VideoRecorder( path, gpMapPanel, frameRate );

   pVR->SetType( type );
   pVR->SetName( name );
   //pVR->SetPath( path );
   //pVR->SetFrameRate( frameRate );
   pVR->SetCaptureMethod( (VRMETHOD) method );
   pVR->SetExtra( extra );

   int index = gpView->AddVideoRecorder( pVR );
   return index;   
   }


int EnvStartVideoCapture( int vrID )
   {
   VideoRecorder *pVR = gpView->GetVideoRecorder( vrID );

   if ( pVR != NULL )
      {
      pVR->StartCapture();
      return 1;
      }

   return 0;
   }


int EnvCaptureVideo( int vrID )
   {
   VideoRecorder *pVR = gpView->GetVideoRecorder( vrID );

   if ( pVR != NULL )
      {
      if ( pVR->m_pCaptureWnd == gpMapPanel && pVR->m_extra > 0 )  // is there a column defined?  If so, make sure it is active
         {
         // get existing active layer
         int activeCol = gpCellLayer->GetActiveField();

         int col = (int) pVR->m_extra;

         if ( col != activeCol )
            {
            gpMapPanel->m_mapFrame.SetActiveField( col, true );
            gpMapPanel->RedrawWindow();
            }

         pVR->CaptureFrame();
         if ( col != activeCol )
            {
            gpMapPanel->m_mapFrame.SetActiveField( activeCol, true );
            gpMapPanel->RedrawWindow();
            }
         }
      }
   
   return 1;
   }


int EnvEndVideoCapture( int vrID )
   {
   VideoRecorder *pVR = gpView->GetVideoRecorder( vrID );
   
   if ( pVR )
      {
      pVR->EndCapture();
      return 1;
      }

   return 0;
   }


void EnvSetLLMapText( LPCTSTR text )
   {
   gpMapPanel->UpdateLLText( text );
   }


void EnvRedrawMap( void )
   {
   if ( gpMapPanel )
      {
      CDC *dc = gpMapPanel->m_pMapWnd->GetDC();
      gpMapPanel->m_pMapWnd->DrawMap(*dc);
      gpMapPanel->m_pMapWnd->ReleaseDC(dc);

      // yield control to other processes
      MSG  msg;
      while( PeekMessage( &msg, NULL, NULL, NULL , PM_REMOVE ) )
         {
         TranslateMessage(&msg); 
         DispatchMessage(& msg); 
         }
      }
   }

int EnvRunQueryBuilderDlg( LPTSTR qbuffer, int bufferSize )
   {
   PolQueryDlg dlg( gpCellLayer, NULL );

   dlg.m_queryString = qbuffer;

   int retVal = (int) dlg.DoModal();

   if ( retVal == IDOK )
      lstrcpyn( qbuffer, dlg.m_queryString, bufferSize );

   return retVal;
   }


//int EnvStandardizeOutputFilename( LPTSTR filename, LPTSTR pathAndFilename, int maxLength )
//   {
//   EnvCleanFileName( filename );
//    
//   CString path = PathManager::GetPath( PM_OUTPUT_DIR );  // {ProjectDir}\Outputs\CurrentScenarioName\
//   path += filename;
//   _tcsncpy( pathAndFilename, (LPCTSTR) path, maxLength );
//
//   return path.GetLength();
//   }


int EnvCleanFileName (LPTSTR filename)
   {
   if ( filename == NULL )
      return 0;

   TCHAR *ptr = filename;

   while ( *ptr != NULL )
      {
      switch( *ptr )
         {
         case ' ':   *ptr = '_';    break;  // blanks
         case '\\':  *ptr = '_';    break;  // illegal filename characters 
         case '/':   *ptr = '_';    break;
         case ':':   *ptr = '_';    break;
         case '*':   *ptr = '_';    break;
         case '"':   *ptr = '_';    break;
         case '?':   *ptr = '_';    break;
         case '<':   *ptr = '_';    break;
         case '>':   *ptr = '_';    break;
         case '|':   *ptr = '_';    break;
         }

      ptr++;
      }


   return (int) _tcslen( filename );
   }
      


/*x
bool EnvContext::FindIDUattributeCols()
{
   MapLayer* pIDUlayer = (MapLayer*)pMapLayer;
   bool ok = true;

   // Some IDU attributes, in alphabetic order.
   ok = ok && pIDUlayer->CheckCol(AREA, "AREA", TYPE_INT, CC_MUST_EXIST);
   ok = ok && pIDUlayer->CheckCol(COMID, "COMID", TYPE_DOUBLE, CC_MUST_EXIST));
   ok = ok && pIDUlayer->CheckCol(ECOREGION, "ECOREGION", TYPE_DOUBLE, CC_MUST_EXIST));
   ok = ok && pIDUlayer->CheckCol(ELEV_MEAN, "ELEV_MEAN", TYPE_DOUBLE, CC_MUST_EXIST));
   ok = ok && pIDUlayer->CheckCol(F_THETA, "F_THETA", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(FIELD_CAP, "FIELD_CAP", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(HBVCALIB, "HBVCALIB", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(HRU_ID, "HRU_ID", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(HRU_NDX, "HRU_NDX", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(H2O_MELT, "H2O_MELT", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(IDU_ID, "IDU_ID", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(IRRIGATION, "IRRIGATION", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(LAI, "LAI", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(LULC_A, "LULC_A", TYPE_DOUBLE, CC_MUST_EXIST));
   ok = ok && pIDUlayer->CheckCol(MAXSNOW, "MAXSNOW", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(PVT, "PVT", TYPE_DOUBLE, CC_MUST_EXIST));
   ok = ok && pIDUlayer->CheckCol(RAD_SW, "RAD_SW", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(SM_DAY, "SM_DAY", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(SNOW_SWE, "SNOW_SWE", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(SPHUMIDITY, "SPHUMIDITY", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(TEMP, "TEMP", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(TMAX, "TMAX", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(TMIN, "TMIN", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(TREE_HT, "TREE_HT", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(VPD_SCALAR, "VPD_SCALAR", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(WETL_CAP, "WETL_CAP", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(WETL_ID, "WETL_ID", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(WETL2Q, "WETL2Q", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(WETNESS, "WETNESS", TYPE_DOUBLE, CC_AUTOADD);
   ok = ok && pIDUlayer->CheckCol(WINDSPEED, "WINDSPEED", TYPE_FLOAT, CC_AUTOADD);

   return(ok);
} // end of FindIDUattributeCols()
x*/