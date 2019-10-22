// LateralExchanges.cpp : Implements global evapotransporation methods for Flow
//

#include "stdafx.h"
#pragma hdrstop

#include "GlobalMethods.h"
#include "Flow.h"
#include <omp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern FlowProcess *gpFlow;
extern FlowModel   *gpModel;


bool LateralExchange::Step( FlowContext *pFlowContext )
   {
   // handle NONE, EXTERNAL cases if defined
   if ( GlobalMethod::Step( pFlowContext ) == true )
      return true;

   switch( m_method )
      {
      case GM_LINEAR_RESERVOIR:
         return SetGlobalHruToReachExchangesLinearRes();

      case GM_NONE:
         return true;

      default:
         ASSERT( 0 );
      }

   return false;
   }


bool LateralExchange::SetGlobalHruToReachExchangesLinearRes( void )
   {
   int catchmentCount = gpModel->GetCatchmentCount(); // (int) m_catchmentArray.GetSize();
   int hruLayerCount = gpModel->GetHRULayerCount();

   // iterate through catchments/hrus/hrulayers, calling fluxes as needed
   for ( int i=0; i < catchmentCount; i++ )
      {
      Catchment *pCatchment = gpModel->GetCatchment( i ); //m_catchmentArray[ i ];
      ASSERT( pCatchment != NULL );

      int hruCount = pCatchment->GetHRUCount();
      for ( int h=0; h < hruCount; h++ )
         {
         HRU *pHRU = pCatchment->GetHRU( h );
         
         int l = hruLayerCount-1;      // bottom layer only
         HRULayer *pHRULayer = pHRU->GetLayer( l );
         
         float depth = 1.0f; 
         float porosity = 0.4f;
         float voidVolume = depth*porosity*pHRU->m_HRUeffArea_m2;
         float wc = float( pHRULayer->m_volumeWater/voidVolume );
 
         float waterDepth = wc*depth;
         float baseflow = GetBaseflow(waterDepth)*pHRU->m_HRUeffArea_m2;//m3/d

         pHRULayer->m_contributionToReach = baseflow;
         pCatchment->m_contributionToReach += baseflow;
         }
      }

   return true;
   }


LateralExchange *LateralExchange::LoadXml( TiXmlElement *pXmlLateralExchange, LPCTSTR filename )
   {
   LateralExchange *pLatExch = new LateralExchange( "Lateral Exchange" );  // defaults to GMT_NONE

   if ( pXmlLateralExchange == NULL )
      return pLatExch;

   LPTSTR name  = NULL;
   LPTSTR method  = NULL;
   LPTSTR query = NULL;   // ignored for now

   XML_ATTR attrs[] = {
      // attr                 type          address                   isReq  checkCol
      { "name",               TYPE_STRING,  &name,                    false,   0 },
      { "method",             TYPE_STRING,  &method,                  false,   0 },
      { "query",              TYPE_STRING,  &query,                   false,   0 },
      { NULL,                 TYPE_NULL,    NULL,                     false,  0 } };

   bool ok = TiXmlGetAttributes( pXmlLateralExchange, attrs, filename );
   if ( ! ok )
      {
      CString msg; 
      msg.Format( _T("Flow: Misformed element reading <lateral_exchange> attributes in input file %s"), filename );
      Report::ErrorMsg( msg );
      return NULL;
      }

   if ( name != NULL && *name != NULL && *name != ' ' )
      pLatExch->m_name = name;

   if ( method )
      {
      switch( method[ 0 ] )
         {
         case 'l':
         case 'L':
            pLatExch->SetMethod( GM_LINEAR_RESERVOIR  );
            break;

         case 'N':
         case 'n':
            pLatExch->SetMethod( GM_NONE );
            break;

         case 'f':
         case 'F':
            if ( strncmp( method, "fn", 2 ) == 0 )
              {
              pLatExch->m_method = GM_EXTERNAL;
              // source string syntax= fn:<dllpath:function> for functional, db:<datasourcepath:columnname> for datasets
              pLatExch->m_extSource = method;
              FLUXSOURCE sourceLocation = ParseSource( pLatExch->m_extSource, pLatExch->m_extPath, pLatExch->m_extFnName,
                    pLatExch->m_extFnDLL, pLatExch->m_extFn );
              
              if ( sourceLocation != FS_FUNCTION )
                 {
                 Report::ErrorMsg( "Fatal Error on direct lateral exchange solution method - no solution will be performed" );
                 pLatExch->SetMethod( GM_NONE );
                 }
              }
            break;         

         default:
            {
            CString msg;
            msg.Format( "Flow: Invalid method '%s' specified for <lateral_exchange> tag reading %s", method, filename );
            Report::WarningMsg( msg );
            }
         }
      }

   return pLatExch;
   }
