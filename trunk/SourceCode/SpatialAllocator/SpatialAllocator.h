#pragma once

#include <EnvExtension.h>

#include <FDATAOBJ.H>
#include <QueryEngine.h>
#include <MapExprEngine.h>
#include <PtrArray.h>
#include <RANDOM.HPP>
#include <FDATAOBJ.H>
#include "EnvEngine/EnvConstants.h"

/*---------------------------------------------------------------------------------------------------------------------------------------
 general idea:  the SpatialAllocator class represents a process that has targets that it allocates to the landscape, limited by various constraints
 
 Targets are defined in the xml input file, specified as series of preferneces and/or constraints. 

 The basic idea is the for each IDU, score the idu for a particular attribute. If any constraints apply, the score is zeroed out.
     Scoring is achieved by applying preferences scores.  A preference score is a computed quantity that computes a weighted average
     score with a preference count bonus.

------------------------------------------------------------------------------------------------------------------------------------------- */

enum TARGET_SOURCE { TS_UNDEFINED=0, TS_TIMESERIES=1, TS_RATE=2, TS_FILE=3, TS_USEALLOCATIONSET=4, TS_USEALLOCATION=5 };
enum TARGET_DOMAIN { TD_UNDEFINED=0, TD_AREA=2, TD_PCTAREA=3, TD_ATTR=3, TD_USEALLOCATION=4 };

enum CONSTRAINT_TYPE { CT_QUERY=0 };
enum ALLOC_METHOD { AM_SCORE_PRIORITY=0, AM_BEST_WINS=1 };
enum EXPAND_TYPE  { ET_CONSTANT=0, ET_UNIFORM=1, ET_NORMAL=2, ET_WEIBULL=3, ET_LOGNORMAL=4, ET_EXPONENTIAL=5 };

class AllocationSet;
class Allocation;

struct IDU_SCORE { int idu; float score; };

class Preference
{
public:
   CString m_name;
   CString m_queryStr;
   CString m_weightExpr;

   Query   *m_pQuery;
   MapExpr *m_pMapExpr;

//protected:
   float m_value;    // current value

public:
   Preference( Allocation *pAlloc, LPCTSTR name, LPCTSTR queryStr, LPCTSTR weightExpr );

   ~Preference( void ) { if ( m_pMapExpr != NULL ) delete m_pMapExpr; }

   bool Init( void );
   };


class Constraint
{
public:
   CString m_name;
   CString m_queryStr;
   CONSTRAINT_TYPE m_cType;

   Query  *m_pQuery;
 
   Constraint( LPCTSTR name, LPCTSTR queryStr );

   ~Constraint( void ) { }

   bool Init( void );
};


// a "TargetContainer" is a either an allocation or an allocation set that 
// specifies a target to be achieved.  Because either can be a Target

enum TARGET_CLASS { TC_ALLOCATION, TC_ALLOCATIONSET };

class TargetContainer
{
public:
   CString m_name;
   TARGET_SOURCE  m_targetSource;   // where are target values defined? timeseries, rate, file, ...
   TARGET_DOMAIN  m_targetDomain;   // what units are being allocated area, pctarea, attr???
  
   CString m_targetValues;          // string containing target source information/values
   
   CString m_targetBasisField;      // NEW, contains name of field that is the basis for what is used as the target
   int     m_colTargetBasis;        // field being allocated, e.g. the units of the target values, default = "AREA"

public:
   FDataObj *m_pTargetData;      // input data table for target inputs
  
   // Best Wins temporary variables used during run
   float m_allocationSoFar;     // temporary variable, units of basis
   float m_currentTarget;
   float m_targetRate;   // for TT_RATE only!!

   CString m_targetQuery;           // only used if target domain = TD_PCTAREA; defines 
   Query  *m_pTargetQuery;          // the area...???

   TargetContainer() 
      : m_name( "" )
      , m_targetSource( TS_UNDEFINED )
      , m_targetDomain( TD_UNDEFINED )
      , m_targetValues()
      , m_targetBasisField()
      , m_colTargetBasis( -1 )
      , m_pTargetData( NULL )
      , m_allocationSoFar(0)
      , m_currentTarget( 0 )
      , m_targetRate( 0 )
      , m_pTargetQuery( NULL )
         { }

   TargetContainer( LPCTSTR name ) 
      : m_name( name )
      , m_targetSource( TS_UNDEFINED )
      , m_targetDomain( TD_UNDEFINED )
      , m_targetValues()
      , m_targetBasisField()
      , m_colTargetBasis( -1 )
      , m_pTargetData( NULL )
      , m_allocationSoFar(0)
      , m_currentTarget( 0 )
      , m_targetRate( 0 )
      , m_pTargetQuery( NULL )
         { }

   TargetContainer(LPTSTR name, TARGET_SOURCE source, TARGET_DOMAIN domain, LPCTSTR targetValues ) 
      : m_name( name )
      , m_targetSource( source )
      , m_targetDomain( domain )
      , m_targetValues( targetValues )
      , m_targetBasisField()
      , m_colTargetBasis( -1 )
      , m_pTargetData( NULL )
      , m_allocationSoFar(0)
      , m_currentTarget( 0 )
      , m_targetRate( 0 )
      , m_pTargetQuery( NULL )
         { }
   
   TargetContainer( TargetContainer &tc ) { *this = tc; }

   TargetContainer &operator = ( TargetContainer &tc )
      {
      m_name = tc.m_name;
      m_pTargetQuery   = tc.m_pTargetQuery;
 
      m_targetSource = tc.m_targetSource;
      m_targetDomain = tc.m_targetDomain;
      m_targetValues = tc.m_targetValues;
      }


   ~TargetContainer() { if ( m_pTargetData != NULL ) delete m_pTargetData; } 
   
   virtual TARGET_CLASS GetTargetClass( void ) = 0;

   void SetTargetParams( MapLayer *pLayer, LPCTSTR basis, LPCTSTR source, LPCTSTR values, LPCTSTR domain, LPCTSTR query );

   float SetTarget( int year );   
   bool GetTargetRate( float &rate ) { if ( m_targetSource != TS_RATE ) return false; rate = (float) atof( m_targetValues ); return true; }

   void Init( int id, int colAllocSet, int colSequence );

   bool IsTargetDefined() { return ( m_targetSource == TS_UNDEFINED || m_targetSource == TS_USEALLOCATIONSET || m_targetSource == TS_USEALLOCATION ) ? false : true; }

};


// an allocation represents an <allocation> tag in the xml, e.g.
//  <allocation name="Soybeans" id="12"
//              target_type="rate" 
//              target_values="0.005" 
//              col="SA_BEANS" >
// it contains targets for a given allocation as well as the code used to id the allocation.
// if col is specified, a field is added to the IDUs that has the score of the allocation 
// stored

class Allocation : public TargetContainer
{
public:
   int m_id;      // id
   
   // expansion params.
   bool    m_useExpand;          // allow expanded outcomes?
   CString m_expandAreaStr;      // string specified in 'expand_area" tag
   int     m_expandType;         // 0=constant value (m_expandMaxArea), 1 = uniform, 2 = normal
   
   Rand   *m_pRand;           
   double  m_expandAreaParam0;   // distribution-dependant param 0 - holds max value for constants, shape param for dists
   double  m_expandAreaParam1;   // distribution-dependant param 0, scale param for dists

   float   m_expandMaxArea;   // maximum area allowed for expansion, total including kernal
   
   CString m_expandQueryStr;  // expand into adjacent IDUs that satify this query 
   Query  *m_pExpandQuery;
   
   // area stats
   float   m_initPctArea;
   float   m_constraintAreaSatisfied;

   // basis stats
   float   m_cumBasisActual;
   float   m_cumBasisTarget;

   // additional stats
   float m_minScore;       // min and max score for this allocation
   float m_maxScore;
   int   m_scoreCount;     // number of scores greater than 0 at any given time
   int   m_usedCount;      
   float m_scoreArea;      // area of scores greater than 0 at any given time
   float m_scoreBasis;    // target amount associated with scored IDUs
   float m_expandArea;
   float m_expandBasis;
     
/////protected:
/////   FDataObj *m_pTargetData;      // input data table for target inputs
  
public:   
   PtrArray< Preference > m_prefsArray;
   PtrArray< Constraint > m_constraintArray;
  
   CArray< int > m_sequenceArray;   // array of sequence codes if defined
   CMap< int, int, int, int > m_sequenceMap;   // key=Attr code, value=index in sequence array

   CArray< IDU_SCORE, IDU_SCORE& > m_iduScoreArray;

   CString m_scoreCol;
   int     m_colScores;

   // Best Wins temporary variables used during run
   ////////float m_allocationSoFar;     // temporary variable, units of basis
   ////////float m_currentTarget;
   ////////float m_targetRate;   // for TT_RATE only!!

   // expand stats
   //bool  m_isExpand;
   //float m_maxExpandSize;

   // ScorePriority temporary variables
   int m_currentIduScoreIndex;   // this stores the current index of this allocation in the 
   
   AllocationSet *m_pAllocSet;   // associated allocation set

public:
   // constructor
   Allocation( AllocationSet *pSet, LPCTSTR name, int id ); //, TARGET_SOURCE targetSource, LPCTSTR targetValues );

   Allocation( Allocation &a ) { *this = a; }
   Allocation &operator = ( const Allocation &a ) { }

   ~Allocation( void );

   // methods
   float SetMaxExpandArea( void );
   int   GetCurrentIdu  ( void ) { return m_currentIduScoreIndex >= m_iduScoreArray.GetSize() ? -1 : m_iduScoreArray[ m_currentIduScoreIndex ].idu; }
   float GetCurrentScore( void ) { return m_currentIduScoreIndex >= m_iduScoreArray.GetSize() ? -1 : m_iduScoreArray[ m_currentIduScoreIndex ].score; }

   bool  IsSequence( void ) { return m_sequenceArray.GetSize() > 0; }

   virtual TARGET_CLASS GetTargetClass( void ) { return TC_ALLOCATION; }
};


class AllocationSet : public TargetContainer
{
public:
   CString m_field;
   CString m_seqField;   // used to id the current sequence
   CString m_targetLocation;  // this can be an expression, in which case it is used to define a probabiliy surface
                              // that is sampled to determine the order in which IDUs are selected

   int m_col;                 // the IDU field which is populated by the "winning" allocation ID

   int m_colSequence;

   // note: the following are also defined at the Allocation level 
   bool m_inUse;
   bool m_shuffleIDUs;

   MapExpr  *m_pTargetMapExpr;
   
   PtrArray< Allocation > m_allocationArray;

   //RandUniform *m_pRandUnif;     // for shuffling IDUs   

   FDataObj *m_pOutputData;         // target and realized % areas for each allocation
   //FDataObj *m_pOutputDataCum;      // target and realized cumulative % areas for each allocation
   
   AllocationSet( LPCTSTR name, LPCTSTR field ) 
      : TargetContainer( name ), 
        m_field( field ), m_col( -1 ), m_colSequence( -1 ),
        m_inUse( true ), m_shuffleIDUs( true ), m_pOutputData( NULL ), /* m_pOutputDataCum( NULL ), */
        m_pTargetMapExpr( NULL )
        { }
        
   AllocationSet( AllocationSet &as ) { *this = as; }

   AllocationSet &operator = ( AllocationSet &as )
      {
      m_name        = as.m_name;
      m_field       = as.m_field;
      m_seqField    = as.m_seqField;
      m_col         = as.m_col;
      m_colSequence = as.m_colSequence;
      m_inUse       = as.m_inUse;
      m_shuffleIDUs = as.m_shuffleIDUs;

      m_allocationArray = as.m_allocationArray;
      
      m_pOutputData    = new FDataObj( *as.m_pOutputData );
      //m_pOutputDataCum = new FDataObj( *as.m_pOutputDataCum );

      m_pTargetQuery   = as.m_pTargetQuery;
      m_pTargetMapExpr = as.m_pTargetMapExpr;

      m_targetSource = as.m_targetSource;
      m_targetDomain = as.m_targetDomain;
      m_targetValues = as.m_targetValues;
      }

   ~AllocationSet( ) { if ( m_pOutputData ) delete m_pOutputData;/*  if ( m_pOutputDataCum ) delete m_pOutputDataCum; */}

   int OrderIDUs( CUIntArray &iduArray, RandUniform *pRandUnif );
   int GetAllocationCount( void ) { return (int) m_allocationArray.GetSize(); }
   Allocation *GetAllocation( int i ) { return m_allocationArray.GetAt( i ); }

   virtual TARGET_CLASS GetTargetClass( void ) { return TC_ALLOCATIONSET; }
};



class SpatialAllocator : public EnvAutoProcess
{
public:
   SpatialAllocator() : EnvAutoProcess(), m_pMapExprEngine( NULL ), m_pMapLayer( NULL ), 
      m_pQueryEngine( NULL ), m_allocMethod( AM_SCORE_PRIORITY ), m_scoreThreshold(0), 
      m_shuffleIDUs( false ), m_pRandUnif(NULL), m_totalArea(0), m_collectExpandStats(false), 
      m_pExpandStats(NULL),
		m_runNumber( -1 )
 { }
   
   ~SpatialAllocator( void );
   
   BOOL Init   ( EnvContext *pEnvContext, LPCTSTR initStr );
   BOOL InitRun( EnvContext *pEnvContext, bool useInitialSeed );
   BOOL Run    ( EnvContext *pContext );
	BOOL EndRun ( EnvContext *pContext );
   //BOOL Setup( EnvContext *pContext, HWND hWnd )      { return FALSE; }

   MapLayer      *m_pMapLayer;            // owned by Envision
   MapExprEngine *m_pMapExprEngine;       // owned by this process

   PtrArray< AllocationSet > m_allocationSetArray;

   QueryEngine   *m_pQueryEngine;         // owned by this process
   int m_colArea;                         // default="AREA"
   ALLOC_METHOD m_allocMethod;

   CArray< int, int > m_expansionIDUs;   // idus that

   float m_scoreThreshold;    // 0 be default - pref scores < this value are not considered

protected:
   CUIntArray m_iduArray;    // used for shuffling IDUs

   void SetAllocationTargets( int yearOfRun );  // always 0-based 
   void ScoreIduAllocations( EnvContext *pContext, bool useAddDelta );
   void AllocScorePriority( EnvContext *pContext, bool useAddDelta );
   void AllocBestWins( EnvContext *pContext, bool useAddDelta );
   bool PopulateSequences( void );
   float Expand( int idu, Allocation *pAlloc, bool useAddDelta, int currAttrCode, int newAttrCode, int currSequenceID, int newSequenceID, EnvContext*, CArray< bool > &isOpenArray, float &expandArea );
   float AddIDU( int idu, Allocation *pAlloc, bool &addToPool, float areaSoFar, float maxArea );
   int  CollectExpandStats( void );
   void CollectData( EnvContext *pContext );
	
   bool LoadXml( LPCTSTR filename );

   bool m_shuffleIDUs;
   RandUniform *m_pRandUnif;     // for shuffling IDUs   

   float m_totalArea;

	int m_runNumber;

   // data collection
   FDataObj m_outputData;

   bool m_collectExpandStats;
   FDataObj *m_pExpandStats;    // cols are size bins, rows are active allocations
   };
