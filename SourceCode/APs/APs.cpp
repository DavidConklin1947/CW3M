// APs.cpp 
//

#include "stdafx.h"
#include "APs.h"

#include <Maplayer.h>
#include <Map.h>
#include <math.h>
#include <UNITCONV.H>
#include <path.h>
#include <EnvEngine\EnvModel.h>
#include <EnvInterface.h>
#include <direct.h>
#include "AlgLib\ap.h"
#include <PathManager.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static char * climateScenarioNames[] =
{ "MIROC",      // Climate Scenario = 0 (default value) (reference scenario)
"GFDL",        // Climate Scenario = 1
"HadGEM",      // Climate Scenario = 2
"MIROC",  	   // Climate Scenario = 3.  StationaryClim scenario uses random water years drawn from MIROC 1950-2009.
"historic",    // Climate Scenario = 4.  WeatherOnGrids\ActualWeather\WRB_4km_1979-2011\abat_willamette_...
"Baseline"};   // Climate Scenario = 5.  Actual weather

static char * PopScenarioNames[] =
   {
   "CurrentCourse",	// pop scenario = 0 => reference scenario
   "Pop_UGBx2",	// scenario = 1 => population increments to UGAs are twice as large
   "NoGrow", // pop scenario = 2 => population and income doesn't change from year to year
   "Extreme", // pop scenario = 3 => higher commercial and manufacturing incomes, and population increments to UGAs are twice as large
   "NoPopGrowth", // pop scenario = 4 => population doesn't change from year to year, but income as in reference scenario
   "NoIncGrowth", // pop scenario = 5 => population as in reference scenario, but income doesn't change from year to year
   "BaselinePopulationAndIncome" // pop scenario = 6 => actual population and income
   };



///////////////////////////////////////////////////////////////////////////////////////
//  WW2100AP  - the AutoProcess...
///////////////////////////////////////////////////////////////////////////////////////

WW2100AP::WW2100AP() 
: EnvAutoProcess( 12, 13 + MAX_FISH_GROUPS) // These need to be updated by hand as input and output vars are added?
, m_paThreshold( 0.5f )
, m_colTMax( -1 )
, m_colInSlices( -1 )
, m_colCarbonStock (-1)
, m_colDisturb (-1)
, m_colAGECLASS (-1)
, m_colStandOrig (-1)
, m_colLulcMed (-1)
, m_colH2ORESIDNT( -1 )
, m_colH2OINDCOMM( -1 )
, m_colH2OPRICRES(-1)
, m_colH2OPRICIND(-1)
, m_colAREA( -1 )
, m_colUGB( -1 )
, m_colPopDens( -1 )
, m_colZONED_USE( -1 )
, m_colLCC( -1 )        // Land capability class
, m_colCOUNTYID( -1 )
, m_colNearestUgb( -1 )
, m_colFRMRNT_LR( -1 )
, m_colFRMRNT_YR( -1 )
, m_colWR_SHUTOFF( -1 )
, m_colWR_SH_NDX( -1 )
, m_colIRRH2OCOST( -1 )
, m_colUW_AC( -1 )
, m_colUW_LRAC( -1 )
, m_colPROB_DEV( -1 )
, m_colPROBUSECHG( -1 )
, m_colPVT( -1 )
, m_colNRUGBndx( -1 )
, m_colSTREAM_LEN( -1 )
, m_colD_UGB_EE( -1 )
, m_colMETRO_RSRV( -1 )
, m_colStateAbbre( -1 )
, m_colPVTCode( -1 )
, m_colCT_Text( -1 )
, m_colSS_Text( -1 )
, m_colSTART_AGE(-1)
, m_colEND_AGE(-1)

, m_disturb (0)
, m_ageClass (0)
, m_lulcMed (0)

, m_useColdStart( 0 )
, m_useColdStart_SWHCfile( false )
, m_useColdStart_ForestSTMfiles( false )
, m_useColdStart_PopUnitsfile( false )
, m_useCarbonModel( 0 )
, m_useFishModel( 0 )
, m_idLandTrans( 0 )
, m_LTtestMode( 0 )
, m_idUW( 0 )
, m_idUX( 0 )
, m_useDGVMvegtypeData( 0 )
, m_idIrr( -1 )
, m_idPG( 0 )
, m_useFR( 0 )
, m_colPopCap( -1 )
, m_colPopAvail( -1 )
, m_colUgaPriority( -1 )
, m_colUgaExpEvent( -1 )
, m_colAllowedDens( -1 )
, m_colNdu( -1 )
, m_colNdu00( -1 )
, m_colNewDU( -1 )
, m_colConserve( -1 )
, m_colDEV_VAL(-1)
, m_colAG_VAL(-1)
, m_colFOR_VAL(-1)

, m_MetroAdjDens( 0 )
, m_EugeneSpringfieldAdjDens( 0 )
, m_SalemAdjDens( 0 )
, m_CorvallisAdjDens( 0 )
, m_AlbanyAdjDens( 0 )
, m_McMinnvilleAdjDens( 0 )
, m_NewbergAdjDens( 0 )
, m_WoodburnAdjDens( 0 )

, m_commonInitComplete( false )
, m_commonInitRunComplete( false )
, m_commonEndRunComplete( false )
, m_climateFilesAreOpen( false )
, m_climateDataHasBeenInitialized( false )
, m_tauClimate( 30. )
, m_tauWR(10.)
, m_yrOfMeanValues( 0 )
, m_records_UPP( -1 )
, m_colUPP_UGB_DH_ID( -1 )
, m_colUPP_UGB_name( -1 )
, m_colUPP_pop2010( -1 )

, m_colLIFT(-1)
, m_colDISTANCE(-1)
, m_colECC0(-1)
, m_colECC(-1)
, m_colIRP0(-1)
, m_colIRP(-1)
, m_colWR_PROB(-1)
, m_colWR_IRRIG_S(-1)
, m_colWRPRIORITY(-1)

, m_colKUCHLER(-1)

, m_colFLOODPLAIN(-1)

, m_colLAI(-1)

, m_colXURBH2OPRC(-1)
, m_colXIRP_MIN(-1)

, m_initial_irrigation_fraction( -1. )
, m_IrrTestMode( 0 )

, m_pCropPG( NULL )
, m_pCropPW( NULL )
, m_pCropSnowpack( NULL )

, m_FRtestMode( 0 )
, m_PGtestMode( 0 )

, m_testMode( 0 )

, m_UWinVarIndex( 0 )
, m_FRinVarIndex( 0 )
, m_LToutVarIndex( 0 )
, m_FishOutVarIndex( 0 )
, m_UXinVarIndex( 0 )
, m_UXoutVarIndex( 0 )
, m_PGoutVarIndex( 0 )
, m_urbanWaterDemandIndex( 0 )

, m_UWinVarCount( 0 )
, m_FRinVarCount( 0 )
, m_LToutVarCount( 0 )
, m_FishOutVarCount( 0 )
, m_UXoutVarCount( 0 )
, m_PGoutVarCount( 0 )
, m_urbanWaterDemandVarCount( 0 ) 

, m_currentClimateScenarioIndex( -1 )
, m_currentPopScenarioIndex( -1 )
, m_currentUGAScenarioIndex( -1 )
, m_currentUWScenarioIndex( -1 )
, m_currentCropScenarioIndex( -1 )
, m_dynamicWRType( -1 )

, m_pDataObj_DGVMvegtype( NULL )
, m_pDataObj_Precip( NULL )
, m_pDataObj_Tmin( NULL )
, m_BasinMeanTminGrowingSeason( 0.0f )
, m_BasinMeanPrcpGrowingSeason( 0.0f )

, m_metroDemandNonResidential( 0.0f )
, m_eugSprDemandNonResidential( 0.0f )
, m_salemKeiserDemandNonResidential( 0.0f )
, m_corvallisDemandNonResidential( 0.0f )
, m_albanyDemandNonResidential( 0.0f )
, m_mcMinnDemandNonResidential( 0.0f )
, m_newbergDemandNonResidential( 0.0f )
, m_woodburnDemandNonResidential( 0.0f )
, m_metroDemandResidential( 0.0f )
, m_eugSprDemandResidential( 0.0f )
, m_salemKeiserDemandResidential( 0.0f )
, m_corvallisDemandResidential( 0.0f )
, m_albanyDemandResidential( 0.0f )
, m_mcMinnDemandResidential( 0.0f )
, m_newbergDemandResidential( 0.0f )
, m_woodburnDemandResidential( 0.0f )
, m_isBasin( false )
, m_pUWmfgInc( NULL )
, m_pUWcommInc( NULL )
, m_pUWcoolingCost( NULL )
, m_pHholdInc( NULL )

, m_FRoutVarIndex(-1)
, m_FRoutVarCount(-1)

, m_currentIrrScenarioIndex(-1)
, m_IrrInVarIndex(-1)
, m_IrrInVarCount(-1)

, m_colD_HWY(-1)
, m_colCENTROIDX(-1)
, m_colCENTROIDY(-1)

, m_colTMIN_GROW(-1)
, m_colPRCP_GROW(-1)
, m_colPRCPSPRING(-1)
, m_colPRCPWINTER(-1)
, m_colPRCP_JUN(-1)
, m_colPRCP_JUL(-1)
, m_colPRCP_AUG(-1)

, m_colTMINGROAVG(-1)
, m_colPRCPGROAVG(-1)
, m_colPRCPJUNAVG(-1)
, m_colPRCPJULAVG(-1)
, m_colPRCPAUGAVG(-1)
   {
   // Add input vars.

   /* 1 */ m_GetWeatherInVarIndex = AddInputVar("UseStationaryClim", m_useStationaryClim, "0=Off, 1=On");
   /* 2 */ AddInputVar("StationaryClimFirstYear", m_stationaryClimFirstYear, "first year of interval");
   /* 3 */ AddInputVar("StationaryClimLastYear", m_stationaryClimLastYear, "last year of interval");
   m_GetWeatherInVarCount = 3;

   /* 1 */ m_DGVMvegtypeDataInVarIndex = AddInputVar("Climate Scenario", m_currentClimateScenarioIndex, "0=Ref, 1=LowClim, 2=HiClim, 3=Stationary, 4=Historical, 5=Baseline");
   m_DGVMvegtypeDataInVarCount = 1;

   /* 1 */ m_IrrInVarIndex = AddInputVar("Climate Scenario", m_currentClimateScenarioIndex, "0=Ref, 1=LowClim, 2=HiClim, 3=Stationary, 4=Historical, 5=Baseline");
   /* 2 */ AddInputVar("Irrigation Scenario", m_currentIrrScenarioIndex, "0=Ref (~2/3), 1=More (~5/6) 2=None (0)");
   /* 3 */ AddInputVar("Crop Scenario", m_currentCropScenarioIndex, "0=Ref, 1=AllFallow");
   m_IrrInVarCount = 3;

   /* 1 */ m_CropInVarIndex = AddInputVar("Crop Scenario", m_currentCropScenarioIndex, "0=Ref, 1=AllFallow");
   m_CropInVarCount = 1;
   
   /* 1 */ m_PGinVarIndex = AddInputVar("Population Scenario", m_currentPopScenarioIndex, "0=Ref, 1=HiPop, 2=NoGrow, 3=Extreme, 4=NoPopGrowth, 5=NoIncGrowth, 6=Baseline");
   m_PGinVarCount = 1;

   /* 1 */ m_FRinVarIndex = AddInputVar("Climate Scenario", m_currentClimateScenarioIndex, "0=Ref, 1=LowClim, 2=HiClim, 3=Stationary, 4=Historical, 5=Baseline");
   /* 2 */ AddInputVar("Dynamic Water Right type", m_dynamicWRType, "0=none, 1=unused, 2=unused, 3=NewIrrig Scenario, 4=Extreme Scenario");
   m_FRinVarCount = 2;

   /* 1 */ m_UXinVarIndex = AddInputVar(_T("UGA Scenario"), m_currentUGAScenarioIndex, "0=Ref, 1=UrbExpand");
   m_UXinVarCount = 1;

   /* 1 */ m_UWinVarIndex = AddInputVar("Urban Water Scenario", m_currentUWScenarioIndex, "0=Ref, 1=FullCostUrb, 2=Extreme, 3=Managed, 4=Short Run Demand Model");
   /* 2 */ AddInputVar("Population Scenario", m_currentPopScenarioIndex, "0=Ref, 1=HiPop, 2=NoGrow, 3=Extreme, 4=NoPopGrowth, 5=NoIncGrowth, 6=Baseline");
   m_UWinVarCount = 2;

   /* 1 */ m_LTinVarIndex = AddInputVar("Population Scenario", m_currentPopScenarioIndex, "0=Ref, 1=HiPop, 2=NoGrow, 3=Extreme, 4=NoPopGrowth, 5=NoIncGrowth, 6=Baseline");
   m_LTinVarCount = 1;

   // Add 13 + MAX_FISH_GROUPS output vars.

   /* 1 */ m_UXoutVarIndex = AddOutputVar(_T("Developed fractions"), &m_UXoutVars, _T(""));
   m_UXoutVarCount = 1;

   /* 1 */ m_urbanWaterDemandIndex = AddOutputVar(_T("Urban Water Demand: Residential, Commercial, and Industry ( ccf per day )"), &m_annualUrbanWaterDemand, _T(""));
   /* 2 */ AddOutputVar(_T("ECON Income"), &m_ECONincomeOutVar, _T(""));
   m_urbanWaterDemandVarCount = 2;

   /* 1 */ m_LToutVarIndex = AddOutputVar("Metro UGB developed fraction", m_UGAdevelopedFrac[UGA_Metro], "");
   /* 2 */ AddOutputVar("Eugene-Springfield UGB developed fraction", m_UGAdevelopedFrac[UGA_EugeneSpringfield], "");
   /* 3 */ AddOutputVar("Salem UGB developed fraction", m_UGAdevelopedFrac[UGA_SalemKeizer], "");
   /* 4 */ AddOutputVar("Corvallis UGB developed fraction", m_UGAdevelopedFrac[UGA_Corvallis], "");
   /* 5 */ AddOutputVar("Albany UGB developed fraction", m_UGAdevelopedFrac[UGA_Albany], "");
   /* 6 */ AddOutputVar("McMinnville UGB developed fraction", m_UGAdevelopedFrac[UGA_McMinnville], "");
   /* 7 */ AddOutputVar("Newberg UGB developed fraction", m_UGAdevelopedFrac[UGA_Newberg], "");
   /* 8 */ AddOutputVar("Woodburn UGB developed fraction", m_UGAdevelopedFrac[UGA_Woodburn], "");
   /* 9 */ AddOutputVar("Ag IDUs eligible to change", m_LTagCount, "");
   /* 10 */ AddOutputVar("Forest IDUs eligible to change", m_LTforestCount, "");
   /* 11 */ AddOutputVar(_T("Land-use Transitions per eligible IDU"), &m_LToutVars, _T("Land-use Transitions per eligible IDU"));
   m_LToutVarCount = 11;

   for (int fish_group_index = 0; fish_group_index < MAX_FISH_GROUPS; fish_group_index++)
      {
      CString msg;
      msg.Format(_T("Species group %d"), fish_group_index + 1);

      int outVarIndex;
      outVarIndex = AddOutputVar(msg.GetString(), m_FishGroupPresenceProbability[fish_group_index], "");
      if (fish_group_index == 0) m_FishOutVarIndex = outVarIndex;
      }
   m_FishOutVarCount = MAX_FISH_GROUPS;

   } // end of WW2100AP default constructor


WW2100AP::~WW2100AP( void )
   {
   if ( m_pUWmfgInc != NULL )
      delete m_pUWmfgInc;

   if ( m_pUWcommInc != NULL )
      delete m_pUWcommInc;

   if ( m_pUWcoolingCost != NULL )
      delete m_pUWcoolingCost;

   if (  m_pHholdInc != NULL )
      delete m_pHholdInc;
   }


int WW2100AP::InputVar( int id, MODEL_VAR** modelVar )
   {
   int numVars = 0;
   *modelVar = NULL;

   if ( m_useColdStart==id ) 
      {
      }
   else if ( m_useCarbonModel==id )
      {
      }
   else if (ID_GETWEATHER == id)
      {
      *modelVar = m_inputVars.GetData() + m_GetWeatherInVarIndex;
      numVars = m_GetWeatherInVarCount;
      } // end of if ( ID_GETWEATHER==id )
   else if (m_idUW == id)
      {
      *modelVar = m_inputVars.GetData() + m_UWinVarIndex;
      numVars = m_UWinVarCount;
      } // end of if ( m_idUW==id )
   else if ( m_idLandTrans==id )
      {
      *modelVar = m_inputVars.GetData() + m_LTinVarIndex;
      numVars = m_LTinVarCount;
      } // end of if ( m_idLandTrans==id )
   else if ( m_useFishModel==id )
      {
      }
   else if ( m_idUX==id )
      {
      *modelVar = m_inputVars.GetData() + m_UXinVarIndex;
      numVars = m_UXinVarCount;
      }
   else if ( m_useDGVMvegtypeData==id )
      {
      *modelVar = m_inputVars.GetData() + m_DGVMvegtypeDataInVarIndex;
      numVars = m_DGVMvegtypeDataInVarCount;
      }
   else if ( m_idIrr==id )
      {
      *modelVar = m_inputVars.GetData() + m_IrrInVarIndex;
      numVars = m_IrrInVarCount;
      }
   else if ( m_idPG==id ) 
      {
      *modelVar = m_inputVars.GetData() + m_PGinVarIndex;
      numVars = m_PGinVarCount;
      }
   else if ( m_useFR == id )
	  {
	   *modelVar = m_inputVars.GetData() + m_FRinVarIndex;
	   numVars = m_FRinVarCount;
      }
   else if (m_useCrop==id)
      { 
      *modelVar = m_inputVars.GetData() + m_CropInVarIndex;
      numVars = m_CropInVarCount;
      }

   return(numVars);
   } // end of WW2100AP::InputVar()


int WW2100AP::OutputVar( int id, MODEL_VAR** modelVar )
   {
   int numVars = 0;
   *modelVar = NULL;

   if ( m_useColdStart==id ) 
      {
      }
   else if ( m_useCarbonModel==id )
      {
      }
   else if ( m_idUW==id )
      {		
		*modelVar = m_outputVars.GetData() + m_urbanWaterDemandIndex;
		numVars = m_urbanWaterDemandVarCount;
      } // end of if ( m_idUW==id )
   else if ( m_idLandTrans==id )
      {
      *modelVar = m_outputVars.GetData() + m_LToutVarIndex;
      numVars = m_LToutVarCount;
      } // end of if ( m_idLandTrans==id )
   else if ( m_useFishModel==id )
      {
      *modelVar = m_outputVars.GetData() + m_FishOutVarIndex;
      numVars = m_FishOutVarCount;
      }
   else if ( m_idUX==id )
      {
      *modelVar = m_outputVars.GetData() + m_UXoutVarIndex;
      numVars = m_UXoutVarCount;
      }
   else if ( m_useDGVMvegtypeData==id )
      {
      }
   else if ( m_idIrr==id )
      {
      }
   else if ( m_idPG==id ) 
      {
      *modelVar = m_outputVars.GetData() + m_PGoutVarIndex;
      numVars = m_PGoutVarCount;
      }
   else if ( m_useFR==id)
      {
      *modelVar = m_outputVars.GetData() + m_FRoutVarIndex;
      numVars = m_FRoutVarCount;
      }
   else if (m_useCrop==id)
      { 
      }

   return(numVars);
   } // end of WW2100AP::OutputVar()


BOOL WW2100AP::Init( EnvContext *pContext, LPCTSTR initStr )
   { 
   m_pEnvContext = pContext;
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;

   m_climateDataHasBeenInitialized = m_climateFilesAreOpen = m_commonInitComplete = false;
   m_climateScenarioName = climateScenarioNames[0]; // Corresponds to default value (0) of m_currentClimateScenarioIndex;
   m_popScenarioName = "Unknown";

   bool ok = LoadXml( pContext, initStr );

   if (!m_commonInitComplete)
      {
      CheckCol(pLayer, m_colRPA, "RPA", TYPE_INT, CC_MUST_EXIST);
      CheckCol(pLayer, m_colAREA, "AREA", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol(pLayer, m_colUGB, "UGB", TYPE_INT, CC_MUST_EXIST);
      CheckCol(pLayer, m_colPOP, "POP", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol(pLayer, m_colOWNER, "OWNER", TYPE_INT, CC_MUST_EXIST);
      CheckCol(pLayer, m_colLulcA, "LULC_A", TYPE_INT, CC_MUST_EXIST);

      CheckCol(pLayer, m_colLIFT, "LIFT", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol(pLayer, m_colDISTANCE, "DISTANCE", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol(pLayer, m_colECC0, "ECC0", TYPE_FLOAT, CC_AUTOADD);
      pLayer->SetColData(m_colECC0, 0, true);
      CheckCol(pLayer, m_colECC, "ECC", TYPE_FLOAT, CC_AUTOADD);
      pLayer->SetColData(m_colECC, 0, true);
      CheckCol(pLayer, m_colIRP0, "IRP0", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol(pLayer, m_colIRP, "IRP", TYPE_FLOAT, CC_AUTOADD);
      CheckCol(pLayer, m_colWR_PROB, "WR_PROB", TYPE_FLOAT, CC_AUTOADD);
      CheckCol(pLayer, m_colWR_IRRIG_S, "WR_IRRIG_S", TYPE_INT, CC_MUST_EXIST);
      CheckCol(pLayer, m_colWRPRIORITY, "WRPRIORITY", TYPE_INT, CC_AUTOADD);
      CheckCol(pLayer, m_colKUCHLER, "KUCHLER", TYPE_INT, CC_AUTOADD);

      CheckCol(pLayer, m_colFLOODPLAIN, "FLOODPLAIN", TYPE_INT, CC_MUST_EXIST);

      CheckCol(pLayer, m_colLAI, "LAI", TYPE_FLOAT, CC_MUST_EXIST);

      CheckCol(pLayer, m_colXURBH2OPRC, "XURBH2OPRC", TYPE_FLOAT, CC_AUTOADD);
      pLayer->SetColData(m_colXURBH2OPRC, 1.0f, true);
      CheckCol(pLayer, m_colXIRP_MIN, "XIRP_MIN", TYPE_FLOAT, CC_AUTOADD);
      pLayer->SetColData(m_colXIRP_MIN, 1.0f, true);

      CheckCol(pLayer, m_colGrdCellNdx, "GrdCellNdx", TYPE_INT, CC_MUST_EXIST);

      CheckCol(pLayer, m_colTMIN_GROW, "TMIN_GROW", TYPE_FLOAT, CC_AUTOADD);
      CheckCol(pLayer, m_colPRCP_GROW, "PRCP_GROW", TYPE_FLOAT, CC_AUTOADD);
      CheckCol(pLayer, m_colPRCPSPRING, "PRCPSPRING", TYPE_FLOAT, CC_AUTOADD);
      CheckCol(pLayer, m_colPRCPWINTER, "PRCPWINTER", TYPE_FLOAT, CC_AUTOADD);
      CheckCol(pLayer, m_colPRCP_JUN, "PRCP_JUN", TYPE_FLOAT, CC_AUTOADD);
      CheckCol(pLayer, m_colPRCP_JUL, "PRCP_JUL", TYPE_FLOAT, CC_AUTOADD);
      CheckCol(pLayer, m_colPRCP_AUG, "PRCP_AUG", TYPE_FLOAT, CC_AUTOADD);

      CheckCol(pLayer, m_colTMINGROAVG, "TMINGROAVG", TYPE_FLOAT, CC_AUTOADD);
      CheckCol(pLayer, m_colPRCPGROAVG, "PRCPGROAVG", TYPE_FLOAT, CC_AUTOADD); 
      CheckCol(pLayer, m_colPRCPJUNAVG, "PRCPJUNAVG", TYPE_FLOAT, CC_AUTOADD);
      CheckCol(pLayer, m_colPRCPJULAVG, "PRCPJULAVG", TYPE_FLOAT, CC_AUTOADD);
      CheckCol(pLayer, m_colPRCPAUGAVG, "PRCPAUGAVG", TYPE_FLOAT, CC_AUTOADD);

      ASSERT(m_colRPA>=0);
      ASSERT(m_colAREA>=0);
      ASSERT(m_colUGB>=0);
      ASSERT(m_colPOP>=0);
      ASSERT(m_colOWNER>=0);
      ASSERT(m_colLulcA>=0);
      InitPopDensCalcs(pContext);

      m_commonInitComplete = true;
      }

   if ( m_useColdStart==pContext->id ) 
      {
      CheckCol( pLayer, m_colSWHC_TOP,  "SWHC_TOP",     TYPE_FLOAT, CC_AUTOADD );
      CheckCol( pLayer, m_colSWHC_MID,  "SWHC_MID",     TYPE_FLOAT, CC_AUTOADD );
      CheckCol( pLayer, m_colSWHC_DEEP,  "SWHC_DEEP",     TYPE_FLOAT, CC_AUTOADD );
      CheckCol( pLayer, m_colSOIL_DEPTH,  "SOIL_DEPTH",     TYPE_FLOAT, CC_AUTOADD );
      CheckCol( pLayer, m_colDGVMvegtyp,  "DGVMvegtyp",  TYPE_INT,   CC_AUTOADD );
      // CheckCol( pLayer, m_colGrdCellNdx,  "GrdCellNdx",  TYPE_INT,   CC_MUST_EXIST );
      CheckCol( pLayer, m_colRPA,         "RPA",         TYPE_INT,   CC_AUTOADD );
      CheckCol( pLayer, m_colIDU_ID,   "IDU_ID",   TYPE_INT,   CC_MUST_EXIST );
      CheckCol( pLayer, m_colZONED_USE,        "ZONED_USE",        TYPE_INT,   CC_AUTOADD );
      CheckCol( pLayer, m_colUGB_NAME,     "UGB_NAME",     TYPE_STRING, CC_AUTOADD );
      CheckCol( pLayer, m_colUGB,         "UGB",         TYPE_INT,   CC_MUST_EXIST );
      CheckCol( pLayer, m_colIRRIGATION,  "IRRIGATION",  TYPE_INT, CC_AUTOADD );
      CheckCol( pLayer, m_colLulcA,       "LULC_A",      TYPE_LONG,  CC_MUST_EXIST );      
      CheckCol( pLayer, m_colWREXISTS,    "WREXISTS",    TYPE_INT,  CC_AUTOADD );

      CheckCol( pLayer, m_colStateAbbre, "StateAbbre",   TYPE_STRING, CC_MUST_EXIST );
      CheckCol( pLayer, m_colPVTCode,    "PVTCode",      TYPE_STRING, CC_MUST_EXIST );
      CheckCol( pLayer, m_colCT_Text,    "CT_Text",      TYPE_STRING, CC_AUTOADD );
      CheckCol( pLayer, m_colSS_Text,    "SS_Text",      TYPE_STRING, CC_AUTOADD );
      CheckCol( pLayer, m_colPVT,        "PVT",          TYPE_INT, CC_AUTOADD );
      CheckCol(pLayer, m_colVEGCLASS, "VEGCLASS", TYPE_INT, CC_MUST_EXIST);
      CheckCol(pLayer, m_colAGECLASS, "AGECLASS", TYPE_INT, CC_MUST_EXIST);

      m_colSTART_AGE = pLayer->GetFieldCol("START_AGE");
      m_colEND_AGE = pLayer->GetFieldCol("END_AGE");
      m_colMEDIAN_AGE = pLayer->GetFieldCol("MEDIAN_AGE");

      InitColdStart( pContext );
      }

   if ( m_useCarbonModel==pContext->id )
      {
      CheckCol( pLayer, m_colCarbonStock, "CARBONSTOC",  TYPE_FLOAT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colDisturb,     "DISTURB" ,    TYPE_INT,   CC_MUST_EXIST );
      CheckCol( pLayer, m_colAGECLASS,    "AGECLASS" ,   TYPE_INT,   CC_MUST_EXIST );
      CheckCol( pLayer, m_colLulcMed,       "LULC_B",      TYPE_LONG,  CC_MUST_EXIST );
      CheckCol( pLayer, m_colStandOrig,   "STANDORIG",   TYPE_LONG,  CC_MUST_EXIST );
      
      InitCarbon( pContext );
      }

   if ( m_idUW==pContext->id )
      {

      CheckCol( pLayer, m_colH2ORESIDNT, "H2ORESIDNT" , TYPE_FLOAT, CC_AUTOADD );      // urban water 
      CheckCol( pLayer, m_colH2OINDCOMM, "H2OINDCOMM" , TYPE_FLOAT, CC_AUTOADD );      // urban water 
      CheckCol( pLayer, m_colAREA,       "AREA",        TYPE_DOUBLE, CC_MUST_EXIST );   // urban water 
      CheckCol( pLayer, m_colUGB,        "UGB",         TYPE_INT,   CC_MUST_EXIST );   // urban water 
      CheckCol( pLayer, m_colCOUNTYID,   "COUNTYID",    TYPE_INT,   CC_MUST_EXIST );   // ag trans 
      CheckCol( pLayer, m_colGrdCellNdx,  "GrdCellNdx",  TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colPOP, "POP" , TYPE_FLOAT, CC_MUST_EXIST );      // urban water 
      CheckCol( pLayer, m_colWREXISTS,    "WREXISTS",    TYPE_INT,  CC_AUTOADD );

		pLayer->SetColData(m_colH2ORESIDNT, VData(0),true );
		pLayer->SetColData(m_colH2OINDCOMM, VData(0), true);

		m_annualUrbanWaterDemand.Clear();
		m_annualUrbanWaterDemand.SetName(_T("UrbanWaterDemand( ccf per day )"));
		m_annualUrbanWaterDemand.SetSize(22, 0);
		m_annualUrbanWaterDemand.SetLabel(0, "Year");
		m_annualUrbanWaterDemand.SetLabel(1, "Metro-Residential");
		m_annualUrbanWaterDemand.SetLabel(2, "Eugene-Springfield-Residential");
		m_annualUrbanWaterDemand.SetLabel(3, "Salem-Keiser-Residential");
		m_annualUrbanWaterDemand.SetLabel(4, "Corvallis-Residential");
		m_annualUrbanWaterDemand.SetLabel(5, "Albany-Total");
		m_annualUrbanWaterDemand.SetLabel(6, "McMinnville-Total");
		m_annualUrbanWaterDemand.SetLabel(7, "Newberg-Total");
		m_annualUrbanWaterDemand.SetLabel(8, "Woodburn-Total");
		m_annualUrbanWaterDemand.SetLabel(9, "Metro-NonResidential");
		m_annualUrbanWaterDemand.SetLabel(10, "Eugene-Springfield-NonResidential");
		m_annualUrbanWaterDemand.SetLabel(11, "Salem-Keiser-NonResidential");
		m_annualUrbanWaterDemand.SetLabel(12, "Corvallis-NonResidential");
      m_annualUrbanWaterDemand.SetLabel(13, "BentonCo-RuralResidental (ccf/day/household)");
      m_annualUrbanWaterDemand.SetLabel(14, "ClackamasCo-RuralResidental (ccf/day/household)");
      m_annualUrbanWaterDemand.SetLabel(15, "LaneCo-RuralResidental (ccf/day/household)");
      m_annualUrbanWaterDemand.SetLabel(16, "LinnCo-RuralResidental (ccf/day/household)");
      m_annualUrbanWaterDemand.SetLabel(17, "MarionCo-RuralResidental (ccf/day/household)");
      m_annualUrbanWaterDemand.SetLabel(18, "MultnomahCo-RuralResidental (ccf/day/household)");
      m_annualUrbanWaterDemand.SetLabel(19, "PolkCo-RuralResidental (ccf/day/household)");
      m_annualUrbanWaterDemand.SetLabel(20, "WashingtonCo-RuralResidental (ccf/day/household)");
      m_annualUrbanWaterDemand.SetLabel(21, "YamhillCo-RuralResidental (ccf/day/household)");

      ASSERT(m_colUGB>=0); 
      ASSERT(m_colAREA>=0); 
      ASSERT(m_colPOP>=0); 
      ASSERT(m_colH2ORESIDNT>=0 && m_colH2OINDCOMM>=0);
      ASSERT(m_colCOUNTYID>=0); 
      ASSERT(m_colGrdCellNdx>=0);
      // The UW_AC, UW_LRAC, H2OPRICRES, and H2OPRICIND columns are optional; they are used only if they are present.

      m_ECONincomeOutVar.Clear();
      m_ECONincomeOutVar.SetName(_T("ECON Income ($ and $M)"));
      m_ECONincomeOutVar.AppendCols(34);
      m_ECONincomeOutVar.SetLabel(0, "Year");
      m_ECONincomeOutVar.SetLabel(1, "Benton household inc ($)");
      m_ECONincomeOutVar.SetLabel(2, "Clackamas household inc ($)");
      m_ECONincomeOutVar.SetLabel(3, "Columbia household inc ($)");
      m_ECONincomeOutVar.SetLabel(4, "Lane household inc ($)");
      m_ECONincomeOutVar.SetLabel(5, "Linn household inc ($)");
      m_ECONincomeOutVar.SetLabel(6, "Marion household inc ($)");
      m_ECONincomeOutVar.SetLabel(7, "Multnomah household inc ($)");
      m_ECONincomeOutVar.SetLabel(8, "Polk household inc ($)");
      m_ECONincomeOutVar.SetLabel(9, "Washington household inc ($)");
      m_ECONincomeOutVar.SetLabel(10, "Yamhill household inc ($)");
      m_ECONincomeOutVar.SetLabel(11, "Metro household inc ($)");
      m_ECONincomeOutVar.SetLabel(12, "Benton mfg inc ($M)");
      m_ECONincomeOutVar.SetLabel(13, "Clackamas mfg inc ($M)");
      m_ECONincomeOutVar.SetLabel(14, "Columbia mfg inc ($M)");
      m_ECONincomeOutVar.SetLabel(15, "Lane mfg inc ($M)");
      m_ECONincomeOutVar.SetLabel(16, "Linn mfg inc ($M)");
      m_ECONincomeOutVar.SetLabel(17, "Marion mfg inc ($M)");
      m_ECONincomeOutVar.SetLabel(18, "Multnomah mfg inc ($M)");
      m_ECONincomeOutVar.SetLabel(19, "Polk mfg inc ($M)");
      m_ECONincomeOutVar.SetLabel(20, "Washington mfg inc ($M)");
      m_ECONincomeOutVar.SetLabel(21, "Yamhill mfg inc ($M)");
      m_ECONincomeOutVar.SetLabel(22, "Metro mfg inc ($M)");
      m_ECONincomeOutVar.SetLabel(23, "Benton comm inc ($M)");
      m_ECONincomeOutVar.SetLabel(24, "Clackamas comm inc ($M)");
      m_ECONincomeOutVar.SetLabel(25, "Columbia comm inc ($M)");
      m_ECONincomeOutVar.SetLabel(26, "Lane comm inc ($M)");
      m_ECONincomeOutVar.SetLabel(27, "Linn comm inc ($M)");
      m_ECONincomeOutVar.SetLabel(28, "Marion comm inc ($M)");
      m_ECONincomeOutVar.SetLabel(29, "Multnomah comm inc ($M)");
      m_ECONincomeOutVar.SetLabel(30, "Polk comm inc ($M)");
      m_ECONincomeOutVar.SetLabel(31, "Washington comm inc ($M)");
      m_ECONincomeOutVar.SetLabel(32, "Yamhill comm inc ($M)");
      m_ECONincomeOutVar.SetLabel(33, "Metro comm inc ($M)");

      InitUrbanWater(pContext);

      } // end of if ( m_idUW==pContext->id )

   if ( m_idLandTrans==pContext->id )
      {
      CheckCol( pLayer, m_colPROB_DEV,   "PROB_DEV",    TYPE_DOUBLE, CC_AUTOADD);
      CheckCol( pLayer, m_colPROBUSECHG, "PROBUSECHG",  TYPE_DOUBLE, CC_AUTOADD);
      CheckCol( pLayer, m_colNRUGBndx, "NRUGBndx",  TYPE_INT, CC_AUTOADD);
      CheckCol( pLayer, m_colSTREAM_LEN, "STREAM_LEN",  TYPE_FLOAT, CC_AUTOADD);
      CheckCol( pLayer, m_colD_UGB_EE, "D_UGB_EE",  TYPE_FLOAT, CC_AUTOADD);
      CheckCol( pLayer, m_colPVT, "PVT",  TYPE_INT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colAGECLASS, "AGECLASS",  TYPE_INT, CC_MUST_EXIST);

      CheckCol( pLayer, m_colOWNER, "OWNER",  TYPE_INT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colCOUNTYID, "CountyId",  TYPE_INT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colLulcA, "LULC_A",  TYPE_INT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colAREA, "AREA", TYPE_DOUBLE, CC_MUST_EXIST ); 
      CheckCol( pLayer, m_colD_CITY_20K, "D_CITY_20K", TYPE_FLOAT, CC_MUST_EXIST );   
      CheckCol( pLayer, m_colSLOPE_DEG, "SLOPE_DEG", TYPE_FLOAT, CC_MUST_EXIST );   
      CheckCol( pLayer, m_colFRMRNT_LR, "FRMRNT_LR", TYPE_FLOAT, CC_MUST_EXIST );   
      CheckCol( pLayer, m_colELEV, "ELEV_MEAN", TYPE_DOUBLE, CC_MUST_EXIST );
      CheckCol( pLayer, m_colVEGCLASS, "VEGCLASS", TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colUGB, "UGB", TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colPOP, "POP", TYPE_FLOAT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colPopDens, "POPDENS", TYPE_FLOAT, CC_MUST_EXIST );
	  CheckCol(pLayer, m_colDEV_VAL, "DEV_VAL", TYPE_FLOAT, CC_AUTOADD);
	  CheckCol(pLayer, m_colAG_VAL, "AG_VAL", TYPE_FLOAT, CC_AUTOADD);
	  CheckCol(pLayer, m_colFOR_VAL, "FOR_VAL", TYPE_FLOAT, CC_AUTOADD);



      ASSERT(m_colOWNER>=0);
      ASSERT(m_colCOUNTYID>=0); 
      ASSERT(m_colLulcA>=0);
      ASSERT(m_colPROBUSECHG>=0);
      ASSERT(m_colUGB>=0); 
      ASSERT(m_colAREA>=0); 
      ASSERT(m_colD_CITY_20K>=0); 
      ASSERT(m_colSLOPE_DEG>=0); 
      ASSERT(m_colFRMRNT_LR>=0); 
      ASSERT(m_colELEV>=0); 
      ASSERT(m_colSTREAM_LEN>=0);  
      ASSERT(m_colD_UGB_EE>=0);  
      ASSERT(m_colPROB_DEV>=0); 
      ASSERT(m_colPVT>=0); 
      ASSERT(m_colAGECLASS>=0); 
      ASSERT(m_colVEGCLASS>=0); 
      ASSERT(m_colNRUGBndx>=0); 
      ASSERT(m_colPOP>=0);
      ASSERT(m_colPopDens>=0);

      InitLandTrans( pContext );


      m_LToutVars.Clear();
      m_LToutVars.SetName(_T("LandUseTransitionPerEligibleIDU"));
      m_LToutVars.AppendCols(5);
      m_LToutVars.SetLabel(0, "Year");
      m_LToutVars.SetLabel(1, "Ag -> Developed");
      m_LToutVars.SetLabel(2, "Ag -> Forest");
      m_LToutVars.SetLabel(3, "Forest -> Developed");
      m_LToutVars.SetLabel(4, "Forest -> Ag");

      } // end of if ( m_idLandTrans==pContext->id )

   if ( m_useFishModel==pContext->id )
      {     
      MapLayer *pStreamLayer = (MapLayer*)pContext->pReachLayer;
      CheckCol( pStreamLayer, m_colInSlices, "IN_SLICES", TYPE_LONG, CC_MUST_EXIST );
      CheckCol( pStreamLayer, m_colTMax,     "TEMP_MAX",  TYPE_FLOAT, CC_AUTOADD );
      
      // add a column for each species group
      for ( int i=0; i < GetSppGroupCount(); i++ )
         {
         SpeciesGroup *pGroup = m_sppGroupArray[ i ];
         CheckCol( pStreamLayer, pGroup->m_colScore, pGroup->m_name, TYPE_LONG, CC_AUTOADD );
         }

      InitFish( pContext );
      }

   if ( m_idUX==pContext->id )
      {
      CheckCol( pLayer, m_colOWNER,  "OWNER",   TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colUGB,    "UGB",     TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colLulcA,  "LULC_A",  TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colAREA,   "AREA",    TYPE_DOUBLE, CC_MUST_EXIST );
      CheckCol( pLayer, m_colIDU_ID, "IDU_ID", TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colZONED_USE,  "ZONED_USE",     TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colMETRO_RSRV, "METRO_RSRV", TYPE_INT, CC_AUTOADD );
      CheckCol(pLayer, m_colRPA, "RPA", TYPE_INT, CC_MUST_EXIST);
      CheckCol(pLayer, m_colD_HWY, "D_HWY", TYPE_DOUBLE, CC_MUST_EXIST);
      CheckCol(pLayer, m_colCENTROIDX, "CENTROIDX", TYPE_LONG, CC_MUST_EXIST);
      CheckCol(pLayer, m_colCENTROIDY, "CENTROIDY", TYPE_LONG, CC_MUST_EXIST);

      ASSERT(m_colOWNER>=0);
      ASSERT(m_colUGB>=0);
      ASSERT(m_colLulcA>=0);
      ASSERT(m_colAREA>=0);
      ASSERT(m_colIDU_ID>=0);
      ASSERT(m_colZONED_USE>=0);
      ASSERT(m_colMETRO_RSRV>=0);
      ASSERT(m_colRPA>=0);
      ASSERT(m_colD_HWY >= 0);
      ASSERT(m_colCENTROIDX >= 0);
      ASSERT(m_colCENTROIDY >= 0);

      InitUX(pContext);
      }

   if ( m_useDGVMvegtypeData==pContext->id )
      {
      CheckCol( pLayer, m_colDGVMvegtyp,  "DGVMvegtyp",  TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colGrdCellNdx,  "GrdCellNdx",  TYPE_INT, CC_MUST_EXIST );
      
      for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
         {
         int vtype = -1; pLayer->GetData(idu, m_colDGVMvegtyp, vtype);
         int kuchler = CrosswalkVtypeToKuchler(vtype);
         bool saveFlag = pLayer->m_readOnly;  pLayer->m_readOnly = false;
         pLayer->SetData(idu, m_colKUCHLER, kuchler);
         pLayer->m_readOnly = saveFlag;
         } // end of loop thru IDUs to initialize KUCHLER

      } // end of if ( m_useDGVMvegtypeData==pContext->id )

   if ( m_idIrr==pContext->id )
      {
      CheckCol( pLayer, m_colELEV, "ELEV_MEAN", TYPE_DOUBLE, CC_MUST_EXIST );
      CheckCol( pLayer, m_colLCC, "LCC", TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colZONED_USE, "ZONED_USE", TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colWREXISTS, "WREXISTS", TYPE_INT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colDRAINAGE, "DRAINAGE", TYPE_FLOAT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colD_GWATER, "DEP_GWAT", TYPE_FLOAT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colAWS, "AWS", TYPE_FLOAT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colD_WRIVER, "D_WRIVER", TYPE_FLOAT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colD_CITY_50K, "D_CITY_50K", TYPE_FLOAT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colLulcA,  "LULC_A",  TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colIRRIGATION,  "IRRIGATION",  TYPE_INT, CC_AUTOADD );
      CheckCol( pLayer, m_colGrdCellNdx,  "GrdCellNdx",  TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colFRMRNT_LR, "FRMRNT_LR", TYPE_FLOAT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colIDU_ID, "IDU_ID", TYPE_INT, CC_MUST_EXIST);
      CheckCol(pLayer, m_colAREA, "AREA", TYPE_DOUBLE, CC_MUST_EXIST);
      // CheckCol( pLayer, m_col, "", TYPE_, CC_MUST_EXIST );

      InitIrrChoice( pContext );
      }

   if ( m_idPG==pContext->id )
      {
      CheckCol( pLayer, m_colRPA,  "RPA",  TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colUGB,  "UGB",  TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colAREA,  "AREA",  TYPE_DOUBLE, CC_MUST_EXIST );
      CheckCol( pLayer, m_colLulcA,  "LULC_A",  TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colOWNER,  "OWNER",  TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colPOP,  "POP",  TYPE_FLOAT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colPopDens,  "POPDENS",  TYPE_FLOAT, CC_MUST_EXIST );
      
      m_PGoutVars.Clear();
      m_PGoutVars.SetName(_T("UGAadjPopDens(people per acre)"));
      m_PGoutVars.AppendCols(9);
      m_PGoutVars.SetLabel(0, "Year");
      m_PGoutVars.SetLabel(1, "Metro");
      m_PGoutVars.SetLabel(2, "Eugene-Springfield");
      m_PGoutVars.SetLabel(3, "Salem-Keiser");
      m_PGoutVars.SetLabel(4, "Corvallis");
      m_PGoutVars.SetLabel(5, "Albany");
      m_PGoutVars.SetLabel(6, "McMinnville");
      m_PGoutVars.SetLabel(7, "Newberg");
      m_PGoutVars.SetLabel(8, "Woodburn");
/* 1 */ m_PGoutVarIndex = AddOutputVar(_T("Pop Growth: UGA adjusted densities(people per acre)"), &m_PGoutVars, _T(""));
		m_PGoutVarCount = 1;
	  }

   if (m_useFR==pContext->id)
      {
      CheckCol( pLayer, m_colLCC, "LCC", TYPE_INT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colWREXISTS, "WREXISTS", TYPE_INT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colD_GWATER, "DEP_GWAT", TYPE_FLOAT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colAREA, "AREA", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colIRRIGATION, "IRRIGATION", TYPE_INT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colELEV, "ELEV_MEAN", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colWR_SHUTOFF, "WR_SHUTOFF", TYPE_INT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colFRMRNT_LR, "FRMRNT_LR", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colFRMRNT_YR, "FRMRNT_YR", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colWR_SH_NDX, "WR_SH_NDX", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colIRRH2OCOST, "IRRH2OCOST", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colLulcA, "LULC_A", TYPE_INT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colGrdCellNdx, "GrdCellNdx", TYPE_INT, CC_MUST_EXIST);

      m_FRoutVars.Clear();
      m_FRoutVars.SetName(_T("ECON Metrics"));
      m_FRoutVars.AppendCols(10);
      m_FRoutVars.SetLabel(0, "Year");
      m_FRoutVars.SetLabel(1, "Water scarcity at efficient margin ($ per ac)");
      m_FRoutVars.SetLabel(2, "IDU having water scarcity at efficient margin (IDU index)");
      m_FRoutVars.SetLabel(3, "Water scarcity at efficient margin ($ per ac) for IDUs with LCC 1 thru 4");
      m_FRoutVars.SetLabel(4, "IDU having water scarcity at efficient margin for IDUs with LCC 1 thru 4 (IDU index)");
      m_FRoutVars.SetLabel(5, "Water scarcity at legal margin for irrigation water rights ($ per ac)");
      m_FRoutVars.SetLabel(6, "IDU having water scarcity at legal margin for irrigation water rights (IDU index)");
      m_FRoutVars.SetLabel(7, "priority of IDU having water scarcity at legal margin for irrigation water rights");
      m_FRoutVars.SetLabel(8, "Welfare loss for irrigation at regulated margin ($)");
      m_FRoutVars.SetLabel(9, "Winter precip over WRB (mm)");
      /* 1 */ m_FRoutVarIndex = AddOutputVar(_T("ECON Metrics"), &m_FRoutVars, _T(""));
      m_FRoutVarCount = 1;
      ok &= InitFR( pContext );
      }

   if (m_useCrop==pContext->id)
      { // CheckCol( pLayer, m_col, "", TYPE_, CC_MUST_EXIST);
      // independent variables
      CheckCol( pLayer, m_colLCC, "LCC", TYPE_INT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colELEV, "ELEV_MEAN", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colSLOPE_DEG, "SLOPE_DEG", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colWREXISTS, "WREXISTS", TYPE_INT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colWR_SH_NDX, "WR_SH_NDX", TYPE_FLOAT, CC_MUST_EXIST);
      CheckCol( pLayer, m_colFRMRNT_LR, "FRMRNT_LR", TYPE_FLOAT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colGrdCellNdx, "GrdCellNdx", TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colLulcA, "LULC_A", TYPE_INT, CC_MUST_EXIST );

      // dependent variables
      CheckCol( pLayer, m_colLulcMed, "LULC_B", TYPE_INT, CC_MUST_EXIST );
      CheckCol( pLayer, m_colVEGCLASS, "VEGCLASS", TYPE_INT, CC_MUST_EXIST );

      ok &= InitCrop( pContext );
      }

   return ok;
   } // end of WW2100AP::Init()


BOOL WW2100AP::InitRun( EnvContext *pContext, bool useInitSeed )
   {   
   MapLayer *pLayer = (MapLayer*)pContext->pMapLayer;
      
   if (pContext->id==ID_COLDSTART) return true; // Don't do anything at InitRun time.

   BOOL rtnFlag = TRUE;

   if (!m_commonInitRunComplete)
      {
      CString msg;

      pLayer->SetColData(m_colXURBH2OPRC, 1.0f, true);
      pLayer->SetColData(m_colXIRP_MIN, 1.0f, true);

      if (m_currentCropScenarioIndex == -1) m_currentCropScenarioIndex = 0; // Used by both Crop Choice and Irrigation Decision models

      if (m_currentClimateScenarioIndex == -1) m_currentClimateScenarioIndex = 0;
      int num_climate_scenarios = sizeof(climateScenarioNames)/sizeof(char *);
      m_climateScenarioName = (0<=m_currentClimateScenarioIndex && m_currentClimateScenarioIndex<num_climate_scenarios) ?
         climateScenarioNames[m_currentClimateScenarioIndex] : "Unknown";

      msg.Format("WW2100AP.InitRun: pContext->id, m_currentClimateScenarioIndex, m_climateScenarioName = %d, %d, %s", 
            pContext->id, m_currentClimateScenarioIndex, m_climateScenarioName);
      Report::LogMsg(msg);  

      if (m_currentPopScenarioIndex == -1) m_currentPopScenarioIndex = 0;
      int num_pop_scenarios = sizeof(PopScenarioNames) / sizeof(char *);
      m_popScenarioName = (0 <= m_currentPopScenarioIndex && m_currentPopScenarioIndex<num_pop_scenarios) ?
         PopScenarioNames[m_currentPopScenarioIndex] : "Unknown";

      msg.Format("WW2100AP.InitRun: m_currentPopScenarioIndex, m_popScenarioName = %d, %s",
         m_currentPopScenarioIndex, m_popScenarioName);
      Report::LogMsg(msg);

      m_pHholdInc = new FDataObj;
      CString pathAndFileName;
      pathAndFileName = ReplaceSubstring(m_HholdInc_file, "POPULATION_SCENARIO_NAME", m_popScenarioName);
      m_recordsHholdInc = m_pHholdInc->ReadAscii(pathAndFileName, ',', TRUE);
      if (m_recordsHholdInc <= 0)
         {
         CString msg;
         msg.Format("WW2100AP: Missing data. m_recordsHholdInc = %d", m_recordsHholdInc);
         Report::ErrorMsg(msg);
         return false;
         }
      if (!ConfirmCols(m_pHholdInc))
         {
         CString msg;
         msg.Format("WW2100AP: Columns missing or out of order. ConfirmCols(m_pHholdInc) = %d", ConfirmCols(m_pHholdInc));
         Report::ErrorMsg(msg);
         return false;
         }
      m_HholdInc_yr0 = m_pHholdInc->GetAsInt(0, 0);

      // Clear out the data rows in the DataObj output vars.
      m_UXoutVars.ClearRows();
      m_annualUrbanWaterDemand.ClearRows();
      m_LToutVars.ClearRows();

      m_commonEndRunComplete = false;
      m_commonInitRunComplete = true;
      }

   if (m_useDGVMvegtypeData==pContext->id) rtnFlag = rtnFlag && InitRunDGVMvegtypeData( pContext );
   if ( m_idUX==pContext->id ) rtnFlag = rtnFlag && InitRunUX(pContext);
   if ( m_idLandTrans==pContext->id ) rtnFlag = rtnFlag && InitRunLandTrans( pContext );
   if ( m_idUW==pContext->id ) rtnFlag = rtnFlag && InitRunUrbanWater( pContext );
   if (m_idIrr==pContext->id) rtnFlag = rtnFlag && InitRunIrrChoice( pContext );
   if (m_idPG==pContext->id) rtnFlag = rtnFlag && InitRunPG(pContext); 
   if (m_useFR==pContext->id) rtnFlag = rtnFlag && InitRunFR( pContext );
   if (m_useCrop==pContext->id) rtnFlag = rtnFlag && InitRunCrop( pContext );

	m_PGoutVars.ClearRows();
	m_annualUrbanWaterDemand.ClearRows();

   return rtnFlag;
   } // end of WW2100AP::InitRun()


bool WW2100AP::RunGetWeather(EnvContext *pContext)
   {
   if (m_currentClimateScenarioIndex == STATIONARY_CLIM_SCENARIO_INDEX // For backward compatibility with WW2100
      || m_useStationaryClim != 0)
      {
      double rand_num = m_WeatherRandomDraw.GetUnif01();
      pContext->weatherYear = (int)(rand_num * (m_stationaryClimLastYear - m_stationaryClimFirstYear) + m_stationaryClimFirstYear);

      CString msg;
      msg.Format("RunGetWeather() rand_num = %f, m_stationaryClimFirstYear = %d, m_stationaryClimLastYear = %d, pContext->weatherYear = %d",
         rand_num, m_stationaryClimFirstYear, m_stationaryClimLastYear, pContext->weatherYear);
      Report::LogMsg(msg);
      }
   else pContext->weatherYear = pContext->currentYear;

   return(true);
   } // end of RunGetWeather()


BOOL WW2100AP::Run(EnvContext *pContext)
   {
   // testMessage(pContext, _T("Run"));
   // If this is the cold start instantiation of WW2100AP, don't do anything at Run time.
   if (pContext->id==ID_COLDSTART) return true; 

   if (pContext->id == ID_GETWEATHER) return(RunGetWeather(pContext));

   if ( m_useCarbonModel==pContext->id )     RunCarbon    ( pContext );
   if ( m_useFishModel==pContext->id )       RunFish      ( pContext );
   if ( m_idLandTrans==pContext->id)    RunLandTrans   ( pContext );
   if ( m_idUW==pContext->id ) RunUrbanWater( pContext );
   if ( m_idUX==pContext->id ) RunUX( pContext );
   if ( m_useDGVMvegtypeData==pContext->id ) RunDGVMvegtypeData( pContext );
   if ( m_idIrr==pContext->id ) RunIrrChoice( pContext );
   if ( m_idPG==pContext->id ) RunPopGrowth( pContext );
   if ( m_useFR==pContext->id ) RunFR( pContext );
   if ( m_useCrop==pContext->id) RunCrop( pContext );

   return TRUE;
   }


BOOL WW2100AP::EndRun( EnvContext *pContext )
   {

   if (!m_commonEndRunComplete)
      {
      m_currentPopScenarioIndex = -1;
      m_currentClimateScenarioIndex = -1;
      m_currentUGAScenarioIndex = -1;
      m_currentUWScenarioIndex = -1;
      m_currentIrrScenarioIndex = -1;

	  if (m_pHholdInc != NULL) { delete m_pHholdInc; m_pHholdInc = NULL; }

      m_commonInitRunComplete = false;
      m_commonEndRunComplete = true;
      }

   if (m_idUW == pContext->id)
      {
	  if (m_pUWmfgInc != NULL) { delete m_pUWmfgInc; m_pUWmfgInc = NULL; }
	  if (m_pUWcommInc != NULL) { delete m_pUWcommInc; m_pUWcommInc = NULL; }
      }

   if (m_idPG == pContext->id)
	   {
	   m_UGApopProj_table.Clear();
	   m_records_UPP = -1;
	   }

   m_climateDataHasBeenInitialized = false;
   if (m_climateFilesAreOpen)
      {
      m_pDataObj_Precip->Close();
      m_pDataObj_Tmin->Close();
      m_climateFilesAreOpen = false;
	  if (m_pDataObj_Precip != NULL) { delete m_pDataObj_Precip; m_pDataObj_Precip = NULL; }
	  if (m_pDataObj_Tmin != NULL) { delete m_pDataObj_Tmin; m_pDataObj_Tmin = NULL; }
      }

   if (m_pDataObj_DGVMvegtype!=NULL)
      {
      m_pDataObj_DGVMvegtype->Close();
      if (m_pDataObj_DGVMvegtype != NULL) { delete m_pDataObj_DGVMvegtype; m_pDataObj_DGVMvegtype = NULL; }
      }

   return true;
   } // end of EndRun()


bool WW2100AP::InitColdStart( EnvContext *pContext )
   {
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;
   m_projectionWKT = pContext->pMapLayer->m_projection;

   CString msg;

   if (m_useColdStart_vegclass_initialization)
      {
      // int idu_count = 0;
      for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
         {
         int vegclass, pvt;
         pLayer->GetData(idu, m_colVEGCLASS, vegclass);
         pLayer->GetData(idu, m_colPVT, pvt);
         if (vegclass>=1000000 && pvt>=1) continue;

         if (vegclass<1000000)
            {
            CString CTSS_Text, PVT_Text, CT_Text, SS_Text;
            pLayer->GetData( idu, m_colPVTCode,  PVT_Text);
            pLayer->GetData( idu, m_colStateAbbre,  CTSS_Text);
            VegState iduVS(PVT_Text, CTSS_Text);

            if (iduVS.Forest())
               {
               int stratum = iduVS.Stratum();      
               pLayer->SetData( idu, m_colPVT, stratum );
               int ctss = iduVS.CTSS();
               pLayer->SetData( idu, m_colVEGCLASS, ctss);
               pLayer->SetData( idu, m_colCT_Text, iduVS.CoverType());
               pLayer->SetData( idu, m_colSS_Text, iduVS.StructuralStage());
               }
            else
               {
               // pLayer->SetData( idu, m_colPVT, -99 );
               // pLayer->SetData( idu, m_colCT_Text, _T(""));
               // pLayer->SetData( idu, m_colSS_Text, _T(""));
               if (iduVS.m_foundCTflag || iduVS.m_foundSSflag)
                  {
                  msg.Format("WW2100AP: inconsistent veg state: PVTCode, StateAbbrev = %s, %s in ColdStart", (LPCTSTR) PVT_Text, (LPCTSTR) CTSS_Text);
                  Report::LogMsg(msg);
                  }
               }
            }
         // idu_count++;
         // msg.Format("*** vegclass initialization first loop; idu_count = %d", idu_count);
         // Report::LogMsg(msg);
         } // end of loop thru IDUs
       msg.Format("WW2100AP: first part of vegclass initialization is complete");
       Report::LogMsg(msg);
      
      // Now deal with forested IDUs which have unknown PVTs
      QueryEngine *pQE = new QueryEngine(pLayer);
      int pass_count = 0;
      int found_count = 0;
      do
         {  
         pass_count++;
         msg.Format("WW2100AP: *** second part of vegclass initialization: starting pass %d now", pass_count);
         Report::LogMsg(msg); 
         found_count = 0;
         int not_found_count = 0;
         for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
            { 
            int vegclass, stratum, idu_id;
            pLayer->GetData(idu, m_colVEGCLASS, vegclass);
            if (vegclass<1000000) continue;
            pLayer->GetData(idu, m_colPVT, stratum);
            if (stratum>=1) continue;
            pLayer->GetData(idu, m_colIDU_ID, idu_id);
            CString query_str;
            query_str.Format("VEGCLASS = %d and PVT>=1 and NextTo(IDU_ID = %d)", vegclass, idu_id);
            Query *pIdu_query = pQE->ParseQuery(query_str, -1, "VEGCLASS query");
            ASSERT(pIdu_query!=NULL); 

            bool result = false;
            for ( MapLayer::Iterator iduTgt = pLayer->Begin( ); !result && (iduTgt != pLayer->End()); iduTgt++ )
               {               
               bool ok = pIdu_query->Run( iduTgt, result );

               if (result)
                  { int pvtTgt;
                  pLayer->GetData(iduTgt, m_colPVT, pvtTgt);
                  pLayer->SetData( idu, m_colPVT, pvtTgt );
                  found_count++;
                  msg.Format("WW2100AP: inferred PVT %d for VEGCLASS %d, found_count = %d", pvtTgt, vegclass, found_count);
                  Report::LogMsg(msg);
                  }
               } // end of inner loop thru IDUs
            if (!result) 
               {
               not_found_count++;
               msg.Format("WW2100AP: did not infer PVT for VEGCLASS %d, not_found_count = %d, found_count = %d", 
                     vegclass, not_found_count, found_count);
               Report::LogMsg(msg);
               }
            } // end of outer loop thru IDUs
         } while (found_count>0);

      pass_count = 0;
      found_count = 0;
      do
         {  
         pass_count++;
         msg.Format("WW2100AP: *** third part of vegclass initialization: starting pass %d now", pass_count);
         Report::LogMsg(msg); 
         found_count = 0;
         int not_found_count = 0;
         for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
            { 
            int vegclass, stratum, idu_id;
            pLayer->GetData(idu, m_colVEGCLASS, vegclass);
            if (vegclass<1000000) continue;
            pLayer->GetData(idu, m_colPVT, stratum);
            if (stratum>=1) continue;
            int cover_typeSrc = vegclass/10000;
            pLayer->GetData(idu, m_colIDU_ID, idu_id);
            CString query_str;
            query_str.Format("PVT>=1 and NextTo(IDU_ID = %d)", idu_id);
            Query *pIdu_query = pQE->ParseQuery(query_str, -1, "VEGCLASS query");
            ASSERT(pIdu_query!=NULL); 

            bool result = false;
            for ( MapLayer::Iterator iduTgt = pLayer->Begin( ); !result && (iduTgt != pLayer->End()); iduTgt++ )
               {               
               bool ok = pIdu_query->Run( iduTgt, result );

               if (result)
                  { int vegclassTgt, cover_typeTgt, pvtTgt;
                  pLayer->GetData(iduTgt, m_colVEGCLASS, vegclassTgt);
                  cover_typeTgt = vegclassTgt/10000;
                  result = cover_typeSrc==cover_typeTgt;
                  if (result)
                     {
                     pLayer->GetData(iduTgt, m_colPVT, pvtTgt);
                     pLayer->SetData( idu, m_colPVT, pvtTgt );
                     found_count++;
                     msg.Format("WW2100AP: inferred PVT %d for VEGCLASS %d from VEGCLASS %d, found_count = %d", 
                           pvtTgt, vegclass, vegclassTgt, found_count);
                     Report::LogMsg(msg);
                     }
                  }
               } // end of inner loop thru IDUs
            if (!result) 
               {
               not_found_count++;
               msg.Format("WW2100AP: did not infer PVT for VEGCLASS %d, not_found_count = %d, found_count = %d", 
                     vegclass, not_found_count, found_count);
               Report::LogMsg(msg);
               }
            } // end of outer loop thru IDUs
         } while (found_count>0);

      } // end of if (m_useColdStart_vegclass_initialization)

   if (m_useColdStart_standage_reconciliation)
      {
      RandUniform UnitUniformDist;
      int count = 0;
      msg.Format("WW2100AP: Opening A2S file %s", (LPCTSTR)m_A2S_file);
      Report::LogMsg(msg, RT_INFO);
      m_records_A2S = m_A2S_table.ReadAscii(m_A2S_file, ',', TRUE);
      msg.Format("WW2100AP: m_records_A2S = %d", m_records_A2S);
      Report::LogMsg(msg, RT_INFO);

      int A2Scol_START_AGE, A2Scol_fmh, A2Scol_fwi, A2Scol_fdw, A2Scol_fdd, A2Scol_fsi;
      A2Scol_START_AGE = A2Scol_fmh = A2Scol_fwi = A2Scol_fdw = A2Scol_fdd = A2Scol_fsi = -1;
      if (m_records_A2S>0)
         { 
         A2Scol_START_AGE = m_A2S_table.GetCol("START_AGE");
         A2Scol_fmh = m_A2S_table.GetCol("fmh");
         A2Scol_fwi = m_A2S_table.GetCol("fwi");
         A2Scol_fdw = m_A2S_table.GetCol("fdw");
         A2Scol_fdd = m_A2S_table.GetCol("fdd");
         A2Scol_fsi = m_A2S_table.GetCol("fsi");
         }
      if (m_records_A2S <= 0 || A2Scol_START_AGE<0 || A2Scol_fmh<0 || A2Scol_fwi<0 || A2Scol_fdw<0 || A2Scol_fdd<0 || A2Scol_fsi<0
            || !(m_colMEDIAN_AGE >= 0 && m_colSTART_AGE >= 0 && m_colEND_AGE >= 0))
         {
         msg.Format("WW2100AP: Cannot reconcile median age with start age and end age because file, columns in file, or attributes are missing: "
            "m_records_A2S = %d, A2Scol_START_AGE = %d, A2Scol_fmh = %d, A2Scol_fwi = %d, A2Scol_fdw = %d, A2Scol_fdd = %d, A2Scol_fwi = %d, \n"
            "m_colMEDIAN_AGE = %d, m_colSTART_AGE = %d, m_colEND_AGE = %d",
            m_records_A2S, A2Scol_START_AGE, A2Scol_fmh, A2Scol_fwi, A2Scol_fdw, A2Scol_fdd, A2Scol_fsi, m_colMEDIAN_AGE, m_colSTART_AGE, m_colEND_AGE);
         Report::WarningMsg(msg);
         }
      else for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
         { // Deal with any inconsistencies between MEDIAN_AGE, START_AGE, and END_AGE
         int pvt, colA2S;
         CString PVT_Text;
         pLayer->GetData(idu, m_colPVT, pvt);
         switch (pvt)
            {
            case 2: /* fmh */ colA2S = A2Scol_fmh; PVT_Text = "OWC_fmh"; break;
            case 6: /* fwi */ colA2S = A2Scol_fwi; PVT_Text = "OWC_fwi"; break;
            case 1: /* fdw */ colA2S = A2Scol_fdw; PVT_Text = "OWC_fdw"; break;
            case 9: /* fdd */ colA2S = A2Scol_fdd; PVT_Text = "OWC_fdm"; break;
            case 3: /* fsi */ colA2S = A2Scol_fsi; PVT_Text = "OWC_fsi"; break;
            default: colA2S = 0; break;
            } // end of switch (pvt)
         if (colA2S < 1) continue;

         int median_age, start_age, end_age;
         median_age = start_age = end_age = -1;
         pLayer->GetData(idu, m_colMEDIAN_AGE, median_age);

         // Randomize nominal_age_age within the age range that represented by MEDIAN_age.
         double rand_num = UnitUniformDist.RandValue(0., 1.);  //random number between 0 and 1, uniform distribution
         int nominal_age = -1;
         if (median_age <= 26) nominal_age = median_age;
         else if (median_age == 45) nominal_age = (int)(26.5 + rand_num*(62.5 - 26.5)); // random integer on [26.5, 62.5]
         else if (median_age == 80) nominal_age = (int)(62.5 + rand_num*(115.5 - 62.5)); // random integer on [62.5, 115.5]
         else if (median_age == 150) nominal_age = (int)(115.5 + rand_num*(200.5 - 115.5)); // random integer on [115.5, 200.5]
         else if (median_age == 250) nominal_age = (int)(200.5 + rand_num*(300.5 - 200.5)); // random integer on [200.5, 300.5]
         else
            {
            msg.Format("WW2100AP: unexpected median_age = %d.", median_age);
            Report::ErrorMsg(msg);
            return(false);
            }
         pLayer->GetData(idu, m_colSTART_AGE, start_age);
         pLayer->GetData(idu, m_colEND_AGE, end_age);
         if (nominal_age < start_age || end_age < nominal_age)
            { // Try to change the VEGCLASS to one which is consistent with nominal_age.
            bool foundFlag = false;
            int iRec = 0;
            while (!foundFlag && iRec < (m_records_A2S - 1))
               {
               int A2S_start_age = m_A2S_table.GetAsInt(A2Scol_START_AGE, iRec);
               int A2S_next_start_age = m_A2S_table.GetAsInt(A2Scol_START_AGE, iRec + 1);
               foundFlag = A2S_start_age <= nominal_age && nominal_age < A2S_next_start_age;
               if (!foundFlag) iRec++;
               } // end of loop thru A2S table
            CString CTSSfromFile = m_A2S_table.GetAsString(colA2S, iRec); 
            if (CTSSfromFile.GetLength() <= 0)
               {
               msg.Format("WW2100AP: CTSSfromFile has zero length.");
               Report::ErrorMsg(msg);
               return(false);
               }

            // Construct a new VEGCLASS and set AGECLASS to nominal_age.
            VegState iduVS(PVT_Text, CTSSfromFile);
            if (!iduVS.Forest())
               {
               msg.Format("WW2100AP: VegState(%s, %s) failed to construct a forest VEGCLASS.  colA2s = %d, iRow = %d, "
                     "m_foundPVTflag = %d, m_foundCTflag = %d, m_foundSSflag = %d\n Stratum_ID = %d, CTSS_ID = %d, CoverType = %s, StructuralStage = %s", 
                     PVT_Text, CTSSfromFile, colA2S, iRec, iduVS.m_foundPVTflag, iduVS.m_foundCTflag, iduVS.m_foundSSflag, iduVS.Stratum(), iduVS.CTSS(),
                     iduVS.CoverType(), iduVS.StructuralStage());
               Report::WarningMsg(msg);
               }
            else
               {
               int ctss = iduVS.CTSS();
               pLayer->SetData(idu, m_colVEGCLASS, ctss);
               pLayer->SetData(idu, m_colCT_Text, iduVS.CoverType());
               pLayer->SetData(idu, m_colSS_Text, iduVS.StructuralStage());
               pLayer->SetData(idu, m_colAGECLASS, nominal_age);
               count++;
               } // end of if (!iduVS.Forest()) ... else ...
            } // end of trying to change the VEGCLASS for this IDU
         } // end of else for loop on idus for reconciling median age
      msg.Format("WW2100AP: Changed VEGCLASS for %d IDUs to make AGECLASS consistent with MEDIAN_AGE.", count);
      Report::LogMsg(msg);
      } // end of if (m_useColdStart_standage_reconciliation)
        
   RandUniform randomDraw;

   if (m_initial_irrigation_fraction>=0.) 
         for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
      {
         int lulc_a, wrexists;
         pLayer->GetData( idu, m_colLulcA,  lulc_a);
         if (lulc_a!=LULCA_AGRICULTURE) continue;
         pLayer->GetData( idu, m_colWREXISTS,  wrexists);
         if (!IsIrrigable(wrexists)) continue;
         double draw = randomDraw.RandValue(0., 1.);
         int irrigation = draw<=m_initial_irrigation_fraction ? 1 : 0;
         pLayer->SetData( idu, m_colIRRIGATION,  irrigation );
      } // end of loop thru IDUs

   if ( m_useColdStart_SWHCfile )
      { // Read the netCDF file and populate three SWHC attributes.
      msg.Format( "WW2100AP: Opening SWHC file %s", (LPCTSTR) m_SWHCfile );
      Report::LogMsg( msg, RT_INFO );
      m_pDataObj_SWHCtop = new GeoSpatialDataObj();
      m_pDataObj_SWHCtop->InitLibraries();
      m_pDataObj_SWHCtop->Open( (LPCTSTR) m_SWHCfile, _T("SWHC_TOP") );
      m_pDataObj_SWHCtop->ReadSpatialData();
      m_pDataObj_SWHCmid = new GeoSpatialDataObj();
      m_pDataObj_SWHCmid->InitLibraries();
      m_pDataObj_SWHCmid->Open( (LPCTSTR) m_SWHCfile, _T("SWHC_MID") );
      m_pDataObj_SWHCmid->ReadSpatialData();
      m_pDataObj_SWHCdeep = new GeoSpatialDataObj();
      m_pDataObj_SWHCdeep->InitLibraries();
      m_pDataObj_SWHCdeep->Open( (LPCTSTR) m_SWHCfile, _T("SWHC_DEEP") );
      m_pDataObj_SWHCdeep->ReadSpatialData();
      m_pDataObj_SoilDepth = new GeoSpatialDataObj();
      m_pDataObj_SoilDepth->InitLibraries();
      m_pDataObj_SoilDepth->Open( (LPCTSTR) m_SWHCfile, _T("SOIL_DEPTH") );
      m_pDataObj_SoilDepth->ReadSpatialData();

      if (!CheckCol( pLayer, m_colGrdCellNdx,  "GrdCellNdx",  TYPE_INT,   CC_MUST_EXIST )) InitializeGrdCellNdx();
      // loop through the IDUs
      for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
         {
         int gridCellIndex; // int &r_gridCellIndex = gridCellIndex;
         pLayer->GetData( idu, m_colGrdCellNdx,  gridCellIndex );
         float swhc = m_pDataObj_SWHCtop->Get( gridCellIndex, 0); // r_gridCellIndex, yearNdx); 
         if (swhc>1.e36) swhc = 0.; // NetCDF fill value is 9.96921e+36
         pLayer->SetData( idu, m_colSWHC_TOP,  swhc );
         swhc = m_pDataObj_SWHCmid->Get( gridCellIndex, 0); // r_gridCellIndex, yearNdx); 
         if (swhc>1.e36) swhc = 0.; // NetCDF fill value is 9.96921e+36
         pLayer->SetData( idu, m_colSWHC_MID,  swhc );
         swhc = m_pDataObj_SWHCdeep->Get( gridCellIndex, 0); // r_gridCellIndex, yearNdx); 
         if (swhc>1.e36) swhc = 0.; // NetCDF fill value is 9.96921e+36
         pLayer->SetData( idu, m_colSWHC_DEEP,  swhc );
         float soil_depth = m_pDataObj_SoilDepth->Get( gridCellIndex, 0); // r_gridCellIndex, yearNdx); 
         if (soil_depth>1.e36) soil_depth = 0.; // NetCDF fill value is 9.96921e+36
         pLayer->SetData( idu, m_colSOIL_DEPTH,  soil_depth );
         } // end IDU loop
      } // end of if (m_useColdStart_SWHCfile )

   if ( m_useColdStart_PopUnitsfile ) 
      {
      m_records_PU = m_PopUnits_table.ReadAscii(m_PopUnits_file, ',', TRUE);
      if (m_records_PU<=0)
         {
         msg.Format("WW2100AP_ColdStart: missing or empty Population Units file\n"
            "records_PU = %d\n", m_records_PU);
         Report::ErrorMsg( msg );
         return false;
         }

      bool err = false;
      m_colPU_IDU_ID = m_PopUnits_table.GetCol("IDU_ID"); err |= m_colPU_IDU_ID<0; // ASSERT(!err);
      m_colPU_UGB_name = m_PopUnits_table.GetCol("UGB_name"); err |= m_colPU_UGB_name<0; // ASSERT(!err);
      m_colPU_UGB = m_PopUnits_table.GetCol("UGB"); err |= m_colPU_UGB<0; // ASSERT(!err);
      m_colPU_UGB_DH_ID = m_PopUnits_table.GetCol("UGB_DH_ID"); err |= m_colPU_UGB_DH_ID<0; // ASSERT(!err);
      m_colPU_RR = m_PopUnits_table.GetCol("RR"); err |= m_colPU_RR<0; // ASSERT(!err);
      m_colPU_RR_DH_ID = m_PopUnits_table.GetCol("RR_DH_ID"); err |= m_colPU_RR_DH_ID<0; // ASSERT(!err);
      if (err)
         {
         msg.Format("WW2100AP:ColdStart - One or more missing columns in the PopulationUnits file.\n"
               "m_colPU_IDU_ID, _UGB_name, _UGB_DH_ID, _RR_DH_ID = %d, %d, %d, %d", 
               m_colPU_IDU_ID, m_colPU_UGB_name, m_colPU_UGB_DH_ID, m_colPU_RR_DH_ID);
         Report::ErrorMsg(msg);
         return false;
         }

      // Loop thru the IDUs and the records in the PopulationUnits file
      int iRec = 0;
      int colIDU_ID;  pLayer->CheckCol(colIDU_ID, "IDU_ID", TYPE_INT, CC_MUST_EXIST);
      for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
         {

         if ((iRec/25000)*25000==iRec)
            {
            msg.Format( "WW2100AP_ColdStart: PopUnits file iRec = %d", iRec );
            Report::LogMsg( msg, RT_INFO );
            }

         int idu_idFromFile = m_PopUnits_table.GetAsInt(m_colPU_IDU_ID, iRec);
         int idu_idFromIDUlayer; pLayer->GetData(idu, colIDU_ID, idu_idFromIDUlayer);
         ASSERT(idu_idFromFile==idu_idFromIDUlayer);

         int rpaFlag = m_PopUnits_table.GetAsInt( m_colPU_RR, iRec );
         int rpaID = m_PopUnits_table.GetAsInt( m_colPU_RR_DH_ID, iRec );
         int ugbFlag = m_PopUnits_table.GetAsInt( m_colPU_UGB, iRec );
         int ugbID = m_PopUnits_table.GetAsInt( m_colPU_UGB_DH_ID, iRec );
         ASSERT(rpaFlag<=0 || (rpaID>0 && rpaID<MAX_RR_DH_ID));
         ASSERT(ugbFlag<=0 || (ugbID>0 && ugbID<(MAX_RR_DH_ID+MAX_UGB)));
         ASSERT(rpaFlag<=0 || ugbFlag<=0);

         if (rpaFlag) 
            {
            int ugb;
            pLayer->GetData( idu, m_colUGB, ugb);
            if (ugb==0) pLayer->SetData(idu, m_colRPA, rpaID);
            else
               {
               msg.Format( "WW2100AP_ColdStart: an RPA IDU has a non-zero UGB in IDU.dbf; "
                     "it will be removed from the RPA. iRec, rpaID, ugb = %d, %d, %d", iRec, rpaID, ugb );
               Report::LogMsg( msg, RT_INFO );
               pLayer->SetData(idu, m_colRPA, 0);
               }
            }
         else if (ugbFlag)
            {
            pLayer->SetData(idu, m_colRPA, 0);
            CString ugbName = m_PopUnits_table.GetAsString(m_colPU_UGB_name, iRec);
            CString ugaNameFromDatabase;
            pLayer->GetData( idu, m_colUGB_NAME, ugaNameFromDatabase);
            if (ugbName!=ugaNameFromDatabase)
               {
               msg.Format( "WW2100AP_ColdStart: PopUnits ugbName is not the same as ugaNameFromDatabase."
                     "iRec, ugbName, ugaNameFromDatabase = %d, %s, %s (ugaNameFromDatabase may be blank).", 
                     iRec, ugbName.GetString(), ugaNameFromDatabase.GetString() );
               Report::LogMsg( msg, RT_INFO );
               }
            else
               {
               int ugbFromID = UGBlookup(ugbID);
               int ugbFromDatabase;
               pLayer->GetData( idu, m_colUGB, ugbFromDatabase);
               if (ugbFromID!=ugbFromDatabase)
                  {
                  msg.Format( "WW2100AP_ColdStart: PopUnits ugbFromID is not the same as ugaFromDatabase."
                        "iRec, ugbID, ugbFromID, ugbFromDatabase = %d, %d, %d, %d", 
                        iRec, ugbID, ugbFromID, ugbFromDatabase );
                  Report::LogMsg( msg, RT_INFO );
                  }
               }
            }
         else pLayer->SetData(idu, m_colRPA, 0);

         iRec++;
         } // end IDU loop

      } // end of if ( m_useColdStart_PopUnitsfile ) 

   if ( m_useColdStart_ForestSTMfiles ) 
      {
      // Construct the two .csv files used by DynamicVeg
      // First, open the 7 input .csv files.
      m_records_CT = m_CoverTypes_table.ReadAscii(m_CoverTypes_file, ',', TRUE);
      m_records_SS = m_StructuralStages_table.ReadAscii(m_StructuralStages_file, ',', TRUE);
      int records_TT = m_TransitionTypes_table.ReadAscii(m_TransitionTypes_file, ',', TRUE);
      int records_Stratum2PVT = m_Stratum2PVT_table.ReadAscii(m_Stratum2PVT_file, ',', TRUE);
      m_records_LAIandC = m_LAIandC_table.ReadAscii(m_LAIandC_file, ',', TRUE);
      int records_DT = m_DeterministicTransitionInput_table.ReadAscii(m_DeterministicTransitionInput_file, ',', TRUE);
      int records_PT = m_ProbabilisticTransitionInput_table.ReadAscii(m_ProbabilisticTransitionInput_file, ',', TRUE);
      // int records_TPM = m_TransitionProbabilityMultiplierInput_table.ReadAscii(m_TransitionProbabilityMultiplierInput_file, ',', TRUE);

      if (m_records_CT<=0 || m_records_SS<=0 || records_TT<=0 || records_Stratum2PVT<=0 || records_DT<=0 || records_PT<=0) // || records_TPM<=0)
         {
         msg.Format("WW2100AP_ColdStart: missing or empty file\n"
            "records_CT, SS, TT, Stratum2PVT, DT, PT = %d, %d, %d, %d, %d, %d\n",
            m_records_CT, m_records_SS, records_TT, records_Stratum2PVT, records_DT, records_PT); // , records_TPM);
         Report::ErrorMsg( msg );
         return false;
         }

      // Second, get the column numbers.
      bool err = false;
      m_colCT_Abbrev = m_CoverTypes_table.GetCol("Abbrev"); err |= m_colCT_Abbrev<0; // ASSERT(!err);
      m_colCT_CTID = m_CoverTypes_table.GetCol("CTID"); err |= m_colCT_CTID<0; // ASSERT(!err);
      m_colSS_Abbrev = m_StructuralStages_table.GetCol("Abbrev"); err |= m_colSS_Abbrev<0; // ASSERT(!err);
      m_colSS_SSID = m_StructuralStages_table.GetCol("SSID"); err |= m_colSS_SSID<0; // ASSERT(!err);
      int colTT_TTID = m_TransitionTypes_table.GetCol("Transition Type ID"); err |= colTT_TTID<0; // ASSERT(!err);
      int colTT_Abbrev = m_TransitionTypes_table.GetCol("Abbreviation"); err |= colTT_Abbrev<0; // ASSERT(!err);
      int colTT_DISTURB = m_TransitionTypes_table.GetCol("DISTURB"); err |=colTT_DISTURB <0; // ASSERT(!err);
      int colStratum2PVT_SID = m_Stratum2PVT_table.GetCol("Stratum ID"); err |= colStratum2PVT_SID<0; // ASSERT(!err);
      int colStratum2PVT_Abbrev = m_Stratum2PVT_table.GetCol("Abbreviation"); err |= colStratum2PVT_Abbrev<0; // ASSERT(!err);
      int colStratum2PVT_PVTname = m_Stratum2PVT_table.GetCol("PVTname"); err |= colStratum2PVT_PVTname<0; // ASSERT(!err);
      int colStratum2PVT_PVTnumber = m_Stratum2PVT_table.GetCol("PVTnumber"); err |= colStratum2PVT_PVTnumber<0; // ASSERT(!err);
      m_colLAIandC_Abbrev = m_LAIandC_table.GetCol("VegclassAbbrev"); err |= m_colLAIandC_Abbrev<0; // ASSERT(!err);
      m_colLAIandC_LAI = m_LAIandC_table.GetCol("LAI"); err |= m_colLAIandC_LAI<0; // ASSERT(!err);
      m_colLAIandC_CARBON = m_LAIandC_table.GetCol("CARBON"); err |= m_colLAIandC_CARBON<0; // ASSERT(!err);
      int colDT_FromStratum = m_DeterministicTransitionInput_table.GetCol("From Stratum"); err |= colDT_FromStratum<0; // ASSERT(!err);
      int colDT_FromClass = m_DeterministicTransitionInput_table.GetCol("From Class"); err |= colDT_FromClass<0; // ASSERT(!err);
      int colDT_ToStratum = m_DeterministicTransitionInput_table.GetCol("To Stratum"); err |= colDT_ToStratum<0; // ASSERT(!err);
      int colDT_ToClass = m_DeterministicTransitionInput_table.GetCol("To Class"); err |= colDT_ToClass<0; // ASSERT(!err);
      int colDT_AgeMin = m_DeterministicTransitionInput_table.GetCol("Age Min"); err |= colDT_AgeMin<0; // ASSERT(!err);
      int colDT_AgeMax = m_DeterministicTransitionInput_table.GetCol("Age Max"); err |= colDT_AgeMax<0; // ASSERT(!err);
      int colPT_FromStratum = m_ProbabilisticTransitionInput_table.GetCol("From Stratum"); err |= colPT_FromStratum<0; // ASSERT(!err);
      int colPT_FromClass = m_ProbabilisticTransitionInput_table.GetCol("From Class"); err |= colPT_FromClass<0; // ASSERT(!err);
      int colPT_ToStratum = m_ProbabilisticTransitionInput_table.GetCol("To Stratum"); err |= colPT_ToStratum<0; // ASSERT(!err);
      int colPT_ToClass = m_ProbabilisticTransitionInput_table.GetCol("To Class"); err |= colPT_ToClass<0; // ASSERT(!err);
      int colPT_TransitionType = m_ProbabilisticTransitionInput_table.GetCol("Transition Type"); err |= colPT_TransitionType<0; // ASSERT(!err); // e.g. "WFSR"
      int colPT_Prob = m_ProbabilisticTransitionInput_table.GetCol("Prob"); err |= colPT_Prob<0; // ASSERT(!err);
      int colPT_Propn = m_ProbabilisticTransitionInput_table.GetCol("Propn"); err |= colPT_Propn<0; // ASSERT(!err);
      int colPT_AgeMin = m_ProbabilisticTransitionInput_table.GetCol("Age Min"); err |= colPT_AgeMin<0; // ASSERT(!err);
      int colPT_AgeMax = m_ProbabilisticTransitionInput_table.GetCol("Age Max"); err |= colPT_AgeMax<0; // ASSERT(!err);
      int colPT_AgeShift = m_ProbabilisticTransitionInput_table.GetCol("Age Shift"); err |= colPT_AgeShift<0; // ASSERT(!err);
      int colPT_AgeReset = m_ProbabilisticTransitionInput_table.GetCol("Age Reset"); err |= colPT_AgeReset<0; // ASSERT(!err);
      int colPT_TSTMin = m_ProbabilisticTransitionInput_table.GetCol("TST Min"); err |= colPT_TSTMin<0; // ASSERT(!err);
      int colPT_TSTMax = m_ProbabilisticTransitionInput_table.GetCol("TST Max"); err |= colPT_TSTMax<0; // ASSERT(!err);
      int colPT_TSTShift = m_ProbabilisticTransitionInput_table.GetCol("TST Shift"); err |= colPT_TSTShift<0; // ASSERT(!err);
      // int colTPM_Stratum = m_TransitionProbabilityMultiplierInput_table.GetCol("Stratum"); err |= colTPM_Stratum<0; // ASSERT(!err);
      // int colTPM_Timestep = m_TransitionProbabilityMultiplierInput_table.GetCol("Timestep"); err |= colTPM_Timestep<0; // ASSERT(!err);
      // int colTPM_MultiplierType = m_TransitionProbabilityMultiplierInput_table.GetCol("Multiplier Type"); err |= colTPM_MultiplierType<0; // ASSERT(!err);
      // int colTPM_TransitionType = m_TransitionProbabilityMultiplierInput_table.GetCol("Transition Type"); err |= colTPM_TransitionType<0; // ASSERT(!err); // e.g. "fdd2fsi"
      // int colTPM_Mean = m_TransitionProbabilityMultiplierInput_table.GetCol("Mean"); err |= colTPM_Mean<0; // ASSERT(!err);
      if (err)
         {
         msg.Format("WW2100AP:ColdStart - One or more missing columns in the input data files.\n");
         Report::ErrorMsg(msg);
         return false;
         }

      // Build the deterministic transition lookup table.
      msg.Format("WW2100AP:ColdStart - Building deterministic transition lookup table now...\n"); 
      Report::InfoMsg(msg);

      FILE *oFile = NULL;  
	  PCTSTR deterministicFilename = (PCTSTR)m_DeterministicTransitionOutput_file;
	  int errNo = fopen_s(&oFile, deterministicFilename, "w");
      if ( errNo != 0 )
         {
         CString msg( " WW2100AP:ColdStart -  ERROR: Could not open output file " );
         msg += m_DeterministicTransitionOutput_file;
         Report::ErrorMsg( msg );
         return false;
         }
      fprintf(oFile, "REGION,PVT,PVTto,VEGCLASSfrom,VEGCLASSto,ABBREVfrom,ABBREVto,STARTAGE,ENDAGE,RNDAGE,LAI,CARBON\n");
      int * PstatesInSTM = (int *)malloc(records_DT*4*sizeof(int)); ASSERT(PstatesInSTM!=NULL); // for each record: pvt, vegclass, 
            // RegHarTransitionMissingFlag, WFSRtransitionMissingFlag
      for (int iRec=0; iRec<records_DT; iRec++)
         {
            CString stratumFrom, vegclassFromAbbrev, stratumTo, vegclassToAbbrev, pvtFromName;
            int pvtFrom, pvtTo, vegclassFromNum, vegclassToNum, startAge, endAge, rndAge;
            float lai, carbon;

            stratumFrom = m_DeterministicTransitionInput_table.GetAsString(colDT_FromStratum,  iRec); ASSERT(stratumFrom.GetLength()>0);
            pvtFrom = lookup(&m_Stratum2PVT_table, records_Stratum2PVT, colStratum2PVT_Abbrev, stratumFrom, colStratum2PVT_PVTnumber);
            stratumTo = m_DeterministicTransitionInput_table.GetAsString(colDT_ToStratum,  iRec); 
            if(stratumTo.GetLength()<=0) stratumTo = stratumFrom;
            pvtTo = lookup(&m_Stratum2PVT_table, records_Stratum2PVT, colStratum2PVT_Abbrev
               , stratumTo, colStratum2PVT_PVTnumber);
            vegclassFromAbbrev = m_DeterministicTransitionInput_table.GetAsString(colDT_FromClass,  iRec); ASSERT(vegclassFromAbbrev.GetLength()>0);
            vegclassFromNum = VegclassNum(vegclassFromAbbrev);
            vegclassToAbbrev = m_DeterministicTransitionInput_table.GetAsString(colDT_ToClass,  iRec); ASSERT(vegclassToAbbrev.GetLength()>0);
            vegclassToNum = VegclassNum(vegclassToAbbrev);
            m_DeterministicTransitionInput_table.Get(colDT_AgeMin,  iRec, startAge);
            if (startAge<0) startAge = 0;
            m_DeterministicTransitionInput_table.Get(colDT_AgeMax,  iRec, endAge);
            if (endAge<startAge) endAge = startAge;
            rndAge = startAge + iRec%(endAge - startAge + 1);

            lookup(&m_Stratum2PVT_table, records_Stratum2PVT, colStratum2PVT_Abbrev, stratumFrom, colStratum2PVT_PVTname, &pvtFromName); ASSERT(pvtFromName.GetLength()>0);
            lai = LeafAreaIndex(vegclassFromAbbrev);
            carbon = CarbonDensity(vegclassFromAbbrev);

            fprintf(oFile, "1,%d,%d,%d,%d,%s,%s,%d,%d,%d,%f,%f\n", pvtFrom, pvtTo, vegclassFromNum, vegclassToNum, (LPCTSTR) vegclassFromAbbrev, (LPCTSTR) vegclassToAbbrev,
                  startAge, endAge, rndAge, lai, carbon); 
            CString msg;      
            msg.Format("WW2100AP: 1,%d,%d,%d,%d,%s,%s,%d,%d,%d,%f,%f\n", pvtFrom, pvtTo, vegclassFromNum, vegclassToNum, (LPCTSTR) vegclassFromAbbrev, (LPCTSTR) vegclassToAbbrev,
                  startAge, endAge, rndAge, lai, carbon);      
            Report::LogMsg(msg); 
            *(PstatesInSTM + 4*iRec) = pvtFrom;
            *(PstatesInSTM + 4*iRec + 1) = vegclassFromNum;  
            *(PstatesInSTM + 4*iRec + 2) = endAge>=40 ? 1 : 0; // RegHarTransitionMissingFlag = true for states which can be >= 40 yrs old      
            *(PstatesInSTM + 4*iRec + 3) = 1; // WFSRtransitionMissingFlag = true      
         }
      fclose(oFile);

      // Identify any vegclass value for which there is no corresponding state in the STM.
      msg.Format("WW2100AP: *** Checking for CT:SS values which are not represented in the STM now");
      Report::LogMsg(msg); 
      ASSERT(m_colVEGCLASS>0); 
      ASSERT(m_colPVT>0);
      ASSERT(m_colIDU_ID>0);
      int IDUs_with_missing_states = 0;
      for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
         { int vegclass = 0;
         pLayer->GetData(idu, m_colVEGCLASS, vegclass);
         if (vegclass<1000000) continue;
         int pvt = 0;
         pLayer->GetData(idu, m_colPVT, pvt);
         bool found = false;
         for (int iRec=0; iRec<records_DT; iRec++)
            {
            found = pvt==*(PstatesInSTM + 4*iRec) && vegclass==*(PstatesInSTM + 4*iRec + 1);
            if (found) break;
            }
         if (!found)
            {
            int idu_id = -1;
            pLayer->GetData(idu, m_colIDU_ID, idu_id);
            msg.Format("WW2100AP: missing state in STM: idu_id, pvt, vegclass = %d, %d, %d", idu_id, pvt, vegclass);
            Report::LogMsg(msg);
            IDUs_with_missing_states++;
            }
         } // end of loop thru IDUs

      if (IDUs_with_missing_states>0)
         {
         msg.Format("WW2100AP: IDUs_with_missing_states = %d", IDUs_with_missing_states);
         Report::LogMsg(msg);
         }

      // Build the probabilistic transition lookup table.
      msg.Format("WW2100AP:ColdStart - Building probabilistic transition lookup table now...\n"); 
      Report::InfoMsg(msg);

	  PCTSTR probabilisticFilename = (PCTSTR)m_ProbabilisticTransitionOutput_file;
	  errNo = fopen_s(&oFile, probabilisticFilename, "w");
      if ( errNo != 0 )
         {
         CString msg( " WW2100AP:ColdStart -  ERROR: Could not open output file " );
         msg += m_ProbabilisticTransitionOutput_file;
         Report::ErrorMsg( msg );
         return false;
         }
      fprintf(oFile, "VEGCLASSfrom,VEGCLASSto,ABBREVfrom,ABBREVto,FUTSI,PVT,PVTto,REGEN,p,pxPropor,REGION,DISTURB,VDDTProbType,"
            "MINAGE,MAXAGE,TSD,RELATIVEAGE,KEEPRELAGE,PROPORTION,TSDMAX,RELTSD,RNDAGE\n");
      for (int iRec=0; iRec<records_PT; iRec++)
         {
         CString stratumFrom, vegclassFromAbbrev, stratumTo, vegclassToAbbrev, pvtFromName, transitionTypeAbbrev, keepRelativeAgeString;
         int pvtFrom, pvtTo, vegclassFromNum, vegclassToNum, startAge, endAge, rndAge, disturbNum, transitionTypeNum, tsd,
               relativeAge, keepRelativeAgeNum, tsdMax, relTsd;
         float probability, proportion;

         stratumFrom = m_ProbabilisticTransitionInput_table.GetAsString(colPT_FromStratum,  iRec); ASSERT(stratumFrom.GetLength()>0);
         pvtFrom = lookup(&m_Stratum2PVT_table, records_Stratum2PVT, colStratum2PVT_Abbrev, stratumFrom, colStratum2PVT_PVTnumber);
         stratumTo = m_ProbabilisticTransitionInput_table.GetAsString(colPT_ToStratum,  iRec); 
         if(stratumTo.GetLength()<=0) stratumTo = stratumFrom;
         pvtTo = lookup(&m_Stratum2PVT_table, records_Stratum2PVT, colStratum2PVT_Abbrev
            , stratumTo, colStratum2PVT_PVTnumber);
         vegclassFromAbbrev = m_ProbabilisticTransitionInput_table.GetAsString(colPT_FromClass,  iRec); ASSERT(vegclassFromAbbrev.GetLength()>0);
         vegclassFromNum = VegclassNum(vegclassFromAbbrev);
         vegclassToAbbrev = m_ProbabilisticTransitionInput_table.GetAsString(colPT_ToClass,  iRec); ASSERT(vegclassToAbbrev.GetLength()>0);
         vegclassToNum = VegclassNum(vegclassToAbbrev);
         probability = m_ProbabilisticTransitionInput_table.GetAsFloat(colPT_Prob, iRec);
         proportion = m_ProbabilisticTransitionInput_table.GetAsFloat(colPT_Propn, iRec);
         transitionTypeAbbrev = m_ProbabilisticTransitionInput_table.GetAsString(colPT_TransitionType, iRec); ASSERT(transitionTypeAbbrev.GetLength()>0);
         disturbNum = lookup(&m_TransitionTypes_table, records_TT, colTT_Abbrev, transitionTypeAbbrev, colTT_DISTURB);
         transitionTypeNum = lookup(&m_TransitionTypes_table, records_TT, colTT_Abbrev, transitionTypeAbbrev, colTT_TTID);
         m_ProbabilisticTransitionInput_table.Get(colPT_AgeMin,  iRec, startAge);
         if (startAge<0) startAge = 0;
         m_ProbabilisticTransitionInput_table.Get(colPT_AgeMax,  iRec, endAge);
         if (endAge<startAge) endAge = startAge;
         tsd = m_ProbabilisticTransitionInput_table.GetAsInt(colPT_TSTMin,  iRec);
         relativeAge = m_ProbabilisticTransitionInput_table.GetAsInt(colPT_AgeShift,  iRec);
         keepRelativeAgeString = m_ProbabilisticTransitionInput_table.GetAsString(colPT_AgeReset,  iRec);
         if (keepRelativeAgeString==_T("Yes")) keepRelativeAgeNum = -1;
         else { ASSERT(keepRelativeAgeString==_T("No")); keepRelativeAgeNum = 0; }
         tsdMax = m_ProbabilisticTransitionInput_table.GetAsInt(colPT_TSTMax,  iRec);
         relTsd = m_ProbabilisticTransitionInput_table.GetAsInt(colPT_TSTShift,  iRec);
         rndAge = startAge + iRec%(endAge - startAge + 1);

         fprintf(oFile, "%d,%d,%s,%s,0,%d,%d,0,%f,%f,1,%d,%d,%d,%d,%d,%d,%d,%f,%d,%d,%d\n", 
            vegclassFromNum, vegclassToNum, (LPCTSTR) vegclassFromAbbrev, (LPCTSTR) vegclassToAbbrev, pvtFrom, pvtTo, probability, probability*proportion,
            disturbNum, transitionTypeNum, startAge, endAge, tsd, relativeAge, keepRelativeAgeNum, proportion, tsdMax, relTsd, rndAge);

         if (disturbNum==1 || disturbNum==23)
            { 
            bool foundFlag = FALSE;
            int iRec_found = -1;
            for (int iRec_DT=0; iRec_found<0 && iRec_DT<records_DT; iRec_DT++) 
               {
               if (pvtFrom==(*(PstatesInSTM + iRec_DT*4)) && vegclassFromNum==(*(PstatesInSTM + iRec_DT*4 + 1)))
                     iRec_found = iRec_DT;
               }
               
            ASSERT(iRec_found>=0);
            if (disturbNum==1) *(PstatesInSTM + iRec_found*4 + 2) = 0; // RegHarTransitionMissingFlag = FALSE
            if (disturbNum==23) *(PstatesInSTM + iRec_found*4 + 3) = 0; // WFSRtransitionMissingFlag = FALSE
            }
               
         } // end of loop thru the probabilistic transition list
      fclose(oFile);

      // Identify any states for which the WFSR or RegHar transitions are missing
      for (int iRec_DT=0; iRec_DT<records_DT; iRec_DT++) 
            if (*(PstatesInSTM + iRec_DT*4 + 2)!=0 || *(PstatesInSTM + iRec_DT*4 + 3)!=0)
         {
         CString msg;
         msg.Format("WW2100AP: pvt %d state %d is missing its RegHar transition", *(PstatesInSTM + iRec_DT*4), *(PstatesInSTM + iRec_DT*4 + 1));
         
         if (*(PstatesInSTM + iRec_DT*4 + 2)!=0) 
            Report::LogMsg(msg);

         msg.Format("WW2100AP: pvt %d state %d is missing its WFSR transition", *(PstatesInSTM + iRec_DT*4), *(PstatesInSTM + iRec_DT*4 + 1));
         if (*(PstatesInSTM + iRec_DT*4 + 3)!=0) Report::LogMsg(msg);
         } // end of if (*(PstatesInSTM + iRec_DT*4 + 2)!=0 || *(PstatesInSTM + iRec_DT*4 + 3)!=0)
      free(PstatesInSTM);

      // Read the DGVM netCDF file to initialize the GrdCellNdx and DGVMvegtype IDU attributes
      CString pathAndFileName = ReplaceSubstring(m_DGVMvegtypefile.DGVMvegtype_filename, "CLIMATE_SCENARIO_NAME", m_climateScenarioName);
      pathAndFileName = PathManager::MakeAbsolutePath(pathAndFileName, PM_PROJECT_DIR);
      msg.Format( "WW2100AP_ColdStart: Opening DGVM vegtype file %s", (LPCTSTR) pathAndFileName );
      Report::LogMsg( msg, RT_INFO );
      CString varName(_T("VTYPE"));
      m_pDataObj_DGVMvegtype = new GeoSpatialDataObj();
      m_pDataObj_DGVMvegtype->InitLibraries();
      m_pDataObj_DGVMvegtype->Open( (LPCTSTR) pathAndFileName, (LPCTSTR) varName );
      m_pDataObj_DGVMvegtype->ReadSpatialData();

      // Sort out which gridcell encloses the centroid of each IDU and
      // loop through the IDUs to initialize the data in the DGVMvegtype column.
      int idu_count = 1;
      for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
         {

         if ((idu_count/25000)*25000==idu_count)
            {
            msg.Format( "WW2100AP_ColdStart: DGVMvegtype idu_count = %d", idu_count );
            Report::LogMsg( msg, RT_INFO );
            }
         idu_count++;

         Poly *pPoly = pLayer->GetPolygon(idu);
         Vertex centroid = pPoly->GetCentroid();
         if ((centroid.x!=centroid.x || centroid.y!=centroid.y) || (centroid.x==0 && centroid.y==0)) // detect NaNs
            {
            msg.Format( "WW2100AP_ColdStart: bad centroid at idu_count = %d", idu_count );
            Report::LogMsg( msg, RT_INFO );
            continue;
            }

         int gridCellIndex; int &r_gridCellIndex = gridCellIndex;
         bool OKflag = true;
         int vtype = (int)m_pDataObj_DGVMvegtype->Get( centroid.x, centroid.y, r_gridCellIndex, 0, m_projectionWKT, false, &OKflag  ); 
         if (!OKflag)
            { CString msg;
            msg.Format("*** WW2100AP::InitColdStart() m_pDataObj_DGVMvegtype->Get() failed. centroid = (%f,%f)", centroid.x, centroid.y);
            Report::LogMsg(msg);
            return(false);
            }
			pLayer->SetData( idu, m_colGrdCellNdx,  gridCellIndex );
         pLayer->SetData( idu, m_colDGVMvegtyp,  vtype );
         } // end IDU loop
	   msg.Format("WW2100AP:ColdStart - initialized grid cell index\n"); 
      Report::InfoMsg(msg);
      } // end of if ( m_useColdStart_ForestSTMfiles ) 

   return true;
   }  // end of WW2100AP_ColdStart()



float WW2100AP::CarbonDensity(CString vegclassAbbrev)
   {
   int iRec = find(&m_LAIandC_table, m_records_LAIandC, m_colLAIandC_Abbrev, vegclassAbbrev);
   float carbonDensity = 0.f;
   if (iRec>=0) m_LAIandC_table.GetAsFloat(m_colLAIandC_CARBON, iRec);
   return(carbonDensity);
   }

   
int WW2100AP::VegclassNum(CString vegclassAbbrev)
   { int colon_pos;
      colon_pos = vegclassAbbrev.Find(':');
      CString coverTypeAbbrev = vegclassAbbrev.Left(colon_pos);
      int iRec = find(&m_CoverTypes_table, m_records_CT, m_colCT_Abbrev, coverTypeAbbrev);
      int coverTypeNum = m_CoverTypes_table.GetAsInt(m_colCT_CTID, iRec);
      int len = vegclassAbbrev.GetLength();
      CString structuralStageAbbrev = vegclassAbbrev.Right(len - colon_pos - 1);
      iRec = find(&m_StructuralStages_table, m_records_SS, m_colSS_Abbrev, structuralStageAbbrev);
      int structuralStageNum = m_StructuralStages_table.GetAsInt(m_colSS_SSID, iRec);
      int vegclassNum = 10000*coverTypeNum + 10*structuralStageNum;
      return(vegclassNum);
   } // end of VegclassNum()


int WW2100AP::lookup(VDataObj * tableP, int nRec, int colIn, CString inString, int colOut)
   { int outVal, iRec;
   iRec = find(tableP, nRec, colIn, inString);
   ASSERT(iRec>=0 && iRec<nRec);
   outVal = tableP->GetAsInt (colOut, iRec);
   return(outVal);
   } // end of int WW2100AP::lookup(table, colIn, inString, colOut)


void WW2100AP::lookup(VDataObj * tableP, int nRec, int colIn, CString inString, int colOut, CString * outStringP)
   { int iRec;
   iRec = find(tableP, nRec, colIn, inString);
   ASSERT(iRec>=0 && iRec<nRec);
   *outStringP = tableP->GetAsString(colOut, iRec);
   } // end of void WW2100AP::lookup(table, colIn, inString, colOut, outStringP)


int WW2100AP::find(VDataObj * tableP, int nRec, int col, CString tgt)
   {
   for (int iRec=0; iRec<nRec; iRec++)
      {
      CString possibleMatch;
      possibleMatch = tableP->GetAsString(col, iRec);
      if (possibleMatch==tgt) return(iRec);
      }
   // ASSERT(0);
   return(-1);
   } // end of WW2100AP::find(table, nRec, col, tgt)
   

bool WW2100AP::RunCarbon( EnvContext *pContext )
   {
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;
   testMessage(pContext, _T("RunCarbon"));
  
   // loop through the IDU
   for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
      {
      pLayer->GetData( idu, m_colDisturb,  m_disturb );
      pLayer->GetData( idu, m_colAGECLASS, m_ageClass );
      pLayer->GetData( idu, m_colLulcMed,    m_lulcMed );

      if ( m_disturb > 0 ) //sets Stand Origin necessary for Carbon Stock Model
         {
         AddDelta( pContext, idu, m_colDisturb, -m_disturb );

         switch( m_disturb )
            {
            case STAND_REPLACING_FIRE: 
               pLayer->SetData( idu, m_colStandOrig, m_disturb );//sets for which carbon stock curve to use
               break;

            case HARVEST:
               pLayer->SetData( idu, m_colStandOrig, m_disturb );//sets for which carbon stock curve to use
               break;
            }
        }
      
      SetCarbonStocks(pContext,idu); //set carbon stocks based on Dave Turner's BGC age dependent carbon stock data
      } // end IDU loop
   
   return TRUE;
   }


bool WW2100AP::InitCarbon( EnvContext *pContext )
   {
   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;

   pLayer->SetColData( m_colDisturb, VData( 0 ), true );

   // loop through the IDU
   for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
      {
      pLayer->GetData( idu, m_colDisturb,  m_disturb );
      pLayer->GetData( idu, m_colAGECLASS, m_ageClass );
      pLayer->GetData( idu, m_colLulcMed,    m_lulcMed );

      SetCarbonStocks(pContext,idu); //set carbon stocks based on Dave Turner's BGC age dependent carbon stock data
      } // end IDU loop
   
   return true;
   }


void WW2100AP::testMessage(EnvContext *pContext, LPCTSTR modelName)
   {
   if (m_testMode==0) return;

   CString msg;
   msg.Format("WW2100AP: currentYear, id, WW2100AP modelName = %d, %d, %s", pContext->currentYear, pContext->id, modelName );
   Report::LogMsg(msg);
   } // end of testMessage()


bool WW2100AP::SetCarbonStocks( EnvContext *pContext, int idu )
   {
/*   The Carbon Stock equations we're derived from data from David Turner.  See documentation in \envision\StudyAreas\WW2100\Carbon_Stocks.
    The POLY_FIT routine in the programming language IDL uses matrix inversion to determine the coefficients.
   
   The outputs consisted of all (8) carbon stocks (stem, leaf, coarse root, fine root, CWD, snag, litter, and soil – units are kgC/m2) 
*/   

   int   stand_orig;
   double conifer_cut;
   double conifer_fire; 
   double hardwood_cut; 
   double hardwood_fire;
   double mixed_cut;
   double mixed_fire; 

   MapLayer *pLayer = (MapLayer*) pContext->pMapLayer;

   float t = (float) m_ageClass;  // use t for easier reading in equations
   
   if (m_lulcMed >= 421 && m_lulcMed <= 427) //conifers
      {
      pLayer->GetData( idu, m_colStandOrig, stand_orig );
   
      switch( stand_orig )
         {
         case STAND_REPLACING_FIRE:
            conifer_fire = 26.8898 + -0.0415684*t + 0.00167319*pow(t,2) + 3.14600e-005*pow(t,3) + -5.54763e-007*pow(t,4) + 3.58783e-009*pow(t,5) + -1.19731e-011*pow(t,6) + 2.05240e-014*pow(t,7) + -1.42951e-017*pow(t,8);
            AddDelta( pContext, idu, m_colCarbonStock, (float)conifer_fire );
            break;

        case HARVEST:
            conifer_cut = 20.4171 + -0.598971*t + 0.0548317*pow(t,2) + -0.00180103*pow(t,3) + 3.36073e-005*pow(t,4) + -3.66820e-007*pow(t,5) + 2.31088e-009*pow(t,6) + -7.77620e-012*pow(t,7) + 1.08143e-014*pow(t,8);
            AddDelta( pContext, idu, m_colCarbonStock, (float)conifer_cut );
            break;

         default: // ignore everything else
            break;
         }  //end switch
      }  //end lulcMed
   
   if (m_lulcMed == 41) //hardwood
      {
      pLayer->GetData( idu, m_colStandOrig,    stand_orig );
   
      switch( stand_orig )
         {
         case STAND_REPLACING_FIRE:
            hardwood_fire = 27.9127 + -0.0730026*t + 0.00343994*pow(t,2) + -3.57970e-005*pow(t,3) + 1.88955e-007*pow(t,4) + -5.91881e-010*pow(t,5) + 1.17555e-012*pow(t,6) + -1.45755e-015*pow(t,7) + 8.88775e-019*pow(t,8);
            AddDelta( pContext, idu, m_colCarbonStock, (float)hardwood_fire );
            break;

         case HARVEST:
            hardwood_cut = 22.9227 + -1.00516*t + 0.0805146*pow(t,2) + -0.00261155*pow(t,3) + 4.70406e-005*pow(t,4) + -4.96630e-007*pow(t,5) + 3.04462e-009*pow(t,6) + -1.00215e-011*pow(t,7) + 1.36868e-014*pow(t,8);
            AddDelta( pContext, idu, m_colCarbonStock, (float)hardwood_cut );
            break;

         default: // ignore everything else
            break;
         } //end switch
      }//end lulcMed

   if (m_lulcMed == 43) //mixed forest
      {
      pLayer->GetData( idu, m_colStandOrig,    stand_orig );
   
      switch( stand_orig )
         {
         case STAND_REPLACING_FIRE:
            mixed_fire = 36.5431 + -0.295834*t + 0.00734912*pow(t,2) + -5.37541e-005*pow(t,3) + 1.34681e-007*pow(t,4) + 2.93952e-010*pow(t,5) + -2.53656e-012*pow(t,6) + 5.57049e-015*pow(t,7) + -4.26544e-018*pow(t,8);
            AddDelta( pContext, idu, m_colCarbonStock, (float)mixed_fire );
            break;

         case HARVEST:
            mixed_cut = 26.7337 + -1.30292*t + 0.0992068*pow(t,2) + -0.00318044*pow(t,3) + 5.73999e-005*pow(t,4) + -6.08372e-007*pow(t,5) + 3.74219e-009*pow(t,6) + -1.23491e-011*pow(t,7) + 1.68977e-014*pow(t,8);
            AddDelta( pContext, idu, m_colCarbonStock, (float)mixed_cut );
            break;

          default: // ignore everything else
             break;
          } //end switch
      } //end lulcMed
   
   return TRUE;
   }


bool WW2100AP::InitializeTestMode(MapLayer *pLayer, int lastXndx, int lastPndx)
   {
   CheckCol( pLayer, m_colBETAX, "BETAX", TYPE_FLOAT, CC_AUTOADD );
   CheckCols(pLayer, lastXndx, _T("X"), &m_colX0);
   CheckCols(pLayer, lastPndx, _T("PROB"), &m_colPROB0);

   for ( MapLayer::Iterator idu = pLayer->Begin( ); idu != pLayer->End(); idu++ )
      {
      if (lastXndx>=0) for (int Xndx=0; Xndx<=lastXndx; Xndx++) pLayer->SetData( idu, m_colX0+Xndx, 0.0f ); 
      if (lastPndx>=0) for (int Pndx=0; Pndx<=lastPndx; Pndx++) pLayer->SetData( idu, m_colPROB0+Pndx, 0.0f ); 
      pLayer->SetData( idu, m_colBETAX, 0.0f ); 
      } // end of loop on IDUs

   return true;
   } // end of initializeTestMode()


bool WW2100AP::CheckCols(MapLayer *pLayer, int lastNdx, LPCTSTR prefix, int * pCol0)
   {
   int ndx = 0;
   int colNum;
   while (ndx<=lastNdx) 
      {
      int newColNum;
      CString colName;
      colName.Format("%s%d", prefix, ndx);
      CheckCol( pLayer, newColNum, colName.GetString(), TYPE_FLOAT, CC_AUTOADD );
      if (ndx==0) *pCol0 = colNum = newColNum;
      else 
         {
         ASSERT(newColNum==colNum+1);
         colNum = newColNum;
         }
      ndx++;
      } // end of while (ndx<=lastNdx)

   return true;
   } // end of CheckCols()


float ModelTimeDependentCoefficient::Get(float time)
   {
   float rtnVal = 0;

   switch (m_method)
      {
      case TM_TIMESERIES_LINEAR:
         ASSERT( m_pTDdata != NULL );
         rtnVal =  m_pTDdata->IGet(time, 0, 1, IM_LINEAR);
         break;

      case TM_TIMESERIES_CONSTANT_RATE:
         ASSERT( m_pTDdata != NULL );
         rtnVal =  m_pTDdata->IGet((float) time, 0, 1, IM_CONSTANT_RATE);
         break;

/* start_value and rate haven't been implemented yet in LoadXml()
      case TM_RATE_LINEAR:
         Eyr = m_TDcoefficient.m_start_value * (1 + (float(m_TDcoefficient.m_rate)*(pContext->yearOfRun+1))); 
         break;

      case TM_RATE_EXP:
         Eyr = m_TDcoefficient.m_start_value * exp( float(m_TDcoefficient.m_rate)*(pContext->yearOfRun+1));
         break;
*/

      default:
         ASSERT( 0 );
      } // end of switch (m_method)

   return(rtnVal);
   } // end of ModelTimeDependentCoefficient::Get()


float TDdataset::Get(float time)
   {
   float rtnVal = 0;

   switch (m_type)
      {
      case TD_TYPE_FILE:
         ASSERT( m_pTDdata != NULL );
//         break;
      case TD_TYPE_TIMESERIES:
         switch (m_method)
            {
            case TD_METHOD_LINEAR:
               ASSERT( m_pTDdata != NULL );
               rtnVal =  m_pTDdata->IGet(time, 0, 1, IM_LINEAR);
               break;
            case TD_METHOD_CONSTANT_RATE:
               ASSERT( m_pTDdata != NULL );
               rtnVal =  m_pTDdata->IGet((float) time, 0, 1, IM_CONSTANT_RATE);
               break;
            default:
               ASSERT(0);
               break;
            } // end of switch(m_method)
         break; // end of case TD_DATA_TIMESERIES
      default:
         ASSERT(0);
         break;
      } // end of switch (m_type)

   return(rtnVal);
   } // end of TDdataset::Get()


bool TDdataset::InterpretValueStr(CString scenarioName)
   {
   switch( m_type )   
      {
      case TD_TYPE_FILE:
         {
         CString pathAndFileName = ReplaceSubstring(m_value_str, "SCENARIO_NAME", scenarioName);
         m_pTDdata = new FDataObj( 2, 0 );
         int rows = m_pTDdata->ReadAscii( pathAndFileName );
         if ( rows <= 0 )
            {
            CString msg; msg.Format("InterpretValueStr() Unable to load target file %s", pathAndFileName);
            Report::ErrorMsg( msg );
            return(false);
            }
         }
         break;
      case TD_TYPE_TIMESERIES:
         {
         m_pTDdata = new FDataObj( 2, 0 );

         TCHAR *targetValues = new TCHAR[ lstrlen( m_value_str ) + 2 ];
         lstrcpy( targetValues, m_value_str );

         TCHAR *nextToken = NULL;
         LPTSTR token = _tcstok_s( targetValues, _T(",() ;\r\n"), &nextToken );

         float pair[2];
         while ( token != NULL )
            {
            pair[0] = (float) atof( token );
            token   = _tcstok_s( NULL, _T(",() ;\r\n"), &nextToken );
            pair[1] = (float) atof( token );
            token   = _tcstok_s( NULL, _T(",() ;\r\n"), &nextToken );
   
            m_pTDdata->AppendRow( pair, 2 );
            } // end of while(token!=NULL)

         delete [] targetValues;
         } // end of block for case TD_TYPE_TIMESERIES
         break;
      default:
         ASSERT(0);
         break;
      } // end of switch(m_type)

   return(true);
   } // end of InterpretValueStr()


bool WW2100AP::InterpretXmlCoefficients(char * symbolPrefix, double coeff[], int numCoeff, PtrArray< ModelScalar > coeffsFromXml)
   {
   // Interpret the coefficients from the XML file.  This is designed to work for coefficients with subscripts from 0 thru 99.
   ASSERT(numCoeff>0 && numCoeff<=99);
   
   int numCoeffsFromXml = (int)coeffsFromXml.GetSize();
   if (numCoeffsFromXml<=0) return(true);

   for (int i = 0; i<coeffsFromXml.GetSize(); i++)
      { // Interpret coefficients, e.g. beta0, beta1, ..., beta<numCoeff-1>
      CString symbol = coeffsFromXml[i]->m_symbol;
      int nPrefix = (int)strlen(symbolPrefix);
      int nXml = symbol.GetLength();
      bool recognized = nXml>nPrefix && nXml<=nPrefix+2 && symbol.Left(nPrefix)==symbolPrefix; 
      int tens_digit = nXml==nPrefix+1 ? 0 : symbol[nPrefix] - '0';
      int ones_digit = symbol[nXml==nPrefix+1 ? nPrefix : nPrefix+1] - '0';
      recognized &= 0<=tens_digit && tens_digit<=numCoeff/10 && 0<=ones_digit && ones_digit<=9;
      int index = 10*tens_digit + ones_digit;
      recognized &= index<numCoeff;
      if (!recognized)
         {
         CString msg;
         msg.Format("WW2100AP Unrecognized coefficient symbol %s.  Recognizable symbols are %s0, ... %s%d", 
               (LPCTSTR)  symbol, symbolPrefix, symbolPrefix, numCoeff - 1);
         Report::ErrorMsg(msg);
         return false;
         }
      ASSERT(0<=index && index<numCoeff);
      coeff[index] = coeffsFromXml[i]->m_value;
      }

   return(true);
   } // end of InterpretXmlCoefficients 


void WW2100AP::RequiredTagIsMissing(CString tag, CString filename)
   {
   RequiredTagIsMissing(tag, filename, CString("."));
   }

   
void WW2100AP::RequiredTagIsMissing(CString tag, CString filename, CString whenClause)
      {
      CString msg( "WW2100AP: Missing <");
      msg += tag;
      msg += "> tag when reading ";
      msg += filename;
      msg += ".  This is a required tag";
      msg += whenClause;
      Report::ErrorMsg( msg );
      }


bool WW2100AP::LoadXmlTDcoefficient(TiXmlElement *pXml0, LPCTSTR fileName)
   {
   bool ok;

   TiXmlElement *pXmlTDcoefficient = pXml0->FirstChildElement( _T( "time_dependent_coefficient" ) );
   if ( pXmlTDcoefficient == NULL )
      {
      CString msg( "WW2100AP: Missing <time_dependent_coefficient> tag when reading " );
      msg += fileName;
      msg += ".  This is a required tag";
      Report::ErrorMsg( msg );
      return false;
      }

   LPTSTR _symbol = NULL;
   LPTSTR _method = NULL;
   LPTSTR _value = NULL;
   LPTSTR _description = NULL;
   XML_ATTR TDcoefficientAttrs[] = {
         // attr        type           address          isReq  checkCol
         { "symbol",    TYPE_STRING,   &_symbol,         true,     0 },
         { "method",    TYPE_STRING,   &_method,         true,     0 },
         { "value",     TYPE_STRING,   &_value,          true,     0 },
         { "description", TYPE_STRING, &_description,    false,    0 },
         { NULL,           TYPE_NULL,     NULL,         false,    0 } };

   ok = TiXmlGetAttributes( pXmlTDcoefficient, TDcoefficientAttrs, fileName, NULL );     
   //if (ok) ok = TiXmlGetAttributes( pXmlTDcoefficient, TDcoefficientAttrs, fileName, NULL );
   //if (ok) ok = TiXmlGetAttributes( pXmlTDcoefficient, TDcoefficientAttrs, fileName, NULL );
   //if (ok) TiXmlGetAttributes( pXmlTDcoefficient, TDcoefficientAttrs, , fileName, NULL );
   if (!ok || _symbol==NULL || _method==NULL || _value==NULL) 
      {
      CString msg( "WW2100AP: Missing or malformed symbol, method, or value in <time_dependent_coefficient> tag when reading " );
      msg += fileName;
      msg += ".  This is a required tag";
      Report::ErrorMsg( msg );
      return false;
      }

   TARGET_METHOD method;
   if ( lstrcmpi( _method, _T("rateLinear") ) == 0 ) method = TM_RATE_LINEAR;
   else if ( lstrcmpi( _method, _T("rateExp") ) == 0 ) method = TM_RATE_EXP;
   else if ( lstrcmpi( _method, _T("timeseries_linear") ) == 0 ) method = TM_TIMESERIES_LINEAR;
   else if ( lstrcmpi( _method, _T("timeseries_constant_rate") ) == 0 ) method = TM_TIMESERIES_CONSTANT_RATE;
   else
      {
      CString msg; 
      msg.Format( _T("WW2100AP: Unrecognized method in <time_dependent_coefficient> attributes in input file %s; symbol = %s, method = %s"), 
            fileName, _symbol, _method );
      Report::ErrorMsg( msg );
      return false;
      }

   m_TDcoefficient.m_symbol = _symbol;
   m_TDcoefficient.m_method = method;
   m_TDcoefficient.m_description = _description;
   switch( method )   
      {
      case TM_TIMESERIES_LINEAR:
         {
         m_TDcoefficient.m_targetValues = _value;
         m_TDcoefficient.m_pTDdata = new FDataObj( 2, 0 );

         TCHAR *targetValues = new TCHAR[ lstrlen( _value ) + 2 ];
         lstrcpy( targetValues, _value );

         TCHAR *nextToken = NULL;
         LPTSTR token = _tcstok_s( targetValues, _T(",() ;\r\n"), &nextToken );

         float pair[2];
         while ( token != NULL )
            {
            pair[0] = (float) atof( token );
            token   = _tcstok_s( NULL, _T(",() ;\r\n"), &nextToken );
            pair[1] = (float) atof( token );
            token   = _tcstok_s( NULL, _T(",() ;\r\n"), &nextToken );
   
            m_TDcoefficient.m_pTDdata->AppendRow( pair, 2 );
            }

         delete [] targetValues;
         }
         break;

      case TM_RATE_LINEAR:
      case TM_RATE_EXP:
      case TM_TIMESERIES_CONSTANT_RATE:
      default:
         {
         CString msg; 
         msg.Format( _T("WW2100AP: Unimplemented method in <time_dependent_coefficient> attributes in input file %s; symbol = %s, method = %s"), 
               fileName, _symbol, _method );
         Report::ErrorMsg( msg );
         return false;
         }
         break;
      } // end of switch ( method )

   return true;
   } // end of LoadXmlTDcoefficient() 


bool WW2100AP::LoadXmlTDdata(const char * symbolStr, LPCTSTR fileName, TiXmlElement *pXmlTDdata, TDdataset * pTDdata)
   {
   bool ok;

//   TiXmlElement *pXmlTDdata = pXml0->FirstChildElement( _T( "time_dependent_data" ) );
   if ( pXmlTDdata == NULL )
      {
      CString msg( "WW2100AP: Missing <time_dependent_data> tag when reading " );
      msg += fileName;
      msg += ".  This is a required tag";
      Report::ErrorMsg( msg );
      return false;
      }

   LPTSTR _symbol = NULL;
   LPTSTR _type = NULL;
   LPTSTR _method = NULL;
   LPTSTR _value = NULL;
   LPTSTR _description = NULL;

   XML_ATTR TDdataAttrs[] = {
         // attr        type           address          isReq  checkCol
         { "symbol",    TYPE_STRING,   &_symbol,         true,     0 },
         { "type",      TYPE_STRING,   &_type,           true,     0 },
         { "method",    TYPE_STRING,   &_method,         true,     0 },
         { "value",     TYPE_STRING,   &_value,          true,     0 },
         { "description", TYPE_STRING, &_description,    false,    0 },
         { NULL,           TYPE_NULL,     NULL,         false,    0 } };
   ok = TiXmlGetAttributes( pXmlTDdata, TDdataAttrs, fileName, NULL );  
   
   if (ok && _symbol != NULL )
      ok = strcmp(symbolStr, _symbol)==0;   
   
   if ( !ok )
      {
      CString msg;
      msg.Format("WW2100AP: In %s <time_dependent_data>, found '%s' when looking for %s.", fileName, _symbol, symbolStr);
      Report::ErrorMsg( msg );
      return false;
      }
   if (ok) ok = TiXmlGetAttributes( pXmlTDdata, TDdataAttrs, fileName, NULL );
   //if (ok) ok = TiXmlGetAttributes( pXmlTDdata, TDdataAttrs, fileName, NULL );
   //if (ok) ok = TiXmlGetAttributes( pXmlTDdata, TDdataAttrs, fileName, NULL );
   //if (ok) TiXmlGetAttributes( pXmlTDdata, TDdataAttrs,  fileName, NULL );
   if (!ok || _symbol==NULL || _type==NULL || _method==NULL || _value==NULL) 
      {
      CString msg( "WW2100AP: Missing or malformed symbol, type, method, or value in <time_dependent_data> tag when reading " );
      msg += fileName; 
      msg += " ";
      
      if ( _symbol != NULL )
        msg += _symbol;
      else
         msg += "<undefined>";

      msg += ".  This is a required tag";
      Report::ErrorMsg( msg );
      return false;
      }
/*
enum TD_DATA_TYPE {TD_TYPE_FILE, TD_TYPE_TIMESERIES};
enum TD_DATA_METHOD {TD_METHOD_LINEAR, TD_METHOD_CONSTANT_RATE};
*/
   // Interpret type
   TD_DATA_TYPE td_type;
   if ( lstrcmpi( _type, _T("file") ) == 0 ) td_type = TD_TYPE_FILE;
   else if ( lstrcmpi( _type, _T("timeseries") ) == 0 ) td_type = TD_TYPE_TIMESERIES;
   else
   {
   CString msg; 
   msg.Format( _T("WW2100AP: Unrecognized type in <time_dependent_dataset> attributes in input file %s; symbol = %s, type = %s"), 
         fileName, _symbol, _type );
   Report::ErrorMsg( msg );
   return false;
   }

   // Interpret method
   TD_DATA_METHOD method;
   if ( lstrcmpi( _method, _T("linear") ) == 0 ) method = TD_METHOD_LINEAR;
   else if ( lstrcmpi( _method, _T("constant_rate") ) == 0 ) method = TD_METHOD_CONSTANT_RATE;
   else
   {
   CString msg; 
   msg.Format( _T("WW2100AP: Unrecognized method in <time_dependent_dataset> attributes in input file %s; symbol = %s, method = %s"), 
         fileName, _symbol, _method );
   Report::ErrorMsg( msg );
   return false;
   }

   pTDdata->m_symbol = _symbol;
   pTDdata->m_type = td_type;
   pTDdata->m_method = method;
   pTDdata->m_description = _description;
   switch( method )   
      {
      case TD_METHOD_LINEAR:
         {
         pTDdata->m_value_str = _value;
         pTDdata->m_pTDdata = new FDataObj( 2, 0 );

         TCHAR *targetValues = new TCHAR[ lstrlen( _value ) + 2 ];
         lstrcpy( targetValues, _value );

         TCHAR *nextToken = NULL;
         LPTSTR token = _tcstok_s( targetValues, _T(",() ;\r\n"), &nextToken );

         float pair[2];
         while ( token != NULL )
            {
            pair[0] = (float) atof( token );
            token   = _tcstok_s( NULL, _T(",() ;\r\n"), &nextToken );
            pair[1] = (float) atof( token );
            token   = _tcstok_s( NULL, _T(",() ;\r\n"), &nextToken );
   
            pTDdata->m_pTDdata->AppendRow( pair, 2 );
            }

         delete [] targetValues;
         }
         break;

      case TD_METHOD_CONSTANT_RATE:
      default:
         {
         CString msg; 
         msg.Format( _T("WW2100AP: Unimplemented method in <time_dependent_data> attributes in input file %s; symbol = %s, method = %s"), 
               fileName, _symbol, _method );
         Report::ErrorMsg( msg );
         return false;
         }
         break;
      } // end of switch ( method )

   return true;
   } // end of LoadXmlTDdata() 


bool WW2100AP::attCheck(int attVal, int minVal, int maxVal, LPCTSTR attName) 
   {
   bool rtnVal = minVal<=attVal && attVal<=maxVal;

   if (!rtnVal)
      {
      CString msg;
      msg.Format("WW2100AP: attribute %s is out of limits. attVal, minVal, maxVal = %d, %d, %d. Acceptable range includes limit values.", 
            attName, attVal, minVal, maxVal);
      Report::ErrorMsg(msg);
      }

   return(rtnVal);
   } // end of attCheck() for integer attributes


bool WW2100AP::attCheck(float attVal, float minVal, float maxVal, LPCTSTR attName) 
   {
   bool rtnVal = minVal<attVal && attVal<maxVal;

   if (!rtnVal)
      {
      CString msg;
      msg.Format("WW2100AP: attribute %s is out of limits. attVal, minVal, maxVal = %f, %f, %f. Limit values are excluded from acceptable range.", 
            attName, attVal, minVal, maxVal);
      Report::ErrorMsg(msg);
      }

   return(rtnVal);
   } // end of attCheck()


bool WW2100AP::LoadXml( EnvContext *pContext, LPCTSTR filename )
   {
   CString msg;

   // have xml string, start parsing
   TiXmlDocument doc;
   bool ok = doc.LoadFile( filename );

   if ( ! ok )
      {      
      Report::ErrorMsg( doc.ErrorDesc() );
      return false;
      }
 
   // start interating through the nodes
   TiXmlElement *pXmlEnvision = doc.RootElement();  // Envision
   TiXmlElement *pXmlRoot = pXmlEnvision->FirstChildElement( "ww2100" );
   if (!m_commonInitComplete)
      {
      m_testMode = 0;
      pXmlRoot->Attribute( "test_mode", &m_testMode );

      double tau = -1.f;
      const char * attStr = pXmlRoot->Attribute( "tau_climate", &tau );
      if (attStr!=NULL)
         { // Try to use value of tau from XML file.
         if (tau>=1) m_tauClimate = (float)tau;
         else 
            {
            CString msg;
            msg.Format("WW2100AP: tau_climate specified as %f in XML file %s, but tau must be >= 1.", (float)tau, filename);
            Report::ErrorMsg(msg);
            return false;
            }
         } // end of if (attStr!=NULL)
      else m_tauClimate = 30.; // Use default value of tau.

      tau = -1.f;
      attStr = pXmlRoot->Attribute( "tau_WR", &tau );
      if (attStr!=NULL)
         { // Try to use value of tau from XML file.
         if (tau>=1) m_tauWR = (float)tau;
         else 
            {
            CString msg;
            msg.Format("WW2100AP: tau_WR specified as %f in XML file %s, but tau must be >= 1.", (float)tau, filename);
            Report::ErrorMsg(msg);
            return false;
            }
         } // end of if (attStr!=NULL)
      else m_tauWR = 10.; // Use default value of tau.

      m_LandUseInterval = -1;
      attStr = pXmlRoot->Attribute( "land_use_interval", &m_LandUseInterval );
      if (attStr!=NULL && !attCheck(m_LandUseInterval, 1, 30, "land_use_interval")) return(false);
      if (attStr==NULL) m_LandUseInterval = 5; // Use the default value of land_use_interval

      m_UGApopProj_file = "";
      m_UGApopProj_file = pXmlRoot->Attribute( "UGApop_file" );

      m_HholdInc_file = "";
      m_HholdInc_file = pXmlRoot->Attribute( "household_income_file" );

      ok = LoadXmlTDcoefficient(pXmlRoot, filename);

      TiXmlElement *pXmlClimatefile = pXmlRoot->FirstChildElement( "climate_files" );
      if ( pXmlClimatefile == NULL ) RequiredTagIsMissing(_T("climate_files"), filename);

      XML_ATTR climatefileAttrs[] = {
         // attr                     type           address                  isReq  checkCol        
         { "precip_filename",       TYPE_CSTRING,  &(m_Precip_filename),      true,   0 },
         { "tmin_filename",         TYPE_CSTRING,  &(m_Tmin_filename),        true,   0 },
         { NULL,                    TYPE_NULL,     NULL,                      false,  0 } };

      ok &= TiXmlGetAttributes( pXmlClimatefile, climatefileAttrs, filename, NULL ); 
      //ok &= TiXmlGetAttributes( pXmlClimatefile, climatefileAttrs, "tmin_filename", filename, NULL );  
      }

   // Cold start nodes
   TiXmlElement *pXmlColdStart = pXmlRoot->FirstChildElement( _T("ColdStart" ) );
   if ( pXmlColdStart == NULL ) { RequiredTagIsMissing("ColdStart", filename); return false; }
   m_useColdStart = 0;
   pXmlColdStart->Attribute( "id", &m_useColdStart );

   if (m_useColdStart==pContext->id)
      {
      int intval = 0;
      pXmlColdStart->Attribute("use_vegclass_initialization", &intval);
      m_useColdStart_vegclass_initialization = intval != 0;

      m_A2S_file = ""; // age2SS.csv
      m_A2S_file = pXmlColdStart->Attribute("Age2StructuralStageFile");
      m_useColdStart_standage_reconciliation = m_A2S_file.GetLength()>0;

      m_initial_irrigation_fraction = -1;
      pXmlColdStart->Attribute( "initial_irrigation_fraction", &m_initial_irrigation_fraction );

      m_SWHCfile = "";
      m_SWHCfile = pXmlColdStart->Attribute( "SWHCfile" );
      m_useColdStart_SWHCfile = m_SWHCfile.GetLength()>0;

      m_PopUnits_file = "";
      m_PopUnits_file = pXmlColdStart->Attribute( "PopulationUnitsFile" );
      m_useColdStart_PopUnitsfile = m_PopUnits_file.GetLength()>0;

      TiXmlElement *pXmlForestSTMfiles = pXmlColdStart->FirstChildElement( _T( "ForestSTMfiles" ) );
      m_useColdStart_ForestSTMfiles = pXmlForestSTMfiles != NULL;
      if ( m_useColdStart_ForestSTMfiles ) 
         {
         XML_ATTR setAttrs[] = {
            // attr                     type           address                                     isReq  checkCol        
            { "DGVMvegtype_file",    TYPE_CSTRING,  &(m_DGVMvegtypefile.DGVMvegtype_filename),     true,   0 },
            { "CoverTypes_file", TYPE_CSTRING, &(m_CoverTypes_file), true, 0 },
            { "StructuralStages_file", TYPE_CSTRING, &(m_StructuralStages_file), true, 0 },
            { "TransitionTypes_file", TYPE_CSTRING, &(m_TransitionTypes_file), true, 0 },
            { "Stratum2PVT_file", TYPE_CSTRING, &(m_Stratum2PVT_file), true, 0 },
            { "LAIandC_file", TYPE_CSTRING, &(m_LAIandC_file), true, 0 },
            { "DeterministicTransitionInput_file", TYPE_CSTRING, &(m_DeterministicTransitionInput_file), true, 0 },
            { "ProbabilisticTransitionInput_file", TYPE_CSTRING, &(m_ProbabilisticTransitionInput_file), true, 0 },
            // { "TransitionProbabilityMultiplierInput_file", TYPE_CSTRING, &(m_TransitionProbabilityMultiplierInput_file), true, 0 },
            { "DeterministicTransitionOutput_file", TYPE_CSTRING, &(m_DeterministicTransitionOutput_file), true, 0 },
            { "ProbabilisticTransitionOutput_file", TYPE_CSTRING, &(m_ProbabilisticTransitionOutput_file), true, 0 },
            // { "TransitionProbabilityMultiplierOutputFile", TYPE_CSTRING, &(m_TransitionProbabilityMultiplierOutputFile), true, 0 },
            { NULL,                      TYPE_NULL,     NULL,                                        false,  0 } };

         ok = TiXmlGetAttributes( pXmlForestSTMfiles, setAttrs, filename, NULL );
         }

      }
   // else { RequiredTagIsMissing("ForestSTMfiles", filename, " when doing a cold start."); return false; }

   // fish model nodes
   TiXmlElement *pXmlFish = pXmlRoot->FirstChildElement( _T("fish" ) );
   if ( pXmlFish == NULL )
      {
      CString msg( "WW2100AP: Missing <fish> tag when reading " );
      msg += filename;
      msg += ".  This is a required tag";
      Report::ErrorMsg( msg );
      return false;
      }

   m_useFishModel = 0;
   pXmlFish->Attribute( "id", &m_useFishModel );
   
   if (m_useFishModel==pContext->id)
      {

      TiXmlElement *pXmlGroups = pXmlFish->FirstChildElement( _T( "groups" ) );
      if ( pXmlGroups == NULL )
         {
         CString msg( "WW2100AP: Missing <groups> tag when reading " );
         msg += filename;
         msg += ".  This is a required tag";
         Report::ErrorMsg( msg );
         return false;
         }

      TiXmlElement *pXmlGroup = pXmlGroups->FirstChildElement( _T("group" ) );
      while ( pXmlGroup != NULL )
         {
         int id = -1;
         LPTSTR name = NULL;
     
         XML_ATTR group[] = {
            // attr        type           address          isReq  checkCol
            { "name",         TYPE_STRING,   &name,        true,     0 },
            { "id",           TYPE_INT,      &id,          true,     0 },
            { NULL,           TYPE_NULL,     NULL,         false,    0 } };

         ok = TiXmlGetAttributes( pXmlGroup, group, filename );
         if ( ! ok )
            {
            CString msg; 
            msg.Format( _T("WW2100AP: Misformed root element reading <group> attributes in input file %s"), filename );
            Report::ErrorMsg( msg );
            return false;
            }

         SpeciesGroup *pGroup = new SpeciesGroup;
         pGroup->m_id = id;
         pGroup->m_name = name;
         if (m_sppGroupArray.GetSize() < MAX_FISH_GROUPS) m_sppGroupArray.Add( pGroup );
         else
            {
            CString msg;
            msg.Format(_T("WW2100AP: number of fish groups in %s exceeds MAX_FISH_GROUPS.  MAX_FISH_GROUPS is %d."), filename, MAX_FISH_GROUPS);
            Report::ErrorMsg(msg);
            return false;
            }

         pXmlGroup = pXmlGroup->NextSiblingElement( _T("group" ) );
         }

      // next, process <species> tags
      TiXmlElement *pXmlSpp = pXmlFish->FirstChildElement( _T("species" ) );
      if ( pXmlSpp == NULL )
         {
         CString msg( "WW2100AP: Missing <species> tag when reading " );
         msg += filename;
         msg += ".  This is a required tag";
         Report::ErrorMsg( msg );
         return false;
         }

      TiXmlElement *pXmlSpecie = pXmlSpp->FirstChildElement( _T("specie" ) );
      while ( pXmlSpecie != NULL )
         {
         int use = 0;
         LPTSTR name = NULL;
         LPTSTR code = NULL;
         int groupID = NULL;
         float intercept = 0;
         float tMax = 0;

         XML_ATTR species[] = {
            // attr        type           address          isReq  checkCol
            { "name",         TYPE_STRING,   &name,        true,     0 },
            { "code",         TYPE_STRING,   &code,        true,     0 },
            { "group",        TYPE_INT,      &groupID,     true,     0 },
            { "use",          TYPE_INT,      &use,         true,     0 },
            { "intercept",    TYPE_FLOAT,    &intercept,   true,     0 },
            { "tmax",         TYPE_FLOAT,    &tMax,        true,     0 },
            { NULL,           TYPE_NULL,     NULL,         false,    0 } };

         ok = TiXmlGetAttributes( pXmlSpecie, species, filename );
         if ( ! ok )
            {
            CString msg; 
            msg.Format( _T("WW2100AP: Misformed root element reading <specie> attributes in input file %s"), filename );
            Report::ErrorMsg( msg );
            return false;
            }

         FishSpecie *pFish = new FishSpecie;
         pFish->m_code = code;
         pFish->m_name = name;
         pFish->m_use = use;
         pFish->m_intercept = intercept;
         pFish->m_tMax = tMax;
         pFish->m_group = groupID;
      
         SpeciesGroup *pGroup = FindSppGroupFromID( groupID );
         if ( pGroup == NULL )
            {
            delete pFish;
            CString msg; 
            msg.Format( _T("Fish Model: Unable to find Species Group %i references by specie %s in input file %s"), groupID, name, filename );
            Report::ErrorMsg( msg );
            }
         else
            pGroup->m_sppArray.Add( pFish );

         pXmlSpecie = pXmlSpecie->NextSiblingElement( _T("specie" ) );
         }
      } // end of logic to process <fish > </fish>

   // next, carbon model nodes
   TiXmlElement *pXmlCarbon = pXmlRoot->FirstChildElement( _T("carbon" ) );
   if ( pXmlCarbon == NULL )
      {
      CString msg( "WW2100AP: Missing <carbon> tag when reading " );
      msg += filename;
      msg += ".  This is a required tag";
      Report::ErrorMsg( msg );
      return false;
      }

   m_useCarbonModel = 0;
   pXmlCarbon->Attribute( "id", &m_useCarbonModel );

   // next, land use transitions model nodes
   TiXmlElement *pXmlSubProc = NULL;
   if (StartXMLforSubprocess(pContext, filename, pXmlRoot, _T("land_use_transitions" ), &m_idLandTrans, &pXmlSubProc, &m_LTtestMode))
      {
      if (m_idLandTrans>0 && m_idLandTrans==pContext->id)
         {
         bool ok;

         // Get the coefficients and parameters for the developed use predicted value model.      
         TiXmlElement * pXmlPredictedVal;
         TiXmlElement * pXmlCoefficient;
         TiXmlElement * pXmlParameter;
         pXmlPredictedVal = pXmlSubProc->FirstChildElement(_T("developed_use"));
         ok = pXmlPredictedVal!=NULL;
         ok &= LoadXmlCoefficients(filename, pXmlPredictedVal, &m_LTcoefArray);
         ok &= InterpretXmlCoefficients("beta", m_LTdevBeta, 10, m_LTcoefArray);
         pXmlCoefficient = pXmlPredictedVal->FirstChildElement("coefficient");
         ok &= LoadXmlCoefficient(filename, pXmlCoefficient, _T("Correction_factor"), &m_LTdevCorrFactor);
         pXmlParameter = pXmlPredictedVal->FirstChildElement("parameter");
         ok &= LoadXmlParameter(filename, pXmlParameter, _T("imp"), &m_LTdevImp);
         
         // Get the coefficients and parameters for the agricultural use predicted value model.      
         pXmlPredictedVal = pXmlSubProc->FirstChildElement(_T("agricultural_use"));
         ok = pXmlPredictedVal!=NULL;
         ok &= LoadXmlCoefficients(filename, pXmlPredictedVal, &m_LTcoefArray);
         ok &= InterpretXmlCoefficients("beta", m_LTagBeta, 11, m_LTcoefArray);
         pXmlCoefficient = pXmlPredictedVal->FirstChildElement("coefficient");
         ok &= LoadXmlCoefficient(filename, pXmlCoefficient, _T("Correction_factor"), &m_LTagCorrFactor);
         pXmlParameter = pXmlPredictedVal->FirstChildElement("parameter");
         ok &= LoadXmlParameter(filename, pXmlParameter, _T("imp"), &m_LTagImp);
         pXmlParameter = pXmlParameter->NextSiblingElement("parameter");
         ok &= LoadXmlParameter(filename, pXmlParameter, _T("mean_imp"), &m_LTagMeanImp);
         
         // Get the coefficients and parameters for the forest use predicted value model.      
         pXmlPredictedVal = pXmlSubProc->FirstChildElement(_T("forest_use"));
         ok = pXmlPredictedVal!=NULL;
         ok &= LoadXmlCoefficients(filename, pXmlPredictedVal, &m_LTcoefArray);
         ok &= InterpretXmlCoefficients("beta", m_LTforestBeta, 14, m_LTcoefArray);
         pXmlCoefficient = pXmlPredictedVal->FirstChildElement("coefficient");
         ok &= LoadXmlCoefficient(filename, pXmlCoefficient, _T("Correction_factor"), &m_LTforestCorrFactor);
         pXmlParameter = pXmlPredictedVal->FirstChildElement("parameter");
         ok &= LoadXmlParameter(filename, pXmlParameter, _T("imp"), &m_LTforestImp);
         pXmlParameter = pXmlParameter->NextSiblingElement("parameter");
         ok &= LoadXmlParameter(filename, pXmlParameter, _T("mean_imp"), &m_LTforestMeanImp);

         // Get the coefficients and parameters for calculating transition probabilities.
         TiXmlElement * pXmlTransitions = pXmlSubProc->FirstChildElement(_T("transitions"));
         ok &= pXmlTransitions!=NULL;
         TiXmlElement * pXmlModelScalar = pXmlTransitions->FirstChildElement("coefficient");
         ok &= LoadXmlCoefficient(filename, pXmlModelScalar, _T("self"), &m_LTself); 
         pXmlModelScalar = pXmlModelScalar->NextSiblingElement("coefficient");        
         ok &= LoadXmlCoefficient(filename, pXmlModelScalar, _T("crossR"), &m_LTcrossR);         
         pXmlModelScalar = pXmlModelScalar->NextSiblingElement("coefficient");        
         ok &= LoadXmlCoefficient(filename, pXmlModelScalar, _T("DR"), &m_LT_DR);  
         TiXmlElement * pXmlTransition = pXmlTransitions->FirstChildElement("transition");       
         ok &= LoadXmlTransition(filename, pXmlTransition, _T("ag"), &m_LTfromAg); 
         pXmlTransition = pXmlTransition->NextSiblingElement("transition");       
         ok &= LoadXmlTransition(filename, pXmlTransition, _T("forest"), &m_LTfromForest); 
               
         } // end of if (m_idLandTrans>0 && m_idLandTrans==pContext->id)
      } // end of if (StartXMLforSubprocess(...))
   else return(false); // Couldn't find the land_use_transitions tag

   // next, urban water model nodes
   pXmlSubProc = NULL;
   if (StartXMLforSubprocess(pContext, filename, pXmlRoot, _T("urban_water" ), &m_idUW, &pXmlSubProc, &m_UWtestMode))
      {
      if (m_idUW>0 && m_idUW==pContext->id)
         {
         m_UWmfgInc_file = "";
         m_UWmfgInc_file = pXmlSubProc->Attribute( "manufacturing_income_file" );
         m_UWcommInc_file = "";
         m_UWcommInc_file = pXmlSubProc->Attribute( "commercial_income_file" );
         m_UWcoolingCost_file = "";
         m_UWcoolingCost_file = pXmlSubProc->Attribute( "cooling_cost_file" );
         m_UW_FCUresPrice_file = "";
         m_UW_FCUresPrice_file = pXmlSubProc->Attribute("FullCostUrb_residential_price_file");
         m_UW_FCUindPrice_file = "";
         m_UW_FCUindPrice_file = pXmlSubProc->Attribute("FullCostUrb_commercial_industrial_price_file");

         TiXmlElement *pXmlPrices = pXmlSubProc->FirstChildElement( _T( "prices" ) );
         if ( pXmlPrices == NULL ) { RequiredTagIsMissing( _T( "prices" ) , filename); return false; }

         TiXmlElement *pXmlPrice = pXmlPrices->FirstChildElement( _T("price_ccf" ) );
         while ( pXmlPrice != NULL )
            {
            LPTSTR area_str = NULL;
            float residential_price = 0.f;
            float non_residential_price = 0.f;
            int IBR_code = 0;
            float population_divisor = 0.f;
     
            XML_ATTR Price[] = {
               // attr              type           address                 isReq  checkCol
               { "area",            TYPE_STRING,   &area_str,              true,     0 },
               { "residential",     TYPE_FLOAT,    &residential_price,     true,     0 },
               { "non-residential", TYPE_FLOAT,    &non_residential_price, true,     0 },
               { "IBR",             TYPE_INT,      &IBR_code,              true,     0 },
               { "population_divisor", TYPE_FLOAT, &population_divisor,    true,     0 },
               { NULL,              TYPE_NULL,     NULL,                   false,    0 } };

            ok = TiXmlGetAttributes( pXmlPrice, Price, filename );
            if ( ! ok )
               {
               CString msg; 
               msg.Format( _T("WW2100AP: Misformed root element reading <price_ccv> attributes in input file %s"), filename );
               Report::ErrorMsg( msg );
               return false;
               }

            PriceCCF *pPrice = new PriceCCF;
            pPrice->m_area = area_str;
            pPrice->m_residential_price = residential_price;
            pPrice->m_non_residential_price = non_residential_price;
            if (!attCheck(population_divisor, 0.f, 100.f, _T("population_divisor"))) return(false);
            pPrice->m_population_divisor = population_divisor;
            m_UWpriceArray.Add(pPrice);

            pXmlPrice = pXmlPrice->NextSiblingElement( _T("price_ccf" ) );
            } // end of while( pXmlPrice != NULL )
         } // end of if (m_idUW>0 && m_idUW==pContext->id)
      } // end of if (StartXMLforSubprocess(...))
   else return(false); // Couldn't find the urban_water tag


   // next, DGVM vegtype model nodes
   TiXmlElement *pXmlDGVMvegtypeData = pXmlRoot->FirstChildElement( _T("DGVMvegtypeData" ) );
   if ( pXmlDGVMvegtypeData == NULL )
      {
      CString msg( "WW2100AP: Missing <DGVMvegtypeData> tag when reading " );
      msg += filename;
      msg += ".  This is a required tag";
      Report::ErrorMsg( msg );
      return false;
      }

   m_useDGVMvegtypeData = 0;
   pXmlDGVMvegtypeData->Attribute( "id", &m_useDGVMvegtypeData );

   if (m_useDGVMvegtypeData==pContext->id)
      {
      TiXmlElement *pXmlDGVMvegtypeDatafile = pXmlDGVMvegtypeData->FirstChildElement( "DGVMvegtypeData_file" );
      XML_ATTR setAttrs[] = {
         // attr                     type           address                                     isReq  checkCol        
         { "DGVMvegtype_filename",    TYPE_CSTRING,  &(m_DGVMvegtypefile.DGVMvegtype_filename),     true,   0 },
         { NULL,                      TYPE_NULL,     NULL,                                        false,  0 } };

      ok = TiXmlGetAttributes( pXmlDGVMvegtypeDatafile, setAttrs, filename, NULL );     

      } // end of if (m_useDGVMvegtypeData)


   // next, Irrigation Choice model nodes
   if (StartXMLforSubprocess(pContext, filename, pXmlRoot, _T("irrigation_choice"), &m_idIrr, &pXmlSubProc))
      {
      if (m_idIrr==pContext->id)
         {
         int int_att_val = 0;
         const char * attStr = pXmlSubProc->Attribute( "test_mode", &int_att_val );
         if (attStr!=NULL)
            { // Try to use value from XML file.
            if (int_att_val>=0 && int_att_val<=2) m_IrrTestMode = int_att_val;
            else 
               {
               CString msg;
               msg.Format("WW2100AP Irrigation Choice: test_mode specified as %s in XML file %s, but test_mode must be >= 0 and <=2.", attStr, filename);
               Report::ErrorMsg(msg);
               return false;
               }
            } // end of if (attStr!=NULL)
         else m_IrrTestMode = 0; // Use default value.

         TiXmlElement *pXmlCoefficients = pXmlSubProc->FirstChildElement( _T( "coefficients" ) );
         if ( pXmlCoefficients == NULL )
            {
            CString msg( "WW2100AP: Missing <coefficients> tag when reading " );
            msg += filename;
            msg += ".  This is a required tag";
            Report::ErrorMsg( msg );
            return false;
            }

         TiXmlElement *pXmlCoefficient = pXmlCoefficients->FirstChildElement( _T("coefficient" ) );
         while ( pXmlCoefficient != NULL )
            {
            float coef_value = 0.f;
            LPTSTR symbol = NULL;
            LPTSTR description = NULL;
     
            XML_ATTR Coefficient[] = {
               // attr        type           address          isReq  checkCol
               { "symbol",    TYPE_STRING,   &symbol,         true,     0 },
               { "value",     TYPE_FLOAT,    &coef_value,     true,     0 },
               { "description", TYPE_STRING, &description,    false,    0 },
               { NULL,           TYPE_NULL,     NULL,         false,    0 } };

            ok = TiXmlGetAttributes( pXmlCoefficient, Coefficient, filename );
            if ( ! ok )
               {
               CString msg; 
               msg.Format( _T("WW2100AP: Misformed root element reading <coefficient> attributes in input file %s"), filename );
               Report::ErrorMsg( msg );
               return false;
               }

            ModelCoefficient *pCoefficient = new ModelCoefficient;
            pCoefficient->m_symbol = symbol;
            pCoefficient->m_coef_value = coef_value;
            m_IrrCoefficientArray.Add( pCoefficient );

            pXmlCoefficient = pXmlCoefficient->NextSiblingElement( _T("Coefficient" ) );
            }
         } // end of if (m_idIrr)
      } // end of if (StartXMLforSubprocess(...))


   // popGrowth nodes
/*
   TiXmlElement *pXmlPopGrowth = pXmlRoot->FirstChildElement( _T("popGrowth" ) );
   if ( pXmlPopGrowth == NULL ) { RequiredTagIsMissing("popGrowth", filename); return false; }
   m_idPG = 0;
   pXmlPopGrowth->Attribute( "id", &m_idPG );

   if (m_idPG==pContext->id)
      {
*/
   if (StartXMLforSubprocess(pContext, filename, pXmlRoot, _T("popGrowth" ), &m_idPG, &pXmlSubProc, &m_PGtestMode))
      {
      if (m_idPG>0 && m_idPG==pContext->id)
         {
         m_popGrowth_RPAcolName = ""; m_popGrowth_RPAcolName = pXmlSubProc->Attribute( "ruralPopUnitCol" );
         m_RPApopProj_file = "";
         m_RPApopProj_file = pXmlSubProc->Attribute( "RRpop_file" );
         } // end of if (m_idPG>0 && m_idPG==pContext->id)
      } // end of if (StartXMLforSubprocess(...))

   // farmland rent nodes
   if (StartXMLforSubprocess(pContext, filename, pXmlRoot, _T("farmland_rent"), &m_useFR, &pXmlSubProc))
      {
      if (m_useFR>0 && m_useFR==pContext->id)
         {
         int int_att_val = 0;
         const char * attStr = pXmlSubProc->Attribute( "test_mode", &int_att_val );
         if (attStr!=NULL)
            { // Try to use value from XML file.
            if (int_att_val>=0 && int_att_val<=2) m_FRtestMode = int_att_val;
            else 
               {
               CString msg;
               msg.Format("WW2100AP Farmland Rent: test_mode specified as %s in XML file %s, but test_mode must be >= 0 and <=2.", attStr, filename);
               Report::ErrorMsg(msg);
               return false;
               }
            } // end of if (attStr!=NULL)
         else m_FRtestMode = 0; // Use default value.

         bool rtnFlag;
         rtnFlag = LoadXmlCoefficients(filename, pXmlSubProc, &m_FRcoefficientArray, _T("a"));
         if (!rtnFlag) return(false);

         // Load the time dependent data.
         TiXmlElement *pXmlTDdata = pXmlSubProc->FirstChildElement( _T( "time_dependent_data" ) );
         LoadXmlTDdata(_T( "Cw_groundwater" ), filename, pXmlTDdata, &m_FR_Cw_ground);
         pXmlTDdata = pXmlTDdata->NextSiblingElement( _T("time_dependent_data" ) );
         LoadXmlTDdata(_T( "Cw_surfacewater" ), filename, pXmlTDdata, &m_FR_Cw_surface);

         } // end of if (m_useFR>0 && m_useFR==pContext->id)
      } // end of if (StartXMLforSubprocess(pContext, filename, pXmlRoot, _T("farmland_rent"), &m_useFR, &pXmlSubProc))
   else return(false); // Couldn't find the farmland_rent tag

   // crop choice nodes
   pXmlSubProc = NULL;
   if (StartXMLforSubprocess(pContext, filename, pXmlRoot, _T("crop_choice"), &m_useCrop, &pXmlSubProc))
      {
      if (m_useCrop>0 && m_useCrop==pContext->id)
         {
         int int_att_val = 0;
         const char * attStr = pXmlSubProc->Attribute( "test_mode", &int_att_val );
         m_CropTestMode = 0;
         if (attStr!=NULL)
            { // Try to use value from XML file.
            if (int_att_val>=0 && int_att_val<=2) m_CropTestMode = int_att_val;
            else 
               {
               CString msg;
               msg.Format("WW2100AP Crop Choice: test_mode specified as %s in XML file %s, but test_mode must be >= 0 and <=2.", attStr, filename);
               Report::ErrorMsg(msg);
               return false;
               }
            } // end of if (attStr!=NULL)

         m_cropfile = "";
         m_cropfile = pXmlSubProc->Attribute( "coefficients_file" );
         if ( m_cropfile.GetLength()<=0) { RequiredTagIsMissing(_T("coefficients_file"), filename); return(false); }

         // process <time_dependent_datasets> block
         LoadXmlTDdatasets(filename, pXmlSubProc, &m_TDdatasetArray);
         } // end of if (m_useCrop>0 && m_useCrop==pContext->id)
      } // end of if (StartXMLforSubprocess(...))
   else return(false); // Couldn't find the crop_choice tag

   // next, UGA expansion model nodes
   pXmlSubProc = NULL;
   if (StartXMLforSubprocess(pContext, filename, pXmlRoot, _T("UGA_expansion" ), &m_idUX, &pXmlSubProc, &m_UXtestMode))
      {
      if (m_idUX>0 && m_idUX==pContext->id)
         {
         int attVal = -1;
         const char * attStr = pXmlSubProc->Attribute( "default_threshold_pct", &attVal );
         m_UXdefaultThresholdPct = (float)attVal;
         m_UGAthreshold_file = "";
         m_UGAthreshold_file = pXmlSubProc->Attribute("UGA_threshold_pct_file");

         if (attStr != NULL && !attCheck(m_UXdefaultThresholdPct, 50., 100., "default_threshold_pct")) return(false);
         if (attStr==NULL) m_UXdefaultThresholdPct = 80.; // Use the default value of developed_threshold_pct
         bool rtnFlag;
         rtnFlag = LoadXmlUGAs(filename, pXmlSubProc);
         if (!rtnFlag) return(false);
         }
      } // end if (StartXMLforSubprocess(...UGA_expansion...)

   return true;
   } // end of LoadXml()


bool WW2100AP::StartXMLforSubprocess(EnvContext *pContext, LPCTSTR file_name, TiXmlElement * pXml_root, 
      CString subprocess_tag, int * p_id, TiXmlElement **ppXmlSubProc, int *pTestMode)
   {
   bool rtnVal = StartXMLforSubprocess(pContext, file_name, pXml_root, subprocess_tag, p_id, ppXmlSubProc);
   *pTestMode = 0;
   
   if (rtnVal)
      *(*ppXmlSubProc)->Attribute( "test_mode", pTestMode );   ///?Check!!!

   return(rtnVal);
   } // end of StartXMLforSubprocess(7 argument version)


bool WW2100AP::StartXMLforSubprocess(EnvContext *pContext, LPCTSTR file_name, TiXmlElement * pXml_root, 
      CString subprocess_tag, int * p_id, TiXmlElement **ppXmlSubProc)
   {
   *ppXmlSubProc = pXml_root->FirstChildElement( subprocess_tag );
   if ( *ppXmlSubProc == NULL ) { RequiredTagIsMissing(subprocess_tag, file_name); return(false); }

   *p_id = 0;
   if (*(*ppXmlSubProc)->Attribute( "id", p_id ) == NULL) { RequiredTagIsMissing("id", file_name); return(false); };

   return(true);
   } // end of StartXMLforSubprocess()


bool WW2100AP::LoadXmlTransitionConstraint(LPCTSTR filename, TiXmlElement *pXmlConstraint, LPCTSTR tgtToUse,   
      LTconstraintParams *pConstraint)
   {
   bool ok;
   ok = pXmlConstraint!=NULL;

   // Read the "from_use" field.
   CString from_useStr;
   XML_ATTR setAttrs[] = {
      // attr         type           address       isReq  checkCol        
      { "to_use", TYPE_CSTRING,  &from_useStr,     true,   0 },
      { NULL,       TYPE_NULL,     NULL,             false,  0 } };

   ok &= TiXmlGetAttributes( pXmlConstraint, setAttrs, filename, NULL ); 
   ok &= from_useStr==tgtToUse; 

   ModelScalar ms; ms.m_value = 0.;
   TiXmlElement *pXmlModelScalar = pXmlConstraint->FirstChildElement(_T("parameter"));
   ok &= LoadXmlParameter(filename, pXmlModelScalar, "max", &ms); 
   pConstraint->maxVal = (float)ms.m_value;

   pXmlModelScalar = pXmlConstraint->FirstChildElement(_T("coefficient"));
   ok &= LoadXmlParameter(filename, pXmlModelScalar, "scale_factor", &ms);   
   pConstraint->scaleFactor = (float)ms.m_value;
   
   return(ok);
   } // end of LoadXmlTransitionConstraint()


bool WW2100AP::LoadXmlTransition(LPCTSTR filename, TiXmlElement *pXmlTransition, 
      LPCTSTR tgtFromUse,  LTparamSet *pConstraintSet)
   {
   bool ok;
   ok = pXmlTransition!=NULL;

   // Read the "from_use" field.
   CString from_useStr;
   XML_ATTR setAttrs[] = {
      // attr         type           address       isReq  checkCol        
      { "from_use", TYPE_CSTRING,  &from_useStr,     true,   0 },
      { NULL,       TYPE_NULL,     NULL,             false,  0 } };

   ok &= TiXmlGetAttributes( pXmlTransition, setAttrs, filename, NULL ); 
   ok &= from_useStr==tgtFromUse;    
   
   TiXmlElement *pXmlConstraint = pXmlTransition->FirstChildElement(_T("to"));
   ok &= pXmlConstraint!=NULL; 
   ok &= LoadXmlTransitionConstraint(filename, pXmlConstraint, _T("ag"), &(pConstraintSet->toAg));

   pXmlConstraint = pXmlConstraint->NextSiblingElement(_T("to"));
   ok &= pXmlConstraint!=NULL; 
   ok &= LoadXmlTransitionConstraint(filename, pXmlConstraint, _T("forest"), &(pConstraintSet->toForest));

   pXmlConstraint = pXmlConstraint->NextSiblingElement(_T("to"));
   ok &= pXmlConstraint!=NULL; 
   ok &= LoadXmlTransitionConstraint(filename, pXmlConstraint, _T("developed"), &(pConstraintSet->toDeveloped));

   return(ok);
   } // end of LoadXmlTransition()


bool WW2100AP::LoadXmlTDdatasets(LPCTSTR file_name, TiXmlElement *pXmlSubProc, PtrArray< TDdataset > *pTDdatasetArray)
   {
   TiXmlElement *pXmlTDdatasets = pXmlSubProc->FirstChildElement( _T( "time_dependent_datasets" ) );
   if ( pXmlTDdatasets == NULL )  { RequiredTagIsMissing(_T("time_dependent_datasets"), file_name); return(false); };

   TiXmlElement *pXmlTDdataset = pXmlTDdatasets->FirstChildElement( _T("time_dependent_data" ) );
   while ( pXmlTDdataset != NULL )
      {
      LPTSTR symbol_str = NULL;
      LPTSTR description_str = NULL;
      LPTSTR type_str = NULL;
      LPTSTR method_str = NULL;
      LPTSTR value_str = NULL;
     
      XML_ATTR TDdatasetAttr[] = {
         // attr        type           address           isReq  checkCol
         { "symbol",    TYPE_STRING,   &symbol_str,      true,     0 },
         { "description", TYPE_STRING, &description_str, false,    0 },
         { "type",      TYPE_STRING,   &type_str,        true,     0 },
         { "method",    TYPE_STRING,   &method_str,      false,    0 },
         { "value",     TYPE_STRING,   &value_str,       false,    0 },
         { NULL,        TYPE_NULL,     NULL,             false,    0 } };

      bool ok = TiXmlGetAttributes( pXmlTDdataset, TDdatasetAttr, file_name );
      if ( ! ok )
         {
         CString msg; 
         msg.Format( _T("WW2100AP: Misformed root element reading <time_dependent_data ... > attributes in input file %s"), file_name );
         Report::ErrorMsg( msg );
         return false;
         }

      // Interpret type
      TD_DATA_TYPE td_type = TD_TYPE_FILE;
      
      if ( type_str != NULL )
         {
         if ( lstrcmpi( type_str, _T("file") ) == 0 ) td_type = TD_TYPE_FILE;
         else if ( lstrcmpi( type_str, _T("timeseries") ) == 0 ) td_type = TD_TYPE_TIMESERIES;
         else
            {
            CString msg; 
            msg.Format( _T("WW2100AP: Unrecognized type in <time_dependent_dataset> attributes in input file %s; symbol = %s, type = %s"), 
                  file_name, symbol_str, type_str );
            Report::ErrorMsg( msg );
            return false;
            }
         }

      // Interpret method
      TD_DATA_METHOD method = TD_METHOD_LINEAR;

      if ( method_str != NULL )
         {
         if ( lstrcmpi( method_str, _T("linear") ) == 0 ) method = TD_METHOD_LINEAR;
         else if ( lstrcmpi( method_str, _T("constant_rate") ) == 0 ) method = TD_METHOD_CONSTANT_RATE;
         else
            {
            CString msg; 
            msg.Format( _T("WW2100AP: Unrecognized method in <time_dependent_dataset> attributes in input file %s; symbol = %s, method = %s"), 
                  file_name, symbol_str, method_str );
            Report::ErrorMsg( msg );
            return false;
            }
         }

      TDdataset *pTDdataset = new TDdataset;
      pTDdataset->m_symbol = symbol_str;
      pTDdataset->m_description = description_str;
      pTDdataset->m_type = td_type;
      pTDdataset->m_method = method;
      pTDdataset->m_value_str = value_str;
      pTDdatasetArray->Add( pTDdataset );

      pXmlTDdataset = pXmlTDdataset->NextSiblingElement( _T("time_dependent_data" ) );
      }

   return(true);
   } // end of LoadXmlTDdatasets()


CString ReplaceSubstring(CString origStr,  CString strToBeReplaced,  CString replacementStr)
   {
   CString str_to_return = origStr;
   int len_to_be_replaced = strToBeReplaced.GetLength();
   int orig_len = origStr.GetLength();
   int remainder_len = orig_len - len_to_be_replaced;

   int char_pos = 0;
   while (char_pos<=remainder_len) 
      {
      if ((origStr.Left(char_pos + len_to_be_replaced)).Right(len_to_be_replaced)==strToBeReplaced)
         {
         CString first_part = origStr.Left(char_pos);
         CString last_part = origStr.Right(remainder_len - char_pos);
         str_to_return = first_part + replacementStr + last_part;
         break;
         }
      char_pos++;
      }
/*
   CString msg;
   msg.Format("ReplaceSubstring: %s, %s, %s, %s", origStr, strToBeReplaced, replacementStr, str_to_return);
   Report::LogMsg(msg);
*/
   return(str_to_return);
   } // end of ReplaceSubString()
         

bool WW2100AP::LoadXmlScalar(LPCTSTR file_name, TiXmlElement *pXmlScalar, 
       LPCSTR tgtSymbol, ModelScalar *pScalar)
   {
   bool ok = false;

   if (pXmlScalar!=NULL)
      {
      double _scalar_value = 0.;
      double _offset_value = 0.;
      LPTSTR _symbol = NULL;
      LPTSTR _description = NULL;
     
      XML_ATTR Scalar[] = {
         // attr        type            address          isReq  checkCol
         { "symbol",    TYPE_STRING,    &_symbol,         true,     0 },
         { "value",     TYPE_DOUBLE,    &_scalar_value,   true,     0 },
         { "offset_value", TYPE_DOUBLE, &_offset_value,   false,    0 },
         { "description", TYPE_STRING,  &_description,    false,    0 },
         { NULL,           TYPE_NULL,   NULL,             false,    0 } };

      ok = TiXmlGetAttributes( pXmlScalar, Scalar, file_name );

      if ( !ok || ( _symbol != NULL && (_tcscmp(_symbol, tgtSymbol) != 0)))
         {
         CString msg; 
         msg.Format( _T("WW2100AP: Misformed root element reading attributes in input file %s, symbol %s"), 
               file_name, tgtSymbol );
         Report::ErrorMsg( msg );
         return false;
         }

      pScalar->m_symbol = _symbol;
      pScalar->m_value = _scalar_value;
      pScalar->m_offset_value = _offset_value;
      pScalar->m_description = _description;
      }

   return(ok);
   } // end of LoadXmlScalar()


bool WW2100AP::LoadXmlUGA(LPCTSTR file_name, TiXmlElement *pXmlUGA)
   {
   bool ok = false;

   if (pXmlUGA!=NULL)
      {
      int _id = -1;
      LPTSTR _name = NULL;
      float _city_center_E = -1;
      float _city_center_N = -1;
      float _threshold_pct = 80.;
      int _display_flag = 0;
     
      XML_ATTR Atts[] = {
         // attr        type           address          isReq  checkCol
         { "id",        TYPE_INT,      &_id,             true,     0},
         { "name",      TYPE_STRING,   &_name,           true,     0 },
         { "city_center_E", TYPE_FLOAT,   &_city_center_E, true,     0 },
         { "city_center_N", TYPE_FLOAT,   &_city_center_N, true,     0 },
         { "threshold_pct", TYPE_FLOAT, &_threshold_pct,  false,    0 },
         { "display", TYPE_INT, &_display_flag, false, 0 },
         { NULL,           TYPE_NULL,  NULL,             false,    0 } };

      ok = TiXmlGetAttributes( pXmlUGA, Atts, file_name );
      if ( !ok || _id<0 || _id>MAX_UGA_NDX)
         {
         CString msg; 
         msg.Format( _T("WW2100AP: Misformed element reading UGA attributes in input file %s, or UGA id out of range"), 
               file_name);
         Report::ErrorMsg( msg );
         return false;
      }

      int ugb = _id;
      m_UXugaArray[ugb].id = _id;
      m_UXugaArray[ugb].name = _name;
      m_UXugaArray[ugb].city_center.x = _city_center_E;
      m_UXugaArray[ugb].city_center.y = _city_center_N;
      m_UXugaArray[ugb].threshold_pct = _threshold_pct;
      m_UXugaArray[ugb].display_flag = _display_flag;
      }

   return(ok);
   } // end of LoadXmlUGA()


bool WW2100AP::LoadXmlParameter(LPCTSTR file_name, TiXmlElement *pXmlParameter, 
      LPCSTR tgtSymbol, ModelScalar *pParameter)
   {
   bool ok;
   ok = LoadXmlScalar(file_name, pXmlParameter, tgtSymbol, pParameter);
   return(ok);
   } // end of LoadXmlParameter()


bool WW2100AP::LoadXmlCoefficient(LPCTSTR file_name, TiXmlElement *pXmlCoefficient, 
      LPCSTR tgtSymbol, ModelScalar *pCoefficient)
   {
   bool ok;
   ok = LoadXmlScalar(file_name, pXmlCoefficient, tgtSymbol, pCoefficient);
   return(ok);
   } // end of LoadXmlCoefficient()


bool WW2100AP::LoadXmlCoefficients(LPCTSTR file_name, TiXmlElement *pXmlSubProc, PtrArray< ModelScalar > *pCoefficientArray)
   {
   return(LoadXmlCoefficients(file_name, pXmlSubProc, pCoefficientArray, _T("beta")));
   } // end of LoadXmlCoefficients(3 argument form)


bool WW2100AP::LoadXmlCoefficients(LPCTSTR file_name, TiXmlElement *pXmlSubProc, PtrArray< ModelScalar > *pCoefficientArray, LPCTSTR prefix)
   {
   TiXmlElement *pXmlCoefficients = pXmlSubProc->FirstChildElement( _T( "coefficients" ) );
   if ( pXmlCoefficients == NULL )  { RequiredTagIsMissing(_T("coefficients"), file_name); return(false); };

   pCoefficientArray->Clear();

   TiXmlElement *pXmlCoefficient = pXmlCoefficients->FirstChildElement( _T("coefficient" ) );
   int coefNum = 0;
   while ( pXmlCoefficient != NULL )
      {
      ModelScalar *pCoefficient = new ModelScalar;
      CString tgtSymbol;
      tgtSymbol.Format("%s%d", prefix, coefNum);
      
      bool ok = LoadXmlCoefficient(file_name, pXmlCoefficient, tgtSymbol, pCoefficient);
      if (ok) 
         pCoefficientArray->Add( pCoefficient );
      
      else 
         delete pCoefficient;

      pXmlCoefficient = pXmlCoefficient->NextSiblingElement( _T("coefficient" ) );
      coefNum++;
      } // end of while ( pXmlCoefficient != NULL )

   return(true);
   } // end of LoadXmlCoefficients()


bool WW2100AP::LoadXmlUGAs(LPCTSTR file_name, TiXmlElement *pXmlSubProc)
   {
   TiXmlElement *pXmlUGAs = pXmlSubProc->FirstChildElement( _T( "UGAs" ) );
   if ( pXmlUGAs == NULL )  { RequiredTagIsMissing(_T("UGAs"), file_name); return(false); };

   for (int ugb=0; ugb<=MAX_UGA_NDX; ugb++)
      {
      m_UXugaArray[ugb].id = -1;
      m_UXugaArray[ugb].name = "";
      m_UXugaArray[ugb].city_center.x = -1.; 
      m_UXugaArray[ugb].city_center.y = -1.; 
      m_UXugaArray[ugb].threshold_pct = 100.f;
      } // end of loop thru ugbs

   TiXmlElement *pXmlUGA = pXmlUGAs->FirstChildElement( _T("UGA" ) );
   while ( pXmlUGA != NULL )
      {
      bool ok = LoadXmlUGA(file_name, pXmlUGA);

      pXmlUGA = pXmlUGA->NextSiblingElement( _T("UGA" ) );
      } // end of while ( pXmlUGA != NULL )

   return(true);
   } // end of LoadXmlUGAs()

