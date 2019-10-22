// SpatialAllocator.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"

#include "SpatialAllocator.h"

#include <tixml.h>
#include <BitArray.h>
#include <randgen/Randunif.hpp>
#include <misc.h>
#include <PathManager.h>
#include <fstream>
#include <iostream>
#include <EnvEngine/EnvModel.h>
#include <EnvInterface.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern SpatialAllocator *theProcess;

int CompareScores(const void *elem0, const void *elem1 );

//std::ofstream m_spatialAllocatorResultsFile;

Preference::Preference( Allocation *pAlloc, LPCTSTR name, LPCTSTR queryStr, LPCTSTR weightExpr ) 
   : m_name( name )
   , m_queryStr( queryStr )
   , m_weightExpr( weightExpr )
   , m_pQuery( NULL )
   , m_pMapExpr( NULL ) 
   , m_value( 0 )
   {
   // compile query
   if (queryStr != NULL && queryStr[0] != NULL )
      {
      m_pQuery = theProcess->m_pQueryEngine->ParseQuery( queryStr, 0, "SpatialAllocator.Preference" );

      if ( m_pQuery == NULL )
         {
         CString msg;
         msg.Format( "Spatial Allocator: Bad Preference query encountered reading Allocation '%s' - Query is '%s'", (LPCTSTR) pAlloc->m_name, queryStr );
         msg += queryStr;
         Report::ErrorMsg( msg );
         }
      }

   // see the if weightExpr is a constant
   bool isConstant = true;
   for( int i=0; i < lstrlen( weightExpr ); i++ )
      if ( !( isdigit( weightExpr[ i ] ) || weightExpr[ i ] == '-' || weightExpr[ i ] == '.' ) )
         {
         isConstant = false;
         break;
         }

   if ( isConstant )
      m_value = (float) atof( weightExpr );
   else
      {
      m_pMapExpr = theProcess->m_pMapExprEngine->AddExpr( name, weightExpr, queryStr );
      bool ok = theProcess->m_pMapExprEngine->Compile( m_pMapExpr );

      if ( ! ok )
         {
         CString msg( "Spatial Allocator: Unable to compile map expression " );
         msg += weightExpr;
         msg += " for <preference> '";
         msg += name;
         msg += "'.  The expression will be ignored";
         Report::ErrorMsg( msg );
         }
      }
   }


Constraint::Constraint( LPCTSTR name, LPCTSTR queryStr ) 
   : m_name( name )
   , m_queryStr( queryStr )
   , m_cType( CT_QUERY )
   , m_pQuery( NULL ) 
   { 
   m_pQuery = theProcess->m_pQueryEngine->ParseQuery( queryStr, 0, "SpatialAllocator.Constraint" );

   if ( m_pQuery == NULL )
      {
      CString msg( "Spatial Allocator: Unable to compile constraint query" );
      msg += queryStr;
      msg += " for <constraint> '";
      msg += name;
      msg += "'.  The query will be ignored";
      Report::ErrorMsg( msg );
      }
   }


float TargetContainer::SetTarget( int year )
   {
   // is a target defined?

   if ( ! IsTargetDefined() )
      {
      m_currentTarget = -1;
      return 0;
      }

   // target is defined, get value
   ASSERT( m_pTargetData->GetRowCount() > 0 );
   m_currentTarget = m_pTargetData->IGet( (float) year, 0, 1 );   // xCol=0, yCol=1

   // apply domain, query modifiers if needed
   if ( this->m_targetDomain == TD_PCTAREA )
      {
      // targets values should be considered decimal percentages. These area applied to the 
      // area that satisfies the target query, or the entire study area if a query is not defined

      float targetArea = 0;
      MapLayer *pLayer = theProcess->m_pMapLayer;

      if ( this->m_pTargetQuery != NULL )
         {
         // run query
         this->m_pTargetQuery->Select( true );

         int selectedCount = pLayer->GetSelectionCount();
         
         for ( int i=0; i < selectedCount; i++ )
            {
            int idu = pLayer->GetSelection( i );

            float area = 0;
            pLayer->GetData( idu, theProcess->m_colArea, area );

            targetArea += area;
            }

         pLayer->ClearSelection();
         }
      else
         targetArea = pLayer->GetTotalArea();

      // have area to use, convert target fromdecimal  percent to area
      m_currentTarget *= targetArea;
      }

   return m_currentTarget;
   }


void TargetContainer::Init( int id, int colAllocSet, int colSequence )
   {
   // not a valid target?  no action required then
   if (! IsTargetDefined() )
      {
      m_currentTarget = -1;
      return;
      }

   m_pTargetData = new FDataObj( 2, 0 );   // two cols, zero rows

   //if ( m_targetLocation.GetLength() > 0 )
   //   {
   //   m_pTargetMapExpr = theProcess->m_pMapExprEngine->AddExpr( m_name, m_targetLocation, NULL );  // no query string needed, this is handled by constraints
   //   bool ok = theProcess->m_pMapExprEngine->Compile( m_pTargetMapExpr );
   //
   //   if ( ! ok )
   //      {
   //      CString msg( "Spatial Allocator: Unable to compile target location map expression " );
   //      msg += m_targetLocation;
   //      msg += " for <allocation> '";
   //      msg += m_name;
   //      msg += "'.  The expression will be ignored";
   //      Report::ErrorMsg( msg );
   //
   //      theProcess->m_pMapExprEngine->RemoveExpr( pExpr );
   //      m_pTargetMapExpr = NULL;
   //      }
   //   }

   switch( this-> m_targetSource )
      {
      case TS_FILE:
         {
         int rows = m_pTargetData->ReadAscii( this->m_targetValues );
         if ( rows <= 0 )
            {
            CString msg( "Spatial Allocator: Unable to load target file '" );
            msg += m_targetValues;
            msg += "' - this allocation will be ignored";
            Report::ErrorMsg( msg );
            }
         }
         break;

      case TS_TIMESERIES:
         {
         TCHAR *targetValues = new TCHAR[ this->m_targetValues.GetLength()+2 ];
         memset( targetValues, 0, (this->m_targetValues.GetLength()+2) * sizeof( TCHAR ) );

         lstrcpy( targetValues, this->m_targetValues );
         TCHAR *nextToken = NULL;

         // note: target values look like sequences of x/y pairs
         LPTSTR token = _tcstok_s( targetValues, _T(",() ;\r\n"), &nextToken );
         float pair[2];
         while ( token != NULL )
            {
            pair[0] = (float) atof( token );
            token   = _tcstok_s( NULL, _T(",() ;\r\n"), &nextToken );
            pair[1] = (float) atof( token );
            token   = _tcstok_s( NULL, _T(",() ;\r\n"), &nextToken );
   
            m_pTargetData->AppendRow( pair, 2 );
            }

         delete [] targetValues;
         }
         break;

      case TS_RATE:
         {
         m_targetRate = (float) atof( m_targetValues );

         // note: target values are expressed in terms of the target basis field
         TCHAR *targetValues = new TCHAR[ this->m_targetValues.GetLength()+2 ];
         memset( targetValues, 0, (this->m_targetValues.GetLength()+2 )*sizeof( TCHAR ) );

         lstrcpy( targetValues, this->m_targetValues );
         TCHAR *nextToken = NULL;

         float rate = (float) atof( targetValues );
         float basis = 0;
         MapLayer *pLayer = theProcess->m_pMapLayer;

         //bool isSequence = this->IsSequence() && m_pAllocSet->m_colSequence >= 0;
         
         // get starting values from map
         for ( MapLayer::Iterator idu=pLayer->Begin(); idu < pLayer->End(); idu++ )
            {
            float iduBasis = 0;
            pLayer->GetData( idu, m_colTargetBasis, iduBasis );

            if ( colSequence >= 0  )
               {
               int sequenceID = -1;
               pLayer->GetData( idu, colSequence, sequenceID );

               if ( sequenceID == id )
                  basis += iduBasis;
               }
            else
               {
               int attr = -1;
               pLayer->GetData( idu, colAllocSet, attr );
               if ( attr == id )
                  basis += iduBasis;
               }
            }

         float pair[ 2 ];
         pair[0] = 0;
         pair[1] = basis;
         m_pTargetData->AppendRow( pair, 2 );

         float growth = basis * rate * 100; 
         
         pair[0] = 100;
         pair[1] = basis + growth;
         m_pTargetData->AppendRow( pair, 2 );

         delete [] targetValues;
         }

      break;
      }
   
   //CString msg( "SA: Initial Allocation Target Table for " );
   //msg += m_name;   
   //Report::LogMsg( msg );
   //
   //for ( int i=0; i < m_pTargetData->GetRowCount(); i++ )
   //   {
   //   int year = m_pTargetData->GetAsInt( 0, i );
   //
   //   float area = m_pTargetData->GetAsFloat( 1, i );
   //
   //   CString data;
   //   if ( this->m_targetDomain == TD_PCTAREA )
   //      {
   //      area *= 100;
   //      data.Format( "    Year: %i  Target Area: %.2f percent", year, area );
   //      }
   //   else
   //      {
   //      area /= 10000;  // m2 to ha
   //      data.Format( "    Year: %i  Target Area: %.1f ha", year, area );
   //      }
   //
   //   Report::LogMsg( data );
   //   }
   }


void TargetContainer::SetTargetParams( MapLayer *pLayer, LPCTSTR basis, LPCTSTR source, LPCTSTR values, LPCTSTR domain, LPCTSTR query )
   {
   // source
   if ( source != NULL )
      {
      if ( source[0] == 't' || source[0] == 'T' )
         this->m_targetSource = TS_TIMESERIES;
      else if ( source[0] == 'f' || source[0] == 'F' )
         this->m_targetSource = TS_FILE;
      else if ( source[0] == 'r' || source[0] == 'R' )
         this->m_targetSource = TS_RATE;
      else if ( source[0] == 'u'  || source[0] == 'U')
         {
         if ( this->GetTargetClass() == TC_ALLOCATION )
            this->m_targetSource = TS_USEALLOCATIONSET;
         else
            this->m_targetSource = TS_USEALLOCATION;
         }
      }
   else
      {
      // no source specified - assume specified elsewhere
      this->m_targetSource = TS_UNDEFINED;
      }

   // values
   if ( values != NULL )
      this->m_targetValues = values;      

   // domain
   if ( domain != NULL && domain[ 0 ] == 'p' )   // note - if not specified, TD_AREA is used
      {
      m_targetDomain = TD_PCTAREA;
      
      if ( query != NULL )
         {
         this->m_targetQuery = query;
         this->m_pTargetQuery = theProcess->m_pQueryEngine->ParseQuery( query, 0, "" );

         if ( this->m_pTargetQuery == NULL )
            {
            CString msg( "Spatial Allocator: Unable to parse target query '" );
            msg += query;
            msg += "' - it will be ignored";
            Report::WarningMsg( msg );
            }
         }
      }
         
   // basis field (default is "AREA" unless defined elsewhere, in which case it is blank.)
   if ( basis != NULL )
      this->m_targetBasisField = basis;
   else
      {
      // defined at the allocation set level?
      if ( this->GetTargetClass() == TC_ALLOCATIONSET && ( this->m_targetSource == TS_USEALLOCATION ) )
         this->m_targetBasisField = "AREA";
      else if ( this->GetTargetClass() == TC_ALLOCATION && this->m_targetSource != TS_USEALLOCATIONSET )
         this->m_targetBasisField = "AREA";
      else
         this->m_targetBasisField == "";
      }

   this->m_colTargetBasis = pLayer->GetFieldCol( this->m_targetBasisField );
   }





Allocation::Allocation( AllocationSet *pSet, LPCTSTR name, int id ) //, TARGET_SOURCE targetSource, LPCTSTR targetValues )
   : TargetContainer( name ) // targetSource, TD_AREA, targetValues )
   , m_id( id )
   //, m_pTargetMapExpr( NULL )
   , m_useExpand( false )       // allo expanded outcomes?
   , m_expandAreaStr()
   , m_expandType( 0 )
   , m_pRand( NULL )
   , m_expandMaxArea( -1 )      // maximum area allowed for expansion, total including kernal
   , m_expandAreaParam0( -1 )
   , m_expandAreaParam1( 0 )
   , m_expandQueryStr()         // expand into adjacent IDUs that satify this query 
   , m_pExpandQuery( NULL )
   , m_initPctArea( 0 )
   , m_cumBasisActual( 0 )
   , m_cumBasisTarget( 0 )
   , m_minScore( 0 )       // min and max score for this allocation
   , m_maxScore( 0 )
   , m_scoreCount( 0 )     // number of scores greater than 0 at any given time
   , m_usedCount( 0 )
   , m_scoreArea( 0 )      // area of scores greater than 0 at any given time
   ,  m_scoreBasis( 0 )    // 
   , m_expandArea( 0 )    // 
   ,  m_expandBasis( 0 )    // 
   , m_constraintAreaSatisfied( 0 )
   , m_colScores( -1 )
   , m_pAllocSet( pSet )
   //, m_iduScoreArray( NULL )
   , m_currentIduScoreIndex( -1 )
   { }



Allocation::~Allocation( void )
   {
   if ( m_pRand != NULL )
      delete m_pRand;
   }


/*
Allocation& Allocation::operator = (const Allocation &a )
   {
   m_name          = a.m_name;
   m_id            = a.m_id;
    m_targetSource    = a. m_targetSource;
   m_targetValues  = a.m_targetValues;
   m_useExpand     = a.m_useExpand;
   m_expandAreaStr = a.m_expandAreaStr;
   m_expandType    = a.m_expandType;

   if ( a.m_pRand != NULL )
      {
      switch( m_expandType )
         {
         case ET_UNIFORM:
            m_pRand = new RandUniform( a.m_expandAreaParam0, a.m_expandAreaParam1, 0 );
            break;

         case ET_NORMAL:
            m_pRand = new RandNormal( a.m_expandAreaParam0, a.m_expandAreaParam1, 0 );
            break;

         case ET_WEIBULL:
            m_pRand = new RandWeibull( a.m_expandAreaParam0, a.m_expandAreaParam1 );
            break;

         case ET_LOGNORMAL:
            m_pRand = new RandLogNormal( a.m_expandAreaParam0, a.m_expandAreaParam1, 0 );
            break;

         case ET_EXPONENTIAL:
            m_pRand = new RandExponential( a.m_expandAreaParam0 );
            break;
         }
      }
   else
      m_pRand = NULL;

   m_expandAreaParam0 = a.m_expandAreaParam0;
   m_expandAreaParam1 = a.m_expandAreaParam1;
   m_expandMaxArea = a.m_expandMaxArea;
   m_expandQueryStr = a.m_expandQueryStr;

   if ( a.m_pExpandQuery != NULL )
      m_pExpandQuery = new Query( *a.m_pExpandQuery );
   else
      m_pExpandQuery = NULL;

   m_initPctArea   = a.m_initPctArea;
   m_cumBasisActual       = a.m_cumBasisActual;
   m_cumBasisTarget = a.m_cumBasisTarget;

   m_targetData    = a.m_targetData;
   
   m_prefsArray.DeepCopy( a.m_prefsArray );
   m_constraintArray.DeepCopy( a.m_constraintArray );
  
   m_sequenceArray.Copy( a.m_sequenceArray );
   //m_sequenceMap = (const CMap<int,int,int,int> & ) a.m_sequenceMap;
   m_iduScoreArray.Copy( a.m_iduScoreArray );

   m_scoreCol  = a.m_scoreCol;
   m_colScores = a.m_colScores;

   m_allocationSoFar = a.m_allocationSoFar;
   m_currentTarget   = a.m_currentTarget;
   m_targetRate      = a.m_targetRate;
   m_currentIduScoreIndex = a.m_currentIduScoreIndex;
   
   m_pAllocSet = a.m_pAllocSet;   // associated allocation set
   
   return *this;
   }
*/


float Allocation::SetMaxExpandArea( void )
   {
   switch( this->m_expandType )
      {
      case ET_CONSTANT:
         break;      // nothing required

      case ET_UNIFORM:
      case ET_NORMAL:
      case ET_WEIBULL:
      case ET_LOGNORMAL:
      case ET_EXPONENTIAL:
         ASSERT( m_pRand != NULL );
         m_expandMaxArea = (float) m_pRand->RandValue();
         break;
      }

   return m_expandMaxArea;
   }



int AllocationSet::OrderIDUs( CUIntArray &iduArray, RandUniform *pRandUnif )
   {
   int iduCount = (int) iduArray.GetSize();

   // order is dependant on two things:
   // 1) whether shuffling is turned on - this turns on/off random/probabilistic sampling
   // 2) whether a target location surface is defined.
   // --------------------------------------------------------------------------
   //            Shuffling ->   |          On            |         Off         |
   // --------------------------------------------------------------------------
   //  Location is defined      | probablistically       | pick from best      |
   //                           | sample location surface| downward            |
   // --------------------------------------------------------------------------
   //  Location is NOT defined  | shuffle IDUs           | go in IDU order     |
   // --------------------------------------------------------------------------

   if ( m_shuffleIDUs )
      {
      // case 1: Shuffling on, location defined...
      if ( m_pTargetMapExpr )
         {
         // generate probability surface
         //for ( int i=0; i < 


         }
      else  // case 2:  shuffling on, no location defined
         {
         ::ShuffleArray< UINT >( iduArray.GetData(), iduCount, pRandUnif );
         }
      }
   else
      {
      // case 3: Shuffling off, location defined...
      if ( m_pTargetMapExpr )
         {
         }
      else  // case 4:  shuffling off, no location defined
         {
         for ( int i=0; i < iduCount; i++ )
            iduArray[ i ] = i;
         }
      }

   return iduCount;
   }





//=====================================================================================================
// S P A T I A L    A L L O C A T O R
//=====================================================================================================

SpatialAllocator::~SpatialAllocator( void )
   { 
   if ( m_pQueryEngine ) 
      delete m_pQueryEngine; 
   
   if ( m_pMapExprEngine ) 
      delete m_pMapExprEngine; 

   if ( m_pRandUnif )
      delete m_pRandUnif;

   }



BOOL SpatialAllocator::Init( EnvContext *pEnvContext, LPCTSTR initStr )
   {
   m_pMapLayer = (MapLayer*) pEnvContext->pMapLayer;

   // allocate expression and query engines
   ASSERT( m_pMapExprEngine == NULL );
   m_pMapExprEngine = new MapExprEngine( m_pMapLayer ); // pEnvContext->pExprEngine;
   
   if ( m_pQueryEngine == NULL )
      m_pQueryEngine = new QueryEngine( m_pMapLayer );

   // initialize internal array for sorting IDUs
   int iduCount = m_pMapLayer->GetPolygonCount();

   m_iduArray.SetSize( iduCount );
   for ( int i=0; i < iduCount; i++ )
      m_iduArray[ i ] = i;

   // load input file
   if ( LoadXml( initStr ) == false )
      return FALSE;
   
   // iterate through AllocationSets and Allocations to initialize internal data
   for( int i=0; i < (int) m_allocationSetArray.GetSize(); i++ )
      {
      AllocationSet *pAllocSet = m_allocationSetArray[ i ];
      
      for ( int j=0; j < (int) pAllocSet->m_allocationArray.GetSize(); j++ )
         {
         Allocation *pAlloc = pAllocSet->m_allocationArray[ j ];

         pAlloc->m_iduScoreArray.SetSize( iduCount );
         for ( int k=0; k < iduCount; k++ )
            {
            pAlloc->m_iduScoreArray[ k ].idu = -1;
            pAlloc->m_iduScoreArray[ k ].score = 0;
            }
         }
      }

   // allocation sequences on map
   PopulateSequences();

   // set up initial target structures for each allocation and expose input/output variables variables
   for ( int i=0; i < (int) m_allocationSetArray.GetSize(); i++ )
      {
      AllocationSet *pAllocSet = m_allocationSetArray[ i ];

      pAllocSet->Init( -1, pAllocSet->m_col, -1 );

      // set up data objects for the AllocationSet
      bool isTargetDefined = pAllocSet->IsTargetDefined();

      int cols = 0;
      if ( isTargetDefined )
         cols = 1 + 2 + 9*pAllocSet->GetAllocationCount();  // time + 2 per AllocationSet + 9 per Allocation
      else
         cols = 1 + 0 + 10*pAllocSet->GetAllocationCount();  // time + 0 per AllocationSet + 10 per Allocation

      // first one percent areas
      int col = 0;
      CString label( "AllocationSet-" );
      label += pAllocSet->m_name;
      pAllocSet->m_pOutputData = new FDataObj( cols, 0 );
      pAllocSet->m_pOutputData->SetName( label );
      pAllocSet->m_pOutputData->SetLabel( col++, "Time" );

      if ( isTargetDefined )
         {
         pAllocSet->m_pOutputData->SetLabel( col++, "Total Target");
         pAllocSet->m_pOutputData->SetLabel( col++, "Total Realized");         
         }

      for ( int i=0; i < pAllocSet->GetAllocationCount(); i++ )
         {
         Allocation *pAlloc = pAllocSet->GetAllocation( i );
         
         if ( ! isTargetDefined )
            {
            label = pAlloc->m_name + "-Target";
            pAllocSet->m_pOutputData->SetLabel( col++, label );
            }
                  
         label = pAlloc->m_name + "-Realized";
         pAllocSet->m_pOutputData->SetLabel( col++, label );

         label = pAlloc->m_name + "-pctRealized";
         pAllocSet->m_pOutputData->SetLabel( col++, label );

         label = pAlloc->m_name + "-ConstraintArea";
         pAllocSet->m_pOutputData->SetLabel( col++, label );

         label = pAlloc->m_name + "-MinScore";
         pAllocSet->m_pOutputData->SetLabel( col++, label );

         label = pAlloc->m_name + "-MaxScore";
         pAllocSet->m_pOutputData->SetLabel( col++, label );

         label = pAlloc->m_name + "-ScoreCount";
         pAllocSet->m_pOutputData->SetLabel( col++, label );

         label = pAlloc->m_name + "-ScoreBasis";
         pAllocSet->m_pOutputData->SetLabel( col++, label );

         label = pAlloc->m_name + "-ExpandArea";
         pAllocSet->m_pOutputData->SetLabel( col++, label );

         label = pAlloc->m_name + "-ExpandBasis";
         pAllocSet->m_pOutputData->SetLabel( col++, label );
         }

      // add output variables
      CString varName( pAllocSet->m_name );
      varName += ".InUse";
      AddInputVar( varName, pAllocSet->m_inUse, "Use this Allocation Set for the given scenario" );

      varName = "AllocationSet-";
      varName += pAllocSet->m_name;
      AddOutputVar( varName, pAllocSet->m_pOutputData, "" );

      for ( int j=0; j < pAllocSet->GetAllocationCount(); j++ )
         {
         Allocation *pAlloc = pAllocSet->GetAllocation( j );
         pAlloc->Init( pAlloc->m_id, pAllocSet->m_col, pAlloc->IsSequence() ? pAllocSet->m_colSequence : -1 );

         // next, for each Allocation, add input variable for the target rate and initPctArea
         if ( pAlloc->m_targetSource == TS_RATE )
            {
            CString varName( pAlloc->m_name );
            varName += ".Target Rate";
            AddInputVar( varName, pAlloc->m_targetRate, "Target Rate of change of the allocation, dec. percent" );
         
            //if ( pAlloc->IsSequence() )
            //   {
            //   varName = pAlloc->m_name;
            //   varName += ".Initial Percent Area";
            //   AddInputVar( varName, pAlloc->m_initPctArea, "Initial portion of landscape area, dec. percent" );
            //   }
            }
         }
      }
   
   //InitRun( pEnvContext, false );

   return TRUE;
   }


BOOL SpatialAllocator::InitRun( EnvContext *pEnvContext, bool useInitialSeed )
   {
   // populate initial scores
   SetAllocationTargets( pEnvContext->weatherYear );
   ScoreIduAllocations( pEnvContext, false );

   // clear data objects
   for( int i=0; i < (int) m_allocationSetArray.GetSize(); i++ )
      {
      AllocationSet *pAllocSet = m_allocationSetArray[ i ];
      pAllocSet->m_allocationSoFar = 0;
      
      // init each allocation
      for ( int j=0; j < pAllocSet->GetAllocationCount(); j++ )
         {
         Allocation *pAlloc = pAllocSet->GetAllocation( j );
         pAlloc->m_cumBasisActual = 0;
         pAlloc->m_cumBasisTarget = 0;
         pAlloc->m_allocationSoFar = 0;
         pAlloc->m_constraintAreaSatisfied = 0;
         }

      // clear out output data
      if ( pAllocSet->m_pOutputData )
         pAllocSet->m_pOutputData->ClearRows();

      //if ( pAllocSet->m_pOutputDataCum )
      //   pAllocSet->m_pOutputDataCum->ClearRows();
      }
   
   return TRUE; 
   }


BOOL SpatialAllocator::Run( EnvContext *pContext )
   {
   // set the desired value for the targets
   SetAllocationTargets( pContext->weatherYear );

   // first score idus for each allocation
   ScoreIduAllocations( pContext, true );

   // so, at this point we've computed scores for each allocation in each alloction set
   // now allocate them based on scores and allocation targets.  
   switch( m_allocMethod )
      {
      case AM_SCORE_PRIORITY:
         AllocScorePriority( pContext, true);
         break;

      case AM_BEST_WINS:
         AllocBestWins( pContext, true );
         break;
      }

   // report results to output window
   CollectData( pContext );

   return TRUE;
   }


BOOL SpatialAllocator::EndRun( EnvContext *pContext )
   {
	return true;
	}


bool SpatialAllocator::PopulateSequences( void )
   {
   // only called during Init - sets up initial sequences
   //Report::LogMsg( "Spatial Allocator: Populating Sequences" );
   bool hasSequences = false;

   // get total basis value
   // NOTE: FOR SEQUENCES, ONLY AREA IS CURRENTLY CONSIDERED - THIS SHOULD BE MODIFIED IN THE FUTURE
   
   m_totalArea = m_pMapLayer->GetTotalArea();
   int colArea = m_pMapLayer->GetFieldCol( "AREA" );

   // determine whether there are any sequences
   for ( int i=0; i < (int) m_allocationSetArray.GetSize(); i++ )
      {
      AllocationSet *pAllocSet = m_allocationSetArray[ i ];
      
      if ( pAllocSet->m_inUse )
         {   
         if ( pAllocSet->m_colSequence >= 0 )
            {
            m_pMapLayer->SetColNoData( pAllocSet->m_colSequence );
            hasSequences = true;

            // if a sequence, initialize based on starting pct
            for ( int j=0; j < (int) pAllocSet->m_allocationArray.GetSize(); j++ )
               {
               Allocation *pAlloc = pAllocSet->m_allocationArray[ j ];
   
               pAlloc->m_allocationSoFar = 0;
               pAlloc->m_currentTarget = pAlloc->m_initPctArea * m_totalArea;   // why is this here?
               }
            }
         }
      }

   if ( hasSequences == false )
      return false;

   // set up a shuffle array to randomly look through IDUs when allocating sequences
   int iduCount = m_pMapLayer->GetPolygonCount(); 
   CArray< int > iduArray;
   iduArray.SetSize( iduCount );
   for ( int i=0; i < iduCount; i++ )
      iduArray[ i ] = i;      

   RandUniform rnd( 0, iduCount );

   ::ShuffleArray< int >( iduArray.GetData(), iduCount, &rnd );

   // iterate through maps as needed
   //for ( int MapLayer::Iterator idu=m_pMapLayer->Begin(); idu < m_pMapLayer->End(); idu++ )
   for ( int m=0; m < iduCount; m++ )
      {
      int idu = iduArray[ m ];         // a random grab

      m_pQueryEngine->SetCurrentRecord( idu );
      m_pMapExprEngine->SetCurrentRecord( idu );

      for( int i=0; i < (int) m_allocationSetArray.GetSize(); i++ )
         {
         AllocationSet *pAllocSet = m_allocationSetArray[ i ];
 
         if ( pAllocSet->m_inUse && pAllocSet->m_colSequence >= 0 )
            {
            // is this IDU already tagged to a sequence in this allocation set?  if so, skip it.
            int sequenceID = -1;
            m_pMapLayer->GetData( idu, pAllocSet->m_colSequence, sequenceID );

            if ( sequenceID >= 0 )
               continue;   // already in a sequence, so skip it and go to next AllocationSet

            int currAttr = -1;   // this is the current IDU's value for the AllocSet's col
            m_pMapLayer->GetData( idu, pAllocSet->m_col, currAttr );
            
            // look through allocations in this Allocation Set, to try to allocate them
            for ( int j=0; j < (int) pAllocSet->m_allocationArray.GetSize(); j++ )
               {
               Allocation *pAlloc = pAllocSet->m_allocationArray[ j ];

               if ( pAlloc->IsSequence() && pAlloc->m_allocationSoFar < pAlloc->m_currentTarget )
                  {
                  // is it already in a sequence (from a prior Allocation in this AllocationSet)?
                  int _sequenceID = -1;
                  m_pMapLayer->GetData( idu, pAllocSet->m_colSequence, _sequenceID );

                  if ( _sequenceID >= 0 )
                     break;   // already in a sequence, so skip on to the next AllocationSet

                  // next, see if the attribute in the coverage is in the sequence
                  bool inSequence = false;
                  for ( int k=0; k < (int) pAlloc->m_sequenceArray.GetSize(); k++ )
                     {
                     if ( pAlloc->m_sequenceArray[ k ] == currAttr )
                        {
                        inSequence = true;
                        break;
                        }
                     }

                  // skip this allocation if the IDU doesn't fit it.
                  if ( inSequence == false )   
                     continue;   // skip to next Allocation 

                  // second, see if it passes all constraints
                  bool passConstraints = true;
               
                  for ( int k=0; k < (int) pAlloc->m_constraintArray.GetSize(); k++ )
                     {
                     Constraint *pConstraint = pAlloc->m_constraintArray[ k ];
                     bool result = false;
                     bool ok = pConstraint->m_pQuery->Run( idu, result );
                  
                     if ( result == false )
                        {
                        passConstraints = false;
                        break;
                        }               
                     }

                  // did we pass the constraints?  If so, add area and tag coverage
                  if ( passConstraints )
                     {
                     m_pMapLayer->SetData( idu, pAllocSet->m_colSequence, pAlloc->m_id );

                     float area = 0;
                     m_pMapLayer->GetData( idu, m_colArea, area );
                     pAlloc->m_allocationSoFar += area;
                     //break;  // go to next AllocationSet
                     }
                  }  // end of:  if pAlloc->IsSequence() && pAlloc->m_allocationSoFar < pAlloc->m_currentTarget )
               }  // end of: for each Allocation
            }  // end of: if ( pAllocSet->m_inUse )
         }  // end of: for each AllocationSet
      }  // end of: for each IDU

   // report results
   for( int i=0; i < (int) m_allocationSetArray.GetSize(); i++ )
      {
      AllocationSet *pAllocSet = m_allocationSetArray[ i ];

      if ( pAllocSet->m_inUse )
         { 
         // otherwise, look through allocations to try to allocate them
         for ( int j=0; j < (int) pAllocSet->m_allocationArray.GetSize(); j++ )
            {
            Allocation *pAlloc = pAllocSet->m_allocationArray[ j ];
   
            if ( pAlloc->IsSequence() )
               {
               CString msg;
               msg.Format( "Spatial Allocator: Sequence '%s' allocated to %4.1f percent of the area (target = %4.1f)",
                  (LPCTSTR) pAlloc->m_name, pAlloc->m_allocationSoFar * 100 / m_totalArea, pAlloc->m_initPctArea*100 );
   
               pAlloc->m_allocationSoFar = 0;
               pAlloc->m_currentTarget = 0;
   
               Report::LogMsg( msg );
               }
            }
         }
      }

   return true;
   }


void SpatialAllocator::SetAllocationTargets( int currentYear )
   {
   for( int i=0; i < (int) m_allocationSetArray.GetSize(); i++ )
      {
      AllocationSet *pAllocSet = m_allocationSetArray[ i ];

      pAllocSet->SetTarget( currentYear );

      for ( int j=0; j < (int) pAllocSet->m_allocationArray.GetSize(); j++ )
         {
         Allocation *pAlloc = pAllocSet->m_allocationArray[ j ];

         if ( pAllocSet->m_inUse )
            pAlloc->SetTarget( currentYear );
         else
            pAlloc->m_currentTarget = 0;
         }
      }
   }


void SpatialAllocator::ScoreIduAllocations( EnvContext *pContext, bool useAddDelta )
   {
   int iduCount = m_pMapLayer->GetRecordCount();

   /////////////////
   int scoreCount = 0;
   /////////////////

   //int iduCount = OrderIDUs();
   //
   //if ( m_shuffleIDUs )   // randomize rank ordering for a given score?  then shuffle the IDU order
   //   ::ShuffleArray< UINT >( m_iduArray.GetData(), iduCount, m_pRandUnif );
   
   // first, zero out allocation scores.  They will get calculated in the next section 
   for( int i=0; i < (int) m_allocationSetArray.GetSize(); i++ )
      {
      AllocationSet *pAllocSet = m_allocationSetArray[ i ];
      pAllocSet->m_allocationSoFar = 0;
      pAllocSet->OrderIDUs( m_iduArray, m_pRandUnif );
      
      if ( pAllocSet->m_inUse )
         {
         for ( int j=0; j < (int) pAllocSet->m_allocationArray.GetSize(); j++ )
            {
            Allocation *pAlloc = pAllocSet->m_allocationArray[ j ];
            pAlloc->m_allocationSoFar = 0;
            pAlloc->m_constraintAreaSatisfied = 0;
            pAlloc->m_minScore   = (float) LONG_MAX;     // min and max score for this allocation
            pAlloc->m_maxScore   = (float) -LONG_MAX;
            pAlloc->m_scoreCount = 0;     // number of scores greater than 0 at any given time
            pAlloc->m_usedCount  = 0;     // number of scores that are actually applied at any given time
            pAlloc->m_scoreArea  = 0;     // area of scores greater than 0 at any given time
            pAlloc->m_scoreBasis  = 0;    // target values of scores greater than 0 at any given time
            pAlloc->m_expandArea  = 0;    // target values of scores greater than 0 at any given time
            pAlloc->m_expandBasis = 0;    // target values of scores greater than 0 at any given time

            for ( int k=0; k < iduCount; k++ )
               {
               pAlloc->m_iduScoreArray[ k ].idu = m_iduArray[ k ];
               pAlloc->m_iduScoreArray[ k ].score = this->m_scoreThreshold-1; 
               }
            }
         }
      }

   // next, score each IDU for each allocation
   // we iterate as follows:
   //  for each IDU...
   //     for each AllocationSet
   //         for each Allocation
   //             apply constraints.  If idu passes...
   //                 evaluate score via preference expressions
   //                 write to the IDU if a column specified for the Allocation

   
   //for ( MapLayer::Iterator idu=m_pMapLayer->Begin(); idu < m_pMapLayer->End(); idu++ )
   for ( int m=0; m < iduCount; m++ )
      {
      int idu = m_iduArray[ m ];

      m_pQueryEngine->SetCurrentRecord( idu );
      m_pMapExprEngine->SetCurrentRecord( idu );

      for( int i=0; i < (int) m_allocationSetArray.GetSize(); i++ )
         {
         AllocationSet *pAllocSet = m_allocationSetArray[ i ];
      
         if ( pAllocSet->m_inUse )
            {
            // iterate through the allocation set's allocations
            for ( int j=0; j < (int) pAllocSet->m_allocationArray.GetSize(); j++ )
               {
               Allocation *pAlloc = pAllocSet->m_allocationArray[ j ];

               bool passConstraints = true;
               // first, apply any constraints
               for ( int k=0; k < (int) pAlloc->m_constraintArray.GetSize(); k++ )
                  {
                  Constraint *pConstraint = pAlloc->m_constraintArray[ k ];

                  if ( pConstraint->m_pQuery == NULL )   // skip this constraint
                     {
                     passConstraints = false;
                     continue;
                     }

                  bool result = false;
                  bool ok = pConstraint->m_pQuery->Run( idu, result );
                  
                  if ( result == false )
                     {
                     passConstraints = false;
                     break;
                     }               
                  }
   
               // did we pass the constraints?  If so, evaluate preferences
               if ( passConstraints )
                  {
                  float area = 0;
                  m_pMapLayer->GetData( idu, m_colArea, area );
                  pAlloc->m_constraintAreaSatisfied += area;

                  float score = 0;
                  for ( int k=0; k < (int) pAlloc->m_prefsArray.GetSize(); k++ )
                     {
                     Preference *pPref = pAlloc->m_prefsArray[ k ];
                     float value = 0;

                     bool passedQuery = true;
                     if ( pPref->m_pQuery != NULL ) // is the preference associated with a query?
                        {
                        bool result = false;
                        bool ok = pPref->m_pQuery->Run( idu, result );
            
                        if ( !ok || ! result )
                           passedQuery = false;
                        }

                     if ( passedQuery )
                        {
                        if ( pPref->m_pMapExpr )   // is the preference an expression that reference fields in the IDU coverage?
                           {
                           bool ok = m_pMapExprEngine->EvaluateExpr( pPref->m_pMapExpr, false ); // pPref->m_pQuery ? true : false );
                           if ( ok )
                              value += (float) pPref->m_pMapExpr->GetValue();
                           }
                        else
                           value = pPref->m_value;
                        }
                     
                     score += value;      // accumulate preference scores
                     }  // end of: for each preference in this allocation
   
                  pAlloc->m_iduScoreArray[ m ].idu   = idu;
                  pAlloc->m_iduScoreArray[ m ].score = score;

                  // update min/max scores if needed
                  if ( score < pAlloc->m_minScore )
                     pAlloc->m_minScore = score;

                  if ( score > pAlloc->m_maxScore )
                     pAlloc->m_maxScore = score;

                  // if score is valid, update associated stats for this allocation
                  if ( score >= m_scoreThreshold )
                     {
                     /////////////
                     if ( i==0 && j == 0 ) scoreCount++;
                     //////////////

                     pAlloc->m_scoreCount++;                    
                     pAlloc->m_scoreArea += area;

                     // target value
                     float targetValue = 0;

                     int targetCol = -1;
                     if ( pAllocSet->m_colTargetBasis >= 0  )
                        targetCol = pAllocSet->m_colTargetBasis;
                     else
                        targetCol = pAlloc->m_colTargetBasis;

                     m_pMapLayer->GetData( idu, targetCol, targetValue );
                     pAlloc->m_scoreBasis += targetValue;
                     }   
                  }  // end of: if (passedConstraints)
               else
                  {  // failed constraint
                  pAlloc->m_iduScoreArray[ idu ].score = m_scoreThreshold-1;    // indicate we failed a constraint
                  }   

               // if field defined for this allocation, populate it with the score
               if ( pAlloc->m_colScores >= 0 )
                  {
                  float newScore = pAlloc->m_iduScoreArray[ idu ].score;                  
                  UpdateIDU( pContext, idu, pAlloc->m_colScores, newScore, useAddDelta );
                  }

               }  // end of: for ( each allocation )
            }  // end of: if ( pAllocSet->m_inUse )
         }  // end of: for ( each allocationSet )
      }  // end of:  for ( each IDU )

      ////////
      //AllocationSet *pAS = this->m_allocationSetArray[ 0 ];
      //Allocation *pAlloc = pAS->GetAllocation( 0 );
      //
      //FILE *fp = fopen( "\\Envision\\StudyAreas\\CentralOregon\\Test.csv", "wt" );
      //float maxScore = -2;
      //for ( int i=0; i < iduCount; i++ )
      //   {
      //   if ( pAlloc->m_iduScoreArray[i].score > maxScore )
      //      maxScore = pAlloc->m_iduScoreArray[i].score;
      //
      //   fprintf( fp, "%.1f\n", pAlloc->m_iduScoreArray[ i ].score );
      //   }
      //fclose( fp );
      //         /////////
      //CString msg;
      //msg.Format( "MAXSCORE A: %.1f, scoreCount=%i (%i)", maxScore, scoreCount, pAlloc->m_scoreCount );
      //Report::LogMsg( msg, RT_ERROR );
      ///////

   return;
   }


void SpatialAllocator::AllocBestWins( EnvContext *pContext, bool useAddDelta )
   {
   for ( MapLayer::Iterator idu=m_pMapLayer->Begin(); idu < m_pMapLayer->End(); idu++ )
      {
      for( int i=0; i < (int) m_allocationSetArray.GetSize(); i++ )
         {
         AllocationSet *pAllocSet = m_allocationSetArray[ i ];

         if ( pAllocSet->m_inUse )
            {
            int bestIndex = -1;
            float bestScore = 0;
         
            // find the "best" allocation
            for ( int j=0; j < (int) pAllocSet->m_allocationArray.GetSize(); j++ )
               {
               Allocation *pAlloc = pAllocSet->m_allocationArray[ j ];

               float allocationSoFar = 0;
               float currentTarget = 0;
               if ( pAlloc->m_targetSource == TS_USEALLOCATIONSET )
                  {
                  allocationSoFar = pAllocSet->m_allocationSoFar;
                  currentTarget = pAllocSet->m_currentTarget;
                  }
               else
                  {
                  allocationSoFar = pAlloc->m_allocationSoFar;
                  currentTarget = pAlloc->m_currentTarget;
                  }
                  
               // is this one already fully allocated?  Then skip it
               if ( allocationSoFar >= currentTarget )
                  continue;
               
               if ( pAlloc->m_iduScoreArray[ idu ].score > bestScore )
                  {
                  bestIndex = j;
                  bestScore = pAlloc->m_iduScoreArray[ idu ].score;
                  }            
               }  // end of: for ( each allocation )
   
            // done assessing best choice, now allocate it
            if ( bestIndex >= 0 )
               {
               Allocation *pAlloc = pAllocSet->m_allocationArray[ bestIndex ];
               
               if ( useAddDelta )
                  {
                  int currAttrCode = 0;
       
                  m_pMapLayer->GetData( idu, pAllocSet->m_col, currAttrCode );
   
                  if  ( currAttrCode != pAlloc->m_id )
                     AddDelta( pContext, idu, pAllocSet->m_col, pAlloc->m_id );
                  }
               else
                  m_pMapLayer->SetData( idu, pAllocSet->m_col, pAlloc->m_id );
               
               float basis = 0;
               m_pMapLayer->GetData( idu, pAlloc->m_colTargetBasis, basis );

               pAlloc->m_allocationSoFar += basis;

               if ( pAlloc->GetTargetClass() == TS_USEALLOCATIONSET )
                  pAllocSet->m_allocationSoFar += basis;
               }
            }  // end of: if ( pAllocSet->m_inUse )
         }  // end of: for ( each allocationSet )
      }  // end of: for ( each idu );

   return;
   }


void SpatialAllocator::AllocScorePriority( EnvContext *pContext, bool useAddDelta )
   {
   // this gets called once per time step
   int iduCount = m_pMapLayer->GetRecordCount();

      
   ////////
   //AllocationSet *pAS = this->m_allocationSetArray[ 0 ];
   //Allocation *pAl = pAS->GetAllocation( 0 );
   //
   //QueryEngine qe( (MapLayer*) pContext->pMapLayer );
   //Query *pQuery = qe.ParseQuery( pAl->m_constraintArray[0]->m_queryStr, 0, "");
   //
   //float maxScore = -2;
   //float constraintArea = 0;
   //for ( int i=0; i < iduCount; i++ )
   //   {
   //   if ( pAl->m_iduScoreArray[i].score > maxScore )
   //      maxScore = pAl->m_iduScoreArray[i].score;
   //
   //   bool result = false;
   //   pQuery->Run( i, result );
   //
   //   if ( result )
   //      {
   //      float area = 0;
   //      pContext->pMapLayer->GetData( i, m_colArea, area );
   //      constraintArea += area;
   //      }
   //   }
   //CString msg;
   //msg.Format( "MAXSCORE start of asp: %.1f, count=%i, used=%i, constraintArea=%i", maxScore, pAl->m_scoreCount, pAl->m_usedCount, (int) constraintArea );
   //Report::LogMsg( msg, RT_ERROR );
   ///////





   //BitArray isIduOpen( iduCount, true ); 
   CArray< bool > isIduOpen;           // this contains a flag indicating whether a given IDU has been allocated yet
   isIduOpen.SetSize( iduCount );      // this array is never shuffled, always in IDU order

   // first, for each allocation, sort it's score list
   for ( int i=0; i < (int) m_allocationSetArray.GetSize(); i++ )
      {
      // get an allocation set
      AllocationSet *pAllocSet = m_allocationSetArray[ i ];

      if ( pAllocSet->m_inUse )
         {
         int terminateFlag = 0;     // 0=not terminated, 1=nothing left to allocate, 

         // start by initializing openIDU array for this allocation set
         for ( int k=0; k < iduCount; k++ )
            isIduOpen[ k ] = true; 
         
         int allocCount = (int) pAllocSet->m_allocationArray.GetSize();
   
         // for each allocation in the set, sort the score array
         for ( int j=0; j < allocCount; j++ )
            {
            Allocation *pAlloc = pAllocSet->m_allocationArray[ j ];   // an Allocation defines scores for a particular alloc tpe (e.g. "Corn")
   
            // sort it, so the front of the list is the best scoring 
            qsort( pAlloc->m_iduScoreArray.GetData(), iduCount, sizeof( IDU_SCORE ), CompareScores );
          
            //////////////////////////////
            // dump the allocations
            //int colLulcA = m_pMapLayer->GetFieldCol( "LULC_A" );
            //for ( int l=0; l < 10; l++ )
            //   {
            //   int lulc = 0;
            //   m_pMapLayer->GetData( pAlloc->m_iduScoreArray[ l ].idu, colLulcA, lulc );
            //
            //   CString msg;
            //   msg.Format( "Score: %6.2f  IDU: %i   LULC_A: %i\n", pAlloc->m_iduScoreArray[ l ].score, pAlloc->m_iduScoreArray[ l ].idu, lulc ); //  );
            //   TRACE( msg );
            //   }

            //for ( int l=0; l < 20; l++ )
            //   {
            //   CString msg;
            //   msg.Format( "Score: %6.2f  IDU: %i\n", pAlloc->m_iduScoreArray[ l ].score, pAlloc->m_iduScoreArray[ l ].idu ); //  );
            //   TRACE( msg );
            //   }


            //////////////////////////////////////

            // and initialize runtime monitor variables
            pAlloc->m_currentIduScoreIndex = 0;  // this indicates we haven't pulled of this list yet
            }
  
         // now, cycle through lists, tracking best score, until everything is allocated
         bool stillAllocating = true;
         float lastBestScore = (float) LONG_MAX;
   
         while ( stillAllocating )    // start looping throught the allocation lists until they are fully allocated
            {
            // get the Allocation with the best idu score by looking through the allocation score arrays
            // Basic idea is that each allocation has a sorted list that, at the front, lists
            // that allocation's best score and corresponding IDU.  All list start out pointing to the 
            // front of the list.  We then start pulling of list entries, keeping track of the current "best"
            // score.  We pop an item of an Allocation's array if the allocation score is 
            // better than any other currently active allocation's score.  An Allocation score is
            // "active" if it has not been fully allocated.
            // Once a best score is identified, the corresponding allocation is applied to the IDU,
            // and the IDU is marked a no longer available.  
            // at the end of each step, the allocation stack are checked to see if any of the top IDUs 
            // are no longer available - if so, those are popped of the list.
   
            Allocation *pBest = NULL;
            float currentBestScore = (float) -LONG_MAX;
   
            for ( int j=0; j < allocCount; j++ )
               {
               Allocation *pAlloc = pAllocSet->m_allocationArray[ j ];
   
               // if this Allocation has already been fully consumed, don't consider it
               if ( pAlloc->m_currentIduScoreIndex >= iduCount )
                  continue;

               // if this Allocation is aleady fully allocated, don't consider it further
               if ( pAlloc->m_targetSource != TS_USEALLOCATIONSET 
                 && pAlloc->m_allocationSoFar >= pAlloc->m_currentTarget )
                  continue;
   
               // if the IDU that represents the current best score for this Allocation is 
               // not available (already allocated), don't consider it
               int currentBestIDU = pAlloc->GetCurrentIdu();
               ASSERT( currentBestIDU >= 0 );
   
               if ( isIduOpen[ currentBestIDU ] == false )
                  continue;
   
               // if this Allocation's current best score <= the current best, don't consider it,
               // some other allocation has a higher score
               float currentScore = pAlloc->GetCurrentScore();

               if ( currentScore <= currentBestScore )
                  {
                  //TRACE3( "SA: Allocation %s - this allocations current best score (%f) is less than the current best score (%f)\n", 
                  //   (LPCTSTR) pAlloc->m_name, pAlloc->m_iduScoreArray[ pAlloc->m_currentIduScoreIndex ].score, currentBestScore );
                  continue;
                  }
                  
               if ( currentScore < this->m_scoreThreshold )  // are we into the failed scores? (didn't pass the constraints?)
                  continue;

               // it passes, so this allocation becomes the leading candidate so far
               pBest = pAlloc;
               currentBestScore = currentScore;
               }  // end of: for each Allocation
   
            if ( pBest == NULL )
               {
               terminateFlag = 1; 
               break;      // nothing left to allocate
               }

            // at this point, we've ID'd the best next score and associated Allocation.  Next, allocate it
            int idu = pBest->GetCurrentIdu();
            isIduOpen[ idu ] = false;
   
            lastBestScore = pBest->GetCurrentScore();
            
            // apply the allocation to the IDUs 
            int currAttrCode = -1;
            m_pMapLayer->GetData( idu, pAllocSet->m_col, currAttrCode );
            
            int newAttrCode = pBest->m_id;    // this is the value to write to the database col
            int newSequenceID = -1;
            int currSequenceID = -1;
   
            // if this is a sequence, figure out where we are in the sequence
            if ( pBest->IsSequence() )
               {
               m_pMapLayer->GetData( idu, pAllocSet->m_colSequence, currSequenceID );
   
               int seqIndex = 0;
               if ( currSequenceID == pBest->m_id )   // are we already in the sequence?
                  {
                  BOOL found = pBest->m_sequenceMap.Lookup( currAttrCode, seqIndex );
                  ASSERT( found );
   
                  seqIndex++;    // move to the next item in the sequence, wrapping if necessary
                  if ( seqIndex >= (int) pBest->m_sequenceArray.GetSize() )
                     seqIndex = 0;
   
                  newAttrCode = pBest->m_sequenceArray[ seqIndex ];
                  newSequenceID = pBest->m_id;
                  }
               else  // starting a new sequence
                  {
                  newAttrCode = pBest->m_sequenceArray[ 0 ];
                  newSequenceID = pBest->m_id;
                  }
               }
   
            // ok, new attrCode and sequenceID determined, write to the IDU coverage
            this->UpdateIDU( pContext, idu, pAllocSet->m_col, newAttrCode, useAddDelta );
            
            if ( pAllocSet->m_colSequence >= 0 && currSequenceID != newSequenceID )
               this->UpdateIDU( pContext, idu, pAllocSet->m_colSequence, newSequenceID, useAddDelta );

            // update internals with allocation basis value
            float basis = 0;
            m_pMapLayer->GetData( idu, pBest->m_colTargetBasis, basis );

            pBest->m_allocationSoFar += basis;   // accumulate the basis target of this allocation
            pBest->m_usedCount++;

            if ( pBest->m_targetSource == TS_USEALLOCATIONSET )
               pAllocSet->m_allocationSoFar += basis;
                        
            // Expand if needed
            if ( pBest->m_useExpand )
               {
               float expandArea = 0;
               float expandAreaBasis = Expand( idu, pBest, useAddDelta, currAttrCode, newAttrCode, currSequenceID, newSequenceID, pContext, isIduOpen, expandArea );

               pBest->m_allocationSoFar += expandAreaBasis;   // accumulate the basis target of this allocation
               pBest-> m_expandBasis    += expandAreaBasis;   
               pBest->m_expandArea      += expandArea;

               if ( pBest->m_targetSource == TS_USEALLOCATIONSET )
                  pAllocSet->m_allocationSoFar += expandAreaBasis;
               }

            pBest->m_currentIduScoreIndex++;    // pop of the front of the list for the best allocation
   
            // are we done yet?
            stillAllocating = false;
   
            for ( int j=0; j < allocCount; j++ )
               {
               // we look at the allocationSet first, to see if the target is defined there.

               // is the allocation global?
               Allocation *pAlloc = pAllocSet->m_allocationArray[ j ];

               float currentTarget = 0;
               float allocationSoFar = 0;
               if ( pAlloc->m_targetSource == TS_USEALLOCATIONSET )
                  {
                  currentTarget = pAllocSet->m_currentTarget;
                  allocationSoFar = pAllocSet->m_allocationSoFar;
                  }
               else
                  {
                  currentTarget = pAlloc->m_currentTarget;
                  allocationSoFar = pAlloc->m_allocationSoFar;
                  }
   
               if ( allocationSoFar < currentTarget )
                  {
                  // move past any orphaned IDU_SCORE entries
                  int idu = pAlloc->GetCurrentIdu();
                  if ( idu >= iduCount || idu < 0 )
                        break;

                  bool isOpen = isIduOpen[ idu ];
      
                  while ( isOpen == false )     // loop through "closed" idus to find the first "open" (available) idu)
                     {
                     pAlloc->m_currentIduScoreIndex++;
      
                     if ( pAlloc->m_currentIduScoreIndex >= iduCount )
                        break;
      
                     idu = pAlloc->GetCurrentIdu();

                     if ( idu >= iduCount || idu < 0 )
                        break;

                     isOpen = isIduOpen[ idu ];
                     }
                  }
               }  // end of: for ( j < allocationCount )
   
            // check to see if allocations still needed
            for ( int j=0; j < allocCount; j++ )
               {
               Allocation *pAlloc = pAllocSet->m_allocationArray[ j ];

               float currentTarget = 0;
               float allocationSoFar = 0;
               if ( pAlloc->m_targetSource == TS_USEALLOCATIONSET )
                  {
                  currentTarget = pAllocSet->m_currentTarget;
                  allocationSoFar = pAllocSet->m_allocationSoFar;
                  }
               else
                  {
                  currentTarget = pAlloc->m_currentTarget;
                  allocationSoFar = pAlloc->m_allocationSoFar;
                  }
   
               if ( ( allocationSoFar < currentTarget )  // not fully allocated already?
                 && ( pAlloc->m_currentIduScoreIndex < iduCount ) )          // and still on the list?
                  {
                  stillAllocating = true;
                  break;
                  }
               }
            }  // end of:  while ( stillAllocating )

         // dump some results

         // temporary!
         //int outcount = 0;
         //if ( i == 0 )     // first allocationset only
         //   {
         //   Allocation *pAlloc = pAllocSet->GetAllocation( 0 );
         //   for ( int j=0; j < iduCount; j++ )
         //      {
         //      //if ( pAlloc->m_iduScoreArray[j].score >= this->m_scoreThreshold )
         //      //   {
         //         CString msg;
         //         msg.Format( "Score: %6.2f  Rank: %i\n", pAlloc->m_iduScoreArray[j].score, j );
         //         Report::LogMsg( msg, RT_ERROR );
         //         outcount++;
         //      //   }
         //
         //      if ( outcount > 20 )
         //         break;
         //      }
         //
         //   CString msg;
         //   msg.Format( "Current Score Rank (%s) = %i, UsedCount=%i\n", (PCTSTR) pAlloc->m_name, pAlloc->m_currentIduScoreIndex, pAlloc->m_usedCount );
         //   Report::LogMsg( msg, RT_ERROR );               
         //   }
         }     // end of:  if ( pAllocSet->m_inUse )
      }  // end of: for ( each AllocationSet )


  ////////
   //pAS = this->m_allocationSetArray[ 0 ];
   //pAl = pAS->GetAllocation( 0 );
   //
   //maxScore = -2;
   //for ( int i=0; i < iduCount; i++ )
   //   {
   //   if ( pAl->m_iduScoreArray[i].score > maxScore )
   //      maxScore = pAl->m_iduScoreArray[i].score;
   //   }
   //         /////////
   //msg.Format( "MAXSCORE end   of asp: %.1f, count=%i, used=%i", maxScore, pAl->m_scoreCount, pAl->m_usedCount );
   //Report::LogMsg( msg, RT_ERROR );
   ///////




   }

   
bool SpatialAllocator::LoadXml( LPCTSTR filename )
   {
   // search for file along path
   CString path;
   int result = PathManager::FindPath( filename, path );  // finds first full path that contains passed in path if relative or filename, return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found 

   if ( result < 0 )
      {
      CString msg( "Spatial Allocator: Unable to find input file " );
      msg += filename;
      Report::ErrorMsg( msg );
      return false;
      }

   TiXmlDocument doc;
   bool ok = doc.LoadFile( path );

   if ( ! ok )
      {      
      Report::ErrorMsg( doc.ErrorDesc() );
      return false;
      }

   CString msg( "Spatial Allocator: Loading input file " );
   msg += path;
   Report::LogMsg( msg );
 
   // start interating through the nodes
   TiXmlElement *pXmlRoot = doc.RootElement();  // spatial_allocator

   LPTSTR areaCol = NULL; //, scoreCol=NULL;
   LPTSTR method  = NULL;
   int    shuffleIDUs = 1;
   XML_ATTR rattrs[] = { // attr          type           address       isReq checkCol
                      { "area_col",     TYPE_STRING,   &areaCol,       false, CC_MUST_EXIST | TYPE_FLOAT },
                      //{ "score_col",    TYPE_STRING,   &scoreCol,      false, CC_AUTOADD    | TYPE_FLOAT },
                      { "method",       TYPE_STRING,   &method,        false, 0 },
                      { "shuffle_idus", TYPE_INT,      &shuffleIDUs,   false, 0 },
                      { NULL,           TYPE_NULL,     NULL,           false, 0 } };

   if ( TiXmlGetAttributes( pXmlRoot, rattrs, path, m_pMapLayer ) == false )
      return false;
   
   if ( areaCol == NULL )
      areaCol = "AREA";

   this->m_colArea = m_pMapLayer->GetFieldCol( areaCol );
   if ( m_colArea < 0 )
      {
      CString msg( "Spatial Allocator: unable to find AREA field in input file" );
      msg += filename;
      Report::ErrorMsg( msg );
      return false;
      }

   //if ( scoreCol )
   //   m_colScore = m_pMapLayer->GetFieldCol( scoreCol );


   ALLOC_METHOD am = AM_SCORE_PRIORITY;
   if ( method != NULL )
      {
      switch( method[ 0 ] )
         {
         case 's':       // score priority - already the default
            break;

         case 'b':       // best wins
            am = AM_BEST_WINS;
            break;
         }
      }

   this->m_allocMethod = am;

   if ( shuffleIDUs )
      {
      m_shuffleIDUs = true;
      m_pRandUnif = new RandUniform( 0.0, (double) m_pMapLayer->GetRecordCount(), 0 );
      }
   else
      m_shuffleIDUs = false;

   // assume only one allocation set for now
   TiXmlElement *pXmlAllocSet = pXmlRoot->FirstChildElement( "allocation_set" );
   if ( pXmlAllocSet == NULL )
      {
      CString msg( "Spatial Allocator: missing <allocation_set> element in input file " );
      msg += path;
      Report::ErrorMsg( msg );
      return false;
      }

   while ( pXmlAllocSet != NULL )
      {
      // lookup fields
      LPTSTR name     = NULL;
      LPTSTR field    = NULL;
      LPTSTR seqField = NULL;
      LPTSTR targetSource = NULL;
      LPTSTR targetValues = NULL;
      LPTSTR targetBasis  = NULL;
      int use = 1;

      XML_ATTR attrs[] = { // attr          type           address       isReq checkCol
                         { "name",         TYPE_STRING,   &name,         true,  0 },
                         { "use",          TYPE_INT,      &use,          false, 0 },
                         { "col",          TYPE_STRING,   &field,        true,  CC_MUST_EXIST | TYPE_LONG },
                         { "sequence_col", TYPE_STRING,   &seqField,     false, CC_AUTOADD | TYPE_LONG },
                         // these are optional, only specify if using global (meaning allocation_set) targets
                         { "target_type",   TYPE_STRING,   &targetSource, false,  0 },
                         { "target_basis",  TYPE_STRING,   &targetBasis,  false,  0 },
                         { "target_values", TYPE_STRING,   &targetValues, false,  0 },

                         { NULL,           TYPE_NULL,     NULL,          false, 0 } };

      if ( TiXmlGetAttributes( pXmlAllocSet, attrs, path, m_pMapLayer ) == false )
         return false;

      AllocationSet *pAllocSet = new AllocationSet( name, field );
      pAllocSet->m_col = m_pMapLayer->GetFieldCol( field );
      pAllocSet->m_inUse = use ? true : false;

      pAllocSet->m_targetSource = TS_USEALLOCATION;
      pAllocSet->SetTargetParams( m_pMapLayer, targetBasis, targetSource, targetValues, NULL, NULL );      

      if ( seqField != NULL )
         {
         pAllocSet->m_seqField = seqField;
         pAllocSet->m_colSequence = m_pMapLayer->GetFieldCol( seqField );
         }
      
      this->m_allocationSetArray.Add( pAllocSet );

      // have allocationset set up, get allocations
      TiXmlElement *pXmlAlloc = pXmlAllocSet->FirstChildElement( "allocation" );
      if ( pXmlAlloc == NULL )
         {
         CString msg( "Spatial Allocator: missing <allocation> element in input file - at least one allocation is required..." );
         msg += path;
         Report::ErrorMsg( msg );
         return false;
         }

      while ( pXmlAlloc != NULL )
         {
         // lookup fields
         LPTSTR name   = NULL;
         int attrCode  = -1;
         LPTSTR targetSource = NULL;
         LPTSTR targetValues = NULL;
         LPTSTR targetBasis  = NULL;
         LPTSTR targetDomain = NULL;
         LPTSTR targetQuery  = NULL;
         LPTSTR constraint   = NULL;
         LPCTSTR scoreCol    = NULL;
         LPCTSTR seq   = NULL;

         LPCTSTR expandQuery = NULL;
         //float   expandMaxArea  = -1;
         LPCTSTR expandArea = NULL;

         float initPctArea = 0;

         XML_ATTR attrs[] = { // attr          type           address       isReq checkCol
                            { "name",          TYPE_STRING,   &name,         true,  0 },
                            { "id",            TYPE_INT,      &attrCode,     true,  0 },
                            { "target_type",   TYPE_STRING,   &targetSource, false, 0 },
                            { "target_values", TYPE_STRING,   &targetValues, false, 0 },
                            { "target_domain", TYPE_STRING,   &targetDomain, false, 0 },
                            { "target_basis",  TYPE_STRING,   &targetBasis,  false, 0 },
                            { "target_query",  TYPE_STRING,   &targetQuery,  false, 0 },
                            { "constraint",    TYPE_STRING,   &constraint,   false, 0 },
                            { "score_col",     TYPE_STRING,   &scoreCol,     false, CC_AUTOADD | TYPE_FLOAT },
                            { "sequence",      TYPE_STRING,   &seq,          false, 0 },
                            { "init_pct_area", TYPE_FLOAT,    &initPctArea,  false, 0 },
                            { "expand_query",  TYPE_STRING,   &expandQuery,  false, 0 },
                            { "expand_area",   TYPE_STRING,   &expandArea,   false, 0 },
                            { NULL,            TYPE_NULL,     NULL,        false, 0 } };
      
         if ( TiXmlGetAttributes( pXmlAlloc, attrs, path, m_pMapLayer ) == false )
            return false;

         // is a target specified?  Okay if allocation set has targets defined
         bool ok = true;
         if ( targetSource == NULL && pAllocSet->m_targetSource != TS_UNDEFINED && pAllocSet->m_targetSource != TS_USEALLOCATION )
            {
            ok = false;
            CString msg;
            msg.Format( "Spatial Allocator: missing 'target_type' attribute for allocation %s", name );
            Report::ErrorMsg( msg );
            }

         if ( ok )
            {
            Allocation *pAlloc = new Allocation( pAllocSet, name, attrCode ); //, ttype, targetValues );
            pAlloc->m_targetSource = TS_RATE;
            pAlloc->SetTargetParams( m_pMapLayer, targetBasis, targetSource, targetValues, targetDomain, targetQuery );

            // score column (optional)
            if ( scoreCol != NULL )
               {
               pAlloc->m_scoreCol = scoreCol;
               m_pMapLayer->CheckCol( pAlloc->m_colScores, scoreCol, TYPE_FLOAT, CC_AUTOADD );
               }

            if ( seq != NULL ) // is this a sequence?  then parse out attr codes
               {
               TCHAR *buffer = new TCHAR[ lstrlen( seq ) + 1 ];
               lstrcpy( buffer, seq );
               TCHAR *next = NULL;
               TCHAR *token = _tcstok_s( buffer, _T(","), &next );
               while ( token != NULL )
                  {
                  int attrCode = atoi( token );
                  int index = (int) pAlloc->m_sequenceArray.Add( attrCode );
                  pAlloc->m_sequenceMap.SetAt( attrCode, index );
                  token = _tcstok_s( NULL, _T( "," ), &next );
                  }
               delete [] buffer;

               pAlloc->m_initPctArea = initPctArea;
               }

            // constraint
            if ( constraint != NULL )
               {
               Constraint *pConstraint = new Constraint( "_default", constraint );
               pAlloc->m_constraintArray.Add( pConstraint );
               }
            
            if ( expandQuery != NULL )
               {
               if ( *expandQuery == '@' )
                  expandQuery = constraint;

               Query *pQuery = theProcess->m_pQueryEngine->ParseQuery( expandQuery, 0, "" );

               if ( pQuery == NULL )
                  {
                  CString msg( "Spatial Allocator: Unable to parse expand query '" );
                  msg += expandQuery;
                  msg += "' - it will be ignored";
                  Report::WarningMsg( msg );
                  }
               else
                  {
                  pAlloc->m_expandQueryStr = expandQuery;
                  pAlloc->m_pExpandQuery = pQuery;
                  pAlloc->m_useExpand = true;
                  }
               }

            // was a string provided describing the max area expansion?  If so, parse it.
            if ( expandArea != NULL )
               {
               pAlloc->m_expandAreaStr = expandArea;

               // legal possibiliites include:  a constant number, uniform(lb,ub), normal( mean, variance)
               bool success = true;
               switch( expandArea[ 0 ] )
                  {
                  case 'u':      // uniform distribution
                  case 'U':
                  case 'n':      // normal distribution
                  case 'N':
                  case 'w':      // weibull distribution
                  case 'W':
                  case 'l':      // lognormal distribution
                  case 'L':
                  case 'e':      // lognormal distribution
                  case 'E':
                     {
                     TCHAR buffer[ 64 ];
                     lstrcpy( buffer, expandArea );
                     TCHAR *token = _tcschr( buffer, '(' );
                     if ( token == NULL ) { success = false; break; }

                     token++;    // point to first param (mean)
                     pAlloc->m_expandAreaParam0 = atof( token );

                     if ( expandArea[ 0 ] != 'e' && expandArea[ 0 ] != 'E' )
                        {
                        token = _tcschr( buffer, ',' );
                        if ( token == NULL ) { success = false; break; }
                        pAlloc->m_expandAreaParam1 = atof( token+1 );
                        }

                     switch( expandArea[ 0 ] )
                        {
                        case 'u':      // uniform distribution
                        case 'U':
                           pAlloc->m_expandType = ET_UNIFORM;
                           pAlloc->m_pRand = new RandUniform( pAlloc->m_expandAreaParam0, pAlloc->m_expandAreaParam1, 0 );
                           break;

                        case 'n':      // normal distribution
                        case 'N':
                           pAlloc->m_expandType = ET_NORMAL;
                           pAlloc->m_pRand = new RandNormal( pAlloc->m_expandAreaParam0, pAlloc->m_expandAreaParam1, 0 );
                           break;

                        case 'w':      // weibull distribution
                        case 'W':
                           pAlloc->m_expandType = ET_WEIBULL;
                           pAlloc->m_pRand = new RandWeibull( pAlloc->m_expandAreaParam0, pAlloc->m_expandAreaParam1 );
                           break;

                        case 'l':      // lognormal distribution
                        case 'L':
                           pAlloc->m_expandType = ET_LOGNORMAL;
                           pAlloc->m_pRand = new RandLogNormal( pAlloc->m_expandAreaParam0, pAlloc->m_expandAreaParam1, 0 );
                           break;

                        case 'e':      // exponential distribution
                        case 'E':
                           pAlloc->m_expandType = ET_EXPONENTIAL;
                           pAlloc->m_pRand = new RandExponential( pAlloc->m_expandAreaParam0 );
                           break;
                        }

                     pAlloc->m_useExpand  = true;
                     }
                     break;

                  case '0':      // it is a number
                  case '1':
                  case '2':
                  case '3':
                  case '4':
                  case '5':
                  case '6':
                  case '7':
                  case '8':
                  case '9':
                     pAlloc->m_expandAreaParam0 = pAlloc->m_expandMaxArea = (float) atof( expandArea );
                     pAlloc->m_useExpand = true;
                     break;

                  default:
                     success = false;
                  }

               if ( success == false )
                  {
                  CString msg( "Spatial Allocator: unrecognized expand area specification: " );
                  msg += expandArea;
                  Report::ErrorMsg( msg );
                  pAlloc->m_useExpand = false;
                  }
               }  // end of: if ( expandArea != NULL )
                  
            pAllocSet->m_allocationArray.Add( pAlloc );

            // next, get any preferences
            TiXmlElement *pXmlPref = pXmlAlloc->FirstChildElement( "preference" );
      
            while ( pXmlPref != NULL )
               {
               // lookup fields
               LPTSTR name   = NULL;
               LPTSTR query  = NULL;
               LPTSTR wtExpr = NULL;
      
               XML_ATTR attrs[] = { // attr          type           address   isReq checkCol
                                  { "name",          TYPE_STRING,   &name,    true,  0 },
                                  { "query",         TYPE_STRING,   &query,   true,  0 },
                                  { "weight",        TYPE_STRING,   &wtExpr,  true,  0 },
                                  { NULL,            TYPE_NULL,     NULL,          false, 0 } };
            
               if ( TiXmlGetAttributes( pXmlPref, attrs, path ) == false )
                  return false;
               
               Preference *pPref = new Preference( pAlloc, name, query, wtExpr );
               pAlloc->m_prefsArray.Add( pPref );

               pXmlPref = pXmlPref->NextSiblingElement( "preference" );
               }

            // next, get any preferences  NOTE: THIS HAS BEEN DEPRECATED
            TiXmlElement *pXmlConstraint = pXmlAlloc->FirstChildElement( "constraint" );
            //if ( pXmlConstraint == NULL )
            //   {
            //   CString msg( "Spatial Allocator: missing <preference> element in input file - at least one prefernec is required for each allocation..." );
            //   msg += path;
            //   Report::ErrorMsg( msg );
            //   return false;
            //   }
      
            while ( pXmlConstraint != NULL )
               {
               // lookup fields
               LPTSTR name  = NULL;
               LPTSTR query = NULL;
      
               XML_ATTR attrs[] = { // attr          type           address   isReq checkCol
                                  { "name",          TYPE_STRING,   &name,    true,  0 },
                                  { "query",         TYPE_STRING,   &query,   true,  0 },
                                  { NULL,            TYPE_NULL,     NULL,     false, 0 } };
            
               if ( TiXmlGetAttributes( pXmlConstraint, attrs, path ) == false )
                  return false;
               
               Constraint *pConstraint = new Constraint( name, query );
               pAlloc->m_constraintArray.Add( pConstraint );

               pXmlConstraint = pXmlConstraint->NextSiblingElement( "constraint" );
               }
            }  // end of: if ( ok )

         pXmlAlloc = pXmlAlloc->NextSiblingElement( "allocation" );
         }

      pXmlAllocSet = pXmlAllocSet->NextSiblingElement( "allocation_set" );
      }

   return true;
   }



float SpatialAllocator::Expand( int idu, Allocation *pAlloc, bool useAddDelta, int currAttrCode, int newAttrCode, 
            int currSequenceID, int newSequenceID, EnvContext *pContext, CArray< bool > &isIduOpen, float &expandArea )
   {   
   // expand the allocation to the area around the idu provided
   // returns the basis value associated with the expand area.
   ASSERT( this->m_pMapLayer != NULL );
   MapLayer *pLayer = this->m_pMapLayer;

   if ( m_colArea < 0 )
      m_colArea = pLayer->GetFieldCol( _T( "AREA" ) );

   // basic idea - expand in concentric circles aroud the idu (subject to constraints)
   // until necessary area is found.  We do this keeping track of candidate expansion IDUs
   // in m_expansionArray, starting with the "nucleus" IDU and iteratively expanding outward.
   // for the expansion array, we track the index in the expansion array of location of
   // the lastExpandedIndex and the number of unprocessed IDUs that are expandable (poolSize).
   int neighbors[ 64 ];
   bool addToPool = false;
   INT_PTR lastExpandedIndex = 0;
   int poolSize = 1;       // number of unprocessed IDUs that are expandable

   pAlloc->SetMaxExpandArea();

   // note: values in m_expansionArray are -(index+1)
   m_expansionIDUs.RemoveAll();
   m_expansionIDUs.Add( idu );  // the first one (nucleus) is added whether it passes the query or not
                                // note: this means the nucleus IDU gets treated the same as any
                                // added IDU's passing the test, meaning the expand outcome gets applied
                                // to the nucleus as well as the neighbors.  However, the nucleus
                                // idu is NOT included in the "areaSoFar" calculation, and so does not
                                // contribute to the area count to the max expansion area
   float areaSoFar = 0;
    
   // We collect all the idus that are next to the idus already processed that we haven't seen already
   while( poolSize > 0 && ( pAlloc->m_expandMaxArea <= 0 || areaSoFar < pAlloc->m_expandMaxArea ) )
      {
      INT_PTR countSoFar = m_expansionIDUs.GetSize();

      // iterate from the last expanded IDU on the list to the end of the current list,
      // adding any neighbor IDU's that satisfy the expansion criteria.
      for ( INT_PTR i=lastExpandedIndex; i < countSoFar; i++ )
         {
         int nextIDU = m_expansionIDUs[ i ];

         if ( nextIDU >= 0 )   // is this an expandable IDU?
            {
            Poly *pPoly = pLayer->GetPolygon( nextIDU );  // -1 );    // why -1?
            int count = pLayer->GetNearbyPolys( pPoly, neighbors, NULL, 64, 1 );

            for ( int i=0; i < count; i++ )
               {
               // see if this is an elgible IDU (if so, addToPool will be set to true and the IDU area will be returned. 
               float area = AddIDU( neighbors[ i ], pAlloc, addToPool, areaSoFar, pAlloc->m_expandMaxArea ); 

               if ( addToPool )  // AddIDU() added on to the expansion array AND it is an expandable IDU?
                  {
                  poolSize++;
                  areaSoFar += area;
                  ASSERT( pAlloc->m_expandMaxArea <= 0 || areaSoFar <= pAlloc->m_expandMaxArea );
                  // goto applyOutcome;
                  }
               }

            poolSize--;    // remove the "i"th idu just processed from the pool of unexpanded IDUs 
                           // (if it was in the pool to start with)
            }  // end of: if ( nextIDU > 0 ) 
         }  // end of: for ( i < countSoFar ) - iterated through last round of unexpanded IDUs

      lastExpandedIndex = countSoFar;
      }

   // at this point, the expansion area has been determined (it's stored in the m_expansionIDUs array) 
   // - apply outcomes.  NOTE: this DOES NOT includes the kernal IDU, since it is the first idu in the expansionIDUs array
   float totalBasis = 0;

   for ( INT_PTR i=1; i < m_expansionIDUs.GetSize(); i++ ) // skip kernal IDU
      {
      int expIDU = m_expansionIDUs[ i ];
      if ( expIDU >= 0 )
         {
         // get the basis associated with the expansion IDU being examined
         float basis = 0;
         pLayer->GetData( expIDU, pAlloc->m_colTargetBasis, basis );
         totalBasis += basis;

         isIduOpen[ expIDU ] = false;    // indicate we've allocated this IDU

         if ( useAddDelta )
            {
            // apply the new attribute.
            if ( currAttrCode != newAttrCode )
                AddDelta( pContext, expIDU, pAlloc->m_pAllocSet->m_col, newAttrCode );
           
            // apply the new sequence
            if ( currSequenceID != newSequenceID )
               AddDelta( pContext, expIDU, pAlloc->m_pAllocSet->m_colSequence, newSequenceID );
            }
         else
            {
            if ( currAttrCode != newAttrCode )
               m_pMapLayer->SetData( expIDU, pAlloc->m_pAllocSet->m_col, newAttrCode );

            if ( pAlloc->m_pAllocSet->m_colSequence >= 0 && currSequenceID != newSequenceID )
               m_pMapLayer->SetData( expIDU, pAlloc->m_pAllocSet->m_colSequence, newSequenceID );
            }
         }
      }

   expandArea = areaSoFar;

   return totalBasis;  // note: areaSoFar only includes expansion area, not kernal area
   }


float SpatialAllocator::AddIDU( int idu, Allocation *pAlloc, bool &addToPool, float areaSoFar, float maxArea )
   {
   // have we seen this on already?
   for ( int i=0; i < m_expansionIDUs.GetSize(); i++ )
      {
      int expansionIndex = m_expansionIDUs[ i ];

      if ( ( expansionIndex < 0 && -(expansionIndex+1) == idu )    // seen but not an expandable IDU
        || ( expansionIndex >= 0 && expansionIndex == idu ) )      // seen and is an expandable IDU
         {
         addToPool = false;
         return 0;
         }
      }

   // so, we haven't seen this one before. Is the constraint satisified?
   addToPool = true;

   if ( pAlloc->m_pExpandQuery )
      {
      bool result = false;
      bool ok = pAlloc->m_pExpandQuery->Run( idu, result );
      if ( ! ok || ! result )
         addToPool = false;
      }

   // would this IDU cause the area limit to be exceeded?
   float area = 0;
   m_pMapLayer->GetData( idu, m_colArea, area );

   if ( addToPool && ( maxArea > 0 ) && ( ( areaSoFar + area ) > maxArea ) )
      addToPool = false;

   float allocationSoFar = 0;
   float currentTarget = 0;
   if ( pAlloc->m_targetSource == TS_USEALLOCATIONSET )
      {
      allocationSoFar = pAlloc->m_pAllocSet->m_allocationSoFar;
      currentTarget = pAlloc->m_pAllocSet->m_currentTarget;
      }
   else
      {
      allocationSoFar = pAlloc->m_allocationSoFar;
      currentTarget = pAlloc->m_currentTarget;
      }

   if ( addToPool && ( ( allocationSoFar + areaSoFar + area ) > currentTarget ) )
      addToPool = false;

   if ( addToPool )
      m_expansionIDUs.Add( idu/*+1*/ );
   else
      m_expansionIDUs.Add( -(idu+1) ); // Note: what gets added to the expansion array is -(index+1)
   
   return area;
   }



int SpatialAllocator::CollectExpandStats( void )
   {
   //isIduOpen.SetSize( iduCount );      // this array is never shuffled, always in IDU order

   // first, for each allocation, sort it's score list
   for ( int i=0; i < (int) m_allocationSetArray.GetSize(); i++ )
      {
      // get an allocation set
      AllocationSet *pAllocSet = m_allocationSetArray[ i ];

      if ( pAllocSet->m_inUse )
         {
         int allocCount = (int) pAllocSet->m_allocationArray.GetSize();
   
         // for each allocation in the set, sort the score array
         for ( int j=0; j < allocCount; j++ )
            {
            Allocation *pAlloc = pAllocSet->m_allocationArray[ j ];   // an Allocation defines scores for a particular alloc tpe (e.g. "Corn")
   
            }
         }
      }

   return 0;
   }


void SpatialAllocator::CollectData(EnvContext *pContext)
   {
   //----------------------------------------------
   // output data format
   //
   // For each allocation set that is in use, data is collected
   // AS.
   // if target is defined at the AllocationSet level:
   //    Time
   //    Total Target
   //    Total Realized
   //    for each allocation
   //        Realized
   //        pct Realized
   //        ConstraintsArea
   //        MinScore
   //        MaxScore
   //        ScoreCount
   //        ScoreBasis
   //        ExpandArea
   //        ExpandBasis
   //
   // if targets are defined for each individual allocation:
   //    Time
   //    for each allocation
   //        Target
   //        Realized
   //        pct Realized
   //        ConstraintsArea
   //        MinScore
   //        MaxScore
   //        ScoreCount
   //        ScoreBasis
   //        ExpandArea
   //        ExpandBasis

   // collect output
   for( int i=0; i < (int) m_allocationSetArray.GetSize(); i++ )
      {
      AllocationSet *pAllocSet = m_allocationSetArray[ i ];

      bool isTargetDefined = pAllocSet->IsTargetDefined();

      //if ( pAllocSet->m_inUse )
      //   {
      // collect data (whether in use or not - should be smarter than this!!!
      CString msg( "Spatial Allocator: " );

      CArray< float, float > outputs;
      outputs.Add( (float) pContext->currentYear );  // time

      if ( isTargetDefined )
         {
         outputs.Add( pAllocSet->m_currentTarget );
         outputs.Add( pAllocSet->m_allocationSoFar );
         }

      for ( int j=0; j < pAllocSet->GetAllocationCount(); j++ )
         {
         Allocation *pAlloc = pAllocSet->GetAllocation( j );

         if ( ! isTargetDefined )
            outputs.Add( pAlloc->m_currentTarget );

         //outputs.Add( 100*pAlloc->m_currentTarget/m_totalArea );  ////????
         //outputs.Add( 100*pAlloc->m_allocationSoFar/m_totalArea );////????
         outputs.Add( pAlloc->m_allocationSoFar );    // realized target

         if ( isTargetDefined )
            outputs.Add( pAlloc->m_allocationSoFar*100 / pAllocSet->m_currentTarget );
         else
            outputs.Add( pAlloc->m_allocationSoFar*100 / pAlloc->m_currentTarget );

         outputs.Add( pAlloc->m_constraintAreaSatisfied );

         if ( pAlloc->m_minScore >= (float) LONG_MAX )
            pAlloc->m_minScore = -99;

         if ( pAlloc->m_maxScore <= 0 )
            pAlloc->m_maxScore = -99;  //m_scoreThreshold-1;

         outputs.Add( pAlloc->m_minScore );
         outputs.Add( pAlloc->m_maxScore );
         outputs.Add( (float) pAlloc->m_scoreCount );
         outputs.Add( pAlloc-> m_scoreBasis );

         outputs.Add( pAlloc->m_expandArea );
         outputs.Add( pAlloc-> m_expandBasis );

         //pAlloc->m_cumBasisTarget += pAlloc->m_currentTarget;   ////????
         //pAlloc->m_cumBasisActual       += pAlloc->m_allocationSoFar;  ////????
         //
         //cumOutputs.Add( pAlloc->m_cumBasisTarget );
         //cumOutputs.Add( pAlloc->m_cumBasisActual );
         }

      if ( pContext->yearOfRun >= 0 )
         {
         if ( pAllocSet->m_pOutputData )
            pAllocSet->m_pOutputData->AppendRow( outputs );

         if ( pAllocSet->m_inUse )
            {
            // report resutls for this allocation set
            CString msg;
            if ( isTargetDefined )
               msg.Format("Spatial Allocator [%s]: Time: %i,  weatherYear: %d, Total Target: %.0f,  Realized: %.0f", (LPCTSTR)pAllocSet->m_name, pContext->currentYear, pContext->weatherYear, outputs[1], outputs[2]);
            else
               msg.Format("Spatial Allocator [%s]: Time: %i, weatherYear: %d", (LPCTSTR)pAllocSet->m_name, pContext->currentYear, pContext->weatherYear );

            Report::LogMsg( msg );

            for ( int j=0; j < pAllocSet->GetAllocationCount(); j++ )
               {
               Allocation *pAlloc = pAllocSet->GetAllocation( j );

               if ( isTargetDefined )
                  msg.Format( "  Allocation [%s]: Realized: %.0f,  %% Realized: %.1f,  Constraint Area: %.0f,  Min Score: %.0f,  Max Score: %.0f,  Count: %i (%i used), Score Basis: %.0f, Expand Area: %.0f, Expand Basis: %.0f, currentIndex=%i",
                                  (LPCTSTR) pAlloc->m_name, pAlloc->m_allocationSoFar, pAlloc->m_allocationSoFar*100/pAllocSet->m_currentTarget, pAlloc->m_constraintAreaSatisfied,
                                  pAlloc->m_minScore, pAlloc->m_maxScore, pAlloc->m_scoreCount, pAlloc->m_usedCount, pAlloc-> m_scoreBasis, pAlloc->m_expandArea, pAlloc->m_expandBasis, pAlloc->m_currentIduScoreIndex );
               else
                  msg.Format( "  Allocation [%s]: Target: %.0f,  Realized: %.1f,  %% Realized: %.1f,  Constraint Area: %.0f,  Min Score: %.0f,  Max Score: %.0f,  Count: %i (%i used), Score Basis: %.0f, Expand Area: %.0f, Expand Basis: %.0f, currentIndex=%i",
                                  (LPCTSTR) pAlloc->m_name, pAlloc->m_currentTarget, pAlloc->m_allocationSoFar, pAlloc->m_allocationSoFar*100/pAlloc->m_currentTarget, pAlloc->m_constraintAreaSatisfied,
                                  pAlloc->m_minScore, pAlloc->m_maxScore, pAlloc->m_scoreCount, pAlloc->m_usedCount, pAlloc-> m_scoreBasis, pAlloc->m_expandArea, pAlloc->m_expandBasis, pAlloc->m_currentIduScoreIndex );

               Report::LogMsg( msg );
               }
            }
         }

      // all done with this allocation set
      }
   }




int CompareScores(const void *elem0, const void *elem1 )
   {
   // The return value of this function should represent whether elem0 is considered
   // less than, equal to, or greater than elem1 by returning, respectively, 
   // a negative value, zero or a positive value.
   
   IDU_SCORE *pIduScore0 = (IDU_SCORE*) elem0;
   IDU_SCORE *pIduScore1 = (IDU_SCORE*) elem1;

   if ( pIduScore0->score < pIduScore1->score )
      return 1;
   else if ( pIduScore0->score == pIduScore1->score )
      return 0;
   else
      return -1;
   }


