#include "stdafx.h"

#include "EnvExtension.h"
#include <Maplayer.h>

/*---------------------------------------------------------------------------------------------------------------------------------------
 * general idea:
 *    1) Create subclasses of EnvEvalModel, EnvAutoProcess for any models you wnat to implement
 *    2) Add any exposed variables by calling
 *         bool EnvExtension::DefineVar( bool input, int index, LPCTSTR name, void *pVar, TYPE type, MODEL_DISTR distType, VData paramLocation, VData paramScale, VData paramShape, LPCTSTR desc );
 *         for any exposed variables
 *    3) Override any of the methods defined in EnvExtension classes as needed
 *    4) Update dllmain.cpp
 *    5) Update *.def file
 *
 *
 *  Required Project Settings (<All Configurations>)
 *    1) General->Character Set: Not Set
 *    2) C/C++ ->General->Additional Include Directories: $(SolutionDir);$(SolutionDir)\libs;
 *    3) C/C++ ->Preprocessor->Preprocessor Definitions: WIN32;_WINDOWS;_AFXEXT;"__EXPORT__=__declspec( dllimport )";
 *    4) Linker->General->Additional Library Directories: $(SolutionDir)\$(ConfigurationName)
 *    5) Linker->Input->Additional Dependencies: libs.lib
 *    6) Build Events->Post Build Events: copy $(TargetPath) $(SolutionDir)
 *
 *    Debug Configuration Only:  C/C++ ->Preprocessor->Preprocessor Definitions:  Add DEBUG;_DEBUG
 *    Release Configuration Only:  C/C++ ->Preprocessor->Preprocessor Definitions:  Add NDEBUG
 *---------------------------------------------------------------------------------------------------------------------------------------
 */

EnvExtension::EnvExtension( int inputCount, int outputCount )
: m_inputVars()
, m_outputVars()
, m_id( -1 )
, m_type( -1 )
, m_pEnvExtension( NULL )
   {
   if ( inputCount > 0 )
      m_inputVars.SetSize( inputCount );

   if ( outputCount > 0 )
      m_outputVars.SetSize( outputCount );
   }


EnvExtension::~EnvExtension()
   {
   }


int EnvExtension::InputVar ( int id, MODEL_VAR** modelVar )
   {
   if ( m_inputVars.GetCount() > 0  )
      *modelVar = m_inputVars.GetData();
   else
      *modelVar = NULL;
   
   return (int) m_inputVars.GetCount();
   }



int EnvExtension::OutputVar( int id, MODEL_VAR** modelVar )
   {
   if ( m_outputVars.GetCount() > 0  )
      *modelVar = m_outputVars.GetData();
   else
      *modelVar = NULL;
   
   return (int) m_outputVars.GetCount();
   }



int EnvExtension::AddVar( bool input, LPCTSTR name, void *pVar, TYPE type, MODEL_DISTR distType, VData paramLocation, VData paramScale, VData paramShape, LPCTSTR description, int flags /*=0*/ )
   {
   MODEL_VAR modelVar( name, pVar, type, distType, paramLocation, paramScale, paramShape, description, flags );

   if ( input )
      return (int) m_inputVars.Add( modelVar );
   else
      return (int) m_outputVars.Add( modelVar );
   }


bool EnvExtension::DefineVar( bool input, int index, LPCTSTR name, void *pVar, TYPE type, MODEL_DISTR distType, VData paramLocation, VData paramScale, VData paramShape, LPCTSTR description )
   {
   if ( input )
      {
      if ( index >= m_inputVars.GetSize() )
         return false;

      MODEL_VAR &modelVar = m_inputVars.GetAt( index );
      modelVar.name = name;
      modelVar.pVar = pVar;
      modelVar.type = type;
      modelVar.distType = distType;
      modelVar.paramLocation = paramLocation;
      modelVar.paramScale = paramScale;
      modelVar.paramShape = paramShape;
      modelVar.description = description;
      modelVar.flags = 0;
      modelVar.extra = 0;
      return true;
      }
   else
      {
      if ( index >= m_outputVars.GetSize() )
         return false;

      MODEL_VAR &modelVar = m_outputVars.GetAt( index );
      modelVar.name = name;
      modelVar.pVar = pVar;
      modelVar.type = type;
      modelVar.distType = distType;
      modelVar.paramLocation = paramLocation;
      modelVar.paramScale = paramScale;
      modelVar.paramShape = paramShape;
      modelVar.description = description;
      return true;
      }
   }


INT_PTR EnvExtension::AddDelta( EnvContext *pContext, int idu, int col, VData newValue )
   {
   // valid AddDelta ptr?  (NULL means add delta clled during Init() )
   if ( pContext->ptrAddDelta != NULL )
      return pContext->ptrAddDelta( pContext->pEnvModel, idu, col, pContext->currentYear, newValue, pContext->handle );
   else
      {
      MapLayer *pMapLayer = (MapLayer*) pContext->pMapLayer;
      pMapLayer->SetData( idu, col, newValue );
      return -1;
      }
   }


bool EnvExtension::CheckCol( const MapLayer *pLayer, int &col, LPCTSTR label, TYPE type, int flags )
   {
   return ((MapLayer*) pLayer)->CheckCol( col, label, type, flags );
   }



bool EnvExtension::UpdateIDU( EnvContext *pContext, int idu, int col, VData &value, bool useAddDelta )
   {
   if ( useAddDelta )
      {
      VData oldValue;
      pContext->pMapLayer->GetData( idu, col, oldValue );
      if ( oldValue != value )
         {
         this->AddDelta( pContext, idu, col, value );
         return true;
         }
      else
         return false;
      }
   else
      ((MapLayer*)(pContext->pMapLayer))->SetData( idu, col, value );

   return true;
   }



bool EnvExtension::UpdateIDU( EnvContext *pContext, int idu, int col, float value, bool useAddDelta )
   {
   if ( useAddDelta )
      {
      float oldValue;
      bool successful = pContext->pMapLayer->GetData( idu, col, oldValue );
      if ( !successful || oldValue != value )
         this->AddDelta( pContext, idu, col, value );
      }
   else
      ((MapLayer*)(pContext->pMapLayer))->SetData( idu, col, value );

   return true;
   }

bool EnvExtension::UpdateIDU( EnvContext *pContext, int idu, int col, int value, bool useAddDelta )
   {
   if ( useAddDelta )
      {
      int oldValue;
      pContext->pMapLayer->GetData( idu, col, oldValue );
      if ( oldValue != value )
         {
         this->AddDelta( pContext, idu, col, value );
         return true;
         }
      else
         return false;
      }
   else
      ((MapLayer*)(pContext->pMapLayer))->SetData( idu, col, value );

   return true;
   }

bool EnvExtension::UpdateIDU( EnvContext *pContext, int idu, int col, double value, bool useAddDelta )
   {
   if ( useAddDelta )
      {
      double oldValue;
      pContext->pMapLayer->GetData( idu, col, oldValue );
      if ( oldValue != value )
         {
         this->AddDelta( pContext, idu, col, VData( value ) );
         return true;
         }
      else
         return false;
      }
   else
      ((MapLayer*)(pContext->pMapLayer))->SetData( idu, col, value );

   return true;
   }

bool EnvExtension::UpdateIDU( EnvContext *pContext, int idu, int col, short value, bool useAddDelta )
   {
   if ( useAddDelta )
      {
      int oldValue;
      pContext->pMapLayer->GetData( idu, col, oldValue );
      if ( oldValue != value )
         {
         this->AddDelta( pContext, idu, col, value );
         return true;
         }
      else
         return false;
      }
   else
      ((MapLayer*)(pContext->pMapLayer))->SetData( idu, col, value );

   return true;
   }

bool EnvExtension::UpdateIDU( EnvContext *pContext, int idu, int col, bool value, bool useAddDelta )
   {
   if ( useAddDelta )
      {
      bool oldValue;
      pContext->pMapLayer->GetData( idu, col, oldValue );
      if ( oldValue != value )
         {
         this->AddDelta( pContext, idu, col, value );
         return true;
         }
      else
         return false;
      }
   else
      ((MapLayer*)(pContext->pMapLayer))->SetData( idu, col, value );

   return true;
   }

