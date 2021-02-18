#include "stdafx.h"
#pragma hdrstop

#include "Scenario.h"

#include "EnvModel.h"
#include "Policy.h"
#include "../Flow/Flow.h"

#include <tinyxml.h>
#include <PathManager.h>


extern PolicyManager *gpPolicyManager;
extern EnvModel      *gpModel;


bool NormalizeWeights( float &w0, float &w1, float &w2 )
   {
   float sum = w0 + w1 + w2;

   if ( sum > 0 )
      {
      w0 = w0 / sum;
      w1 = w1 / sum;
      w2 = 1.0f - w0 - w1;
      return true;
      }

   return false;
   }


bool NormalizeWeights( float &w0, float &w1, float &w2, float &w3 )
   {
   float sum = w0 + w1 + w2 + w3;
   
   if ( sum > 0 )
      {
      w0 = w0 / sum;
      w1 = w1 / sum;
      w2 = w2 / sum;
      w3 = 1.0f - w0 - w1 - w2;
      return true;
      }

   return false;
   }


bool NormalizeWeights( float &w0, float &w1, float &w2, float &w3, float &w4 )
   {
   float sum = w0 + w1 + w2 + w3 + w4;
   
   if ( sum > 0 )
      {
      w0 = w0 / sum;
      w1 = w1 / sum;
      w2 = w2 / sum;
      w3 = w3 / sum;
      w4 = 1.0f - w0 - w1 - w2 - w3;
      return true;
      }

   return false;
   }



VAR_INFO::VAR_INFO( AppVar *pAppVar )
: name( pAppVar->m_name )
, vtype( V_APPVAR )
, pVar( pAppVar )
, type( pAppVar->GetValue().type )   // always a PTR type
, defaultValue( pAppVar->GetValue() )
, paramLocation( 0 )
, paramScale( 0 )
, paramShape( 0 )
, distType( MD_CONSTANT )
, pRand( NULL )
, inUse( true )
  {  }



Scenario::Scenario( LPCSTR name ) 
: m_name( name ),
  m_isShared( true ),
  m_isEditable( true ),
  m_isDefault( false ),
  //m_actorAltruismWt( 0.33f ),
  //m_actorSelfInterestWt( 0.33f ),
  m_policyPrefWt( 0.5f ),
  m_decisionElements( gpModel->m_decisionElements ),
  m_evalModelFreq( 1 ),
  m_runCount( 0 )
   {
   // construct a default scenario

   //===================== M O D E L  M E T A G O A L   S E T T I N G S  =====================
   int metagoalCount = EnvModel::GetMetagoalCount();
	for( int i=0; i < metagoalCount; i++ )
	   {
      METAGOAL *pGoal = EnvModel::GetMetagoalInfo( i );
      //ENV_EVAL_MODEL *pModel = pGoal->pEvalModel;

      //if ( pModel != NULL )
      //   {
         m_varInfoArray.Add( VAR_INFO( pGoal->name, V_META, (void*) &(pGoal->weight), TYPE_FLOAT, NULL, VData( 0.0f ), VData(0.0f), VData(0.0f), true,
         "Metagoal weighting [-3, +3] for this model - determines relative priorities for altruistic actors based on scarcity" ) );
      //   }
      }

   //=========================== M O D E L / P R O C E S S  U S A G E ========================
   int modelCount = EnvModel::GetModelCount();
   //
   //if ( modelCount > 0 )
   //   {
   //   for ( int i=0; i < modelCount; i++ )
   //      {
   //      ENV_EVAL_MODEL *pInfo = EnvModel::GetModelInfo( i );
   //      m_varInfoArray.Add( VAR_INFO( CString( pInfo->name+".InUse"), V_MODEL, (void*) &(pInfo->use), TYPE_BOOL, NULL, VData( 0.0f ), VData(0.0f), VData(0.0f), false,
   //      "1=enable model for this scenario, 0=disable model for this scenario" ) );
   //      }
   //   }
   //
   int apCount = EnvModel::GetAutonomousProcessCount();
   //
   //if ( apCount > 0 )
   //   {
   //   for ( int i=0; i < apCount; i++ )
   //      {
   //      ENV_AUTO_PROCESS *pInfo = EnvModel::GetAutonomousProcessInfo( i );
   //      m_varInfoArray.Add( VAR_INFO( CString( pInfo->name+".InUse"), V_AP, (void*) &(pInfo->use), TYPE_BOOL, NULL, VData( 0.0f ), VData(0.0f), VData(0.0f), false,
   //      "1=enable process for this scenario, 0=disable process for this scenario" ) );
   //      }
   //   }

   //================================ M O D E L   V A R I A B L E S ==========================
   //int modelCount = EnvModel::GetModelCount();
   for( int i=0; i < modelCount; i++ )
	   {
      ENV_EVAL_MODEL *pModel = EnvModel::GetModelInfo( i );
      if ( pModel->use && pModel->inputVarFn != NULL )
         {
         MODEL_VAR *modelVarArray = NULL;
         int varCount = pModel->inputVarFn( pModel->id, &modelVarArray );

         // for each variable exposed by the model, add it to the possible scenario variables
         for ( int j=0; j < varCount; j++ )
            {
            MODEL_VAR &mv = modelVarArray[ j ];
            if ( mv.pVar != NULL )
               {
               VAR_INFO vi( V_MODEL, mv, pModel->name );    // set vi.pModelVar = &mv
               m_varInfoArray.Add( vi );
               }
            }
         }
      }

   //================= A U T O N O U S  P R O C E S S   V A R I A B L E S ==================
   //int apCount = EnvModel::GetAutonomousProcessCount();
   for( int i=0; i < apCount; i++ )
	   {
      ENV_AUTO_PROCESS *pAP = EnvModel::GetAutonomousProcessInfo( i );
      if ( pAP->use && pAP->inputVarFn != NULL )
         {
         MODEL_VAR *modelVarArray = NULL;
         int varCount = pAP->inputVarFn( pAP->id, &modelVarArray );

         // for each variable exposed by the model, add it to the possible scenario variables
         for ( int j=0; j < varCount; j++ )
            {
            MODEL_VAR &mv = modelVarArray[ j ];
            if ( mv.pVar != NULL )
               m_varInfoArray.Add( VAR_INFO( V_AP, mv, pAP->name ) );
            }
         }
      }

   //================= A P P L I C A T I O N   V A R I A B L E S ==================
   int appVarCount = gpModel->GetAppVarCount( AVT_APP );
   for( int i=0; i < appVarCount; i++ )
	   {
      AppVar *pAppVar = gpModel->GetAppVar( i );   // note: type AVT_APP will always be at the head of the list
      m_varInfoArray.Add( VAR_INFO( pAppVar ) );
      }


   //================= S C E N A R I O  V A R I A B L E S ==========================
   m_varInfoArray.Add( VAR_INFO( "Decision Elements", V_SCENARIO, (void*) &m_decisionElements, TYPE_INT, NULL, VData( gpModel->m_decisionElements ), VData(), VData(), false, 
    "Decision Elements to use. This should be the sum of the desired elements: 0=none, 1=Actor Value Alignment, 2=Landscape Feedback, 4=Global Policy Preferences, 8=Utility, 16=Social Network" ) );


   //=================================== P O L I C I E S =====================================
   // NOTE!!! Policies should always come last
   int policyCount = gpPolicyManager->GetPolicyCount();
   for( int i=0; i < policyCount; i++ )
	   {
      Policy *pPolicy = gpPolicyManager->GetPolicy( i );
      m_policyInfoArray.AddPolicyInfo( pPolicy, pPolicy->m_use );
      }   

   Initialize();     // copy any needed info into the scenario
   }

Scenario::Scenario(Scenario &s )
   : m_name( s.m_name )
   , m_description( s.m_description )
   , m_originator( s.m_originator )
   , m_isEditable( s.m_isEditable )
   , m_isShared( s.m_isShared )
   , m_isDefault( s.m_isDefault )
   , m_policyPrefWt( s.m_policyPrefWt )
   , m_decisionElements( s.m_decisionElements )
   , m_evalModelFreq( s.m_evalModelFreq )
   , m_runCount( s.m_runCount )
   , m_varInfoArray( s.m_varInfoArray )
   , m_policyInfoArray( s.m_policyInfoArray )

   { }


Scenario::~Scenario(void)
   { }


int Scenario::GetVarCount( int type /*=V_ALL*/, bool inUseOnly /*=false*/ )
   {
   int count = 0;

   for ( int i=0; i < m_varInfoArray.GetSize(); i++ )
      {
      VAR_INFO &vi = this->GetVarInfo( i );

      if ( int( vi.vtype ) & type )
         {
         if ( !inUseOnly || vi.inUse )
            ++count;
         }
      }

   return count;
   }


VAR_INFO *Scenario::GetVarInfo( LPCSTR name )
   {
   int count = (int) m_varInfoArray.GetSize();

   for ( int i=0; i < count; i++ )
      {
      VAR_INFO *pInfo = &m_varInfoArray[ i ];

      if ( pInfo->name.Compare( name ) == 0 )
         return pInfo;
      }
   
   return NULL;
   }

 
VAR_INFO *Scenario::FindVar( void* ptr )
   {
   int count = (int) m_varInfoArray.GetSize();

   for ( int i=0; i < count; i++ )
      {
      VAR_INFO *pInfo = &m_varInfoArray[ i ];

      if ( pInfo->pVar = ptr )
         return pInfo;
      }
   
   return NULL;
   }


int Scenario::GetPolicyCount( bool inUseOnly /*=false*/ )
   {
   int count = 0;

   for ( int i=0; i < m_policyInfoArray.GetSize(); i++ )
      {
      POLICY_INFO &pi = this->GetPolicyInfo( i );
 
      if ( !inUseOnly || pi.inUse )
         ++count;
      }

   return count;
   }


POLICY_INFO *Scenario::GetPolicyInfo( LPCSTR name )
   {
   int count = (int) m_policyInfoArray.GetSize();

   for ( int i=0; i < count; i++ )
      {
      POLICY_INFO &pi = m_policyInfoArray[ i ];

      if ( pi.policyName.Compare( name ) == 0 )
         return &m_policyInfoArray[ i ];
      }
   
   return NULL;
   }


POLICY_INFO *Scenario::GetPolicyInfoFromID( int id )
   {
   int count = (int) m_policyInfoArray.GetSize();

   for ( int i=0; i < count; i++ )
      {
      POLICY_INFO &pi = m_policyInfoArray[ i ];

      if ( pi.policyID == id )
         return &m_policyInfoArray[ i ];
      }
   
   return NULL;
   }


void Scenario::RemovePolicyInfo( Policy *pPolicy )
   {
   for ( int i=0; i < GetPolicyCount(); i++ )
      {
      if ( this->m_policyInfoArray[ i ].pPolicy == pPolicy )
         {
         m_policyInfoArray.RemoveAt( i );
         return;
         }
      }
   }



// Initialize() initializes all scenario variables.
void Scenario::Initialize()
   {
   for ( int i=0; i < m_varInfoArray.GetSize(); i++ )
      {
      VAR_INFO &vi = m_varInfoArray[ i ];

      switch( vi.vtype )
         {
         case V_META:
         case V_MODEL:
         case V_AP:
            {
            if ( vi.pRand )
               {
               delete vi.pRand;
               vi.pRand = NULL;
               }

            switch( vi.distType )
               {
               case MD_CONSTANT:
                  break;                
                  
               case MD_UNIFORM:
                  {
                  double location, scale;
                  vi.paramLocation.GetAsDouble( location );
                  vi.paramScale.GetAsDouble( scale );
                  vi.pRand = new RandUniform( location, location+scale, 0 );
                  }
                  break;                  
                  
               case MD_NORMAL:
                  {
                  double location, scale;
                  vi.paramLocation.GetAsDouble( location );
                  vi.paramScale.GetAsDouble( scale );
                  vi.pRand = new RandNormal( location, scale, 0 );
                  }
                  break;

               case MD_WEIBULL:
                  {
                  double location, scale, shape;
                  vi.paramLocation.GetAsDouble( location );
                  vi.paramScale.GetAsDouble( scale );
                  vi.paramShape.GetAsDouble( shape );
                  vi.pRand = new RandWeibull( location, scale, 0 );
                  }
                  break;
                  
               case MD_LOGNORMAL:
                  {
                  double location, scale, shape;
                  vi.paramLocation.GetAsDouble( location );
                  vi.paramScale.GetAsDouble( scale );
                  vi.paramShape.GetAsDouble( shape );
                  vi.pRand = new RandLogNormal( location, scale, 0 );
                  }
                  break;
               }
            }
            break;

         case V_APPVAR:
            {
            AppVar *pAppVar = (AppVar*) vi.pVar;
            if ( pAppVar->IsGlobal() )
               pAppVar->Evaluate();   // evaluate the AppVar expr and store result
            }
            break;

         case V_SCENARIO:
            break; // nothing required

         default: ASSERT( 0 );
         }
      }
   }


int Scenario::SetScenarioVars( int runFlag )
   {
   // runFlag:  0 = set scenario variables, no randomization (uses paramLocation)
   //           1 = set scenario variable, using randomization if variable is defined as random
   //          -1 = set scenario variable EXCEPT any whose "useInSensitivity" flags are 
   //               set to -1, no randomization


   // Envision variables:
   //gpDoc->m_model.m_altruismWt = m_actorAltruismWt;
   //gpDoc->m_model.m_selfInterestWt = m_actorSelfInterestWt;
   gpModel->m_policyPrefWt = m_policyPrefWt;
   gpModel->m_evalFreq     = m_evalModelFreq;

   // policies
   for ( int i=0; i < this->m_policyInfoArray.GetSize(); i++ )
      {
      POLICY_INFO &pInfo = m_policyInfoArray[ i ];

      Policy *pPolicy = gpPolicyManager->GetPolicyFromID( pInfo.policyID );
      if ( pPolicy )
         {
         // if doing sensitivity with this policy, reverse it's normal use.
         // Otherwise, set it based on the scenario
         if ( runFlag == -1 && pPolicy->m_useInSensitivity == -1 )
            pPolicy->m_use = ! pInfo.inUse;
         else
            pPolicy->m_use = pInfo.inUse;
         }
      }

   // model variables
   for ( int i=0; i < m_varInfoArray.GetSize(); i++ )
      {
      VAR_INFO &vi = m_varInfoArray[ i ];    // scenario-specific model variable info

      switch( vi.vtype )
         {
         case V_META:
         case V_MODEL:
         case V_AP:
            {
            // its a ptr to a metagoal, model or autoproc variable.  Two cases are possible:
            // - it is a random value (pRand != NULL ) or it is a constant
            // first get the value, then interpret it
            bool useSensitivity = false;
            VData saValue;

            switch( vi.vtype )
               {
               case V_META:
                  {
                  METAGOAL *pGoal = (METAGOAL*) vi.pVar;
                  useSensitivity = runFlag == -1 && pGoal->useInSensitivity == -1;
                  saValue = pGoal->saValue;
                  }
                  break;

               case V_MODEL:
               case V_AP:
                  {
                  MODEL_VAR *pModelVar = vi.pModelVar;
                  ASSERT( pModelVar != NULL && pModelVar->pVar != NULL );
                  useSensitivity = runFlag == -1 && pModelVar->useInSensitivity == -1;
                  saValue = pModelVar->saValue;
                  }
                  break;
               }

            VData val;

            if ( vi.inUse == false )   // nothing specified, use default if exists 
               {
               if ( vi.defaultValue.IsNull() == false )
                  val = vi.defaultValue;
               }
            else
               {
               if ( vi.pRand != NULL && runFlag == 1 )
                  val = vi.pRand->RandValue();
               else
                  {
                  ASSERT( runFlag != 1 || vi.distType == MD_CONSTANT );
                  val = vi.paramLocation; // constant
                  }
               }
            
            switch( vi.type )
               {
               case TYPE_BOOL:
                  {
                  double r;
                  val.GetAsDouble( r );
                  if ( r > 0.5 )
                     *((bool*)vi.pVar) = true;
                  else
                     *((bool*)vi.pVar) = false;

                  // are we doing sensitivity analysis?
                  if ( useSensitivity )
                     *((bool*)vi.pVar) = !(*((bool*)vi.pVar) );   // flip it
                  }
                  break;

               case TYPE_UINT:
                  {
                  int v;
                  // are we doing sensitivity analysis?
                  if ( useSensitivity )
                     {
                     if ( v == 0 )
                        v = 1;
                     else if ( v == 1 )
                        v = 0;
                     else
                        saValue.GetAsInt( v );
                     }
                  else
                     val.GetAsInt( v );

                  *((UINT*)vi.pVar) = (UINT) v;
                  }
                  break;
                  
               case TYPE_INT:
                  {
                  int v;
                  // are we doing sensitivity analysis?
                  if ( useSensitivity )
                     {
                     if ( v == 0 )
                        v = 1;
                     else if ( v == 1 )
                        v = 0;
                     else
                        saValue.GetAsInt( v );
                     }
                  else
                     val.GetAsInt( v );

                  *((int*)vi.pVar) = (int) v;
                  }
                  break;

               case TYPE_SHORT:
                  {
                  int v;
                  // are we doing sensitivity analysis?
                  if ( useSensitivity )
                     {
                     if ( v == 0 )
                        v = 1;
                     else if ( v == 1 )
                        v = 0;
                     else
                        saValue.GetAsInt( v );
                     }
                  else
                     val.GetAsInt( v );

                  *((short*)vi.pVar) = (short) v;
                  }
                  break;

               case TYPE_LONG:
                  {
                  int v;
                  // are we doing sensitivity analysis?
                  if ( useSensitivity )
                     {
                     if ( v == 0 )
                        v = 1;
                     else if ( v == 1 )
                        v = 0;
                     else
                        saValue.GetAsInt( v );
                     }
                  else
                     val.GetAsInt( v );

                  *((long*)vi.pVar) = (long) v;
                  }
                  break;

               case TYPE_FLOAT:
                  {
                  float v;
                  // are we doing sensitivity analysis?
                  if ( useSensitivity )
                     {
                     if ( v == 0 )
                        v = 1;
                     else if ( v == 1 )
                        v = 0;
                     else
                        saValue.GetAsFloat( v );
                     }
                  else
                     val.GetAsFloat( v );

                  *((float*)vi.pVar) = v;
                  }
                  break;

               case TYPE_DOUBLE:
                  {
                  double v;
                  // are we doing sensitivity analysis?
                  if ( useSensitivity )
                     {
                     if ( v == 0 )
                        v = 1;
                     else if ( v == 1 )
                        v = 0;
                     else
                        saValue.GetAsDouble( v );
                     }
                  else
                     val.GetAsDouble( v );

                  *((double*)vi.pVar) = v;
                  }
                  break;
                  
               default:
                  ASSERT( 0 );
               }
            }
            break;

         case V_APPVAR:
            {
            AppVar *pAppVar = (AppVar*) vi.pVar;
            
            ASSERT( pAppVar != NULL );
            ASSERT( pAppVar->m_pMapExpr != NULL );

            if ( pAppVar->m_useInSensitivity == -1 )  // fized value
               {
               pAppVar->SetValue( pAppVar->m_saValue );
               }
            else if ( pAppVar->m_timing > 0 )
               {
               if ( vi.inUse )
                  {
                  // use whatever expression string is stored the the paramLocation VData
                  bool ok = vi.paramLocation.GetAsString( pAppVar->m_expr );
                  ASSERT( ok );
                  }
               else
                  {
                  if ( vi.defaultValue.IsNull() == false )
                     pAppVar->m_expr = vi.defaultValue.GetAsString();
                  }

               if ( pAppVar->IsGlobal() )
                  pAppVar->Evaluate();   // evaluate the AppVar expr and store result
               }
            }
            break;

         case V_SCENARIO:
            {
            // decision Elements?
            if ( vi.pVar == (void*) &m_decisionElements )
               {
               // update EnvModel variable
               if ( vi.inUse )
                  vi.paramLocation.GetAsInt( gpModel->m_decisionElements );
               else
                  vi.defaultValue.GetAsInt( gpModel->m_decisionElements );
               }
            }
            break;

         default: ASSERT( 0 );
         }
      }

   return 1;
   }


int ScenarioManager::CopyScenarioArray( CArray< Scenario*, Scenario* > &toArray, CArray< Scenario*, Scenario* > &fromArray )
   {
   for ( int i=0; i < toArray.GetSize(); i++ )
      delete toArray[ i ];

   toArray.RemoveAll();

   for ( int i=0; i < fromArray.GetSize(); i++ )
      toArray.Add( new Scenario( *(fromArray[ i ]) ) );

   return (int) fromArray.GetSize();
   }


Scenario *ScenarioManager::GetScenario( LPCSTR name )
   {
   for ( int i=0; i < GetCount(); i++ )
      {
      Scenario *pScenario = GetScenario( i );
      if ( pScenario->m_name.CompareNoCase( name ) == 0 )
         return pScenario;
      }

   return NULL;
   }


int ScenarioManager::GetScenarioIndex( Scenario *pScenario )
   {
   for ( int i=0; i < GetCount(); i++ )
      {
      if ( m_scenarioArray.GetAt( i ) == pScenario )
         return i;
      }

   return -1;
   }


void ScenarioManager::DeleteScenario( int index )
   {
   if ( index >= GetCount() )
      return;

   Scenario *pScenario = GetScenario( index );

   m_scenarioArray.RemoveAt( index );
   delete pScenario;
   }


void ScenarioManager::SetDefaultScenario( int index )
   {
   for ( int i=0; i < this->GetCount(); i++ )
      GetScenario( i )->m_isDefault = ( i == index ? true : false );
   }


int ScenarioManager::GetDefaultScenario()
   {
   for ( int i=0; i < this->GetCount(); i++ )
      if ( GetScenario( i )->m_isDefault )
         return i;

   return -1;
   }


bool ScenarioManager::LoadXml(LPCSTR _filename)
{
   CString filename;
   if (PathManager::FindPath(_filename, filename) < 0) //  return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found 
   {
      CString msg;
      msg.Format("ScenarioManager: Input file '%s' not found - no scenarios will be loaded", _filename);
      Report::ErrorMsg(msg);
      return(false);
   }

   TiXmlDocument doc;
   bool ok = doc.LoadFile(filename);

   if (!ok)
   {
      CString msg;
      msg.Format("Error reading scenario input file, %s", (LPCTSTR)m_path);
      Report::ErrorMsg(msg);
      return(false);
   }

// start iterating through the scenarios
   TiXmlElement* pXmlRoot = doc.RootElement();

   int defaultIndex = 0;
   pXmlRoot->Attribute("default", &defaultIndex);

   int count = LoadXml(pXmlRoot, false);
   if (count <= 0)
   {
      CString msg;
      msg.Format("ScenarioManager::LoadXmL() No scenarios found in scenario input file, %s", (LPCTSTR)m_path);
      Report::ErrorMsg(msg);
      return(false);
   }

   SetDefaultScenario(defaultIndex);

   m_loadStatus = 1;   // loaded from XML file
   m_importPath = filename;
   return(true);
} // end of ScenarioManager::LoadXml(LPCSTR _filename)


int ScenarioManager::LoadXml(LPCSTR _filename, bool isImporting, bool appendToExisting)
{
   if (appendToExisting == false)
      RemoveAll();

   CString filename;
   if (PathManager::FindPath(_filename, filename) < 0) //  return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found 
   {
      CString msg;
      msg.Format("ScenarioManager: Input file '%s' not found - no scenarios will be loaded", _filename);
      Report::ErrorMsg(msg);
      return false;
   }

// have xml string, start parsing
   TiXmlDocument doc;
   bool ok = doc.LoadFile(filename);

   if (!ok)
   {
#ifndef NO_MFC
      CString msg;
      msg.Format("Error reading scenario input file, %s", (LPCTSTR)m_path);
      AfxGetMainWnd()->MessageBox(doc.ErrorDesc(), msg);
#endif
      return -1;
   }

// start interating through the scenarios
   TiXmlElement* pXmlRoot = doc.RootElement();

   int defaultIndex = 0;
   pXmlRoot->Attribute("default", &defaultIndex);

   int count = LoadXml(pXmlRoot, appendToExisting);

   SetDefaultScenario(defaultIndex);

   if (appendToExisting == false)
   {
      if (isImporting)
      {
         m_loadStatus = 1;   // loaded from XML file
         m_importPath = filename;
      }
      else     // loading from ENVX file
      {
         m_loadStatus = 0;
         m_path = filename;
      }
   }

   return count;
} // end of ScenarioManager::LoadXml(LPCSTR _filename, bool isImporting, bool appendToExisting)


int ScenarioManager::LoadXml( TiXmlNode *pScenarios, bool appendToExisting )
   {
   m_loadStatus = 0;

   if ( pScenarios == NULL )
      return -1;
   
   // file specified?.
   LPCTSTR file = pScenarios->ToElement()->Attribute( _T("file") );
   
   if ( file != NULL && file[0] != NULL )
      return LoadXml( file, true, appendToExisting );

   // Note: Files are of the form
   // <scenarios default=[index]>
   //    <scenario name='name' originator='orig' isEditable='0|1' isShared='0|1' actorAltruismWt='value' evalModelFreq='value'>
   //       <policies>
   //           <policy id='value' name='policyname' inUse='0|1' />
   //       </policies>
   //       <vars>
   //         <var vtype='[1|2]' name='description' distType='0-4' paramLocation='value' paramScale='value' paramShape='value' inUse='[0|1]' />
   //       </vars>
   //    </scenario>
   // </scenarios>
   //
   // see EnvExtraData.h for the distType values.
   ASSERT( pScenarios != NULL );

   bool loadSuccess = true;

   // iterate through scenarios
   TiXmlNode *pXmlScenarioNode = NULL;
   while( pXmlScenarioNode = pScenarios->IterateChildren( pXmlScenarioNode ) )
      {
      // get the scenario
      TiXmlElement *pXmlScenario = pXmlScenarioNode->ToElement();

      if ( pXmlScenario == NULL )
         continue;
    
      const char *name = pXmlScenario->Attribute( "name" );

      if ( name == NULL )
         {
         CString msg( "Misformed Scenario element reading" );
         msg += m_path;

         Report::InfoMsg( msg );
         loadSuccess = false;
         continue;
         }
   
      Scenario *pScenario = new Scenario( name );

      for ( int i=0; i < pScenario->GetPolicyCount(); i++ )
         pScenario->GetPolicyInfo( i ).inUse = false;

      // get the scenario attributes
      //TiXmlGetAttr( pXmlScenario, _T("originator"), _T("originator"),  LPCTSTR itemName, LPCTSTR attrName, int &value, int defaultValue, bool isRequired )
      const char *originator = pXmlScenario->Attribute( "originator" );
      if ( originator != NULL )
         pScenario->m_originator = originator;

      int isEditable = 1;
      int isShared   = 1;
      pXmlScenario->Attribute( "isEditable", &isEditable );
      pXmlScenario->Attribute( "isShared",   &isShared );

      //double actorAltruismWt = 0.33;
      //double actorSelfInterestWt = 0.33;
      double policyPrefWt = 0.34;
      int evalModelFreq = 1;
      int decisionElements = gpModel->m_decisionElements;

      //pXmlScenario->Attribute( "landscapeFeedbackWt", &actorAltruismWt );
      //pXmlScenario->Attribute( "actorValueWt", &actorSelfInterestWt );
      pXmlScenario->Attribute( "policyPreferenceWt", &policyPrefWt     );
      pXmlScenario->Attribute( "evalModelFreq",      &evalModelFreq    );
      pXmlScenario->Attribute( "decisionElements",   &decisionElements );

      //pScenario->m_actorAltruismWt = (float) actorAltruismWt;
      //pScenario->m_actorSelfInterestWt = (float) actorSelfInterestWt;
      pScenario->m_policyPrefWt = (float) policyPrefWt;
      pScenario->m_evalModelFreq    = evalModelFreq;
      pScenario->m_decisionElements = decisionElements;

      Shade_a_latorData* pSAL = &(pScenario->m_shadeAlatorData);
      pSAL->m_valid = false;
      const char * file_name = pXmlScenario->Attribute("Shade-A-Lator_input_file");
      if (file_name != NULL)
      {
         pSAL->m_valid = true;
         pScenario->m_shadeAlatorData.m_input_file_name = CString(file_name);
         int comid;  pXmlScenario->Attribute("downstream_comid", &comid);
         pSAL->m_valid = pSAL->m_valid && comid > 0;
         pScenario->m_shadeAlatorData.m_comid_downstream = comid;
         pXmlScenario->Attribute("upstream_comid", &comid);
         pSAL->m_valid = pSAL->m_valid && comid > 0;
         pScenario->m_shadeAlatorData.m_comid_upstream = comid;
         const char* output_file_name = pXmlScenario->Attribute("Shade-A-Lator_output_file");
         pSAL->m_valid = pSAL->m_valid && output_file_name != NULL;
         pScenario->m_shadeAlatorData.m_output_file_name = (output_file_name == NULL) ? CString("") : CString(output_file_name);

         // Example: SAL_start_year = "2019" SAL_start_month = "11" SAL_start_day = "1" >
         int SAL_start_year;  pXmlScenario->Attribute("SAL_start_year", &SAL_start_year);
         int SAL_start_month;  pXmlScenario->Attribute("SAL_start_month", &SAL_start_month);
         int SAL_start_day;  pXmlScenario->Attribute("SAL_start_day", &SAL_start_day);
         pScenario->m_shadeAlatorData.m_startDate = SYSDATE(SAL_start_month, SAL_start_day, SAL_start_year);
         int SAL_end_year;  pXmlScenario->Attribute("SAL_end_year", &SAL_end_year);
         int SAL_end_month;  pXmlScenario->Attribute("SAL_end_month", &SAL_end_month);
         int SAL_end_day;  pXmlScenario->Attribute("SAL_end_day", &SAL_end_day);
         pScenario->m_shadeAlatorData.m_endDate = SYSDATE(SAL_end_month, SAL_end_day, SAL_end_year);
// ???      pSAL->m_valid = pSAL->m_valid && IsValidDate(pSAL->m_startDate) && IsValidDate(pSAL->m_endDate);

         double canopy_density_pct = 0; pXmlScenario->Attribute("SAL_canopy_density_pct", &canopy_density_pct);
         pScenario->m_shadeAlatorData.m_canopyDensity_pct = canopy_density_pct;
         pScenario->m_shadeAlatorData.m_SALlai = -(log(1. - (canopy_density_pct / 100.)) / BEERS_LAW_K);
         double height_threshold_m = 0;
         const char* height_threshold_attribute = pXmlScenario->Attribute("SAL_height_threshold_m", &height_threshold_m);
         double future_height_m = 0;
         const char* future_height_attribute = pXmlScenario->Attribute("SAL_future_height_m", &future_height_m);
         if (height_threshold_attribute != NULL && future_height_attribute != NULL)
         {
            pScenario->m_shadeAlatorData.m_heightThreshold_m = height_threshold_m;
            pScenario->m_shadeAlatorData.m_futureHeight_m = future_height_m;
         }
         if (!pSAL->m_valid)
         {
            CString msg;
            msg.Format("ScenarioManager::LoadXml() Unable to interpret Shade-a-lator fields in scenario %s.", pScenario->m_name);
            Report::WarningMsg(msg);
         }
      } // end of if (file_name != NULL)

      //NormalizeWeights( pScenario->m_actorAltruismWt, pScenario->m_actorSelfInterestWt, pScenario->m_policyPrefWt );
      TiXmlNode *pXmlDescNode = pXmlScenarioNode->FirstChild( "description" );
      if ( pXmlDescNode )
         {
         TiXmlElement *pDesc = pXmlDescNode->ToElement();
         pScenario->m_description = pDesc->GetText();
         pScenario->m_description.Trim();
         }
            
      // policies are next
      TiXmlNode *pXmlPoliciesNode = pXmlScenarioNode->FirstChild("policies");

      if ( pXmlPoliciesNode != NULL )
         {
         TiXmlNode *pXmlPolicyNode = NULL;
         while( pXmlPolicyNode = pXmlPoliciesNode->IterateChildren( pXmlPolicyNode ) )
            {
            // get the policy
            TiXmlElement *pXmlPolicy = pXmlPolicyNode->ToElement();

            const char *name = pXmlPolicy->Attribute( "name" );
            int id;
            int inUse;
            pXmlPolicy->Attribute( "id", &id );
            pXmlPolicy->Attribute( "inUse", &inUse );

            // make sure the policy name, id match an existing policy
            Policy *pPolicy = gpPolicyManager->GetPolicyFromID( id );

            if ( pPolicy == NULL )
               {
               CString msg;
               msg.Format( "A Policy (%s, ID: %i) was referenced by a scenario that does not exist in the current policy set.  It will be removed next time you save the scenario.",
                  name, id );
               Report::WarningMsg( msg );
               }
            else
               {
               // policy found, store the recorded info
               POLICY_INFO *pInfo = pScenario->GetPolicyInfoFromID( id );
               ASSERT (pInfo != NULL);
               pInfo->inUse = inUse ? true : false;
               }
            }
         }  // end of: if ( pXmlPoliciesNode != NULL )

      // set all scenario variable off
      for ( int i=0; i < pScenario->GetVarCount(); i++ )
         pScenario->GetVarInfo( i ).inUse = false;

      // basic scenario parameters set, add variables
      //   <var vtype='[1|2]' name='description' distType='0-4' paramLocation='value' paramScale='value' paramShape='value' inUse='[0|1]' />
      //   <var vtype='[1|2]' name='description' value='[value]' inUse='[0|1]' />
      // get vars node to iterate though
      TiXmlNode *pXmlVarsNode = pXmlScenarioNode->LastChild();
      if ( pXmlVarsNode != NULL )
         {
         TiXmlNode *pXmlVarNode = NULL;
         while( pXmlVarNode = pXmlVarsNode->IterateChildren( pXmlVarNode ) )
            {
            const char *name     = NULL;
            int    distType      = 0;
            int    vtype         = 0; // required
            int    inUse         = 1;
            float  value         = 0.0f;
            float  paramScale    = 0.0f;
            float  paramLocation = 0.0f;
            float  paramShape    = 0.0f;
            XML_ATTR attrs[] = { 
               // attr            type           address         isReq checkCol
               { "name",          TYPE_STRING,   &name,          true,    0 },
               { "vtype",         TYPE_INT,      &vtype,         true,    0 },
               { "distType",      TYPE_INT,      &distType,      false,   0 },
               { "inUse",         TYPE_INT,      &inUse,         true,    0 },
               { "value",         TYPE_FLOAT,    &value,         false,   0 },
               { "paramScale",    TYPE_FLOAT,    &paramScale,    false,   0 },
               { "paramLocation", TYPE_FLOAT,    &paramLocation, false,   0 },
               { "paramShape",    TYPE_FLOAT,    &paramShape,    false,   0 },
               { NULL,           TYPE_NULL,     NULL,            false,   0 } };

            TiXmlElement *pXmlVar = pXmlVarNode->ToElement();
               
            if ( TiXmlGetAttributes( pXmlVar, attrs, this->m_path ) == false )  ///filename, m_pMapLayer ) == false )
               return false;
   
            //// get the policy
            //const char *name = pXmlVar->Attribute( "name" );
            //ASSERT( name != NULL );
            //
            //int    distType =0;
            //int    vtype; // required
            //int    inUse = 1;
            //double paramScale = 0.0f;
            //double paramLocation = 0.0f;
            //double paramShape = 0.0f;
            //
            //int retVal = pXmlVar->QueryIntAttribute( "vtype", &vtype );    // required
            //ASSERT( retVal == TIXML_SUCCESS );
            //
            //retVal = pXmlVar->QueryIntAttribute( "distType", &distType );  // required only for non-policies
            //ASSERT( retVal ==  TIXML_SUCCESS );
            //
            //retVal = pXmlVar->QueryIntAttribute( "inUse", &inUse );        // required
            //ASSERT( retVal == TIXML_SUCCESS );
            //
            //
            //pXmlVar->Attribute( "paramScale",    &paramScale );
            //pXmlVar->Attribute( "paramLocation", &paramLocation );
            //pXmlVar->Attribute( "paramShape",    &paramShape );

            VAR_INFO *pInfo = pScenario->GetVarInfo( name );
  
            if ( pInfo == NULL ) // still not found?
               {
               CString msg;
               msg.Format( "Scenario variable [%s] in %s is not a valid scenario variable... ignoring", name, (LPCTSTR) m_path );

               Report::WarningMsg( msg );
               loadSuccess = false;
               continue;
               }
         
            pInfo->inUse    = inUse ? true : false;
            pInfo->distType = (MODEL_DISTR) distType;

            if ( distType == MD_CONSTANT && pXmlVar->Attribute( "value" ) != NULL ) // constant????
               pInfo->paramLocation = value;
            else
               {
               pInfo->paramScale    = (float) paramScale;
               pInfo->paramLocation = (float) paramLocation;
               pInfo->paramShape    = (float) paramShape;
               }
            }  // end of 
         }

      AddScenario( pScenario );
      }

   if ( loadSuccess == false )
      Report::WarningMsg( "There was a problem loading one or more scenarios.  See the Message Tab for details." );

   return GetCount();
   }


bool ScenarioManager::LoadClimateScenariosXml(LPCSTR _filename, FlowContext * pFlowContext)
{
   char drive_str[5] = "";
   int nchars = 0;
   nchars = GetEnvironmentVariable("CLIMATE_DRIVE", drive_str, sizeof(drive_str));
   CString msg;
   if (nchars > 0)
   {
      msg.Format("ScenarioManager::LoadClimateScenariosXml() environment variable CLIMATE_DRIVE = %s Only the first letter matters.", drive_str);
      drive_str[1] = ':';
      drive_str[2] = 0;
   }
   else  msg.Format("ScenarioManager::LoadClimateScenariosXml() GetEnvironmentVariable(CLIMATE_DRIVE...) returned nchars = %d", nchars);
   Report::LogMsg(msg);

   CString filename;
   if (PathManager::FindPath(_filename, filename) < 0) //  return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found 
   {
      CString msg;
      msg.Format("ScenarioManager: Input file '%s' not found - no scenarios will be loaded", _filename);
      Report::ErrorMsg(msg);
      return(false);
   }

   TiXmlDocument doc;
   bool ok = doc.LoadFile(filename);
   if (!ok)
   {
      CString msg;
      msg.Format("Error reading scenario input file, %s", (LPCTSTR)m_path);
      Report::ErrorMsg(msg);
      return(false);
   }

   TiXmlElement* pXmlScenarios = doc.RootElement();

   int defaultIndex = 0;
   XML_ATTR scenariosAttrs[] = {
    // attr                     type          address                                isReq  checkCol
    { "default",                 TYPE_INT,      &defaultIndex,                       false,   0 },
    { NULL,                      TYPE_NULL,     NULL,                                false,   0 } };
   ok = TiXmlGetAttributes(pXmlScenarios, scenariosAttrs, filename.GetString(), NULL);
   ASSERT(ok);

   TiXmlElement* pXmlScenario = pXmlScenarios->FirstChildElement("climate_scenario");
   while (pXmlScenario != NULL)
   {
      LPCTSTR name = NULL;
      int id = -1;
      int max_days_in_climate_year = 366;
      int first_year = 2010;
      int last_year = 2010;
      CString row_0_location = "N";

      XML_ATTR scenarioAttrs[] = {
         // attr      type          address         isReq  checkCol
         { "name",    TYPE_STRING,   &name,         true,   0 },
         { "id",      TYPE_INT,      &id,           true,    0 },
         { "maxDaysInClimateYear", TYPE_INT, &max_days_in_climate_year, true, 0 },
         { "firstYear", TYPE_INT,      &first_year, false, 0 },
         { "lastYear", TYPE_INT,      &last_year, false, 0 },
         { "NetCDF_row_0_location_N_or_S", TYPE_CSTRING, &row_0_location, false, 0},
         { NULL,      TYPE_NULL,     NULL,          false,   0 } };

      bool ok = TiXmlGetAttributes(pXmlScenario, scenarioAttrs, filename);
      if (!ok)
      {
         CString msg;
         msg.Format(_T("Flow: Misformed element reading <climate_scenario> attributes in input file %s - it is missing the 'id' attribute"), filename.GetString());
         Report::ErrorMsg(msg);
         break;
      }

      CString row_0_location_lc = row_0_location;
      row_0_location_lc.MakeLower();
      bool row_0_in_south = false;
      if (row_0_location_lc == "s" || row_0_location_lc == "south") row_0_in_south = true;
      else if (row_0_location_lc == "n" || row_0_location_lc == "north") row_0_in_south = false;
      else
      {
         CString msg;
         msg.Format("LoadClimateScenariosXml() cannot interpret NetCDF_row_0_location_N_or_S = %s in climate scenario %s. Please specify N for north or S for south.",
            row_0_location.GetString(), name);
         Report::ErrorMsg(msg);
         return(false);
      }

      ClimateScenario* pScenario = new ClimateScenario;
      pScenario->m_id = id;
      pScenario->m_name = name;
      pScenario->m_maxDaysInClimateYear = max_days_in_climate_year;
      pScenario->m_firstYear = first_year;
      pScenario->m_lastYear = last_year;
      pScenario->m_row0inSouth = row_0_in_south;

      FlowModel* pFlowModel = pFlowContext->pFlowModel;

      pFlowModel->m_scenarioArray.Add(pScenario);

      //---------- <climate> ----------------------------------
      TiXmlElement* pXmlClimate = pXmlScenario->FirstChildElement("climate");
      while (pXmlClimate != NULL)
      {
         CString path;
         LPTSTR varName = NULL;
         CString cdtype;
         float delta = -999999999.0f;
         float elev = 0.0f;
         CString units = "";

         XML_ATTR climateAttrs[] = {
            // attr        type          address     isReq  checkCol
            { "type",      TYPE_CSTRING,  &cdtype,   true,   0 },
            { "path",      TYPE_CSTRING,  &path,     true,   0 },
            { "var_name",  TYPE_STRING,   &varName,  false,   0 },
            { "delta",     TYPE_FLOAT,    &delta,    false,  0 },
            { "elev",      TYPE_FLOAT,    &elev,     false,  0 },
            { "units",     TYPE_CSTRING,  &units,    false,  0 },
         { NULL,        TYPE_NULL,     NULL,      false,  0 } };

         bool ok = TiXmlGetAttributes(pXmlClimate, climateAttrs, filename);
         if (!ok)
         {
            CString msg;
            msg.Format(_T("Flow: Misformed element reading <climate> attributes in input file %s"), filename.GetString());
            Report::ErrorMsg(msg);
         }
         else
         {
            ClimateDataInfo* pInfo = new ClimateDataInfo;
            pInfo->m_varName = varName;
            pInfo->m_unitsAttribute = units;
            pInfo->m_firstYear = pScenario->m_firstYear;
            pInfo->m_lastYear = pScenario->m_lastYear;
            pInfo->m_maxDaysInClimateYear = pScenario->m_maxDaysInClimateYear;
            pInfo->m_row0inSouth = pScenario->m_row0inSouth;

            // Figure out the path to the climate data - 3 cases.
            // "path" here is the string read from Flow.xml.
            // Case 1: path contains a colon.  Interpret it as an absolute path containing a drive letter.
            // Case 2: path begins with a backslash.  Interpret it as an absolute path on the drive identified by the environment variable CLIMATE_DRIVE, if any,
            //         or otherwise on the same drive as the current working directory.
            // Case 3: otherwise, interpret the path as relative to the current working directory.
            CStringArray tokens;
            int driveLetter = ::Tokenize(CString(path), ":", tokens);
            if (driveLetter > 1) // if there was a colon (Tokenize() returns 1+the number of colons in the string)
               pInfo->m_path = path; // Case 1
            else if (path.GetAt(0) == '\\')
            { // Case 2. nchars is the length of the CLIMATE_DRIVE environment variable, or 0 if CLIMATE_DRIVE is undefined.
               if (nchars > 0) pInfo->m_path = CString(drive_str) + path;
               else pInfo->m_path = path;
            }
            else pInfo->m_path = pFlowModel->m_path + path; // Case 3        

            pInfo->m_useDelta = pXmlClimate->Attribute("delta") ? true : false;
            if (pInfo->m_useDelta)
               pInfo->m_delta = delta;

            pInfo->m_type = CDT_UNKNOWN;
            switch (cdtype[0])
            {
               case 'p':     // precip
                  pInfo->m_type = CDT_PRECIP;
                  break;

               case 't':
                  if (cdtype[2] == 'v')          // tavg
                     pInfo->m_type = CDT_TMEAN;
                  else if (cdtype[2] == 'i')     // tmin
                     pInfo->m_type = CDT_TMIN;
                  else if (cdtype[2] == 'a')     // tmax
                     pInfo->m_type = CDT_TMAX;
                  if (units == "K") pInfo->m_offset = -273.15f;
                  break;

               case 's':
                  if (cdtype[1] == 'o')          // tavg
                     pInfo->m_type = CDT_SOLARRAD;
                  break;

               case 'h':
                  pInfo->m_type = CDT_SPHUMIDITY;
                  break;

               case 'r':
                  if (cdtype[3] == 'v')          // rhavg
                     pInfo->m_type = CDT_RELHUMIDITY;
                  else if (cdtype[3] == 'i')     // rhmin
                     pInfo->m_type = CDT_RHMIN;
                  else if (cdtype[3] == 'a')     // rhmax
                     pInfo->m_type = CDT_RHMAX;
                  break;

               case 'w':
                  pInfo->m_type = CDT_WINDSPEED;
                  break;

               case 'u':
                  pInfo->m_type = CDT_UAS;
                  break;

               case 'v':
                  pInfo->m_type = CDT_VAS;
                  break;

               default: break;
            }  // end of: switch( type[ 0 ] )
            if (pInfo->m_type == CDT_UNKNOWN)
            {
               CString msg;
               msg.Format("Flow Error: Unrecognized climate data '%s'type reading file %s", cdtype.GetString(), filename);
               delete pInfo;
               pInfo = NULL;
            }

            if (pInfo != NULL)
            {
            //pModel->m_climateDataInfoArray.Add( pInfo );
               pScenario->m_climateDataMap.SetAt(pInfo->m_type, pInfo);
               pScenario->m_climateInfoArray.Add(pInfo);
            }

         }  // end of: else ( valid info )

         pXmlClimate = pXmlClimate->NextSiblingElement("climate");
      }  // end of: while ( pXmlClimate != NULL )

      pFlowModel->m_currentFlowScenarioIndex = defaultIndex;

      pXmlScenario = pXmlScenario->NextSiblingElement("climate_scenario");
   } // end of while (pXmlScenario != NULL)

   return(true);
} // end of LoadClimateScenariosXml() 


int ScenarioManager::SaveXml( LPCSTR filename )
   {
   if ( m_includeInSave == false )
      return 0;

   // open the file and write contents
   FILE *fp;
   fopen_s( &fp, filename, "wt" );
   if ( fp == NULL )
      {
      LONG s_err = GetLastError();
      CString msg = "Failure to open " + CString(filename) + " for writing.  ";
      Report::SystemErrorMsg( s_err, msg );
      return false;
      }

   bool useFileRef = ( m_loadStatus == 1  ? true : false );

   int count = SaveXml( fp, true, useFileRef );
   fclose( fp );

   return count;
   }


int ScenarioManager::SaveXml( FILE *fp, bool includeHdr, bool useFileRef )
   {
   ASSERT( fp != NULL );

   if ( includeHdr )
      fputs( "<?xml version='1.0' encoding='utf-8' ?>\n\n", fp );

      if ( useFileRef ) 
      {
      if ( m_importPath.IsEmpty() )
         {
#ifndef NO_MFC
         CFileDialog dlg( FALSE, _T("xml"), "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "XML Files|*.xml|All files|*.*||" );
         if ( dlg.DoModal() == IDCANCEL )
            return 0;
      
         m_importPath = dlg.GetPathName();
#else
	 return 0;
#endif
         }

      int loadStatus = m_loadStatus;
      m_loadStatus = 0;
      fprintf( fp, "<scenarios default='%i' file='%s' />\n", GetDefaultScenario(), (LPCTSTR) m_importPath );
      SaveXml( m_importPath );
      m_loadStatus = loadStatus;
      return GetCount();
      }

    fprintf( fp, "<scenarios default='%i'>\n", GetDefaultScenario() );

    for ( int i=0; i < GetCount(); i++ )
      {
      Scenario *pScenario = GetScenario( i );
      
      int isEditable = pScenario->m_isEditable ? 1 : 0;
      int isShared   = pScenario->m_isShared   ? 1 : 0;
      
//      fprintf( fp, "\t<scenario name='%s' \n\t\toriginator='%s' \n\t\tisEditable='%i' \n\t\tisShared='%i' \n\t\tlandscapeFeedbackWt='%g' \n\t\tactorValueWt='%g' \n\t\tpolicyPreferenceWt='%g' \n\t\tevalModelFreq='%i'>\n",
//         (LPCSTR) pScenario->m_name, (LPCSTR) pScenario->m_originator, isEditable, isShared, gpDoc->m_model.m_altruismWt, gpDoc->m_model.m_selfInterestWt, gpDoc->m_model.m_policyPrefWt,
//         gpDoc->m_model.m_evalFreq );
      fprintf( fp, "\t<scenario name='%s' \n\t\toriginator='%s' \n\t\tisEditable='%i' \n\t\tisShared='%i' \n\t\tpolicyPreferenceWt='%g' \n\t\tevalModelFreq='%i'>\n",
         (LPCSTR) pScenario->m_name, (LPCSTR) pScenario->m_originator, isEditable, isShared, gpModel->m_policyPrefWt, gpModel->m_evalFreq );

      fputs( "\t\t<description>\n\t\t\t", fp );
      if ( pScenario->m_description.GetLength() > 0 )
         fputs( pScenario->m_description, fp );
      fputs( "\n\t\t</description>\n", fp );


      fputs( "\t\t<policies>\n", fp );
      
      // policies next
      for ( int i=0; i < pScenario->GetPolicyCount(); i++ )
         {
         POLICY_INFO &pi = pScenario->GetPolicyInfo( i );
         fprintf( fp, "\t\t\t<policy id='%i' name='%s' inUse='%i' />\n", pi.policyID, (LPCSTR) pi.policyName, pi.inUse );
         }

      fputs( "\t\t</policies>\n", fp );
      
      // vars next
      fputs( "\t\t<vars>\n", fp );
      
      for ( int j=0; j < pScenario->GetVarCount(); j++ )
         {
         VAR_INFO &vi = pScenario->GetVarInfo( j );

         int inUse = vi.inUse ? 1 : 0;
         float paramLocation = 0;
         float paramScale = 0;
         float paramShape = 0;
         vi.paramLocation.GetAsFloat( paramLocation );
         vi.paramScale.GetAsFloat( paramScale );
         vi.paramShape.GetAsFloat( paramShape );


         switch( vi.vtype )
            {
            case V_META:
            case V_MODEL:
            case V_AP:
            case V_SCENARIO:
               {
               if ( vi.distType == MD_CONSTANT )
                  fprintf( fp, "\t\t\t<var vtype='%i' inUse='%i' name='%s' value='%f' />\n", (int) vi.vtype, inUse, (LPCTSTR) vi.name, paramLocation );
               else
                  fprintf( fp, "\t\t\t<var vtype='%i' inUse='%i' distType='%i' name='%s' paramLocation='%f' paramScale='%f' paramShape='%f' />\n",
                     (int) vi.vtype, inUse, (int) vi.distType, (LPCTSTR) vi.name, paramLocation, paramScale, paramShape );
               }
               break;

            default:
               ASSERT( 0 );
            }
         }
      fputs( "\t\t</vars>\n", fp );

      fputs( "\t</scenario>\n\n", fp );
      }

   fputs( "\t</scenarios>\n", fp );
   return GetCount();

   }


void ScenarioManager::AddPolicyToScenarios( Policy *pPolicy, bool inUse )
   {
   for ( int i=0; i < GetCount(); i++ )
      {
      Scenario *pScenario = GetScenario( i );
      pScenario->AddPolicyInfo( pPolicy, inUse );
      }
   }



void ScenarioManager::RemovePolicyFromScenarios( Policy *pPolicy )
   {
   for ( int i=0; i < GetCount(); i++ )
      {
      Scenario *pScenario = GetScenario( i );
      pScenario->RemovePolicyInfo( pPolicy );
      }
   }


