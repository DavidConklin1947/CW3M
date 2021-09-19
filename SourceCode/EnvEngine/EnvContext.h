// EnvContext.h
//
///////////////////////////////////////////////////////

#if !defined (__ENVCONTEXT_H__INCLUDED_)
#define __ENVCONTEXT_H__INCLUDED_

#include <Vdata.h>
#include <DATE.HPP>
//x #include "..\Flow\Flow.h"
#include "ptrarray.h"


class ActorManager;
class PolicyManager;
class DeltaArray;
class MapLayer;
class IDUlayer;
class LulcTree;
class MapExprEngine;
class QueryEngine;
class EnvModel;
class EnvContext;
class FDataObj;
class FlowContext;

struct MODEL_VAR;

// Envision extension types
enum { EET_AUTOPROCESS=1, 
       EET_EVALMODEL=2, 
       EET_ANALYSISMODULE=4, 
       EET_DATAMODULE=8, 
       EET_INPUT_VISUALIZER=16, 
       EET_RT_VISUALIZER=32, 
       EET_POSTRUN_VISUALIZER=64 };

// visualizer types.  These populate ENV_VISUALIZER::type
enum { VT_INPUT=1, VT_RUNTIME=2, VT_POSTRUN=4, VT_POSTRUN_GRAPH=8, VT_POSTRUN_MAP=16, VT_POSTRUN_VIEW=32 };

// 

struct ENV_EXT_INFO
{
public:
   ENV_EXT_INFO( void ) : types( 0 ), extra( 0 ) { }

   int     types;     // or'd combination of Envision extension types given above;
   CString description;

   long    extra;
};


//-----------------------------------------------------------------------------
//--------------- I N F O   I N T E R F A C E ---------------------------------
//-----------------------------------------------------------------------------
typedef void (PASCAL *GETEXTINFOFN) ( ENV_EXT_INFO* );


//-----------------------------------------------------------------------------
//------------------ E X T E N S I O N    I N T E R F A C E S -----------------
//-----------------------------------------------------------------------------
typedef BOOL (PASCAL *SETUPFN)(EnvContext*, HWND);

//-----------------------------------------------------------------------------
//------------------ P R O C E S S    I N T E R F A C E S ---------------------
//-----------------------------------------------------------------------------
typedef BOOL (PASCAL *INITFN)          (EnvContext*, LPCTSTR  initInfo); 
typedef BOOL (PASCAL *INITRUNFN)       (EnvContext*, bool useInitialSeed );
typedef BOOL (PASCAL *RUNFN)           (EnvContext* );
typedef BOOL (PASCAL *ENDRUNFN)        (EnvContext* );
typedef BOOL (PASCAL *PUSHFN)          (EnvContext*); // deprecated
typedef BOOL (PASCAL *POPFN)           (EnvContext*); // deprecated
typedef BOOL (PASCAL *PROCESSMAPFN)    (MapLayer *pLayer, int id);
typedef int  (PASCAL *INPUTVARFN)      (int modelID, MODEL_VAR**);
typedef int  (PASCAL *OUTPUTVARFN)     (int modelID, MODEL_VAR**);


//-----------------------------------------------------------------------------
//--------- A N A L Y S I S   M O D U L E    I N T E R F A C E S --------------
//-----------------------------------------------------------------------------
typedef BOOL (PASCAL *ANALYSISMODULEFN)(EnvContext*, HWND, LPCTSTR initInfo );


//-----------------------------------------------------------------------------
//-------------- D A T A  M O D U L E    I N T E R F A C E S ------------------
//-----------------------------------------------------------------------------
typedef BOOL (PASCAL *DATAMODULEFN)(EnvContext*, HWND, LPCTSTR initInfo );


//-----------------------------------------------------------------------------
//--------------- V I S U A L I Z E R   I N T E R F A C E S -------------------
//-----------------------------------------------------------------------------        INPUT        RUNTIME      POSTRUN
typedef BOOL (PASCAL *INITWNDFN)  (EnvContext*, HWND );     //       required      optional     optional
typedef BOOL (PASCAL *UPDATEWNDFN)(EnvContext*, HWND );     // for init of run         optional      optional     optional


//-----------------------------------------------------------------------------
//--------- A C T O R   D E C I S I O N   I N T E R F A C E -------------------
//-----------------------------------------------------------------------------
typedef BOOL (PASCAL *UTILITYFN)(EnvContext* );


//-----------------------------------------------------------------------------
//--------------- I N T E R F A C E    S T R U C T U R E S  -------------------
//-----------------------------------------------------------------------------

struct ENV_EXTENSION
   {
   HINSTANCE hDLL;
   SETUPFN   setupFn;
   int       id;                // model id as specified by the project file
   int       handle;            // unique handle for this model/process, passed in the context
   CString   name;              // name of the model
   CString   path;              // path to the dll
   CString   initInfo;          // string passed to the model, specified in the project file
   bool      use;               // use this model during the run?
   int       frequency;         // how often this process is executed
   float     runTime;           // amount of time this process has executed

   ENV_EXTENSION( HINSTANCE _hDLL, 
               SETUPFN _setupFn,
               int     _id,
               LPCTSTR _initInfo,
               LPCTSTR _name, 
               LPCTSTR _path ) 
          : hDLL( _hDLL )
          , setupFn( _setupFn )
          , id( _id )
          , name( _name )
          , path( _path )
          , initInfo( _initInfo )
          , use( true )
          , frequency( 0 )
          , runTime( 0 )
      { }

   virtual ~ENV_EXTENSION() { }
   };


struct ENV_PROCESS : public ENV_EXTENSION
   {
   INITFN            initFn;
   INITRUNFN         initRunFn;
   RUNFN             runFn;
   ENDRUNFN          endRunFn;
   PUSHFN            pushFn;
   POPFN             popFn;
   INPUTVARFN        inputVarFn;
   OUTPUTVARFN       outputVarFn;
   int               frequency;           // how often this process is executed
   int               col;                 // -1=unused, 0=used but invalid, > 0=valid (StoreModelCols() needs to be called)                     
   CString           fieldName;           // name of column reserved for this model, or empty if not used

   float             timeStep;		      // time interval at which this process executes (default=1 year)

   bool              initRunOnStartup;    // run InitRun() at startup (after Init())

   CString           dependencyNames;
   int               dependencyCount;
   ENV_PROCESS     **dependencyArray;     // ptr to process(es) that this process is dependant on         
   
   ENV_PROCESS( HINSTANCE _hDLL, 
               RUNFN _runFn,
               INITFN _initFn,
               INITRUNFN _initRunFn,
               ENDRUNFN _endRunFn,
               PUSHFN _pushFn,
               POPFN  _popFn,
               SETUPFN _showPropertiesFn,
               INPUTVARFN _inputVarFn,
               OUTPUTVARFN _outputVarFn,
               int     _id,
               LPCTSTR _initFnInfo,
               LPCTSTR _name, 
               LPCTSTR _path ) 
          : ENV_EXTENSION( _hDLL, _showPropertiesFn, _id, _initFnInfo, _name, _path )
          , runFn ( _runFn )
          , initFn( _initFn )
          , initRunFn( _initRunFn ) 
          , endRunFn( _endRunFn )
          , pushFn( _pushFn )
          , popFn( _popFn )
          , inputVarFn( _inputVarFn )
          , outputVarFn( _outputVarFn )
          , frequency( 1 )
          , col( -1 )
          , timeStep( 1.0f )
          , initRunOnStartup( false )
          , dependencyCount( 0 )
          , dependencyArray( NULL )
      { }

   virtual ~ENV_PROCESS() { if ( dependencyArray != NULL ) delete [] dependencyArray;  }
   };


struct ENV_EVAL_MODEL : public ENV_PROCESS
   {
   int        showInResults;        // determines whether this model is used in the presentation of results
                                    // 0=don't show anywhere, 1=show everywhere, 2=show where unscaled (beyond [-3..3]) can be shown, 4=show as pie chart (default is scatter).

   //int        decisionUse;          // 1 = use in actor self-interest decision, 2 = use in altruism decision, 0=neither
   //float      metagoal;             // metagoals (-3 to +3)
   float      score;                // current value of the abundance metric (-3 to +3)
   float      rawScore;             // current value of abundance metric (nonscaled)
   FDataObj  *pDataObj;             // current (optional) data object returned by the model (NULL if not used)
   float      rawScoreMin;
   float      rawScoreMax;          
   CString    rawScoreUnits;        // string decribing units of raw score
   float      gain;                 // default (1.); set by EvalModelLearning
   float      offset;               // default (0.); ditto
   //auto_ptr< Non_param_cdf<> > apNpCdf;

   static int nextHandle;

   ENV_EVAL_MODEL( HINSTANCE _hDLL, 
               RUNFN     _runFn,
               INITFN    _initFn,
               INITRUNFN _initRunFn,
               ENDRUNFN  _endRunFn,
               PUSHFN    _pushFn,
               POPFN     _popFn,
               SETUPFN _showPropertiesFn,
               INPUTVARFN  _inputVarFn,
               OUTPUTVARFN _outputVarFn,
               int    _id,
               int    _showInResults,
               LPCTSTR _initFnInfo,
               LPCTSTR _name, 
               LPCTSTR _path ) 
          : ENV_PROCESS( _hDLL, _runFn,_initFn,_initRunFn, _endRunFn, _pushFn, _popFn, _showPropertiesFn, _inputVarFn, _outputVarFn, _id, _initFnInfo, _name, _path )
          , score( 0.f )
          , rawScore( 0.f)
          , pDataObj( NULL )
          , gain(1.f)
          , offset(0.f)
          , showInResults( _showInResults )
         { handle = nextHandle++; }

      // ScaleScore(arg) validates and scales the float & arg; 
      // On ERROR returns non-zero (out of bounds and NaN have different return vals).
      inline int ScaleScore(float & arg) const;  
   };


// autonomous process descriptions
struct ENV_AUTO_PROCESS : public ENV_PROCESS
   {
   PROCESSMAPFN processMapFn;
   int          timing;   // 0 = pre-year (runs at beginning of year), 1 = post-year (runs at the end of the year)
   int          sandbox;  // 0 = dont use in sandbox, 1 = use in sandbox
   static int   nextHandle;

   ENV_AUTO_PROCESS( HINSTANCE _hDLL, 
            RUNFN _runFn,
            INITFN _initFn,
            INITRUNFN _initRunFn, 
            ENDRUNFN _endRunFn,
            PUSHFN _pushFn,
            POPFN  _popFn,
            SETUPFN _showPropertiesFn,
            INPUTVARFN _inputVarFn,
            OUTPUTVARFN _outputVarFn,
            int     _id,
            LPCTSTR _initFnInfo,
            LPCTSTR _name, 
            LPCTSTR _path ) 
          : ENV_PROCESS( _hDLL, _runFn,_initFn,_initRunFn, _endRunFn, _pushFn, _popFn, _showPropertiesFn, _inputVarFn, _outputVarFn, _id, _initFnInfo, _name, _path )
      , processMapFn( NULL )
      , timing( -1 )
      , sandbox( 0 )
      { handle = nextHandle++; }
   };



struct ENV_VISUALIZER : public ENV_EXTENSION
   {
   int  type;  // VT_INPUT, VT_RTVIEW, ...
   bool inRegistry;

   INITFN      initFn;
   INITRUNFN   initRunFn;
   RUNFN       runFn;
   ENDRUNFN    endRunFn;
   //VPOSTRUNFN    postRunFn; 
   INITWNDFN   initWndFn;
   UPDATEWNDFN updateWndFn;
 
   ENV_VISUALIZER( HINSTANCE _hDLL, 
               RUNFN       _runFn,
               INITFN      _initFn,
               INITRUNFN   _initRunFn, 
               ENDRUNFN    _endRunFn,
               INITWNDFN   _initWndFn,
               UPDATEWNDFN _updateWndFn,
               //VPOSTRUNFN  _postRunFn,
               SETUPFN _showPropertiesFn,
               int     _id,
               int     _type,
               LPCTSTR _initFnInfo,
               LPCTSTR _name, 
               LPCTSTR _path,
               bool    _inRegistry ) 
          : ENV_EXTENSION( _hDLL, _showPropertiesFn, _id, _initFnInfo, _name, _path )
          , type( _type )
          , inRegistry( _inRegistry )
          , runFn ( _runFn )
          , initFn( _initFn )
          , initRunFn( _initRunFn )
          //, postRunFn( _postRunFn )
          , endRunFn( _endRunFn )
          , initWndFn( _initWndFn )
          , updateWndFn( _updateWndFn )
         { }

   virtual ~ENV_VISUALIZER() { }
   };


struct ENV_ANALYSISMODULE : public ENV_EXTENSION
   {
   int type;              // reserved
   bool inRegistry;

   ANALYSISMODULEFN runFn;

   ENV_ANALYSISMODULE( HINSTANCE _hDLL, 
               ANALYSISMODULEFN _runFn,
               int    _id,
               int    _type,
               LPCSTR _name, 
               LPCSTR _path,
               bool   _inRegistry ) 
          : ENV_EXTENSION( _hDLL, NULL, _id, NULL, _name, _path )
          , type( 0 )
          , inRegistry( _inRegistry )
          , runFn( _runFn )
          { }

   virtual ~ENV_ANALYSISMODULE() { }
   };



struct ENV_DATAMODULE : public ENV_EXTENSION
   {
   int  type;
   bool inRegistry;

   DATAMODULEFN runFn;

   ENV_DATAMODULE( HINSTANCE _hDLL, 
               DATAMODULEFN _runFn,
               int    _id,
               int    _type,
               LPCSTR _name, 
               LPCSTR _path,
               bool   _inRegistry ) 
          : ENV_EXTENSION( _hDLL, NULL, _id, NULL, _name, _path )
          , type( 0 )
          , inRegistry( _inRegistry )
          , runFn( _runFn )
          { }

   virtual ~ENV_DATAMODULE() { }
   };

class SubstituteString 
{
public:
   SubstituteString() { m_stringToReplace = ""; m_replacementString = ""; }

   SubstituteString(CString string_to_replace, CString replacement_string)
   {
      m_stringToReplace = string_to_replace;
      m_replacementString = replacement_string;
   }

public:
   CString m_stringToReplace;
   CString m_replacementString;

};



//-----------------------------------------------------------------------------
//------------ E N V C O N T E X T   D E F I N I T I O N ----------------------
//-----------------------------------------------------------------------------

class EnvContext
{
public:
   INT_PTR         (*ptrAddDelta)(EnvModel *pModel,  int cell, int col, int year, VData newValue, int handle );
   bool coldStartFlag;
   bool spinupFlag;
   int m_maxDaysInYear; // 360=>12 equal length months of 30 days (Hadley), 365=>no leap days, 366=>Gregorian calendar. Also 0 and 1 for debugging.
   int             startYear;             // calendar year in which the simulation started (e.g. 2008)
   int             endYear;               // last calendar year of simulation (e.g. 2050)
   int             currentYear;           // current calendar year of run, incremented from startYear (e.g. 2010)
   SubstituteString m_studyAreaName;
   PtrArray < SubstituteString > m_substituteStrings;

   FlowContext* m_pFlowContext;
   SYSDATE         m_simDate;               // date being simulated
   int daysInCurrentYear; // 360 or 365 or 366. Gets set in FlowModel::Run()
   int weatherYear; // calendar year from which to draw the weather information; usually the same as currentYear
   int m_useWaterYears; // 0 => simulation year starts on Jan 1; 1 => simulation year starts on Oct 1; water years are named by the calendar year in which they end.
   int m_doy0ofOct1;   int   m_yearsInStartingClimateAverages; // initial long term average climate values are calculated using this number of years, if possible
   float m_BasinMeanTminGrowingSeason; // long term average growing season TMIN over the ag land in the basin
   float m_BasinMeanPrcpGrowingSeason; // long term average growing season precip over the ag land in the basin
   int             yearOfRun;             // 0-based year of run  (e.g. 1)
   int             run;                   // 0-based, incremented after each run.
   int             scenarioIndex;         // 0-based scenario index 
   bool            showMessages;
   int             logMsgLevel;           // see flags in envmodel.h
   const IDUlayer *pMapLayer;             // pointer to the IDU layer.  This is const because extensions generally shouldn't write to this.
   const MapLayer * pReachLayer;
   const MapLayer * pHRUlayer;
   const MapLayer *pSubcatchmentLayer;
   const MapLayer *pNodeLayer;
   const MapLayer *pLinkLayer;
   const MapLayer * pGWlayer;
   ActorManager   *pActorManager;         // pointer to the actor manager
	PolicyManager  *pPolicyManager;        // pointer to the policy manager
   DeltaArray     *pDeltaArray;           // pointer to the current delta array
   EnvModel       *pEnvModel;             // pointer to the current model
   LulcTree       *pLulcTree;             // pointer to the lulc tree used in the simulation
   QueryEngine    *pQueryEngine;          // pointer to an query engine
   MapExprEngine  *pExprEngine;           // pointer to an expression engine
   int             id;                    // id of the module being called
   int             handle;                // unique handle of the module
   int             col;                   // database column to populate, -1 for no column
   INT_PTR         firstUnseenDelta;      // models should start scanning the deltaArray here
   INT_PTR         lastUnseenDelta;       // models should stop scanning the delta array here.  This will be the index of the last delta
   bool           *targetPolyArray;       // array of polygon indices with true/false to process (NULL=process all)
   int             targetPolyCount;       // number of elements in targetPolyarray (0 to run everywhere
   ENV_EXTENSION  *pExtensionInfo;        // opaque ptr to ENV_EVAL_MODEL, ENV_AUTO_PROCESS, ANAL, VIZ as appropriate; thus deprecates: id, col, score
#ifndef NO_MFC
   CWnd           *pWnd;                  // relevant window, NULL or type depends on context
#endif

   // these values are set and returned by eval models, ignored by autonomous processes
   float           score;                 // overall score for this evaluation, -3 to +3 (unused by AP's)
   float           rawScore;              // model-specific raw score ( not normalized )

   FDataObj       *pDataObj;              // data object returned from the model at each time step (optional, NULL if not used)
   INT_PTR         extra;                 // extra data - depends on type
   int             reserved[32];

	EnvContext( IDUlayer *_pMapLayer )
		: ptrAddDelta( NULL )
      , startYear( 0 )
      , endYear( 0 )
      , currentYear( 0 )
      , yearOfRun( 0 )
      , m_pFlowContext(NULL)
      , run( -1 )
      , scenarioIndex( -1 )
      , showMessages( true )
      , logMsgLevel( 0 )
      , pMapLayer(_pMapLayer)
      , pActorManager( NULL )
      , pPolicyManager( NULL )
      , pDeltaArray( NULL )
      , pEnvModel( NULL )
      , pLulcTree( NULL )
      , pQueryEngine( NULL )
      , pExprEngine( NULL )
      , col( -1 )
      , id( -1 )
      , handle( -1 )
      , firstUnseenDelta( -1 )
      , lastUnseenDelta( -1 )
      , pExtensionInfo( NULL )
#ifndef NO_MFC
      , pWnd( NULL )
#endif
      , targetPolyArray( NULL )
      , targetPolyCount( 0 )
      , score( 0 )
      , rawScore( 0 )
      , pDataObj( NULL )
      , pHRUlayer(NULL)
      , pSubcatchmentLayer( NULL )
      , pNodeLayer( NULL )
      , pLinkLayer( NULL )
      , pGWlayer(NULL)
      , coldStartFlag(false)
      , spinupFlag(false)
      , m_maxDaysInYear(366) 
      , daysInCurrentYear(365)
      , m_useWaterYears(0)
      { }

};


//-----------------------------------------------------------------------------
//--------- E X P O S E D    M O D E L    V A R I A B L E S -------------------
//-----------------------------------------------------------------------------

enum MODEL_DISTR { MD_CONSTANT=0, MD_UNIFORM=1, MD_NORMAL=2, MD_LOGNORMAL=3, MD_WEIBULL=4 };

enum { MVF_LEVEL_0=0, MVF_LEVEL_1=1 };  // Model Variable Flags 


struct MODEL_VAR
{
public:
   CString name;              // name of the variable
   void   *pVar;              // address of the model variable. 
   TYPE    type;              // type of the address variable (see typedefs.h)
   MODEL_DISTR distType;      // destribution type (see enum above)
   VData   paramLocation;     // value of the location parameter
   VData   paramScale;        // value of the scale parameter   
   VData   paramShape;        // value of the shape parameter 
   CString description;       // description of the model variable
   int     flags;             // default=MVF_LEVEL_0, MVF_LEVEL_1 
   INT_PTR extra;             // if flags > 0, contains ptr to containing???
   int     useInSensitivity;
   VData   saValue;  
   // e.g. ~Normal(location, shape), 

   MODEL_VAR () : 
         name("uninitialized"),
         pVar(NULL),
         type(TYPE_NULL),
         distType(MD_CONSTANT),
         paramLocation(),//-DBL_MAX),
         paramScale(),
         paramShape(),
         description("uninitialized"),
         flags( 0 ),
         extra( 0 ),
         useInSensitivity( 0 ),
         saValue( 0 )
         {}

   MODEL_VAR(   
      const CString & name_,
      void *pVar_,
      TYPE type_,
      MODEL_DISTR distType_,
      VData paramLocation_,
      VData paramScale_,
      VData paramShape_,
      const CString & description_,
      int   flags_
      ) :
         name(name_),
         pVar(pVar_),
         type(type_),
         distType(distType_),
         paramLocation(paramLocation_),
         paramScale(paramScale_),
         paramShape(paramShape_),
         description(description_),
         flags( flags_ ),
         extra( 0 ),
         useInSensitivity( 0 ),
         saValue( 0 )
      {}

   MODEL_VAR ( const MODEL_VAR &mv ) : 
         name( mv.name ),
         pVar( mv.pVar ),
         type( mv.type ),
         distType( mv.distType ),
         paramLocation( mv.paramLocation ),
         paramScale( mv.paramScale ),
         paramShape( mv.paramShape ),
         description( mv.description ),
         flags( mv.flags ),
         extra( mv.extra ),
         useInSensitivity( 0 ),
         saValue( 0 )
         {} 
};



#endif
