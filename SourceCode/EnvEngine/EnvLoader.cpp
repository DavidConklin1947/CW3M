#include "stdafx.h"

#ifdef NO_MFC
#include<no_mfc_files/NoMfcFiles.h>
#endif

#include "EnvLoader.h"
#include "EnvModel.h"
#include "DataManager.h"
#include "EnvConstants.h"
#include "Actor.h"
#include "Scenario.h"
#include "EnvMisc.h"
#include "..\Flow\Flow.h"

#include <MAP.h>

#include <Path.h>
#include <PathManager.h>
#include <QueryEngine.h>


#ifndef __ENV_ENGINE__
#include "../EnvView.h"
#include "../SpatialIndexDlg.h"
#include "../MapPanel.h"
#include <Maplayer.h>
#include <MapWnd.h>

extern MapLayer  *gpCellLayer;
extern CEnvView  *gpView;
extern MapPanel  *gpMapPanel;
#endif

using namespace nsPath;


EnvLoader::EnvLoader( void )
: m_pMap( NULL )
, m_pIDULayer( NULL )
, m_pModel( NULL )
, m_pPolicyManager( NULL )
, m_pActorManager( NULL )
, m_pScenarioManager( NULL )
, m_pQueryEngine( NULL )
   {
   }


EnvLoader::~EnvLoader()
   {

   }


int EnvLoader::LoadProject( LPCTSTR filename, Map *pMap, EnvModel *pModel, PolicyManager *pPolicyManager,
                 ActorManager *pActorManager, ScenarioManager *pScenarioManager  )
   {
   //AFX_MANAGE_STATE(AfxGetStaticModuleState());
   m_pMap    = pMap;
   m_pModel  = pModel;
   m_pPolicyManager = pPolicyManager;
   m_pActorManager = pActorManager;
   m_pScenarioManager = pScenarioManager;
   
   ASSERT( m_pMap             != NULL );
   ASSERT( m_pModel           != NULL );
   ASSERT( m_pPolicyManager   != NULL );
   ASSERT( m_pActorManager    != NULL );
   ASSERT( m_pScenarioManager != NULL );

   m_pModel->m_envContext.pActorManager  = m_pActorManager;
   m_pModel->m_envContext.pPolicyManager = m_pPolicyManager;
   
   //???Check
   //Report::errorMsgProc  = NULL; // ErrorReportProc;
   //Report::statusMsgProc = NULL; // StatusMsgProc;
   //Report::reportFlag    = NULL; // ERF_CALLBACK;      // disable onscreen messageboxes while loading

     //???Check
   //Clear();

   //???Check
   //InitDoc();

   // set up default paths.
   // first, ENVISION.EXE executable directory
   TCHAR _path[ MAX_PATH ];  // e.g. _path = "D:\Envision\src\x64\Debug\Envision.exe"
   _path[0] = NULL;
   int count = GetModuleFileName( NULL, _path, MAX_PATH );

   if ( count < 3 ) return -1;

   _path[0] = _path[1] = ' '; // blank out drive letter

   CPath appPath( _path, epcTrim | epcSlashToBackslash );
   appPath.RemoveFileSpec();           // e.g. c:\Envision - directory for executables
   PathManager::AddPath( appPath );

   // second, the directory of the envx file
   CPath envxPath( filename, epcTrim | epcSlashToBackslash );
   envxPath.RemoveFileSpec();
   PathManager::AddPath( envxPath );

   
   CString logFile( envxPath );
   logFile += "\\EnvisionStartup.log";
   Report::OpenFile( logFile );

   // third, the IDU path - this is determined below
   LPCTSTR paths = NULL;

   bool cellLayerInitialized = false;
   //TCHAR actorInitArgs[ 128 ];
   MAP_UNITS mapUnits = MU_UNKNOWN;

   // have xml string, start parsing
   TiXmlDocument doc;
   bool ok = doc.LoadFile( filename );

   bool loadSuccess = true;

   if ( ! ok )
      {
      CString msg( _T("Error reading input file ") );
      msg += filename;
      Report::ErrorMsg( doc.ErrorDesc(), msg );
      return -1;
      }

   // start interating through the document
   // general structure is of the form:
   // <Envision ver='x.y'>
   TiXmlElement *pXmlRoot = doc.RootElement();

   double ver = 0;
   LPCTSTR exportBmpCol = NULL;
   int logMsgLevel = 0;
    
   pXmlRoot->Attribute( "ver", &ver );
   
   TiXmlElement *pXmlSettings = pXmlRoot->FirstChildElement( _T("settings") );
   if ( pXmlSettings == NULL )
      Report::ErrorMsg( _T("Error finding <settings> section reading project file" ) );
   else
      {
      m_pActorManager->m_actorAssociations = 0;
      int loadSharedPolicies = 1;
      int debug = 0;
      int aim = 0;
      int noBuffer = 1;
      int multiRunDecadalMapsModulus = 10;
      int defaultPeriod = 25;
      int multiRunIterations = 20;
      int dynamicUpdate = 3;
      int spatialIndexDistance = 0;
      int startYear = 2010;
      int referenceStartYear = 2010;
      int yearsInStartingClimateAverages = 10;
      double policyPrefWt = 0.33;
      double areaCutoff = 0;
      int deltaAllocationSize = 0;
      int decisionElements = -1;
      int decisionMethod = -1; 
      int dynamicActors  = 1;
      int shuffleActorPolys = 0;
      int runParallel = 0;
      int addReturnsToBudget = 0;
      double exportBmpSize=0;
      int exportMaps = 0;   // deprecated
      int exportMapInterval = 0;
      int exportBmpInterval = 0;
      int exportOutputs = 0;
      int exportDeltas = 0;
      int collectPolicyData = 1;
      int discardMultiRunDeltas = 0;
      int startRunNumber = 0;
      int coldStartFlag = 0, spinupFlag = 0;
      int maxDaysInYear = -1;
      int useWaterYears = 0;
      LPCTSTR fieldInfoFiles       = NULL;
      LPCTSTR exportBmpCols        = NULL;
      LPCTSTR exportDeltaFieldList = NULL;
      LPCTSTR constraints          = NULL; 
      LPCTSTR _mapUnits            = NULL;
      CString study_area_name;

      XML_ATTR attrs[] = 
         { // attr                             type        address                  isReq checkCol
         { _T("studyAreaName"),              TYPE_CSTRING,  &study_area_name,       false,  0 },
         { _T("actorInitMethod"),            TYPE_INT,      &aim,                   false,  0 },
         { _T("actorAssociations"),          TYPE_INT,      &m_pActorManager->m_actorAssociations, false, 0 },
         { _T("loadSharedPolicies"),         TYPE_INT,      &loadSharedPolicies,    false,  0 },
         { _T("debug"),                      TYPE_INT,      &debug,                 false,  0 },
         { _T("startYear"),                  TYPE_INT,      &startYear,             false,  0 },
         { _T("referenceStartYear"),         TYPE_INT,      &referenceStartYear,    false,  0 },
         { _T("yearsInStartingClimateAverages"), TYPE_INT,  &yearsInStartingClimateAverages, false, 0 }, 
         { _T("logMsgLevel"),                TYPE_INT,      &logMsgLevel,           false,  0 },
         { _T("noBuffering"),                TYPE_INT,      &noBuffer,              false,  0 },
         { _T("multiRunDecadalMapsModulus"), TYPE_INT,      &multiRunDecadalMapsModulus, false, 0 },
         { _T("defaultPeriod"),              TYPE_INT,      &defaultPeriod,         false,  0 },
         { _T("multiRunIterations"),         TYPE_INT,      &multiRunIterations,    false,  0 },
         { _T("dynamicUpdate"),              TYPE_INT,      &dynamicUpdate,         false,  0 },
         { _T("spatialIndexDistance"),       TYPE_INT,      &spatialIndexDistance,  false,  0 },
         { _T("areaCutoff"),                 TYPE_DOUBLE,   &areaCutoff,            false,  0 },
         { _T("deltaAllocationSize"),        TYPE_INT,      &deltaAllocationSize,   false,  0 },
         { _T( "fieldInfoFiles"),            TYPE_STRING,   &fieldInfoFiles,        false,  0 },
         { _T("actorDecisionElements"),      TYPE_INT,      &decisionElements,      false,  0 },    
         { _T("actorDecisionMethod"),        TYPE_INT,      &decisionMethod,        false,  0 },
         { _T("dynamicActors"),              TYPE_INT,      &dynamicActors,         false,  0 },
         { _T("policyPreferenceWt"),         TYPE_DOUBLE,   &policyPrefWt,          false,  0 },
         { _T("shuffleActorPolys"),          TYPE_INT,      &shuffleActorPolys,     false,  0 },
         { _T("parallel"),                   TYPE_INT,      &runParallel,           false,  0 },
         { _T("addReturnsToBudget"),         TYPE_INT,      &addReturnsToBudget,    false,  0 },
         { _T("collectPolicyData" ),         TYPE_INT,      &collectPolicyData,     false,  0 },
         { _T("exportMapInterval"),          TYPE_INT,      &exportMapInterval,     false,  0 },
         { _T("exportBmpInterval"),          TYPE_INT,      &exportBmpInterval,     false,  0 },
         { _T("exportBmpPixelSize"),         TYPE_DOUBLE,   &exportBmpSize,         false,  0 },
         { _T("exportBmpCols"),              TYPE_STRING,   &exportBmpCols,         false,  0 },
         { _T("exportOutputs"),              TYPE_INT,      &exportOutputs,         false,  0 },
         { _T("exportDeltas"),               TYPE_INT,      &exportDeltas,          false,  0 },
         { _T("exportDeltaCols"),            TYPE_STRING,   &exportDeltaFieldList,  false,  0 }, 
         { _T("discardMultiRunDeltas"),      TYPE_INT,      &discardMultiRunDeltas, false,  0 }, 
         { _T("constraints"),                TYPE_STRING,   &constraints,           false,  0 },
         { _T("mapUnits"),                   TYPE_STRING,   &_mapUnits,             false,  0 },
         { _T("path"),                       TYPE_STRING,   &paths,                 false,  0 },
         { _T("startRunNumber"),             TYPE_INT,      &startRunNumber,        false,  0 },
         { _T("coldStart"),                  TYPE_INT,      &coldStartFlag,         false,  0 },
         { _T("spinup"),                  TYPE_INT,      &spinupFlag,         false,  0 },
      { _T("maxDaysInYear"),              TYPE_INT,      &maxDaysInYear,         false,  0 },
      { _T("useWaterYears"),              TYPE_INT,      &useWaterYears,         false,  0 },
      { NULL,                             TYPE_NULL,     NULL,                   false,  0 } };

      bool ok = TiXmlGetAttributes( pXmlSettings, attrs, filename );
      if ( ! ok )
         return -2;

      //???Check
      //if ( constraints != NULL )
      //   this->m_constraintArray.Add( constraints );  // NEEDS WORK!!!!!! - multiple constraints, check quer
      if (study_area_name.IsEmpty())
      {
         CString msg; msg.Format("Please specify studyAreaName=... in the <settings> block of the ENVX file.");
         Report::LogMsg(msg);
//         return(-2);
      }

      m_pModel->m_envContext.m_studyAreaName = SubstituteString("{studyAreaName}", study_area_name);
      m_pModel->m_envContext.m_substituteStrings.Add(&m_pModel->m_envContext.m_studyAreaName);

      m_pModel->m_envContext.coldStartFlag = coldStartFlag == 1;
      m_pModel->m_envContext.spinupFlag = spinupFlag == 1;
      if (m_pModel->m_envContext.coldStartFlag) m_pModel->m_envContext.spinupFlag = true;
      if (m_pModel->m_envContext.coldStartFlag || m_pModel->m_envContext.spinupFlag)
      {
         CString msg;
         msg.Format("coldStartFlag is %s and spinupFlag is %s",
            m_pModel->m_envContext.coldStartFlag ? "on" : "off", m_pModel->m_envContext.spinupFlag ? "on" : "off");
         Report::LogMsg(msg);
      }

      if (maxDaysInYear != -1)
      {
         CString msg;
         msg.Format("maxDaysInYear = %i. maxDaysInYear is usually determined by the selected climate scenario when a simulation is run. ", maxDaysInYear);
         Report::WarningMsg(msg);
      }
      m_pModel->m_envContext.m_maxDaysInYear = maxDaysInYear;

      m_pModel->m_envContext.m_useWaterYears = useWaterYears == 1 ? 1 : 0;

      m_pModel->m_shuffleActorIDUs = shuffleActorPolys ? true : false;
      m_pModel->m_debug = debug;

      if ( _mapUnits )
         {
         switch( _mapUnits[ 0 ] )
            {
            case 'f':      mapUnits = MU_FEET;     break;
            case 'm':      mapUnits = MU_METERS;   break;
            case 'd':      mapUnits = MU_DEGREES;  break;
            default:       mapUnits = MU_UNKNOWN;  break;
            }
         }

      //m_pModel->m_logMsgLevel = logMsgLevel;
      BufferOutcomeFunction::m_enabled = noBuffer ? false : true;
      m_pModel->m_pDataManager->multiRunDecadalMapsModulus = multiRunDecadalMapsModulus;
      m_pModel->m_dynamicUpdate = dynamicUpdate;
      m_pModel->m_allowActorIDUChange = dynamicActors ? true : false;
      
      if (startYear > 2099)
      {
         CString msg;
         msg.Format("LoadProject() startYear = %d is > 2099", startYear);
         Report::WarningMsg(msg);
      }
      m_pModel->m_startYear = startYear;

      m_pModel->m_referenceStartYear = referenceStartYear;
      m_pModel->m_envContext.m_yearsInStartingClimateAverages = yearsInStartingClimateAverages;
      m_pModel->m_yearsToRun = defaultPeriod;
      m_pModel->m_iterationsToRun = multiRunIterations;
      m_pModel->m_startRunNumber = startRunNumber;

      m_pModel->m_runParallel   = runParallel ? true : false;
      m_pModel->m_addReturnsToBudget = addReturnsToBudget ? true : false;
      //m_pModel->m_exportMaps = exportMaps ? true : false;
      m_pModel->m_exportMaps = exportMapInterval > 0 ? true : false;
      m_pModel->m_exportMapInterval = exportMapInterval;

      m_pModel->m_exportBmps = (exportBmpSize > 0 && exportBmpInterval > 0 ) ? true : false;
      m_pModel->m_exportBmpInterval = exportBmpInterval;
      m_pModel->m_exportBmpFields = exportBmpCols;
      m_pModel->m_exportBmpSize = (float) exportBmpSize;

      m_pModel->m_exportOutputs = exportOutputs ? true : false;
      m_pModel->m_exportDeltas  = exportDeltas ? true : false;
      m_pModel->m_discardMultiRunDeltas = discardMultiRunDeltas ? true : false;

      if ( exportDeltaFieldList != NULL )
         m_pModel->m_exportDeltaFields;

      m_pModel->m_collectPolicyData = collectPolicyData;

      // take care of cols after idu layer loaded

      if ( deltaAllocationSize > 0 )
         DeltaArray::m_deltaAllocationSize = deltaAllocationSize;
      
      if ( /*actorAltruismWt > 0 || actorSelfInterestWt > 0  || */ policyPrefWt  >= 0 )
         {
         //this->m_altruismWt = (float) actorAltruismWt;
         //this->m_selfInterestWt = (float) actorSelfInterestWt;
         m_pModel->m_policyPrefWt = (float) policyPrefWt;

         //::NormalizeWeights( this->m_altruismWt, this->m_selfInterestWt, this->m_policyPrefWt );
         }

      if ( decisionElements <= 0 )
         decisionElements = DE_ALTRUISM | DE_SELFINTEREST | DE_GLOBALPOLICYPREF;

      m_pModel->SetDecisionElements( decisionElements );

      if ( decisionMethod > 0 )
         m_pModel->m_decisionType = (DECISION_TYPE) decisionMethod;
      else
         m_pModel->m_decisionType = DT_PROBABILISTIC;

      m_pModel->m_spatialIndexDistance = spatialIndexDistance;
      m_pModel->m_areaCutoff = (float) areaCutoff;

#ifndef __ENV_ENGINE__
      if ( fieldInfoFiles )
         {
         TCHAR buffer[ 1024];
         lstrcpy( buffer, fieldInfoFiles );
         LPTSTR next;
         TCHAR *fiFile = _tcstok_s( buffer, _T(",\n"), &next );

         //???Check
         //while ( fiFile != NULL )
         //   {
         //   m_fieldInfoFileArray.Add( fiFile );
         //   fiFile = _tcstok_s( NULL, _T( ",\n" ), &next );
         //   }
         }
#endif
      if ( aim < AIM_NONE || aim >= AIM_END )
         Report::ErrorMsg("Invalid actorInitialization flag in .envx file - must be 0-4");

      m_pActorManager->m_actorInitMethod = (ACTOR_INIT_METHOD) aim;
      }  // end of: else ( pSettings != NULL )

   // next - layers
   TiXmlElement *pXmlLayers = pXmlRoot->FirstChildElement( _T("layers") );

   if ( pXmlLayers == NULL )
      Report::ErrorMsg( _T("Error finding <layers> section reading project file" ) );
   else
      {
      // iterate through layers
      TiXmlElement *pXmlLayer = pXmlLayers->FirstChildElement( _T("layer") );

      while ( pXmlLayer != NULL )
         {
         int type = 0;
         int data = 1;
         int records = -1;
         int labelSize = 0;
         CString path;
         LPCTSTR name=NULL, initField=NULL, overlayFields=NULL, color=NULL, fieldInfoFile=NULL,
            labelField=NULL, labelFont=NULL, labelColor=NULL, labelQuery=NULL;

         XML_ATTR layerAttrs[] = 
            { // attr                 type        address                isReq checkCol
	      { _T("name"),            TYPE_STRING,   (void*)&name,              true,   0 },
            { _T("path"),            TYPE_CSTRING,   (void*)&path,              true,   0 },
            { _T("initField"),       TYPE_STRING,   (void*)&initField,         false,  0 },
            { _T("overlayFields"),   TYPE_STRING,   (void*)&overlayFields,     false,  0 },
            { _T("color"),           TYPE_STRING,   (void*)&color,             true,   0 },
            { _T("fieldInfoFile"),   TYPE_STRING,   (void*)&fieldInfoFile,     false,  0 },
            { _T("labelField"),      TYPE_STRING,   (void*)&labelField,        false,  0 },
            { _T("labelFont"),       TYPE_STRING,   (void*)&labelFont,         false,  0 },
            { _T("labelSize"),       TYPE_INT,      &labelSize,         false,  0 },
            { _T("labelColor"),      TYPE_STRING,   (void*)&labelColor,        false,  0 },
            { _T("labelQuery"),      TYPE_STRING,   (void*)&labelQuery,        false,  0 },
            { _T("type"),            TYPE_INT,      &type,              true,   0 },
            { _T("records"),         TYPE_INT,      &records,           false,  0 },
            { _T("includeData"),     TYPE_INT,      &data,              false,  0 },
            { NULL,                  TYPE_NULL,     NULL,               false,  0 } };
		   		
         bool ok = TiXmlGetAttributes( pXmlLayer, layerAttrs, filename );
         if ( ! ok )
            return -3;

         ::ApplySubstituteStrings(path, m_pModel->m_envContext.m_substituteStrings);

         // parse color
         int red=0, green=0, blue=0;
         int count = sscanf_s( (TCHAR*)color, "%i,%i,%i", &red, &green, &blue );
         if ( count != 3 )
            {
            CString msg;
            msg.Format( "Misformed color information for layer %s", name );
            Report::ErrorMsg( msg );
            }
         
         bool loadFieldInfo = true;
         if ( fieldInfoFile != NULL && fieldInfoFile[ 0 ] != '\0' )
            loadFieldInfo = false;
         
         int layerIndex = LoadLayer( m_pMap, name, path, type, data, red, green, blue, 0, records, initField, overlayFields, loadFieldInfo );
         if ( layerIndex < 0 )
            {
            CString msg;
            msg.Format( "Bad %s layer %s in input file, %s; please fix the problem before continuing.", 
               (LPCSTR)name, (LPCSTR)path, (LPCSTR) filename );
            Report::ErrorMsg( msg );
            return -4;
            }
         //else
         //   {
         //   CString msg;
         //   msg.Format( "Loaded layer %s", path );
         //   Report::InfoMsg( msg );
         //   }

         // if IDU, add path for shape file to the PathManager
         if ( layerIndex == 0 )
            {
            m_pIDULayer = (IDUlayer *)m_pMap->GetLayer( 0 );
            if (strcmp(m_pIDULayer->m_name, "IDU") != 0)
            {
               CString msg = "LoadProject(): Please make the IDU <layer> the first <layer> specified in your ENVX file.";
               Report::ErrorMsg(msg);
               return(-5);
            }
            //if ( m_pQueryEngine != NULL )
            //   delete m_pQueryEngine;
            //
            //m_pQueryEngine = new QueryEngine( m_pIDULayer );
            //m_pModel->m_pQueryEngine = m_pQueryEngine;

            m_pModel->SetIDULayer( m_pIDULayer ); // creates query engine for IDU layer
            m_pQueryEngine = m_pModel->m_pQueryEngine;
            PolicyManager::m_pQueryEngine = m_pQueryEngine;

            m_pIDULayer->SetNoOutline();

         // No better time than now to execute this.

            if ( m_pModel->m_spatialIndexDistance > 0 && m_pIDULayer->LoadSpatialIndex( NULL, (float) m_pModel->m_spatialIndexDistance ) < 0 )
               {
#ifndef __ENV_ENGINE__
               SpatialIndexDlg dlg;
               gpCellLayer = m_pIDULayer;
               dlg.m_maxDistance = m_pModel->m_spatialIndexDistance;

               if ( dlg.DoModal() == IDOK )
                  m_pModel->m_spatialIndexDistance = dlg.m_maxDistance;
#endif
               }

            CString fullPath;
            PathManager::FindPath( path, fullPath );

            CPath _fullPath( fullPath );
            _fullPath.RemoveFileSpec();

            PathManager::AddPath( _fullPath );

            m_pIDULayer = (IDUlayer *)m_pMap->GetLayer( 0 );
            bool found_columns = m_pIDULayer->FindIDUattributeCols();
            if (!found_columns)
               {
               CString msg = "LoadProject(): FindIDUattributeCols() returned false.";
               Report::ErrorMsg(msg);
               }
            }

         CString layerName = name;
         if (layerName == "Subcatchment") m_pModel->m_envContext.pSubcatchmentLayer = m_pMap->GetLayer(layerIndex);
         else if (layerName == "Node") m_pModel->m_envContext.pNodeLayer = m_pMap->GetLayer(layerIndex);
         else if (layerName == "Link") m_pModel->m_envContext.pLinkLayer = m_pMap->GetLayer(layerIndex);
         else if (layerName == "HRU") m_pModel->m_envContext.pHRUlayer = m_pMap->GetLayer(layerIndex);
         else if (layerName == "Reach") m_pModel->m_envContext.pReachLayer = m_pMap->GetLayer(layerIndex);
         else if (layerName == "Groundwater") m_pModel->m_envContext.pGWlayer = m_pMap->GetLayer(layerIndex);

         // take care of labels stuff
         MapLayer *pMapLayer = m_pMap->GetLayer( layerIndex );
         if ( labelField != NULL )
            {
            int labelCol = pMapLayer->GetFieldCol( labelField );

            if ( labelCol >= 0 )
               {
               pMapLayer->SetLabelField( labelCol );
              
               if ( labelFont == NULL )
                  pMapLayer->SetLabelFont( _T("Arial" ) );
               else
                  pMapLayer->SetLabelFont( labelFont );

              float _labelSize = 120;
              if ( labelSize > 0 )
                 _labelSize = (float) labelSize;

              pMapLayer->SetLabelSize( _labelSize );

               int count = sscanf_s( labelColor, "%i,%i,%i", &red, &green, &blue );
               if ( count != 3 )
                  {
                  CString msg;
                  msg.Format( "Misformed label color information for layer %s", name );
                  Report::ErrorMsg( msg );
                  }
               else
                  pMapLayer->SetLabelColor( RGB( red, green, blue ) );

               pMapLayer->ShowLabels();

               if ( labelQuery != NULL )
                  pMapLayer->m_labelQueryStr = labelQuery;
               }
            }

         if ( fieldInfoFile != NULL && fieldInfoFile[ 0 ] != NULL )
            this->LoadFieldInfoXml( pMapLayer, fieldInfoFile );
         
         // look for any join tables defined for this layer
         TiXmlElement *pXmlJoinTable = pXmlLayer->FirstChildElement( _T("join_table") );
         while ( pXmlJoinTable != NULL )
            {
            LPTSTR path      = NULL;
            LPTSTR layerCol  = NULL;
            LPTSTR joinCol   = NULL;
            XML_ATTR joinAttrs[] = {
               // attr         type           address   isReq  checkCol
               { "path",       TYPE_STRING,  &path,     true,   0 },
               { "layerCol",   TYPE_STRING,  &layerCol, true,   0 },
               { "joinCol",    TYPE_STRING,  &joinCol,  true,   0 },
               { NULL,         TYPE_NULL,    NULL,      false,  0 } };
         
            ok = TiXmlGetAttributes( pXmlJoinTable, joinAttrs, filename );
            if ( ok )
               {
               // find file
               CString fullPath;
               if ( PathManager::FindPath( path, fullPath ) < 0 )
                  {
                  CString msg;
                  msg.Format( "Join table '%s' not found.  This table will not be loaded.", path );
                  Report::ErrorMsg( msg );
                  }
               else
                  {
                  int colLayer = pMapLayer->GetFieldCol( layerCol );

                  if ( colLayer < 0 )
                     {
                     CString msg;
                     msg.Format( "Join Table: Layer field '%s' not found in layer '%s'.  This table will not be loaded", 
                        layerCol, (LPCTSTR) pMapLayer->m_name );
                     Report::ErrorMsg( msg );
                     }
                  else
                     {
                     DbTable *pJoinTable = new DbTable( DOT_VDATA );
                     pJoinTable->LoadDataCSV( fullPath );

                     // look for join col
                     int colJoin = pJoinTable->GetFieldCol( joinCol );

                     if ( colJoin < 0 )
                        {
                        CString msg;
                        msg.Format( "Join Table: Join field '%s' not found in Join Table '%s'.  This table will not be loaded", 
                           joinCol, (LPCTSTR) fullPath );
                        Report::ErrorMsg( msg );

                        delete pJoinTable;
                        }
                     else  // finally - success
                        {
                        pMapLayer->m_pDbTable->AddJoinTable( pJoinTable, layerCol, joinCol, true );
                        }
                     }
                  }
               }

            pXmlJoinTable = pXmlJoinTable->NextSiblingElement( _T("join_table" ) );
            }

         pXmlLayer = pXmlLayer->NextSiblingElement( _T("layer" ) );
         }

      // done loading layer, load background if defined
      //<layers bkgr='\Envision\StudyAreas\WW2100\transformed_warped_ww2100.tif' left='' top='' right='' bottom=''>
      LPTSTR bkgrPath=NULL, left=NULL, top=NULL, right=NULL, bottom=NULL, prjFile=NULL;
      // any additional layers info provided?
	   XML_ATTR layersAttrs[] = {
         // attr         type           address   isReq  checkCol
         { "bkgr",       TYPE_STRING,  &bkgrPath, false,   0 },
         { "left",       TYPE_STRING,  &left,     false,   0 },
         { "right",      TYPE_STRING,  &right,    false,   0 },
         { "top",        TYPE_STRING,  &top,      false,   0 },
         { "bottom",     TYPE_STRING,  &bottom,   false,   0 },
         { "prjFile",    TYPE_STRING,  &prjFile,  false,   0 },
         { NULL,         TYPE_NULL,    NULL,      false,   0 } };

      ok = TiXmlGetAttributes( pXmlLayers, layersAttrs, filename );
      if ( ok )
         {
         if ( bkgrPath != NULL )
            {
            REAL _left, _top, _right, _bottom;
            m_pIDULayer->GetExtents( _left, _right, _bottom, _top );

            if ( left != NULL && *left != NULL )
               _left = (float) atof( left );
            
            if ( top != NULL && *top != NULL )
               _top = (float) atof( top );

            if ( right != NULL && *right != NULL )
               _right = (float) atof( right );

            if ( bottom != NULL && *bottom != NULL )
               _bottom = (float) atof( bottom );

            m_pMap->m_bkgrPath = bkgrPath;
            m_pMap->m_bkgrLeft   = _left;
            m_pMap->m_bkgrTop    = _top;
            m_pMap->m_bkgrRight  = _right;
            m_pMap->m_bkgrBottom = _bottom;

#ifndef __ENV_ENGINE__
            //m_pMap->LoadBkgrImage(bkgrPath, _left, _top, _right, _bottom );
#endif
            }
         }
      }

   // IDU loaded, so process path setting if anything defined
   if ( paths != NULL )
      {
      m_pModel->m_searchPaths = paths;

      TCHAR *buffer = new TCHAR[ lstrlen( paths ) + 1 ];
      lstrcpy( buffer, paths );

      // parse the path, adding as you go.
      LPTSTR next;
      TCHAR *token = _tcstok_s( buffer, _T(",;|"), &next );
      while ( token != NULL )
         {
         PathManager::AddPath( token );
         token = _tcstok_s( NULL, _T( ",;|" ), &next );
         }

      delete [] buffer;
      }

   // next - lulcTree
   Report::StatusMsg("Loading LULC Tree information");
   m_pModel->m_lulcTree.m_path.Empty(); // = filename;
   TiXmlNode *pXmlLulcTree = pXmlRoot->FirstChild(_T("lulcTree"));
   int lulcCount = m_pModel->m_lulcTree.LoadXml(pXmlLulcTree, false);

   // this has to wait until the LULC tree is loaded
   if ( !cellLayerInitialized)
      {
#ifndef NO_MFC
      CWaitCursor c;
#endif
      m_pModel->StoreIDUCols();    // requires models needed to be loaded before this point
      m_pModel->VerifyIDULayer();     // requires StoreCellCols() to be call before this is called, but not models
      cellLayerInitialized = true;
      }





#ifndef __ENV_ENGINE__
   // visualizers
   TiXmlElement *pVisualizers = pXmlRoot->FirstChildElement( _T("visualizers") );
   if ( pVisualizers != NULL )
      {
      // iterate through layers
      TiXmlElement *pViz = pVisualizers->FirstChildElement( _T("visualizer") );

      while ( pViz != NULL )
         {
         LPCTSTR name     = pViz->Attribute( _T("name") );
         LPCTSTR path     = pViz->Attribute( _T("path") );
         LPCTSTR type     = pViz->Attribute( _T("type") );
         LPCTSTR initInfo = pViz->Attribute( _T("initInfo") );
         LPCTSTR use      = pViz->Attribute( _T("use" ) );

         // load visualizer stuff here
         CString msg( "Loading Visualizer: " );
         msg += name;
         msg += " (";
         msg += path;
         msg += ")";
         Report::InfoMsg( msg );

         int _type = atoi( type );
         int _use  = atoi( use );
         LoadVisualizer( name, path, _use, _type, initInfo );

         msg.Format( "Loaded Visualizer %s (%s)", name, path );
         Report::InfoMsg( msg );
         
         pViz = pViz->NextSiblingElement( _T("visualizer" ) );
         }
      }
#endif

   // next - app vars
   TiXmlElement *pXmlAppVars = pXmlRoot->FirstChildElement( _T("app_vars") );
   if ( pXmlAppVars != NULL )
      {
      // iterate through layers
      TiXmlElement *pXmlAppVar = (TiXmlElement*) pXmlAppVars->FirstChildElement( _T("app_var") );

      while ( pXmlAppVar != NULL )
         {
         LPCTSTR name  = NULL;
         LPCTSTR desc  = NULL;
         LPCTSTR value = NULL;
         LPCTSTR col = NULL;
         int timing = 3;

         XML_ATTR appVarAttrs[] = 
            { // attr          type        address    isReq checkCol
            { "name",        TYPE_STRING,   &name,     true,   0 },
            { "description", TYPE_STRING,   &desc,     false,  0 },
            { "value",       TYPE_STRING,   &value,    false,  0 },
            { "col",         TYPE_STRING,   &col,      false,  CC_AUTOADD },
            { "timing",      TYPE_INT,      &timing,   false,  0 },
            { NULL,          TYPE_NULL,     NULL,      false,  0 } };
  
         bool ok = TiXmlGetAttributes( pXmlAppVar, appVarAttrs, filename, m_pIDULayer );

         if ( name == NULL )
            {
            CString msg;
            //if ( name != NULL )
            //   msg.Format( _T("Error reading <app_var> '%s': missing value attribute" ), name );
            //else
               msg = _T("Error reading <app_var>: missing name attribute" );

            Report::ErrorMsg( msg );
            }
         else
            {
            AppVar *pVar = new AppVar( name, desc, value );   // needs expression support

            pVar->m_avType = AVT_APP;
            
            //if ( value != NULL && value[ 0 ] != NULL )      // note: timing = 1 until set below
            //   pVar->Evaluate();

            pVar->m_timing = timing;

            if ( pVar->m_timing == -1 ) // hack for now
               pVar->SetValue( VData( (float) atof( value ) ) );

            m_pModel->AddAppVar( pVar, true );    // create and evaluate using map expression evaluator
            }

         pXmlAppVar = pXmlAppVar->NextSiblingElement( _T("app_var" ) );
         }
      }  // end of: if ( pAppVars != NULL )


   // next - autonomous processes
   TiXmlElement *pXmlProcesses = pXmlRoot->FirstChildElement( _T("autonomous_processes") );
   if ( pXmlProcesses == NULL )
      Report::ErrorMsg( _T("Error finding <autonomous_processes> section reading project file" ) );
   else
      {
      // iterate through layers
      TiXmlElement *pXmlProcess = pXmlProcesses->FirstChildElement( _T("autonomous_process") );
 
      while ( pXmlProcess != NULL )
         {
         int id = -1;
         int use = 1;
         int freq = 1;
         int timing = 0;
         int sandbox = 0;
         int initRunOnStartup = 0;

         LPCTSTR name = pXmlProcess->Attribute( _T("name") );

         CString path = pXmlProcess->Attribute( _T("path") );
         ::ApplySubstituteStrings(path, m_pModel->m_envContext.m_substituteStrings);

         LPCTSTR fieldName  = pXmlProcess->Attribute( _T("fieldName") );

         CString initInfo = pXmlProcess->Attribute(_T("initInfo"));
         ::ApplySubstituteStrings(initInfo, m_pModel->m_envContext.m_substituteStrings);

         pXmlProcess->Attribute( _T("id"), &id );
         pXmlProcess->Attribute( _T("use"), &use );
         pXmlProcess->Attribute( _T("freq"), &freq );
         pXmlProcess->Attribute( _T("timing"), &timing );
         pXmlProcess->Attribute( _T("sandbox"), &sandbox );
         pXmlProcess->Attribute( _T("initRunOnStartup"), &initRunOnStartup );

         // load model stuff here
         CString msg( "Loading " );
         msg += name;
         msg += "(";
         msg += path;
         msg += ")";
         Report::InfoMsg( msg );

         int useCol = 0;
         if ( fieldName != NULL && lstrlen( fieldName ) > 0 )
            useCol = 1;

         bool ok = LoadAP( m_pModel, name, path, id, use, timing, sandbox, useCol, fieldName, initInfo, NULL, initRunOnStartup );
                  
         if ( ok )
            msg.Format( "Loaded Autonomous Process %s (%s)", name, path );
         else
            msg.Format( "Unable to Load Autonomous Process %s (%s)", name, path );

         Report::InfoMsg( msg );
         pXmlProcess = pXmlProcess->NextSiblingElement( _T("autonomous_process" ) );
         }
      }

	// next - models
	TiXmlElement *pXmlModels = pXmlRoot->FirstChildElement(_T("models"));
	if (pXmlModels == NULL)
		Report::ErrorMsg(_T("Error finding <models> section reading project file"));
	else
		{
		// iterate through layers
		TiXmlElement *pXmlModel = pXmlModels->FirstChildElement(_T("model"));

		while (pXmlModel != NULL)
			{
			int id = -1;
			int use = 1;
			int freq = 1;
			//int decisionUse = 3;
			int showInResults = 1;
			int initRunOnStartup = 0;
			double gain = 1;
			double offset = 0;

			LPCTSTR name = pXmlModel->Attribute(_T("name"));
			LPCTSTR path = pXmlModel->Attribute(_T("path"));
			LPCTSTR fieldName = pXmlModel->Attribute(_T("fieldName"));
			LPCTSTR initInfo = pXmlModel->Attribute(_T("initInfo"));

			pXmlModel->Attribute(_T("id"), &id);
			pXmlModel->Attribute(_T("use"), &use);
			pXmlModel->Attribute(_T("freq"), &freq);
			//pXmlModel->Attribute( _T("decisionUse"), &decisionUse );
			pXmlModel->Attribute(_T("showInResults"), &showInResults);
			pXmlModel->Attribute(_T("gain"), &gain);
			pXmlModel->Attribute(_T("offset"), &offset);
			pXmlModel->Attribute(_T("initRunOnStartup"), &initRunOnStartup);

			LPCTSTR dependencyNames = pXmlModel->Attribute(_T("dependencies"));

			// load model stuff here
			CString msg("Loading model: ");
			msg += name;
			msg += " (";
			msg += path;
			msg += ")";
			Report::InfoMsg(msg);

			int useCol = 0;
			if (fieldName != NULL && lstrlen(fieldName) > 0)
				useCol = 1;

			bool ok = LoadModel(m_pModel, name, path, id, use, showInResults, /*decisionUse,*/ useCol, fieldName, initInfo, dependencyNames, initRunOnStartup, (float)gain, (float)offset);

			if (ok)
				msg.Format("Loaded Eval Model %s (%s)", name, path);
			else
				msg.Format("Unable to Load Eval Model %s (%s)", name, path);

			Report::InfoMsg(msg);

			pXmlModel = pXmlModel->NextSiblingElement(_T("model"));
			}
		}

	// next - metagoals
   TiXmlElement *pXmlMetagoals = pXmlRoot->FirstChildElement( _T("metagoals") );
   if ( pXmlMetagoals == NULL )
      Report::ErrorMsg( _T("Error finding <metagoals> section reading project file" ) );
   else
      {
      // iterate through layers
      TiXmlElement *pXmlMetagoal = pXmlMetagoals->FirstChildElement( _T("metagoal") );
 
      while ( pXmlMetagoal != NULL )
         {
         int decisionUse = 1;
         LPCTSTR name = pXmlMetagoal->Attribute( _T("name") );
         LPCTSTR model = pXmlMetagoal->Attribute( _T("model") );
         pXmlMetagoal->Attribute( _T("decision_use"), &decisionUse );
         ENV_EVAL_MODEL *pModel = NULL;

         if ( decisionUse <= 0 || decisionUse > 3 )
            {
            CString msg( _T("Error reading metagoal <") );
            msg += name;
            msg += _T( "> - 'decision_use' attribute must be 1,2 or 3.  This metagoal will be ignored." );
            Report::ErrorMsg( msg );
            continue;
            }

         if ( decisionUse & DU_ALTRUISM )
            {
            if ( model == NULL )  // look for corresponding model
               model = name;

            pModel = EnvModel::FindModelInfo( model );
            if ( pModel == NULL )
               {
               CString msg( _T( "Error finding evalative model <") );
               msg += model;
               msg += _T("> when reading metagoal. You must provide an appropriate model reference.  Altruistic decision-making will be disabled for this metagoal");
               Report::ErrorMsg( msg );
               
               if ( decisionUse & DU_SELFINTEREST )
                  decisionUse = DU_SELFINTEREST;
               else
                  decisionUse = 0;
               }
            }

         if ( decisionUse > 0 )
            {
            METAGOAL *pMeta = new METAGOAL;
            pMeta->name = name;
            pMeta->pEvalModel = pModel;
            pMeta->decisionUse = decisionUse;
            m_pModel->AddMetagoal( pMeta );
            }

         pXmlMetagoal = pXmlMetagoal->NextSiblingElement( _T("metagoal" ) );
         }
      }


   // basic setting loaded, take care of everything else
   if ( m_pIDULayer == NULL )
      return -4;
   
   // fixup exportBmpCol is needed
   //if ( exportBmpCol != NULL )
   //   this->m_exportBmpCol = m_pIDULayer->GetFieldCol( exportBmpCol );    

   //for ( int i=0; i < m_fieldInfoFileArray.GetSize(); i++ )
   //   {
   //   CString msg( "Loading Field Information File: " );
   //   msg += m_fieldInfoFileArray[ i ];
   //   Report::StatusMsg( msg );
   //   LoadFieldInfoXml( NULL, m_fieldInfoFileArray[ i ] );
   //   }

   Report::StatusMsg( "Finished loading Field Information - Reconciling data types..." );

   //ReconcileDataTypes();      // require map layer to be loaded, plus map field infos?
   SetDependencies();

   // next - policies 
   Report::StatusMsg( "Loading Policies..." );
   m_pPolicyManager->m_path = filename;   // envx
   TiXmlNode *pXmlPolicies = pXmlRoot->FirstChild( _T("policies") ); 
   int policyCount = m_pPolicyManager->LoadXml( pXmlPolicies, false /* not appending*/ );

   CString msg;
   msg.Format( "Loaded %i policies from %s", policyCount, m_pPolicyManager->m_path );
   Report::InfoMsg( msg );
   //gpView->m_policyEditor.ReloadPolicies( false );

   // next - actors
   Report::StatusMsg( "Loading Actors..." );   
   if ( m_pActorManager->m_actorInitMethod != AIM_NONE )
      {
      m_pActorManager->m_path.Empty(); // = filename;
      TiXmlNode *pXmlActors = pXmlRoot->FirstChild( _T("actors") );
      int actorCount = m_pActorManager->LoadXml( pXmlActors, false );  // creates groups only

      if ( m_pActorManager->m_path.IsEmpty() )
         msg.Format( "Loaded %i actors from %s", actorCount, filename );
      else
         msg.Format( "Loaded %i actors from %s", actorCount, (PCTSTR) m_pActorManager->m_path );
      Report::InfoMsg( msg );
      }
   
   m_pActorManager->CreateActors();             // this makes the actors from the groups
   

   // next, RtViews
#ifndef __ENV_ENGINE__

   TiXmlElement *pXmlViews = pXmlRoot->FirstChildElement( _T("views") );
   if ( pXmlViews != NULL )
      {
      TiXmlElement *pXmlPage = pXmlViews->FirstChildElement( _T("page") );

      while ( pXmlPage != NULL )
         {
         LPTSTR name;
         XML_ATTR pageAttrs[] = {
            // attr                 type          address    isReq  checkCol
            { "name",               TYPE_STRING,  &name,     true,   0 },
            { NULL,                 TYPE_NULL,     NULL,     false,  0 } };
            
         ok = TiXmlGetAttributes( pXmlPage, pageAttrs, filename );
         if ( ! ok )
            {
            CString msg; 
            msg.Format( _T("Envision: Misformed <page> element reading input file %s"), filename );
            Report::ErrorMsg( msg );
            }
         else
            {
            // process page
            RtViewPage *pPage = gpView->m_viewPanel.AddPage( name );
            TiXmlElement *pXmlElement = pXmlPage->FirstChildElement( _T("element") );
      
            while ( pXmlElement != NULL )
               {
               LPTSTR name=NULL, type=NULL, col=NULL;
               int left=0, top=0, right=0, bottom=0;

               XML_ATTR elemAttrs[] = {
                  // attr          type          address    isReq  checkCol
                  { "name",        TYPE_STRING,  &name,     true,   0 },
                  { "left",        TYPE_INT,     &left,     true,   0 },
                  { "right",       TYPE_INT,     &right,    true,   0 },
                  { "top",         TYPE_INT,     &top,      true,   0 },
                  { "bottom",      TYPE_INT,     &bottom,   true,   0 },
                  { "type",        TYPE_STRING,  &type,     true,   0 },
                  { "col",         TYPE_STRING,  &col,      false,  0 },
                  //{ "layer",       TYPE_STRING,  &col,      false,  0 },
                  { NULL,          TYPE_NULL,     NULL,     false,  0 } };
                  
               ok = TiXmlGetAttributes( pXmlElement, elemAttrs, filename );
               if ( ! ok )
                  {
                  RTV_TYPE t = RTVT_MAP;
                  if ( *type == 'v' )
                     t = RTVT_VISUALIZER;

                  RtViewWnd *pView = NULL;
                  CRect rect( left, top, right, bottom );
                  switch( t )
                     {
                     case RTVT_MAP:
                        {
                        int _col= m_pIDULayer->GetFieldCol( col );

                        if ( _col >= 0 )
                           pView = pPage->AddView( t, _col, 0, rect );
                        }
                        break;

                     case RTVT_VISUALIZER:
                        break;
                     }
                  }

               pXmlElement = pXmlElement->NextSiblingElement( _T("element") );
               }
            }

         pXmlPage = pXmlPage->NextSiblingElement( _T("page") );
         }
      }  // end of: if ( pXmlView != NULL );

#endif

#ifndef __ENV_ENGINE__

   // next, Zooms
   TiXmlElement *pXmlZooms = pXmlRoot->FirstChildElement( _T("zooms") );
   if ( pXmlZooms != NULL )
      {
      LPCTSTR defaultZoom = pXmlZooms->Attribute( "default" );
      
      TiXmlElement *pXmlZoom = pXmlZooms->FirstChildElement( _T("zoom") );

      while ( pXmlZoom != NULL )
         {
         LPTSTR name = NULL;

         float xMin, yMin, xMax, yMax;

         XML_ATTR zoomAttrs[] = {
            // attr      type          address    isReq  checkCol
            { "name",    TYPE_STRING,  &name,     true,   0 },
            { "left",    TYPE_FLOAT,   &xMin,     true,   0 },
            { "right",   TYPE_FLOAT,   &xMax,     true,   0 },
            { "bottom",  TYPE_FLOAT,   &yMin,     true,   0 },
            { "top",     TYPE_FLOAT,   &yMax,     true,   0 },
            { NULL,      TYPE_NULL,     NULL,     false,  0 } };
            
         ok = TiXmlGetAttributes( pXmlZoom, zoomAttrs, filename );
         if ( ! ok )
            {
            CString msg; 
            msg.Format( _T("Envision: Misformed <zoom> element reading input file %s"), filename );
            Report::ErrorMsg( msg );
            }
         else
            {
            // process page
            int index = gpView->AddZoom( name, xMin, yMin, xMax, yMax );

            if ( defaultZoom != NULL && lstrcmp( defaultZoom, name ) == 0 )
               {
               gpView->m_pDefaultZoom = gpView->GetZoomInfo( index );
               gpView->SetZoom( index );
               }
            }

         pXmlZoom = pXmlZoom->NextSiblingElement( _T("zoom") );
         }
      }  // end of: if ( pXmlZoom != NULL );
#endif

   // next, probes
   /*
   TiXmlNode *pXmlProbes = pXmlRoot->FirstChild( _T("probes") );
   if ( pXmlProbes != NULL )
      {
      TiXmlNode *pXmlProbe = pXmlProbes->FirstChild( "probe" );

      while ( pXmlProbe != NULL )
         {
         LPCTSTR type;
         int id;
         
         XML_ATTR attrs[] = {
            // attr     type          address   isReq  checkCol
            { "type",   TYPE_STRING,   &type,    true,     0 },
            { "id",     TYPE_INT,      &id,      true,     0 },
            { NULL,     TYPE_NULL,     NULL,     false,    0 } };

         ok = TiXmlGetAttributes( pXmlProbe->ToElement(), attrs, filename );
         if ( ! ok )
            {
            CString msg; 
            msg.Format( _T("Misformed probe element reading <probe> attributes in input file %s"), filename );
            Report::ErrorMsg( msg );
            }
         else
            {
            Policy *pPolicy = m_pPolicyManager->GetPolicyFromID( id );

            if ( pPolicy == NULL )
               {
               CString msg; 
               msg.Format( _T("Unable to probe policy '%i' reading %s - this probe will be ignored"),
                  id, filename );
               Report::ErrorMsg( msg );
               }
            else
               {
               Probe *pProbe = new Probe( pPolicy );
               m_pModel->AddProbe( pProbe );
               }
            }
         pXmlProbe = pXmlProbe->NextSibling( "probe" );
         }
      }
      */
   // set all layers MAP_UNITS
   m_pMap->m_mapUnits = mapUnits;

   for ( int i=0; i < m_pMap->GetLayerCount(); i++ )
      {
      MapLayer *pLayer = m_pMap->GetLayer( i ) ;
      pLayer->m_mapUnits = mapUnits;

      if ( i == 0 ) // first layer?
         {
         // classify data;
         if ( m_pModel->m_lulcTree.GetLevels() > 0 )
            {
            int col = pLayer->GetFieldCol( EnvModel::m_lulcTree.GetFieldName( 1 ) );

            if ( col < 0 )
               col = pLayer->m_pDbTable->AddField( _T("LULC_A"), TYPE_INT, true );

            ASSERT( col >= 0 );
            pLayer->SetActiveField( col );
            Report::StatusMsg( "Setting up map layer bins..." );
            Map::SetupBins( m_pMap, 0, -1 );
            }
         }
      }
 
   // next, add output directory (e.g. projPath= "C:\Envision\MyProject\Outputs" )
   CString outputPath = PathManager::GetPath( PM_PROJECT_DIR );  // note: always terminated with a '\'
   outputPath += "Outputs\\";
   PathManager::AddPath( outputPath ); // note: this will get scenario name appended below, after scenarios loaded
   
   // next, initialize models and visualizers
   Report::StatusMsg( "Initializing Models and Processes..." );
   m_pModel->InitModels();     // EnvContext gets set up here

   if ( m_pModel->m_decisionElements & DE_SOCIALNETWORK )
      m_pModel->InitSocialNetwork();

   InitVisualizers();

   // next - scenarios
   Report::StatusMsg( "Loading Scenarios..." );   
   Report::LogMsg( "Loading Scenarios" );

   TiXmlElement* pXmlScenarios = pXmlRoot->FirstChildElement("scenarios");

   CString simulation_scenarios_file;
   int default_simulation_scenario = 0;
   CString climate_scenarios_file;
   int default_climate_scenario = 0;

   XML_ATTR scenario_attrs[] = {
      {"simulation_scenarios_file", TYPE_CSTRING, &simulation_scenarios_file, false, 0},
      {"default_simulation_scenario", TYPE_INT, &default_simulation_scenario, false, 0},
      {"climate_scenarios_file", TYPE_CSTRING, &climate_scenarios_file, true, 0},
      {"default_climate_scenario", TYPE_INT, &default_climate_scenario, false, 0},
      { NULL,                      TYPE_NULL,     NULL,                                false,   0 } };

   ok = TiXmlGetAttributes(pXmlScenarios, scenario_attrs, filename, NULL);
   ::ApplySubstituteStrings(simulation_scenarios_file, m_pModel->m_envContext.m_substituteStrings);
   ::ApplySubstituteStrings(climate_scenarios_file, m_pModel->m_envContext.m_substituteStrings);

   if (default_simulation_scenario < 0) default_simulation_scenario = 0;
   if (default_climate_scenario < 0) default_climate_scenario = 0;

   m_pScenarioManager->m_path.Empty(); // = filename;
   int scenario_count = 0;
   bool simulation_scenarios_loaded = false;
   simulation_scenarios_loaded = !simulation_scenarios_file.IsEmpty() && m_pScenarioManager->LoadXml(simulation_scenarios_file);
   if (simulation_scenarios_loaded)
   {
      scenario_count = m_pScenarioManager->GetCount();
      msg.Format("Loaded %i scenarios from %s", scenario_count, filename);
      Report::InfoMsg(msg);
   }
   else 
   {
      Scenario* pScenario = new Scenario("Default Scenario");
      m_pScenarioManager->AddScenario(pScenario);
      m_pScenarioManager->SetDefaultScenario(0);
   }

   if (default_simulation_scenario >= scenario_count) default_simulation_scenario = scenario_count - 1;

   m_pScenarioManager->SetDefaultScenario(default_simulation_scenario);
   m_pModel->SetScenario(m_pScenarioManager->GetScenario(default_simulation_scenario));
 
   // reset output path to correct directory for output
   // output directory (e.g. C:\CW3M_McKenzie_0.4.2\McKenzie\Outputs\Baseline )
   outputPath = PathManager::GetPath(PM_IDU_DIR);
   outputPath += "Outputs\\";
   Scenario* pScenario = m_pScenarioManager->GetScenario(default_simulation_scenario);
   if (pScenario != NULL) outputPath += m_pScenarioManager->GetScenario(default_simulation_scenario)->m_name;
   PathManager::SetPath(PM_OUTPUT_DIR, outputPath);

   SHCreateDirectoryEx(NULL, outputPath, NULL);

   // Now load climate scenarios.
   FlowContext * pFlowContext = pModel->m_envContext.m_pFlowContext; // When FlowContext::LoadXml() fails, pFlowContext is NULL.
   if (pFlowContext != NULL) m_pScenarioManager->LoadClimateScenariosXml(climate_scenarios_file, pFlowContext);

   Report::StatusMsg( "Compiling Policies" );
   Report::LogMsg( "Compiling Policies" );
   m_pPolicyManager->CompilePolicies( m_pIDULayer );

   //gpView->m_policyEditor.ReloadPolicies( false );
   //gpView->AddStandardRecorders();

   m_pModel->CreateModelAppVars();  // from model outputs, scores, rawscores

   //this->m_envContext.pMapLayer = m_pIDULayer;
   //gpDoc->UnSetChanged( CHANGED_ACTORS | CHANGED_POLICIES | CHANGED_SCENARIOS | CHANGED_PROJECT );

   // all done
   msg = "Done loading ";
   msg += filename;
   Report::StatusMsg( msg  );

   Report::CloseFile();

   m_pModel->m_logMsgLevel = logMsgLevel;

   return 1;   
   }


int EnvLoader::LoadLayer( Map *pMap, LPCTSTR name, LPCTSTR path, int type,  int includeData, int red, int green, int blue, int extraCols, int records, LPCTSTR initField, LPCTSTR overlayFields, bool loadFieldInfo )
   {
   m_pMap = pMap;
   ASSERT ( m_pMap != NULL );
   int layerIndex = m_pMap->GetLayerCount();

   // if first map, add extra cols for models
   // note: name will point to a name of the coverage, this is not the same as the path or filename
   //       path will be the fully or partially qualified path/filename
   // note: ExtraCols will be set to not save by default
   MapLayer *pLayer = AddMapLayer( path, type, extraCols, includeData, records, loadFieldInfo );

   if ( pLayer != NULL && pLayer->m_pDbTable != NULL)      // map successfully added?
      {
      // add in any other relevent information
      pLayer->m_name = name;
      pLayer->SetOutlineColor( RGB( red, green, blue ) );

      if ( includeData )
         {
         if ( pLayer->GetActiveField() < 0 )
            pLayer->SetActiveField( 0 );

         ASSERT( pLayer->m_pDbTable != NULL );
         }

      CString msg( "Loading map layer: " );
      msg += path;
      Report::InfoMsg( msg );

      // is an overlay layer is defined, set it up
      if ( overlayFields != NULL )
         {
         pLayer->m_overlayFields = overlayFields;
         /*
         CStringArray _overlayFields;
         int overlayCount = Tokenize( overlayFields, ",;", _overlayFields );

         for ( int k=0; k < overlayCount; k++ )
            {
            int overlayCol = pLayer->GetFieldCol( _overlayFields[ i ] );

            if ( overlayCol < 0 )
               {
               CString msg( "Unable to locate overlay field [" );
               msg += overlay;
               msg += "] ... The Overlay layer will not be created";
               Report::ErrorMsg( msg );
               }
            else
               {
               MapLayer *pOverlay = pMap->CreatePointOverlay( pLayer, overlayCol ); 
               if ( pOverlay == NULL )
                  {
                  CString msg( "Unable to create Overlay layer [" );
                  msg += overlay;
                  msg += "]";
                  Report::ErrorMsg( msg );
                  }
               else
                  {
                  pLayer->m_overlayCol = overlayCol;
                  pOverlay->SetActiveField( overlayCol );
                  pOverlay->UseVarWidth( 6 );
                  }
               }
            } */
         } 

      // is an initial field defined, set it up
      if ( initField && lstrlen( initField ) > 0 )
         {
         int initFieldCol = pLayer->GetFieldCol( initField );
         pLayer->m_initInfo = initField;
         
         if ( initFieldCol < 0 )
            {
            CString msg( "Unable to locate initialization field [" );
            msg += initField;
            msg += "] while loading layer ";
            msg += name;
            msg += "... ignored...";
            Report::WarningMsg( msg );
            }
         else
            pLayer->SetActiveField( initFieldCol );
         }

      //CheckValidFieldNames( pLayer );      
      }

   else
      {
      layerIndex = -1; // because failed
      }

   return layerIndex;
   }


MapLayer *EnvLoader::AddMapLayer( LPCSTR _path, int type, int extraCols, int includeData, int records, bool loadFieldInfo )
   {
   // add the layer to the map
   ASSERT( m_pMap != NULL );
   if ( m_pMap == NULL )
      return NULL;

  // VData::m_useWideChar = false;
// ESRI DBF strings are often double byte...kbv 02/09/2010
   VData::SetUseWideChar( true );

   MapLayer *pLayer = NULL;

   CString path;
   PathManager::FindPath( _path, path );
   
   // load layer stuff here
   CString msg;
   msg.Format( "Loading layer %s", (LPCTSTR) path );
   Report::InfoMsg( msg );

   switch ( type )
      {
      case 0:     // shape file
         {
         pLayer = m_pMap->AddShapeLayer( path, includeData ? true : false, extraCols, records );

         if ( pLayer != NULL && loadFieldInfo )
            {
            // add a field info file if one exists
            nsPath::CPath fiPath( path );
            fiPath.RenameExtension( "xml" );

            if ( fiPath.Exists() )
               LoadFieldInfoXml( pLayer, fiPath );
            }
         // all done
         //return pLayer;
         }
         break;
         
      case 1:     // grid file
         pLayer = m_pMap->AddGridLayer( path, DOT_INT );
         break;

      case 2:     // grid file
         pLayer = m_pMap->AddGridLayer( path, DOT_FLOAT );
         break;
      }

   if ( pLayer != NULL )
      {
      CString msg;
      msg.Format( "Loaded layer %s", (PCTSTR) path );
      Report::InfoMsg( msg );
      }
   else  // pLayer = NULL
      {
      CString msg( "Unable to load Map Layer " );
      msg += path;
      msg += "... Would you like to browse for the file?";

#ifndef __ENV_ENGINE__

      if ( gpView->MessageBox( msg, "Layer Not Found", MB_YESNO ) == IDYES )
         {
         char *cwd = _getcwd( NULL, 0 );

         CFileDialog dlg( TRUE, NULL, path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
               "Shape files|*.shp|Grid Files|*.grd|All files|*.*||");

         if ( dlg.DoModal() == IDOK )
            {
            CString filename( dlg.GetPathName() );

            _chdir( cwd );
            free( cwd );
            return AddMapLayer( filename, type, extraCols, includeData, records, loadFieldInfo );
            }
         
         _chdir( cwd );
         free( cwd );
         }
#endif

      }  // end of: if ( pLayer == NULL )

   m_pMap->m_name = path;

   return pLayer;
   }  




int EnvLoader::LoadFieldInfoXml( MapLayer *pLayer, LPCSTR _filename )
   {
   // have xml string, start parsing
   CString filename;
   PathManager::FindPath( _filename, filename );

   TiXmlDocument doc;
   bool ok = doc.LoadFile( filename );

   if ( ! ok )
      {
#ifndef NO_MFC
      CString msg;
      msg.Format("Error reading FieldInfo input file, %s", (PCTSTR) filename );
      AfxGetMainWnd()->MessageBox( doc.ErrorDesc(), msg );
#endif
      return false;
      }

   MAP_FIELD_INFO *pFieldInfo = NULL;

   // start interating through the nodes
   TiXmlElement *pXmlRoot = doc.RootElement();

   if ( pLayer == NULL )
      {
      CString layer = pXmlRoot->Attribute( "layer" );
      pLayer = m_pIDULayer->m_pMap->GetLayer( layer );

      if ( pLayer == NULL )
         return -1;
      }

   pLayer->m_pFieldInfoArray->Clear();
   pLayer->m_pFieldInfoArray->m_path = filename;
   
   // load any field/submenu tags defined in the XML
   int loadSuccess = LoadFieldInfoXml( pXmlRoot, NULL, pLayer, filename );

   // now add any missing, but required field names
   if ( pLayer == m_pIDULayer )
      {
      int levels = EnvModel::m_lulcTree.GetLevels();
      CString lulcA, lulcB,  lulcC, lulcD;
      if ( levels > 0 ) lulcA = EnvModel::m_lulcTree.GetFieldName( 1 );
      if ( levels > 1 ) lulcB = EnvModel::m_lulcTree.GetFieldName( 2 );
      if ( levels > 2 ) lulcC = EnvModel::m_lulcTree.GetFieldName( 3 );
      if ( levels > 3 ) lulcD = EnvModel::m_lulcTree.GetFieldName( 4 );
/*
      //-------------------------|fieldname|level| label ------------------------------|-type---|bincount|displayFlags|bsFlag|min,max    |save );
      if ( ( levels > 0 ) && (pFieldInfo = m_pIDULayer->FindFieldInfo( lulcA )) == NULL )
         {
         int index = m_pIDULayer->AddFieldInfo( lulcA, 0, "Coarse-Articulation Land Use", _T(""), TYPE_INT, MFIT_CATEGORYBINS, BCF_GRAY_INCR, 0, -1.f, -1.f, true );
         pFieldInfo = m_pIDULayer->GetFieldInfo( index );
         pFieldInfo->SetExtra( 1 );     // show in results

         // add attributes
         LulcNode *pNode = EnvModel::m_lulcTree.GetRootNode();
         while( (pNode = EnvModel::m_lulcTree.GetNextNode()) != NULL )
            if ( pNode->GetNodeLevel() == 1 )
               pFieldInfo->AddAttribute( VData(pNode->m_id), pNode->m_name, pNode->m_color, pNode->m_id-0.0001f, pNode->m_id+0.0001f );
         }

      if ( (levels > 1 ) && (pFieldInfo = m_pIDULayer->FindFieldInfo( lulcB )) == NULL )
         {
         int index = m_pIDULayer->AddFieldInfo( lulcB, 0, "Mid-Articulation Land Use", _T(""),TYPE_INT, MFIT_CATEGORYBINS, BCF_GRAY_INCR, 0, -1.f, -1.f, true );
         pFieldInfo = m_pIDULayer->GetFieldInfo( index );
         pFieldInfo->SetExtra( 1 );     // show in results

         // add attributes
         LulcNode *pNode = EnvModel::m_lulcTree.GetRootNode();
         while( (pNode = EnvModel::m_lulcTree.GetNextNode()) != NULL )
            if ( pNode->GetNodeLevel() == 2 )
               pFieldInfo->AddAttribute( VData(pNode->m_id), pNode->m_name, pNode->m_color, pNode->m_id-0.0001f, pNode->m_id+0.0001f );
         }

      if ( (levels > 2 ) && (pFieldInfo = m_pIDULayer->FindFieldInfo( lulcC )) == NULL )
         {
         int index = m_pIDULayer->AddFieldInfo( lulcC, 0, "Fine-Articulation Land Use", _T(""), TYPE_INT, MFIT_CATEGORYBINS, BCF_GRAY_INCR, 0, -1.f, -1.f, true );
         pFieldInfo = m_pIDULayer->GetFieldInfo( index );
         pFieldInfo->SetExtra( 1 );     // show in results

         // add attributes
         LulcNode *pNode = EnvModel::m_lulcTree.GetRootNode();
         while( (pNode = EnvModel::m_lulcTree.GetNextNode()) != NULL )
            if ( pNode->GetNodeLevel() == 3 )
               pFieldInfo->AddAttribute( VData(pNode->m_id), pNode->m_name, pNode->m_color, pNode->m_id-0.0001f, pNode->m_id+0.0001f );
         }

      if ( (levels > 3 ) && (pFieldInfo = m_pIDULayer->FindFieldInfo( lulcD )) == NULL )
         {
         int index = m_pIDULayer->AddFieldInfo( lulcD, 0, "Extra Fine-Articulation Land Use", _T(""), TYPE_INT, MFIT_CATEGORYBINS, BCF_GRAY_INCR, 0, -1.f, -1.f, true );
         pFieldInfo = m_pIDULayer->GetFieldInfo( index );
         pFieldInfo->SetExtra( 1 );     // show in results

         // add attributes
         LulcNode *pNode = EnvModel::m_lulcTree.GetRootNode();
         while( (pNode = EnvModel::m_lulcTree.GetNextNode()) != NULL )
            if ( pNode->GetNodeLevel() == 4 )
               pFieldInfo->AddAttribute( VData(pNode->m_id), pNode->m_name, pNode->m_color, pNode->m_id-0.0001f, pNode->m_id+0.0001f );
         }

*/

      /*
      if ( (pFieldInfo = m_pIDULayer->FindFieldInfo( "SCORE" )) == NULL )
         {
         int index = m_pIDULayer->AddFieldInfo( "SCORE", 1, "Alternative Score", _T(""), TYPE_FLOAT, MFIT_QUANTITYBINS, BCF_GRAY_INCR, 0, -1.f, -1.f, false );
         m_pIDULayer->GetFieldInfo( index )->SetExtra( 1 );     // show in results
         } */

      //if ( (pFieldInfo = m_pIDULayer->FindFieldInfo( "ACTORGROUP" )) == NULL )
      //   m_pIDULayer->AddFieldInfo( "ACTORGROUP", 0, "Class of Actor managing this cell", _T(""), _T(""), TYPE_INT, MFIT_QUANTITYBINS, /*0,*/ BCF_MIXED ,    0,    -1.f, -1.f, false );

      MAP_FIELD_INFO *pFieldInfo = NULL;
      if ( (pFieldInfo = pLayer->FindFieldInfo( "POLICY" )) == NULL )
         {
         int index = pLayer->AddFieldInfo( "POLICY", 0, "Policy applied to this cell", _T(""),  _T(""), TYPE_INT,  MFIT_CATEGORYBINS, /*0,*/ BCF_MIXED, 0, -1.f, -1.f, true );
         pLayer->GetFieldInfo( index )->SetExtra( 1 );     // show in results
         }

      if ( (pFieldInfo = m_pIDULayer->FindFieldInfo( "POLICYAPPS" )) == NULL )
         {
         int index = pLayer->AddFieldInfo( "POLICYAPPS", 0, "Total Policys applied to this cell", _T(""),  _T(""), TYPE_INT, MFIT_QUANTITYBINS, /*0,*/ BCF_MIXED ,    0,    -1.f, -1.f, true  );
         pLayer->GetFieldInfo( index )->SetExtra( 1 );
         }
      }

   // fix up any submenus
   //int fieldInfoCount = pLayer->GetFieldInfoCount( 1 );
   //for ( int i=0; i < fieldInfoCount; i++ )
   //   {
   //   MAP_FIELD_INFO *pInfo = pLayer->GetFieldInfo( i );
   //
   //   if ( pInfo->parentName.GetLength() > 0 )
   //      {
   //      // this one goes on a submenu, so find the parent field info
   //      for ( int j=0; j < fieldInfoCount; j++ )
   //         {
   //         if ( i == j )
   //            continue;
   //
   //         MAP_FIELD_INFO *pParentInfo = pLayer->GetFieldInfo( j );
   //
   //         if ( pInfo->parentName.CompareNoCase( pParentInfo->fieldname ) == 0 ) // match found?
   //            {
   //            pInfo->pParent = pParentInfo;
   //            break;
   //            }
   //         }  // end of: for ( j < fieldInfoCount )
   //      }  // end of:  if ( has a parent )
   //   }

   return loadSuccess;
   }


int EnvLoader::LoadFieldInfoXml( TiXmlElement *pXmlElement, MAP_FIELD_INFO *pParent, MapLayer *pLayer, LPCTSTR filename )
   {
   int loadSuccess = 1;
   // iterate through 
   TiXmlNode *pXmlFieldNode = NULL;
   while( pXmlFieldNode = pXmlElement->IterateChildren( pXmlFieldNode ) )
      {
      if ( pXmlFieldNode->Type() != TiXmlNode::ELEMENT )
            continue;

      // get the classification
      TiXmlElement *pXmlFieldElement = pXmlFieldNode->ToElement();

      CString elementName = pXmlFieldElement->Value();
      int index = -1;
      // is it a submenu definition?
      if ( elementName.CompareNoCase( _T("submenu" ) )  == 0 )
         {
         LPTSTR label   = NULL;
      
         // lookup fields
         XML_ATTR attrs[] = { // attr         type           address  isReq checkCol
                           { "label",         TYPE_STRING,   &label,  true,  0 },
                           { NULL,            TYPE_NULL,     NULL,    false, 0 } };
   
         bool ok = TiXmlGetAttributes( pXmlFieldElement, attrs, filename );
         if ( ! ok )
            {
            loadSuccess = 0;
            continue;
            }

         index = pLayer->AddFieldInfo( label, 0, label, "", "", 0, MFIT_SUBMENU, 0, 0, -1.0f, -1.0f, true );
         MAP_FIELD_INFO *pSubMenu = pLayer->GetFieldInfo( index );
         if ( LoadFieldInfoXml( pXmlFieldElement, pSubMenu, pLayer, filename ) == 0 )
            loadSuccess = 0;
         }

      else if ( elementName.CompareNoCase( _T( "field" ) ) == 0 )
         {
         LPTSTR col         = NULL;
         LPTSTR label       = NULL;
         int    level       = 0;
         LPTSTR mfiType     = NULL;
         int    displayFlag = 0;
         int    binStyle    = 0;
         float  minVal      = 0;
         float  maxVal      = 0;
         bool   results     = false;
         int    decadalMaps = 0;
         bool   siteAttr    = false;
         bool   outcomes    = false;
         LPTSTR datatype    = NULL;
      
         // lookup fields
         XML_ATTR fieldAttrs[] = { // attr          type           address       isReq checkCol
                         { "col",           TYPE_STRING,   &col          , true,  0 },
                         { "label",         TYPE_STRING,   &label        , true,  0 },
                         { "level",         TYPE_INT,      &level        , false, 0 }, /*deprecated*/
                         { "mfiType",       TYPE_STRING,   &mfiType      , false, 0 },
                         { "displayFlag",   TYPE_INT,      &displayFlag  , true,  0 },
                         { "binStyle",      TYPE_INT,      &binStyle     , true,  0 },
                         { "min",           TYPE_FLOAT,    &minVal       , false, 0 },
                         { "max",           TYPE_FLOAT,    &maxVal       , false, 0 },
                         { "results",       TYPE_BOOL,     &results      , true,  0 },
                         { "decadalMaps",   TYPE_INT,      &decadalMaps  , true,  0 },
                         { "useInSiteAttr", TYPE_BOOL,     &siteAttr     , true,  0 },
                         { "useInOutcomes", TYPE_BOOL,     &outcomes     , true,  0 },
                         { "datatype",      TYPE_STRING,   &datatype     , true,  0 },
                         { NULL,            TYPE_NULL,     NULL,           false, 0 } };
   
         bool ok = TiXmlGetAttributes( pXmlFieldElement, fieldAttrs, filename, pLayer );
   
         if ( ! ok )
            {
            //loadSuccess = false;
            continue;
            }
   
         MFI_TYPE _mfiType = MFIT_CATEGORYBINS;
         if ( mfiType != NULL )
         _mfiType = (MFI_TYPE) atoi( mfiType );
         
         int colIndex = pLayer->GetFieldCol( col );
         TYPE type = TYPE_NULL;
   
         if ( datatype == NULL )
            {
            if ( colIndex < 0 )
               {
               CString msg;
               msg.Format( "The column (%s) in the database referred to in %s is missing", col, (LPCSTR) filename );
               }
            else
               type = pLayer->m_pDbTable->GetFieldInfo( colIndex ).type;
            }
         else
            {
            if ( lstrcmpi( datatype, "long" ) == 0 )
               type = TYPE_LONG;
            else if ( lstrcmpi( datatype, "double" ) == 0 )
               type = TYPE_DOUBLE;
            else if ( lstrcmpi( datatype, "float" ) == 0 )
               type = TYPE_DOUBLE;
            else if ( lstrcmpi( datatype, "short" ) == 0 )
               type = TYPE_SHORT;
            else if ( lstrcmpi( datatype, "int" ) == 0 )
               type = TYPE_INT;
            else if ( lstrcmpi( datatype, "integer" ) == 0 )
               type = TYPE_INT;
            else if ( lstrcmpi( datatype, "boolean" ) == 0 )
               type = TYPE_BOOL;
            else if ( lstrcmpi( datatype, "string" ) == 0 )
               type = TYPE_DSTRING;
            else
               {
               CString msg;
               msg.Format( "Unrecognized type (%s) reading field info for field [%s]", datatype, col );
               Report::ErrorMsg( msg );
               type = TYPE_LONG;
               }
            }
   
         index = pLayer->AddFieldInfo( col, 0 /*level*/, label, "", "", type, _mfiType, displayFlag, binStyle, minVal, maxVal, true );

         MAP_FIELD_INFO *pFieldInfo = NULL;
   
         // fill out extra data for the field.  byte 1=results, byte 2=useInSiteAttr, byte3=useInOutcome, HIWORD=decadalMaps
         if ( index >= 0 )
            pFieldInfo = pLayer->GetFieldInfo( index );
         else
            pFieldInfo = pLayer->FindFieldInfo( col );    // info previously defined...
   
         if ( pFieldInfo )
            {
            if ( results  ) pFieldInfo->SetExtra( 1 );
            if ( siteAttr ) pFieldInfo->SetExtra( 2 );
            if ( outcomes ) pFieldInfo->SetExtra( 4 );
            pFieldInfo->SetExtra( MAKELONG( 0, short( decadalMaps ) ) );

            if ( pParent != NULL )
               {
               pFieldInfo->pParent = pParent;
               //pFieldInfo->level = 1;
               }
   
            // load description
            TiXmlNode *pXmlDescNode = pXmlFieldNode->FirstChild( "description" );
            if ( pXmlDescNode )
               {
               TiXmlElement *pDesc = pXmlDescNode->ToElement();
               pFieldInfo->description = pDesc->GetText();
               }
   
            // load source information
            TiXmlNode *pXmlSourceNode = pXmlFieldNode->FirstChild( "source" );
            if ( pXmlSourceNode )
               {
               TiXmlElement *pSource = pXmlSourceNode->ToElement();
               pFieldInfo->source = pSource->GetText();
               }
       
            // load attribute information
            TiXmlNode *pXmlAttrsNode = pXmlFieldNode->FirstChild( "attributes" );
            TiXmlNode *pXmlAttrNode = NULL;
   
            while( pXmlAttrNode = pXmlAttrsNode->IterateChildren( pXmlAttrNode ) )
               {
               if ( pXmlAttrNode->Type() != TiXmlNode::ELEMENT )
                     continue;
   
               // get the classification
               TiXmlElement *pXmlAttrElement = pXmlAttrNode->ToElement();
   
               const TCHAR *value  = pXmlAttrElement->Attribute( "value" );
               const TCHAR *label  = pXmlAttrElement->Attribute( "label" );
               const TCHAR *color  = pXmlAttrElement->Attribute( "color" );
               const TCHAR *minVal = pXmlAttrElement->Attribute( "minVal" );
               const TCHAR *maxVal = pXmlAttrElement->Attribute( "maxVal" );
               const TCHAR *transparency = pXmlAttrElement->Attribute( "transparency" );
   
               if ( label == NULL || color == NULL )
                  {
                  CString msg( "Misformed <attr> element reading " );
                  msg += filename;
                  msg += " A required attribute is missing.";
                  Report::ErrorMsg( msg );
                  loadSuccess = false;
                  continue;
                  }
   
               // convert non-string values
               VData _value;
               if ( value )
                  {
                  _value = value;
                  _value.ChangeType( type );
                  }
   
               int red=-1, green=-1, blue=-1, alpha=-1;
               int count = 0;
               TCHAR *ptr = (LPTSTR) color;
               while ( *ptr == ' ' || *ptr == '(' )
                  ptr++;
               
               // should be negative or a number - verify
               if ( ! isdigit( *ptr ) )
                  {
                  CString msg;
                  msg.Format( "Error reading color information for field '%s' attribute '%s'.  Invalid color format specified.",
                     col, label );
                  Report::WarningMsg( msg );
                  }
               else
                  {
                  int count = sscanf_s( ptr, "%i,%i,%i", &red, &green, &blue );
               
                  if ( count != 3 )
                     {
                     CString msg( "Misformed <attr> color element reading" );
                     msg += filename;
                     Report::ErrorMsg( msg );
                     loadSuccess = false;
                     }
                  }
               
               int _transparency = 0;
               if ( transparency != NULL )
                  {
                  _transparency = atoi(  transparency );
                  }
   
               float _minVal = 0;
               float _maxVal = 0;
   
               if ( minVal != NULL )
                  _minVal = (float) atof( minVal );
   
               if ( maxVal != NULL )
                  _maxVal = (float) atof( maxVal );
   
               pFieldInfo->AddAttribute( _value, label, RGB( red, green, blue ), _minVal, _maxVal, _transparency );
               }  // while (iterating attr tags)
            }  // end of: if ( pFieldInfo != NULL )
         }  
      }  // end of: while (iterating through nodes)

   return loadSuccess;
   }



bool EnvLoader::LoadModel( EnvModel *pModel, LPCTSTR name, LPCTSTR path, int modelID, int use, int showInResults, /* int decisionUse, */
        int useCol, LPCTSTR fieldname, LPCTSTR initInfo, LPCTSTR dependencyNames, int initRunOnStartup, float gain, float offset )
   {
   m_pModel = pModel;

   HINSTANCE hDLL = LOAD_LIBRARY( path ); 
   if ( hDLL == NULL )
      {
      CString msg( _T( "Unable to load ") );
      msg += path;
      msg += _T(" or a dependent DLL" );
      Report::ErrorMsg( msg );

      Report::BalloonMsg( msg );
      return false;
      }

   RUNFN runFn = (RUNFN) PROC_ADDRESS( hDLL, "EMRun" );
   if ( ! runFn )
      {
      CString msg( "Unable to find function 'EMRun()' in " );
      msg += path;
      Report::ErrorMsg( msg );
      FREE_LIBRARY( hDLL );
      return false;
      }
   else
      {
      INITRUNFN  initRunFn  = (INITRUNFN) PROC_ADDRESS( hDLL, "EMInitRun" );
      ENDRUNFN   endRunFn   = (ENDRUNFN)  PROC_ADDRESS( hDLL, "EMEndRun" );
      INITFN     initFn     = (INITFN)    PROC_ADDRESS( hDLL, "EMInit" );
      PUSHFN     pushFn     = (PUSHFN)    PROC_ADDRESS( hDLL, "EMPush" );
      POPFN      popFn      = (POPFN)     PROC_ADDRESS( hDLL, "EMPop"  );
      SETUPFN setupFn = (SETUPFN) PROC_ADDRESS( hDLL, "EMSetup" );
      INPUTVARFN inputVarFn = (INPUTVARFN) PROC_ADDRESS( hDLL, "EMInputVar" );
      OUTPUTVARFN outputVarFn = (OUTPUTVARFN) PROC_ADDRESS( hDLL, "EMOutputVar" );
      
      ENV_EVAL_MODEL *pInfo = new  ENV_EVAL_MODEL( hDLL, runFn, initFn, initRunFn, endRunFn, pushFn, popFn, setupFn, inputVarFn, outputVarFn, modelID, showInResults, /*decisionUse,*/ initInfo, name, path );

      m_pModel->AddModel( pInfo );

      pInfo->use = use ? true : false;
      pInfo->col = useCol ? 0 : -1;
      pInfo->fieldName = fieldname;
      pInfo->dependencyNames = dependencyNames;
      pInfo->initRunOnStartup = initRunOnStartup != 0 ? true : false;
      pInfo->gain = gain;
      pInfo->offset = offset;
      }

   return true;
   }


bool EnvLoader::LoadAP( EnvModel *pModel, LPCTSTR name, LPCTSTR path, int apID, int use, int timing, int sandbox, int useCol, LPCTSTR fieldname, LPCTSTR initInfo, LPCTSTR dependencyNames, int initRunOnStartup )
   {
   m_pModel = pModel;

   HINSTANCE hDLL = LOAD_LIBRARY( path ); 
   if ( hDLL == NULL )
      {
      CString msg( "Unable to find " );
      msg += path;
      msg += _T(" or a dependent DLL" );
      Report::ErrorMsg( msg );

      Report::BalloonMsg( msg );
      return false;
      }

   RUNFN runFn = (RUNFN) PROC_ADDRESS( hDLL, "APRun" );
   if ( ! runFn )
      {
      CString msg( "Unable to find function 'APRun()' in " );
      msg += path;
      Report::ErrorMsg( msg );
      FREE_LIBRARY( hDLL );
      return false;
      }
   else
      {
      INITRUNFN  initRunFn  = (INITRUNFN) PROC_ADDRESS( hDLL, "APInitRun" );
      ENDRUNFN   endRunFn   = (ENDRUNFN)  PROC_ADDRESS( hDLL, "APEndRun" );
      INITFN     initFn     = (INITFN)    PROC_ADDRESS( hDLL, "APInit" );
      PUSHFN     pushFn     = (PUSHFN)    PROC_ADDRESS( hDLL, "APPush" );
      POPFN      popFn      = (POPFN)     PROC_ADDRESS( hDLL, "APPop"  );
      SETUPFN setupFn = (SETUPFN) PROC_ADDRESS( hDLL, "APSetup" );
      INPUTVARFN inputVarFn = (INPUTVARFN) PROC_ADDRESS( hDLL, "APInputVar" );
      OUTPUTVARFN outputVarFn = (OUTPUTVARFN) PROC_ADDRESS( hDLL, "APOutputVar" );
      
      ENV_AUTO_PROCESS *pInfo = new ENV_AUTO_PROCESS( hDLL, runFn, initFn, initRunFn, endRunFn, pushFn, popFn, setupFn, inputVarFn, outputVarFn, apID, initInfo, name, path );

      m_pModel->AddAutonomousProcess( pInfo );

      pInfo->timing  = timing;
      pInfo->sandbox = sandbox;
      pInfo->use = true;
      pInfo->col = useCol ? 0 : -1;
      pInfo->fieldName = fieldname;
      pInfo->use = use ? true : false;
      pInfo->initRunOnStartup = initRunOnStartup != 0 ? true : false;
      pInfo->dependencyNames = dependencyNames;
      }
   
   return true;
   }


bool EnvLoader::LoadExtension( LPCTSTR name, LPCTSTR path, int modelID, int use, int showInResults, int decisionUse, int useCol, LPCTSTR fieldname, LPCTSTR initInfo, float gain, float offset )
   {/*
   HINSTANCE hDLL = LOAD_LIBRARY( path ); 
   if ( hDLL == NULL )
      {
      CString msg( "Unable to find " );
      msg += path;
      msg += _T(" or a dependent DLL" );
      Report::ErrorMsg( msg );

      SystemErrorMsg( GetLastError() );
      return false;
      }

   RUNFN runFn = (RUNFN) PROC_ADDRESS( hDLL, "EMRun" );
   if ( ! runFn )
      {
      CString msg( "Unable to find function 'EMRun()' in " );
      msg += path;
      Report::ErrorMsg( msg );
      FREE_LIBRARY( hDLL );
      return false;
      }
   else
      {
      INITRUNFN  initRunFn  = (INITRUNFN) PROC_ADDRESS( hDLL, "EMInitRun" );
      ENDRUNFN   endRunFn   = (ENDRUNFN)  PROC_ADDRESS( hDLL, "EMEndRun" );
      INITFN     initFn     = (INITFN)    PROC_ADDRESS( hDLL, "EMInit" );
      PUSHFN     pushFn     = (PUSHFN)    PROC_ADDRESS( hDLL, "EMPush" );
      POPFN      popFn      = (POPFN)     PROC_ADDRESS( hDLL, "EMPop"  );
      SETUPFN setupFn = (SETUPFN) PROC_ADDRESS( hDLL, "EMSetup" );
      INPUTVARFN inputVarFn = (INPUTVARFN) PROC_ADDRESS( hDLL, "EMInputVar" );
      OUTPUTVARFN outputVarFn = (OUTPUTVARFN) PROC_ADDRESS( hDLL, "EMOutputVar" );
                        
      int index = m_pModel->AddModel( new ENV_EVAL_MODEL( hDLL, runFn, initFn, initRunFn, endRunFn, pushFn, 
            popFn, setupFn, inputVarFn, outputVarFn, modelID, showInResults, decisionUse, initInfo, name, path ) );
      ENV_EVAL_MODEL *pInfo =  m_pModel->GetModelInfo( index );
      pInfo->use = use ? true : false;
      pInfo->col = useCol ? 0 : -1;
      pInfo->fieldName = fieldname;
      pInfo->gain = gain;
      pInfo->offset = offset;
      }
   */
   return true;
   }



bool EnvLoader::LoadVisualizer( LPCTSTR name, LPCTSTR path, int use, int type, LPCTSTR initInfo )
   {
   HINSTANCE hDLL = LOAD_LIBRARY( path ); 
   if ( hDLL == NULL )
      {
      CString msg( "Unable to find " );
      msg += path;
      msg += _T(" or a dependent DLL" );
      Report::ErrorMsg( msg );

      Report::BalloonMsg( msg );
      return false;
      }

   INITFN      initFn       = (INITFN)      PROC_ADDRESS( hDLL, "VInit" );
   INITRUNFN   initRunFn    = (INITRUNFN)   PROC_ADDRESS( hDLL, "VInitRun" );
   ENDRUNFN    endRunFn     = (ENDRUNFN)    PROC_ADDRESS( hDLL, "VEndRun" );
   RUNFN       runFn        = (RUNFN)       PROC_ADDRESS( hDLL, "VRun" );
   INITWNDFN   initWndFn    = (INITWNDFN)   PROC_ADDRESS( hDLL, "VInitWindow" );
   UPDATEWNDFN updateWndFn  = (UPDATEWNDFN) PROC_ADDRESS( hDLL, "VUpdateWindow" );
   SETUPFN     setupFn      = (SETUPFN)     PROC_ADDRESS( hDLL, "VSetup" );

   ENV_VISUALIZER *pViz = new ENV_VISUALIZER( hDLL, runFn, initFn, initRunFn, endRunFn, initWndFn, updateWndFn, setupFn, 0, type, initInfo, name, path, false );

   m_pModel->AddVisualizer( pViz );

   pViz->use = use ? true : false;
   
   return true;
   }


void EnvLoader::SetDependencies( void )
   {
   TCHAR buffer[ 256 ];
   LPTSTR nextToken;

   PtrArray< ENV_PROCESS > tempArray( false );

   for ( int i=0; i < EnvModel::GetModelCount(); i++ )
      {
      ENV_EVAL_MODEL *pInfo = EnvModel::GetModelInfo( i );
      if ( ! pInfo->dependencyNames.IsEmpty() )
         {
         lstrcpy( buffer, pInfo->dependencyNames );

         // parse buffer
         // Establish string and get the first token:
         LPTSTR modelName = _tcstok_s( buffer, _T(" ,;"), &nextToken ); // C4996
         while( modelName != NULL )
            {
            ENV_EVAL_MODEL *pDependency = EnvModel::FindModelInfo( modelName );

            if ( pDependency == NULL )
               {
               CString msg;
               msg.Format( _T("Unable to find dependency <%s> specified for model %s"), modelName, (LPCTSTR) pInfo->name );
               Report::ErrorMsg( msg );
               }
            else
               tempArray.Add( pInfo );

            modelName = _tcstok_s( NULL, _T(" ,;"), &nextToken );
            }
         }

      int count = (int) tempArray.GetSize();
      
      if ( count > 0 )
         {
         pInfo->dependencyCount = count;
         pInfo->dependencyArray = new ENV_PROCESS*[ count ];
         }

      tempArray.Clear();
      }  // end of: for ( i < EnvModel::GetModelCount() )


   for ( int i=0; i < EnvModel::GetAutonomousProcessCount(); i++ )
      {
      ENV_AUTO_PROCESS *pInfo = EnvModel::GetAutonomousProcessInfo( i );
      if ( ! pInfo->dependencyNames.IsEmpty() )
         {
         lstrcpy( buffer, pInfo->dependencyNames );

         // parse buffer
         // Establish string and get the first token:
         LPTSTR apName = _tcstok_s( buffer, _T(" ,;"), &nextToken ); // C4996
         while( apName != NULL )
            {
            ENV_AUTO_PROCESS *pDependency = EnvModel::FindAutonomousProcessInfo( apName );

            if ( pDependency == NULL )
               {
               CString msg;
               msg.Format( _T("Unable to find dependency <%s> specified for autonomous process %s"), apName, (LPCTSTR) pInfo->name );
               Report::ErrorMsg( msg );
               }
            else
               tempArray.Add( pInfo );

            apName = _tcstok_s( NULL, _T(" ,;"), &nextToken );
            }
         }

      int count = (int) tempArray.GetSize();
      
      if ( count > 0 )
         {
         pInfo->dependencyCount = count;
         pInfo->dependencyArray = new ENV_PROCESS*[ count ];
         }

      tempArray.Clear();
      }  // end of: for ( i < EnvModel::GetModelCount() )
   }




// Note: InitVisualizers should be called AFTER the models are initialized but BEFORE
// any models are run.
void EnvLoader::InitVisualizers()   
   {
   //----------------------------------------------------------------
   // ---------- Initialize Visualizers -----------------------------
   //----------------------------------------------------------------
   EnvContext &context = m_pModel->m_envContext;  //( m_pIDULayer );

   context.showMessages = (m_pModel->m_debug > 0);
   context.pActorManager = m_pActorManager;          // pointer to the actor manager
   context.pPolicyManager = m_pPolicyManager;         // pointer to the policy manager
   context.pEnvModel = m_pModel;                // pointer to the current model
   context.pLulcTree = &m_pModel->m_lulcTree;             // lulc tree used in the simulation

   // Call the init function (if available) for the visualizer
   for ( int i=0; i < m_pModel->GetVisualizerCount(); i++ )
      {
      ENV_VISUALIZER *pInfo = m_pModel->GetVisualizerInfo( i );

      if ( pInfo->use && pInfo->initFn != NULL )
         {
         INITFN initFn = pInfo->initFn;

         if ( initFn != NULL )
            {
            CString msg = "Initializing ";
            msg += pInfo->name;
            Report::StatusMsg( msg );
            context.id  = pInfo->id;
            context.handle = pInfo->handle;
            context.pExtensionInfo = pInfo;
            initFn( &context, (LPCTSTR) pInfo->initInfo );
            context.extra = 0;
            }
         }
      }

   Report::StatusMsg( "" );
   return;
   }

