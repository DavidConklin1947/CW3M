#pragma once

#include "Policy.h"
#include <randgen/Rand.hpp>
#include <Typedefs.h>
#ifndef NO_MFC
#include <afxtempl.h>
#endif
#include "EnvContext.h"
#include "tixml.h"
#include "FDATAOBJ.H"
#include "Flow\Flow.h"

class ScenarioManager;
class AppVar;
class Shade_a_latorData;

bool NormalizeWeights( float &w0, float &w1, float &w2 );
bool NormalizeWeights( float &w0, float &w1, float &w2, float &w3 );
bool NormalizeWeights( float &w0, float &w1, float &w2, float &w3, float &w4 );

enum VTYPE { V_UNKNOWN=0, V_SCENARIO=1, V_META=2, V_MODEL=4, V_AP=8, V_APPVAR=16,  }; // must be powers of 2

const int  V_ALL  (int( V_META | V_MODEL | V_AP | V_APPVAR | V_SCENARIO ));

struct VAR_INFO
{
public:
   CString name;
   VTYPE vtype;      // address?  Policy ptr?
   void *pVar;       // pointer to the variable to be used      
   TYPE  type;       // type of that variable
   MODEL_DISTR distType;
   Rand *pRand;      // random number generator
   VData defaultValue;
   VData paramLocation; // Note: also stores ID for policies
   VData paramScale;
   VData paramShape;
   bool  inUse;      // use this variable?
   CString description;

   MODEL_VAR *pModelVar;  // set of model var-associated VAR_INFOs, NULL otherwise

   // various constructors
   VAR_INFO() : vtype( V_UNKNOWN ), pVar( NULL ), type( TYPE_NULL ), distType( MD_CONSTANT ), pRand( NULL ), defaultValue(), inUse( true ), pModelVar( NULL ) { }

   VAR_INFO( LPCSTR _name, VTYPE _vtype, void* _pVar, TYPE _type, Rand *_pRand, VData _paramLocation, VData _paramScale, VData _paramShape, bool _inUse, LPCSTR _description )
      : name( _name ), vtype( _vtype ), pVar( _pVar ), type( _type ), distType( MD_CONSTANT ), pRand( _pRand ), 
        defaultValue( _paramLocation ), paramLocation( _paramLocation ), paramScale( _paramScale ), paramShape( _paramShape ),
        inUse( _inUse ), description( _description ), pModelVar( NULL ) { } 

   VAR_INFO( const VAR_INFO &vi ) { *this = vi; }

   // from a exposed Model Output
   VAR_INFO( VTYPE vt, MODEL_VAR &mv, LPCSTR modelName )
      : vtype( vt ), pVar( mv.pVar ), type( mv.type ), defaultValue(), paramLocation( mv.paramLocation ), paramScale( mv.paramScale ), 
      paramShape( mv.paramShape ), distType( mv.distType ), pRand( NULL ), inUse( true ),
      pModelVar( &mv )
      {
      name = modelName;
      name += ".";
      name += mv.name;

      description = mv.description;

      // get the default value from the model variable
      defaultValue = mv.paramLocation;
      }

   // from an AppVar
   VAR_INFO( AppVar *pAppVar );

   void operator = ( const VAR_INFO &e ) 
      {
      name = e.name; vtype = e.vtype; pVar = e.pVar; type = e.type; distType = e.distType;
      paramLocation = e.paramLocation; paramScale = e.paramScale; paramShape = e.paramShape;
      inUse = e.inUse; description = e.description; 
      defaultValue = e.defaultValue; pModelVar = e.pModelVar;
      pRand = NULL;
      }

   ~VAR_INFO() { if ( pRand != NULL ) delete pRand; }
};


class VarInfoArray : public CArray< VAR_INFO, VAR_INFO& >
{
public:
   VarInfoArray() : CArray< VAR_INFO, VAR_INFO& >() { }

   VarInfoArray( VarInfoArray &viArray ) : CArray< VAR_INFO, VAR_INFO& >()
      {
      for ( int i=0; i < viArray.GetSize(); i++ )
         {
         VAR_INFO &vi = viArray.GetAt( i );
         Add( vi );
         }
      }
};


//=======================================================================================
// Scenario class manages the definition of scenarios to be run.  A Scenario is a 
//   collection of variables (VAR_INFO's) and policies that constitute the settings for
//   a given run or multirun.
//
// To add a new VAR_INFO to a scenario, do the following:
//   1) Add the VAR_INFO to the Scenario's collection
//=======================================================================================

struct POLICY_INFO
   {
   CString policyName;
   int     policyID;
   bool    inUse;
   Policy *pPolicy;

   POLICY_INFO() : policyName( "" ), policyID( -1 ), inUse( false ) {}

   POLICY_INFO( Policy *_pPolicy, bool use ) : policyName( _pPolicy->m_name ),
      policyID( _pPolicy->m_id ), inUse( use ), pPolicy( _pPolicy ) { }

   POLICY_INFO( const POLICY_INFO &pi ) : policyName( pi.policyName ), policyID( pi.policyID ), inUse( pi.inUse ), pPolicy( pi.pPolicy ) { }
   };


class PolicyInfoArray : public CArray< POLICY_INFO, POLICY_INFO& >
{
public:
   PolicyInfoArray() : CArray< POLICY_INFO, POLICY_INFO& >() { }

   PolicyInfoArray( PolicyInfoArray &pi ) : CArray< POLICY_INFO, POLICY_INFO& >()
      {
      for ( int i=0; i < pi.GetSize(); i++ )
         Add( pi.GetAt( i ) );      
      }

   int AddPolicyInfo( Policy *pPolicy, bool use )
         { return (int) CArray<POLICY_INFO, POLICY_INFO&>::Add( POLICY_INFO( pPolicy, use ) ); }
};


class Shade_a_latorData
{
public:
   Shade_a_latorData() { m_input_file_name = CString(""); m_comid_downstream = 0; m_output_file_name = CString(""); };
   ~Shade_a_latorData() {};
public:
   CString m_input_file_name;
   int m_comid_downstream;
   Reach* m_pReach_downstream;
   int m_comid_upstream;
   double m_riverKmAtUpperEnd; // river km at upstream end of upstream reach
   Reach* m_pReach_upstream;
   int m_reachCt;
   double m_reachesLength_m;
   CString m_output_file_name;
   FDataObj m_SALoutputData;
   SYSDATE m_startDate, m_endDate;
   int m_numCols;
   double m_canopyDensity_pct;
   double m_SALlai;
   double m_heightThreshold_m;
   double m_futureHeight_m;
}; // end of class Shade_a_latorData


class Scenario
{
friend class ScenarioManager;

public:
   Scenario( LPCSTR name );   // construct a default scenario
   Scenario( Scenario& );     // copy constructor
   ~Scenario( void );

   CString m_name;
   CString m_description;
   CString m_originator;
   bool    m_isEditable;
   bool    m_isShared;

private:
   bool    m_isDefault;        // maintained by the scenario manager

public:

   //float   m_actorAltruismWt;       // Value between 0-1.
   //float   m_actorSelfInterestWt;   // Value between 0-1.
   float   m_policyPrefWt;          // Value between 0-1.
   int     m_decisionElements;      // scenario-specific version

   int     m_evalModelFreq;    // Frequency (years) at which landscape evaluative models are run

   int    m_runCount;          // how many times has this scenario been run?

   Shade_a_latorData m_shadeAlatorData;

   void Initialize();
   int  SetScenarioVars( int runFlag );   // see envModel.cpp for runFlag definition

   int GetVarCount( int type=V_ALL, bool inUseOnly=false );
   VAR_INFO &GetVarInfo( int i ) { return m_varInfoArray[ i ]; }
   VAR_INFO *GetVarInfo( LPCSTR description );
   VAR_INFO *FindVar( void* ptr );

   bool IsDefault() { return m_isDefault; }

   int GetPolicyCount( bool inUseOnly=false );
   int AddPolicyInfo( Policy *pPolicy, bool inUse ) { return m_policyInfoArray.AddPolicyInfo( pPolicy, inUse ); }
   POLICY_INFO &GetPolicyInfo( int i ) { return m_policyInfoArray[ i ]; }
   POLICY_INFO *GetPolicyInfo( LPCSTR name );
   POLICY_INFO *GetPolicyInfoFromID( int id );
   void RemovePolicyInfo( Policy *pPolicy );

protected:
   VarInfoArray    m_varInfoArray;        // contains the variables use on this scenarion
   PolicyInfoArray m_policyInfoArray;     // contains info about the policies used in this scenario
};


class ScenarioManager
{
public:
   ScenarioManager() : m_loadStatus( -1 ), m_includeInSave( true ) { }
   ~ScenarioManager() { RemoveAll(); }

   CString m_path;
   CString m_importPath;          // if imported, path of  import file
   int     m_loadStatus;          // -1=undefined, 0=loaded from envx, 1=loaded from xml file
   bool    m_includeInSave;       // when saving project with external ref, update this file?

   CArray< Scenario*, Scenario* > m_scenarioArray;

   void RemoveAll() { for (int i=0; i < GetCount(); i++ ) delete GetScenario( i ); m_scenarioArray.RemoveAll(); }
   int  GetCount() { return(int) m_scenarioArray.GetSize(); }
   int  AddScenario( Scenario *pScenario ) { return (int) m_scenarioArray.Add( pScenario ); }
   void DeleteScenario( int index );
   Scenario *GetScenario( int i ) { if ( i >= 0 )return m_scenarioArray[ i ]; else return NULL; }
   Scenario *GetScenario( LPCSTR );
   int  GetScenarioIndex( Scenario *pScenario );
   void SetDefaultScenario( int index );
   int  GetDefaultScenario();

   //int LoadScenarios( LPCSTR filename=NULL );
   //int SaveScenarios( LPCSTR filename );

   bool LoadXml(LPCTSTR filename);    
   bool LoadClimateScenariosXml(LPCSTR _filename, FlowContext * pFlowContext);
   int LoadXml(LPCTSTR filename, bool isImporting, bool appendToExisting);       // returns number of nodes in the first level
   int LoadXml( TiXmlNode *pLulcTree, bool appendToExisting );
   int SaveXml( LPCTSTR filename );
   int SaveXml( FILE *fp, bool includeHdr, bool useFileRef );


   void AddPolicyToScenarios( Policy *pPolicy, bool inUse );
   void RemovePolicyFromScenarios( Policy *pPolicy );

   static int CopyScenarioArray( CArray< Scenario*, Scenario* > &toScenarioArray, CArray< Scenario*, Scenario* > &fromScenarioArray );

};
