// EvapTrans.cpp : Implements global evapotransporation methods for Flow
//

#include "stdafx.h"
#pragma hdrstop

#include "GlobalMethods.h"
#include <DATE.HPP>
#include "Flow.h"
#include "FlowInterface.h"
#include <UNITCONV.H>
#include <omp.h>
#include <EnvEngine\EnvConstants.h>
#include <vector>

#include "boost\multi_array.hpp"
typedef boost::multi_array<float, 2> twoDarray;
typedef boost::multi_array<float, 3> threeDarray;
typedef twoDarray::index twoDindex;

#include <PathManager.h>


using namespace std;

extern FlowProcess *gpFlow;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define EPSILON  0.00000001
#define FP_EQUAL(a, b)             (fabs(a-b) <= EPSILON)

#define CROP_NOT_IN_HRU 1000
#define GROWING_SEASON_HASNT_STARTED 999
#define GROWING_SEASON_HASNT_ENDED -999


EvapTrans::EvapTrans( LPCTSTR name )
: GlobalMethod( name, GM_NONE )
, m_currMonth( -1 )                                               // needed only for Hargreaves reference ET calculation, could be moved to ETEquation based on doy
, m_flowContext( NULL )                                           // pointer to flowContext to inform ETEquation
, m_pCropTable( NULL )                                            // pointer to Crop table (.csv) holding crop coefficents
, m_pSoilTable( NULL )                                            // pointer to Soil table (.csv) holding soil values
, m_pStationTable( NULL )                                         // pointer to the 5 Agrimet Station coefficents for Kimberly-Penman reference ET calculation
, m_phruCropStartGrowingSeasonArray( NULL )                       // FDataObj holding the planting doy for every crop for every HRU
, m_phruCropEndGrowingSeasonArray( NULL )                         // FDataObj holding the harvest doy for every crop for every HRU
, m_phruCurrCGDDArray( NULL )                                     // FDataObj holding the cummulative Growing Degree Days for the crop since planted
, m_pLandCoverCoefficientLUT( NULL )                              // FDataObj holing the land cover coefficents representing percentile stages of growth 
//  , m_pDailyReferenceET( NULL )                                 // DEBUGGING: Uncomment to output reference ET for a particular HRU
, m_bulkStomatalResistance( 1000.0f )                             // Bulk Stomatal Resistance for Penman Monteith reference ET calculation 
, m_latitude( 45.0f )                                             // latitude needed for reference ET calculation
, m_longitude(123.0f)                                             // longitue needed for reference ET calculation
, m_pLayerDists( NULL )                                           // pointer to the soil layer distribution
, m_colLulc( -1 )                                                 // LULC_B idu layer column ("LULC_B")
, m_colArea( -1 )                                                 // Area idu layer column ("AREA")
, m_colIduSoilID( -1 )                                            // Soil ID idu layer column ("SoilID")
, m_colLAI( -1 )                                                  // Leaf Area Index idu layer column ("LAI")
, m_colLULC_A(-1)
, m_colPVT(-1)
, m_colAgeClass( -1 )                                             // Age Class idu layer column ("AGECLASS")
, m_colDailyET( -1 )                                              // Daily ET idu layer column ("ET_DAY")
, m_colDailyMaxET( -1 )                                           // Daily Maximum ET idu layer column ("MAX_ET_DAY")
, m_colSM_DAY( -1 )                                    // Daily Soil Moisture idu layer column ("SM_DAY")
, m_colDailySOILH2OEST(-1)
, m_colDailyIRR_STATE(-1)
, m_colDailyIRRRQST_D(-1)
, m_colDailyIRRACRQ_D(-1)
, m_colDailyPRECIP(-1)
, m_colIDUIndex(-1)
, m_colIrrigation(-1)                                   
, m_colAnnAvgMaxET( -1 )                                          // Annual Maximum ET idu layer column ("MAX_ET_YR")
, m_colF_THETA(-1)
, m_colVPD(-1)
, m_colVPD_SCALAR(-1)
, m_colRAD_SW(-1)
, m_colHruPV_HERE(-1)
, m_colYieldReduction( -1 )                                       // Crop Yield reduction idu layer column ("YieldReduc")
, m_colAgrimetStationID( -1 )                                     // Agrimet Station ID idu layer column, only needed for Kimberly-Penman reference ET calculation
, m_colSoilTableSoilID(-1)                                        // Soil Table ID column to match idu layer SOILID            
, m_colWP( -1 )                                                   // Wilting Point of soil ("WP")
, m_colFC( -1 )                                                   // Field Capacity of soil ("FC")
, m_colHbvET_MULT(-1)
, m_colCropTableLulc( -1 )                                        // Crop Table LULC_B column to match idu layer LULC_B
, m_cropCount( -1 )                                               // Crop count of the number of crops modeled
, m_plantingMethodCol( -1 )                                       // Planting Method (1 = T30, 2 = CGDD) crop table column  
, m_plantingThresholdCol( -1 )                                    // Threshold of either Temp or Heat Unit (dependent on Planting Method) crop table column 
, m_earliestPlantingDateCol( -1 )                                 // Earliest Planting Day of the Year crop table column 
, m_gddBaseTempCol( -1 )                                          // Growing Degree Day Base Temp crop table column
, m_termMethodCol( -1 )                                           // Harvest Method (1=CGDD, 2=Frost, 3=Happen First (1 or 2), 4=Desert Vegetation Class) crop table column
, m_gddEquationTypeCol( -1 )                                      // GDD Equation either for Corn or all other crops (1-= other crops, 2=corn) crop table column
, m_termCGDDCol( -1 )                                             // Harvest Method by CGDD crop table column
, m_termTempCol( -1 )                                             // Harvest Method by Frost crop table column
, m_binToEFCCol( -1 )                                               
, m_cGDDtoEFCCol( -1 )
, m_binEFCtoTermCol( -1 )
, m_binMethodCol( -1 )
, m_constCropCoeffEFCtoTCol( -1 )
, m_depletionFractionCol( -1 )                                    
, m_yieldResponseFactorCol( -1 )                                  // Growing Season Yield Reduction Factor crop table column
, m_precipThresholdCol( -1 )                                      // last sseven days of precipitation threshold for planting
, m_colPLANTDATE( -1 )                                      // doy (=day of year) for start of growing season in idu layer column ("PLANTDATE")
, m_colEndGrowSeason( -1 )                                        // doy (=day of year) for end of growing season in idu layer column ("HARVDATE")
, m_runCount( 0 )                                                 // run count for integration
, m_ETEq( this )                                                  // the type of ET equation to use for calculation of the reference ET (ASCE, FAO56, Penn_Mont, Kimb_Penn, Hargreaves)
, m_CO2_scalar(0.f)
, m_atmCO2conc(0.f)
, m_effBulkStomatalResistance(1.e10)
, m_TurnerScenario(0) 

, m_colTMIN(-1)
, m_colTMIN_GROW(-1)
, m_colTMINGROAVG(-1)
, m_colPRCP_GROW(-1)
, m_colPRCPSPRING(-1)
, m_colPRCPWINTER(-1)
, m_colPRCP_JUN(-1)
, m_colPRCP_JUL(-1)
, m_colPRCP_AUG(-1)
   {
   this->m_timing = GMT_CATCHMENT | GMT_START_STEP;               // Called during GetCatchmentDerivatives()
   this->m_ETEq.SetMode( ETEquation::HARGREAVES );                // default reference ET is Hargreaves
   gpFlow->AddInputVar("Turner scenario", m_TurnerScenario, "0 = Isolate soil H2O effect on ET (default), 1 = Isolate climate effect on PET, 2 = Isolate CO2 effect on bulk resistance"
      ", 3 = Isolate evaporative demand effect on AET using MIROC climate, 4 = Isolate evaporative demand effect on AET using GFDL climate, 3 = Isolate evaporative demand effect on AET using HadGEM climate");
   }


EvapTrans::~EvapTrans()
   {
   if ( m_pCropTable )
      delete m_pCropTable;

   if ( m_pSoilTable )
      delete m_pSoilTable;

   if ( m_pStationTable )
      delete m_pStationTable;

   if ( m_pLayerDists )
      delete m_pLayerDists;

   if ( m_phruCropStartGrowingSeasonArray )
      delete  m_phruCropStartGrowingSeasonArray;

   if ( m_phruCropEndGrowingSeasonArray )
      delete m_phruCropEndGrowingSeasonArray;

   if ( m_phruCurrCGDDArray )
      delete  m_phruCurrCGDDArray;

   if ( m_pLandCoverCoefficientLUT )
      delete m_pLandCoverCoefficientLUT;

   /* if ( m_pDailyReferenceET != NULL )
       delete m_pDailyReferenceET;*/
   }


bool EvapTrans::Init( FlowContext *pFlowContext )
{
   GlobalMethod::Init(pFlowContext); // compile query


   if (m_dateEvapTransLastExecuted.year != 9999)
   { // EvapTrans::Init() will be executed once for each <evap_trans> block in Flow.xml.
      // Put stuff here which only needs to be executed the first time that EvapTrans::Init() is called.

      m_dateEvapTransLastExecuted = SYSDATE(1, 1, 9999);
   } // end of if (m_dateEvapTransLastExecuted.year != 9999)

   { // We wouldn't have to do this more than once if we made all these m_... members static.
      m_pIDUlayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;
      m_pHRUlayer = (MapLayer*)pFlowContext->pEnvContext->pHRUlayer;
      m_flowContext = pFlowContext;

      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colDailyET, "ET_DAY", TYPE_FLOAT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colDailyMaxET, "MAX_ET_DAY", TYPE_FLOAT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colAnnAvgMaxET, "MAX_ET_YR", TYPE_FLOAT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colSM_DAY, "SM_DAY", TYPE_FLOAT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colDailySOILH2OEST, "SOILH2OEST", TYPE_FLOAT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colDailyIRR_STATE, "IRR_STATE", TYPE_INT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colDailyIRRRQST_D, "IRRRQST_D", TYPE_FLOAT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colDailyIRRACRQ_D, "IRRACRQ_D", TYPE_FLOAT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colDailyPRECIP, "PRECIP", TYPE_FLOAT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colIDUIndex, "IDU_ID", TYPE_INT, CC_MUST_EXIST);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colIrrigation, "IRRIGATION", TYPE_INT, CC_MUST_EXIST);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colF_THETA, "F_THETA", TYPE_FLOAT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colVPD, "VPD", TYPE_FLOAT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colVPD_SCALAR, "VPD_SCALAR", TYPE_FLOAT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colLULC_A, "LULC_A", TYPE_INT, CC_MUST_EXIST);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colRAD_SW, "RAD_SW", TYPE_FLOAT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colLAI, "LAI", TYPE_FLOAT, CC_MUST_EXIST);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colPVT, "PVT", TYPE_INT, CC_MUST_EXIST);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, m_colAgeClass, "AGECLASS", TYPE_INT, CC_MUST_EXIST);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, this->m_colPLANTDATE, "PLANTDATE", TYPE_INT, CC_AUTOADD);
      gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, this->m_colEndGrowSeason, "HARVDATE", TYPE_INT, CC_AUTOADD);

      m_pIDUlayer->CheckCol(m_colTMIN, "TMIN", TYPE_FLOAT, CC_AUTOADD);
      m_pIDUlayer->CheckCol(m_colTMIN_GROW, "TMIN_GROW", TYPE_FLOAT, CC_AUTOADD);
      m_pIDUlayer->CheckCol(m_colTMINGROAVG, "TMINGROAVG", TYPE_FLOAT, CC_AUTOADD);
      m_pIDUlayer->CheckCol(m_colPRCP_GROW, "PRCP_GROW", TYPE_FLOAT, CC_AUTOADD);
      m_pIDUlayer->CheckCol(m_colPRCPSPRING, "PRCPSPRING", TYPE_FLOAT, CC_AUTOADD);
      m_pIDUlayer->CheckCol(m_colPRCPWINTER, "PRCPWINTER", TYPE_FLOAT, CC_AUTOADD);
      m_pIDUlayer->CheckCol(m_colPRCP_JUN, "PRCP_JUN", TYPE_FLOAT, CC_AUTOADD);
      m_pIDUlayer->CheckCol(m_colPRCP_JUL, "PRCP_JUL", TYPE_FLOAT, CC_AUTOADD);
      m_pIDUlayer->CheckCol(m_colPRCP_AUG, "PRCP_AUG", TYPE_FLOAT, CC_AUTOADD);

      m_pHRUlayer->CheckCol(m_colHruPV_HERE, "PV_HERE", TYPE_INT, CC_AUTOADD);
   }

   switch (m_method)
   {
      case GM_PENMAN_MONTEITH:
      {
         m_CO2effectMetrics.SetName("EVTR CO2 Effect");
         m_CO2effectMetrics.SetSize(5, 0);
         m_CO2effectMetrics.SetLabel(0, "Year");
         m_CO2effectMetrics.SetLabel(1, "weatherYear");
         m_CO2effectMetrics.SetLabel(2, "atm CO2 conc (ppm)");
         m_CO2effectMetrics.SetLabel(3, "CO2_factor");
         m_CO2effectMetrics.SetLabel(4, "effective bulk stomatal resistance (s per m)");
         gpFlow->AddOutputVar("EVTR CO2 effect", &m_CO2effectMetrics, "EVTR CO2 effect");
      }
      break; // end of case GM_PENMAN_MONTEITH

      case GM_FAO56:
      {
         // Create and initialize the planting and harvest doy and cumulative gorwing degree days since planting lookup tables
         int hruCount = pFlowContext->pFlowModel->GetHRUCount();
         m_phruCropStartGrowingSeasonArray = new IDataObj(hruCount, max(0, m_cropCount), CROP_NOT_IN_HRU);
         m_phruCropEndGrowingSeasonArray = new IDataObj(hruCount, max(0, m_cropCount), CROP_NOT_IN_HRU);
         m_phruCurrCGDDArray = new FDataObj(hruCount, max(0, m_cropCount), 0.0f);
      }
      break; // end of case GM_FAO56

      case GM_WETLAND_ET:
         break; // end of case GM_WETLAND_ET

      default: ASSERT(0); 
         break;
   }

   return true;
} // end of EvapTrans::Init()


bool EvapTrans::InitRun( FlowContext *pFlowContext )
{
   GlobalMethod::InitRun( pFlowContext );

   // Note: At InitRun time, pEnvContext->currentYear is set to the calendar year before the first calendar year of the simulation.
   if (m_dateEvapTransLastExecuted.year != pFlowContext->pEnvContext->currentYear)
   { // In each simulation run, EvapTrans::InitRun() will be executed once for each <evap_trans> block in Flow.xml.
      // Put stuff here which only needs to be executed the first time that EvapTrans::InitRun() is called in a simulation run.

      m_dateEvapTransLastExecuted = SYSDATE(1, 1, pFlowContext->pEnvContext->currentYear);
   } // end of if (m_dateEvapTransLastExecuted.year != pFlowContext->pEnvContext->currentYear)

   switch (m_method)
   {
      case GM_PENMAN_MONTEITH:
      {
         m_CO2effectMetrics.ClearRows();
      }
      break; // end of case GM_PENMAN_MONTEITH

      case GM_FAO56:
      {
         int hruCount = pFlowContext->pFlowModel->GetHRUCount();

         // Create and initialize the planting and harvest doy and cumulative gorwing degree days since planting lookup tables
         m_phruCropStartGrowingSeasonArray = new IDataObj(hruCount, max(0, m_cropCount), CROP_NOT_IN_HRU);
         m_phruCropEndGrowingSeasonArray = new IDataObj(hruCount, max(0, m_cropCount), CROP_NOT_IN_HRU);
         m_phruCurrCGDDArray = new FDataObj(hruCount, max(0, m_cropCount), 0.0f);
      }
      break; // end of case GM_FAO56

      case GM_WETLAND_ET:
         break; // end of case GM_WETLAND_ET

      default: ASSERT(0);
         break;
   }

   // m_pDailyReferenceET->ClearRows();

   //run dependent setup
   m_currMonth = -1;

   return(true);
} // end of EvapTrans::InitRun()


bool EvapTrans::StartYear( FlowContext *pFlowContext )
   {
   GlobalMethod::StartYear( pFlowContext );  

   if (m_dateEvapTransLastExecuted.year != pFlowContext->pEnvContext->currentYear)
   { // In each simulation year, EvapTrans::StartYear() will be executed once for each <evap_trans> block in Flow.xml.
      // Put stuff here which only needs to be executed the first time that EvapTrans::StartYear() is called in a simulation year.

      m_dateEvapTransLastExecuted = SYSDATE(1, 1, pFlowContext->pEnvContext->currentYear);
   } // end of if (m_dateEvapTransLastExecuted.year != pFlowContext->pEnvContext->currentYear)

   FlowModel *pModel = pFlowContext->pFlowModel;
   MapLayer  *pIDUlayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

   int iduCount = (int)pFlowContext->pEnvContext->pMapLayer->GetRecordCount();
   m_flowContext = pFlowContext;
   m_runCount = 0;

   switch (m_method)
   {
      case GM_PENMAN_MONTEITH:
      {
         for (int i = 0; i < iduCount; i++)
         {
            m_iduIrrRequestArray[i] = 0.0f;
            m_iduAnnAvgMaxETArray[i] = 0.0f;
            m_iduSeasonalAvgETArray[i] = 0.0f;
            m_iduSeasonalAvgMaxETArray[i] = 0.0f;
            if ( !pModel->m_estimateParameters )
            {
               gpFlow->UpdateIDU( pFlowContext->pEnvContext, i, m_colAnnAvgMaxET, m_iduAnnAvgMaxETArray[ i ], true );   // mm/year
            }
         } // end of loop thru IDUs
      }
      { // For the Penman-Monteith method, calculate effective bulk stomatal resistance as a function of atmospheric CO2 concentration.
         float CO2_factor = m_TurnerScenario == 2 ? 1.f : 0.f; // Proportional increase in the base resistance over the range of CO2 influence.
         float CO2min = 400.f; // parts per million
         float CO2max = 800.f; // parts per million
         m_atmCO2conc = CO2byYear(pFlowContext->pEnvContext->weatherYear, pFlowContext->pFlowModel->GetScenarioName());
         m_CO2_scalar = (m_atmCO2conc - CO2min) / (CO2max - CO2min);
         if (m_CO2_scalar < 0.f) m_CO2_scalar = 0.f;
         else if (m_CO2_scalar > 1.f) m_CO2_scalar = 1.f;
         m_effBulkStomatalResistance = (float)(m_bulkStomatalResistance * (1.f + m_CO2_scalar * CO2_factor));

         CString msg;
         msg.Format("*** EvapTrans::StartYear() weatherYear = %d, scenarioName = %s, m_TurnerScenario = %d, m_atmCO2conc = %f, CO2_factor = %f, m_CO2_scalar = %f, m_effBulkStomatalResistance = %f",
            pFlowContext->pEnvContext->weatherYear, (pFlowContext->pFlowModel->GetScenarioName()).GetString(), m_TurnerScenario, m_atmCO2conc, CO2_factor, m_CO2_scalar, m_effBulkStomatalResistance);
         Report::LogMsg(msg);
      }
      break; // end of case GM_PENMAN_MONTEITH

      case GM_FAO56:
      {
         ASSERT(m_pCropTable != NULL);
         FillLookupTables(pFlowContext);
    
         // write planting dates to map
         for ( int h = 0; h < pModel->GetHRUCount(); h++ )
         {
            HRU *pHRU = pModel->GetHRU(h);

            for ( int i = 0; i < (int)pHRU->m_polyIndexArray.GetSize(); i++ )
            {
               int idu = pHRU->m_polyIndexArray[ i ];

               int lulc = -1;
               if ( pIDUlayer->GetData( idu, m_colLulc, lulc ) )
               {
                  // have lulc for this IDU, find the corresponding row in the cc table
                  int row = m_pCropTable->Find( m_colCropTableLulc, VData( lulc ), 0 );

                  int startDate = GROWING_SEASON_HASNT_STARTED;
                  int harvestDate = GROWING_SEASON_HASNT_ENDED;
                  if ( row >= 0 )
                  {
                     startDate = this->m_phruCropStartGrowingSeasonArray->Get( h, row );
                     harvestDate = this->m_phruCropEndGrowingSeasonArray->Get( h, row );
                  }

                  if ( !pModel->m_estimateParameters )
                  {
                      gpFlow->UpdateIDU( pFlowContext->pEnvContext, idu, m_colPLANTDATE, startDate, true );
                      gpFlow->UpdateIDU( pFlowContext->pEnvContext, idu, m_colEndGrowSeason, harvestDate, true );
                  }
               }
            } // end of loop thru IDUs in one HRU
         } // end of loop thru HRUs
      }
      break; // end of case GM_FAO56

      case GM_WETLAND_ET:
      { // For the Penman-Monteith method, calculate effective bulk stomatal resistance as a function of atmospheric CO2 concentration.
         float CO2_factor = m_TurnerScenario == 2 ? 1.f : 0.f; // Proportional increase in the base resistance over the range of CO2 influence.
         float CO2min = 400.f; // parts per million
         float CO2max = 800.f; // parts per million
         m_atmCO2conc = CO2byYear(pFlowContext->pEnvContext->weatherYear, pFlowContext->pFlowModel->GetScenarioName());
         m_CO2_scalar = (m_atmCO2conc - CO2min) / (CO2max - CO2min);
         if (m_CO2_scalar < 0.f) m_CO2_scalar = 0.f;
         else if (m_CO2_scalar > 1.f) m_CO2_scalar = 1.f;
            m_effBulkStomatalResistance = (float)(m_bulkStomatalResistance * (1.f + m_CO2_scalar * CO2_factor));

         CString msg;
         msg.Format("*** EvapTrans::StartYear() weatherYear = %d, scenarioName = %s, m_TurnerScenario = %d, m_atmCO2conc = %f, CO2_factor = %f, m_CO2_scalar = %f, m_effBulkStomatalResistance = %f",
               pFlowContext->pEnvContext->weatherYear, (pFlowContext->pFlowModel->GetScenarioName()).GetString(), m_TurnerScenario, m_atmCO2conc, CO2_factor, m_CO2_scalar, m_effBulkStomatalResistance);
         Report::LogMsg(msg);
      }
      break; // end of case GM_WETLAND_ET

      default: ASSERT(0);
         break;
   } // end of switch (m_method)

   return(true);
} // end of EvapTrans::StartYear()


bool EvapTrans::StartStep(FlowContext *pFlowContext)
{
   if (m_dateEvapTransLastExecuted.month != pFlowContext->pEnvContext->m_simDate.month || m_dateEvapTransLastExecuted.day != pFlowContext->pEnvContext->m_simDate.day)
   { // In each simulation day, EvapTrans::StartStep() will be executed once for each <evap_trans> block in Flow.xml.
      // Put stuff here which only needs to be executed the first time that EvapTrans::StartStep() is called in a simulation day.

      // Zero out the ET_DAY IDU attribute.
      int iduCount = (int)pFlowContext->pEnvContext->pMapLayer->GetRecordCount();
      for (int i = 0; i < iduCount; i++) gpFlow->UpdateIDU(pFlowContext->pEnvContext, i, m_colDailyET, 0, false);

      m_dateEvapTransLastExecuted.month = pFlowContext->pEnvContext->m_simDate.month;
      m_dateEvapTransLastExecuted.day = pFlowContext->pEnvContext->m_simDate.day;
   } // end of if (m_dateEvapTransLastExecuted.month != pFlowContext->pEnvContext->m_simDate.month || m_dateEvapTransLastExecuted.day != pFlowContext->pEnvContext->m_simDate.day)

   switch (m_method)
   {
      case GM_PENMAN_MONTEITH:
      break; // end of case GM_PENMAN_MONTEITH

      case GM_FAO56:
      {
         ASSERT(m_pCropTable != NULL);
         // populate the Crop Table with begining of step values:
         int hruCount = pFlowContext->pFlowModel->GetHRUCount();
         int doy = pFlowContext->dayOfYear;

         // iterate through HRUs, setting values for each hru X crop for the cummulative degree
         // days since planted (m_phruCurrCGDDArray)
         for ( int hruIndex = 0; hruIndex < hruCount; hruIndex++ )
         {
            HRU *pHRU = pFlowContext->pFlowModel->GetHRU(hruIndex);

            // iterate through crops, filling cGDD array with the cumulative growing degree days since planted
            for ( int row = 0; row < m_cropCount; row++ )
            {
               float baseTemp = m_pCropTable->GetAsFloat( this->m_gddBaseTempCol, row );
               int gddEquationType = m_pCropTable->GetAsInt( this->m_gddEquationTypeCol, row );
               int useGDD = m_pCropTable->GetAsInt( this->m_useGDDCol, row );

               bool isCorn = ( gddEquationType == 1 ) ? false : true;
               bool isGDD = ( useGDD == 1 ) ? true : false;

               // if doy is within growing season
               int plantingDoy = m_phruCropStartGrowingSeasonArray->Get( hruIndex, row );
               int harvestDoy = m_phruCropEndGrowingSeasonArray->Get( hruIndex, row );

               if ( doy >= plantingDoy && doy <= harvestDoy )
               {
                  if (isGDD)
                  {
                     float cGDD = m_phruCurrCGDDArray->Get( hruIndex, row );
                     CalculateCGDD( pFlowContext, pHRU, baseTemp, doy, isCorn, cGDD );
                     m_phruCurrCGDDArray->Set( hruIndex, row, cGDD );
                  }
                  else m_phruCurrCGDDArray->Set( hruIndex, row, -1 );
               }  // end doy in growing season
            }  // end row 
         } // end hru 
      } 
      break; // end of case GM_FAO56

      case GM_WETLAND_ET:
         break;

      default: ASSERT(0);
         break;
   } // end of switch(m_method)

   return true;
} // end of EvapTrans::StartStep(FlowContext *pFlowContext)


bool EvapTrans::Step( FlowContext *pFlowContext )
{
   if ( GlobalMethod::Step( pFlowContext ) == true )
      return true;

   if ( pFlowContext == NULL )
      return false;

   // In each simulation day, EvapTrans::Step() will be executed once for each <evap_trans> block in Flow.xml.
   // If there were stuff which should only be executed the first time Step() is called in a simulation day,
   // we would have to add a flag to keep track of whether or not Step() had been called in the given simulation day.

   int hruCount = pFlowContext->pFlowModel->GetHRUCount();

   // iterate through hrus/hrulayers 
   //  #pragma omp parallel for 
   //  int idusProcessed = 0;
   for ( int h = 0; h < hruCount; h++ )
      {
      HRU *pHRU = pFlowContext->pFlowModel->GetHRU( h );
      int doy = pFlowContext->dayOfYear;

      if ( DoesHRUPassQuery( h ) )
         {
         GetHruET( pFlowContext, pHRU, h );                                      
         }
      }

   m_runCount++;

   //CString msg;
   //msg.Format( "EvapTrans:Run() '%s' processed %i IDUs", this->m_name, idusProcessed );
   //Report::LogMsg( msg );
   return TRUE;
} // end of EvapTrans::Step()


bool EvapTrans::EndStep( FlowContext *pFlowContext )
   {
   return true;
   }


bool EvapTrans::EndYear( FlowContext *pFlowContext )
   {
   FlowModel *pModel = pFlowContext->pFlowModel;
   MapLayer  *pIDUlayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

   int iduCount = (int)pFlowContext->pEnvContext->pMapLayer->GetRecordCount();


   // write annual ET and Yield Reduction fraction to map
   for ( int i = 0; i < iduCount; i++ )
      {
      if ( !pModel->m_estimateParameters )
         {
           gpFlow->UpdateIDU( pFlowContext->pEnvContext, i, m_colAnnAvgMaxET, m_iduAnnAvgMaxETArray[ i ], true );   // mm/year

         float yieldFraction = 1.0f;

         // output yield reduction ratio values to map if it is a crop
         if ( m_pCropTable )
            {
            // get lulc id from HRU
            int lulc = -1;
            int row = -1;
            if ( pIDUlayer->GetData( i, m_colLulc, lulc ) )
               {
               // have lulc for this HRU, find the corresponding row in the cc table
               row = m_pCropTable->Find( m_colCropTableLulc, VData( lulc ), 0 );
               if ( row >= 0 && m_yieldResponseFactorCol >= 0 && m_iduSeasonalAvgMaxETArray[ i ] > 0.0f )
                  {
                  float ratio = m_iduSeasonalAvgETArray[ i ] / m_iduSeasonalAvgMaxETArray[ i ];
                  float ky = m_pCropTable->GetAsFloat( this->m_yieldResponseFactorCol, row );
                  yieldFraction = 1.0f - ( ky * ( 1.0f - ratio ) );   // (1 - Ya/Ym)
                  }  // end 
               }   // end lulc match
            }  // end cropTable

         if ( yieldFraction < 0.0f ) yieldFraction = 1.0f;
         if ( yieldFraction > 1.0f ) yieldFraction = 0.0f;
         if ( m_colYieldReduction > 0 ) gpFlow->UpdateIDU( pFlowContext->pEnvContext, i, m_colYieldReduction, yieldFraction, true );     // fraction
         }
      }

   CArray< float, float> rowCO2effectMetrics;
   rowCO2effectMetrics.SetSize(5);
   rowCO2effectMetrics[0] = (float)pFlowContext->pEnvContext->currentYear;
   rowCO2effectMetrics[1] = (float)pFlowContext->pEnvContext->weatherYear;
   rowCO2effectMetrics[2] = m_atmCO2conc;
   rowCO2effectMetrics[3] = m_CO2_scalar;
   rowCO2effectMetrics[4] = m_effBulkStomatalResistance;
   m_CO2effectMetrics.AppendRow(rowCO2effectMetrics);

   if (pFlowContext->pEnvContext->startYear + pFlowContext->pEnvContext->yearOfRun == pFlowContext->pEnvContext->endYear)
   { // This is the end of the run.  Set the input variables back to their default values.
      m_TurnerScenario = 0; 
      m_dateEvapTransLastExecuted = SYSDATE(1, 1, 9999);
   }

   return true;
   } // end of EndYear()


bool EvapTrans::SetMethod( GM_METHOD method )
   {
   m_method = method;

   switch ( method )
      {
      case GM_ASCE:                m_ETEq.SetMode( ETEquation::ASCE );        break;
      case GM_FAO56:               m_ETEq.SetMode( ETEquation::FAO56 );       break;
      case GM_HARGREAVES:          m_ETEq.SetMode( ETEquation::HARGREAVES );  break;
      case GM_PENMAN_MONTEITH:     m_ETEq.SetMode( ETEquation::PENMAN_MONTEITH );   break;
      case GM_WETLAND_ET:          m_ETEq.SetMode(ETEquation::WETLAND_ET); break;
      default:
         return false;
      }

   return true;
   }


float EvapTrans::UpdateCGDD(float cgddIn, bool isCorn, float tMax, float tMin, float tMean, float baseTemp)
{
   float cGDD = cgddIn;
   float val1 = 0.0f;
   float val2 = 0.0f;

   if (cGDD < 0.0f) cGDD = 0.0f;

   if (isCorn)
   {
      val1 = min(tMax, 30);
      val2 = min(tMin, 30);
      cGDD += (max(val1, 10.0f) + max(val2, 10.0f)) / 2.0f - 10.0f;
   }
   else cGDD += max((tMean - baseTemp), 0.0f);

   return(cGDD);
} // end of UpdateCGDD()


// Returns a cumulative GDD for a single day
void EvapTrans::CalculateCGDD( FlowContext *pFlowContext, HRU *pHRU, float baseTemp, int doy, bool isCorn, float &cGDD )
   {

   float tMean = 0.0f;
   float tMin = 0.0f;
   float tMax = 0.0f;
   float val1 = 0.0f;
   float val2 = 0.0f;

   if ( cGDD < 0.0f )
      {
      cGDD = 0.0f;
      }

   // determine cGDD for corn
   if ( isCorn )
      {
      pFlowContext->pFlowModel->GetHRUClimate( CDT_TMAX, pHRU, doy, tMax );
      pFlowContext->pFlowModel->GetHRUClimate( CDT_TMIN, pHRU, doy, tMin );
      val1 = min( tMax, 30 );
      val2 = min( tMin, 30 );

      cGDD += ( max( val1, 10.0f ) + max( val2, 10.0f ) ) / 2.0f - 10.0f;
      }
   // determine cGDD for crops other than corn
   else
      {
      pFlowContext->pFlowModel->GetHRUClimate( CDT_TMEAN, pHRU, doy, tMean );
      cGDD += max( ( tMean - baseTemp ), 0.0f );
      }

   // Make sure today's weather is loaded into the IDU and HRU layers.
   pFlowContext->pFlowModel->GetTodaysWeatherField(CDT_TMAX);
   pFlowContext->pFlowModel->GetTodaysWeatherField(CDT_TMIN);
   pFlowContext->pFlowModel->GetTodaysWeatherField(CDT_TMEAN);

   } // end of CalculateCGDD( FlowContext *pFlowContext, HRU *pHRU, float baseTemp, int doy, bool isCorn, float &cGDD )

// Determines the Planting and Harvest Dates as zero-based day of the year for all crops for all HRUS
bool EvapTrans::FillLookupTables( FlowContext* pFlowContext ) // determine planting and harvest dates for every possible crop in every HRU
{ // Called from EvapTrans::StartYear()
   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      float tmin_grow; m_pIDUlayer->GetData(idu, m_colTMIN_GROW, tmin_grow);
      if (!IDU_IN_AG_BASIN(tmin_grow)) continue;
      if (INITIALIZE_EXP_AVG(tmin_grow)) m_pIDUlayer->SetDataU(idu, m_colTMINGROAVG, TOKEN_INIT_EXP_AVG); // This IDU just transitioned to ag.
      m_pIDUlayer->SetDataU(idu, m_colTMIN_GROW, 0);
      m_pIDUlayer->SetDataU(idu, m_colPRCP_GROW, 0);
      m_pIDUlayer->SetDataU(idu, m_colPRCPSPRING, 0);
      m_pIDUlayer->SetDataU(idu, m_colPRCPWINTER, 0);
      m_pIDUlayer->SetDataU(idu, m_colPRCP_JUN, 0);
      m_pIDUlayer->SetDataU(idu, m_colPRCP_JUL, 0);
      m_pIDUlayer->SetDataU(idu, m_colPRCP_AUG, 0);
   } // end of loop through IDUs

   if ( !m_pCropTable ) return(false);
      
   MapLayer *pHRUlayer = (MapLayer *)pFlowContext->pEnvContext->pHRUlayer;
   int hruCount = pFlowContext->pFlowModel->GetHRUCount();

   // reset LUTs annually
   for ( int hru_ndx = 0; hru_ndx < hruCount; hru_ndx++ )
   {
      for ( int j = 0; j < m_cropCount; j++ )
      {
         m_phruCropStartGrowingSeasonArray->Set( hru_ndx, j, CROP_NOT_IN_HRU);
         m_phruCropEndGrowingSeasonArray->Set( hru_ndx, j, CROP_NOT_IN_HRU);
         m_phruCurrCGDDArray->Set( hru_ndx, j, 0.0f );
      }

      HRU *pHRU = pFlowContext->pFlowModel->GetHRU(hru_ndx);
      for (int i = 0; i < (int)pHRU->m_polyIndexArray.GetSize(); i++)
      {
         int idu = pHRU->m_polyIndexArray[i];
         int lulc = -1; m_pIDUlayer->GetData(idu, m_colLulc, lulc);
         int row = m_pCropTable->Find(m_colCropTableLulc, VData(lulc), 0);
         if (row >= 0) 
         { // A crop is to be grown in this IDU.
            m_phruCropStartGrowingSeasonArray->Set(hru_ndx, row, GROWING_SEASON_HASNT_STARTED);
            m_phruCropEndGrowingSeasonArray->Set(hru_ndx, row, GROWING_SEASON_HASNT_ENDED);
         }
      } // end of loop thru IDUs for this HRU
   } // end of loop thru HRUs

   twoDarray p7_accum(boost::extents[6][hruCount]);
   twoDarray t30_accum(boost::extents[29][hruCount]);
   twoDarray cgdd(boost::extents[m_cropCount][hruCount]);
   CArray<int, int> crops_in_hru; crops_in_hru.SetSize(hruCount);

   for (int hru_ndx = 0; hru_ndx < hruCount; hru_ndx++)
   {
      crops_in_hru[hru_ndx] = 0;
      for (int crop_ndx = 0; crop_ndx < m_cropCount; crop_ndx++) if (m_phruCropStartGrowingSeasonArray->Get(hru_ndx, crop_ndx) != CROP_NOT_IN_HRU) crops_in_hru[hru_ndx]++;
   }

   int num_hrus_planted = 0;
   int num_hrus_harvested = 0;
   int earliest_planting_doy = 366;
   int earliest_harvest_doy = 366;
   int doy = -1;

   for (doy = 0; doy < pFlowContext->pEnvContext->daysInCurrentYear; doy++)
      {
      pFlowContext->pFlowModel->GetCurrentYearDailyWeatherField(CDT_PRECIP, doy);
      pFlowContext->pFlowModel->GetCurrentYearDailyWeatherField(CDT_TMIN, doy);

      pFlowContext->pFlowModel->DailyUpdateToSingleYearWeatherAverages(doy, pFlowContext->pEnvContext->daysInCurrentYear, pFlowContext->pEnvContext->weatherYear);

      if (num_hrus_harvested >= hruCount) continue;

      if (num_hrus_planted < hruCount) num_hrus_planted = 0;
      num_hrus_harvested = 0;
      for (int hruIndex = 0; hruIndex < hruCount; hruIndex++)
      {
         HRU *pHRU = pFlowContext->pFlowModel->GetHRU(hruIndex);

         float precip, tmax, tmin, tmean;
         pFlowContext->pFlowModel->GetHRUClimate(CDT_PRECIP, pHRU, doy, precip);
         pFlowContext->pFlowModel->GetHRUClimate(CDT_TMAX, pHRU, doy, tmax);
         pFlowContext->pFlowModel->GetHRUClimate(CDT_TMIN, pHRU, doy, tmin);
         pFlowContext->pFlowModel->GetHRUClimate(CDT_TMEAN, pHRU, doy, tmean);

         float p7 = -1.f;
         if (doy == 0)  p7 = p7_accum[0][hruIndex] = precip;
         else
         {
            int num_days = min(7, doy + 1);
            float p_accum = precip;
            for (int i = 0; i < num_days - 1; i++) p_accum += p7_accum[i][hruIndex];
            p7 = p_accum / num_days;
            for (int i = num_days - 2; i > 0; i--) p7_accum[i][hruIndex] = p7_accum[i - 1][hruIndex];
            p7_accum[0][hruIndex] = precip;
         }

         float t30 = -273.f;
         if (doy == 0)  t30 = t30_accum[0][hruIndex] = tmean;
         else
         {
            int num_days = min(30, doy + 1);
            float t_accum = tmean;
            for (int i = 0; i < num_days - 1; i++) t_accum += t30_accum[i][hruIndex];
            t30 = t_accum / num_days;
            for (int i = num_days - 2; i > 0; i--) t30_accum[i][hruIndex] = t30_accum[i - 1][hruIndex];
            t30_accum[0][hruIndex] = tmean;
         }

         if (num_hrus_planted < hruCount) // Are there any hrus for which all the planting dates haven't been found yet?
         { // Yes. This might be one of them. Try to fill in any missing planting dates for this HRU
            int num_crops_planted = 0;
            for (int row = 0; (num_crops_planted < crops_in_hru[hruIndex]) && (row < m_cropCount); row++) if (m_phruCropStartGrowingSeasonArray->Get(hruIndex, row) != CROP_NOT_IN_HRU)
            {
               if (m_phruCropStartGrowingSeasonArray->Get(hruIndex, row) == CROP_NOT_IN_HRU) continue;
               if (m_phruCropStartGrowingSeasonArray->Get(hruIndex, row) != GROWING_SEASON_HASNT_STARTED) num_crops_planted++;
               else
               { // try to fill in missing planting date for this crop
                  float baseTemp = m_pCropTable->GetAsFloat(this->m_gddBaseTempCol, row);
                  int gddEquationType = m_pCropTable->GetAsInt(this->m_gddEquationTypeCol, row);
                  bool isCorn = (gddEquationType == 1) ? false : true;
                  float cgdd_prev = doy >= 1 ? cgdd[row][hruIndex] : 0.f;
                  float cgdd_now = UpdateCGDD(cgdd_prev, isCorn, tmax, tmin, tmean, baseTemp);
                  cgdd[row][hruIndex] = cgdd_now;

                  int plantingMethod = m_pCropTable->GetAsInt(this->m_plantingMethodCol, row);
                  CString msg;
                  float plantingThreshold = m_pCropTable->GetAsFloat(this->m_plantingThresholdCol, row);
                  int earliestPlantingDate = m_pCropTable->GetAsInt(this->m_earliestPlantingDateCol, row);
                  float precipThreshold = (m_precipThresholdCol >= 0) ? m_pCropTable->GetAsFloat(this->m_precipThresholdCol, row) : 1000.f;
                  switch (plantingMethod)
                  {
                     case 1:
                        {
                           int t30Baseline = m_pCropTable->GetAsInt(this->m_t30BaselineCol, row);
                           if (t30 >= plantingThreshold  && doy >= earliestPlantingDate && p7 <= precipThreshold)
                           {
                              // put planting date into lookup table
                              m_phruCropStartGrowingSeasonArray->Set(hruIndex, row, doy - t30Baseline); num_crops_planted++;
                              if (doy < earliest_planting_doy)
                                 earliest_planting_doy = doy;
                           }
                        }
                        break;
                     case 2:
                        if (cgdd[row][hruIndex] >= plantingThreshold && doy >= earliestPlantingDate && p7 <= precipThreshold)
                        {
                           // put planting date into lookup table
                           m_phruCropStartGrowingSeasonArray->Set(hruIndex, row, doy); num_crops_planted++;
                           if (doy < earliest_planting_doy)
                              earliest_planting_doy = doy;
                        }
                        break;
                     default:
                        msg.Format("EvapTrans::FillLookupTables() Unrecognized planting method. doy = %d, hruIndex = %d, row = %d, plantingMethod = %d", doy, hruIndex, row, plantingMethod);
                        Report::ErrorMsg(msg);
                        return(false);
                  } // end of switch(plantingMethod)
               } // end of if (...) else ...
               if (num_crops_planted >= crops_in_hru[hruIndex]) num_hrus_planted++;
            } // end of loop thru crops 
         } // end of if (num_hrus_planted < hruCount)

         if (num_hrus_planted <= 0) continue; // If nothing has been planted yet, there is no need to look for harvest dates.

         // Try to fill in any missing harvest dates for this hru
         int num_crops_harvested = 0;
         for (int row = 0; (num_crops_harvested < crops_in_hru[hruIndex]) && (row < m_cropCount); row++)
         {
            if (m_phruCropEndGrowingSeasonArray->Get(hruIndex, row) == CROP_NOT_IN_HRU) continue;
            if (m_phruCropEndGrowingSeasonArray->Get(hruIndex, row) != GROWING_SEASON_HASNT_ENDED) num_crops_harvested++; // Already harvested.
            else
            {
               int plantingDoy = m_phruCropStartGrowingSeasonArray->Get(hruIndex, row);
               if (plantingDoy == doy) continue; // If just planted today, then it is too soon to harvest.
               float harvestCGDDThreshold = 10000.0F;
               float killingFrostThreshold = -273.0F;
               float baseTemp = m_pCropTable->GetAsFloat(this->m_gddBaseTempCol, row);
               int termMethod = m_pCropTable->GetAsInt(this->m_termMethodCol, row);
               int gddEquationType = m_pCropTable->GetAsInt(this->m_gddEquationTypeCol, row);
               bool isCorn = (gddEquationType == 1) ? false : true;
               cgdd[row][hruIndex] = UpdateCGDD(doy > plantingDoy ? cgdd[row][hruIndex] : 0.f, isCorn, tmax, tmin, tmean, baseTemp);

               // determine harvest date
               switch (termMethod)
               {
               case 1:
               {
                  harvestCGDDThreshold = m_pCropTable->GetAsFloat(this->m_termCGDDCol, row);
                  if (cgdd[row][hruIndex] >= harvestCGDDThreshold)
                  {
                     m_phruCropEndGrowingSeasonArray->Set(hruIndex, row, doy); num_crops_harvested++;
                     if (doy < earliest_harvest_doy) earliest_harvest_doy = doy;
                  }
               }
               break;

               case 2:
               {
                  killingFrostThreshold = m_pCropTable->GetAsFloat(this->m_termTempCol, row);
                  int minGrowingSeason = m_pCropTable->GetAsInt(this->m_minGrowSeasonCol, row);
                  if (doy - plantingDoy > minGrowingSeason)
                  {
                     if (tmin <= killingFrostThreshold)
                     {
                        m_phruCropEndGrowingSeasonArray->Set(hruIndex, row, doy); num_crops_harvested++;
                        if (doy < earliest_harvest_doy) earliest_harvest_doy = doy;
                     }
                  }
               }
               break;

               case 3:
               {
                  harvestCGDDThreshold = m_pCropTable->GetAsFloat(this->m_termCGDDCol, row);
                  killingFrostThreshold = m_pCropTable->GetAsFloat(this->m_termTempCol, row);
                  int minGrowingSeason = m_pCropTable->GetAsInt(this->m_minGrowSeasonCol, row);

                  if (doy - plantingDoy > minGrowingSeason)
                  {
                     if (cgdd[row][hruIndex] >= harvestCGDDThreshold || tmin <= killingFrostThreshold)
                     {
                        m_phruCropEndGrowingSeasonArray->Set(hruIndex, row, doy); num_crops_harvested++;
                        if (doy < earliest_harvest_doy) earliest_harvest_doy = doy;
                     }
                  }
               }
               break;

               case 4:
               {
                  // special case harvestDate = July 15 + (July 15 - plantingDate)
                  int harvest_doy = 196 + (196 - m_phruCropStartGrowingSeasonArray->Get(hruIndex, row));
                  m_phruCropEndGrowingSeasonArray->Set(hruIndex, row, harvest_doy); num_crops_harvested++;
                  if (harvest_doy < earliest_harvest_doy) earliest_harvest_doy = doy;
               }
               break;

               default:
                  m_phruCropEndGrowingSeasonArray->Set(hruIndex, row, pFlowContext->pEnvContext->daysInCurrentYear - 1); num_crops_harvested++;
                  break;
               } // end of switch(termMethod)
            } // end of if (harvest date found previously) ... else ...
         } // end of loop thru crops to find any missing harvest dates

         if (doy == (pFlowContext->pEnvContext->daysInCurrentYear - 1)) for (int row = 0; row < m_cropCount; row++) if (m_phruCropEndGrowingSeasonArray->Get(hruIndex, row) == GROWING_SEASON_HASNT_ENDED)
         { // It is the last day of the year and conditions for harvest were never satisfied.
            // Force harvest date to first day of next year.
            m_phruCropEndGrowingSeasonArray->Set(hruIndex, row, doy + 1); num_crops_harvested++; 
            if (m_phruCropStartGrowingSeasonArray->Get(hruIndex, row) == GROWING_SEASON_HASNT_STARTED)
            { // Crop never got planted at all.  Force planting date to first day of next year
               m_phruCropStartGrowingSeasonArray->Set(hruIndex, row, doy + 1);
            }
         }
         if (num_crops_harvested >= crops_in_hru[hruIndex]) num_hrus_harvested++;

      } // end of loop thru hrus

   } // end of for (int doy = 0; doy < pFlowContext->pEnvContext->daysInCurrentYear; doy++) 
   CString msg;
   msg.Format("EvapTrans::FillLookupTables() earliest_planting_doy = %d, earliest_harvest_doy = %d, final doy = %d", 
      earliest_planting_doy, earliest_harvest_doy, doy);
   Report::LogMsg(msg);

   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      float tmin_grow; m_pIDUlayer->GetData(idu, m_colTMIN_GROW, tmin_grow);
      if (IDU_IN_AG_BASIN(tmin_grow))
            m_pIDUlayer->SetDataU(idu, m_colTMIN_GROW, tmin_grow / 214.f); // There are 214 days in Apr-Oct.
   } // end of loop through IDUs

   int tgt_hru_id = 3765; // SE of Corvallis airport?
   int hru_index = -999; hru_index = pHRUlayer->FindIndex(pFlowContext->pFlowModel->m_colhruHRU_ID, tgt_hru_id);
   msg.Format("EvapTrans::FillLookupTables() for HRU_ID = %d, hru_index = %d", tgt_hru_id, hru_index);
   Report::LogMsg(msg);
   if (hru_index >= 0) for (int row = 0; row < m_cropCount; row++)
   {
      msg.Format("row %d: planting_doy = %d, harvest_doy = %d",
      row, m_phruCropStartGrowingSeasonArray->Get(hru_index, row), m_phruCropEndGrowingSeasonArray->Get(hru_index, row));
      Report::LogMsg(msg);
   }

   // Make sure the weather for January 1st is loaded into the IDU and HRU layers.
   pFlowContext->pFlowModel->GetDailyWeatherField(CDT_TMAX, 0, pFlowContext->pEnvContext->weatherYear);
   pFlowContext->pFlowModel->GetDailyWeatherField(CDT_TMIN, 0, pFlowContext->pEnvContext->weatherYear);
   pFlowContext->pFlowModel->GetDailyWeatherField(CDT_TMEAN, 0, pFlowContext->pEnvContext->weatherYear);
   pFlowContext->pFlowModel->GetDailyWeatherField(CDT_PRECIP, 0, pFlowContext->pEnvContext->weatherYear);

   pFlowContext->pFlowModel->UpdateIDUclimateTemporalAverages(pFlowContext->pEnvContext->m_yearsInStartingClimateAverages, pFlowContext->pEnvContext);
   pFlowContext->pFlowModel->UpdateAgBasinClimateTemporalAverages(pFlowContext->pEnvContext);

   return true;

   } // end of FillLookupTables()


void EvapTrans::LookupLandCoverCoefficient( int row, float cGDD, float denominator, float &landCover_coefficient )
   {
   float percentile = 10.0F*( cGDD / denominator );
   landCover_coefficient = m_pLandCoverCoefficientLUT->IGet( percentile, ++row, IM_LINEAR );
   }


void EvapTrans::CalculateTodaysReferenceET( FlowContext *pFlowContext, HRU *pHRU, unsigned short etMethod, float &referenceET, float sw_coeff, float lw_coeff)
   {
   float precip = 0.0f; pFlowContext->pFlowModel->GetTodaysHRUclimate(CDT_PRECIP, pHRU, precip);
   float tMean = 0.0f; pFlowContext->pFlowModel->GetTodaysHRUclimate(CDT_TMEAN, pHRU, tMean);
   float tMin = 0.0f; pFlowContext->pFlowModel->GetTodaysHRUclimate(CDT_TMIN, pHRU, tMin);
   float tMax = 0.0f; pFlowContext->pFlowModel->GetTodaysHRUclimate(CDT_TMAX, pHRU, tMax);
   float specHumid = 0.0f; pFlowContext->pFlowModel->GetTodaysHRUclimate(CDT_SPHUMIDITY, pHRU, specHumid);
   float windSpeed = 0.0f; pFlowContext->pFlowModel->GetTodaysHRUclimate(CDT_WINDSPEED, pHRU, windSpeed);
   float solarRad = 0.0f; pFlowContext->pFlowModel->GetTodaysHRUclimate(CDT_SOLARRAD, pHRU, solarRad);  
   float elevation = pHRU->m_elevation;

   // placeholder
   double PH_twilightRadiation = 0.0f;    // http://solardat.uoregon.edu/SolarData.html

   switch (etMethod)
      {
      case ETEquation::PENMAN_MONTEITH:
         {
         m_ETEq.SetDailyMinTemperature(tMin);                             //from IDU/HRU
         m_ETEq.SetDailyMaxTemperature( tMax );                           //from IDU/HRU
         m_ETEq.SetDailyMeanTemperature( tMean );                                         //from IDU/HRU //from IDU/HRU
         m_ETEq.SetSpecificHumidity( specHumid ); //PH_humidity);             //from IDU/HRU or context
         m_ETEq.SetWindSpeed( windSpeed );                                    //from IDU/HRU or context
         m_ETEq.SetSolarRadiation( solarRad );                                //from lookup table; may replace with reference longwave/shortwave radiation ref values
         m_ETEq.SetSWcoeff(sw_coeff);
         m_ETEq.SetLWcoeff(lw_coeff);
         m_ETEq.SetTwilightSolarRadiation( PH_twilightRadiation );            //from lookup table; may replace with reference longwave/shortwave radiation ref values
         m_ETEq.SetStationElevation( elevation );                             // can be culled if radiation info can be pulled from elsewhere
         m_ETEq.SetStationLatitude( m_latitude );                             // ??? ; can be culled if long wave radiation info can be pulled from elsewhere 
         m_ETEq.SetStationLongitude( m_longitude );                           // ??? ; can be culled if long wave radiation info can be pulled from elsewhere
         m_ETEq.SetTimeZoneLongitude( m_longitude );                          // ??? ; can be culled if long wave radiation info can be pulled from elsewhere
         m_ETEq.SetDoy( pFlowContext->dayOfYear );
         }
         break;

      case ETEquation::ASCE:
      case ETEquation::FAO56:
         {
         m_ETEq.SetDailyMinTemperature( tMin );                                  //
         m_ETEq.SetDailyMaxTemperature( tMax );                                  //from Climate data
         m_ETEq.SetDailyMeanTemperature( tMean );                                //from Climate data 
         m_ETEq.SetSpecificHumidity( specHumid ); //PH_humidity);                //from Climate data
         m_ETEq.SetWindSpeed( windSpeed );                                       //from Climate data
         m_ETEq.SetSolarRadiation( solarRad );                                   //from Climate data
         m_ETEq.SetSWcoeff(sw_coeff);
         m_ETEq.SetLWcoeff(lw_coeff);

     //    m_ETEq.SetTwilightSolarRadiation( PH_twilightRadiation );               // from lookup table; may replace with reference longwave/shortwave radiation ref values

         m_ETEq.SetStationElevation( elevation );                                // from HRU
         m_ETEq.SetStationLatitude( m_latitude );                                // from input tag or default
         m_ETEq.SetStationLongitude( m_longitude );                              // from input tag or default

         m_ETEq.SetTimeZoneLongitude( m_longitude );                             // ??? ; can be culled if long wave radiation info can be pulled from elsewhere
 
         m_ETEq.SetDoy( pFlowContext->dayOfYear );
         }
         break;

      case ETEquation::HARGREAVES:
         {
         m_ETEq.SetDailyMeanTemperature( tMean );
         m_ETEq.SetMonth( m_currMonth );  //from IDU/HRU
         m_ETEq.SetDoy(pFlowContext->dayOfYear );
         //Express all parameters in mm, get table for lc, fc, and wp definition
         }
         break;

      default:
         ASSERT( 0 );
         int x = 1;
      }
   } // end of CalculateTodaysReferenceET()



//void EvapTrans::GetHruET( FlowContext *pFlowContext, HRU *pHRU, int hruIndex, float &maxET, float &aet)
void EvapTrans::GetHruET( FlowContext *pFlowContext, HRU *pHRU, int hruIndex )
   {
   // This method calculates Potential ET and Actual ET for a given HRU
   //
   // Basic idea: 
   //   1) Using a specified ET method (FAO56, HARGRAVES, PENMAN_MONTEITH ), calculate reference (potential)
   //      ET value for s.
   //   2) Once the maxET is determined, estimate actual ET based on crop coefficients.
   //
   // This method relies on two tables:
   //   1) A soils table "SOILS" which provided information on the wilting point and lower water extraction limit for
   //      the soil type present in the HRU.  This table should be named "Soil.csv" and be present
   //      on the Envision path.  Further, it must contain columns "SoilType", "WP" and "fieldCapacity" with the information
   //      described above
   //   2) a Crop Coefficient table that contains crop coefficients for the HRU land use types for which
   //      actual ET calculations are made.
   //
   //-----------------------------------------------------------------------------------------------

   MapLayer *pIDUlayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

   // These next two are in the AltWaterMaster .xml, but aren't accessible from here.  Note they are 1-based rather than 0-based.
   int default_irrigation_begin_dayofyear = 60; 
   int default_irrigation_end_dayofyear = 304;
   int irrigation_start_doy = default_irrigation_begin_dayofyear - 1; // irrigation_start_doy is 0-based, i.e. Jan 1 = 0.

   int doy_today = pFlowContext->dayOfYear;     // zero-based

   float referenceET = 0.0F;
   unsigned short etMethod = m_ETEq.GetMode();
   float sw_coeff = 1.f;
   float lw_coeff = 1.f;
   int pv_here = 0; m_pHRUlayer->GetData(hruIndex, m_colHruPV_HERE, pv_here);
   if (pv_here == 1)
   {
      sw_coeff = 1.f - pFlowContext->pFlowModel->m_shortwave_interception_fraction;
      lw_coeff = pFlowContext->pFlowModel->m_pv_lw_coeff;
   }
   CalculateTodaysReferenceET( pFlowContext, pHRU, etMethod, referenceET, sw_coeff, lw_coeff);

   // get reference ET from ET method
   referenceET = m_ETEq.Run( etMethod, pHRU );    // reference mm/day (or possibly mm/hr) is returned by method

   if ( referenceET < 0.0f ) { referenceET = 0.0f; }

   /*if (m_pCropTable)
   {
   if (pHRU->m_id == 6524 || pHRU->m_id == 314 || pHRU->m_id == 241 )
   {
   m_currReferenceET = referenceET;
   }
   }*/

  
   float hruAET = 0.0f;
   float hruMaxET = 0.0f;
   // float hruArea = pHRU->m_HRUeffArea_m2;
   float landCover_coefficient = 0.0f;
   float depletionFraction = 1.0f;  // depletionFraction is 1.0 for forest and a function of crop and referenceET for agriculture

   float totalFlux = 0.0f;
   float totalRequest = 0.0f;

   int doy0_irr_season_start = 59; // Julian day (Jan1 = 0) of first day of irrigation season, Mar 1
   int doy0_irr_season_end = 303; // Julian day (Jan1 = 0) of last day of irrigation season, Oct 31

   // initialize growing season to be outside the year
   int plantingDoy = 999;
   int harvestDoy = pFlowContext->pEnvContext->daysInCurrentYear - 1;

   int count = (int)pHRU->m_polyIndexArray.GetSize();
   for ( int i = 0; i < count; i++ )
      {
      int idu = pHRU->m_polyIndexArray[ i ];
   
      // if a query is defined, does this IDU pass the query?
      if (DoesIDUPassQuery(idu))
      {
         float pFraction = 0.0f;  // crop specific depletion fraction in table
         float fc = 0.0f;  // field capacity - maximum soil moisture storage (mm)
         float wp = 0.0f;  // wilting point?
         float et_multiplier = 1.f;

         float maxET = 0.0f; // mm/day
         float aet = 0.0f; // mm/day

         int soilRow = -1;
         if (m_pSoilTable)
         {
         // get soils id from HRU
            int hbvcalib = -1;
            if (pIDUlayer->GetData(idu, m_colIduSoilID, hbvcalib))
            {

               int colECOREGION;
               pIDUlayer->CheckCol(colECOREGION, "ECOREGION", TYPE_INT, CC_MUST_EXIST);
               int ecoregion; pIDUlayer->GetData(idu, colECOREGION, ecoregion);
               ASSERT(!((hbvcalib == 16 || hbvcalib == 25) && (ecoregion == 8 || ecoregion == 9 || ecoregion == 10)));
               if (hbvcalib != 46 && hbvcalib != 9 && hbvcalib != 8 && (ecoregion == 8 || ecoregion == 9 || ecoregion == 10)) hbvcalib = 28; // HBVCALIB==28 is ClackamasAboveRiverMill.

                  // have soil ID for this HRU, find the corresponding row in the soils table
               int row = m_pSoilTable->Find(m_colSoilTableSoilID, VData(hbvcalib), 0);
               soilRow = row;

               if (row >= 0)  // found?
               {
                  m_pSoilTable->Get(m_colFC, row, fc);
                  m_pSoilTable->Get(m_colWP, row, wp);
                  m_pSoilTable->Get(m_colHbvET_MULT, row, et_multiplier);
               }
            }
         }

      
         if (soilRow < 0)
         { // if no soil data found, assume no demand
            aet = maxET = 0;
            m_iduIrrRequestArray[idu] = 0.0f;

            CString msg;
            msg.Format("GetHruET() soilRow = %d, hruIndex = %d, i = %d", soilRow, hruIndex, i);
            Report::ErrorMsg(msg);
			ASSERT(false);
		    return;
         }

      // determine landCover coefficient
            float iduArea = 0.0f; pIDUlayer->GetData(idu, m_colArea, iduArea);
            if (iduArea > 0.0f && pHRU->m_HRUeffArea_m2 > 0.0f)
            {
               if (m_pCropTable)
               {
               // get lulc id from HRU
                  int lulc = -1;
                  int row = -1;
                  landCover_coefficient = 0.0f;

                  if (pIDUlayer->GetData(idu, m_colLulc, lulc))
                  {
                  // have lulc for this HRU, find the corresponding row in the cc table
                     row = m_pCropTable->Find(m_colCropTableLulc, VData(lulc), 0);

                     if (row >= 0)
                     {
                     // the landCover specific values needed for looking up the cGDD percentile
                     // and interpolating the landCover_coefficient               

                        float denominator = -1;
                        float binToEFC = m_pCropTable->GetAsFloat(this->m_binToEFCCol, row);
                        float cGDDtoEFC = m_pCropTable->GetAsFloat(this->m_cGDDtoEFCCol, row);
                        float binEFCtoTerm = m_pCropTable->GetAsFloat(this->m_binEFCtoTermCol, row);
                        int binMethod = m_pCropTable->GetAsInt(this->m_binMethodCol, row);

                        plantingDoy = 999;
                        harvestDoy = pFlowContext->pEnvContext->daysInCurrentYear - 1;
                        plantingDoy = m_phruCropStartGrowingSeasonArray->Get(hruIndex, row);
                        harvestDoy = m_phruCropEndGrowingSeasonArray->Get(hruIndex, row);
                        int useGDD = m_pCropTable->GetAsInt(this->m_useGDDCol, row);


                        bool isGDD = (useGDD == 1) ? true : false;
                        bool isFraction = false;

                        if (doy_today >= plantingDoy && doy_today <= harvestDoy)
                        {
                        //crop specific depletion fraction 
                           pFraction = m_pCropTable->GetAsFloat(this->m_depletionFractionCol, row);

                           // using Growing Degree Days (heat unit)
                           float cGDD = m_phruCurrCGDDArray->Get(hruIndex, row);
                           if (m_calcFracCGDD)
                           {
                              float potentialHeatUnits = m_pCropTable->GetAsFloat(this->m_termCGDDCol, row);
                              int colCGDD = -1;
                              float cGDDFraction = 0.0f;
                              if (potentialHeatUnits != 0.0f)
                                 cGDDFraction = cGDD / potentialHeatUnits;

                              gpFlow->CheckCol(pFlowContext->pEnvContext->pMapLayer, colCGDD, "FracCGDD_D", TYPE_FLOAT, CC_AUTOADD);
                              pIDUlayer->SetData(idu, colCGDD, cGDDFraction); // degree C days
                           }

                           if (cGDD < cGDDtoEFC  && isGDD)
                           {
                              denominator = binToEFC;
                              LookupLandCoverCoefficient(row, cGDD, denominator, landCover_coefficient);
                           }
                        // using Growing Days (number of days)
                           else if (!isGDD)
                           {
                              int growingDays = doy_today - m_phruCropStartGrowingSeasonArray->Get(hruIndex, row);
                              denominator = (float)m_phruCropEndGrowingSeasonArray->Get(hruIndex, row) - m_phruCropStartGrowingSeasonArray->Get(hruIndex, row);
                              LookupLandCoverCoefficient(row, (float)growingDays, denominator, landCover_coefficient);
                           }
                        // using GDD after EFC, first lookup method
                           else if (binMethod == 1)
                           {
                              denominator = binEFCtoTerm;
                              LookupLandCoverCoefficient(row, cGDD, denominator, landCover_coefficient);
                           }
                        // using GDD after EFC, second lookup method
                           else if (binMethod == 2)
                           {
                              landCover_coefficient = m_pCropTable->GetAsFloat(this->m_constCropCoeffEFCtoTCol, row);
                           }
                        }
                        else
                        {
                        // outside of Growing Season landCover_coefficient is 0% percentile value
                           denominator = 1;
                   //        LookupLandCoverCoefficient( row, 0.0f, denominator, landCover_coefficient );
                           landCover_coefficient = 0.10f; // bare soil ET
                        }
                     } // crop is found
                  }  // get lulc
               } // end m_pCropTable
            } // end if has area

         // get potential ET 
            switch (GetMethod())
            {
               case GM_FAO56:
               case GM_HARGREAVES:
                  maxET = referenceET * landCover_coefficient * et_multiplier;
                  ASSERT(maxET >= 0.0f && maxET <= 1.0E10f);
                  /*if (pFraction < 0)
                     depletionFraction = 1.0f;
                     else
                     depletionFraction = pFraction + 0.04f * (5.0f - maxET);    */
                  break;

               case GM_PENMAN_MONTEITH:
                  if (m_iduIrrRequestArray[idu] >= 0.0f)
                     maxET = m_iduIrrRequestArray[idu] * et_multiplier;
                  m_iduIrrRequestArray[idu] = 0.0f;
                  break;

               case GM_WETLAND_ET:
                  if (m_iduIrrRequestArray[idu] >= 0.0f)
                     maxET = m_iduIrrRequestArray[idu];
                  m_iduIrrRequestArray[idu] = 0.0f;
                  break;

               default: ASSERT(0); break;
            } // end of switch (Getmethod())

         // calculate actual ET based on soil condition. This is a fractional value based on the distributions
            float totalVolWater = 0.0f;    // m3
            float soilwater_mm = 0.0f;       // mm 

            // choose the layer distribution that matches
            int layerIndex = m_pLayerDists->GetLayerIndex(idu);
            if (layerIndex != -1)
            {
               // isRatio = m_pLayerDists->m_layerArray[layerIndex]->isRatio;
               int layerDistArraySize = (int)m_pLayerDists->m_layerArray[layerIndex]->layerDistArray.GetSize();

               // determine total available water
               /* for ( */ int j = 0; /* j < layerDistArraySize; j++) */
               {
                  LayerDist *pLD = m_pLayerDists->m_layerArray[layerIndex]->layerDistArray[j];
                  HRULayer *pHRULayer = pHRU->GetLayer(pLD->layerIndex);
                  totalVolWater += float(pHRULayer->m_volumeWater);   // m3  Results in the volume of water
                  // soilwater = pHRULayer->m_wc * pLD->fraction;       // mm.  Results in the length of water   
                  soilwater_mm = (float)(pHRULayer->m_volumeWater > 0. ?
                     (pHRULayer->m_volumeWater / (pHRU->m_HRUeffArea_m2 * pHRULayer->m_HRUareaFraction)) * 1000.f
                     : 0.f);
               }
   /*
                  if ( isRatio )
                  {
                     // determine fraction in each layer to remove later
                     // int layerDistArraySize = (int)m_pLayerDists->m_layerArray[layerIndex]->layerDistArray.GetSize();
                     for (int j = 0; j < layerDistArraySize; j++)
                     {
                        LayerDist *pLD = m_pLayerDists->m_layerArray[layerIndex]->layerDistArray[j];
                        HRULayer *pHRULayer = pHRU->GetLayer(pLD->layerIndex);
                        if ( totalVolWater > 0.0f )
                         pLD->fraction = float(pHRULayer->m_volumeWater) / totalVolWater;
                     }

                     float areaToUse = hruArea - pHRU->m_areaIrrigated;    // m2
                     if ( areaToUse > 0.0f) soilwater = totalVolWater / areaToUse * MM_PER_M;   // mm
                  }
   */
            } // end layer choice

            int irrigate = 0;
            pIDUlayer->GetData(idu, m_colIrrigation, irrigate);
            bool isIrrigated = (irrigate == 1);
            float idu_soilwater_est_begin_mm = soilwater_mm;
            if (isIrrigated && pFlowContext->dayOfYear > 0) pIDUlayer->GetData(idu, m_colDailySOILH2OEST, idu_soilwater_est_begin_mm); // mm
            if (idu_soilwater_est_begin_mm > soilwater_mm) idu_soilwater_est_begin_mm = soilwater_mm;

            // calculate actual ET based on soil condition and relative humidity     
            float threshold = 0.5f * (fc - wp);
            float vpd_scalar = 1.f;
            float fTheta = 0.f;

            int lulc_a = -1; pIDUlayer->GetData(idu, m_colLULC_A, lulc_a);
            if (lulc_a == LULCA_FOREST)
            {
               float vpdMin = 0.610f; // 1.5f; // kPa
               float vpdMax = 3.100f; // 4.0f; // kPa
               vpd_scalar = (vpdMax - GlobalMethod::m_iduVPDarray[idu]) / (vpdMax - vpdMin);
               if (vpd_scalar < 0.02) vpd_scalar = 0.02f; // clip to [0.02, 1.0]
               else if (vpd_scalar > 1.f) vpd_scalar = 1.f;
            }

            if (idu_soilwater_est_begin_mm > threshold) fTheta = 1.0f; // wet conditions; ks = 1
            else if (idu_soilwater_est_begin_mm <= wp) fTheta = (m_TurnerScenario == 1 || m_TurnerScenario == 3 || m_TurnerScenario == 4 || m_TurnerScenario == 5) ? 1.f : 0.f; // dry conditions; ks = 0
            else
            { // intermediate conditions; ks is a linear function of soilwater
               float slope = 1.0F / (threshold - wp);
               fTheta = (m_TurnerScenario == 1 || m_TurnerScenario == 3 || m_TurnerScenario == 4 || m_TurnerScenario == 5) ? 1.f : (slope * (idu_soilwater_est_begin_mm - wp));
            }
            if (m_TurnerScenario == 3 || m_TurnerScenario == 4 || m_TurnerScenario == 5) vpd_scalar = 1.f;
            aet = maxET * (fTheta < vpd_scalar ? fTheta : vpd_scalar);
            float phonyFlux = 0.f;
            if ((m_TurnerScenario == 1 || m_TurnerScenario == 3 || m_TurnerScenario == 4 || m_TurnerScenario == 5) && (idu_soilwater_est_begin_mm - aet < wp)) phonyFlux = wp - (idu_soilwater_est_begin_mm - aet);

            // reset iduCropDemandArray
            m_iduIrrRequestArray[idu] = 0.0f;

            // fill annual maximum ET array
            m_iduAnnAvgMaxETArray[idu] += maxET;   // mm/day

            // fill seasonal actual and maximum ET arrays
            if (doy_today >= plantingDoy && doy_today <= harvestDoy)
            {
               m_iduSeasonalAvgETArray[idu] += aet;   // mm/day
               m_iduSeasonalAvgMaxETArray[idu] += maxET;   // mm/day
            } // end of if (doy_today >= plantingDoy && doy_today <= harvestDoy)

            float rainfall = pHRU->m_currentPrecip;         // mm
         IrrigationState irr_state; 
         float idu_soilwater_est_end_mm = -1.f;
         if (pFlowContext->dayOfYear <= 0) irr_state = IRR_NOT_STARTED_OR_OFF_SEASON;
         else
            {
            pIDUlayer->GetData(idu, m_colDailySOILH2OEST, idu_soilwater_est_begin_mm); // mm
            int irr_state_int;  pIDUlayer->GetData(idu, m_colDailyIRR_STATE, irr_state_int); irr_state = (IrrigationState)irr_state_int; // mm
            } // end of if (pFlowContext->dayOfYear <= 0) ... else
         float avail_water_est_begin =  idu_soilwater_est_begin_mm - wp;
         if (avail_water_est_begin > (fc - wp)) avail_water_est_begin = fc - wp;
         if (avail_water_est_begin < 0.f) avail_water_est_begin = 0.f;

         // calculate irrigation request, if any
         float unsmoothed_irr_req = -1.f;
         float smoothed_irr_req = -1.f;
         float avail_water_est_end = -1.f;
         int start_doy0 = isIrrigated ? max(plantingDoy, doy0_irr_season_start) : 366;
         int end_doy0 = isIrrigated ? min(harvestDoy, doy0_irr_season_end) : -1;
         if (isIrrigated && doy_today >= start_doy0 && doy_today <= end_doy0)
            {
            float max_irr_rate_mm = 7.56f; // 7.56 mm/day equivalent to 1/80 cfs per acre
//          float tgt_min_avail_mm = threshold; // begin irrigating when the stomata begin to close due to dry soil
//          float tgt_min_avail_mm = wp + 0.75f*(fc - wp);
            float tgt_min_soil_moisture_mm = wp + 0.80f*(fc - wp);
            float tgt_min_avail_mm = tgt_min_soil_moisture_mm - wp;
//          float fully_wetted_mm = wp + 0.90f*(fc - wp);
//          float wet_enough_mm = wp + 0.80f*(fc - wp);
            float wet_enough_mm = wp + 0.825f*(fc - wp);
            m_iduIrrRequestArray[idu] = 0.f;
            float moisture_deficit_mm = -1.f;
            float prev_smoothed_irr_req;
            if (doy_today == start_doy0)
               { // Initialize avail_water_est at the beginning of the growing season.
               idu_soilwater_est_begin_mm = soilwater_mm > fc ? fc : soilwater_mm;
               avail_water_est_begin = idu_soilwater_est_begin_mm > wp ? (idu_soilwater_est_begin_mm - wp) : 0.f;
               moisture_deficit_mm = avail_water_est_begin > tgt_min_avail_mm ? 0.f : (tgt_min_avail_mm - avail_water_est_begin);
               smoothed_irr_req = rainfall > moisture_deficit_mm ? 0.f : (moisture_deficit_mm / (1.f - m_irrigLossFactor));
               if (smoothed_irr_req > max_irr_rate_mm) smoothed_irr_req = max_irr_rate_mm;
               prev_smoothed_irr_req = smoothed_irr_req;
               }
            else
               {
               pIDUlayer->GetData(idu, m_colDailyIRRACRQ_D, prev_smoothed_irr_req); // mm/day
               pIDUlayer->GetData(idu, m_colDailySOILH2OEST, idu_soilwater_est_begin_mm);
               avail_water_est_begin = idu_soilwater_est_begin_mm > wp ? (idu_soilwater_est_begin_mm - wp) : 0.f;
               }

            // Irrigation states are  IRR_NOT_STARTED_OR_OFF_SEASON, IRR_FULL, IRR_PARTIAL, IRR_SUSPENDED, IRR_SHUT_OFF_BY_REG_ACTION
            switch (irr_state)
               {
               case IRR_NOT_STARTED_OR_OFF_SEASON:
               case IRR_SUSPENDED:
                  if (avail_water_est_begin < tgt_min_avail_mm && !(rainfall > 2 * maxET))
                     { // avail soil water is below the threshold for irrigation and it's not raining enough to matter
                     irr_state = IRR_FULL;
                     unsmoothed_irr_req = max_irr_rate_mm;
                     }
                  else unsmoothed_irr_req = 0.f;
                  break;
               case IRR_FULL:
               case IRR_PARTIAL:
                  if (idu_soilwater_est_begin_mm >= wet_enough_mm || rainfall > 2 * maxET)
                     {
                     irr_state = IRR_SUSPENDED;
                     unsmoothed_irr_req = 0.f;
                     }
                  else unsmoothed_irr_req = max_irr_rate_mm;
                  break;
               default:
               case IRR_SHUT_OFF_BY_REG_ACTION:
                  unsmoothed_irr_req = 0.f;
                  smoothed_irr_req = 0.f;
                  break;
               }

            /* Unless this is the first day of the season, or the water right has been regulated off, update smoothed_irr_req.
            if ((irr_state != IRR_SHUT_OFF_BY_REG_ACTION) && (doy_today > start_doy0))
               {
               float tau = 7.f; // smoothing time constant
//             float tau_eff = (doy_today - start_doy0) < tau ? (doy_today - start_doy0) : tau;
               float tau_eff = (doy_today - start_doy0) < tau ? (doy_today - start_doy0 + 1.f) : tau;
//             smoothed_irr_req = prev_smoothed_irr_req*exp(-1.f / tau_eff) + unsmoothed_irr_req*(1.f - exp(-1.f / tau_eff));
               smoothed_irr_req = prev_smoothed_irr_req*((tau_eff - 1.f) / tau_eff) + unsmoothed_irr_req*(1.f / tau_eff); // per Bill Jaeger 1/26/16
               if (smoothed_irr_req < 1.f) smoothed_irr_req = 0.f; // per Bill Jaeger 1/26/16
               } // end of if (doy_today > start_doy0)
            */
            m_iduIrrRequestArray[idu] = smoothed_irr_req = unsmoothed_irr_req;

            // Update avail_water_est.
            float soil_water_retention_fraction = 0.995f; // 1.0f; // 0.995f; // 0.99 0.98
            //float rainfall_retention_fraction = 1.0f - HBV::GroundWaterRechargeFraction(0.f, avail_water_est_begin, fc, beta);
            //float rainfall_lossFraction = pow((avail_water_est_begin + wp) / fc, beta); if (lossFraction > 1.0f) lossFraction = 1.0f;
            //rainfall_retention_fraction = 1.0f - lossFraction;
            avail_water_est_end = soil_water_retention_fraction*avail_water_est_begin + /* rainfall_retention_fraction * */rainfall +
               m_iduIrrRequestArray[idu] * (1.f - m_irrigLossFactor) - aet;
            if (avail_water_est_end > fc - wp) avail_water_est_end = fc - wp;
            else if (avail_water_est_end < 0.f) avail_water_est_end = 0.f;
            idu_soilwater_est_end_mm = avail_water_est_end + wp;
            } // end of if (isIrrigated && doy_today >= plantingDoy && doy_today >= doy0_irr_season_start && doy_today <= harvestDoy && doy_today <= doy0_irr_season_end)
         else
            { // Outside of the growing and/or irrigation season
            avail_water_est_end = unsmoothed_irr_req = m_iduIrrRequestArray[idu] = smoothed_irr_req = 0.f;
            idu_soilwater_est_end_mm = soilwater_mm;
            irr_state = IRR_NOT_STARTED_OR_OFF_SEASON;
            }

         // output daily values
         pIDUlayer->m_readOnly = false;

         float et_day_mm; pIDUlayer->GetData(idu, m_colDailyET, et_day_mm);
         et_day_mm += aet;
         pIDUlayer->SetData( idu, m_colDailyET, et_day_mm);  // mm/day

         pIDUlayer->SetData( idu, m_colDailyMaxET, maxET );  // mm/day
         pIDUlayer->SetData( idu, m_colSM_DAY, soilwater_mm );  // mm/day
         pIDUlayer->SetData(idu, m_colVPD, GlobalMethod::m_iduVPDarray[idu]);
         pIDUlayer->SetData(idu, m_colF_THETA, fTheta);  // on the unit interval, dimensionless
         pIDUlayer->SetData(idu, m_colVPD_SCALAR, vpd_scalar);  // on the unit interval, dimensionless
         pIDUlayer->SetData(idu, m_colDailySOILH2OEST, idu_soilwater_est_end_mm);  // mm
         pIDUlayer->SetData(idu, m_colDailyIRR_STATE, irr_state);  // mm
         pIDUlayer->SetData(idu, m_colDailyIRRRQST_D, unsmoothed_irr_req);  // mm
         pIDUlayer->SetData(idu, m_colDailyIRRACRQ_D, smoothed_irr_req);  // mm
         pIDUlayer->m_readOnly = true;

         // store in shared data
         float val = 1.0;
         if (iduArea / pHRU->m_HRUtotArea_m2 < 1.0)
            val = iduArea / pHRU->m_HRUtotArea_m2;

         hruAET += aet * val;
         hruMaxET += maxET * val;

         int time = pFlowContext->dayOfYear;
         // soil layer distribution
         if ( aet > 0.0 )
            {
            // found layer that matches idu
            if ( layerIndex != -1 )
               {
               // determine distributions
               int layerDistArraySize = (int)m_pLayerDists->m_layerArray[ layerIndex ]->layerDistArray.GetSize();
               for ( int j = 0; j < layerDistArraySize; j++ )
                  {
                  LayerDist *pLD = m_pLayerDists->m_layerArray[layerIndex]->layerDistArray[ j ];
                  HRULayer *pHRULayer = pHRU->GetLayer( pLD->layerIndex );
                  float flux = (float)((aet - phonyFlux) * pLD->fraction * iduArea * M_PER_MM);
                  pHRULayer->CheckForNaNs("GetHRUET", pHRULayer->AddFluxFromGlobalHandler( flux, FL_TOP_SINK ));     // m3/d                  
                  } // end of for ( int j = 0; j < layerDistArraySize; j++ )
               } // end of if ( layerIndex != -1 )
            } // end of if ( aet > 0.0 )

         }  // end of: if (idu processed)
      }  // end of: for ( each IDU in the HRU )

   // all done

   pHRU->m_currentMaxET += hruMaxET;    // mm/day
   pHRU->m_currentET += hruAET;     // mm/day

   return;
   }


EvapTrans *EvapTrans::LoadXml( TiXmlElement *pXmlEvapTrans, MapLayer *pIDUlayer, LPCTSTR filename )
   {
   if ( pXmlEvapTrans == NULL )
      return NULL;

   // get attributes
   LPTSTR name = NULL;
   LPTSTR cropTablePath = NULL;
   LPTSTR soilTablePath = NULL;
   LPTSTR method = NULL;
   LPTSTR query = NULL;
   bool use = true;
   LPTSTR lulcCol = NULL;
   LPTSTR soilCol = NULL;
   float latitude = 45;
   float longitude = 123;
   float rSt = 1000.0f;//bulk stomatal resistance across all forest cover classes
   float irrigLossFactor = 0.25f;
   bool calcFracCGDD = false;

   XML_ATTR attrs[] = {
      // attr                 type          address               isReq  checkCol
      { "name", TYPE_STRING, &name, false, 0 },
      { "method", TYPE_STRING, &method, true, 0 },
      { "query", TYPE_STRING, &query, false, 0 },
      { "use", TYPE_BOOL, &use, false, 0 },
      { "lulc_col", TYPE_STRING, &lulcCol, true, CC_MUST_EXIST },
      { "soil_col", TYPE_STRING, &soilCol, false, 0 },
      { "crop_table", TYPE_STRING, &cropTablePath, false, 0 },
      { "soil_table", TYPE_STRING, &soilTablePath, false, 0 },
      { "irrig_loss_factor", TYPE_FLOAT, &irrigLossFactor, false, 0 },
      { "latitude", TYPE_FLOAT, &latitude, true, 0 },
      { "longitude", TYPE_FLOAT, &longitude, true, 0 },
      { "bulkStomatalResistance", TYPE_FLOAT, &rSt, false, 0 },
      { "outputFractionCGDD", TYPE_BOOL, &calcFracCGDD, false, 0 },
      { NULL, TYPE_NULL, NULL, false, 0 } };

   bool ok = TiXmlGetAttributes( pXmlEvapTrans, attrs, filename, pIDUlayer );
   if ( !ok )
      {
      CString msg;
      msg.Format( _T( "EvapTrans::LoadXml() Misformed element reading <evap_trans> attributes in input file %s" ), filename );
      Report::ErrorMsg( msg );
      return NULL;
      }

   EvapTrans *pEvapTrans = new EvapTrans( name );

   pEvapTrans->m_pLayerDists = new LayerDistributions();
   pEvapTrans->m_pLayerDists->ParseLayers( pXmlEvapTrans, pIDUlayer, filename );

   pEvapTrans->m_colLulc = pIDUlayer->GetFieldCol(lulcCol);
   pEvapTrans->m_colIduSoilID = pIDUlayer->GetFieldCol(soilCol);
   pEvapTrans->m_colArea = pIDUlayer->GetFieldCol( "AREA" );

   pEvapTrans->m_calcFracCGDD = calcFracCGDD;

   if ( method != NULL )
      {
      switch ( method[ 0 ] )
         {
         case 'a':
         case 'A':
            pEvapTrans->SetMethod( GM_ASCE );
            pEvapTrans->m_ETEq.SetMode( ETEquation::ASCE );
            break;
         case 'f':
         case 'F':
            {
            if ( method[ 1 ] == 'a' || method[ 1 ] == 'A' )
               {
               pEvapTrans->SetMethod( GM_FAO56 );
               }
            else if ( method[ 1 ] == 'n' || method[ 1 ] == 'N' )
               {
               pEvapTrans->m_extSource = method;
               FLUXSOURCE sourceLocation = ParseSource( pEvapTrans->m_extSource, pEvapTrans->m_extPath, pEvapTrans->m_extFnName,
                  pEvapTrans->m_extFnDLL, pEvapTrans->m_extFn );

               if ( sourceLocation != FS_FUNCTION )
                  {
                  CString msg;
                  msg.Format( "Error parsing <external_method> 'fn=%s' specification for '%s'.  This method will not be invoked.", method, name );
                  Report::ErrorMsg( msg );
                  pEvapTrans->SetMethod( GM_NONE );
                  pEvapTrans->m_use = false;
                  }
               else
                  pEvapTrans->SetMethod( GM_EXTERNAL );
               }
            }
            break;

         case 'p':
         case 'P':
            pEvapTrans->SetMethod( GM_PENMAN_MONTEITH );
            pEvapTrans->m_ETEq.SetMode( ETEquation::PENMAN_MONTEITH );
            break;

         case 'h':
         case 'H':
            pEvapTrans->SetMethod(GM_HARGREAVES);
            pEvapTrans->m_ETEq.SetMode(ETEquation::HARGREAVES);
            break;

         case 'w': // WetlandET
         case 'W':
            pEvapTrans->SetMethod(GM_WETLAND_ET);
            pEvapTrans->m_ETEq.SetMode(ETEquation::WETLAND_ET);
            break;

         case 'k':
         case 'K':
            {
            CString msg; msg.Format("EvapTrans::LoadXml() Kimberly Pennman ET calculation is not implemented.");
            Report::ErrorMsg(msg);
            return(false);
            }

         default:
            pEvapTrans->SetMethod( GM_NONE );
         }
      }

   if ( pEvapTrans->m_method != GM_PENMAN_MONTEITH && pEvapTrans->m_method != GM_WETLAND_ET) 
      {
      CString tmpPath1;

      if ( cropTablePath == NULL )
         {
         CString msg;
         msg.Format( "EvapTrans: Missing crop_table attribute when reading '%s'.  Table be loaded", cropTablePath );
         Report::LogMsg( msg, RT_WARNING );
         pEvapTrans->SetMethod( GM_NONE );
         return pEvapTrans;
         }
      
      CString tmpPath;
      int pathNum = PathManager::FindPath(cropTablePath, tmpPath);

      LPCTSTR firstPart = "";
      if (pathNum > 0) firstPart = PathManager::GetPath(pathNum - 1);
      else if (pathNum < 0 )
         {
         CString msg;
         msg.Format( "EvapTrans: Specified crop table '%s' for method '%s' can not be found, this method will be ignored", cropTablePath, name );
         Report::ErrorMsg(msg);
         pEvapTrans->SetMethod( GM_NONE );
         return pEvapTrans;
         }
      CString pathPrefix = firstPart;
      CString pathSuffix = cropTablePath;
      CString tmpPath2 = pathPrefix + pathSuffix;

      // read in CropTable
      pEvapTrans->m_cropTablePath = tmpPath2;
      pEvapTrans->m_pCropTable = new VDataObj;
      pEvapTrans->m_cropCount = pEvapTrans->m_pCropTable->ReadAscii(pEvapTrans->m_cropTablePath, ',' );

      if ( pEvapTrans->m_cropCount <= 0 )
         {
         CString msg;
         msg.Format( "EvapTrans::LoadXML could not load Crop .csv file \n" );
         Report::ErrorMsg( msg );
         return NULL;
         }
      else
         { 
         CString msg;
         msg.Format("EvapTrans::LoadXml() loaded %d crop entries from file %s.", pEvapTrans->m_cropCount, pEvapTrans->m_cropTablePath);
         Report::LogMsg(msg);
         }

      // get CropTable column numbers for column headers
      pEvapTrans->m_colCropTableLulc = pEvapTrans->m_pCropTable->GetCol( lulcCol );
      pEvapTrans->m_plantingMethodCol = pEvapTrans->m_pCropTable->GetCol( "Planting Date Method" );
      pEvapTrans->m_plantingThresholdCol = pEvapTrans->m_pCropTable->GetCol( "Planting Threshold" );
      pEvapTrans->m_t30BaselineCol = pEvapTrans->m_pCropTable->GetCol( "T30 Baseline [days]" );

      pEvapTrans->m_gddEquationTypeCol = pEvapTrans->m_pCropTable->GetCol( "GDD Equation" );
      pEvapTrans->m_gddBaseTempCol = pEvapTrans->m_pCropTable->GetCol( "GDD Base Temp [C]" );
      pEvapTrans->m_termMethodCol = pEvapTrans->m_pCropTable->GetCol( "Term Date Method" );
      pEvapTrans->m_termTempCol = pEvapTrans->m_pCropTable->GetCol( "Killing Frost Temp [C]" );
      pEvapTrans->m_minGrowSeasonCol = pEvapTrans->m_pCropTable->GetCol( "Min Growing Season [days]" );
      pEvapTrans->m_termCGDDCol = pEvapTrans->m_pCropTable->GetCol( "CGDD P to T" );
      pEvapTrans->m_earliestPlantingDateCol = pEvapTrans->m_pCropTable->GetCol( "Earliest Planting Date" );

      pEvapTrans->m_useGDDCol = pEvapTrans->m_pCropTable->GetCol( "useGDD" );
      pEvapTrans->m_binToEFCCol = pEvapTrans->m_pCropTable->GetCol( "GDD per 10% P to EFC" );
      pEvapTrans->m_cGDDtoEFCCol = pEvapTrans->m_pCropTable->GetCol( "CGDD P/GU to EFC" );
      pEvapTrans->m_binEFCtoTermCol = pEvapTrans->m_pCropTable->GetCol( "GDD per 10% EFC to T" );
      pEvapTrans->m_binMethodCol = pEvapTrans->m_pCropTable->GetCol( "Interp Method EFC to T (1-CGDD; 2-constant)" );
      pEvapTrans->m_constCropCoeffEFCtoTCol = pEvapTrans->m_pCropTable->GetCol( "Kc value EFC to T if constant" );
      pEvapTrans->m_depletionFractionCol = pEvapTrans->m_pCropTable->GetCol( "Depletion Fraction" );
      pEvapTrans->m_precipThresholdCol = pEvapTrans->m_pCropTable->GetCol( "Precipitation Threshold" );  // optional
      pEvapTrans->m_yieldResponseFactorCol = pEvapTrans->m_pCropTable->GetCol( "Yield Response Factor" );  // optional

      if ( pEvapTrans->m_colCropTableLulc < 0 || pEvapTrans->m_plantingThresholdCol < 0 || pEvapTrans->m_t30BaselineCol < 0 ||
         pEvapTrans->m_gddEquationTypeCol < 0 || pEvapTrans->m_gddBaseTempCol < 0 || pEvapTrans->m_termMethodCol < 0 || pEvapTrans->m_minGrowSeasonCol < 0 ||
         pEvapTrans->m_termTempCol < 0 || pEvapTrans->m_termCGDDCol < 0 || pEvapTrans->m_useGDDCol < 0 || pEvapTrans->m_binToEFCCol < 0 || pEvapTrans->m_cGDDtoEFCCol < 0 ||
         pEvapTrans->m_binEFCtoTermCol < 0 || pEvapTrans->m_binMethodCol < 0 || pEvapTrans->m_constCropCoeffEFCtoTCol < 0 || pEvapTrans->m_earliestPlantingDateCol < 0 ||
         pEvapTrans->m_depletionFractionCol < 0 )
         {
         CString msg;
         msg.Format( "Evaptrans::LoadXML: One or more column headings are incorrect in Crop .csv file\n" );
         Report::ErrorMsg( msg );
         return NULL;
         }

      int per0Col = pEvapTrans->m_pCropTable->GetCol(     "0%" );
      int per10Col = pEvapTrans->m_pCropTable->GetCol(   "10%" );
      int per20Col = pEvapTrans->m_pCropTable->GetCol(   "20%" );
      int per30Col = pEvapTrans->m_pCropTable->GetCol(   "30%" );
      int per40Col = pEvapTrans->m_pCropTable->GetCol(   "40%" );
      int per50Col = pEvapTrans->m_pCropTable->GetCol(   "50%" );
      int per60Col = pEvapTrans->m_pCropTable->GetCol(   "60%" );
      int per70Col = pEvapTrans->m_pCropTable->GetCol(   "70%" );
      int per80Col = pEvapTrans->m_pCropTable->GetCol(   "80%" );
      int per90Col = pEvapTrans->m_pCropTable->GetCol(   "90%" );
      int per100Col = pEvapTrans->m_pCropTable->GetCol( "100%" );
      int per110Col = pEvapTrans->m_pCropTable->GetCol( "110%" );
      int per120Col = pEvapTrans->m_pCropTable->GetCol( "120%" );
      int per130Col = pEvapTrans->m_pCropTable->GetCol( "130%" );
      int per140Col = pEvapTrans->m_pCropTable->GetCol( "140%" );
      int per150Col = pEvapTrans->m_pCropTable->GetCol( "150%" );
      int per160Col = pEvapTrans->m_pCropTable->GetCol( "160%" );
      int per170Col = pEvapTrans->m_pCropTable->GetCol( "170%" );
      int per180Col = pEvapTrans->m_pCropTable->GetCol( "180%" );
      int per190Col = pEvapTrans->m_pCropTable->GetCol( "190%" );
      int per200Col = pEvapTrans->m_pCropTable->GetCol( "200%" );

      if ( per0Col < 0 || per10Col < 0 || per20Col < 0 || per30Col < 0 || per40Col < 0 || per50Col < 0 || per60Col < 0 ||
         per70Col < 0 || per80Col < 0 || per90Col < 0 || per100Col < 0 || per110Col < 0 || per120Col < 0 || per130Col < 0 ||
         per140Col < 0 || per150Col < 0 || per160Col < 0 || per170Col < 0 || per180Col < 0 || per190Col < 0 || per200Col < 0 )
         {
         CString msg;
         msg.Format( "Evaptrans::LoadXML: One or more percentile column headings are incorrect in Crop .csv file\n" );
         Report::ErrorMsg( msg );
         return NULL;
         }

      int colCount = pEvapTrans->m_cropCount + 1;
      int rowCount = 21;
      pEvapTrans->m_pLandCoverCoefficientLUT = new FDataObj( colCount, rowCount );

      float percentile;
      for ( int i = 0; i < rowCount; i++ )
         {
         percentile = i * 10.0f;
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( 0, i, percentile );
         }

      for ( int i = 0; i < pEvapTrans->m_cropCount; i++ )
         {
         int j = 0;
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per0Col,   i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per10Col,  i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per20Col,  i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per30Col,  i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per40Col,  i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per50Col,  i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per60Col,  i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per70Col,  i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per80Col,  i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per90Col,  i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per100Col, i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per110Col, i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per120Col, i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per130Col, i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per140Col, i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per150Col, i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per160Col, i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per170Col, i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per180Col, i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per190Col, i ) );
         pEvapTrans->m_pLandCoverCoefficientLUT->Set( i + 1, j++, pEvapTrans->m_pCropTable->GetAsFloat(per200Col, i ) );
         }
      }

   pEvapTrans->m_irrigLossFactor = irrigLossFactor;

   CString tmpPath3;
   if ( soilTablePath == NULL )
      {
      CString msg;
      msg.Format( "EvapTrans: Missing soil_table attribute for method '%s'.  This method will be ignored", name );
      Report::LogMsg( msg, RT_WARNING );
      pEvapTrans->SetMethod( GM_NONE );
      return pEvapTrans;
      }

   // does file exist?
   if (PathManager::FindPath( soilTablePath, tmpPath3 ) < 0 )
      {
      CString msg;
      msg.Format( "EvapTrans: Specified soil table '%s' for method '%s' can not be found, this method will be ignored", soilTablePath, name );
      Report::LogMsg( msg, RT_WARNING );
      pEvapTrans->SetMethod( GM_NONE );
      return pEvapTrans;
      }

   // read in SoilTable
   pEvapTrans->m_soilTablePath = soilTablePath;
   pEvapTrans->m_pSoilTable = new FDataObj;
   int soilCount = pEvapTrans->m_pSoilTable->ReadAscii( tmpPath3, ',' );

   if ( soilCount <= 0 )
      {
      CString msg;
      msg.Format( "EvapTrans::LoadXML could not load Soil .csv file \n" );
      Report::InfoMsg( msg );
      return NULL;
      }

   pEvapTrans->m_colSoilTableSoilID = pEvapTrans->m_pSoilTable->GetCol( soilCol );

   pEvapTrans->m_colWP = pEvapTrans->m_pSoilTable->GetCol( "WP" );
   pEvapTrans->m_colFC = pEvapTrans->m_pSoilTable->GetCol("FC");
   pEvapTrans->m_colHbvET_MULT = pEvapTrans->m_pSoilTable->GetCol("ET_MULT");

   pEvapTrans->m_bulkStomatalResistance = rSt;
   pEvapTrans->m_latitude = latitude;
   pEvapTrans->m_longitude = longitude;

   if ( query )
      pEvapTrans->m_query = query;

   return pEvapTrans;
   }

float EvapTrans::CO2byYear(int calendarYear, CString scenarioName)
   {
#define START_YEAR 1950
#define END_YEAR 2100
#define NYEARS (END_YEAR - START_YEAR + 1)
   double CO2_ppm[NYEARS][3] = { // year, RCP 4.5, RCP 8.5
         { START_YEAR, 310.75, 310.75 },
         { 1951, 311.1, 311.1 },
         { 1952, 311.5, 311.5 },
         { 1953, 311.925, 311.925 },
         { 1954, 312.425, 312.425 },
         { 1955, 313, 313 },
         { 1956, 313.6, 313.6 },
         { 1957, 314.225, 314.225 },
         { 1958, 314.8475, 314.8475 },
         { 1959, 315.5, 315.5 },
         { 1960, 316.2725, 316.2725 },
         { 1961, 317.075, 317.075 },
         { 1962, 317.795, 317.795 },
         { 1963, 318.3975, 318.3975 },
         { 1964, 318.925, 318.925 },
         { 1965, 319.6475, 319.6475 },
         { 1966, 320.6475, 320.6475 },
         { 1967, 321.605, 321.605 },
         { 1968, 322.635, 322.635 },
         { 1969, 323.9025, 323.9025 },
         { 1970, 324.985, 324.985 },
         { 1971, 325.855, 325.855 },
         { 1972, 327.14, 327.14 },
         { 1973, 328.6775, 328.6775 },
         { 1974, 329.7425, 329.7425 },
         { 1975, 330.585, 330.585 },
         { 1976, 331.7475, 331.7475 },
         { 1977, 333.2725, 333.2725 },
         { 1978, 334.8475, 334.8475 },
         { 1979, 336.525, 336.525 },
         { 1980, 338.36, 338.36 },
         { 1981, 339.7275, 339.7275 },
         { 1982, 340.7925, 340.7925 },
         { 1983, 342.1975, 342.1975 },
         { 1984, 343.7825, 343.7825 },
         { 1985, 345.2825, 345.2825 },
         { 1986, 346.7975, 346.7975 },
         { 1987, 348.645, 348.645 },
         { 1988, 350.7375, 350.7375 },
         { 1989, 352.4875, 352.4875 },
         { 1990, 353.855, 353.855 },
         { 1991, 355.0175, 355.0175 },
         { 1992, 355.885, 355.885 },
         { 1993, 356.7775, 356.7775 },
         { 1994, 358.1275, 358.1275 },
         { 1995, 359.8375, 359.8375 },
         { 1996, 361.4625, 361.4625 },
         { 1997, 363.155, 363.155 },
         { 1998, 365.3225, 365.3225 },
         { 1999, 367.3475, 367.3475 },
         { 2000, 368.865, 368.865 },
         { 2001, 370.4675, 370.4675 },
         { 2002, 372.5225, 372.5225 },
         { 2003, 374.76, 374.76 },
         { 2004, 376.8125, 376.8125 },
         { 2005, 378.8125, 378.8125 },
         { 2006, 380.8275, 380.8275 },
         { 2007, 382.7775, 382.7775 },
         { 2008, 384.8, 384.8 },
         { 2009, 386.9516, 387.01226 },
         { 2010, 389.12785, 389.32416 },
         { 2011, 391.27357, 391.63801 },
         { 2012, 393.4211, 394.00866 },
         { 2013, 395.58283, 396.46384 },
         { 2014, 397.76408, 399.00402 },
         { 2015, 399.96631, 401.62793 },
         { 2016, 402.18432, 404.32819 },
         { 2017, 404.41077, 407.09588 },
         { 2018, 406.64292, 409.92701 },
         { 2019, 408.8817, 412.82151 },
         { 2020, 411.12868, 415.78022 },
         { 2021, 413.37804, 418.79629 },
         { 2022, 415.63944, 421.86439 },
         { 2023, 417.93551, 424.99469 },
         { 2024, 420.27395, 428.19734 },
         { 2025, 422.65593, 431.47473 },
         { 2026, 425.07983, 434.82619 },
         { 2027, 427.53791, 438.24456 },
         { 2028, 430.0206, 441.7208 },
         { 2029, 432.5234, 445.25085 },
         { 2030, 435.04594, 448.83485 },
         { 2031, 437.58886, 452.47359 },
         { 2032, 440.13137, 456.177 },
         { 2033, 442.66419, 459.96398 },
         { 2034, 445.20699, 463.85181 },
         { 2035, 447.76978, 467.85003 },
         { 2036, 450.35539, 471.96047 },
         { 2037, 452.96337, 476.18237 },
         { 2038, 455.58649, 480.50799 },
         { 2039, 458.2152, 484.92724 },
         { 2040, 460.84499, 489.43545 },
         { 2041, 463.47549, 494.03235 },
         { 2042, 466.09336, 498.7297 },
         { 2043, 468.67807, 503.52959 },
         { 2044, 471.23389, 508.43266 },
         { 2045, 473.78031, 513.45614 },
         { 2046, 476.32819, 518.61062 },
         { 2047, 478.88085, 523.90006 },
         { 2048, 481.43826, 529.32418 },
         { 2049, 483.99308, 534.8752 },
         { 2050, 486.53532, 540.54279 },
         { 2051, 489.06035, 546.32201 },
         { 2052, 491.53558, 552.21189 },
         { 2053, 493.93186, 558.2122 },
         { 2054, 496.24365, 564.31311 },
         { 2055, 498.47436, 570.51669 },
         { 2056, 500.64502, 576.84343 },
         { 2057, 502.76788, 583.30471 },
         { 2058, 504.84729, 589.90539 },
         { 2059, 506.88408, 596.64656 },
         { 2060, 508.87135, 603.52045 },
         { 2061, 510.79905, 610.5165 },
         { 2062, 512.64717, 617.60526 },
         { 2063, 514.40151, 624.76367 },
         { 2064, 516.06461, 631.99471 },
         { 2065, 517.62854, 639.29052 },
         { 2066, 519.09606, 646.65274 },
         { 2067, 520.48828, 654.09843 },
         { 2068, 521.81772, 661.64491 },
         { 2069, 523.08871, 669.30474 },
         { 2070, 524.30217, 677.07762 },
         { 2071, 525.45089, 684.95429 },
         { 2072, 526.50901, 692.90196 },
         { 2073, 527.45707, 700.89416 },
         { 2074, 528.29593, 708.93159 },
         { 2075, 529.02718, 717.01548 },
         { 2076, 529.64253, 725.13597 },
         { 2077, 530.14419, 733.30667 },
         { 2078, 530.55341, 741.52368 },
         { 2079, 530.88313, 749.80466 },
         { 2080, 531.13797, 758.1823 },
         { 2081, 531.31935, 766.64451 },
         { 2082, 531.48991, 775.17446 },
         { 2083, 531.70213, 783.75141 },
         { 2084, 531.94208, 792.36578 },
         { 2085, 532.20472, 801.0188 },
         { 2086, 532.48672, 809.71464 },
         { 2087, 532.77553, 818.42214 },
         { 2088, 533.06978, 827.15719 },
         { 2089, 533.38792, 835.95594 },
         { 2090, 533.74072, 844.80471 },
         { 2091, 534.13086, 853.72536 },
         { 2092, 534.55754, 862.72597 },
         { 2093, 535.01142, 871.7768 },
         { 2094, 535.47962, 880.86435 },
         { 2095, 535.95487, 889.98162 },
         { 2096, 536.4351, 899.12407 },
         { 2097, 536.91986, 908.28871 },
         { 2098, 537.39855, 917.47137 },
         { 2099, 537.87136, 926.66527 },
         { 2100, 538.3583, 935.87437 },
      };
 
   int rcpNdx = 0;
   if (scenarioName == "MIROC" /* MIROC RCP 8.5*/) rcpNdx = 2;
   else if (scenarioName == "GFDL" /* GFDL RCP 4.5*/) rcpNdx = 1;
   else rcpNdx = 2; // HadGEM RCP 8.5 and default

   float co2_this_year = 0.f;
   int nYears = sizeof(CO2_ppm) / (3 * sizeof(float));
   if (calendarYear < START_YEAR) co2_this_year = (float)CO2_ppm[0][rcpNdx];
   else if (calendarYear >= (END_YEAR)) co2_this_year = (float)CO2_ppm[NYEARS - 1][rcpNdx];
   else co2_this_year = (float)CO2_ppm[calendarYear - START_YEAR][rcpNdx];

   return(co2_this_year);
   } // end of CO2byYear(int calendarYear, CString scenarioName)

