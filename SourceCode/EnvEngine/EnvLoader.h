#pragma once

#include <Maplayer.h>

class Map;
class EnvModel;
class PolicyManager;
class ActorManager;
class ScenarioManager;
class QueryEngine;
class TiXmlElement;

enum AML_TYPE { AMLT_INFER = -1, AMLT_SHAPE = 0, AMLT_INT_GRID = 1, AMLT_FLOAT_GRID = 2 };

class EnvLoader
   {
   public:
      EnvLoader( void );
      //EnvLoader::EnvLoader( Map *pMap, EnvModel *pModel, PolicyManager *pPolicyManager,
      //         ActorManager *pActorManager, ScenarioManager *pScenarioManager );
      ~EnvLoader();

      int LoadProject( LPCTSTR filename, Map *pMap, EnvModel *pModel, PolicyManager *pPolicyManager,
                 ActorManager *pActorManager, ScenarioManager *pScenarioManager  );
      int LoadLayer( Map *pMap, LPCTSTR name, LPCTSTR path, int type,  int includeData, int red, int green, int blue, int extraCols, int records, LPCTSTR initField, LPCTSTR overlayFields, bool loadFieldInfo );
      
      int LoadFieldInfoXml( MapLayer *pLayer, LPCSTR _filename );

      bool LoadModel( EnvModel *pModel, LPCTSTR name, LPCTSTR path, int modelID, int use, int showInResults, /* int decisionUse, */
            int useCol, LPCTSTR fieldname, LPCTSTR initInfo, LPCTSTR dependencyNames, int initRunOnStartup, float gain, float offset );
      bool LoadAP( EnvModel *pModel, LPCTSTR name, LPCTSTR path, int apID, int use, int timing, int sandbox, int useCol, LPCTSTR fieldname, LPCTSTR initInfo, LPCTSTR dependencyNames, int initRunOnStartup );
  
   protected:
      Map             *m_pMap;      
      MapLayer        *m_pIDULayer;          // set during loading of project file
      EnvModel        *m_pModel;
      PolicyManager   *m_pPolicyManager;
      ActorManager    *m_pActorManager;
      ScenarioManager *m_pScenarioManager;
      QueryEngine     *m_pQueryEngine;       // set during load of project file

      //bool m_areModelsInitialized;
      
   public:

   protected:
      int LoadFieldInfoXml( TiXmlElement *pXmlElement, MAP_FIELD_INFO *pParent, MapLayer *pLayer, LPCTSTR filename );
      bool LoadExtension( LPCTSTR name, LPCTSTR path, int modelID, int use, int showInResults, int decisionUse, int useCol, LPCTSTR fieldname, LPCTSTR initInfo, float gain, float offset );
      bool LoadVisualizer( LPCTSTR name, LPCTSTR path, int use, int type, LPCTSTR initInfo );

      //int  ReconcileDataTypes( void );
      void SetDependencies( void );

      void InitVisualizers( void );

      MapLayer *AddMapLayer( LPCSTR _path, int type, int extraCols, int includeData, int records, bool loadFieldInfo );


   };

