#include "stdafx.h"

#pragma hdrstop

#include "GlobalMethods.h"
#include "WaterMaster.h"
#include "WaterRights.h"
#include "Flow.h"

#include <UNITCONV.H>
#include <omp.h>

extern FlowProcess *gpFlow;
extern FlowModel *gpModel;


WaterAllocation::~WaterAllocation( void )
   {
   if ( m_pAltWM ) delete m_pAltWM; 
   }


bool WaterAllocation::Init( FlowContext *pFlowContext )
   {
   GlobalMethod::Init( pFlowContext );

   if ( m_pAltWM )
      m_pAltWM->Init( pFlowContext );

   return true;
   }


bool WaterAllocation::InitRun( FlowContext *pFlowContext )
   {
   GlobalMethod::InitRun( pFlowContext );

   if ( m_pAltWM )
      m_pAltWM->InitRun( pFlowContext );

   return true;
   }




bool WaterAllocation::StartYear( FlowContext *pFlowContext )
   {       
  // if ( GlobalMethod::EndYear( pFlowContext ) == true )
  //    return true;

   switch( m_method )
      {
      case GM_WATER_RIGHTS:
         {
         }
         break;
         
      case GM_ALTWM:
         {
         if ( m_pAltWM == NULL )
            return false;

         return m_pAltWM->StartYear( pFlowContext );
         }
         break;

      //case GM_EXPR_ALLOCATOR:
      //   return RunExprAllocator( pFlowContext );

      default:
         ASSERT( 0 );
      }

   return false;
   }


bool WaterAllocation::StartStep(FlowContext *pFlowContext)
   {
   // if ( GlobalMethod::EndYear( pFlowContext ) == true )
   //    return true;

   switch (m_method)
      {
      case GM_WATER_RIGHTS:
	  case GM_ALTWM:
         {
         }
         break;

         //case GM_EXPR_ALLOCATOR:
         //   return RunExprAllocator( pFlowContext );

      default:
         ASSERT(0);
      }

   return false;
   }


bool WaterAllocation::Step(FlowContext *pFlowContext)
   {       
   if ( ( pFlowContext->timing & m_timing ) == 0 )
      return true;

   if ( GlobalMethod::Step( pFlowContext ) == true )   // returns true if handled, faluse otherwise
      return true;

   switch( m_method )
      {
      case GM_WATER_RIGHTS:
         return RunWaterRights( pFlowContext );

      case GM_ALTWM:
         return RunAltWM( pFlowContext );

      case GM_EXPR_ALLOCATOR:
         return RunExprAllocator( pFlowContext );

      default:
         ASSERT( 0 );
      }

   return false;
   }


bool WaterAllocation::EndStep( FlowContext *pFlowContext )
   {       
   // if ( GlobalMethod::EndYear( pFlowContext ) == true )
   //    return true;

   switch( m_method )
      {
      case GM_WATER_RIGHTS:
         {
         }
         break;

      case GM_ALTWM:
         {
         if ( m_pAltWM == NULL )
            return false;

         return m_pAltWM->EndStep( pFlowContext );
         }
         break;

      //case GM_EXPR_ALLOCATOR:
      //   return RunExprAllocator( pFlowContext );

      default:
         ASSERT( 0 );
      }

   return false;
   }


bool WaterAllocation::EndYear( FlowContext *pFlowContext )
   {       
  // if ( GlobalMethod::EndYear( pFlowContext ) == true )
  //    return true;

   switch( m_method )
      {
      case GM_WATER_RIGHTS:
         {
         }
         break;
         
      case GM_ALTWM:
         {
         if ( m_pAltWM == NULL )
            return false;

         return m_pAltWM->EndYear( pFlowContext );
         }
         break;


      //case GM_EXPR_ALLOCATOR:
      //   return RunExprAllocator( pFlowContext );

      default:
         ASSERT( 0 );
      }

   return false;
   }

bool WaterAllocation::RunWaterRights( FlowContext *pFlowContext )
   {
   return true;
   }


bool WaterAllocation::RunAltWM( FlowContext *pFlowContext )
   {
   if ( m_pAltWM == NULL )
      return false;

   m_pAltWM->Step( pFlowContext );
   return true;
   }


bool WaterAllocation::RunExprAllocator( FlowContext* )
   {
   return true;
   }


// Note: this method is static
WaterAllocation *WaterAllocation::LoadXml( TiXmlElement *pXmlWaterAllocation, LPCTSTR filename )
   {
   CString method;
   bool ok = ::TiXmlGetAttr( pXmlWaterAllocation, "method", method, "", true );

   if ( ! ok )
      return NULL;

   WaterAllocation *pMethod = new WaterAllocation( "Allocation", GM_NONE );

   ASSERT( method.IsEmpty() == false );

   switch( method[ 0 ] )
      {
      case 'W':
      case 'w':   // water_rights
      case 'A':
      case 'a':   // alt water_rights
         {
         pMethod->SetMethod( GM_ALTWM );
         AltWaterMaster *pAWM = new AltWaterMaster( pMethod );
         pAWM->LoadXml( pMethod, pXmlWaterAllocation, filename );
         pMethod->m_pAltWM = pAWM;
         }
         break;

      case 'E':
      case 'e':   // expression
         pMethod->SetMethod( GM_EXPR_ALLOCATOR );
         break;         
      
      case 'F':
      case 'f':   // external function
         {
         pMethod->SetMethod( GM_EXTERNAL );

         // source string syntax= fn:<dllpath:function> for functional, db:<datasourcepath:columnname> for datasets
         pMethod->m_extSource = method;
         FLUXSOURCE sourceLocation = ParseSource( pMethod->m_extSource, pMethod->m_extPath, pMethod->m_extFnName,
               pMethod->m_extFnDLL, pMethod->m_extFn );
         
         if ( sourceLocation != FS_FUNCTION )
            {
            Report::ErrorMsg( "Fatal Error parsing Water Allocation External method - no solution will be performed" );
            pMethod->SetMethod( GM_NONE );
            }
         }
         break;

      default:
         pMethod->SetMethod( GM_NONE );
      }
   
   return pMethod;
   }
