// Flow.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "flow.h"
#include <ScienceFcns.h>
#include "FlowInterface.h"
#include "FlowDlg.h"
#include <map.h>
#include <maplayer.h>
#include <math.h>
#include <fdataobj.h>
#include <idataobj.H>
#include <tinyxml.h>
#include <EnvEngine\EnvModel.h>
#include <UNITCONV.H>
#include <DATE.HPP>
#include <VideoRecorder.h>
#include <PathManager.h>
#include <GDALWrapper.h>
#include <GeoSpatialDataObj.h>
#include "AlgLib\ap.h"

#include <EnvInterface.h>
#include <EnvExtension.h>
#include <EnvEngine\EnvConstants.h>

//#include <EnvView.h>
#include <omp.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IDUlayer* gIDUs = NULL;
extern FlowProcess *gpFlow;

FlowModel *gpModel = NULL; // ??? shouldn't gpModel be an EnvModel instead of a FlowModel?
FlowModel* gFlowModel = NULL;

FILE* insolation_ofile;

SYSDATE EvapTrans::m_dateEvapTransLastExecuted = SYSDATE(0, 0, 0);
float EvapTrans::M_effBulkStomatalResistance = 1000.;
float EvapTrans::M_bulkStomatalResistance = 1000.;

MTDOUBLE HRU::m_mvDepthMelt = 0;  // volume of water in snow
MTDOUBLE HRU::m_mvDepthSWE_mm = 0;   // volume of ice in snow
MTDOUBLE HRU::m_mvTopSoilH2O_mm = 0;
MTDOUBLE HRU::m_mvShallowGW_mm;
MTDOUBLE HRU::m_mvDeepGW_mm;
MTDOUBLE HRU::m_mvCurrentPrecip = 0;
MTDOUBLE HRU::m_mvRainThrufall_mm = 0;
MTDOUBLE HRU::m_mvRainEvap_mm = 0;
MTDOUBLE HRU::m_mvSnowThrufall_mmSWE = 0;
MTDOUBLE HRU::m_mvSnowEvap_mmSWE = 0;
MTDOUBLE HRU::m_mvMelt_mm = 0;
MTDOUBLE HRU::m_mvRefreezing_mm = 0;
MTDOUBLE HRU::m_mvInfiltration_mm = 0;
MTDOUBLE HRU::m_mvRechargeToIrrigatedSoil_mm = 0;
MTDOUBLE HRU::m_mvRechargeToNonIrrigatedSoil_mm = 0;
MTDOUBLE HRU::m_mvRechargeTopSoil_mm = 0;
MTDOUBLE HRU::m_mvRechargeToUpperGW_mm = 0;
MTDOUBLE HRU::m_mvQ0_mm = 0;
MTDOUBLE HRU::m_mvQ2_mm = 0;
MTDOUBLE HRU::m_mvPercolation_mm = 0;
MTDOUBLE HRU::m_mvCurrentMinTemp = 0;
MTDOUBLE HRU::m_mvCurrentET = 0;
MTDOUBLE HRU::m_mvCurrentMaxET = 0;
MTDOUBLE HRU::m_mvCurrentSediment = 0;
MTDOUBLE HRU::m_mvHRU_TO_AQ_mm = 0;
MTDOUBLE HRU::m_mvCurrentRecharge = 0;
MTDOUBLE HRU::m_mvCurrentGwFlowOut = 0;

MTDOUBLE HRU::m_mvCurrentAirTemp = 0;
MTDOUBLE Reach::m_mvCurrentStreamFlow = 0;
MTDOUBLE Reach::m_mvCurrentStreamTemp = 0;
MTDOUBLE Reach::m_mvInstreamWaterRightUse = 0;
MTDOUBLE Reach::m_mvQ_DIV_WRQ = 0;
MTDOUBLE Reach::m_mvINSTRM_REQ = 0;
MTDOUBLE Reach::m_mvRESVR_H2O = 0;
MTDOUBLE Reach::m_mvCurrentTracer = 0;

MapLayer* Reach::pLayer = NULL;

MTDOUBLE HRU::m_mvCumET = 0;
MTDOUBLE HRU::m_mvCumRunoff = 0;
MTDOUBLE HRU::m_mvCumMaxET = 0;
MTDOUBLE HRU::m_mvCumP = 0;
MTDOUBLE HRU::m_mvElws = 0;

MTDOUBLE HRU::m_mvCumRecharge = 0;
MTDOUBLE HRU::m_mvCumGwFlowOut = 0;

MTDOUBLE HRULayer::m_mvWDepth = 0;
MTDOUBLE HRULayer::m_mvWC = 0;
MTDOUBLE HRULayer::m_mvESV = 0;

int ParamTable::CreateMap( void )
   {
   if ( m_pTable == NULL )
      return -1;

   VData data;
   int rows = (int) m_pTable->GetRowCount();

   for ( int i=0; i < m_pTable->GetRowCount(); i++ )
      {
      m_pTable->Get( 0, i, data );
      m_lookupMap.SetAt( data, i );
      }

   return rows;
   }


bool ParamTable::Lookup( VData key, int col, int &value )
   {
   if ( m_pTable == NULL )
      return false;

   int row = -1;

   if ( m_lookupMap.Lookup( key, row ) )
      return m_pTable->Get( col, row, value );

   return false;
   }


bool ParamTable::Lookup( VData key, int col, float &value )
   {
   if ( m_pTable == NULL )
      return false;

   int row = -1;

   if ( m_lookupMap.Lookup( key, row ) )
      return m_pTable->Get( col, row, value );

   return false;
   }


bool ModelOutput::InitModelOutput(MapLayer * pIDUlayer)
   {
   if (gpModel->m_pQE_IDU == NULL)
      { // Do this only on the first IDU domain variable encountered
      gpModel->m_pQE_IDU = new QueryEngine(pIDUlayer); ASSERT(gpModel->m_pQE_IDU != NULL);
      if (gpModel->m_pME_IDU == NULL) { gpModel->m_pME_IDU = new MapExprEngine(pIDUlayer); ASSERT(gpModel->m_pME_IDU != NULL); }
      } // end of if (gpModel->m_pQE_IDU == NULL)

   return(true);
   } // end of InitModelOutput()


bool ModelOutput::InitIDUdomain(MapLayer *pIDUlayer)
   {
   if (m_queryStr.GetLength() <= 0) return(true);

   m_pQuery = gpModel->m_pQE_IDU->ParseQuery(m_queryStr, 0, "Model Output Query");
   if (m_pQuery == NULL)
      {
      CString msg("InitIDUdomain(): Error parsing query expression '");
      msg += m_queryStr;
      msg += "' - the output will be ignored...";
      Report::ErrorMsg(msg);
      m_inUse = false;
      }

   m_pMapExpr = gpModel->m_pME_IDU->AddExpr(m_name, m_exprStr, m_queryStr); ASSERT(m_pMapExpr != NULL);

   bool ok = gpModel->m_pME_IDU->Compile(m_pMapExpr);
   if (!ok)
      {
      CString msg("InitIDUdomain(): Unable to compile map expression ");
      msg += m_exprStr;
      msg += " for <output> '";
      msg += m_name;
      msg += "'.  The expression will be ignored";
      Report::ErrorMsg(msg);
      m_inUse = false;
      }

   return(true);
   } // end of InitIDUdomain()


bool ModelOutput::InitReachDomain(MapLayer *pReachLayer)
   {
   if (m_queryStr.GetLength() <= 0) return(true);

   if (gpModel->m_pQE_Reach == NULL)
   { // Do this only on the first Reach domain variable encountered
      gpModel->m_pQE_Reach = new QueryEngine(pReachLayer); ASSERT(gpModel->m_pQE_Reach != NULL);
      if (gpModel->m_pME_Reach == NULL) { gpModel->m_pME_Reach = new MapExprEngine(pReachLayer); ASSERT(gpModel->m_pME_Reach != NULL); }

      gpModel->AddModelVar(gpModel->m_pME_Reach, "Streamflow (mm)", "reachOutflow", &Reach::m_mvCurrentStreamFlow);
      gpModel->AddModelVar(gpModel->m_pME_Reach, "Stream Temp (C)", "reachTemp", &Reach::m_mvCurrentStreamTemp);
      //gpModel->AddModelVar(gpModel->m_pME_Reach, "INSTRM_WRQ", "INSTRM_WRQ", &Reach::m_mvInstreamWaterRightUse);
      //gpModel->AddModelVar(gpModel->m_pME_Reach, "Q_DIV_WRQ", "Q_DIV_WRQ", &Reach::m_mvQ_DIV_WRQ);
      //gpModel->AddModelVar(gpModel->m_pME_Reach, "INSTRM_REQ", "INSTRM_REQ", &Reach::m_mvINSTRM_REQ);
      gpModel->AddModelVar(gpModel->m_pME_Reach, "RESVR_H2O", "RESVR_H2O", &Reach::m_mvRESVR_H2O);
      gpModel->AddModelVar(gpModel->m_pME_Reach, "Streamflow", "esv_reachOutflow", &Reach::m_mvCurrentTracer);
   } // end of if (gpModel->m_pQE_Reach == NULL)

   m_pQuery = gpModel->m_pQE_Reach->ParseQuery(m_queryStr, 0, "Model Output Query");
   if (m_pQuery == NULL)
   {
      CString msg("InitReachDomain(): Error parsing query expression '");
      msg += m_queryStr;
      msg += "' - the output will be ignored...";
      Report::ErrorMsg(msg);
      m_inUse = false;
   }

   m_pMapExpr = gpModel->m_pME_Reach->AddExpr(m_name, m_exprStr, m_queryStr); ASSERT(m_pMapExpr != NULL);
   bool ok = gpModel->m_pME_Reach->Compile(m_pMapExpr);
   if (!ok)
   {
      CString msg("InitReachDomain(): Unable to compile map expression ");
      msg += m_exprStr;
      msg += " for <output> '";
      msg += m_name;
      msg += "'.  The expression will be ignored";
      Report::ErrorMsg(msg);
      m_inUse = false;
   }

   return(true);
   } // end of InitReachDomain()


bool ModelOutput::InitHRUdomain(MapLayer * pHRUlayer)
{
   if (m_queryStr.GetLength() <= 0) return(true);

   if (gpModel->m_pQE_HRU == NULL)
   { // Do this only on the first HRU domain variable encountered
      gpModel->m_pQE_HRU = new QueryEngine(pHRUlayer); ASSERT(gpModel->m_pQE_HRU != NULL);
      if (gpModel->m_pME_HRU == NULL) { gpModel->m_pME_HRU = new MapExprEngine(pHRUlayer); ASSERT(gpModel->m_pME_HRU != NULL); }


      // add model vars for HRU stuff - these are legal names in ModelOutput expressions
      //                     description          variable name    internal variable address, these are statics
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Melt Depth (mm)", "hruVolMelt", &HRU::m_mvDepthMelt);  //
      gpModel->AddModelVar(gpModel->m_pME_HRU, "SWE Depth (mm)", "hruVolSwe", &HRU::m_mvDepthSWE_mm);   //
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Topsoil moisture (mm)", "hruVolTopSoilH2O", &HRU::m_mvTopSoilH2O_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Shallow Groundwater (mm)", "hruVolSGW", &HRU::m_mvShallowGW_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Deep Groundwater (mm)", "hruVolDGW", &HRU::m_mvDeepGW_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Total Precip (mm)", "hruPrecip", &HRU::m_mvCurrentPrecip);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Rain Thrufall (mm)", "hruRainThrufall", &HRU::m_mvRainThrufall_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Rain Evap (mm)", "hruRainEvap", &HRU::m_mvRainEvap_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Snow Thrufall (mm)", "hruSnowThrufall", &HRU::m_mvSnowThrufall_mmSWE);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Snow Evap (mm)", "hruSnowEvap", &HRU::m_mvSnowEvap_mmSWE);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Air Temp (C)", "hruAirTemp", &HRU::m_mvCurrentAirTemp);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Min Temp (C)", "hruMinTemp", &HRU::m_mvCurrentMinTemp);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "ET (mm)", "hruET", &HRU::m_mvCurrentET);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Maximum ET (mm)", "hruMaxET", &HRU::m_mvCurrentMaxET);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Sediment", "hruSedimentOut", &HRU::m_mvCurrentSediment);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "HRU_TO_AQ (mm)", "HRU_TO_AQ", &HRU::m_mvHRU_TO_AQ_mm);

      gpModel->AddModelVar(gpModel->m_pME_HRU, "Snowmelt (mm SWE)", "hruMelt", &HRU::m_mvMelt_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Refreezing (mm)", "hruRefreezing", &HRU::m_mvRefreezing_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Infiltration (mm)", "hruInfiltration", &HRU::m_mvInfiltration_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Recharge to Irrigated Soil (mm)", "hruRechargeIrrigated", &HRU::m_mvRechargeToIrrigatedSoil_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Recharge to NonIrrigated Soil (mm)", "hruRechargeNonIrrigated", &HRU::m_mvRechargeToNonIrrigatedSoil_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Recharge to Plant Accessible Soil (mm)", "hruRechargeTopSoil", &HRU::m_mvRechargeTopSoil_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Recharge to Upper Groundwater (mm)", "hruRechargeUpperGW", &HRU::m_mvRechargeToUpperGW_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Q0 (mm)", "hruQ0", &HRU::m_mvQ0_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Q2 (mm)", "hruQ2", &HRU::m_mvQ2_mm);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Percolation (mm)", "hruPercolation", &HRU::m_mvPercolation_mm);

      gpModel->AddModelVar(gpModel->m_pME_HRU, "Recharge (mm)", "hruGWRecharge", &HRU::m_mvCurrentRecharge);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "GW Loss (mm)", "hruGWOut", &HRU::m_mvCurrentGwFlowOut);

      gpModel->AddModelVar(gpModel->m_pME_HRU, "Cumu. Rainfall (mm)", "hruCumP", &HRU::m_mvCumP);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Cumu. Runoff (mm)", "hruCumRunoff", &HRU::m_mvCumRunoff);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Cumu. ET (mm)", "hruCumET", &HRU::m_mvCumET);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Cumu. Max ET (mm)", "hruCumMaxET", &HRU::m_mvCumMaxET);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Cumu. GW Out (mm)", "hruCumGWOut", &HRU::m_mvCumGwFlowOut);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Cumu. GW Recharge (mm)", "hruCumGWRecharge", &HRU::m_mvCumRecharge);

      gpModel->AddModelVar(gpModel->m_pME_HRU, "WC", "hruLayerWC", &HRULayer::m_mvWC);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "WC", "hruLayerWDepth", &HRULayer::m_mvWDepth);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Elws", "hruElws", &HRU::m_mvElws);
      gpModel->AddModelVar(gpModel->m_pME_HRU, "Extra State Variable Conc", "esv_hruLayer", &HRULayer::m_mvESV);
   } // end of if (gpModel->m_pQE_HRU == NULL)

   m_pQuery = gpModel->m_pQE_HRU->ParseQuery(m_queryStr, 0, "Model Output Query");
   if (m_pQuery == NULL)
   {
      CString msg("InitHRUdomain(): Error parsing query expression '");
      msg += m_queryStr;
      msg += "' - the output will be ignored...";
      Report::ErrorMsg(msg);
      m_inUse = false;
   }

   m_pMapExpr = gpModel->m_pME_HRU->AddExpr(m_name, m_exprStr, m_queryStr); ASSERT(m_pMapExpr != NULL);
   bool ok = gpModel->m_pME_HRU->Compile(m_pMapExpr);
   if (!ok)
   {
      CString msg("InitHRUdomain(): Unable to compile map expression ");
      msg += m_exprStr;
      msg += " for <output> '";
      msg += m_name;
      msg += "'.  The expression will be ignored";
      Report::ErrorMsg(msg);
      m_inUse = false;
   }

   return(true);
} // end of InitHRUdomain()


bool ModelOutput::InitXHRUdomain(MapLayer * pXHRULayer)
{
   if (m_queryStr.GetLength() <= 0) return(true);

   if (gpModel->m_pQE_XHRU == NULL)
   { // Do this only on the first XHRU domain variable encountered
      gpModel->m_pQE_XHRU = new QueryEngine(pXHRULayer); ASSERT(gpModel->m_pQE_XHRU != NULL);
      if (gpModel->m_pME_XHRU == NULL) { gpModel->m_pME_XHRU = new MapExprEngine(pXHRULayer); ASSERT(gpModel->m_pME_XHRU != NULL); }
   } // end of if (gpModel->m_pQE_XHRU == NULL)

   m_pQuery = gpModel->m_pQE_XHRU->ParseQuery(m_queryStr, 0, "Model Output Query");
   if (m_pQuery == NULL)
   {
      CString msg("InitXHRUdomain(): Error parsing query expression '");
      msg += m_queryStr;
      msg += "' - the output will be ignored...";
      Report::ErrorMsg(msg);
      m_inUse = false;
   }

   m_pMapExpr = gpModel->m_pME_XHRU->AddExpr(m_name, m_exprStr, m_queryStr); ASSERT(m_pMapExpr != NULL);
   bool ok = gpModel->m_pME_XHRU->Compile(m_pMapExpr);
   if (!ok)
   {
      CString msg("InitXHRUdomain(): Unable to compile map expression ");
      msg += m_exprStr;
      msg += " for <output> '";
      msg += m_name;
      msg += "'.  The expression will be ignored";
      Report::ErrorMsg(msg);
      m_inUse = false;
   }

   return(true);
} // end of InitXHRUdomain()


bool ModelOutput::InitLinkDomain(MapLayer * pLinkLayer)
{
   if (m_queryStr.GetLength() <= 0) return(true);

   if (gpModel->m_pQE_Link == NULL)
   { // Do this only on the first Link domain variable encountered
      gpModel->m_pQE_Link = new QueryEngine(pLinkLayer); ASSERT(gpModel->m_pQE_Link != NULL);
      if (gpModel->m_pME_Link == NULL) { gpModel->m_pME_Link = new MapExprEngine(pLinkLayer); ASSERT(gpModel->m_pME_Link != NULL); }
   } // end of if (gpModel->m_pQE_Link == NULL)

   m_pQuery = gpModel->m_pQE_Link->ParseQuery(m_queryStr, 0, "Model Output Query");
   if (m_pQuery == NULL)
   {
      CString msg("InitLinkDomain(): Error parsing query expression '");
      msg += m_queryStr;
      msg += "' - the output will be ignored...";
      Report::ErrorMsg(msg);
      m_inUse = false;
   }

   m_pMapExpr = gpModel->m_pME_Link->AddExpr(m_name, m_exprStr, m_queryStr); ASSERT(m_pMapExpr != NULL);
   bool ok = gpModel->m_pME_Link->Compile(m_pMapExpr);
   if (!ok)
   {
      CString msg("InitLinkDomain(): Unable to compile map expression ");
      msg += m_exprStr;
      msg += " for <output> '";
      msg += m_name;
      msg += "'.  The expression will be ignored";
      Report::ErrorMsg(msg);
      m_inUse = false;
   }

   return(true);
} // end of InitLinkDomain()


bool ModelOutput::InitNodeDomain(MapLayer * pNodeLayer)
{
   if (m_queryStr.GetLength() <= 0) return(true);

   if (gpModel->m_pQE_Node == NULL)
   { // Do this only on the first Node domain variable encountered
      gpModel->m_pQE_Node = new QueryEngine(pNodeLayer); ASSERT(gpModel->m_pQE_Node != NULL);
      if (gpModel->m_pME_Node == NULL) { gpModel->m_pME_Node = new MapExprEngine(pNodeLayer); ASSERT(gpModel->m_pME_Node != NULL); }
   } // end of if (gpModel->m_pQE_Node == NULL)

   m_pQuery = gpModel->m_pQE_Node->ParseQuery(m_queryStr, 0, "Model Output Query");
   if (m_pQuery == NULL)
   {
      CString msg("InitNodeDomain(): Error parsing query expression '");
      msg += m_queryStr;
      msg += "' - the output will be ignored...";
      Report::ErrorMsg(msg);
      m_inUse = false;
   }

   m_pMapExpr = gpModel->m_pME_Node->AddExpr(m_name, m_exprStr, m_queryStr); ASSERT(m_pMapExpr != NULL);
   bool ok = gpModel->m_pME_Node->Compile(m_pMapExpr);
   if (!ok)
   {
      CString msg("InitNodeDomain(): Unable to compile map expression ");
      msg += m_exprStr;
      msg += " for <output> '";
      msg += m_name;
      msg += "'.  The expression will be ignored";
      Report::ErrorMsg(msg);
      m_inUse = false;
   }

   return(true);
} // end of InitNodeDomain()


bool ModelOutput::InitSubcatchDomain(MapLayer * pSubcatchLayer)
{
   if (m_queryStr.GetLength() <= 0) return(true);

   if (gpModel->m_pQE_Subcatch == NULL)
   { // Do this only on the first Subcatch domain variable encountered
      gpModel->m_pQE_Subcatch = new QueryEngine(pSubcatchLayer); ASSERT(gpModel->m_pQE_Subcatch != NULL);
      if (gpModel->m_pME_Subcatch == NULL) { gpModel->m_pME_Subcatch = new MapExprEngine(pSubcatchLayer); ASSERT(gpModel->m_pME_Subcatch != NULL); }
   } // end of if (gpModel->m_pQE_Subcatch == NULL)

   m_pQuery = gpModel->m_pQE_Subcatch->ParseQuery(m_queryStr, 0, "Model Output Query");
   if (m_pQuery == NULL)
   {
      CString msg("InitSubcatchDomain(): Error parsing query expression '");
      msg += m_queryStr;
      msg += "' - the output will be ignored...";
      Report::ErrorMsg(msg);
      m_inUse = false;
   }

   m_pMapExpr = gpModel->m_pME_Subcatch->AddExpr(m_name, m_exprStr, m_queryStr); ASSERT(m_pMapExpr != NULL);
   bool ok = gpModel->m_pME_Subcatch->Compile(m_pMapExpr);
   if (!ok)
   {
      CString msg("InitSubcatchDomain(): Unable to compile map expression ");
      msg += m_exprStr;
      msg += " for <output> '";
      msg += m_name;
      msg += "'.  The expression will be ignored";
      Report::ErrorMsg(msg);
      m_inUse = false;
   }

   return(true);
} // end of InitSubcatchDomain()


ModelOutputGroup &ModelOutputGroup::operator = ( ModelOutputGroup &mog )
   {
   m_name = mog.m_name; 
   m_pDataObj = NULL; 
   m_moCountIdu = mog.m_moCountIdu;
   m_moCountHru = mog.m_moCountHru; 
   m_moCountHruLayer = mog.m_moCountHruLayer; 
   m_moCountReach = mog.m_moCountReach; 
 
   // copy each model output
   for ( int i=0; i < (int) mog.GetSize(); i++ )
      {
      ModelOutput *pOutput = new ModelOutput( *(mog.GetAt( i )) );
      ASSERT( pOutput != NULL );
      this->Add( pOutput );
      }  
   
   return *this;
   }


///////////////////////////////////////////////////////////////////////////////
// F L U X  I N F O 
///////////////////////////////////////////////////////////////////////////////

FluxInfo::FluxInfo( void )
: m_type( FT_UNDEFINED )
, m_dataSource( FS_UNDEFINED )
, m_sourceLocation( FL_UNDEFINED )
, m_sinkLocation( FL_UNDEFINED )
, m_sourceLayer( -1 )
, m_sinkLayer( -1 )
, m_pSourceQuery( NULL )
, m_pSinkQuery( NULL )
, m_pFluxFn( NULL )
, m_pInitFn( NULL )
, m_pInitRunFn( NULL )
, m_pEndRunFn( NULL )
, m_hDLL( NULL )
, m_pFluxData( NULL )
, m_col( -1 )
, m_use( true )
, m_totalFluxRate( 0 )      // instantaneous, reflects total for the given time step
, m_annualFlux( 0 )         // over the current year
, m_cumFlux( 0 )            // over the run
   { }


FluxInfo::~FluxInfo()
   {
   if ( m_pFluxData != NULL )
      delete m_pFluxData;

   if ( m_hDLL )
      AfxFreeLibrary( m_hDLL );
   }


///////////////////////////////////////////////////////////////////////////////
// F L U X 
///////////////////////////////////////////////////////////////////////////////

float Flux::Evaluate( FlowContext *pContext )
   {
   ASSERT( m_pFluxInfo != NULL );
   ASSERT( m_pFluxInfo->m_use != false );
   
   clock_t start = clock();

   switch( m_pFluxInfo->m_dataSource )
      {
      case FS_FUNCTION:
         ASSERT( m_pFluxInfo->m_pFluxFn != NULL );
         m_value = m_pFluxInfo->m_pFluxFn( pContext );
         break;

      case FS_DATABASE:
         ASSERT( m_pFluxInfo->m_pFluxData != NULL );
         m_value = m_pFluxInfo->m_pFluxData->IGet( pContext->time, m_pFluxInfo->m_col, IM_LINEAR );
         break;

      default:
         ASSERT( 0 );
      }

   m_pFluxInfo->m_totalFluxRate += m_value;

   clock_t finish = clock();
   float duration = (float)(finish - start) / CLOCKS_PER_SEC;   
   gpModel->m_totalFluxFnRunTime += duration;   

   return m_value;
   }


///////////////////////////////////////////////////////////////////////////////
// F L U X C O N T A I N E R 
///////////////////////////////////////////////////////////////////////////////

int FluxContainer::AddFlux( FluxInfo *pFluxInfo, StateVar *pStateVar )
   {
   ASSERT( pFluxInfo != NULL );
   ASSERT( pStateVar != NULL );
   
   Flux *pFlux = new Flux( pFluxInfo, pStateVar );
   ASSERT( pFlux != NULL );
   return (int) m_fluxArray.Add( pFlux );
   }


int FluxContainer::RemoveFlux( Flux *pFlux, bool doDelete ) 
   { 
   for ( int i=0; i < (int) m_fluxArray.GetSize(); i++ ) 
      {
      if ( m_fluxArray[ i ] == pFlux ) 
         { 
         if ( doDelete )
            m_fluxArray.RemoveAt( i );
         else
            m_fluxArray.RemoveAtNoDelete( i );

         return i; 
         }
      }
   return -1; 
   }


///////////////////////////////////////////////////////////////////////////////
// H R U L A Y E R  
///////////////////////////////////////////////////////////////////////////////

HRULayer::HRULayer(HRU* pHRU)
   : FluxContainer()
   , StateVarContainer()
   , m_volumeWater(0.0f)
   , m_HRUareaFraction(1.0f)
   , m_contributionToReach(0.0f)
   , m_verticalDrainage(0.0f)
   , m_horizontalExchange(0.0f)
   , m_wc(0.0f)
   , m_volumeLateralInflow(0.0f)
   , m_volumeLateralOutflow(0.0f)
   , m_volumeVerticalInflow(0.0f)
   , m_volumeVerticalOuflow(0.0f)
   , m_pHRU(pHRU)
   , m_layer(-1)
   , m_type(HT_SOIL)
   , m_soilThickness_m(1.0f)
{
   ASSERT( m_pHRU != NULL );
}



HRULayer &HRULayer::operator = ( HRULayer &l )
   {
   m_volumeWater          = l.m_volumeWater;
   m_HRUareaFraction = l.m_HRUareaFraction;
   m_contributionToReach  = l.m_contributionToReach;
   m_verticalDrainage     = l.m_verticalDrainage;
   m_horizontalExchange   = l.m_verticalDrainage;
   m_wc                   = l.m_wc;
   m_volumeLateralInflow  = l.m_volumeLateralInflow ;        // m3/day
   m_volumeLateralOutflow = l.m_volumeLateralOutflow;
   m_volumeVerticalInflow = l.m_volumeVerticalInflow;
   m_volumeVerticalOuflow = l.m_volumeVerticalOuflow;
   m_soilThickness_m = l.m_soilThickness_m;

   // general info
   m_pHRU  = l.m_pHRU;
   m_layer = l.m_layer;
   m_type  = l.m_type;

   return *this;
   }


Reach *HRULayer::GetReach( void )
   {
   return m_pHRU->m_pCatchment->m_pReach;
   }

   

///////////////////////////////////////////////////////////////////////////////
// H R U 
///////////////////////////////////////////////////////////////////////////////

HRU::HRU( void ) 
   : m_id( -1 )
   , m_HRUtotArea_m2(0.0f)
   , m_HRUeffArea_m2(0.0f)
   , m_precip_yr(0.0f)
   , m_precip_wateryr(0.0f)
   , m_precip_10yr( 20 )
   , m_precipCumApr1( 0.0f )
   , m_temp_yr(0.0f)
   , m_rainfall_yr( 0.0f )
   , m_snowfall_yr( 0.0f )
   , m_gwRecharge_yr( 0.0f)       // mm
   , m_gwFlowOut_yr(0.0f)
   , m_areaIrrigated(0.0f)
   , m_temp_10yr( 20 )
   , m_depthMelt( 0.0f )
   , m_depthSWE( 0.0f )
   , m_depthApr1SWE_yr( 0.0f )
   , m_apr1SWE_10yr( 20 )
   , m_centroid(0.0f,0.0f)
   , m_elevation(50.0f)
   , m_currentPrecip(0.0f)
   , m_rainThrufall_mm(0.0f)
   , m_snowThrufall_mm(0.0f)
   , m_rainEvap_mm(0.f)
   , m_snowEvap_mm(0.f)
   , m_melt_mm(0.f)
   , m_refreezing_mm(0.f)
   , m_infiltration_mm(0.f)
   , m_rechargeToIrrigatedSoil_mm(0.f)
   , m_rechargeToNonIrrigatedSoil_mm(0.f)
   , m_rechargeTopSoil_mm(0.f)
   , m_rechargeToUpperGW_mm(0.f)
   , m_Q0_mm(0.f)
   , m_Q2_mm(0.f)
   , m_percolation_mm(0.f)
   , m_currentAirTemp(0.0f)
   , m_currentMinTemp(0.0f)
   , m_currentET(0.0f)
   , m_maxET_yr(0.0f)
   , m_et_yr(0.0f)
   , m_aquifer_recharge_mm(0.0f)
   , m_aquifer_recharge_yr_mm(0.0f)
   , m_currentRunoff(0.0f)
   , m_currentGWRecharge(0.0f)
   , m_currentGWFlowOut(0.0f)
   , m_runoff_yr(0.0f)
   , m_fracIrrigated(0.0)
   , m_meanLAI(0.0f)
   , m_currentMaxET(0.0f)
   , m_currentSediment(0.0f)
   , m_climateIndex( -1 )
   , m_climateRow( -1 )
   , m_climateCol( -1 )
   , m_demRow(-1)
   , m_demCol(-1)
   , m_elws(0.0f)
   , m_soilTemp(10.0f)
   , m_biomass(0.0f)
   , m_pCatchment( NULL ) 
   , m_infiltrationFromStandingH2O_m3(0)
   , m_addedVolume_m3(0)
   { }

HRU::~HRU(void)
{
   //if (m_waterFluxArray != NULL) 
   //   delete[] m_waterFluxArray; 
   
   //if (m_pCatchment) 
   //   delete m_pCatchment; 
}


bool HRU::DistributeToReaches(float amount)
{
   int hru_reach_count = (int)m_reachNdxArray.GetSize();
   ASSERT(hru_reach_count >= 1);
   float frac_tot = 0.;
   for (int hru_reach_ndx = 0; hru_reach_ndx < hru_reach_count; hru_reach_ndx++)
   {
      int reach_layer_ndx = m_reachNdxArray[hru_reach_ndx];
      float frac = 0; gpModel->m_pReachLayer->GetData(reach_layer_ndx, gpModel->m_colReachHRU_FRAC, frac);
      Reach * pReach = gpModel->GetReachFromStreamIndex(reach_layer_ndx);
      pReach->AddFluxFromGlobalHandler(-(frac * amount));
      frac_tot += frac;
   } // end of loop thru reaches associated with this HRU
   ASSERT(abs(1 - frac_tot) < 1e-6);

   return(true);
} // end of HRU::DistributeToReaches()


int HRU::AddLayers( int soilLayerCount, int snowLayerCount, int vegLayerCount, float initWaterContent, float initTemperature, bool grid )
   {
   HRULayer hruLayer( this );

   int layer = 0;

   int i_veg = 0;
   while (i_veg < vegLayerCount)
      {
      HRULayer *pHRULayer = new HRULayer( hruLayer );
      ASSERT( pHRULayer != NULL );

      m_layerArray.Add( pHRULayer );
      
      m_layerArray[layer]->m_layer = layer;
      m_layerArray[layer]->m_type = HT_VEG;
      layer++; i_veg++;
      }

   int i_snow = 0;
   while (i_snow < snowLayerCount)
      {
      HRULayer *pHRULayer = new HRULayer( hruLayer );
      ASSERT( pHRULayer != NULL );

      m_layerArray.Add( pHRULayer );
      m_layerArray[layer]->m_layer = layer;
      m_layerArray[layer]->m_type = HT_SNOW;
      layer++; i_snow++;
      }

   int i_soil = 0;
   while (i_soil < soilLayerCount)
      {
      HRULayer *pHRULayer = new HRULayer( hruLayer );
      ASSERT( pHRULayer != NULL );
 
      m_layerArray.Add( pHRULayer );
      m_layerArray[layer]->m_layer = layer;
      m_layerArray[layer]->m_type = HT_SOIL;

      if (gpModel->m_hruLayerDepths.GetCount()>0)
         m_layerArray[layer]->m_soilThickness_m = (float) atof(gpModel->m_hruLayerDepths[i_soil]);

      if (gpModel->m_initWaterContent.GetCount()==1)
         m_layerArray[layer]->m_volumeWater=atof(gpModel->m_initWaterContent[0]);
      else if (gpModel->m_initWaterContent.GetCount()>1)
         {
         ASSERT(gpModel->m_hruLayerDepths.GetCount()>0);//if this asserts, it probably means you did not include the same number of initial watercontents as you did layer depths.  Check your xml file (catchments tag)
         m_layerArray[layer]->m_volumeWater=atof(gpModel->m_initWaterContent[i_soil])*pHRULayer->m_soilThickness_m*0.4f*m_HRUeffArea_m2;
         }

      layer++; i_soil++;
      }

   for (int i=0;i<m_layerArray.GetSize();i++)
      {
      HRULayer *pLayer = m_layerArray[i];
      if (!grid) //if it is a grid, the water flux array is allocated by BuildCatchmentFromGrids.  
         {
         pLayer->m_waterFluxArray.SetSize( 7 );
         for (int j=0; j < 7; j++)
            pLayer->m_waterFluxArray[j]=0.0f;
         }
      }

   return (int) m_layerArray.GetSize();
   }


///////////////////////////////////////////////////////////////////////////////
// C A T C H M E N T 
///////////////////////////////////////////////////////////////////////////////

Catchment::Catchment( void ) 
: m_id( -1 )
, m_area( 0 )
, m_currentAirTemp( 0 )
, m_currentPrecip( 0 )
, m_currentThroughFall( 0 )
, m_currentSnow( 0 )
, m_currentET( 0 )
, m_currentGroundLoss( 0 )
, m_meltRate( 0 )
, m_contributionToReach( 0.0f )    // m3/day
, m_pDownslopeCatchment( NULL )
, m_pReach( NULL )
, m_pGW( NULL )
   { }


///////////////////////////////////////////////////////////////////////////////
// R E A C H 
///////////////////////////////////////////////////////////////////////////////
  
Reach::Reach(  )
: ReachNode()
, m_wdRatio( 10.0f )
, m_cumUpstreamArea( 0 )
, m_cumUpstreamLength( 0 )
, m_climateIndex( -1 )
, m_alpha( 1.0f)
, m_beta( 3/5.0f)
, m_n( DEFAULT_MANNING_N )
, m_meanYearlyDischarge(0.0f)
, m_maxYearlyDischarge(0.0f)
, m_sumYearlyDischarge(0.0f)
, m_isMeasured(false)
, m_pReservoir( NULL )
, m_pStageDischargeTable( NULL )
, m_instreamWaterRightUse ( 0.0f )
, m_availableDischarge( 0.0f )
, m_IDUndxForReach(-1)
, m_reachAddedVolumeWP(0.,0.)
, m_addedDischarge_cms(0.)
, m_topo(0,0,0,0,44.21228,-122.25528)
, m_wetlNdx(-1)
, m_q2wetl_cms(0.)
   { }


Reach::~Reach( void )
   {
   if ( m_pStageDischargeTable != NULL )
      delete m_pStageDischargeTable;
   }


double Reach::NominalLowFlow_cms()
{
   int scalar = m_streamOrder >= 2 ? m_streamOrder - 1 : 1;
   double reach_nominal_low_flow_cms = NOMINAL_LOW_FLOW_CMS * scalar * scalar * scalar; // ??? a placeholder
   return(reach_nominal_low_flow_cms);
} // end of NominalLowFlow_cms()


double Reach::NominalMinWidth_m()
{
   double reach_min_width_m = (double)(this->m_streamOrder * this->m_streamOrder); // ??? a placeholder
   return(reach_min_width_m);
} // end of SetNominalSubreachGeometry()


double Reach::SubreachNetRad_kJ(int subreachNdx)
// Totals up the incoming shortwave and outgoing longwave from the stream segment parts corresponding to this subreach.
{
   ReachSubnode* pSubreach = GetReachSubnode(subreachNdx);
   double net_rad_kJ = pSubreach->m_sw_kJ - pSubreach->m_lw_kJ;
   return(net_rad_kJ);
} // end of SubReachNetRad_kJ()


double Reach::Evap_m_s(int subreachNdx, double swIn_W_m2, double lwOut_W_m2, double tempAir_degC, double ws_m_sec, double sphumidity)
// returns evaporation rate in m3H2O/m2/sec, i.e. m/sec
{ // Implements eq. 2-105 for evaporation rate on p. 61 of Boyd & Kasper
   // The evap rate is a function of shortwave flux, longwave flux, aerodynamic evaporation, the air temperature,
   // the water temperature, and the relative humidity.  The equation also uses the latent heat of vaporization,
   // the psychrometric constant, and the slope of the saturation vapor v. air temperature curve.  The
   // aerodynamic evaporation is a function of the windspeed.
   // Eq. 2-105 is for evap rate in m3H2O/m2/sec, i.e. m/sec
   ReachSubnode * pSubreach = this->GetReachSubnode(subreachNdx);
   double temp_H2O_degC = pSubreach->m_waterParcel.WaterTemperature();
   double L_e_J_kg = LatentHeatOfVaporization_MJ_kg(temp_H2O_degC) * 1.e6;
   double e_s = WaterParcel::SatVP_mbar(temp_H2O_degC);
   double e_a = WaterParcel::SatVP_mbar(tempAir_degC);
   double E_a = 0.; // ??? aerodynamic evaporation, m/s
   double gamma = 66. / PA_PER_MBAR; // psychrometric constant, mbar/degC, ~= 0.066 kPa/degK = 66 Pa/degC
   double delta = (e_s - e_a) / (temp_H2O_degC - tempAir_degC); // slope of the saturation vapor v. air temperature curve
   double numerator = ((swIn_W_m2 - lwOut_W_m2) / (DENSITY_H2O * L_e_J_kg)) * delta + E_a * gamma;
   double evap_m_s = numerator / (delta + gamma);
   if (evap_m_s < 0.) evap_m_s = 0.;

   return(evap_m_s);
} // end of Evap_m_s()


double Reach::Evap_m_s(double tempH2O_degC, double swIn_W_m2, double lwOut_W_m2, double tempAir_degC, double ws_m_sec, double sphumidity)
// returns evaporation rate in m3H2O/m2/sec, i.e. m/sec
{ // Implements equations 2-105 (evaporation rate), 2-106 (slope of the saturation vapor v. air temperature curve), 
   // 2-108 (aerodynamic evaporation), and 2-109 (wind function) on pp. 61-2 of Boyd & Kasper.
   // The evap rate is a function of shortwave flux, longwave flux, aerodynamic evaporation, the air temperature,
   // the water temperature, and the relative humidity.  The equation also uses the latent heat of vaporization,
   // the psychrometric constant, and the slope of the saturation vapor v. air temperature curve.  The
   // aerodynamic evaporation is a function of the windspeed.
   // Eq. 2-105 is for evap rate in m3H2O/m2/sec, i.e. m/sec

   double net_incoming_rad_W_m2 = swIn_W_m2 - lwOut_W_m2;
   if (net_incoming_rad_W_m2 <= 0) return(0);

   double L_e_J_kg = LatentHeatOfVaporization_MJ_kg(tempH2O_degC) * 1.e6;
   double e_s = WaterParcel::SatVP_mbar(tempH2O_degC);
   double e_a = WaterParcel::SatVP_mbar(tempAir_degC);
   double E_a = 3.083e-9 + 5.845e-9 * ws_m_sec; // aerodynamic evaporation, m/s, uses coefficients from Bowie et al. 1985 cited on p. 62 of Boyd & Kasper
   double gamma = 66. / PA_PER_MBAR; // psychrometric constant, mbar/degC, ~= 0.066 kPa/degK = 66 Pa/degC

   double delta = -1; // slope of the saturation vapor v. air temperature curve. By definition, = d/dtemperature(saturation vp(temperature))
   // Use eq. 2-106 on p. 61 of Boyd & Kasper
   double Ta = tempAir_degC;
   double first_term = 6.1275 * exp(17.27 * Ta / (237.3 + Ta));
   double second_term = 6.1275 * exp(17.27 * (Ta - 1.) / (237.3 + (Ta - 1.)));
   delta = first_term - second_term;

   double numerator = (net_incoming_rad_W_m2 / (DENSITY_H2O * L_e_J_kg)) * delta + E_a * gamma;
   double evap_m_s = numerator / (delta + gamma);
   if (evap_m_s < 0.) evap_m_s = 0.;

   return(evap_m_s);
} // end of Evap_m_s(tempH2O_degC, ...)


/*
double Reach::LatentHeatOfVaporization_J_kg(double temp_H2O_degC) // returns J/kg
{ // Implements eq. 2-95 on p. 59 of Boyd & Kasper
   // Eq. 2-95 on p.59 of Boyd & Kasper seems wrong. The latent heat of vaporization should decrease as water temperature increases.
   double latent_heat_J_kg = 1000. * (2501.4 + (1.83 + temp_H2O_degC));
   return(latent_heat_J_kg);
} // end of LatentHeadOfEvaporation()
*/

double Reach::LatentHeatOfVaporization_MJ_kg(double temp_H2O_degC) // returns MJ/kg
{ // Implements eq. 7-8 on p. 274 of Dingman
   double latent_heat_MJ_kg = 2.50 - (2.36e-3 * temp_H2O_degC);
   return(latent_heat_MJ_kg);
} // end of LatentHeadOfEvaporation()


double FlowModel::VegDensity(double lai)
{
   double veg_density = 1. - exp(-BEERS_LAW_K * lai);
   return(veg_density);
} // end of VegDensity()


double TopoSetting::OpticalAirMassThickness(double solarElev_deg)
// Uses eq. 2-31, p.39 in Boyd & Kasper
{
   double M_A = (35. * exp(-0.0001184 * m_elev_m)) /
      sqrt(1224. * sin(solarElev_deg * PI / 180.) + 1.);

   return(M_A);
} // end of OpticalAirMassThickness()


double TopoSetting::DiffuseFrac(double C_I, int jday0)
{ // Boyd & Kasper eq. 2-35, p. 39.
   // ??? for C_I ~0.8 this produces non-physical diffuse fractions < 0
   int JD = jday0 + 1; // Julian Day as in Boyd in Kasper (aka jday1 in CW3M)
   
   double D_F = (0.938 + 1.071 * C_I) - (5.14 * C_I * C_I) + (2.98 * C_I * C_I * C_I)
      - (sin(2. * PI * (JD - 40) / 365)) * (0.009 - 0.078 * C_I);

   if (D_F < 0.) D_F = 0.;
   return(D_F);
} // end of DiffuseFrac()


double TopoSetting::VegElevEW_deg(double direction_deg, double reach_width_m, double veg_ht_m)
{
   double veg_elev_deg = 0.;
   double eff_dir_deg = direction_deg;
   if (eff_dir_deg > 180.) eff_dir_deg -= 180.;

   double hw_m = reach_width_m / 2.;

   double theta_deg = abs(90. - eff_dir_deg);
   double sin_theta = sin(theta_deg * PI / 180.);
   ASSERT(sin_theta > 0.); if (sin_theta <= 0.) sin_theta = 0.;
   if (sin_theta == 0.) veg_elev_deg = 0.; // center line of stream runs E-W
   else
   { // center line of stream runs NE-SW or SE-NW, including N-S (eff_dir_deg = 0) but not including E-W (eff_dir_deg = 90)
      double shadow_m = hw_m / sin_theta;
      double tan_elev = veg_ht_m / shadow_m;
      veg_elev_deg = atan(tan_elev) * 180. / PI;
   }

   return(veg_elev_deg);
} // end of VegElevEW_deg()


double TopoSetting::VegElevNS_deg(double direction_deg, double reach_width_m, double veg_ht_m)
// Returns the angle between horizontal and the top of the vegetation, looking to the south,
// as a function of the direction of the stream and the height of the streamside vegetation.
// Here we assume the vegetation has the same height on both sides of the stream.
// Picture two triangles, one horizontal and one vertical.
// The horizontal triangle is formed by points S, B, and C, where 
// S  is a point in the center of the stream 
// B is the point on the bank of the stream looking directly to the south
// C is the point in the center of the stream closest to B
// The horizontal angle B-S-C (aka theta) is the angle formed between the direction of the stream and the south
// The vertical triangle is formed by points S, B, and T, where
// T is the point at the top of the vegetation directly above B.
// The vertical angle T-S-B is what we want to calculate and return.
// The distance BC is half the width of the stream.
// The distance BT is the height of the vegetation.
// tan(T-S-B) is BT/BS
// sin(B-S-C) * BS = BC
// We know the angle B-S-C and the distance BC, so we use them to calculate BS.
// We know BT and we use it together with the calculated BS to calculate tan(T-S-B).
{
// ???   ASSERT(reach_width_m > 0. && veg_ht_m >= 0.);
   if (reach_width_m <= 0. || veg_ht_m <= 0.) return(0);

   double veg_elev_deg = 0.;
   double eff_dir_deg = direction_deg;
   if (eff_dir_deg >= 270.) eff_dir_deg -= 180.;
   else if (eff_dir_deg < 90.) eff_dir_deg += 180.;

   double hw_m = reach_width_m / 2.; // This is distance BC.

   double theta_deg = abs(180. - eff_dir_deg); // This is angle B-S-C.
   double sin_theta = sin(theta_deg * PI / 180.);
   ASSERT(sin_theta > 0.); if (sin_theta <= 0.) sin_theta = 0.;
   if (sin_theta == 0.) veg_elev_deg = 0.; // center line of stream runs N-S
   else
   { // center line of stream runs NE-SW or SE-NW, including E-W (eff_dir_deg = 90) but not including N-S (sin(theta) = 0, eff_dir_deg = 180)
      double shadow_m = hw_m / sin_theta; // This is distance BS.
      double tan_elev = veg_ht_m / shadow_m; // This is tan(B-S-T).
      veg_elev_deg = atan(tan_elev) * 180. / PI;
      ASSERT(veg_elev_deg >= 0. && veg_elev_deg <= 90.);
   }
   return(veg_elev_deg);
} // end of VegElevNS_deg()


double TopoSetting::VegShade_pct(double elev_deg, double azimuth_deg, double flowDirection_deg, double reachWidth_m, double vegHt_m)
// Returns the percentage of the stream width shaded from direct beam sunshine by the vegetation;
// does not account for canopy density < 1.
// Picture two triangles, one horizontal and one vertical.
// T is the top of streamside vegetation on the bank of the stream nearest the sun.
// R is the point on the bank directly below T (R for Root).
// S is the point on the stream or on the ground on the other side shaded by T.
// Note that the direction from R to S is opposite the azimuth.
// The vertical triangle is T-R-S.
// B is the point on the same bank as R directly opposite S.
// The horizontal triangle is B-R-S.
// The quantity to be calculated is related to the ratio of the distance BS to the width of the stream.
// When BS > the stream width, the shadow goes clear across the stream, and Shade_pct is 100%.
// When BS is <= the stream width, then Shade_pct is 100 * BS/stream width.
// There is a special case when azimuth_deg = direction_deg.  In that case the shadows fall
// parallel to the stream and Shade_pct is 0.
// Note that RB is parallel to the stream axis.  When the stream is flowing
// away from the sun, then the direction from R to B is the same as flowDirection_deg.
// When the stream is flowing toward the sun, the direction from R to B is 180 degrees
// off from flowDirection_deg.
{
   double shade_pct = 0.;

   double shade_direction_deg = azimuth_deg - 180.; 
   if (shade_direction_deg < 0.) shade_direction_deg += 360.;

   double RB_deg = flowDirection_deg; // Correct when the stream is flowing away from the sun.
   if (abs(flowDirection_deg - azimuth_deg) < 90.)
   { // The stream is flowing towards the sun.
      RB_deg = flowDirection_deg - 180.;
      if (RB_deg < 0.) RB_deg += 360.;
   }

   if (shade_direction_deg == RB_deg) shade_pct = 0.;
   else
   {
      double BRS_deg = abs(RB_deg - shade_direction_deg);
      if (BRS_deg > 180) BRS_deg = 360. - BRS_deg;
      ASSERT(BRS_deg <= 90.);

      double BRS_rad = 2 * PI * BRS_deg / 360.;
      double RST_rad = 2 * PI * elev_deg / 360.;

      double RT_m = vegHt_m;
      double ST_m = RT_m / sin(RST_rad);
      double RS_m = ST_m * cos(RST_rad);

      double sin_BRS = sin(BRS_rad);
      double BS_m = RS_m * sin_BRS;
      shade_pct = (BS_m > reachWidth_m) ? 100. : 100. * BS_m / reachWidth_m;
      ASSERT(!isnan(shade_pct));
   }

   return(shade_pct);
} // end of VegShade_pct()


double TopoSetting::ShadeFrac(int jday0, double* pRadSWestimate_W_m2, double direction_deg, double reachWidth_m, double vegHt_m)
// Integrate over time from solar noon back to sunrise and then from noon to sunset.
// Calculate direct and diffuse shortwave with and without topographic and vegetative shading.
// Uses equations from sec. 2.2.4 Solar Radiation Heat Above Topographic Features, pp. 38-41 in Boyd and Kasper.
// Uses variable names from Boyd & Kasper.
{
   int JD = jday0 + 1; // Julian Day as in Boyd in Kasper (aka jday1 in CW3M)
   double SW_est_J_m2 = 0.;
   double direct_unshaded_kJ_m2 = 0.;
   double diffuse_unshaded_kJ_m2 = 0.;

   double delta_t_min = 30; // timestep in minutes
   double delta_t_hr = delta_t_min / 60.;
   double delta_t_sec = delta_t_min * 60.;

   double phi_SRG_W_m2 = 1367.; // global solar flux
   double phi_SRB_W_m2 = 0.; // total unshaded shortwave, including direct and diffuse
   double phi_SRB1_W_m2 = 0.; // direct unshaded shortwave
   double phi_SRD1_W_m2 = 0.; // diffuse unshaded shortwave
   double azimuth_deg = 0.; // 0 is north.

   double T_A = // atmospheric transmissivity as a function of the day of the year
      0.0685 * cos((2. * PI / 365.) * (JD + 10.)) + 0.8; // eq. 2-30

   double direct_lost_to_shade_kJ_m2 = 0.;
   double diffuse_lost_to_shade_kJ_m2 = 0.;

   // First add up the energy from just before solar noon back to sunrise.
   double time_hr = 12. - delta_t_hr;
   double solar_elev_deg = SolarElev_deg(jday0, time_hr);
   bool topo_shaded_already = false;
   bool veg_shaded_already = false;
   double veg_shade_pct = 0;
   while (solar_elev_deg > 0)
   {
      double M_A = OpticalAirMassThickness(solar_elev_deg);
      phi_SRB_W_m2 = phi_SRG_W_m2 * pow(T_A, M_A); // * (1 - 0.65 * C_L^2) cloudiness can be ignored here
      SW_est_J_m2 += phi_SRB_W_m2 * delta_t_sec;
      double C_I = phi_SRB_W_m2 / phi_SRG_W_m2; // clearness index, eq. 2-34
      double D_F = DiffuseFrac(C_I, jday0);
      phi_SRB1_W_m2 = phi_SRB_W_m2 * (1. - D_F);
      phi_SRD1_W_m2 = phi_SRB_W_m2 * D_F;
      double direct_kJ_m2 = phi_SRB1_W_m2 * delta_t_sec / 1000.;
      direct_unshaded_kJ_m2 += direct_kJ_m2;
      double diffuse_kJ_m2 = phi_SRD1_W_m2 * delta_t_sec / 1000.;
      diffuse_unshaded_kJ_m2 += diffuse_kJ_m2;

      double azimuth_deg = SolarAzimuth_deg(jday0, time_hr);
      if (topo_shaded_already || IsTopoShaded(solar_elev_deg, azimuth_deg))
      {
         direct_lost_to_shade_kJ_m2 += direct_kJ_m2;
         topo_shaded_already = true;
      }
      else
      {
         double prev_veg_shade_pct = veg_shade_pct;
         veg_shade_pct = VegShade_pct(solar_elev_deg, azimuth_deg, direction_deg, reachWidth_m, vegHt_m);
         direct_lost_to_shade_kJ_m2 += (veg_shade_pct / 100.) * (m_vegDensity * direct_kJ_m2);
      }

      time_hr -= delta_t_hr;
      solar_elev_deg = SolarElev_deg(jday0, time_hr);
   } // end of loop from noon back to dawn

   // Now add up the energy at noon and forward until sunset.
   time_hr = 12.;
   solar_elev_deg = SolarElev_deg(jday0, time_hr);
   topo_shaded_already = false;
   veg_shaded_already = false;
   while (solar_elev_deg > 0)
   {
      double M_A = OpticalAirMassThickness(solar_elev_deg);
      phi_SRB_W_m2 = phi_SRG_W_m2 * pow(T_A, M_A); // * (1 - 0.65 * C_L^2) cloudiness can be ignored here
      SW_est_J_m2 += phi_SRB_W_m2 * delta_t_sec;
      double C_I = phi_SRB_W_m2 / phi_SRG_W_m2; // clearness index, eq. 2-34
      double D_F = DiffuseFrac(C_I, jday0);
      phi_SRB1_W_m2 = phi_SRB_W_m2 * (1. - D_F);
      phi_SRD1_W_m2 = phi_SRB_W_m2 * D_F;
      double direct_kJ_m2 = phi_SRB1_W_m2 * delta_t_sec / 1000.;
      direct_unshaded_kJ_m2 += direct_kJ_m2;
      double diffuse_kJ_m2 = phi_SRD1_W_m2 * delta_t_sec / 1000.;
      diffuse_unshaded_kJ_m2 += diffuse_kJ_m2;

      double azimuth_deg = SolarAzimuth_deg(jday0, time_hr);
      if (topo_shaded_already || IsTopoShaded(solar_elev_deg, azimuth_deg))
      {
         direct_lost_to_shade_kJ_m2 += direct_kJ_m2;
         topo_shaded_already = true;
      }
      else if (veg_shaded_already || IsVegShaded(solar_elev_deg, azimuth_deg))
      {
         direct_lost_to_shade_kJ_m2 += m_vegDensity * direct_kJ_m2;
         veg_shaded_already = true;
      }

      time_hr += delta_t_hr;
      solar_elev_deg = SolarElev_deg(jday0, time_hr);
   } // end of loop from noon forward to sunset

   double net_direct_kJ_m2 = direct_unshaded_kJ_m2 - direct_lost_to_shade_kJ_m2;
   double net_diffuse_kJ_m2 = diffuse_unshaded_kJ_m2;

   double shade_frac = 1. - (net_direct_kJ_m2 + net_diffuse_kJ_m2) / (direct_unshaded_kJ_m2 + diffuse_unshaded_kJ_m2);
   *pRadSWestimate_W_m2 = SW_est_J_m2 / SEC_PER_DAY;

   ASSERT(0. < *pRadSWestimate_W_m2 && *pRadSWestimate_W_m2 <= phi_SRG_W_m2);
   ASSERT(0. <= shade_frac && shade_frac <= 1.);

   return(shade_frac);
} // end of ShadeFrac()


double TopoSetting::SolarDeclination_deg(int jday0)
{
   // from Wikipedia article "Position of the Sun" on 2/3/21
   double N_day = jday0; // Jan 1 = 0
   // First approximation
   // declination_deg = -23.44deg * cos((360deg/365days) * (N_days + 10days))
   double arg_for_cos_deg = (360. / 365.) * (N_day + 10.);
   double arg1_for_cos_radian = arg_for_cos_deg * PI / 180.;
   double declination1_deg = -23.44 * cos(arg1_for_cos_radian);

   // Second approximation
   // declination_deg = asin(sin(-23.44deg) * 
   //    cos((360deg/365.24days)*(N_day + 10) + (360deg/PI) *  0.0167 * sin((360deg/365.24day)*(N_day - 2))))
   const double W = 360. / 365.24; // mean angular orbital velocity in degrees per day 
   double axial_tilt_deg = 23.44;
   double axial_tilt_radian = axial_tilt_deg * PI / 180.;
   double time_since_perihelion_day = N_day - 2.;
   double arg_for_sin_deg = W * time_since_perihelion_day;
   double arg_for_sin_radian = arg_for_sin_deg * PI / 180.;
   double arg2_for_cos_deg = W * (N_day + 10.) + W * 0.0167 * sin(arg_for_sin_radian);
   double arg2_for_cos_radian = arg2_for_cos_deg * PI / 180.;
   double arg_for_asin = sin(-axial_tilt_radian) * cos(arg2_for_cos_radian);
   double declination2_radian = asin(arg_for_asin);
   double declination2_deg = (180. / PI) * declination2_radian;

   return(declination2_deg);
} // end of SolarDeclination_deg()


double TopoSetting::SolarElev_deg(double lat_deg, int jday0, double time_hr)
// from itacanet.org/the-sun-as-a-source-of-energy/part-3-calculating-solar-angles
// alpha = altitude angle
// delta = declination
// omega = hour angle (noon = 0)
// phi = latitude
// alpha = sin(delta) * sin(phi) + cos(delta) * cos(omega) * cos (phi)
{
   double declination_deg = SolarDeclination_deg(jday0);
   double delta = declination_deg * PI / 180.;

   double phi = lat_deg * PI / 180.;

   double hour_angle_deg = ((time_hr - 12.) / 24.) * 360.;
   double omega = hour_angle_deg * PI / 180.;

   double alpha = sin(delta) * sin(phi) + cos(delta) * cos(omega) * cos(phi);

   double solar_elev_in_northern_hemisphere_deg = alpha * 180. / PI;
   return(solar_elev_in_northern_hemisphere_deg);
} // end of SolarElev_deg()


double TopoSetting::SolarElev_deg(int jday0, double time_hr)
// from itacanet.org/the-sun-as-a-source-of-energy/part-3-calculating-solar-angles
// alpha = altitude angle
// delta = declination
// omega = hour angle (noon = 0)
// phi = latitude
// alpha = sin(delta) * sin(phi) + cos(delta) * cos(omega) * cos (phi)
{
   double solar_elev_in_northern_hemisphere_deg = SolarElev_deg(m_lat_deg, jday0, time_hr);
   return(solar_elev_in_northern_hemisphere_deg);
} // end of SolarElev_deg()


double TopoSetting::SolarAzimuth_deg(int jday, double time_hr) // 0 deg is north
// From Wikipedia "Solar azimuth angle" 2/1/21 and "Solar zenith angle" 2/1/21
{
   double delta_deg = SolarDeclination_deg(jday); // ???
   double delta_radian = delta_deg * PI / 180.;
   double cos_delta = cos(delta_radian);

   double h_radian = (time_hr * (PI / 12.)); // hour angle

   double phi_radian = m_lat_deg * (PI / 180);

   double cos_theta_s = sin(phi_radian) * sin(delta_radian) + cos(phi_radian) * cos_delta * cos(h_radian);
   double theta_s_radian = acos(cos_theta_s); // solar zenith angle


   double sin_phi_s = -sin(h_radian) * cos(delta_radian) / sin(theta_s_radian);
   double azimuth_radians_from_south = asin(sin_phi_s);
   double azimuth_deg_from_north = (azimuth_radians_from_south * (180. / PI)) + 180.;

   return(azimuth_deg_from_north);
} // end of SolarAzimuth_deg()


bool TopoSetting::IsTopoShaded(double solarElev_deg, double solarAzimuth_deg)
{
   // Interpolate the topographic elevation in the direction of the sun.
   ASSERT(0. <= solarAzimuth_deg && solarAzimuth_deg < 360.);
   double topo_elev_N_deg = (m_topoElevW_deg + m_topoElevE_deg) / 2.;
   double topo_elev_deg;
   if (solarAzimuth_deg < 90.) topo_elev_deg = m_topoElevE_deg;
   else if (solarAzimuth_deg < 180.) topo_elev_deg = m_topoElevE_deg + ((solarAzimuth_deg - 90.) / 90.) * (m_topoElevS_deg - m_topoElevE_deg);
   else if (solarAzimuth_deg < 270.) topo_elev_deg = m_topoElevS_deg + ((solarAzimuth_deg - 180.) / 90.) * (m_topoElevW_deg - m_topoElevS_deg);
   else topo_elev_deg = m_topoElevW_deg;

   bool return_val = topo_elev_deg >= solarElev_deg;
   return(return_val);
} // end of IsTopoShaded()


bool TopoSetting::IsVegShaded(double solarElev_deg, double solarAzimuth_deg)
{
   // Interpolate the veg elevation in the direction of the sun.
   ASSERT(0. <= solarAzimuth_deg && solarAzimuth_deg < 360.);
   double veg_elev_deg;
   if (solarAzimuth_deg < 90.) veg_elev_deg = m_vegElevNS_deg + (solarAzimuth_deg / 90.) * (m_vegElevEW_deg - m_vegElevNS_deg);
   else if (solarAzimuth_deg < 180.) veg_elev_deg = m_vegElevEW_deg + ((solarAzimuth_deg - 90.) / 90.) * (m_vegElevNS_deg - m_vegElevEW_deg);
   else if (solarAzimuth_deg < 270.) veg_elev_deg = m_vegElevNS_deg + ((solarAzimuth_deg - 180.) / 90.) * (m_vegElevEW_deg - m_vegElevNS_deg);
   else veg_elev_deg = m_vegElevEW_deg + ((solarAzimuth_deg - 270.) / 90.) * (m_vegElevNS_deg - m_vegElevEW_deg);

   bool return_val = veg_elev_deg >= solarElev_deg;
   return(return_val);
} // end of IsTopoShaded()


TopoSetting::TopoSetting(double elev_m, double topoElevE_deg, double topoElevS_deg, double topoElevW_deg, double lat_deg = 44.21228, double long_deg = -122.25528)
// 44.21228 N, 122.25528 W is the location of the H.J. Andrews Experimental Forest (44deg 12' 44.2"N, 122deg 15' 19" W)
{
   m_elev_m = elev_m;
   m_topoElevE_deg = topoElevE_deg;
   m_topoElevS_deg = topoElevS_deg;
   m_topoElevW_deg = topoElevW_deg;
   double avg_topo_elev_deg = (topoElevE_deg + topoElevS_deg + topoElevW_deg) / 3.;
   m_topoViewToSky = 1. - avg_topo_elev_deg / 90.;
   m_lat_deg = lat_deg;
   m_long_deg = long_deg;
   m_vegDensity = 0.;
   m_vegElevEW_deg = m_vegElevNS_deg = 0.;
} // end of TopoSetting constructor


double FlowModel::GetReachShade_a_lator_W_m2(Reach* pReach, double SW_unshaded_W_m2)
{
   double direction_deg = pReach->Att(ReachDIRECTION);
   double width_m = pReach->Att(ReachWIDTHREACH);
   ASSERT(width_m > 0.);
   double veg_ht_m = pReach->Att(ReachVEGHTREACH);
   double rad_sw_est_W_m2 = 0.;
   ASSERT(pReach->m_topo.m_vegDensity >= 0.);
   double shade_frac = pReach->m_topo.ShadeFrac(m_flowContext.dayOfYear, &rad_sw_est_W_m2, direction_deg, width_m, veg_ht_m);
   m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachRAD_SW_EST, rad_sw_est_W_m2);
   double shade_coeff = 1. - shade_frac;
   m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachSHADECOEFF, shade_coeff);

   double shaded_SW_W_m2 = shade_coeff * SW_unshaded_W_m2;
   return(shaded_SW_W_m2);
} // end of GetReachShade_a_lator_W_m2()


double Reach::GetSubreachViewToSky_frac(int subreachNdx)
{
   double vts_frac = 0.9;

   // ??? Ultimately, vts_frac should be adjusted, to take into account shading by bankside vegetation and the width of the reach.

   return(vts_frac);
} // end of GetSubreachViewToSky_frac()


double Reach::GetSubreachArea_m2(int subreachNdx)
{
   ReachSubnode* pNode = GetReachSubnode(subreachNdx);
   return(pNode->m_subreach_surf_area_m2);
} // end of GetSegmentArea_m2()


WaterParcel Reach::GetSubreachDischargeWP(int subnode)
{
   if (subnode < 0) subnode = this->GetSubnodeCount() - 1;

   ReachSubnode* pNode = (ReachSubnode*)this->m_subnodeArray[subnode];
   ASSERT(pNode != NULL);
   ASSERT(!isnan(pNode->m_dischargeWP.m_volume_m3));
   return(pNode->m_dischargeWP);
} // end of GetDischargeWP()


WaterParcel Reach::GetReachDischargeWP() // Calculates and returns Q_DISCHARG
{
   int subnode = GetSubnodeCount() - 1;
   ReachSubnode* pNode = (ReachSubnode*)m_subnodeArray[subnode];
   WaterParcel downstream_outflowWP = pNode->m_dischargeWP;

   downstream_outflowWP.Discharge(m_q2wetl_cms * SEC_PER_DAY);

   return(downstream_outflowWP);
} // end of GetReachDischargeWP()


double Reach::GetUpstreamInflow( )
   {
   double Q = 0.0f;
   ReachSubnode *pNodeLeft = NULL;
   if (this->m_pLeft && this->m_pLeft->m_polyIndex >= 0)
      {
      int lastSubnode = this->m_pLeft->GetSubnodeCount()-1;
      pNodeLeft = (ReachSubnode*) this->m_pLeft->m_subnodeArray[ lastSubnode ];
      }

   ReachSubnode *pNodeRight = NULL;
   if (this->m_pRight && this->m_pRight->m_polyIndex >= 0)
      {
      int lastSubnode = this->m_pRight->GetSubnodeCount()-1;
      pNodeRight = (ReachSubnode*) this->m_pRight->m_subnodeArray[ lastSubnode ];
      }

   if (pNodeLeft != NULL)
      Q = pNodeLeft->m_discharge;
   if (pNodeRight != NULL)
      Q += pNodeRight->m_discharge;

   return Q;
   }

bool Reach::GetUpstreamInflow(double &QLeft, double &QRight)
{
   ReachSubnode *pNodeLeft = NULL;
   if (this->m_pLeft && this->m_pLeft->m_polyIndex >= 0)
   {
      int lastSubnode = this->m_pLeft->GetSubnodeCount() - 1;
      pNodeLeft = (ReachSubnode*) this->m_pLeft->m_subnodeArray[lastSubnode];
   }

   ReachSubnode *pNodeRight = NULL;
   if (this->m_pRight && this->m_pRight->m_polyIndex >= 0)
   {
      int lastSubnode = this->m_pRight->GetSubnodeCount() - 1;
      pNodeRight = (ReachSubnode*) this->m_pRight->m_subnodeArray[lastSubnode];
   }

   if (pNodeLeft != NULL)
      QLeft = pNodeLeft->m_discharge;
   if (pNodeRight != NULL)
      QRight = pNodeRight->m_discharge;

   return true;
}


bool Reach::AccumWPadditions(WaterParcel incomingWP)
{
   if (isnan(incomingWP.m_volume_m3) || isnan(m_additionsWP.m_volume_m3))
   {
      ASSERT(false);
      m_nanOccurred = true;
      return(false);
   }

   WaterParcel orig_stored_valueWP = m_additionsWP;
   m_additionsWP.MixIn(incomingWP); 
   if (isnan(m_additionsWP.m_volume_m3))
   {
      ASSERT(false);
      m_nanOccurred = true;
      m_additionsWP = orig_stored_valueWP;
      return(false);
   }

   return(true);
} // end of AccumWPadditions();


bool Reach::AddRunoffToReach(double runoffVol_m3)
{
   ASSERT(runoffVol_m3 >= 0.); // This will also catch when runoffVol_m3 is a NaN.
   ASSERT(!isnan(m_globalHandlerFluxValue));

   // For reaches, negative flux values are additions to the reach, 
   // unlike HRUs, where negative flux values are withdrawals from the water compartment. Really! DRC 4/5/22
   m_globalHandlerFluxValue = (float)(m_globalHandlerFluxValue - runoffVol_m3);
   return(true);

} // end of AddRunoffToReach()


bool Reach::AccumWithdrawals(double withdrawal_volume_m3)
{
   ASSERT(withdrawal_volume_m3 >= 0.); // This should also catch NaNs in withdrawal_volume_m3.
   ASSERT(!isnan(m_globalHandlerFluxValue));

   m_globalHandlerFluxValue = (float)(m_globalHandlerFluxValue + withdrawal_volume_m3);
   return(true);
} // end of AccumWithdrawals()


bool Reach::AddFluxFromGlobalHandler(float value) // value is in m3
// negative values of m_globalHandlerFluxValue are sinks (water entering the reach), positive values are sources (water leaving the reach) (m3/day)  
{
   // ASSERT(value >= 0.); // Eventually, for water entering reaches, use AccumWPadditions(WP) instead of AddFluxFromGlobalHandler(volume_m3).
   // For the moment, water entering reaches from the soil uses AddFluxFromGlobalHandler() and is assigned a temperature in 
   // ReachRouting::SolveReachKinematicWave(). DRC 5/25/21

   if (isnan(value) || isnan(m_globalHandlerFluxValue))
   {
      m_nanOccurred = true;
      return(false);
   }

   float orig_stored_value = m_globalHandlerFluxValue;
   m_globalHandlerFluxValue += value;
   if (isnan(m_globalHandlerFluxValue))
   {
      m_nanOccurred = true;
      m_globalHandlerFluxValue = orig_stored_value;
      return(false);
   }

   return(true);
} // end of AddFluxFromGlobalHandler(float) 


///////////////////////////////////////////////////////////////////////////////
// R E S E R V O I R  
///////////////////////////////////////////////////////////////////////////////

Reservoir::Reservoir(void)
   : FluxContainer()
   , StateVarContainer()
   , m_id( -1 )
   , m_col( -1 )
   , m_dam_top_elev( 0 )
   , m_fc1_elev( 0 )
   , m_fc2_elev( 0 )
   , m_fc3_elev( 0 )
   , m_tailwater_elevation( 0 )
   , m_buffer_elevation( 0 )
   , m_turbine_efficiency( 0 )
   , m_minOutflow( 0 )
   , m_maxVolume( 1300 )
   , m_gateMaxPowerFlow( 0 )
   , m_maxPowerFlow( 0 )
   , m_minPowerFlow( 0 )
   , m_powerFlow( 0 )
   , m_gateMaxRO_Flow( 0 )
   , m_maxRO_Flow( 0 )
   , m_minRO_Flow( 0 )
   , m_RO_flow( 0 )
   , m_gateMaxSpillwayFlow( 0 )   
   , m_maxSpillwayFlow( 0 )   
   , m_minSpillwayFlow( 0 )
   , m_spillwayFlow( 0 )
   , m_reservoirType( ResType_FloodControl )
   , m_releaseFreq(1)
   , m_inflow( 0 )
   , m_outflow( 0 )
   , m_outflowWP(0,0)
   , m_elevation( 0 )
   , m_power( 0 )
   , m_filled( 0 )
   , m_volume( 50 )   
   , m_resWP(50, DEFAULT_REACH_H2O_TEMP_DEGC)
   , m_pReach( NULL )
   , m_pFlowfromUpstreamReservoir( NULL )
   , m_pAreaVolCurveTable( NULL )
   , m_pRuleCurveTable( NULL )
   , m_pBufferZoneTable( NULL )
   , m_pCompositeReleaseCapacityTable( NULL )
   , m_pRO_releaseCapacityTable( NULL )
   , m_pSpillwayReleaseCapacityTable( NULL )
   , m_pRulePriorityTable ( NULL )
   , m_pResData( NULL )
   //, m_pResMetData( NULL )
   , m_pResSimFlowCompare( NULL )
   , m_pResSimFlowOutput( NULL )
   , m_pResSimRuleCompare( NULL )
   , m_pResSimRuleOutput( NULL )
   , m_activeRule("Uninitialized")
   , m_zone(-1)
   , m_daysInZoneBuffer(0)
   , m_probMaintenance(-1)
   { }


Reservoir::~Reservoir(void)
   {
   if ( m_pAreaVolCurveTable != NULL )
      delete m_pAreaVolCurveTable;

   if ( m_pRuleCurveTable != NULL )
      delete m_pRuleCurveTable;

   if ( m_pCompositeReleaseCapacityTable != NULL )
      delete m_pCompositeReleaseCapacityTable;

   if ( m_pRO_releaseCapacityTable != NULL )
      delete m_pRO_releaseCapacityTable;
   
   if ( m_pSpillwayReleaseCapacityTable != NULL )
      delete m_pSpillwayReleaseCapacityTable;
   
   if ( m_pRulePriorityTable != NULL )
      delete m_pRulePriorityTable;

   if ( m_pResData != NULL )
      delete m_pResData;

   //if (m_pResMetData != NULL)
   //   delete m_pResMetData;

   if ( m_pResSimFlowCompare != NULL )
      delete m_pResSimFlowCompare;
   
   if ( m_pResSimFlowOutput != NULL )
      delete m_pResSimFlowCompare;

   if ( m_pResSimRuleCompare != NULL )
      delete m_pResSimRuleCompare;

   if ( m_pResSimRuleOutput != NULL )
      delete m_pResSimRuleOutput;

   }


ControlPoint::~ControlPoint(void)
{
   if (  m_pControlPointConstraintsTable != NULL )
      delete m_pControlPointConstraintsTable;

   if (  m_pResAllocation != NULL )
      delete  m_pResAllocation;
}


void Reservoir::InitDataCollection( void )
   {
   if ( m_pResData == NULL )
      {
      if (m_reservoirType == ResType_CtrlPointControl)
         m_pResData = new FDataObj(4, 0);
      else
         m_pResData = new FDataObj(14, 0);

      ASSERT( m_pResData != NULL );
      }

   CString name( this->m_name );
   name += " Reservoir";
   m_pResData->SetName( name );

   if (m_reservoirType == ResType_CtrlPointControl)
      { 
      m_pResData->SetLabel(0, "Time");
      m_pResData->SetLabel(1, "Pool Elevation (m)");
      m_pResData->SetLabel(2, "Inflow (m3/s)");
      m_pResData->SetLabel(3, "Outflow (m3/s)");
      }
   else
      {
      m_pResData->SetLabel(0, "Time");
      m_pResData->SetLabel(1, "Pool Elevation (m)");
      m_pResData->SetLabel(2, "Rule Curve Elevation (m)");
      m_pResData->SetLabel(3, "Inflow (m3/s)");
      m_pResData->SetLabel(4, "Outflow (m3/s)");
      m_pResData->SetLabel(5, "Powerhouse Flow (m3/s)");
      m_pResData->SetLabel(6, "Regulating Outlet Flow (m3/s)");
      m_pResData->SetLabel(7, "Spillway Flow (m3/s)");
	   m_pResData->SetLabel(8, "Zone");
	   m_pResData->SetLabel(9, "Days in Buffer Zone");
	   m_pResData->SetLabel(10, "Buffer Zone Elevation (m)");
	   m_pResData->SetLabel(11, "Water Year Type");
	   m_pResData->SetLabel(12, "Constraint Value");
      m_pResData->SetLabel(13, "Volume (m3)");
      }

   gpFlow->AddOutputVar( name, m_pResData, "" );

   //if (m_pResMetData == NULL)
   //   {
   //   m_pResMetData = new FDataObj(7, 0);
   //   ASSERT(m_pResMetData != NULL);
   //   }
   //
   //CString name2(name);
   //name2 += " Met";
   //m_pResMetData->SetName(name2);
   //
   //m_pResMetData->SetLabel(0, "Time");
   //m_pResMetData->SetLabel(1, "Avg Daily Temp (C)");
   //m_pResMetData->SetLabel(2, "Precip (mm/d)");
   //m_pResMetData->SetLabel(3, "Shortwave Incoming (watts/m2)");
   //m_pResMetData->SetLabel(4, "Specific Humidity ");
   //m_pResMetData->SetLabel(5, "Wind Speed (m/s)");
   //m_pResMetData->SetLabel(6, "Wind Direction");
   //gpFlow->AddOutputVar(name2, m_pResMetData, "");

   if ( m_pResSimFlowCompare == NULL )
      {
      m_pResSimFlowCompare = new FDataObj( 7, 0 );
      ASSERT( m_pResSimFlowCompare != NULL );
      }
   
   m_pResSimFlowCompare->SetLabel( 0, "Time" );
   m_pResSimFlowCompare->SetLabel( 1, "Pool Elevation (m) - ResSIM" );
   m_pResSimFlowCompare->SetLabel( 2, "Pool Elevation (m) - Envision" );
   m_pResSimFlowCompare->SetLabel( 3, "Inflow (m3/s) - ResSIM" );
   m_pResSimFlowCompare->SetLabel( 4, "Inflow (m3/s) - Envision" );
   m_pResSimFlowCompare->SetLabel( 5, "Outflow (m3/s) - ResSIM" );
   m_pResSimFlowCompare->SetLabel( 6, "Outflow (m3/s) - Envision" );
   
   CString name3( this->m_name );
   name3 += " Reservoir - ReSIM Flow Comparison";
   m_pResSimFlowCompare->SetName( name3 );
   
   if ( m_pResSimRuleCompare == NULL )
      {
      m_pResSimRuleCompare = new VDataObj( 7, 0 );
      ASSERT( m_pResSimRuleCompare != NULL );
      }
   
   m_pResSimRuleCompare->SetLabel( 0, "Time" );
   m_pResSimRuleCompare->SetLabel( 1, "Blank");
   m_pResSimRuleCompare->SetLabel( 2, "Inflow (cms)");
   m_pResSimRuleCompare->SetLabel( 3, "Outflow (cms)");
   m_pResSimRuleCompare->SetLabel( 4, "ews (m)");
   m_pResSimRuleCompare->SetLabel( 5, "Zone");
   m_pResSimRuleCompare->SetLabel( 6, "Active Rule - Envision" );

   CString name4( this->m_name );
   name4 += " Reservoir - ReSIM Rule Comparison";
   m_pResSimRuleCompare->SetName( name4 );
   }


// this function iterates through the columns of the rule priority table,
// loading up constraints
bool Reservoir::ProcessRulePriorityTable( void )
   {
   if ( m_pRulePriorityTable == NULL )
      return false;

   // in this table, each column is a zone.  Each row is a csv file
   int cols = m_pRulePriorityTable->GetColCount();
   int rows = m_pRulePriorityTable->GetRowCount();

   this->m_zoneInfoArray.RemoveAll();
   this->m_resConstraintArray.RemoveAll();

   // iterate through the zones defined in the file
   for ( int i=0; i < cols; i++ )
      {
      ZoneInfo *pZone = new ZoneInfo;
      ASSERT( pZone != NULL );

      pZone->m_zone = (ZONE) i;  // ???????

      this->m_zoneInfoArray.Add( pZone );

      // iterate through the csv files for this zone
      for ( int j=0; j < rows; j++ )
         {
         CString filename = m_pRulePriorityTable->GetAsString( i, j );
              
         CString cp_path(gpModel->m_path + m_dir + m_cpdir + filename);
         gpModel->ApplyMacros(cp_path);
         CString rp_path(gpModel->m_path + m_dir + m_rpdir + filename);
         gpModel->ApplyMacros(rp_path);
         ResConstraint *pConstraint = FindResConstraint(rp_path );
       
         if ( pConstraint == NULL && filename.CompareNoCase( "Missing" ) != 0  ) // not yet loaded?  and not 'Missing'?    Still not loaded?
            {
            pConstraint = new ResConstraint;
            ASSERT( pConstraint );

            pConstraint->m_constraintFileName = filename;
      
            // Parse heading to determine whether to look in control point folder or rules folder
            TCHAR heading[ 256 ];
            lstrcpy( heading, filename );
            
            TCHAR *control = NULL;
            TCHAR *next    = NULL;
            TCHAR delim[] = " ,_";  //allowable delimeters, space and underscore
            
            control = _tcstok_s( heading, delim, &next );  // Returns string at head of file. 
            int records = -1;

            if ( lstrcmpi( control, "cp" ) == 0 )
               records = gpModel->LoadTable(cp_path, (DataObj**) &(pConstraint->m_pRCData), 0 );
            else
               records = gpModel->LoadTable( rp_path, (DataObj**) &(pConstraint->m_pRCData), 0 ); 
            
            if ( records <= 0 )  // No records? Display error msg
               {
               CString msg;
               msg.Format( "Flow: Unable to load reservoir constraints for Zone %i table %s in Reservoir rule priority file %s.  This constraint will be ignored...",
                    i, (LPCTSTR) filename, (LPCTSTR) m_pRulePriorityTable->GetName() );

               Report::ErrorMsg( msg );
               delete pConstraint;
               pConstraint = NULL;
               } // end of if (records <=0)
            else
               this->m_resConstraintArray.Add( pConstraint );
            }  // end of ( if ( pConstraint == NULL )
          
         // Get rule type (m_type).  Assume 1st characters of filename until first underscore or space contain rule type.  
         // Parse rule name and determine m_type  (maximum, minimum, Maximum rate of decrease, Maximum Rate of Increase)
         if ( pConstraint != NULL )
            {
            TCHAR  name[ 256 ];
            lstrcpy( name, filename );

            TCHAR *ruletype = NULL;
            TCHAR *next     = NULL;
            TCHAR  delim[] = " ,_";  //allowable delimeters, space and underscore
            
            // Ruletypes
            ruletype = strtok_s(name, delim, &next);  //Returns string at head of file. 
            
            //Compare string at the head of the file to allowable ruletypes
            if ( lstrcmpi( ruletype, "max" ) == 0 )
               pConstraint->m_type = RCT_MAX;
            else if ( lstrcmpi( ruletype, "min") == 0 )
               pConstraint->m_type = RCT_MIN;
            else if ( lstrcmpi( ruletype,"maxi") == 0 )
               pConstraint->m_type = RCT_INCREASINGRATE;
            else if ( lstrcmpi( ruletype, "maxd" ) == 0 )
               pConstraint->m_type = RCT_DECREASINGRATE;
            else if ( lstrcmpi( ruletype,"cp" ) == 0 )      //downstream control point
               pConstraint->m_type = RCT_CONTROLPOINT;
            else if ( lstrcmpi( ruletype, "pow") == 0 )     //Power plant flow
               pConstraint->m_type = RCT_POWERPLANT;
            else if ( lstrcmpi( ruletype, "ro" ) == 0 )     //RO flow
               pConstraint->m_type = RCT_REGULATINGOUTLET;
            else if ( lstrcmpi( ruletype, "spill" ) == 0)   //Spillway flow
               pConstraint->m_type = RCT_SPILLWAY;
            else 
               pConstraint->m_type = RCT_UNDEFINED;    //Unrecognized rule type     
            }  //end of if pConstraint != NULL )
   
         if ( pConstraint != NULL )    // if valid, add to the current zone
            {
            ASSERT( pZone != NULL );
            pZone->m_resConstraintArray.Add( pConstraint );
            }
         }  // end of: if ( j < rows )
      }  // end of: if ( i < cols );

   return true;
   }
   

// get pool elevation from volume - units returned are meters
float Reservoir::GetPoolElevationFromVolume()
   {
   if ( m_pAreaVolCurveTable == NULL )
      return 0;
   
   float volume = (float) m_volume;

   float poolElevation = m_pAreaVolCurveTable->IGet( volume, 1, 0, IM_LINEAR );  // this is in m
   
   return poolElevation;
   }


float Reservoir::GetPoolSurfaceAreaFromVolume_ha() // Returns reservoir water surface area in hectares..
{
   if (m_pAreaVolCurveTable == NULL) return (0);

   float volume = (float)m_volume;
   float pool_area_m2 = m_pAreaVolCurveTable->IGet(volume, 1, 2, IM_LINEAR);  
   return(pool_area_m2);
} // end of GetPoolSurfaceAreaFromVolume()


// get pool volume from elevation - units returned are meters cubed
float Reservoir::GetPoolVolumeFromElevation(float elevation)
   {
   if ( m_pAreaVolCurveTable == NULL )
      return 0;

   float poolVolume = m_pAreaVolCurveTable->IGet( elevation, 0, 1, IM_LINEAR );  // this is in m3
  
   return poolVolume;    //this is M3
   }

float Reservoir::GetBufferZoneElevation( int doy )
   {
   if ( m_pBufferZoneTable == NULL )
      return 0;

   // convert time to day of year
   //int doy = ::GetJulianDay( gpModel->m_month, gpModel->m_day, gpModel->m_year );   // 1-based
   //doy -= 1;  // make zero-based
   //int dayOfYear = int( fmod( m_timeInRun, 365 ) );  // zero based day of year

   //time = fmod( time, 365 );
   float bufferZoneElevation = m_pBufferZoneTable->IGet(float(doy), 1, IM_LINEAR);   // m
   return bufferZoneElevation;    //this is M
   }


// get target pool elevation from rule curve - units returned are meters
float Reservoir::GetTargetElevationFromRuleCurve( int doy  )
   {
   if ( m_pRuleCurveTable == NULL )
      return m_elevation;

   // convert time to day of year
   //int doy = ::GetJulianDay( gpModel->m_month, gpModel->m_day, gpModel->m_year );   // 1-based
   //doy -= 1;  // make zero-based

   //int dayOfYear = int( fmod( m_timeInRun, 365 ) );  // zero based day of year
   //time = fmod( time, 365 );
   float target = m_pRuleCurveTable->IGet( float( doy ), 1, IM_LINEAR );   // m
   return target;
   }


void Reservoir::UpdateMaxGateOutflows(Reservoir *pRes, float currentPoolElevation)
{
   /*ASSERT(pRes->m_pRO_releaseCapacityTable != NULL);
   ASSERT(pRes->m_pSpillwayReleaseCapacityTable != NULL);*/

   if (pRes->m_reservoirType == ResType_FloodControl || pRes->m_reservoirType == ResType_RiverRun)
   {
      if (pRes->m_pRO_releaseCapacityTable != NULL)
         pRes->m_maxRO_Flow = pRes->m_pRO_releaseCapacityTable->IGet(currentPoolElevation, 1, IM_LINEAR);

      pRes->m_maxSpillwayFlow = pRes->m_pSpillwayReleaseCapacityTable->IGet(currentPoolElevation, 1, IM_LINEAR);
   }
}


WaterParcel Reservoir::GetResOutflowWP(Reservoir* pRes, int doy)
{
   ASSERT(pRes != NULL);

   pRes->m_resWP.MixIn(pRes->m_inflowWP);
   float h2o_area_m2 = GetPoolSurfaceAreaFromVolume_ha() * M2_PER_HA;

   Reach* pReach = pRes->m_pReach;
   float temp_air_degC = gpModel->GetTodaysReachTEMP_AIR(pReach);
   float tmax_air_degC = gpModel->GetTodaysReachTMAX_AIR(pReach);
   float reach_precip_mm = gpModel->GetTodaysReachPRECIP(pReach);
   float reach_ws_m_s = gpModel->GetTodaysReachWINDSPEED(pReach);
   float sw_unshaded_W_m2 = gpModel->GetTodaysReachRAD_SW(pReach); // Shortwave from the climate data takes into account cloudiness but not shading.
   double rad_sw_net_W_m2 = sw_unshaded_W_m2 * pRes->m_resSWmult;

   double cloudiness_frac = ReachRouting::Cloudiness(sw_unshaded_W_m2, gpModel->m_flowContext.dayOfYear);
   float sphumidity = gpModel->GetTodaysReachSPHUMIDITY(pReach);
   double vts_frac = pReach->GetSubreachViewToSky_frac(0);
   double z_mean_m; gpModel->m_pStreamLayer->GetData(pReach->m_polyIndex, gpModel->m_colReachZ_MEAN, z_mean_m);
   double ea, vpd;
   double rh_pct = 100 * ETEquation::CalculateRelHumidity(sphumidity, temp_air_degC, tmax_air_degC, (float)z_mean_m, ea, vpd);

   double evap_m3, evap_kJ, sw_kJ, lw_kJ;
   WaterParcel adjustedWP = ReachRouting::ApplyEnergyFluxes(pRes->m_resWP, h2o_area_m2, rad_sw_net_W_m2,
         pRes->m_resWP.WaterTemperature(), temp_air_degC, vts_frac, cloudiness_frac, reach_ws_m_s, sphumidity, rh_pct,
         evap_m3, evap_kJ, sw_kJ, lw_kJ);

   gpModel->m_totEvapFromReservoirsYr_m3 += evap_m3;
   double evap_mm = (evap_m3 / h2o_area_m2) * 1000.;
   gpModel->m_pReachLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachRES_EVAPMM, evap_mm);
   double lw_W_m2 = ((lw_kJ * 1000.) / h2o_area_m2) / SEC_PER_DAY;
   gpModel->m_pReachLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachRES_LW_OUT, lw_W_m2);
   double sw_W_m2 = ((sw_kJ * 1000.) / h2o_area_m2) / SEC_PER_DAY;
   gpModel->m_pReachLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachRES_SW_IN, sw_W_m2);

   pRes->m_resWP = adjustedWP;
   pRes->m_volume = pRes->m_resWP.m_volume_m3;

   double outflow = 0.0;

   float currentPoolElevation = pRes->GetPoolElevationFromVolume();
   pRes->m_elevation = currentPoolElevation;
   UpdateMaxGateOutflows(pRes, currentPoolElevation);

   // check for river run reservoirs 
   if (pRes->m_reservoirType == ResType_RiverRun)
   {
      outflow = (pRes->m_inflow - evap_m3) / SEC_PER_DAY;    // outflow is in cms
      pRes->m_activeRule = "RunOfRiver"; pRes->m_constraintValue = 0.f;
   }
   else
   {
      float targetPoolElevation = pRes->GetTargetElevationFromRuleCurve(doy);
      float targetPoolVolume = pRes->GetPoolVolumeFromElevation(targetPoolElevation);
      float currentVolume = pRes->GetPoolVolumeFromElevation(currentPoolElevation);
      float bufferZoneElevation = pRes->GetBufferZoneElevation(doy);

      if (currentVolume > pRes->m_maxVolume)
      {
         currentVolume = pRes->m_maxVolume;   //Don't allow res volumes greater than max volume.  This code may be removed once hydro model is calibrated.
         pRes->m_volume = currentVolume;      //Store adjusted volume as current volume.
         pRes->m_resWP = WaterParcel(currentVolume, pRes->m_resWP.WaterTemperature());
         currentPoolElevation = pRes->GetPoolElevationFromVolume();  //Adjust pool elevation

         CString msgvol;
         msgvol.Format("Flow: Reservoir volume at '%s' on day of year %i exceeds maximum.  Volume set to maxVolume. Mass Balance not closed.", (LPCTSTR)pRes->m_name, doy);
         Report::StatusMsg(msgvol);
         pRes->m_activeRule = "OverMaxVolume!"; pRes->m_constraintValue = 0.f;

      }

      pRes->m_volume = currentVolume;
      pRes->m_resWP = WaterParcel(currentVolume, pRes->m_resWP.WaterTemperature());

      float desiredRelease = (currentVolume - targetPoolVolume) / SEC_PER_DAY;     //This would bring pool elevation back to the rule curve in one timestep.  Converted from m3/day to m3/s  (cms).  
                                                                   //This would be ideal if possible within the given constraints.
      if (currentVolume < targetPoolVolume) //if we want to fill the reservoir
         desiredRelease = pRes->m_minOutflow;
         //desiredRelease = 0.0f;//kbv 3/16/2014

      float actualRelease = desiredRelease;   //Before any constraints are applied


      pRes->m_activeRule = "GC"; pRes->m_constraintValue = 0.f; //Notation from Ressim...implies that guide curve is reached with actual release (e.g. no other constraints applied).

      ZONE zone = ZONE_UNDEFINED;   //zone 0 = top of dam, zone 1 = flood control high, zone 2 = conservation operations, zone 3 = buffer, zone 4 = alternate flood control 1, zone 5 = alternate flood control 2.  Right now, only use 1 and 2 (mmc 3/2/2012). 

      if (currentPoolElevation > pRes->m_dam_top_elev)
      {
         CString msgtop;
         msgtop.Format("Flow: Reservoir elevation at '%s' on day of year %i exceeds dam top elevation.", (LPCTSTR)pRes->m_name, doy);
         Report::StatusMsg(msgtop);
         currentPoolElevation = pRes->m_dam_top_elev - 0.1f;    //Set just below top elev to keep from exceeding values in lookup tables
         zone = ZONE_TOP;
         pRes->m_activeRule = "OverTopElevation!"; pRes->m_constraintValue = 0.f;
      }

      else if (pRes->m_reservoirType == ResType_CtrlPointControl)
      {
         zone = ZONE_CONSERVATION;
      }
      else if (currentPoolElevation > pRes->m_fc1_elev)
      {
         zone = ZONE_TOP;
         pRes->m_activeRule = "OverFC1"; pRes->m_constraintValue = 0.f;
      }
      else if (currentPoolElevation > targetPoolElevation)
      {
         if (currentPoolElevation <= pRes->m_fc2_elev)
            zone = ZONE_ALTFLOODCONTROL1;
         else if (pRes->m_fc3_elev > 0 && currentPoolElevation > pRes->m_fc3_elev)
            zone = ZONE_ALTFLOODCONTROL2;
         else
            zone = ZONE_FLOODCONTROL;
      }
      else if (currentPoolElevation <= targetPoolElevation)
      {
         if (currentPoolElevation <= bufferZoneElevation) //in the buffer zone
         {
            zone = ZONE_BUFFER;
            if (pRes->m_zone == ZONE_CONSERVATION)//if zone changed from conservation
               pRes->m_daysInZoneBuffer = 1; //reset the number of days
            else if (pRes->m_zone = ZONE_BUFFER)//if the zone did not change (you were in the buffer on the previous day)
               pRes->m_daysInZoneBuffer++; //increment the days in the buffer

         }
         else                                           //in the conservation zone
         {
            if (pRes->m_daysInZoneBuffer > 1)
               zone = ZONE_CONSERVATION;
            else
            {
               zone = ZONE_BUFFER;
               if (pRes->m_zone != ZONE_BUFFER) //if zone changed 
                  pRes->m_daysInZoneBuffer = 1; //reset the number of days
               else pRes->m_daysInZoneBuffer++; //increment the days in the buffer
            }
         }
      }
      else
      {
         CString msg;
         msg.Format("*** GetResOutflowWP(): We should never get here. doy = %d, pRes->m_id = %d", doy, pRes->m_id);
         Report::LogMsg(msg);
      }

  // Once we know what zone we are in, we can access the array of appropriate constraints for the particular reservoir and zone.       
      ZoneInfo* pZone = NULL;
      if (zone >= 0 && zone < pRes->m_zoneInfoArray.GetSize())
         pZone = pRes->m_zoneInfoArray.GetAt(zone);

      if (pZone != NULL)
      {
      // Loop through constraints and modify actual release.  Apply the flood control rules in order here.
         for (int i = 0; i < pZone->m_resConstraintArray.GetSize(); i++)
         {
            ResConstraint* pConstraint = pZone->m_resConstraintArray.GetAt(i);
            ASSERT(pConstraint != NULL);

            CString filename = pConstraint->m_constraintFileName;

            // Get first column label and appropriate data to use for lookup
            DataObj* pConstraintTable = pConstraint->m_pRCData;
            ASSERT(pConstraintTable != NULL);

            CString xlabel = pConstraintTable->GetLabel(0);

            int year = -1, month = 1, day = -1;
            float xvalue = 0;
            float yvalue = 0;

            if (_stricmp(xlabel, "date") == 0)                           // Date based rule?  xvalue = current date.
               xvalue = (float)doy;
            else if (_stricmp(xlabel, "release_cms") == 0)              // Release based rule?  xvalue = release last timestep
               xvalue = (float)pRes->m_outflow / SEC_PER_DAY;                    // SEC_PER_DAY = cubic meters per second to cubic meters per day
            else if (_stricmp(xlabel, "pool_elev_m") == 0)              // Pool elevation based rule?  xvalue = pool elevation (meters)
               xvalue = pRes->m_elevation;
            else if (_stricmp(xlabel, "inflow_cms") == 0)               // Inflow based rule?   xvalue = inflow to reservoir
               xvalue = (float)pRes->m_inflow / SEC_PER_DAY;
            else if (_stricmp(xlabel, "Outflow_lagged_24h") == 0)     // 24h lagged outflow based rule?   xvalue = outflow from reservoir at last timestep
               xvalue = (float)pRes->m_outflow / SEC_PER_DAY;                    // placeholder for now
            else if (_stricmp(xlabel, "date_pool_elev_m") == 0)         // Lookup based on two values...date and pool elevation.  x value is date.  y value is pool elevation
            {
               xvalue = (float)doy;
               yvalue = pRes->m_elevation;
            }
            else if (_stricmp(xlabel, "date_water_year_type") == 0)         //Lookup based on two values...date and wateryeartype (storage in 13 USACE reservoirs on May 20th).
            {
               xvalue = (float)doy;
               yvalue = gpModel->m_waterYearType;
            }
            else if (_stricmp(xlabel, "date_release_cms") == 0)
            {
               xvalue = (float)doy;
               yvalue = (float)pRes->m_outflow / SEC_PER_DAY;
            }
            else                                                    //Unrecognized xvalue for constraint lookup table
            {
               CString msg;
               msg.Format("Flow:  Unrecognized x value for reservoir constraint lookup '%s', %s (id:%i) in stream network", (LPCTSTR)pConstraint->m_constraintFileName, (LPCTSTR)pRes->m_name, pRes->m_id);
               Report::WarningMsg(msg);
            }

            RCTYPE type = pConstraint->m_type;
            float constraintValue = 0;

            ASSERT(pConstraint->m_pRCData != NULL);

            switch (type)
            {
               case RCT_MAX:  //maximum
               {
                  if (yvalue > 0)  //Does the constraint depend on two values?  If so, use both xvalue and yvalue
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, yvalue, IM_LINEAR);
                  else             //If not, just use xvalue
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, 1, IM_LINEAR);

                  if (actualRelease >= constraintValue)
                  {
                     actualRelease = constraintValue;
                     pRes->m_activeRule = pConstraint->m_constraintFileName; pRes->m_constraintValue = constraintValue;
                  }
               }
               break;

               case RCT_MIN:  //minimum
               {
                  if (yvalue > 0)  //Does the constraint depend on two values?  If so, use both xvalue and yvalue
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, yvalue, IM_LINEAR);
                  else             //If not, just use xvalue
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, 1, IM_LINEAR);

                  if (actualRelease <= constraintValue)
                  {
                     actualRelease = constraintValue;
                     pRes->m_activeRule = pConstraint->m_constraintFileName; pRes->m_constraintValue = constraintValue;
                  }
               }
               break;

               case RCT_INCREASINGRATE:  //Increasing Rate
               {
                  if (yvalue > 0)  //Does the constraint depend on two values?  If so, use both xvalue and yvalue
                  {
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, yvalue, IM_LINEAR);
                     constraintValue = constraintValue * 24;   //Covert hourly to daily
                  }
                  else             //If not, just use xvalue
                  {
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, 1, IM_LINEAR);
                     constraintValue = constraintValue * 24;   //Covert hourly to daily
                  }

                  if (actualRelease >= (constraintValue + pRes->m_outflow / SEC_PER_DAY))   //Is planned release more than current release + contstraint? 
                  {
                     actualRelease = (float)((pRes->m_outflow / SEC_PER_DAY) + constraintValue);  //If so, planned release can be no more than current release + constraint.
                     pRes->m_activeRule = pConstraint->m_constraintFileName; pRes->m_constraintValue = constraintValue;
                  }
               }
               break;

               case RCT_DECREASINGRATE:  //Decreasing Rate */
               {
                  if (yvalue > 0)  //Does the constraint depend on two values?  If so, use both xvalue and yvalue
                  {
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, yvalue, IM_LINEAR);
                     constraintValue = constraintValue * 24;   //Covert hourly to daily
                  }
                  else             //If not, just use xvalue
                  {
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, 1, IM_LINEAR);
                     constraintValue = constraintValue * 24;   //Covert hourly to daily
                  }

                  if (actualRelease <= (pRes->m_outflow / SEC_PER_DAY - constraintValue))    //Is planned release less than current release - contstraint?
                  {
                     actualRelease = (float)((pRes->m_outflow / SEC_PER_DAY) - constraintValue);     //If so, planned release can be no less than current release - constraint.
                     pRes->m_activeRule = pConstraint->m_constraintFileName; pRes->m_constraintValue = constraintValue;
                  }
               }
               break;

               case RCT_CONTROLPOINT:  //Downstream control point 
               {
                  CString filename = pConstraint->m_constraintFileName;
                 // get control point location.  Assumes last characters of filename contain a COMID
                  LPTSTR p = (LPTSTR)_tcsrchr(filename, '.');

                  if (p != NULL)
                  {
                     p--;
                     while (isdigit(*p))
                        p--;

                     int comID = atoi(p + 1);
                     pConstraint->m_comID = comID;
                  }

               //Determine which control point this is.....use COMID to identify
                  for (int k = 0; k < gpModel->m_controlPointArray.GetSize(); k++)
                  {
                     ControlPoint* pControl = gpModel->m_controlPointArray.GetAt(k);
                     ASSERT(pControl != NULL);

                     if (pControl->InUse())
                     {
                        int location = pControl->m_location;     // Get COMID of this control point
                        //if (pControl->m_location == pConstraint->m_comID)  //Do they match?
                        if (_stricmp(pControl->m_controlPointFileName, pConstraint->m_constraintFileName) == 0)  //compare names
                        {
                           ASSERT(pControl->m_pResAllocation != NULL);
                           constraintValue = 0.0f;
                           int releaseFreq = 1;

                           for (int l = 0; l < pControl->m_influencedReservoirsArray.GetSize(); l++)
                           {
                              if (pControl->m_influencedReservoirsArray[l] == pRes)
                              {
                                 if (pRes->m_reservoirType == ResType_CtrlPointControl)
                                 {
                                    int rowCount = pControl->m_pResAllocation->GetRowCount();
                                    releaseFreq = pControl->m_influencedReservoirsArray[l]->m_releaseFreq;

                                    if (releaseFreq > 1 && doy >= releaseFreq - 1)
                                    {
                                       for (int k = 1; k <= releaseFreq; k++)
                                       {
                                          float tmp = pControl->m_pResAllocation->Get(l, rowCount - k);
                                          constraintValue += pControl->m_pResAllocation->Get(l, rowCount - k);
                                       }
                                       constraintValue = constraintValue / releaseFreq;
                                    }
                  //                  constraintValue = pControl->m_pResAllocation->IGet(0, l);   //Flow allocated from this control point
                                 }
                                 else
                                    constraintValue = pControl->m_pResAllocation->IGet(0, l);   //Flow allocated from this control point

                                 actualRelease += constraintValue;    //constraint value will be negative if flow needs to be withheld, positive if flow needs to be augmented

                                 if (constraintValue > 0.0)         //Did this constraint affect release?  If so, save as active rule.
                                 {
                                    pRes->m_activeRule = pConstraint->m_constraintFileName; pRes->m_constraintValue = actualRelease;
                                 }
                                 break;
                              }
                           }
                        }
                     }
                  }
               }
               break;

               case RCT_POWERPLANT:  //Power plant rule
               { //first, we have to strip the first header to get to the 2nd, which tells us whether the rule is a min or a max 
                  TCHAR powname[256];
                  lstrcpy(powname, filename);
                  TCHAR* ruletype = NULL;
                  TCHAR* next = NULL;
                  TCHAR delim[] = " ,_";  //allowable delimeters: , ; space ; underscore

                  ruletype = _tcstok_s(powname, delim, &next);  //Strip header.  should be pow_ for power plant rules
                  ruletype = _tcstok_s(NULL, delim, &next);  //Returns next string at head of file (max or min).

                  if (_stricmp(ruletype, "Max") == 0)  //Is this a maximum power flow?  Assign m_maxPowerFlow attribute.
                  {
                     ASSERT(pConstraint->m_pRCData != NULL);
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, 1, IM_LINEAR);
                     pRes->m_maxPowerFlow = constraintValue;  //Just for this timestep.  m_gateMaxPowerFlow is the physical limitation for the reservoir.
                  }
                  else if (_stricmp(ruletype, "Min") == 0)   /// bug!!! maximum) == 0)  //Is this a minimum power flow?  Assign m_minPowerFlow attribute.
                  {
                     ASSERT(pConstraint->m_pRCData != NULL);
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, 1, IM_LINEAR);
                     pRes->m_minPowerFlow = constraintValue;
                  }
             //  pRes->m_activeRule = pConstraint->m_constraintFileName;
               }
               break;

               case RCT_REGULATINGOUTLET:  //Regulating outlet rule
               {
               //first, we have to strip the first header to get to the 2nd, which tells us whether the rule is a min or a max 
                  TCHAR roname[256];
                  lstrcpy(roname, filename);
                  TCHAR* ruletype = NULL;
                  TCHAR* next = NULL;
                  TCHAR delim[] = " ,_";  //allowable delimeters: , ; space ; underscore
                  ruletype = _tcstok_s(roname, delim, &next);  //Strip header.  should be pow_ for power plant rules
                  ruletype = _tcstok_s(NULL, delim, &next);  //Returns next string at head of file (max or min).

                  if (_stricmp(ruletype, "Max") == 0)  //Is this a maximum RO flow?   Assign m_maxRO_Flow attribute.
                  {
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, 1, IM_LINEAR);
                     pRes->m_maxRO_Flow = constraintValue;
                  }
                  else if (_stricmp(ruletype, "Min") == 0)  //Is this a minimum RO flow?   Assign m_minRO_Flow attribute.
                  {
                     ASSERT(pConstraint->m_pRCData != NULL);
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, 1, IM_LINEAR);
                     pRes->m_minRO_Flow = constraintValue;
                  }
             //  pRes->m_activeRule = pConstraint->m_constraintFileName;
               }
               break;

               case RCT_SPILLWAY:   //Spillway rule
               {
               //first, we have to strip the first header to get to the 2nd, which tells us whether the rule is a min or a max 
                  TCHAR spillname[256];
                  lstrcpy(spillname, filename);
                  TCHAR* ruletype = NULL;
                  TCHAR* next = NULL;
                  TCHAR delim[] = " ,_";  //allowable delimeters: , ; space ; underscore

                  ruletype = _tcstok_s(spillname, delim, &next);  //Strip header.  should be pow_ for power plant rules
                  ruletype = _tcstok_s(NULL, delim, &next);  //Returns next string at head of file (max or min).

                  if (_stricmp(ruletype, "Max") == 0)  //Is this a maximum spillway flow?  Assign m_maxSpillwayFlow attribute.
                  {
                     ASSERT(pConstraint->m_pRCData != NULL);
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, 1, IM_LINEAR);
                     pRes->m_maxSpillwayFlow = constraintValue;
                  }
                  else if (_stricmp(ruletype, "Min") == 0)  //Is this a minimum spillway flow?  Assign m_minSpillwayFlow attribute.
                  {
                     ASSERT(pConstraint->m_pRCData != NULL);
                     constraintValue = pConstraint->m_pRCData->IGet(xvalue, 1, IM_LINEAR);
                     pRes->m_minSpillwayFlow = constraintValue;
                  }
             //  pRes->m_activeRule = pConstraint->m_constraintFileName;
               }
               break;
            }  // end of: switch( pConstraint->m_type )

            /*
         float minVolum = pRes->GetPoolVolumeFromElevation(m_inactive_elev);
         //float targetPoolElevation = pRes->GetTargetElevationFromRuleCurve(doy);
         float currentVolume = pRes->GetPoolVolumeFromElevation(currentPoolElevation);
         float rc_outflowVolum = actualRelease*SEC_PER_DAY;//volume that would be drained (m3 over the day)


         if (rc_outflowVolum > currentVolume - minVolum)      //In the inactive zone, water is not accessible for release from any of the gates.
            {
            actualRelease = (currentVolume - minVolum)/SEC_PER_DAY*0.5f;
            CString resMsg;

            resMsg.Format("Pool is only %8.1f m above inactive zone. Outflow set to drain %8.1fm above inactive zone.  RC outflow of %8.1f m3/s (from %s) would have resulted in %8.0f m3 of discharged water (over a day) but there is only %8.0f m3 above the inactive zone", pRes->m_elevation - pRes->m_inactive_elev, (pRes->m_elevation - pRes->m_inactive_elev) / 2, rc_outflowVolum / SEC_PER_DAY, pRes->m_activeRule, rc_outflowVolum, currentVolume - minVolum);
               pRes->m_activeRule = resMsg;

            }
            */
            if (actualRelease < 0.0f)
               actualRelease = 0.0f;
            pRes->m_zone = (int)zone;

          // if (actualRelease < pRes->m_minOutflow)              // No release values less than the minimum
          //     actualRelease = pRes->m_minOutflow;

            if (pRes->m_elevation < pRes->m_inactive_elev)      //In the inactive zone, water is not accessible for release from any of the gates.
               actualRelease = (float)(pRes->m_outflow / SEC_PER_DAY * 0.5f);

            outflow = actualRelease;
         }//end of for ( int i=0; i < pZone->m_resConstraintArray.GetSize(); i++ )
      }
   }  // end of: else (not run of river gate flow

//Code here to assign total outflow to powerplant, RO and spillway (including run of river projects) 
   if (pRes->m_reservoirType == ResType_FloodControl || pRes->m_reservoirType == ResType_RiverRun)
      AssignReservoirOutletFlows(pRes, (float)outflow);

   pRes->m_power = (pRes->m_powerFlow > 0.) ? CalculateHydropowerOutput(pRes) : 0.f; // MW

   /* output activ rule to log window
   if (true || pRes->m_id==5)
      { // Dorena is reservoir 5
      CString msg;
      msg.Format("Flow: Day %i: Active Rule for Reservoir %s is %s", doy, (LPCTSTR)pRes->m_name, (LPCTSTR)pRes->m_activeRule);
      Report::LogMsg(msg);
      }
*/
   pRes->m_outflowWP = WaterParcel(outflow * SEC_PER_DAY, pRes->m_resWP.WaterTemperature());
   pRes->m_resWP.Discharge(pRes->m_outflowWP);

   return(pRes->m_outflowWP);
} // end of GetResOutflowWP()


void Reservoir::AssignReservoirOutletFlows( Reservoir *pRes, float outflow )
   {
   ASSERT( pRes != NULL );

   //Reset gate values to zero
   pRes->m_powerFlow = 0.0;
   pRes->m_RO_flow = 0.0;
   pRes->m_spillwayFlow = 0.0;

   if (outflow <= pRes->m_maxPowerFlow)
      pRes->m_powerFlow = outflow;        //If possible, all flow routed through the powerhouse
   else
      {
      pRes->m_powerFlow = pRes->m_maxPowerFlow;            //If not, first route all water possible through the powerhouse
      float excessFlow = outflow - pRes->m_maxPowerFlow;   //Calculate excess
      if (excessFlow <= pRes->m_maxRO_Flow)                //Next, route all remaining flow possible through the regulating outlet(s), if they exist
         {
         pRes->m_RO_flow = excessFlow;
         if (pRes->m_RO_flow < pRes->m_minRO_Flow)      //Check to make sure the RO flow is not below a stated minimum
            {
            pRes->m_RO_flow = pRes->m_minRO_Flow;      //If so increase the RO flow to the minimum and adjust the power flow
            pRes->m_powerFlow = (outflow - pRes->m_minRO_Flow);
            }
         }
      else
         {
         pRes->m_RO_flow = pRes->m_maxRO_Flow;    //If both power plant and RO are at max capacity, route remaining flow to spillway.
         
         excessFlow -= pRes->m_RO_flow;

         pRes->m_spillwayFlow = excessFlow;
         
         if ( pRes->m_spillwayFlow < pRes->m_minSpillwayFlow )   //If spillway has a minimum flow, reduce RO flow to accomodate
            {
            pRes->m_spillwayFlow = pRes->m_minSpillwayFlow;
            pRes->m_RO_flow -= (pRes->m_minSpillwayFlow - excessFlow);
            }
         else if (pRes->m_spillwayFlow > pRes->m_maxSpillwayFlow)    //Does spillway flow exceed the max?  Give warning.
            {
            CString spillmsg;
            spillmsg.Format( "Flow: Maximum spillway volume exceeded at '%s', (id:%i)", (LPCTSTR) pRes->m_name, pRes->m_id  );
            Report::LogMsg( spillmsg );
            }
         }

     
      }
   //Reset max outflows to gate maximums for next timestep
   pRes->m_maxPowerFlow = pRes->m_gateMaxPowerFlow;
   pRes->m_maxRO_Flow = pRes->m_gateMaxRO_Flow;
   pRes->m_maxSpillwayFlow = pRes->m_gateMaxSpillwayFlow;

   // float massbalancecheck = outflow - (pRes->m_powerFlow + pRes->m_RO_flow + pRes->m_spillwayFlow);    //massbalancecheck should = 0

   } // end of AssignReservoirOutletFlows()

float Reservoir::CalculateHydropowerOutput( Reservoir *pRes )
   {
   float head = pRes->m_elevation - pRes->m_tailwater_elevation;
   if (head < 0.f) head = 0.f;

   return ((1000*pRes->m_powerFlow*9.81f*head*pRes->m_turbine_efficiency)/1000000);   //power output in MW
   }

void Reservoir::CollectData( void )
   {

   }


////////////////////////////////////////////////////////////////////////////////////////////////
// F L O W     M O D E L
////////////////////////////////////////////////////////////////////////////////////////////////

FlowModel::FlowModel()
 : m_id( -1 )
 , m_doyTMAX(-1)
 , m_doyTMIN(-1)
 , m_doyTEMP(-1)
 , m_doyPRECIP(-1)
 , m_doyRAD_SW(-1)
 , m_doySPHUMIDITY(-1)
 , m_doyWINDSPEED(-1)

 , m_ICincludesWP(false)
 , m_detailedOutputFlags( 0 )
 , m_reachTree()
 , m_minStreamOrder( 0 )
 , m_subnodeLength( 0 )
 , m_buildCatchmentsMethod( 1 )
 , m_soilLayerCount( 1 )
 , m_vegLayerCount( 0 )
 , m_snowLayerCount( 0 )
 , m_initTemperature( 20 )
 , m_hruSvCount( 0 )
 , m_reachSvCount( 0 )
 , m_reservoirSvCount( 0 )
 , m_colCatchmentArea( -1 )
 , m_colCatchmentJoin( -1 )

 , m_colPRECIP_YR(-1)
 , m_colPRCP_Y_AVG(-1)
 , m_colET_DAY(-1)
 , m_colET_YR(-1)
 , m_colAET_Y_AVG(-1)
 , m_colMAX_ET_YR(-1)
 , m_colSNOWEVAP_Y(-1)
 , m_colSM2ATM(-1)
 , m_colSM2ATM_YR(-1)
 , m_colSNOWCANOPY(-1)
 , m_colP_MINUS_ET(-1)

, m_colIDU_ID(-1)
, m_colAREA(-1)
, m_colHruAREA(-1)

   , m_colGRID_INDEX(-1)
   , m_colTEMP(-1)
   , m_colTMAX(-1)
   , m_colTMIN(-1)
   , m_colPRECIP(-1)
   , m_colRAD_SW(-1)
   , m_colRAD_SW_YR(-1)
   , m_colSPHUMIDITY(-1)
   , m_colRH(-1)
   , m_colWINDSPEED(-1)

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

   , m_colHruTEMP(-1)
   , m_colHruTMAX(-1)
   , m_colHruTMIN(-1)
   , m_colHruPRECIP(-1)
   , m_colHruRAD_SW(-1)
   , m_colHruSPHUMIDITY(-1)
   , m_colHruRH(-1)
   , m_colHruWINDSPEED(-1)

   , m_colHruBOXSURF_M3(-1)
   , m_colHruH2OMELT_M3(-1)
   , m_colHruH2OSTNDGM3(-1)
   , m_colHruNAT_SOIL(-1)
   , m_colHruIRRIG_SOIL(-1)
   , m_colHruGW_FASTBOX(-1)
   , m_colHruGW_SLOWBOX(-1)

   , m_colHruSM2ATM(-1)
   , m_colHruET_DAY(-1)
   , m_colHruAREA_M2(-1)
   , m_colHruAREA_IRRIG(-1)

   , m_colhruCATCH_NDX(-1)
 , m_colCATCHID(-1)
 , m_colhruCATCH_ID(-1)
 , m_colHRU_ID(-1)
 , m_colhruHRU_ID(-1)
 , m_colHRU_NDX(-1)
 , m_colhruCOMID(-1)

 , m_colCatchmentReachIndex( -1 )
 , m_colCatchmentCatchID( -1 )
 , m_colCatchmentHruID( -1 )
 , m_colCatchmentReachId( -1 )
 , m_colStreamOBJECTID(-1)
 , m_colStreamFrom( -1 )
 , m_colStreamTo( -1 )
 , m_colStreamDOBJECTID(-1)
 , m_colStreamLOBJECTID(-1)
 , m_colStreamROBJECTID(-1)
 , m_colReachSTRM_ORDER( -1 )
    , m_colReachWIDTH_MIN(-1)
    , m_colReachDEPTH_MIN(-1)
    , m_colReachZ_MAX(-1)
    , m_colReachZ_MIN(-1)
 , m_colStreamCOMID(-1)
 , m_colStreamCOMID_DOWN(-1)
 , m_colStreamCOMID_LEFT(-1)
 , m_colStreamCOMID_RT(-1)
 , m_colStreamRES_ID(-1)
 , m_colStreamRESOUTFALL(-1)
 , m_colStreamSTRMVERT0X(-1)
 , m_colStreamSTRMVERT0Y(-1)
 , m_colStreamQ_DIV_WRQ(-1)
 , m_colStreamINSTRM_REQ(-1)
 , m_colStreamREACH_H2O(-1)
 //x , m_colReachREACH_EVAP(-1)
    , m_colReachEVAP_MM(-1)
    , m_colReachRAD_LW_OUT(-1)
    , m_colReachRAD_SW_IN(-1)
    , m_colStreamHYDRO_MW(-1)
    , m_colStreamTEMP_AIR(-1)
    , m_colStreamTMAX_AIR(-1)
    , m_colStreamTMIN_AIR(-1)
    , m_colStreamPRECIP(-1)
    , m_colStreamSPHUMIDITY(-1)
    , m_colStreamRH(-1)
    , m_colStreamRAD_SW(-1)
    , m_colStreamWINDSPEED(-1)
    , m_colStreamTMAX_H2O_Y(-1)
    , m_colStreamIDU_ID(-1)
    , m_colHbvW2A_SLP(-1)
   , m_colHbvW2A_INT(-1)
 , m_colHruSWC( -1 )
 , m_colHruTempYr( -1 )
 , m_colHruTemp10Yr( -1 )
 , m_colHruPrecip10Yr( -1 )
 , m_colCLIMATENDX(-1)
 , m_colMAXSNOW(-1 )
 , m_colHruApr1SWE10Yr( -1 )
 , m_colHruApr1SWE( -1 )
 , m_colHRUMeanLAI(-1)
 , m_colMaxET_yr(-1)
 , m_colIrrigation_yr(-1)
 , m_colIrrigation(-1)
 , m_colRunoff_yr(-1)
 , m_colLulcB(-1)
 , m_colLulcA(-1)
 , m_colPVT(-1)
 , m_colAgeClass(-1)
 , m_colIRRIGATION(-1)
 , m_colSM_DAY(-1)
 , m_colARIDITYNDX(-1)
 , m_colReachLOG_Q(-1)
 , m_colResID( -1 )
 , m_colReachHRU_ID(-1)
 , m_colReachHRU_FRAC(-1)
 , m_colStreamCumArea( -1 )
 , m_colCatchmentCumArea( -1 )
 , m_colTreeID( -1 )
 , m_pMap( NULL )
 , m_pCatchmentLayer( NULL )
 , m_pStreamLayer( NULL )
 , m_pLinkLayer(NULL)
 , m_pResLayer( NULL )
 , m_stopTime( 0.0f )
 , m_currentTime( 0.0f )
 , m_timeStep( 1.0f )
 , m_collectionInterval( 1 )
 , m_useVariableStep( false )
 , m_rkvMaxTimeStep(10.0f)
 , m_rkvMinTimeStep(0.001f)
 , m_rkvTolerance(0.0001f)
 , m_rkvInitTimeStep(0.001f)
 , m_rkvTimeStep(1.0f)
 , m_pReachRouting( NULL )
 , m_pLateralExchange( NULL )
 , m_pHruVertExchange( NULL )

 //, m_extraSvRxnMethod;
 , m_extraSvRxnExtFnDLL( NULL )
 , m_extraSvRxnExtFn( NULL )
 //,  RESMETHOD  m_reservoirFluxMethod;
 , m_reservoirFluxExtFnDLL( NULL )
 , m_reservoirFluxExtFn( NULL )
 , m_pResInflows( NULL )
 , m_pCpInflows( NULL )

 //, m_reachSolutionMethod( RSM_RK4 )
 //, m_latExchMethod( HREX_LINEARRESERVOIR )
 //, m_hruVertFluxMethod( VD_BROOKSCOREY )
 //, m_gwMethod( GWT_NONE )
 , m_extraSvRxnMethod( EXSV_EXTERNAL )
 , m_reservoirFluxMethod( RES_RESSIMLITE)
 //, m_reachExtFnDLL( NULL )
 //, m_reachExtFn( NULL )
 //, m_reachTimeStep( 1.0f )
 //, m_latExchExtFnDLL( NULL )
 //, m_latExchExtFn( NULL )
 //, m_hruVertFluxExtFnDLL( NULL )
 //, m_hruVertFluxExtFn( NULL )
 //, m_gwExtFnDLL( NULL )
 //, m_gwExtFn( NULL )

 , m_wdRatio( 10.0f ) //?????
 , m_minCatchmentArea( -1 )
 , m_maxCatchmentArea( -1 )
 , m_numReachesNoCatchments(0)
 , m_numHRUs(0)
    , m_pQE_IDU(NULL)
    , m_pQE_Reach(NULL)
    , m_pQE_HRU(NULL)
    , m_pQE_XHRU(NULL)
    , m_pQE_Link(NULL)
    , m_pQE_Node(NULL)
    , m_pQE_Subcatch(NULL)
    , m_pME_IDU(NULL)
    , m_pME_Reach(NULL)
    , m_pME_HRU(NULL)
    , m_pME_XHRU(NULL)
    , m_pME_Link(NULL)
    , m_pME_Node(NULL)
    , m_pME_Subcatch(NULL)

 , m_pStreamQuery( NULL )
 , m_pCatchmentQuery( NULL )
 //, m_pReachDischargeData( NULL )
 //, m_pHRUPrecipData( NULL )
 //, m_pHRUETData(NULL)
 , m_pTotalFluxData( NULL )
 , m_pGlobalFlowData( NULL )
 , m_mapUpdate( 2 )
 , m_gridClassification(2)
 , m_totalWaterInputRate( 0 )
 , m_totalWaterOutputRate( 0 )
 , m_totalWaterInput( 0 )
 , m_totalWaterOutput( 0 )
 , m_numQMeasurements(0)
 , m_waterYearType(2.0f)
 , m_pErrorStatData(NULL)
 , m_pParameterData(NULL)
 , m_pDischargeData(NULL)
 , m_estimateParameters(false)
 , m_numberOfRuns(5)
 , m_numberOfYears(1)
 , m_nsThreshold(0.0f)
 , m_saveResultsEvery(10)
 , m_pClimateStationData(NULL)
 , m_pHruGrid(NULL)
 , m_timeOffset(0)
 , m_path()
 , m_grid()
 , m_climateFilePath()
 , m_climateStationElev(0.0f)
 , m_loadClimateRunTime( 0.0f )   
 , m_globalFlowsRunTime( 0.0f )   
 , m_externalMethodsRunTime( 0.0f )   
 , m_gwRunTime( 0.0f )   
 , m_hruIntegrationRunTime( 0.0f )   
 , m_reservoirIntegrationRunTime( 0.0f )   
 , m_reachIntegrationRunTime( 0.0f )   
 , m_massBalanceIntegrationRunTime( 0.0f )
 , m_totalFluxFnRunTime( 0.0f )
 , m_reachFluxFnRunTime( 0.0f )
 , m_hruFluxFnRunTime( 0.0f )
 , m_collectDataRunTime( 0.0f )
 , m_outputDataRunTime( 0.0f )
 , m_checkMassBalance( 0 )
 , m_integrator(IM_RK4)
 , m_minTimeStep(0.001f)
 , m_maxTimeStep(1.0f)
 , m_initTimeStep(1.0f)
 , m_errorTolerance(0.0001f)
 , m_useRecorder( false )
 , m_useStationData( false )
 , m_currentFlowScenarioIndex( 0 )
 , m_waterRightsReportInputVar(0)
 , m_currentEnvisionScenarioIndex( -1 )
 , m_pGrid(NULL)
 , m_minElevation(float(1E-6))
 , m_volumeMaxSWE(0.0f)
 , m_dateMaxSWE(0)
 , m_annualTotalET( 0 )     // acre-ft
 , m_annualTotalPrecip( 0 ) // acre-ft
 , m_annualTotalDischarge( 0 )  //acre-ft
 , m_annualTotalRainfall( 0 )  //acre-ft
 , m_annualTotalSnowfall( 0 )  //acre-ft
 , m_randNorm1(1,1,1)
   , m_StartYear_totH2O_m3(0.)
   , m_StartYear_totReaches_m3(0.)
   , m_StartYear_totReservoirs_m3(0.)
   , m_StartYear_totHRUs_m3(0.)
   , m_totArea(0.f)
   , m_parameter1(0.f)
   , m_parameter2(0.f)
   {
   gpModel = this;
   gFlowModel = this;
//x   gFM = this;

   m_scenarioArray.RemoveAll();

   gpFlow->AddInputVar( "Climate Scenario", m_currentFlowScenarioIndex, "0=MIROC, 1=GFDL, 2=HadGEM, 3=Stationary using MIROC, "
      "4=MACA actual weather 1979-2011, 5=BaselineGrid, 6=BaselinePoly, 7=GriddedRecentWeatherForDemos, 8=BaselineGridMultiyearFiles" );    
   gpFlow->AddInputVar( "Use Parameter Estimation", m_estimateParameters, "true if the model will be used to estimate parameters" );    
   gpFlow->AddInputVar("Grid Classification?", m_gridClassification, "For Gridded Simulation:  Classify attribute 0,1,2?");
   gpFlow->AddInputVar("Water Rights Report", m_waterRightsReportInputVar, "0=off, 1=on");

   gpFlow->AddOutputVar("Date of Max Snow", m_dateMaxSWE, "DOY of Max Snow");
   gpFlow->AddOutputVar("Volume Max Snow (cubic meters)", m_volumeMaxSWE, "Volume Max Snow (m3)");
   } // end of constructor for FlowModel
 

FlowModel::~FlowModel()
   {
   if ( m_pStreamQE )
      delete m_pStreamQE; 
   
   if ( m_pCatchmentQE ) 
      delete m_pCatchmentQE; 

   if (m_pQE_IDU != NULL) delete m_pQE_IDU;
   if (m_pQE_Reach != NULL) delete m_pQE_Reach;
   if (m_pQE_HRU != NULL) delete m_pQE_HRU;
   if (m_pQE_XHRU != NULL) delete m_pQE_XHRU;
   if (m_pQE_Link != NULL) delete m_pQE_Link;
   if (m_pQE_Node != NULL) delete m_pQE_Node;
   if (m_pQE_Subcatch != NULL) delete m_pQE_Subcatch;

   if (m_pME_IDU != NULL) delete m_pME_IDU;
   if (m_pME_Reach != NULL) delete m_pME_Reach;
   if (m_pME_HRU != NULL) delete m_pME_HRU;
   if (m_pME_XHRU != NULL) delete m_pME_XHRU;
   if (m_pME_Link != NULL) delete m_pME_Link;
   if (m_pME_Node != NULL) delete m_pME_Node;
   if (m_pME_Subcatch != NULL) delete m_pME_Subcatch;

   if ( m_pGlobalFlowData != NULL)
      delete m_pGlobalFlowData;

   if ( m_pTotalFluxData != NULL )
      delete m_pTotalFluxData;
   
   if ( m_pErrorStatData != NULL)
      delete m_pErrorStatData;
  
   if ( m_pParameterData != NULL)
      delete m_pParameterData;

   if ( m_pDischargeData != NULL)
      delete m_pDischargeData;

   if ( m_pClimateStationData != NULL)
      { 
      delete m_pClimateStationData;
      m_pClimateStationData = NULL;
      }

   if (m_pHruGrid != NULL)
      delete m_pHruGrid;

   // these are PtrArrays
   m_catchmentArray.RemoveAll();
   m_wetlArray.RemoveAll();
   m_reservoirArray.RemoveAll();
   m_fluxInfoArray.RemoveAll();
   m_stateVarArray.RemoveAll();
   m_vertexArray.RemoveAll();
   m_reachHruLayerDataArray.RemoveAll();
   m_hruLayerExtraSVDataArray.RemoveAll();
   m_reservoirDataArray.RemoveAll();
   m_reachMeasuredDataArray.RemoveAll();
   m_tableArray.RemoveAll();
   m_parameterArray.RemoveAll();
   }


bool FlowModel::InitClimateMeanValues(EnvContext *pContext)
// Called during InitRun().
{
   CString msg; msg.Format("InitClimateMeanValues() starting now."); Report::LogMsg(msg);
   ClimateScenario *pScenario = m_scenarioArray.GetAt(m_currentFlowScenarioIndex);

   MapLayer *pIDUlayer = (MapLayer*)pContext->pMapLayer;
   int first_year_for_climate_averages = pContext->pEnvModel->m_referenceStartYear - pContext->m_yearsInStartingClimateAverages;
   while (first_year_for_climate_averages < pContext->pEnvModel->m_referenceStartYear && !OpenClimateDataFiles(first_year_for_climate_averages))
         first_year_for_climate_averages++;
   if (first_year_for_climate_averages >= pContext->pEnvModel->m_referenceStartYear)
   {
      msg.Format("FlowModel::InitClimateMeanValues() Climate data starts in the same or a later year than referenceStartYear; "
         "unable to calculate long term means.  m_referenceStartYear = %d",
         pContext->pEnvModel->m_referenceStartYear);
      Report::ErrorMsg(msg);
      return false;
   }

   // Calculate the long term mean precip for June, July, Aug, and the growing season (April-Sept) as a whole.

   int initial_tau = pContext->pEnvModel->m_referenceStartYear - first_year_for_climate_averages;
   if (initial_tau < pContext->m_yearsInStartingClimateAverages)
   {
      msg.Format("FlowModel::InitClimateMeanValues() Climate data starts fewer than %d years before the simulation start year;\n"
         "initial values of means will be derived from only %d years of climate data.", pContext->pEnvModel->m_referenceStartYear, initial_tau);
      Report::LogMsg(msg);
   }

   // Zero out all the mean value attributes.
   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      int lulc_a;  m_pIDUlayer->GetData(idu, m_colLulcA, lulc_a);
      float single_year_token_value =  0.f;

      m_pIDUlayer->SetDataU(idu, m_colTMIN_GROW, single_year_token_value);
      m_pIDUlayer->SetDataU(idu, m_colPRCP_GROW, single_year_token_value);
      m_pIDUlayer->SetDataU(idu, m_colPRCPSPRING, single_year_token_value);
      m_pIDUlayer->SetDataU(idu, m_colPRCPWINTER, single_year_token_value);
      m_pIDUlayer->SetDataU(idu, m_colPRCP_JUN, single_year_token_value);
      m_pIDUlayer->SetDataU(idu, m_colPRCP_JUL, single_year_token_value);
      m_pIDUlayer->SetDataU(idu, m_colPRCP_AUG, single_year_token_value);

      float multi_year_token_value = TOKEN_INIT_EXP_AVG;
      m_pIDUlayer->SetDataU(idu, m_colTMINGROAVG, multi_year_token_value);
      m_pIDUlayer->SetDataU(idu, m_colPRCPGROAVG, multi_year_token_value);
      m_pIDUlayer->SetDataU(idu, m_colPRCPJUNAVG, multi_year_token_value);
      m_pIDUlayer->SetDataU(idu, m_colPRCPJULAVG, multi_year_token_value);
      m_pIDUlayer->SetDataU(idu, m_colPRCPAUGAVG, multi_year_token_value);
   }

   // Calculate TMIN_GROW, PRCP_GROW, PRCP_JUN, PRCP_JUL, and PRCP_AUG for the initial year of the initial mean values
   // OpenClimateDataFiles(first_year_for_climate_averages) has already been called above.
   int days_in_first_year_for_climate_averages = min(GetDaysInCalendarYear(first_year_for_climate_averages), pContext->m_maxDaysInYear);
   for (int doy = 0; doy < days_in_first_year_for_climate_averages; doy++) 
      DailyUpdateToSingleYearWeatherAverages(doy, days_in_first_year_for_climate_averages, first_year_for_climate_averages);

   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      float tmin_grow; m_pIDUlayer->GetData(idu, m_colTMIN_GROW, tmin_grow);
      m_pIDUlayer->SetDataU(idu, m_colTMIN_GROW, tmin_grow / 214.f); // There are 214 days in Apr-Oct.
   } // end of loop through IDUs

   UpdateIDUclimateTemporalAverages(pContext->m_yearsInStartingClimateAverages, pContext);

   int year = first_year_for_climate_averages + 1;
   while (year < pContext->pEnvModel->m_referenceStartYear)
   {
      if (!OpenClimateDataFiles(year)) return(false);
      int days_in_year = GetDaysInCalendarYear(year);
      for (int doy = 0; doy < days_in_year; doy++) DailyUpdateToSingleYearWeatherAverages(doy, days_in_year, year);
      for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
      {
         float tmin_grow; m_pIDUlayer->GetData(idu, m_colTMIN_GROW, tmin_grow);
         if (tmin_grow > -98.f)
            m_pIDUlayer->SetDataU(idu, m_colTMIN_GROW, tmin_grow / 214.f); // There are 214 days in Apr-Oct.
      } // end of loop through IDUs
      UpdateIDUclimateTemporalAverages(initial_tau, pContext);
      year++;
   } // end of while (year < pContext->pEnvModel->m_referenceStartYear)

   if (!OpenClimateDataFiles(m_flowContext.pEnvContext->startYear)) return(false);

   msg.Format("InitClimateMeanValues() ending now."); Report::LogMsg(msg);
   return(true);
} // end of InitClimateMeanValues()


bool FlowModel::UpdateIDUclimateTemporalAverages(int tau, EnvContext * pContext)
{
   float exp_term, one_minus_term;
   bool calculate_terms = true;
   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      float tmin_grow; m_pIDUlayer->GetData(idu, m_colTMIN_GROW, tmin_grow);

      if (calculate_terms)
      {
         exp_term = (float)exp(-1. / tau);
         one_minus_term = 1.f - exp_term;
         calculate_terms = false;
      }

      float tmingroavg; m_pIDUlayer->GetData(idu, m_colTMINGROAVG, tmingroavg);
      if (INITIALIZE_EXP_AVG(tmingroavg))
      { // This IDU just transitioned to ag. Initialize the averages with the current year values.
         exp_term = 0.f;
         one_minus_term = 1.f;
         calculate_terms = true;
      }

      m_pIDUlayer->SetDataU(idu, m_colTMINGROAVG, tmingroavg * exp_term + tmin_grow * one_minus_term);

      float prcpgroavg; m_pIDUlayer->GetData(idu, m_colPRCPGROAVG, prcpgroavg);
      float prcp_grow; m_pIDUlayer->GetData(idu, m_colPRCP_GROW, prcp_grow);
      m_pIDUlayer->SetDataU(idu, m_colPRCPGROAVG, prcpgroavg * exp_term + prcp_grow * one_minus_term);

      float prcpjunavg; m_pIDUlayer->GetData(idu, m_colPRCPJUNAVG, prcpjunavg);
      float prcp_jun; m_pIDUlayer->GetData(idu, m_colPRCP_JUN, prcp_jun);
      m_pIDUlayer->SetDataU(idu, m_colPRCPJUNAVG, prcpjunavg * exp_term + prcp_jun * one_minus_term);

      float prcpjulavg; m_pIDUlayer->GetData(idu, m_colPRCPJULAVG, prcpjulavg);
      float prcp_jul; m_pIDUlayer->GetData(idu, m_colPRCP_JUL, prcp_jul);
      m_pIDUlayer->SetDataU(idu, m_colPRCPJULAVG, prcpjulavg * exp_term + prcp_jul * one_minus_term);

      float prcpaugavg; m_pIDUlayer->GetData(idu, m_colPRCPAUGAVG, prcpaugavg);
      float prcp_aug; m_pIDUlayer->GetData(idu, m_colPRCP_AUG, prcp_aug);
      m_pIDUlayer->SetDataU(idu, m_colPRCPAUGAVG, prcpaugavg * exp_term + prcp_aug * one_minus_term);
   }

   return(true);
} // end of UpdateIDUclimateTemporalAverages()


bool FlowModel::UpdateAgBasinClimateTemporalAverages(EnvContext * pEnvContext)
{
   // Multi-year temporal averages for individual IDUs already exist as IDU attributes TMINGROAVG and PRCPGROAVG.
   // This routine calculates the ag basin spatial averages of those 2 attributes, and stores to
   // EnvContext.m_BasinMeanTminGrowingSeason and m_BasinMeanPrcpGrowingSeason.

   double area_accum = 0.;
   double prcp_accum = 0.;
   double tmin_accum = 0.;
   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      float tmingroavg; m_pIDUlayer->GetData(idu, m_colTMINGROAVG, tmingroavg);
      float prcpgroavg; m_pIDUlayer->GetData(idu, m_colPRCPGROAVG, prcpgroavg);
      float area_idu; m_pIDUlayer->GetData(idu, m_colAREA, area_idu);

      prcp_accum += prcpgroavg * area_idu;
      tmin_accum += tmingroavg * area_idu;
      area_accum += area_idu;
   } // idu loop

   pEnvContext->m_BasinMeanTminGrowingSeason = area_accum != 0 ? (float)(tmin_accum / area_accum) : 0.f;
   pEnvContext->m_BasinMeanPrcpGrowingSeason = area_accum != 0 ? (float)(prcp_accum / area_accum) : 0.f;

   CString msg;
   msg.Format("UpdateAgBasinClimateTemporalAverages() m_BasinMeanTminGrowingSeason = %f, m_BasinMeanPrcpGrowingSeason = %f", 
      pEnvContext->m_BasinMeanTminGrowingSeason, pEnvContext->m_BasinMeanPrcpGrowingSeason);
   Report::LogMsg(msg);

   return(true);
} // end of UpdateAgBasinClimateTemporalAverages()


bool FlowModel::DailyUpdateToSingleYearWeatherAverages(int doy, int daysInYear, int year)
{
   int month, day; GetCalDate0(doy, &month, &day, daysInYear);
   if (month >= 11) return(true);

   GetDailyWeatherField(CDT_PRECIP, doy, year);
   GetDailyWeatherField(CDT_TMIN, doy, year);
   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      if (doy == 0)
      {
         m_pIDUlayer->SetDataU(idu, m_colPRCPWINTER, 0.);
         m_pIDUlayer->SetDataU(idu, m_colTMIN_GROW, 0.);
         m_pIDUlayer->SetDataU(idu, m_colPRCP_GROW, 0.);
         m_pIDUlayer->SetDataU(idu, m_colPRCPSPRING, 0.);
         m_pIDUlayer->SetDataU(idu, m_colPRCP_JUN, 0.);
         m_pIDUlayer->SetDataU(idu, m_colPRCP_JUL, 0.);
         m_pIDUlayer->SetDataU(idu, m_colPRCP_AUG, 0.);
      }

      float precip; m_pIDUlayer->GetData(idu, m_colPRECIP, precip);
      float tmin_grow; m_pIDUlayer->GetData(idu, m_colTMIN_GROW, tmin_grow);

      if (month <= 3)
      { // Here "winter" is interpreted as January through March.
         float prcpwinter; m_pIDUlayer->GetData(idu, m_colPRCPWINTER, prcpwinter);
         m_pIDUlayer->SetDataU(idu, m_colPRCPWINTER, precip + prcpwinter);
      }
      else 
      { // Econ models define "growing season" as April through October
         float tmin; m_pIDUlayer->GetData(idu, m_colTMIN, tmin);
         m_pIDUlayer->SetDataU(idu, m_colTMIN_GROW, tmin_grow + tmin);

         float prcp_grow; m_pIDUlayer->GetData(idu, m_colPRCP_GROW, prcp_grow);
         m_pIDUlayer->SetDataU(idu, m_colPRCP_GROW, prcp_grow + precip);

         if (month <= 6)
         { // Econ models define "spring" as April through June
            float prcpspring; m_pIDUlayer->GetData(idu, m_colPRCPSPRING, prcpspring);
            m_pIDUlayer->SetDataU(idu, m_colPRCPSPRING, prcpspring + precip);
         }

         if (month == 6)
         { // June
            float prcp_jun; m_pIDUlayer->GetData(idu, m_colPRCP_JUN, prcp_jun);
            m_pIDUlayer->SetDataU(idu, m_colPRCP_JUN, precip + prcp_jun);
         }

         if (month == 7)
         { // July
            float prcp_jul; m_pIDUlayer->GetData(idu, m_colPRCP_JUL, prcp_jul);
            m_pIDUlayer->SetDataU(idu, m_colPRCP_JUL, precip + prcp_jul);
         }

         if (month == 8)
         { // August
            float prcp_aug; m_pIDUlayer->GetData(idu, m_colPRCP_AUG, prcp_aug);
            m_pIDUlayer->SetDataU(idu, m_colPRCP_AUG, precip + prcp_aug);
         } // end of logic for August
      } // end of ...else if (4 <= month && month <= 10)
   } // end of loop through IDUs

   return(true);
} // end of DailyUpdateToSingleYearWeatherAverages()


bool FlowModel::Init( EnvContext *pEnvContext )
   {
   m_id = pEnvContext->id;

   ASSERT( pEnvContext->pMapLayer != NULL );
   m_pMap = pEnvContext->pMapLayer->GetMapPtr();

   m_flowContext.Reset();
   m_flowContext.timing = GMT_INIT;
   m_flowContext.pFlowModel = this;
   m_flowContext.pEnvContext = pEnvContext;
   pEnvContext->m_pFlowContext = &m_flowContext;

   pEnvContext->pEnvModel->m_pFlowModel = this;

   gIDUs = m_pIDUlayer = (IDUlayer*)pEnvContext->pMapLayer;
   m_pReachLayer = (MapLayer*)pEnvContext->pReachLayer;
   m_pHRUlayer = (MapLayer*)pEnvContext->pHRUlayer;
   m_pLinkLayer = (MapLayer*)pEnvContext->pLinkLayer;
   m_pNodeLayer = (MapLayer*)pEnvContext->pNodeLayer;
   m_pSubcatchLayer = (MapLayer*)pEnvContext->pSubcatchmentLayer;

   if ( m_catchmentLayer.IsEmpty() )
      {
      m_pCatchmentLayer = m_pMap->GetLayer( 0 ); 
      m_catchmentLayer = m_pCatchmentLayer->m_name;
      }
   else
      m_pCatchmentLayer = m_pMap->GetLayer( m_catchmentLayer );
   
   if ( m_pCatchmentLayer == NULL )
      {
      CString msg( "Flow: Unable to locate catchment layer '" );
      msg += m_catchmentLayer;
      msg += "' - this is a required layer";
      Report::ErrorMsg( msg, "Fatal Error" );
      return false;
      }

   m_pStreamLayer = m_pMap->GetLayer(m_streamLayer);
   if (m_pStreamLayer == NULL)
   {
      CString msg("Flow: Unable to locate stream network layer '");
      msg += m_streamLayer;
      msg += "' - this is a required layer";
      Report::ErrorMsg(msg, "Fatal Error");
      return false;
   }

   m_pLinkLayer = (MapLayer *)pEnvContext->pLinkLayer;

   if (m_pLinkLayer == NULL)
   {
      CString msg("Flow: Unable to locate Link layer '");
      msg += "' - this layer is required for running SWMM.";
      Report::LogMsg(msg);
   }

   // columns in IDU layer and similar columns in other layers
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colElev,                 m_elevCol,          TYPE_FLOAT, CC_AUTOADD );
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colLai,                  _T("LAI"),          TYPE_FLOAT, CC_AUTOADD );
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colAgeClass,             _T("AGECLASS"),     TYPE_INT, CC_AUTOADD );
   m_pIDUlayer->CheckCol(m_colIRRIGATION, "IRRIGATION", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colSM_DAY, "SM_DAY", TYPE_FLOAT, CC_MUST_EXIST);
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colLulcB,                _T("LULC_B"),       TYPE_INT, CC_AUTOADD );
   EnvExtension::CheckCol(m_pCatchmentLayer, m_colLulcA, _T("LULC_A"), TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pCatchmentLayer, m_colPVT, _T("PVT"), TYPE_INT, CC_MUST_EXIST);
   EnvExtension::CheckCol(m_pCatchmentLayer, m_colVEGCLASS, _T("VEGCLASS"), TYPE_INT, CC_MUST_EXIST);
   EnvExtension::CheckCol(m_pCatchmentLayer, m_colAGECLASS, _T("AGECLASS"), TYPE_INT, CC_MUST_EXIST);
   EnvExtension::CheckCol(m_pCatchmentLayer, m_colLAI, _T("LAI"), TYPE_FLOAT, CC_MUST_EXIST);
   EnvExtension::CheckCol(m_pCatchmentLayer, m_colTREE_HT, _T("TREE_HT"), TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pCatchmentLayer, m_colCatchmentJoin, m_catchmentJoinCol, TYPE_INT, CC_MUST_EXIST);

   EnvExtension::CheckCol(m_pIDUlayer, m_colPRECIP_YR, "PRECIP_YR", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colPRCP_Y_AVG, "PRCP_Y_AVG", TYPE_DOUBLE, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colET_DAY, "ET_DAY", TYPE_FLOAT, CC_AUTOADD); m_pIDUlayer->SetColDataU(m_colET_DAY, 0);
   EnvExtension::CheckCol(m_pIDUlayer, m_colET_YR, "ET_YR", TYPE_FLOAT, CC_AUTOADD); m_pIDUlayer->SetColDataU(m_colET_YR, 0);
   EnvExtension::CheckCol(m_pIDUlayer, m_colAET_Y_AVG, "AET_Y_AVG", TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colMAX_ET_YR, "MAX_ET_YR", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colSNOWEVAP_Y, "SNOWEVAP_Y", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colSM2ATM, "SM2ATM", TYPE_FLOAT, CC_AUTOADD); m_pIDUlayer->SetColDataU(m_colSM2ATM, 0);
   m_pIDUlayer->CheckCol(m_colSM2ATM_YR, "SM2ATM_YR", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colSNOW_SWE, "SNOW_SWE", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colH2O_MELT, "H2O_MELT", TYPE_DOUBLE, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colFIELD_CAP, "FIELD_CAP", TYPE_DOUBLE, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colSNOWCANOPY, "SNOWCANOPY", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colP_MINUS_ET, "P_MINUS_ET", TYPE_FLOAT, CC_AUTOADD);

   EnvExtension::CheckCol(m_pIDUlayer, m_colIDU_ID, "IDU_ID", TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colHBVCALIB, "HBVCALIB", TYPE_INT, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruHBVCALIB, "HBVCALIB", TYPE_INT, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruFIELD_CAP, "FIELD_CAP", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colECOREGION, "ECOREGION", TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colAREA, "AREA", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colELEV_MEAN, "ELEV_MEAN", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pHRUlayer, m_colHruAREA, "AREA_M2", TYPE_FLOAT, CC_AUTOADD);

   m_pIDUlayer->CheckCol(m_colGRID_INDEX, "GRID_INDEX", TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colTEMP, "TEMP", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colTMAX, "TMAX", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colTMIN, "TMIN", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colPRECIP, "PRECIP", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colRAD_SW, "RAD_SW", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colRAD_SW_YR, "RAD_SW_YR", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colSPHUMIDITY, "SPHUMIDITY", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colRH, "RH", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colWINDSPEED, "WINDSPEED", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colF_THETA, "F_THETA", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colVPD_SCALAR, "VPD_SCALAR", TYPE_FLOAT, CC_AUTOADD);

   EnvExtension::CheckCol(m_pIDUlayer, m_colTMIN_GROW, "TMIN_GROW", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colPRCP_GROW, "PRCP_GROW", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colPRCP_JUN, "PRCP_JUN", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colPRCP_JUL, "PRCP_JUL", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colPRCP_AUG, "PRCP_AUG", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colPRCPSPRING, "PRCPSPRING", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colPRCPWINTER, "PRCPWINTER", TYPE_FLOAT, CC_AUTOADD);

   EnvExtension::CheckCol(m_pIDUlayer, m_colTMINGROAVG, "TMINGROAVG", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colPRCPGROAVG, "PRCPGROAVG", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colPRCPJUNAVG, "PRCPJUNAVG", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colPRCPJULAVG, "PRCPJULAVG", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colPRCPAUGAVG, "PRCPAUGAVG", TYPE_FLOAT, CC_AUTOADD);

   EnvExtension::CheckCol(m_pIDUlayer, m_colCENTROIDX, "CENTROIDX", TYPE_LONG, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colCENTROIDY, "CENTROIDY", TYPE_LONG, CC_AUTOADD);

   m_pHRUlayer->CheckCol(m_colHruHRU_ID, "HRU_ID", TYPE_INT, CC_MUST_EXIST);
   m_pHRUlayer->CheckCol(m_colHruCOMID, "COMID", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colCOMID, "COMID", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colWETNESS, "WETNESS", TYPE_DOUBLE, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colWETL_CAP, "WETL_CAP", TYPE_DOUBLE, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colWETL2Q, "WETL2Q", TYPE_DOUBLE, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colWETL_ID, "WETL_ID", TYPE_INT, CC_MUST_EXIST);

   EnvExtension::CheckCol(m_pHRUlayer, m_colHruTEMP, "TEMP", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pHRUlayer, m_colHruTMAX, "TMAX", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pHRUlayer, m_colHruTMIN, "TMIN", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pHRUlayer, m_colHruPRECIP, "PRECIP", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pHRUlayer, m_colHruRAD_SW, "RAD_SW", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pHRUlayer, m_colHruSPHUMIDITY, "SPHUMIDITY", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pHRUlayer, m_colHruRH, "RH", TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pHRUlayer, m_colHruWINDSPEED, "WINDSPEED", TYPE_FLOAT, CC_AUTOADD);

   m_pHRUlayer->CheckCol(m_colHruSNOW_BOX, "SNOW_BOX", TYPE_FLOAT, CC_AUTOADD); 
   m_pHRUlayer->CheckCol(m_colHruBOXSURF_M3, "BOXSURF_M3", TYPE_DOUBLE, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruH2OMELT_M3, "H2OMELT_M3", TYPE_DOUBLE, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruH2OSTNDGM3, "H2OSTNDGM3", TYPE_DOUBLE, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruNAT_SOIL, "NAT_SOIL", TYPE_FLOAT, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruIRRIG_SOIL, "IRRIG_SOIL", TYPE_FLOAT, CC_AUTOADD); 
   m_pHRUlayer->CheckCol(m_colHruGW_FASTBOX, "GW_FASTBOX", TYPE_FLOAT, CC_AUTOADD); 
   m_pHRUlayer->CheckCol(m_colHruGW_SLOWBOX, "GW_SLOWBOX", TYPE_FLOAT, CC_AUTOADD); 

   m_pHRUlayer->CheckCol(m_colHruSM2ATM, "SM2ATM", TYPE_FLOAT, CC_AUTOADD); 
   m_pHRUlayer->CheckCol(m_colHruET_DAY, "ET_DAY", TYPE_FLOAT, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruAREA_M2, "AREA_M2", TYPE_FLOAT, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruAREA_IRRIG, "AREA_IRRIG", TYPE_FLOAT, CC_AUTOADD);

   EnvExtension::CheckCol(m_pHRUlayer, m_colhruCATCH_NDX, "CATCH_NDX", TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colCATCHID, "CATCHID", TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pHRUlayer, m_colhruCATCH_ID, "CATCH_ID", TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pIDUlayer, m_colHRU_ID, "HRU_ID", TYPE_INT, CC_AUTOADD);
   if (!m_flowContext.pEnvContext->coldStartFlag) EnvExtension::CheckCol(m_pHRUlayer, m_colhruHRU_ID, "HRU_ID", TYPE_INT, CC_MUST_EXIST);
   EnvExtension::CheckCol(m_pIDUlayer, m_colHRU_NDX, "HRU_NDX", TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pHRUlayer, m_colhruCOMID, "COMID", TYPE_INT, CC_MUST_EXIST);

   EnvExtension::CheckCol( m_pCatchmentLayer, m_colCatchmentCatchID,     m_catchIDCol,       TYPE_INT, CC_AUTOADD );
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colCatchmentHruID,       m_hruIDCol,         TYPE_INT, CC_AUTOADD );
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colHruTempYr,            _T("TEMP_YR"),      TYPE_FLOAT, CC_AUTOADD );
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colHruTemp10Yr,          _T("TEMP_10YR"),    TYPE_FLOAT, CC_AUTOADD );
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colHruPrecip10Yr,        _T("PRCP_10YR"),    TYPE_FLOAT, CC_AUTOADD );
   EnvExtension::CheckCol(m_pCatchmentLayer, m_colCLIMATENDX, _T("CLIMATENDX"), TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colMAXSNOW,            _T("MAXSNOW"),      TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colHruApr1SWE,           _T("SNOW_APR"),     TYPE_FLOAT, CC_AUTOADD );
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colHruApr1SWE10Yr,       _T("SNOW_APR10"),   TYPE_FLOAT, CC_AUTOADD );
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colARIDITYNDX,         _T("ARIDITYNDX"),   TYPE_FLOAT, CC_AUTOADD );  

   EnvExtension::CheckCol( m_pCatchmentLayer, m_colMaxET_yr,              _T("MAX_ET_yr"),    TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colRunoff_yr  ,          _T("Runoff_yr"),    TYPE_FLOAT, CC_AUTOADD );
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colIrrigation_yr  ,      _T("Irrig_yr"),     TYPE_FLOAT, CC_AUTOADD );

   EnvExtension::CheckCol(m_pCatchmentLayer, m_colIrrigation, "IRRIGATION", TYPE_INT, CC_AUTOADD); 

   EnvExtension::CheckCol(m_pCatchmentLayer, m_colHRUMeanLAI, "HRU_LAI", TYPE_FLOAT, CC_AUTOADD); 

   // <streams>
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamOBJECTID, _T("OBJECTID"), TYPE_INT, CC_MUST_EXIST);
   EnvExtension::CheckCol( m_pStreamLayer,    m_colStreamFrom,         _T("FNODE_"),   TYPE_INT,   CC_MUST_EXIST );  // required for building topology
   EnvExtension::CheckCol( m_pStreamLayer,    m_colStreamTo,           _T("TNODE_"),   TYPE_INT,   CC_MUST_EXIST );
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamDOBJECTID, _T("DOBJECTID"), TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamLOBJECTID, _T("LOBJECTID"), TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamROBJECTID, _T("ROBJECTID"), TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachSTRM_ORDER, _T("STRM_ORDER"), TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachRIVER_KM, _T("RIVER_KM"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachWIDTH_MIN, _T("WIDTH_MIN"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachDEPTH_MIN, _T("DEPTH_MIN"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachZ_MAX, _T("Z_MAX"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachZ_MIN, _T("Z_MIN"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamCOMID, _T("COMID"), TYPE_INT, CC_MUST_EXIST);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamCOMID_DOWN, _T("COMID_DOWN"), TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamCOMID_LEFT, _T("COMID_LEFT"), TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamCOMID_RT, _T("COMID_RT"), TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachREACH_NDX, _T("REACH_NDX"), TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachREACH_H2O, _T("REACH_H2O"), TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamRES_ID, _T("RES_ID"), TYPE_INT, CC_MUST_EXIST);

   EnvExtension::CheckCol(m_pStreamLayer, m_colReachRESAREA_HA, _T("RESAREA_HA"), TYPE_DOUBLE, CC_AUTOADD);
   m_pStreamLayer->SetColDataU(m_colReachRESAREA_HA, 0);

   EnvExtension::CheckCol(m_pStreamLayer, m_colReachRES_EVAPMM, _T("RES_EVAPMM"), TYPE_DOUBLE, CC_AUTOADD);
   m_pStreamLayer->SetColDataU(m_colReachRES_EVAPMM, 0);

   EnvExtension::CheckCol(m_pStreamLayer, m_colReachRES_H2O, _T("RES_H2O"), TYPE_DOUBLE, CC_AUTOADD);
   m_pStreamLayer->SetColDataU(m_colReachRES_H2O, 0);

   EnvExtension::CheckCol(m_pStreamLayer, m_colReachRES_LW_OUT, _T("RES_LW_OUT"), TYPE_DOUBLE, CC_AUTOADD);
   m_pStreamLayer->SetColDataU(m_colReachRES_LW_OUT, 0);

   EnvExtension::CheckCol(m_pStreamLayer, m_colReachRES_SW_IN, _T("RES_SW_IN"), TYPE_DOUBLE, CC_AUTOADD);
   m_pStreamLayer->SetColDataU(m_colReachRES_SW_IN, 0);

   EnvExtension::CheckCol(m_pStreamLayer, m_colReachRES_TEMP, _T("RES_TEMP"), TYPE_DOUBLE, CC_AUTOADD);
   m_pStreamLayer->SetColDataU(m_colReachRES_TEMP, 0);

   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamRESOUTFALL, _T("RESOUTFALL"), TYPE_INT, CC_AUTOADD);
   m_pStreamLayer->SetColDataU(m_colStreamRESOUTFALL, 0);

   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamSTRMVERT0X, _T("STRMVERT0X"), TYPE_INT, CC_MUST_EXIST);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamSTRMVERT0Y, _T("STRMVERT0Y"), TYPE_INT, CC_MUST_EXIST);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamQ_DIV_WRQ, _T("Q_DIV_WRQ"), TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamINSTRM_REQ, _T("INSTRM_REQ"), TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamREACH_H2O, _T("REACH_H2O"), TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachADDED_VOL, _T("ADDED_VOL"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachADDEDVOL_C, _T("ADDEDVOL_C"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachADDED_Q, _T("ADDED_Q"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachEVAP_MM, _T("EVAP_MM"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamHYDRO_MW, _T("HYDRO_MW"), TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamJoin, m_streamJoinCol, TYPE_INT, CC_MUST_EXIST);
   EnvExtension::CheckCol(m_pStreamLayer, m_colStreamTMAX_H2O_Y, _T("TMAX_H2O_Y"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachTEMP_H2O, _T("TEMP_H2O"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachZ_MEAN, _T("Z_MEAN"), TYPE_DOUBLE, CC_MUST_EXIST);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachRAD_LW_OUT, _T("RAD_LW_OUT"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachRAD_SW_IN, _T("RAD_SW_IN"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachKCAL_REACH, _T("KCAL_REACH"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachAREA_H2O, _T("AREA_H2O"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachWIDTH_CALC, _T("WIDTH_CALC"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachDEPTH, _T("DEPTH"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachDEPTHMANNG, _T("DEPTHMANNG"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachTURNOVER, _T("TURNOVER"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachXFLUX_D, _T("XFLUX_D"), TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachSPRING_CMS, _T("SPRING_CMS"), TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachSPRINGTEMP, _T("SPRINGTEMP"), TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachIN_RUNOFF, _T("IN_RUNOFF"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachIN_RUNOF_C, _T("IN_RUNOF_C"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachOUT_LATERL, _T("OUT_LATERL"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachFLOOD_CMS, _T("FLOOD_CMS"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachFLOOD_C, _T("FLOOD_C"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachQ_UPSTREAM, _T("Q_UPSTREAM"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachQ_MIN, _T("Q_MIN"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachHBVCALIB, _T("HBVCALIB"), TYPE_INT, CC_MUST_EXIST);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachXFLUX_Y, _T("XFLUX_Y"), TYPE_FLOAT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachDIRECTION, _T("DIRECTION"), TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachSLength, _T("SLength"), TYPE_DOUBLE, CC_MUST_EXIST);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachSHADECOEFF, _T("SHADECOEFF"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachBANK_L_IDU, _T("BANK_L_IDU"), TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachBANK_R_IDU, _T("BANK_R_IDU"), TYPE_INT, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachRAD_SW_EST, _T("RAD_SW_EST"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachSHADE_TOPO, _T("SHADE_TOPO"), TYPE_DOUBLE, CC_AUTOADD);
//x   EnvExtension::CheckCol(m_pStreamLayer, m_colReachVEG_DENS, _T("VEG_DENS"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachVGDNS_CALC, _T("VGDNS_CALC"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachVGDNSGIVEN, _T("VGDNSGIVEN"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachVGDNSREACH, _T("VGDNSREACH"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachSHADE_VEG, _T("SHADE_VEG"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachLAI_REACH, _T("LAI_REACH"), TYPE_DOUBLE, CC_AUTOADD);
   EnvExtension::CheckCol(m_pStreamLayer, m_colReachVEGHTREACH, _T("VEGHTREACH"), TYPE_DOUBLE, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachVEGHTGIVEN, "VEGHTGIVEN", TYPE_DOUBLE, CC_AUTOADD);

   m_pReachLayer->CheckCol(m_colReachWIDTHREACH, "WIDTHREACH", TYPE_DOUBLE, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachWIDTHGIVEN, "WIDTHGIVEN", TYPE_DOUBLE, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachRADSWGIVEN, "RADSWGIVEN", TYPE_DOUBLE, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachSAL_REACH, "SAL_REACH", TYPE_INT, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachKCAL_GIVEN, "KCAL_GIVEN", TYPE_DOUBLE, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachTOPOELEV_E, "TOPOELEV_E", TYPE_DOUBLE, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachTOPOELEV_S, "TOPOELEV_S", TYPE_DOUBLE, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachTOPOELEV_W, "TOPOELEV_W", TYPE_DOUBLE, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachVEG_HT_R, "VEG_HT_R", TYPE_DOUBLE, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachVEG_HT_L, "VEG_HT_L", TYPE_DOUBLE, CC_AUTOADD);

   EnvExtension::CheckCol(m_pStreamLayer, m_colReachQ_DISCHARG, _T("Q_DISCHARG"), TYPE_DOUBLE, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachLOG_Q, "LOG_Q", TYPE_FLOAT, CC_AUTOADD);

   m_pReachLayer->CheckCol(m_colReachQ_CAP, "Q_CAP", TYPE_DOUBLE, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachQSPILL_FRC, "QSPILL_FRC", TYPE_DOUBLE, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachQ2WETL, "Q2WETL", TYPE_DOUBLE, CC_AUTOADD);

   EnvExtension::CheckCol( m_pStreamLayer,    m_colStreamCumArea,       _T("CUM_AREA"), TYPE_FLOAT, CC_AUTOADD );
   EnvExtension::CheckCol( m_pCatchmentLayer, m_colCatchmentCumArea,    _T("CUM_AREA"), TYPE_FLOAT, CC_AUTOADD );

   m_pReachLayer->CheckCol(m_colReachHRU_ID, "HRU_ID", TYPE_INT, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachHRU_FRAC, "HRU_FRAC", TYPE_FLOAT, CC_AUTOADD);

   m_pReachLayer->CheckCol(m_colStreamTEMP_AIR, "TEMP_AIR", TYPE_FLOAT, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colStreamTMAX_AIR, "TMAX_AIR", TYPE_FLOAT, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colStreamTMIN_AIR, "TMIN_AIR", TYPE_FLOAT, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colStreamPRECIP, "PRECIP", TYPE_FLOAT, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colStreamSPHUMIDITY, "SPHUMIDITY", TYPE_FLOAT, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colStreamRH, "RH", TYPE_FLOAT, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colStreamRAD_SW, "RAD_SW", TYPE_FLOAT, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colStreamWINDSPEED, "WINDSPEED", TYPE_FLOAT, CC_AUTOADD);

   m_colStreamIDU_ID = m_pReachLayer->GetFieldCol("IDU_ID");
   if (m_colStreamIDU_ID < 0)
   {
      m_pReachLayer->CheckCol(m_colStreamIDU_ID, "IDU_ID", TYPE_INT, CC_AUTOADD);
      m_pReachLayer->SetColDataU(m_colStreamIDU_ID, -1); // Will be populated in InitReaches().
   }

   m_pStreamQE    = new QueryEngine( m_pStreamLayer );
   m_pCatchmentQE = new QueryEngine( m_pCatchmentLayer );

   m_pCatchmentLayer->SetColData( m_colTEMP,             0.0f, true );
   m_pCatchmentLayer->SetColData( m_colHruTempYr,        0.0f, true );
   m_pCatchmentLayer->SetColData( m_colHruTemp10Yr,      0.0f, true );

   m_pCatchmentLayer->SetColData( m_colPRECIP,           0.0f, true );
   m_pCatchmentLayer->SetColDataU( m_colPRECIP_YR,      0.0f);
   m_pCatchmentLayer->SetColData( m_colHruPrecip10Yr,    0.0f, true );

   m_pCatchmentLayer->SetColData( m_colMAXSNOW,        0.0f, true );
   m_pCatchmentLayer->SetColData( m_colHruApr1SWE,       0.0f, true );
   m_pCatchmentLayer->SetColData(m_colHruApr1SWE10Yr, 0.0f, true);

   Report::LogMsg( "Flow: Initializing reaches/catchments/reservoirs/control points" );

   // build basic structures for streams, catchments, HRU's
   InitReaches();      // create, initialize reaches based on stream layer
   { // Check whether reach indices are consistent.
      int reachArray_len = (int)m_reachArray.GetSize();
      int reachLayer_len = m_pReachLayer->GetRowCount();  
      ASSERT(reachArray_len == reachLayer_len);
      for (int poly_ndx = 0; poly_ndx < reachLayer_len; poly_ndx++)
      {
         int poly_comid = 0; m_pReachLayer->GetData(poly_ndx, m_colStreamCOMID, poly_comid);
         int reach_ndx = -1; m_pReachLayer->GetData(poly_ndx, m_colReachREACH_NDX, reach_ndx);
         int array_comid = m_reachArray[reach_ndx]->m_reachID;
         ASSERT(poly_comid == array_comid);
      }
      for (int array_ndx = 0; array_ndx < reachArray_len; array_ndx++)
      {
         Reach* pReach = m_reachArray[array_ndx];
         int poly_ndx = pReach->m_polyIndex;
         int reach_ndx_from_layer = 0; m_pReachLayer->GetData(poly_ndx, m_colReachREACH_NDX, reach_ndx_from_layer);
         ASSERT(array_ndx == reach_ndx_from_layer);
         int reach_ndx_from_Reach_object = pReach->m_reachArrayNdx;
         ASSERT(array_ndx == reach_ndx_from_Reach_object);
      }
   }


   InitCatchments();   // create, initialize catchments based on catchment layer
   InitReservoirs();   // initialize reservoirs

   // connect catchments to reaches
   Report::LogMsg( "Flow: Building topology" );
   if (m_flowContext.pEnvContext->coldStartFlag) ConnectCatchmentsToReaches();
   AssignReachesToHRUs();
   AssignIDUsToStreamBanks();
   
   IdentifyMeasuredReaches();

   bool ok = true; //InitializeReachSampleArray();   // allocates m_pReachDischargeData object
/*
   if ( this->m_pStreamQuery != NULL || ( this->m_minCatchmentArea > 0 && this->m_maxCatchmentArea > 0 ) )
      { 
      EnvExtension::CheckCol( m_pCatchmentLayer, m_colCatchmentCatchID, m_catchIDCol, TYPE_INT, CC_AUTOADD );   // is this really necessary?
      SimplifyNetwork();      
      }
*/
   SetAllCatchmentAttributes();
   PopulateCatchmentCumulativeAreas();
   if (!InitHRULayers(pEnvContext)) return(false);

   // iterate through catchments/hrus/hrulayers, setting up the fluxes
   InitFluxes();

   InitWetlands();

   // allocate integrators, state variable ptr arrays
   InitIntegrationBlocks();
   
   if (!RestoreStateVariables(pEnvContext->spinupFlag)) return(false);

   // Init() to any plugins that have an init method
   InitPlugins();
   m_flowContext.m_pFlowProcess = gpFlow;

   GlobalMethodManager::Init( &m_flowContext );
   
   // add output variables:
   m_pTotalFluxData = new FDataObj(7, 0);
   m_pTotalFluxData->SetName( "Global Flux Summary");
   m_pTotalFluxData->SetLabel( 0, "Year" );
   m_pTotalFluxData->SetLabel( 1, "Annual Total ET (acre-ft)" );
   m_pTotalFluxData->SetLabel( 2, "Annual Total Precip( acre-ft)" );
   m_pTotalFluxData->SetLabel( 3, "Annual Total Discharge (acre-ft)" );
   m_pTotalFluxData->SetLabel( 4, "Delta (Precip-(ET+Discharge)) (acre-ft)" );
   m_pTotalFluxData->SetLabel( 5, "Annual Total Rainfall (acre-ft)" );
   m_pTotalFluxData->SetLabel( 6, "Annual Total Snowfall (acre-ft)" );
   
   gpFlow->AddOutputVar( "Global Flux Summary", m_pTotalFluxData, "" );

   for ( int i=0; i < GetFluxInfoCount(); i++ )
      {
      FluxInfo *pFluxInfo = GetFluxInfo( i );
      CString name( "Flux: " );
      name += pFluxInfo->m_name;
      // need to add units
      gpFlow->AddOutputVar( name, pFluxInfo->m_annualFlux, "" );
      }
   
   for ( int i=0; i < (int) m_reservoirArray.GetSize(); i++ )
      {
      Reservoir *pRes = m_reservoirArray[ i ];
      CString name( pRes->m_name );
      name += "-Filled";
      gpFlow->AddOutputVar( name, pRes->m_filled, "" );      
      }

   // set up dataobj for any model outputs
   for ( int j=0; j < (int) m_modelOutputGroupArray.GetSize(); j++ )
      {
      ModelOutputGroup *pGroup = m_modelOutputGroupArray[ j ];

      int moCount = 0;
      for ( int i=0; i < (int) pGroup->GetSize(); i++ )
         {
         if ( pGroup->GetAt( i )->m_inUse )
            moCount++;
         if ( pGroup->GetAt( i )->m_pDataObjObs!=NULL)
            moCount++;
         }

      ASSERT( pGroup->m_pDataObj == NULL );
      if ( moCount > 0 )
         {
         int cols_to_skip = pGroup->m_moInterval == MOI_UNDEFINED ? 1 : 4;
         pGroup->m_pDataObj = new FDataObj( moCount + cols_to_skip, 0 );
         pGroup->m_pDataObj->SetName( pGroup->m_name );
         switch (pGroup->m_moInterval)
            {
            case MOI_DAILY: 
               pGroup->m_pDataObj->SetLabel(0, "Day index"); 
               pGroup->m_pDataObj->SetLabel(1, "year");
               pGroup->m_pDataObj->SetLabel(2, "month");
               pGroup->m_pDataObj->SetLabel(3, "day");
               break;
            case MOI_MONTHLY: 
               pGroup->m_pDataObj->SetLabel(0, "Month index"); 
               pGroup->m_pDataObj->SetLabel(1, "year");
               pGroup->m_pDataObj->SetLabel(2, "month");
               pGroup->m_pDataObj->SetLabel(3, "days in month");
               break;
            case MOI_YEARLY:
               pGroup->m_pDataObj->SetLabel(0, "Year index");
               pGroup->m_pDataObj->SetLabel(1, "year");
               pGroup->m_pDataObj->SetLabel(2, "months in year");
               pGroup->m_pDataObj->SetLabel(3, "days in year");
               break;
            case MOI_WATERYEAR_DAILY:
               pGroup->m_pDataObj->SetLabel(0, "Day index");
               pGroup->m_pDataObj->SetLabel(1, "water year");
               pGroup->m_pDataObj->SetLabel(2, "month of water year");
               pGroup->m_pDataObj->SetLabel(3, "day");
               break;
            case MOI_WATERYEAR_MONTHLY:
               pGroup->m_pDataObj->SetLabel(0, "Month index");
               pGroup->m_pDataObj->SetLabel(1, "water year");
               pGroup->m_pDataObj->SetLabel(2, "month of water year");
               pGroup->m_pDataObj->SetLabel(3, "days in month");
               break;
            case MOI_WATERYEAR_YEARLY:
               pGroup->m_pDataObj->SetLabel(0, "Year index");
               pGroup->m_pDataObj->SetLabel(1, "water year");
               pGroup->m_pDataObj->SetLabel(2, "months in year");
               pGroup->m_pDataObj->SetLabel(3, "days in year");
               break;
            case MOI_END_OF_YEAR:
               pGroup->m_pDataObj->SetLabel(0, "Year index");
               pGroup->m_pDataObj->SetLabel(1, "year");
               pGroup->m_pDataObj->SetLabel(2, "month");
               pGroup->m_pDataObj->SetLabel(3, "day");
               break;
            case MOI_UNDEFINED:
            default: 
               pGroup->m_pDataObj->SetLabel(0, "time"); 
               break;
            }

         int col = cols_to_skip;
         for ( int i=0; i < (int) pGroup->GetSize(); i++ )
            {
            if ( pGroup->GetAt( i )->m_inUse ) { pGroup->m_pDataObj->SetLabel( col, pGroup->GetAt( i )->m_name ); col++; }
            
            if (pGroup->GetAt( i )->m_pDataObjObs !=NULL)
               {
               CString name;
               name.Format("Obs:%s", (LPCTSTR)pGroup->GetAt(i)->m_nameObs);
               pGroup->m_pDataObj->SetLabel(col, name );     
               col++;
               }
            }
         gpFlow->AddOutputVar( pGroup->m_name, pGroup->m_pDataObj, pGroup->m_name );
         }
      }

   // Flow Water Balance Report
   this->m_FlowWaterBalanceReport.SetName("FLOW Water Balance (mm H2O)");
   this->m_FlowWaterBalanceReport.SetSize(10, 0);
   this->m_FlowWaterBalanceReport.SetLabel(0, "Year");
   this->m_FlowWaterBalanceReport.SetLabel(1, "StartYear total (mm H2O)"); 
   this->m_FlowWaterBalanceReport.SetLabel(2, "StartYear reaches (mm H2O)"); 
   this->m_FlowWaterBalanceReport.SetLabel(3, "StartYear reservoirs (mm H2O)"); 
   this->m_FlowWaterBalanceReport.SetLabel(4, "StartYear HRUs (mm H2O)");
   this->m_FlowWaterBalanceReport.SetLabel(5, "EndYear total (mm H2O)"); 
   this->m_FlowWaterBalanceReport.SetLabel(6, "EndYear reaches (mm H2O)");
   this->m_FlowWaterBalanceReport.SetLabel(7, "EndYear reservoirs (mm H2O)"); // 
   this->m_FlowWaterBalanceReport.SetLabel(8, "EndYear HRUs (mm H2O)"); // ET_YR
   this->m_FlowWaterBalanceReport.SetLabel(9, "EndYear minus StartYear (mm H2O)"); // SNOWEVAP_Y
   gpFlow->AddOutputVar("FLOW Water Balance (mm H2O)", &m_FlowWaterBalanceReport, "FLOW Water Balance (mm H2O)");

   m_ResActiveRuleReport.SetName("FLOW Daily Reservoir Active Rule Report");
   m_ResActiveRuleReport.SetSize(1 + (int)m_reservoirArray.GetSize(), 0);
   m_ResActiveRuleReport.SetLabel(0, "Day");
   for (int i = 0; i < (int)m_reservoirArray.GetSize(); i++)
      {
      Reservoir *pRes = m_reservoirArray[i];
      CString name(pRes->m_name);
      m_ResActiveRuleReport.SetLabel(i + 1, name);
      }
   gpFlow->AddOutputVar("FLOW Daily Reservoir Active Rule", &m_ResActiveRuleReport, "FLOW Daily Reservoir Active Rule");


   // call any initialization methods for fluxes
 
   int fluxInfoCount = (int) m_fluxInfoArray.GetSize();
   for ( int i=0; i < fluxInfoCount; i++ )
      {
      FluxInfo *pFluxInfo = m_fluxInfoArray[ i ];
      if ( pFluxInfo->m_pInitFn != NULL )
         {
         m_flowContext.pFluxInfo = pFluxInfo;
         pFluxInfo->m_pInitFn( &m_flowContext, pFluxInfo->m_initInfo );
         }
      }

   float areaTotal=0.0f;
   int numHRU=0; 
   int numPoly=0;
   for ( int i=0; i < m_catchmentArray.GetSize(); i++ )
      {
      Catchment *pCatchment = m_catchmentArray[i];
      areaTotal += pCatchment->m_area;
      for (int j=0; j < pCatchment->GetHRUCountInCatchment(); j++)
         {
         HRU *pHRU = pCatchment->GetHRUfromCatchment( j );
         for (int k=0;k < pHRU->m_polyIndexArray.GetSize(); k++)
            numPoly++;
         numHRU++;
         }
       }
   m_totArea = areaTotal;
   CString msg;
   msg.Format( "Flow: Total Area %8.1f km2.  Average catchment area %8.1f km2, Average HRU area %8.1f km2, number of polygons %i", 
       areaTotal/10000.0f/100.0f, areaTotal/m_catchmentArray.GetSize()/10000.0f/100.0f, areaTotal/numHRU/10000.0f/100.0f, numPoly );
   Report::LogMsg( msg );

   int numParam = (int) m_parameterArray.GetSize();
   if (m_estimateParameters)
      msg.Format( "Flow: Parameter Estimation run. %i parameters, %i runs, %i years. Scenario settings may override. Verify by checking Scenario variables ",numParam , m_numberOfRuns, m_numberOfYears );
   else
      msg.Format( "Flow: Single Run.  %i years. Scenario settings may override. Verify by checking Scenario variables ",  m_numberOfYears );
   Report::LogMsg( msg, RT_INFO );

   msg.Format( "Flow: Processors available=%i, Max Processors specified=%i, Processors used=%i", ::omp_get_num_procs(), gpFlow->m_maxProcessors, gpFlow->m_processorsUsed );
   Report::LogMsg( msg, RT_INFO );

/*   This code was used to seed some values in the Reach layer /////////////////////////////////////////////

   CString input_file_name = "C:\\CW3M.git\\trunk\\DataCW3M\\McKenzie\\Shade_a_latorData\\cc_input_Data.csv";
   CString input_path;
   bool input_path_ok = !input_file_name.IsEmpty();
   if (!input_path_ok) Report::LogMsg("FlowModel::InitRun() m_shadeAlatorData.m_input_file_name is empty.");
   if (input_path_ok && PathManager::FindPath(input_file_name, input_path) < 0)
   {
      CString msg;
      msg.Format("FlowModel::InitRun() Can't find Shade-a-lator input file %s", input_file_name.GetString());
      Report::WarningMsg(msg);
      input_path_ok = false;
   }
   int data_rows = 0;
   if (input_path_ok)
   { // Process the Shade-a-lator input file.
      Shade_a_latorData SALdata;
      SALdata.m_comid_upstream = 23772865;
      SALdata.m_comid_downstream = 23765583;
      Shade_a_latorData* pSAL = &SALdata;
      FDataObj* pData = new FDataObj;
      data_rows = pData->ReadAscii(input_path);

      // How many reaches correspond to the Shade-a-lator data?
      Reach* pReach_upstream = GetReachFromCOMID(SALdata.m_comid_upstream);
      pSAL->m_pReach_upstream = pReach_upstream;
      Reach* pReach_downstream = GetReachFromCOMID(SALdata.m_comid_downstream);
      pSAL->m_pReach_downstream = pReach_downstream;
      Reach* pReach = pReach_upstream;
      int reach_ct = 0;
      while (pReach != NULL)
      {
         reach_ct++;
         m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachSAL_REACH, true); // flag this reach as a Shade-a-lator reach
         pReach = (pReach->m_reachID == pSAL->m_comid_downstream) ?
            NULL : GetReachFromNode(pReach->m_pDown);
      }
      SALdata.m_reachCt = reach_ct;
      CString msg;
      msg.Format("FlowModel::InitRun() Shade-a-lator input file = %s, comid_upstream = %d, comid_downstream = %d, data_rows = %d, reach_ct = %d",
         input_path.GetString(), SALdata.m_comid_upstream, SALdata.m_comid_downstream,
         data_rows, reach_ct);
      Report::LogMsg(msg);

      SALdata.m_heightThreshold_m = 0;
      SALdata.m_futureHeight_m = 1000.;
      ProcessShade_a_latorInputData(pSAL, pData, data_rows, reach_ct);
   } // end of if (input_path_ok)

*/      /////////////////////////////////////////
   // Initialize the FIELD_CAP attribute.
   ParamTable* pHBVparams = GetTable("HBV");   
   int col_fc = pHBVparams->GetFieldCol("FC");
   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      int hbvcalib = AttInt(idu, HBVCALIB);
      float field_cap_mm = 0.;
      pHBVparams->Lookup(hbvcalib, col_fc, field_cap_mm);
      SetAtt(idu, FIELD_CAP, field_cap_mm);
   } // end of loop through IDUs

   if (m_flowContext.pEnvContext->coldStartFlag)
   { // Initialize the WETNESS and WETL_VOL attributes.
      m_pIDUlayer->SetColDataU(m_colWETNESS, -1000.);
      m_pIDUlayer->SetColDataU(WETL_VOL, 0);
      for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
      {
         int lulc_a = 0; m_pIDUlayer->GetData(idu, m_colLulcA, lulc_a);
         if (lulc_a != LULCA_WETLAND) continue;
         m_pIDUlayer->SetDataU(idu, m_colWETNESS, 0.); // Initialize as saturated soil without standing water.
      } // end of loop through IDUs
   } // end of if (m_flowContext.pEnvContext->coldStartFlag)

   return(true);
} // end of FlowModel::Init()
 

bool FlowModel::DumpReachInsolationData(Shade_a_latorData* pSAL)
{
   SYSDATE start_date = pSAL->m_startDate;
   SYSDATE current_date = m_flowContext.pEnvContext->m_simDate;

   double river_km = pSAL->m_riverKmAtUpperEnd;
   Reach* pReach = pSAL->m_pReach_upstream;
   river_km -= pReach->m_length / 1000.; // Now river_km is the downstream end of the upstream reach
   int comid = pSAL->m_comid_upstream;

   bool done = pReach == NULL;
   while (!done)
   {
      double unshaded_rad_sw_W_m2; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colStreamRAD_SW, unshaded_rad_sw_W_m2);
      double rad_sw_in_W_m2; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachRAD_SW_IN, rad_sw_in_W_m2);
      double rad_lw_out_W_m2; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachRAD_LW_OUT, rad_lw_out_W_m2);

      int num_subreaches = (int)pReach->m_subnodeArray.GetCount();
      double avg_width_m = 0;
      for (int subreach_ndx = 0; subreach_ndx < num_subreaches; subreach_ndx++)
      {
         ReachSubnode* pSubreach = pReach->GetReachSubnode(subreach_ndx);
         avg_width_m += pSubreach->m_subreach_width_m;
      } // end of loop thru subreaches for calculating avg_width_m
      avg_width_m /= num_subreaches;

      double direction; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachDIRECTION, direction);
      double shadecoeff; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachSHADECOEFF, shadecoeff);

      int bank_l_idu_ndx; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachBANK_L_IDU, bank_l_idu_ndx);
      int bank_l_idu_id; m_pIDUlayer->GetData(bank_l_idu_ndx, m_colIDU_ID, bank_l_idu_id);
      int vegclass_l; m_pIDUlayer->GetData(bank_l_idu_ndx, m_colVEGCLASS, vegclass_l);
      int ageclass_l; m_pIDUlayer->GetData(bank_l_idu_ndx, m_colAGECLASS, ageclass_l);
      double tree_ht_l_m = 0; m_pIDUlayer->GetData(bank_l_idu_ndx, m_colTREE_HT, tree_ht_l_m);

      int bank_r_idu_ndx; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachBANK_R_IDU, bank_r_idu_ndx);
      int bank_r_idu_id; m_pIDUlayer->GetData(bank_r_idu_ndx, m_colIDU_ID, bank_r_idu_id);
      int vegclass_r; m_pIDUlayer->GetData(bank_r_idu_ndx, m_colVEGCLASS, vegclass_r);
      int ageclass_r; m_pIDUlayer->GetData(bank_r_idu_ndx, m_colAGECLASS, ageclass_r);
      double tree_ht_r_m = 0; m_pIDUlayer->GetData(bank_r_idu_ndx, m_colTREE_HT, tree_ht_r_m);

      double veg_ht_l_m = 0; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachVEG_HT_L, veg_ht_l_m);
      double veg_ht_r_m = 0; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachVEG_HT_R, veg_ht_r_m);
      double veg_ht_reach_m = 0; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachVEGHTREACH, veg_ht_reach_m);
      double width_given_m = 0; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachWIDTHGIVEN, width_given_m);
      double rad_sw_given_W_m2 = 0; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachRADSWGIVEN, rad_sw_given_W_m2);
      double kcal_given = 0.; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachKCAL_GIVEN, kcal_given);
      double kcal_calculated = (rad_sw_given_W_m2 * width_given_m * pReach->m_length * SEC_PER_DAY / J_PER_CAL) / 1000.;
      double kcal_diff_pct = (kcal_given != 0.) ? (kcal_calculated - kcal_given) / kcal_given : 0.;
      double rad_sw_est_W_m2 = 0.; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachRAD_SW_EST, rad_sw_est_W_m2);
      double topoelev_e_deg = 0.; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachTOPOELEV_E, topoelev_e_deg);
      double topoelev_s_deg = 0.; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachTOPOELEV_S, topoelev_s_deg);
      double topoelev_w_deg = 0.; m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachTOPOELEV_W, topoelev_w_deg);

      if (insolation_ofile != NULL) fprintf(insolation_ofile, "%d, %d, %d, "
         "%f, %d, %f, %f, %f, %f, %f, %f, %f, "
         "%d, %d, %d, %f, "
         "%d, %d, %d, %f, "
         "%d, %f, %f, %f, "
         "%f, %f, %f, %f, %f, %f,"
         "%f, %f, %f\n",
         current_date.year, current_date.month, current_date.day,
         river_km, comid, pReach->m_length, unshaded_rad_sw_W_m2, rad_sw_in_W_m2, rad_lw_out_W_m2, avg_width_m, direction, shadecoeff,
         bank_l_idu_id, vegclass_l, ageclass_l, tree_ht_l_m,
         bank_r_idu_id, vegclass_r, ageclass_r, tree_ht_r_m,
         num_subreaches, veg_ht_l_m, veg_ht_r_m, veg_ht_reach_m,
         width_given_m, rad_sw_given_W_m2, kcal_given, kcal_calculated, kcal_diff_pct, rad_sw_est_W_m2,
         topoelev_e_deg, topoelev_s_deg, topoelev_w_deg);

      river_km -= pReach->m_length / 1000.;

      if (comid == pSAL->m_comid_downstream) done = true;
      else if (pReach->m_pDown != NULL)
      {
         pReach = GetReachFromNode(pReach->m_pDown);
         if (pReach > 0) comid = pReach->m_reachID;
         else done = true;
      }
      else done = true;

   } // end of while (!done)

   return(true);
} // end of DumpReachInsolationData(pSALdata)

void FlowModel::SummarizeIDULULC()
   {

   for (int i = 0; i < m_catchmentArray.GetSize(); i++)
      {
      Catchment *pCatchment = m_catchmentArray[i];
      for (int j = 0; j < pCatchment->GetHRUCountInCatchment(); j++)
         {
         HRU *pHRU = pCatchment->GetHRUfromCatchment(j);
         float meanLAI = 0.0f;
         float totalArea = 0.0f;
         
         for (int k = 0; k < pHRU->m_polyIndexArray.GetSize(); k++)
            {
            float area = 0.0f; m_flowContext.pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[k], m_colCatchmentArea, area);

            float lai = 0.0f;
            int lulc_a = 0; m_flowContext.pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[k], m_colLulcA, lulc_a);
            if (lulc_a == 4)
               {
               int pvt = 0; m_flowContext.pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[k], m_colPVT, pvt);
               if (pvt>0) m_flowContext.pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[k], m_colLai, lai);
               else lai = 1.f;
               }
            else lai = 1.f;
            meanLAI += lai*area;
            totalArea += area;

            int irrigation = 0; m_flowContext.pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[k], m_colIrrigation, irrigation);
            
            }

         pHRU->m_meanLAI = meanLAI / totalArea;
         if (!m_estimateParameters)
            {
            for (int k = 0; k < pHRU->m_polyIndexArray.GetSize(); k++)
               {
               gpFlow->UpdateIDU(m_flowContext.pEnvContext, pHRU->m_polyIndexArray[k], m_colHRUMeanLAI, pHRU->m_meanLAI, false);
               }
            }
         }
      }
   } // end of SummarizeIDULULC

bool FlowModel::InitRun( EnvContext *pEnvContext )
   {
   m_flowContext.Reset();
   m_flowContext.pFlowModel = this;

   m_flowContext.pEnvContext = pEnvContext;
   m_flowContext.timing   = GMT_INITRUN;

   m_currentEnvisionScenarioIndex = pEnvContext->scenarioIndex;
   m_projectionWKT = pEnvContext->pMapLayer->m_projection; //the Well Known Text for the maplayer projection

   int num_available_climate_scenarios = (int)m_scenarioArray.GetCount();
   if (m_currentFlowScenarioIndex >= num_available_climate_scenarios)
   {
      CString msg; msg.Format("FlowModel::InitRun() m_currentFlowScenarioIndex (=%d) is >= num_available_climate_scenarios (=%d)",
         m_currentFlowScenarioIndex, num_available_climate_scenarios);
      Report::ErrorMsg(msg);
      return(false);
   }
   ClimateScenario *pScenario = m_scenarioArray.GetAt(m_currentFlowScenarioIndex);
   int num_climate_files = (int)pScenario->m_climateInfoArray.GetCount();
   for (int climate_file_ndx = 0; climate_file_ndx < num_climate_files; climate_file_ndx++) 
   {
      ClimateDataInfo* pInfo = pScenario->m_climateInfoArray[climate_file_ndx];
      ASSERT(pInfo->m_type != CDT_UNKNOWN);
      pInfo->m_pDataObj = new GeoSpatialDataObj;
      pInfo->m_pDataObj->InitLibraries();
   } // end of loop to initialize access to the netCDF climate data files
   
   if (pEnvContext->m_maxDaysInYear == -1) pEnvContext->m_maxDaysInYear = pScenario->m_maxDaysInClimateYear;

   m_precipNdx.RemoveAll();
   m_tmeanNdx.RemoveAll();
   m_tminNdx.RemoveAll();
   m_tmaxNdx.RemoveAll();
   m_solarradNdx.RemoveAll();
   m_relhumidityNdx.RemoveAll();
   m_sphumidityNdx.RemoveAll();
   m_windspeedNdx.RemoveAll();

   if (m_currentFlowScenarioIndex < 0 || m_currentFlowScenarioIndex >= m_scenarioArray.GetSize())
   {
      CString msg;
      msg.Format("FlowModel::InitRun() m_currentFlowScenarioIndex = %d does not correspond to a climate scenario id", m_currentFlowScenarioIndex);
      Report::ErrorMsg(msg);
      return(false);
   }


   m_totalWaterInputRate  = 0;
   m_totalWaterOutputRate = 0; 
   m_totalWaterInput      = 0;
   m_totalWaterOutput     = 0; 

   SummarizeIDULULC();

  // ReadState();//read file including initial values for the state variables. This avoids model spin up issues

   InitRunFluxes();
   ResetHRUfluxes();
   ResetReachFluxes();
   InitRunPlugins();

   InitReservoirControlPoints();   //initialize reservoir control points
   m_pReachLayer->SetColDataU(m_colReachRES_EVAPMM, 0);
   m_pReachLayer->SetColDataU(m_colReachRES_H2O, 0);
   m_pReachLayer->SetColDataU(m_colReachRES_LW_OUT, 0);
   m_pReachLayer->SetColDataU(m_colReachRES_SW_IN, 0);
   m_pReachLayer->SetColDataU(m_colReachRES_TEMP, 0);
   m_pReachLayer->SetColDataU(m_colReachRESAREA_HA, 0);
   if (!InitRunReservoirs( pEnvContext )) return(false);

   m_pIDUlayer->SetColDataU(m_colSNOWCANOPY, 0.f);
   m_pIDUlayer->SetColDataU(m_colSM2ATM, 0);
   m_pIDUlayer->SetColDataU(m_colWETL2Q, 0);
   m_pIDUlayer->SetAttZero(FLOODDEPTH);
   m_pReachLayer->SetColDataU(m_colReachIN_RUNOFF, 0);
   m_pReachLayer->SetColDataU(m_colReachIN_RUNOF_C, 0);
   m_pReachLayer->SetColDataU(m_colReachOUT_LATERL, 0);
   m_pReachLayer->SetColDataU(m_colReachFLOOD_CMS, 0);
   m_pReachLayer->SetColDataU(m_colReachFLOOD_C, 0);
   m_pReachLayer->SetColDataU(m_colReachQ_UPSTREAM, 0);
   m_pReachLayer->SetColDataU(m_colReachSPRING_CMS, 0);
   m_pReachLayer->SetColDataU(m_colReachSPRINGTEMP, 0);
   m_pReachLayer->SetColDataU(m_colReachADDED_VOL, 0);
   m_pReachLayer->SetColDataU(m_colReachADDEDVOL_C, 0);
   m_pReachLayer->SetColDataU(m_colReachADDED_Q, 0);

   GlobalMethodManager::InitRun( &m_flowContext );

   m_pTotalFluxData->ClearRows();

   // if model outputs are in use, set up the data object
   for ( int i=0; i < (int) m_modelOutputGroupArray.GetSize(); i++ )
      {
      ModelOutputGroup *pGroup = m_modelOutputGroupArray[ i ];

      if ( pGroup->m_pDataObj != NULL )
         {
         pGroup->m_pDataObj->ClearRows();
         pGroup->m_pDataObj->SetSize( pGroup->m_pDataObj->GetColCount(), 0 );
         }
      }

   if ( m_useRecorder && !m_estimateParameters )
      {
      for ( int i=0; i < (int) m_vrIDArray.GetSize(); i++ )
         ::EnvStartVideoCapture( m_vrIDArray[ i ] );
      }

   OpenDetailedOutputFiles();

   m_FlowWaterBalanceReport.ClearRows();

   if ( m_estimateParameters )
      {
      for (int i = 0; i < m_numberOfYears; i++)
         {
         ClimateScenario *pScenario = m_scenarioArray.GetAt(m_currentFlowScenarioIndex);
         for (int j = 0; j < (int)pScenario->m_climateInfoArray.GetSize(); j++)//different climate parameters
            {
            ClimateDataInfo *pInfo = pScenario->m_climateInfoArray[j];
            GeoSpatialDataObj *pObj = new GeoSpatialDataObj;
            pObj->InitLibraries();
            pInfo->m_pDataObjArray.Add(pObj);
            }
         }

      InitializeParameterEstimationSampleArray();
      int initYear = pEnvContext->currentYear+1;//the value of pEnvContext lags the current year by 1????  see envmodel.cpp ln 3008 and 3050
      for ( int i=0; i < m_numberOfRuns; i++ )
         {  
         UpdateMonteCarloInput(pEnvContext,i);
         pEnvContext->run=i; 
         pEnvContext->currentYear=initYear;
         for (int j=0;j<m_numberOfYears;j++)
            { 
            if (j != 0)
            {
               pEnvContext->currentYear++;
            }
            pEnvContext->yearOfRun=j;
         
            Run( pEnvContext );
            }

         UpdateMonteCarloOutput(pEnvContext,i);

         ResetDataStorageArrays(pEnvContext);
         EndRun(pEnvContext);
         }
      } 

   m_loadClimateRunTime = 0;   
   m_globalFlowsRunTime = 0;   
   m_externalMethodsRunTime = 0;   
   m_gwRunTime = 0;   
   m_hruIntegrationRunTime = 0;   
   m_reservoirIntegrationRunTime = 0;   
   m_reachIntegrationRunTime = 0;   
   m_massBalanceIntegrationRunTime = 0; 
   m_totalFluxFnRunTime = 0;
   m_reachFluxFnRunTime = 0;
   m_hruFluxFnRunTime = 0;
   m_collectDataRunTime = 0;
   m_outputDataRunTime = 0;   
   m_stopTime = 0;

   m_pIDUlayer->SetColDataU(m_colPRCP_Y_AVG, 0.);
   m_pIDUlayer->SetColDataU(m_colAET_Y_AVG, 0.);

   WaterParcel totH2OinReachesWP = CalcTotH2OinReaches();
   WaterParcel totH2OinReservoirsWP = CalcTotH2OinReservoirs();
   CString msg; 
   msg.Format("*** FlowModel::InitRun() CalcTotH2OinReaches() = %f m3, %f degC, %f MJ, CalcTotH2OinReservoirs() = %f m3, %f degC, %f MJ, CalcTotH2OinHRUs = %f m3, CalcTotH2O() = %f m3, m_totArea = %f",
      totH2OinReachesWP.m_volume_m3, totH2OinReachesWP.m_temp_degC, totH2OinReachesWP.ThermalEnergy()/1000.,
      totH2OinReservoirsWP.m_volume_m3, totH2OinReservoirsWP.m_temp_degC, totH2OinReservoirsWP.ThermalEnergy()/1000.,
      CalcTotH2OinHRUs(), CalcTotH2O(), m_totArea);
   Report::LogMsg(msg);

   pEnvContext->m_simDate.month = DEC;
   pEnvContext->m_simDate.day = 31;
   pEnvContext->m_simDate.year = pEnvContext->startYear - 1;

   // If years to run is 0, initialize GRID_INDEX and populate the climate mean values. 
   if (pEnvContext->pEnvModel->m_yearsToRun == 0)
   {
      CString msg;
      if (!InitGridIndex())
      {
         msg.Format("FlowModel::InitRun(): InitGridIndex() failed.  This may be because InitGridIndex() requires \n"
            "the use of a climate dataset with single year data files.\n"
            "InitClimateMeanValues() will be skipped.");
         Report::WarningMsg(msg);
      }
      else if (!InitClimateMeanValues(pEnvContext))
      {
         msg.Format("FlowModel::InitRun(): InitClimateMeanValues() failed.");
         Report::WarningMsg(msg);
      }
   } // end of logic for years to run = 0

   Scenario* pSimulationScenario = m_flowContext.pEnvContext->pEnvModel->GetScenario();
   Shade_a_latorData* pSAL = &(pSimulationScenario->m_shadeAlatorData);
   if (!pSAL->m_valid) m_flowContext.m_SALmode = false;
   else
   { 
      // Initialize DumpReachInsolationData()
      // Get the path to the current user's Documents folder.
      PWSTR userPath;
      SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &userPath);
      char szBuffer[255];
      WideCharToMultiByte(CP_ACP, 0, userPath, -1, szBuffer, sizeof(szBuffer), NULL, NULL);

      // Set the string "filename" equal to Users\<username>\Documents\ReachInsolationData.csv
      CString filename;
      filename.Format("%s\\ReachInsolationData.csv", szBuffer);
      const char* filename_const = filename;

      int errNo = fopen_s(&insolation_ofile, filename_const, "w");
      if (errNo != 0)
      {
         CString msg; 
         msg.Format("FlowModel::InitRun() couldn't open %s. errNo = %d, %s", filename_const, errNo, strerror(errNo));
         Report::ErrorMsg(msg);
         return(false);
      }

      fprintf(insolation_ofile, "YEAR, MONTH, DAY, "
         "RIVER_KM, COMID, LENGTH, RAD_SW, RAD_SW_IN, RAD_LW_OUT, WIDTH, DIRECTION, SHADECOEFF, "
         "BANK_L_IDU, VEGCLASS_L, AGE_CLASS_L, TREE_HT_L, "
         "BANK_R_IDU, VEGCLASS_R, AGE_CLASS_R, TREE_HT_R, "
         "NUM_SUBREACHES, VEG_HT_L, VEG_HT_R, VEGHTREACH, "
         "WIDTHGIVEN, RADSWGIVEN, KCAL_GIVEN, KCAL_CALCULATED, KCAL_DIFF_PCT, RAD_SW_EST, "
         "TOPOELEV_E, TOPOELEV_S, TOPOELEV_W\n");

      // If there are Shade-a-lator files, process them.
      m_pReachLayer->SetColDataU(m_colReachSAL_REACH, false);
      m_pReachLayer->SetColDataU(m_colReachRADSWGIVEN, 0);
      m_pReachLayer->SetColDataU(m_colReachKCAL_GIVEN, 0);
      Scenario * pSimulationScenario = pEnvContext->pEnvModel->GetScenario();

      Shade_a_latorData* pSAL = &(pSimulationScenario->m_shadeAlatorData);
      CString SAL_energy_output_data_file_name = pSAL->m_energy_output_file_name;
      ::ApplySubstituteStrings(SAL_energy_output_data_file_name, pEnvContext->m_substituteStrings);
      CString SAL_energy_output_data_path;
      bool SAL_energy_output_data_path_ok = !SAL_energy_output_data_file_name.IsEmpty();
      if (!SAL_energy_output_data_path_ok) Report::LogMsg("FlowModel::InitRun() m_shadeAlatorData.m_energy_output_file_name is empty.");
      SAL_energy_output_data_path_ok = SAL_energy_output_data_path_ok && (PathManager::FindPath(SAL_energy_output_data_file_name, SAL_energy_output_data_path) >= 0);
      int energy_data_rows = 0;
      pSAL->m_energy_numCols = 0;
      if (SAL_energy_output_data_path_ok)
      {
         energy_data_rows = pSimulationScenario->m_shadeAlatorData.m_SALenergyOutputData.ReadAscii(SAL_energy_output_data_path);
         pSAL->m_energy_numCols = pSAL->m_SALenergyOutputData.GetColCount();
         CString msg;
         msg.Format("FlowModel::InitRun() Shade-a-lator energy output data file = %s, energy_data_rows = %d, energy_num_cols = %d",
            SAL_energy_output_data_path.GetString(), energy_data_rows, pSAL->m_energy_numCols);
         Report::LogMsg(msg);

         CString SAL_radiation_output_data_file_name = pSAL->m_radiation_output_file_name;
         ::ApplySubstituteStrings(SAL_radiation_output_data_file_name, pEnvContext->m_substituteStrings);
         CString SAL_radiation_output_data_path;
         bool SAL_radiation_output_data_path_ok = !SAL_radiation_output_data_file_name.IsEmpty();
         if (!SAL_radiation_output_data_path_ok) Report::LogMsg("FlowModel::InitRun() m_shadeAlatorData.m_radiation_output_file_name is empty.");
         SAL_radiation_output_data_path_ok = SAL_radiation_output_data_path_ok && (PathManager::FindPath(SAL_radiation_output_data_file_name, SAL_radiation_output_data_path) >= 0);
         int radiation_data_rows = 0;
         pSAL->m_radiation_numCols = 0;
         if (SAL_radiation_output_data_path_ok)
         {
            radiation_data_rows = pSimulationScenario->m_shadeAlatorData.m_SALradiationOutputData.ReadAscii(SAL_radiation_output_data_path);
            pSAL->m_radiation_numCols = pSAL->m_SALradiationOutputData.GetColCount();
            CString msg;
            msg.Format("FlowModel::InitRun() Shade-a-lator radiation output data file = %s, radiation_data_rows = %d, radiation_num_cols = %d",
               SAL_radiation_output_data_path.GetString(), radiation_data_rows, pSAL->m_radiation_numCols);
            Report::LogMsg(msg);
         }
         else
         {
            CString msg;
            msg.Format("FlowModel::InitRun() Couldn't open Shade-a-lator radiation output data file = %s", SAL_radiation_output_data_path.GetString());
            Report::ErrorMsg(msg);
         }
         ASSERT(energy_data_rows == radiation_data_rows);
      } // of if (SAL_energy_output_data_path_ok)

      CString input_file_name = pSimulationScenario->m_shadeAlatorData.m_input_file_name;
      ::ApplySubstituteStrings(input_file_name, pEnvContext->m_substituteStrings);
      CString input_path;
      bool input_path_ok = !input_file_name.IsEmpty();
      if (!input_path_ok) Report::LogMsg("FlowModel::InitRun() m_shadeAlatorData.m_input_file_name is empty.");
      if (input_path_ok && PathManager::FindPath(input_file_name, input_path) < 0)
      {
         CString msg;
         msg.Format("FlowModel::InitRun() Can't find Shade-a-lator input file %s", input_file_name.GetString());
         Report::WarningMsg(msg);
         input_path_ok = false;
      }
      int data_rows = 0;
      if (input_path_ok) 
      { // Process the Shade-a-lator input file.
         FDataObj * pData = new FDataObj;
         data_rows = pData->ReadAscii(input_path);

         // How many reaches correspond to the Shade-a-lator data?
         Reach* pReach_upstream = GetReachFromCOMID(pSimulationScenario->m_shadeAlatorData.m_comid_upstream);
         pSAL->m_pReach_upstream = pReach_upstream;
         Reach* pReach_downstream = GetReachFromCOMID(pSimulationScenario->m_shadeAlatorData.m_comid_downstream);
         pSAL->m_pReach_downstream = pReach_downstream;
         Reach* pReach = pReach_upstream;
         int reach_ct = 0;
         while (pReach != NULL)
         {
            reach_ct++;
            m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachSAL_REACH, true); // flag this reach as a Shade-a-lator reach
            pReach = (pReach->m_reachID == pSAL->m_comid_downstream) ? 
               NULL : GetReachFromNode(pReach->m_pDown);
         }
         pSimulationScenario->m_shadeAlatorData.m_reachCt = reach_ct;
         CString msg;
         msg.Format("FlowModel::InitRun() Shade-a-lator input file = %s, comid_upstream = %d, comid_downstream = %d, data_rows = %d, reach_ct = %d", 
            input_path.GetString(), pSimulationScenario->m_shadeAlatorData.m_comid_upstream, pSimulationScenario->m_shadeAlatorData.m_comid_downstream, 
            data_rows, reach_ct);
         Report::LogMsg(msg);

         ProcessShade_a_latorInputData(pSAL, pData, data_rows, reach_ct);
      } // end of logic to process the Shade-a-lator input file
      m_flowContext.m_SALmode = input_path_ok;

   } // end of if (pSAL->m_valid)

   RestoreStateVariables(pEnvContext->spinupFlag);

   return TRUE;
   } // end of InitRun()


bool FlowModel::EndRun( EnvContext *pEnvContext )
   {
   int calendar_year = pEnvContext->currentYear + 1;
   SaveState(calendar_year);

   m_flowContext.Reset();
   m_flowContext.pFlowModel = this;
   m_flowContext.pEnvContext = pEnvContext;
   m_flowContext.timing = GMT_ENDRUN;

   EndRunPlugins();
   GlobalMethodManager::EndRun( &m_flowContext );

   CloseDetailedOutputFiles();

   if ( m_useRecorder && !m_estimateParameters )
      {
      for ( int i=0; i < (int) m_vrIDArray.GetSize(); i++  )
         ::EnvEndVideoCapture( m_vrIDArray[ i ] );   // capture main map
      }

   // output timers
   ReportTimings( "Flow: Climate Data Run Time = %.0f seconds", m_loadClimateRunTime );   
   ReportTimings( "Flow: Global Flows Run Time = %.0f seconds", m_globalFlowsRunTime );   
   ReportTimings( "Flow: External Methods Run Time = %.0f seconds", m_externalMethodsRunTime );   
   ReportTimings( "Flow: Groundwater Run Time = %.0f seconds", m_gwRunTime );   
   ReportTimings( "Flow: HRU Integration Run Time = %.0f seconds (fluxes = %.0f seconds)", m_hruIntegrationRunTime, m_hruFluxFnRunTime );   
   ReportTimings( "Flow: Reservoir Integration Run Time = %.0f seconds", m_reservoirIntegrationRunTime );   
   ReportTimings( "Flow: Reach Integration Run Time = %.0f seconds (fluxes = %0.f seconds)", m_reachIntegrationRunTime, m_reachFluxFnRunTime );   
   ReportTimings( "Flow: Total Flux Function Run Time = %.0f seconds", m_totalFluxFnRunTime );   
   ReportTimings( "Flow: Model Output Collection Run Time = %.0f seconds", m_outputDataRunTime );
   ReportTimings( "Flow: Overall Mass Balance Run Time = %.0f seconds", m_massBalanceIntegrationRunTime ); 

   int controlPointCount = (int)m_controlPointArray.GetSize();
   for (int i = 0; i < controlPointCount; i++)
      {
      ControlPoint *pControl = m_controlPointArray.GetAt(i);
      pControl->m_influencedReservoirsArray.RemoveAll();
      if (pControl->m_pControlPointConstraintsTable != NULL) 
         {
         delete pControl->m_pControlPointConstraintsTable;
         pControl->m_pControlPointConstraintsTable = NULL;
         }
      if (pControl->m_pResAllocation != NULL) 
         {
         delete pControl->m_pResAllocation;
         pControl->m_pResAllocation = NULL;
         }
      } // end of loop thru control points

/* This is needed to avoid a memory leak, but it can't be done here because the day strings haven't been written out to the .csv file yet.
// Free up the memory holding the day strings for the FLOW Daily Reservoir Active Rule report.
   for (int day_ndx = 0; day_ndx < m_ResActiveRuleReport.GetRowCount(); day_ndx++)
      {
      char * day_str = m_DayStrings[day_ndx];
      free(day_str);
      }

   m_ResActiveRuleReport.Clear();
*/

   if (insolation_ofile != NULL) fclose(insolation_ofile); // Close the file used by DumpReachInsolationData().

   return true;
   }

int FlowModel::OpenDetailedOutputFiles()
   {
   int version = 1;
   int numElements = 12;
   int numElementsStream = 1;

   if (m_detailedOutputFlags & 1) // daily IDU
      {
      if (!m_pGrid)
         OpenDetailedOutputFilesIDU(m_iduDailyFilePtrArray);
      else
         OpenDetailedOutputFilesGrid(m_iduDailyFilePtrArray);
      }    

   if ( m_detailedOutputFlags & 2 ) // annual IDU
      OpenDetailedOutputFilesIDU( m_iduAnnualFilePtrArray );

   if ( m_detailedOutputFlags & 4 ) // daily Reach
      OpenDetailedOutputFilesReach( m_reachDailyFilePtrArray );

   if ( m_detailedOutputFlags & 8 ) // annual Reach
      OpenDetailedOutputFilesReach( m_reachAnnualFilePtrArray );

   return 1;
   }


int FlowModel::OpenDetailedOutputFilesIDU( CArray< FILE*, FILE* > &filePtrArray )
   {
   int hruCount = GetHRUCount();
   int hruLayerCount = GetHRULayerCount();
   int reachCount = GetReachCount();
   int reservoirCount = (int) m_reservoirArray.GetSize();
   int polyCount = m_pCatchmentLayer->GetRowCount();
   int modelStateVarCount = hruLayerCount;
   int numYears=m_flowContext.pEnvContext->endYear-m_flowContext.pEnvContext->startYear;
   int version = 1;

   CStringArray names;     // units??
   names.Add( "lai" );                   // 0
   names.Add( "age" );                   // 1
   names.Add( "et_yr" );                 // 2
   names.Add( "MAX_ET_yr" );                // 3
   names.Add( "precip_yr" );             // 4
   names.Add( "irrig_yr" );              // 5
   names.Add( "runoff_yr" );             // 6
   names.Add( "storage_yr" );            // 7
   names.Add( "LULC_B" );                // 8
   names.Add( "LULC_A" );                // 9
   names.Add("SWE_Max");                 // 10

   int numElements = 1;
   // close any open files
   for ( int i=0; i < filePtrArray.GetSize(); i++ )
      {
      if ( filePtrArray[ i ] != NULL )
         fclose( filePtrArray[ i ] );
      }

   // clear out any existing file ptrs
   filePtrArray.RemoveAll();

   int count = 0;
   for (int i = 0; i < names.GetSize(); i++)
      {
      CString outputPath;
      outputPath.Format("%s%s%s", PathManager::GetPath(PM_OUTPUT_DIR), (PCTSTR) names[i], ".flw");
      FILE *fp = NULL;
      PCTSTR file = (PCTSTR)outputPath;
	   errno_t err_no = 0;
      err_no = fopen_s(&fp, file, "wb");
      CString msg; msg.Format("*** OpenDetailedOutputFilesIDU(): i = %d, names[i] = %s, outputPath = %s, errno = %d", i, names[i], outputPath, errno); Report::LogMsg(msg);

      filePtrArray.Add(fp);

      if ( fp != NULL )
         {
         // write headers
         TCHAR buffer[64];
         memset( buffer, 0, 64);
         strncpy_s(buffer, names[i], 63);

         fwrite( &version,       sizeof(int), 1, fp);
         fwrite( buffer,         sizeof(char), 64, fp);
         fwrite( &numYears,      sizeof(int), 1, fp);
         fwrite( &hruCount,      sizeof(int), 1, fp);
         fwrite( &hruLayerCount, sizeof(int), 1, fp);
         fwrite( &polyCount,     sizeof(int), 1, fp);
         fwrite( &reachCount,    sizeof(int), 1, fp);
         fwrite( &numElements,   sizeof(int), 1, fp);

         for (int j = 0; j < m_pCatchmentLayer->GetRowCount(); j++)
            {
            float area = 0.0f;
            m_pCatchmentLayer->GetData(j, m_colCatchmentArea, area);
            fwrite(&area, sizeof(float), 1, fp);
            }

         // an indicator for each polygon that can be used to write data back to shapefile.
         // this assumes that the shapefile is NOT SORTED between when this value is written and when it is used.
         for (int j = 0; j < m_pCatchmentLayer->GetRowCount(); j++)
            {
            float polygonOffset = (float)j;
            fwrite(&polygonOffset, sizeof(float), 1, fp);
            }

         count++;
         }  // end of: if ( fp != NULL )
      }  // end of: for ( i < names.GetSize() )

   return count;
   }

int FlowModel::OpenDetailedOutputFilesGrid(CArray< FILE*, FILE* > &filePtrArray)
   {
   int hruCount = GetHRUCount();
   int hruLayerCount = GetHRULayerCount();
   int reachCount = GetReachCount();
   int reservoirCount = (int)m_reservoirArray.GetSize();
   int polyCount = m_pCatchmentLayer->GetRowCount();
   int modelStateVarCount = hruLayerCount;
   int numYears = m_flowContext.pEnvContext->endYear - m_flowContext.pEnvContext->startYear;
   int version = 1;
   int numRows = m_pGrid->GetRowCount();
   int numCols = m_pGrid->GetColCount();
   REAL width = 0.0f; REAL height = 0.0f;
   m_pGrid->GetCellSize(width,height);
   int res = (int)width;

   CStringArray names;     // units??
   names.Add("L1Depth");                   // 0
   names.Add("L2Depth");
   names.Add("L3Depth");
   names.Add("L4Depth");
   names.Add("L5Depth");
   names.Add("Precip");
   names.Add("AET");
   names.Add("Recharge");
   names.Add("PET");
   names.Add("ELWS");
   names.Add("Runoff");
   names.Add("GWOut");
   names.Add("LossToGW");
   names.Add("GainFromGW");


   int numElements = 1;
   // close any open files
   for (int i = 0; i < filePtrArray.GetSize(); i++)
      {
      if (filePtrArray[i] != NULL)
         fclose(filePtrArray[i]);
      }

   // clear out any existing file ptrs
   filePtrArray.RemoveAll();

   int count = 0;
   for (int i = 0; i < names.GetSize(); i++)
      {
      CString outputPath;
      outputPath.Format("%s%s%s", PathManager::GetPath(PM_OUTPUT_DIR), (PCTSTR)names[i], ".flw");
      FILE *fp = NULL;
      PCTSTR file = (PCTSTR)outputPath;
      fopen_s(&fp, file, "wb");

      filePtrArray.Add(fp);

      if (fp != NULL)
         {
         // write headers
         TCHAR buffer[64];
         memset(buffer, 0, 64);
         strncpy_s(buffer, names[i], 63);

         fwrite(&version, sizeof(int), 1, fp);
         fwrite(buffer, sizeof(char), 64, fp);
         fwrite(&numYears, sizeof(int), 1, fp);
         fwrite(&hruCount, sizeof(int), 1, fp);
         fwrite(&hruLayerCount, sizeof(int), 1, fp);
         fwrite(&polyCount, sizeof(int), 1, fp);
         fwrite(&reachCount, sizeof(int), 1, fp);
         fwrite(&numElements, sizeof(int), 1, fp);
         fwrite(&numRows, sizeof(int), 1, fp);
         fwrite(&numCols, sizeof(int), 1, fp);
         fwrite(&res, sizeof(int), 1, fp);

         for (int i = 0; i < hruCount; i++)
            {
            HRU *pHRU = m_hruArray[i];
            int row = pHRU->m_demRow;
            int col = pHRU->m_demCol;
            fwrite(&row, sizeof(int), 1, fp);
            fwrite(&col, sizeof(int), 1, fp);
            }
         for (int i = 0; i < numCols; i++)
            {
            for (int j = 0; j < numRows; j++)
               {
               float value = 0.0f;
               m_pGrid->GetData(j, i, value);
               fwrite(&value, sizeof(float), 1, fp);
               }
            }
         count++;
         }  // end of: if ( fp != NULL )
      }  // end of: for ( i < names.GetSize() )

   return count;
   }


int FlowModel::OpenDetailedOutputFilesReach( CArray< FILE*, FILE* > &filePtrArray )
   {
   int hruCount = GetHRUCount();
   int hruLayerCount = GetHRULayerCount();
   int reachCount = GetReachCount();
   int reservoirCount = (int) m_reservoirArray.GetSize();
   int polyCount = m_pCatchmentLayer->GetRowCount();
   int modelStateVarCount = hruLayerCount;
   int numYears=m_flowContext.pEnvContext->endYear-m_flowContext.pEnvContext->startYear;
   int version = 2;

   CStringArray names;
   names.Add( "Q" );         // 0

   int numElements = 1;
   // close any open files
   for ( int i=0; i < filePtrArray.GetSize(); i++ )
      {
      if ( filePtrArray[ i ] != NULL )
         fclose( filePtrArray[ i ] );
      }

   // clear out any existing file ptrs
   filePtrArray.RemoveAll();

   int count = 0;
   for (int i = 0; i < names.GetSize(); i++)
      {
      CString outputPath;
      outputPath.Format("%s%s%s", PathManager::GetPath(PM_OUTPUT_DIR), (PCTSTR) names[i], ".flw");
      FILE *fp = NULL;
      PCTSTR file = (PCTSTR)outputPath;
      fopen_s(&fp, file, "wb");

      filePtrArray.Add(fp);

      if ( fp != NULL )
         {
         TCHAR buffer[64];
         memset(buffer, 0, 64);
         strncpy_s(buffer, names[i], 63);

         fwrite( &version,       sizeof(int), 1, fp);
         fwrite( buffer,         sizeof(char), 64, fp);
         fwrite( &numYears,      sizeof(int), 1, fp);
         fwrite( &hruCount,      sizeof(int), 1, fp);
         fwrite( &hruLayerCount, sizeof(int), 1, fp);
         fwrite( &polyCount,     sizeof(int), 1, fp);
         fwrite( &reachCount,    sizeof(int), 1, fp);
         fwrite( &numElements,   sizeof(int), 1, fp);

         for (int j = 0; j < reachCount; j++)
            {
            Reach *pReach = m_reachArray[j];
            if (pReach->m_reachArrayNdx >= 0)
               fwrite(&pReach->m_reachArrayNdx, sizeof(int), 1, fp);
            }

         for (int j = 0; j < reachCount; j++)
            {
            Reach *pReach = m_reachArray[j];
            if (pReach->m_reachArrayNdx >= 0)
               fwrite(&pReach->m_reachID, sizeof(int), 1, fp);
            }

         count++;
         }
      }

   return count;
   }


int FlowModel::SaveDetailedOutputIDU(CArray< FILE*, FILE* > &filePtrArray )
   {
   int hruCount = GetHRUCount();
   int hruLayerCount = GetHRULayerCount();
   int reachCount = GetReachCount();
   int reservoirCount = (int) m_reservoirArray.GetSize();
   int modelStateVarCount = ((hruLayerCount*hruCount) + reachCount)*m_hruSvCount + reservoirCount;
    
   for (int fileNdx = 0; fileNdx <= filePtrArray.GetSize(); fileNdx++) if (filePtrArray[fileNdx] == NULL)
	  {
	  CString msg; msg.Format("*** SaveDetailedOutputIDU() fileNdx = %d, filePtrArray[fileNdx] = NULL", fileNdx); Report::ErrorMsg(msg);
	  return(0);
	  }

   // iterate through IDUs
   for (int i = 0; i < m_pCatchmentLayer->GetRowCount(); i++)
      {
      float lai=0.0f; float age=0.0f; float lulcB=0; float lulcA=0;
      float irrig_yr=0.0f;
      float et_yr = 0.0f; float MAX_ET_yr=0.0f; float precip_yr = 0.0f; float runoff_yr = 0.0f; float storage_yr = 0.0f;
      float max_snow = 0.0f;
      m_flowContext.pEnvContext->pMapLayer->GetData(i,m_colLai,lai);
      m_flowContext.pEnvContext->pMapLayer->GetData(i,m_colAgeClass,age);
      m_pIDUlayer->GetData(i, m_colET_YR, et_yr); 
      m_flowContext.pEnvContext->pMapLayer->GetData(i, m_colMaxET_yr, MAX_ET_yr);
      m_pIDUlayer->GetData(i, m_colPRECIP_YR, precip_yr);
      m_flowContext.pEnvContext->pMapLayer->GetData(i,m_colIrrigation_yr,irrig_yr);
      m_flowContext.pEnvContext->pMapLayer->GetData(i,m_colRunoff_yr,runoff_yr);
      m_flowContext.pEnvContext->pMapLayer->GetData(i,m_colLulcB,lulcB);
      m_flowContext.pEnvContext->pMapLayer->GetData(i,m_colLulcA,lulcA);
      m_flowContext.pEnvContext->pMapLayer->GetData(i, m_colMAXSNOW, max_snow);
      double zero = 0.;

      fwrite( &lai,        sizeof(float), 1, filePtrArray[0] );
      fwrite( &age,        sizeof(float), 1, filePtrArray[1] );
      fwrite( &et_yr,      sizeof(float), 1, filePtrArray[2] );
      fwrite( &MAX_ET_yr,     sizeof(float), 1, filePtrArray[3] );
      fwrite( &precip_yr,  sizeof(float), 1, filePtrArray[4] );
      fwrite( &irrig_yr,   sizeof(float), 1, filePtrArray[5] );
      fwrite( &runoff_yr,  sizeof(float), 1, filePtrArray[6] );
      fwrite( &zero, sizeof(float), 1, filePtrArray[7] ); // was storage_yr, now spare
      fwrite( &lulcB,      sizeof(float), 1, filePtrArray[8] );
      fwrite( &lulcA,      sizeof(float), 1, filePtrArray[9] );
      fwrite(&max_snow,    sizeof(float), 1, filePtrArray[11]);
      }

   return 1;
   }



int FlowModel::SaveDetailedOutputReach(CArray< FILE*, FILE* > &filePtrArray )
   {
   ASSERT( filePtrArray.GetSize() == 1 );

   int reachCount = GetReachCount();

   for (int i = 0; i < reachCount; i++)
      {
      Reach *pReach = m_reachArray[i];
      ReachSubnode *pNode = pReach->GetReachSubnode(0);

      if ( filePtrArray[ 0 ] != NULL )       
         fwrite(&pNode->m_discharge, sizeof(float), 1, filePtrArray[0]);
      }

   return 1;
   }


void FlowModel::CloseDetailedOutputFiles()
   {
   for (int i=0; i < (int) m_iduAnnualFilePtrArray.GetSize(); i++)
      {
      if (m_iduAnnualFilePtrArray[i] != NULL )
         {
         fclose(m_iduAnnualFilePtrArray[i]);
         m_iduAnnualFilePtrArray[ i ] = NULL;
         }
      }

   for (int i=0; i < (int) m_iduDailyFilePtrArray.GetSize(); i++)
      {
      if (m_iduDailyFilePtrArray[i] != NULL )
         {
         fclose(m_iduDailyFilePtrArray[i]);
         m_iduDailyFilePtrArray[ i ] = NULL;
         }
      }
         

   for (int i=0; i < (int) m_reachAnnualFilePtrArray.GetSize(); i++)
      {
      if (m_reachAnnualFilePtrArray[i] != NULL )
         {
         fclose(m_reachAnnualFilePtrArray[i]);
         m_reachAnnualFilePtrArray[ i ] = NULL;
         }
      }

   for (int i=0; i < (int) m_reachDailyFilePtrArray.GetSize(); i++)
      {
      if (m_reachDailyFilePtrArray[i] != NULL )
         {
         fclose(m_reachDailyFilePtrArray[i]);
         m_reachDailyFilePtrArray[ i ] = NULL;
         }
      }
   }


/*
flow.ic file with water temperatures (WaterParcel objects)
a binary file
begins with one integer, modelStateVarCount, which is the number of doubles remaining to be written out.
12/8/20 with WaterParcel objects - with each water volume there is also a water temperature
For each layer of each HRU, the volume and water temperature are written out, a total of 2*hruCount*hruLayerCount doubles.
For each subnode of each reach, the discharge, volume, and water temperature are written out as doubles.
Finally, the volume and water temperature of each reservoir are written out, as doubles (2*reservoirCount doubles)
*/
int FlowModel::SaveState(int calendar_year)
   {
   m_ICincludesWP = true;
   int hruCount = GetHRUCount();
   int hruLayerCount = GetHRULayerCount();
   int reachCount = GetReachCount();
   int reservoirCount = (int) m_reservoirArray.GetSize();

   int reachSVcount = 0;
   for (int i = 0; i < reachCount; i++)
   {
      Reach *pReach = m_reachArray[i];
      reachSVcount += 3 * (int)pReach->m_subnodeArray.GetSize(); // this counts discharge, volume, and water temperature for each subnode
   }

   int modelStateVarCount = 2 * hruCount*hruLayerCount + reachSVcount + 2 * reservoirCount;

   double totHRUvol, totReachVol, totReservoirVol;
   totHRUvol = totReachVol = totReservoirVol = 0.;

   FILE *fp;
   CString filename;

   // Get the path to the current user's Documents folder.
   PWSTR userPath;
   SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &userPath);
   char szBuffer[255];
   WideCharToMultiByte(CP_ACP, 0, userPath, -1, szBuffer, sizeof(szBuffer), NULL, NULL);

   // Set the string "file" equal to Users\<username>\Documents\<m_initConditionsFileName>
   filename.Format("%s\\%s", szBuffer, (LPCTSTR)m_initConditionsFileName);
   CString year_string; 
   year_string.Format("%d", calendar_year);
   if (filename.GetLength()==0) filename = "flow.ic";
   filename = filename + year_string;
   const char *file = filename;

   errno_t err;
   err = fopen_s(&fp, file, "wb"); 
   if (err!=0 || fp == NULL)
      {
      CString msg;
      msg.Format("FlowModel::SaveState() Unable to open file %s.  Initial conditions won't be saved.  ", file);
      Report::LogMsg(msg, RT_WARNING);
      return -1;
      }

   fwrite(&modelStateVarCount, sizeof(int), 1, fp);
   for (int i = 0; i < hruCount; i++)
      {
      HRU *pHRU = m_hruArray[i];
      for (int j = 0; j < hruLayerCount; j++)
         {
         HRULayer *pLayer = pHRU->GetLayer(j);
         if (!(pLayer->m_volumeWater==pLayer->m_volumeWater && pLayer->m_volumeWater >= -10.0f))
         {
            CString msg;
            msg.Format("FlowModel::SaveState() Bad value (%f) in HRU %i layer %i. ", pLayer->m_volumeWater, i, j);
            Report::LogMsg(msg, RT_WARNING);
         }

         double volume_water = pLayer->m_volumeWater;
         if (pHRU->m_frc_naturl < 1. && pHRU->m_frc_naturl > 0.) volume_water /= pHRU->m_frc_naturl;
         else if (pHRU->m_frc_naturl <= 0.) volume_water = 0.;
         fwrite(&volume_water, sizeof(double), 1, fp);
         double h2o_temp_degC = DEFAULT_SOIL_H2O_TEMP_DEGC; // eventually = pHRU->m_h2oWP.WaterTemperature();
         fwrite(&h2o_temp_degC, sizeof(double), 1, fp);

         totHRUvol += pLayer->m_volumeWater;
         } // end of loop on HRU layers
      } // end of loop on HRUs

   for (int i = 0; i < reachCount; i++)
      {
      Reach *pReach = m_reachArray[i];
      
      for (int k = 0; k < pReach->m_subnodeArray.GetSize(); k++)
         {
         ReachSubnode *pNode = pReach->GetReachSubnode(k);
         if (pNode->m_discharge==pNode->m_discharge && pNode->m_discharge >= -10.0f
            && !isnan(pNode->m_waterParcel.m_volume_m3))
            { 
               double discharge_as_double = (double)pNode->m_discharge;
               fwrite(&discharge_as_double, sizeof(double), 1, fp);
               fwrite(&pNode->m_waterParcel.m_volume_m3, sizeof(double), 1, fp);
               fwrite(&pNode->m_waterParcel.m_temp_degC, sizeof(double), 1, fp);
               totReachVol += pNode->m_waterParcel.m_volume_m3;
               if (pNode->m_waterParcel.m_volume_m3 < -10.0f)
                  { 
                  CString msg; 
                  msg.Format("FlowModel::SaveState() m_volume_m3 < -10.f, m_volume_m3 = %f, reach = %d, subnode = %d", pNode->m_waterParcel.m_volume_m3, i, k); 
                  Report::LogMsg(msg);
                  }
         }
         else
            {
            CString msg;
            msg.Format("FlowModel::SaveState() Bad value (%f, %f) in Reach %i, subnode %d .  Initial conditions won't be saved.  ", pNode->m_discharge, pNode->m_waterParcel.m_volume_m3, i, k);
            Report::LogMsg(msg, RT_WARNING);
            fclose(fp);
            return -1;
            }
         } // end of loop on subnode
      } // end of loop on reaches

   for (int i = 0; i < reservoirCount; i++)
      {
      Reservoir *pRes = m_reservoirArray[i];
      fwrite(&pRes->m_resWP.m_volume_m3, sizeof(double), 1, fp);
      fwrite(&pRes->m_resWP.m_temp_degC, sizeof(double), 1, fp);
      totReservoirVol += pRes->m_volume;
      } // end of loop on reservoirs

   fclose(fp);
   CString msg1;
   msg1.Format("FlowModel::SaveState() Initial Conditions file %s successfully written. modelStateVarCount = %d \ntotHRUvol, totReachVol, totReservoirVol = %lf %lf %lf", 
         (LPCTSTR)filename, modelStateVarCount, totHRUvol, totReachVol, totReservoirVol);
   Report::LogMsg(msg1);
   return 1;
} // end of SaveState()


bool FlowModel::IsICfileAvailable()
{
   CString effective_IC_filename;

   bool user_specified_a_file = m_initConditionsFileName.GetLength() > 0;
   if (user_specified_a_file) effective_IC_filename = m_initConditionsFileName;
   else
   { // Set the string "effective_IC_filename" equal to "flow<startYear>.ic"
      CString year_string;
      year_string.Format("%d", m_flowContext.pEnvContext->startYear);
      effective_IC_filename = "flow" + year_string + ".ic";
   }

   // does file exist?
   CString tmpPath = PathManager::GetPath(PM_IDU_DIR); // directory with the idu shapefile
   bool rtnval = PathManager::FindPath(effective_IC_filename, tmpPath) >= 0;

   return(rtnval);
} // end of IsICfileAvailable()


bool FlowModel::InitializeSpinup() // Initialize all the state variables from default values.
{
   // Use the default Reach state variable values which were set in InitReaches().
   // Initialize IDU and HRU attribute values and HRU member values.
   // Turn off irrigation everywhere.

   int hruCount = GetHRUCount();
   int hruLayerCount = GetHRULayerCount();
   for (int i = 0; i < hruCount; i++)
   {
      HRU* pHRU = m_hruArray[i];   
      HRULayer* pBox_nat_soil = pHRU->GetLayer(BOX_NAT_SOIL);
      HRULayer* pBox_irrig_soil = pHRU->GetLayer(BOX_IRRIG_SOIL);
      float field_cap_mm = pHRU->AttFloat(HruFIELD_CAP);

      double wetland_area_accum_m2 = 0.;
      int num_idus = (int)pHRU->m_polyIndexArray.GetSize();
      for (int idu_in_hru = 0; idu_in_hru < num_idus; idu_in_hru++)
      {
         int idu_poly_ndx = pHRU->m_polyIndexArray[idu_in_hru];
         if (AttInt(idu_poly_ndx, LULC_A) == LULCA_WETLAND) wetland_area_accum_m2 += Att(idu_poly_ndx, AREA);

         SetAttInt(idu_poly_ndx, IRRIGATION, 0);
      } // end of loop thru IDUs in this HRU
      pHRU->m_wetlandArea_m2 = wetland_area_accum_m2;
      pHRU->m_snowpackArea_m2 = pHRU->m_HRUtotArea_m2 - pHRU->m_wetlandArea_m2;
      pHRU->m_areaIrrigated = pHRU->m_fracIrrigated = 0;
      pHRU->SetAttFloat(HruIRRIG_SOIL, 0);
      pHRU->SetAttFloat(HruAREA_IRRIG, 0);

      for (int l = 0; l < hruLayerCount; l++)
      {
         HRULayer* pBox = pHRU->GetLayer(l);
         pBox->m_contributionToReach = 0.0f;
         pBox->m_verticalDrainage = 0.0f;
         pBox->m_horizontalExchange = 0.0f;

         if (l <= BOX_MELT)
         { // Aboveground soil water compartments: Layer[0] is Snow and Layer[1] is meltwater in snowpack and standing water in a wetland
            pBox->m_volumeWater = 0.f;
            if (gpModel->m_initWaterContent.GetSize() == hruLayerCount) //water content for each layer was specified
               pBox->m_volumeWater = atof(gpModel->m_initWaterContent[l]) * pHRU->m_HRUeffArea_m2;
            if (l == BOX_SURFACE_H2O && pHRU->m_wetlandArea_m2 > 0.)
            { // Special case for the surface water compartment.  Wetlands initialize at field capacity.
               double wetland_frac = pHRU->m_wetlandArea_m2 / pHRU->m_HRUeffArea_m2;
               double unadjusted_vol_mm = (pBox->m_volumeWater / pHRU->m_HRUeffArea_m2) * 1000.;
               double adjusted_vol_mm = wetland_frac * field_cap_mm + (1. - wetland_frac) * unadjusted_vol_mm;
               pBox->m_volumeWater = (adjusted_vol_mm / 1000.) * pHRU->m_HRUeffArea_m2;
            }
         }
         else if (l == BOX_IRRIG_SOIL) pBox->m_volumeWater = 0.;
         else
         { // Belowground non-irrigated soil water compartments
            if (gpModel->m_initWaterContent.GetSize() == hruLayerCount)//water content for each layer was specified
               pBox->m_volumeWater = atof(gpModel->m_initWaterContent[l]) * 0.4f * pHRU->m_HRUeffArea_m2;//assumes porosity is 0.4...we don't know that value at this point in the code, but 0.4 is probably close.
            else if (gpModel->m_initWaterContent.GetSize() == 1) // a single value for water content was specified 
               pBox->m_volumeWater = atof(gpModel->m_initWaterContent[0]) * 0.4f * pHRU->m_HRUeffArea_m2;
            else pBox->m_volumeWater = 0.2 * (double)pHRU->m_HRUeffArea_m2;
         }
         for (int k = 0; k < m_hruSvCount - 1; k++)
         {
            pBox->m_svArray[k] = 0.0f;//concentration = 1 kg/m3 ???????
            pBox->m_svArrayTrans[k] = 0.0f;//concentration = 1 kg/m3
         } // end of loop thru state variables in this layer
      } // end of loop thru layers in this HRU

      pHRU->SetAttFloat(HruSNOW_BOX, 0.);
      pHRU->SetAtt(HruBOXSURF_M3, 0.);
      pHRU->SetAtt(HruH2OMELT_M3, 0.);
      pHRU->SetAtt(HruH2OSTNDGM3, 0.);

      double nat_soil_h2o_mm = 1000. * pBox_nat_soil->m_volumeWater / pHRU->m_HRUtotArea_m2;
      pHRU->SetAttFloat(HruNAT_SOIL, (float)nat_soil_h2o_mm);

      pHRU->SetAttFloat(HruGW_FASTBOX, (float)(1000. * pHRU->GetLayer(BOX_FAST_GW)->m_volumeWater / pHRU->m_HRUtotArea_m2));
      pHRU->SetAttFloat(HruGW_SLOWBOX, (float)(1000. * pHRU->GetLayer(BOX_SLOW_GW)->m_volumeWater / pHRU->m_HRUtotArea_m2));
      pHRU->m_snowpackFlag = pHRU->m_standingH2Oflag = false;

      for (int idu_HRU_ndx = 0; idu_HRU_ndx < pHRU->m_polyIndexArray.GetSize(); idu_HRU_ndx++)
      {
         int idu_poly_ndx = pHRU->m_polyIndexArray[idu_HRU_ndx];
         SetAttFloat(idu_poly_ndx, FIELD_CAP, field_cap_mm);
         SetAtt(idu_poly_ndx, SNOW_SWE, 0.);
         SetAtt(idu_poly_ndx, H2O_MELT, 0.);

         int lulc_a = AttInt(idu_poly_ndx, LULC_A);
         bool is_wetland = lulc_a == LULCA_WETLAND;
         SetAtt(idu_poly_ndx, WETNESS, is_wetland ? 0. : NON_WETLAND_WETNESS_TOKEN);

         SetAttInt(idu_poly_ndx, IRRIGATION, 0);
         SetAttFloat(idu_poly_ndx, SM_DAY, (float)nat_soil_h2o_mm);
      } // end of loop through the IDUs in this HRU

   } // end of loop through HRUs

   // Use the default Reservoir state variable values which were set in InitReservoirs().

   CString msg;
   msg.Format("FlowModel::InitializeSpinup() set default values for state variables.");
   Report::LogMsg(msg);
   WaterParcel totH2OinReachesWP = CalcTotH2OinReaches();
   WaterParcel totH2OinReservoirsWP = CalcTotH2OinReservoirs();
   msg.Format("*** FlowModel::InitializeSpinup() CalcTotH2OinReaches() = %f m3, %f degC, %f MJ, CalcTotH2OinReservoirs() = %f m3, %f degC, %f MJ, CalcTotH2OinHRUs = %f m3, CalcTotH2O() = %f m3, m_totArea = %f",
      totH2OinReachesWP.m_volume_m3, totH2OinReachesWP.m_temp_degC, totH2OinReachesWP.ThermalEnergy() / 1000.,
      totH2OinReservoirsWP.m_volume_m3, totH2OinReservoirsWP.m_temp_degC, totH2OinReservoirsWP.ThermalEnergy() / 1000.,
      CalcTotH2OinHRUs(), CalcTotH2O(), m_totArea);
   Report::LogMsg(msg);

   return(true);
} // end of InitializeSpinup()


bool FlowModel::ReadState()
// Returns true if able to read and restore the state of the hydrology variables, false otherwise.
// WP suffix refers to WaterParcel objects.
{
  m_ICincludesWP = false;

   CString effective_IC_filename;

   int hruCount = GetHRUCount();
   int hru_countWP = 2 * GetHRUCount();
   int hruLayerCount = GetHRULayerCount();
   int reachCount = GetReachCount();
   int reservoirCount = (int)m_reservoirArray.GetSize();
   int reservoir_countWP = 2 * (int)m_reservoirArray.GetSize();

   int reachSVcount = 0;
   int subreach_countWP = 0;
   for (int i = 0; i < reachCount; i++)
   {
      Reach* pReach = m_reachArray[i];
      int num_subreaches = (int)pReach->m_subnodeArray.GetSize();
      reachSVcount += 2 * num_subreaches; // this counts discharge and volume for each subnode
      subreach_countWP += 3 * num_subreaches;
   }

   int modelStateVarCount = hruCount * hruLayerCount + reachSVcount + reservoirCount;
   int model_state_var_countWP = hru_countWP * hruLayerCount + subreach_countWP + reservoir_countWP;

   double tot_HRU_natural_vol_m3, tot_HRU_other_vol_m3, totReachVol, totReservoirVol;
   tot_HRU_natural_vol_m3 = tot_HRU_other_vol_m3 = totReachVol = totReservoirVol = 0.;

   bool user_specified_a_file = m_initConditionsFileName.GetLength() > 0;
   if (user_specified_a_file) effective_IC_filename = m_initConditionsFileName;
   else
   { // Set the string "effective_IC_filename" equal to "flow<startYear>.ic"
      CString year_string;
      year_string.Format("%d", m_flowContext.pEnvContext->startYear);
      effective_IC_filename = "flow" + year_string + ".ic";
   }

   if (!IsICfileAvailable())
   {
      CString msg;
      msg.Format("ReadState(): Initial Conditions file '%s' cannot be found.", (LPCTSTR)effective_IC_filename);
      Report::WarningMsg(msg);
      return(false);
   }

   CString tmpPath = PathManager::GetPath(PM_IDU_DIR);//directory with the idu
   PathManager::FindPath(effective_IC_filename, tmpPath);
   FILE* fp;
   const char* file = tmpPath;
   errno_t err;
   if ((err = fopen_s(&fp, file, "rb")) != 0)
   {
      CString msg;
      msg.Format("ReadState(): Unable to open file %s.  Initial conditions won't be restored.  ", (LPCTSTR)tmpPath);
      Report::WarningMsg(msg);
      if (!user_specified_a_file)
      {
         CString msg;
         msg.Format("ReadState(): No Flow initial conditions file was specified by the user, and we were unable to open the default file %s.", (LPCTSTR)m_initConditionsFileName);
         Report::LogMsg(msg);
      }
      return(false);
   }

   int fileStateVarCount = 0;
   fread(&fileStateVarCount, sizeof(int), 1, fp);
   if (fileStateVarCount != modelStateVarCount && fileStateVarCount != model_state_var_countWP)
   {
      CString msg;
      msg.Format("The current model is different than that saved in %s. %i versus %i or %i state variables.  %s will be overwritten at the end of this run",
         (LPCTSTR)m_initConditionsFileName, fileStateVarCount, modelStateVarCount, model_state_var_countWP, (LPCTSTR)m_initConditionsFileName);
      Report::LogMsg(msg, RT_WARNING);
      return(false);
   }

   // Read the file to populate initial conditions.
   m_ICincludesWP = fileStateVarCount == model_state_var_countWP;
   float _val = 0;
   double h2o_temp_degC = -1;
   bool water_balance_flag = true;
   for (int i = 0; i < hruCount; i++)
   {
      HRU* pHRU = m_hruArray[i];
      double hru_water_m3 = 0.;

      float area_irrig_soil_m2 = pHRU->AttFloat(HruAREA_IRRIG);
      float area_nat_soil_m2 = pHRU->m_HRUtotArea_m2 - area_irrig_soil_m2;
      for (int j = 0; j < hruLayerCount; j++)
      {
         HRULayer* pLayer = pHRU->GetLayer(j);
         double orig_water_vol_m3 = 0.;
         fread(&orig_water_vol_m3, sizeof(double), 1, fp);
         if (m_ICincludesWP) fread(&h2o_temp_degC, sizeof(double), 1, fp);
         double natural_water_vol_m3 = orig_water_vol_m3 * pHRU->m_frc_naturl;
         double other_water_vol_m3 = orig_water_vol_m3 - natural_water_vol_m3;
         pLayer->m_volumeWater = natural_water_vol_m3;
         switch (j)
         {
            case BOX_SNOWPACK: pHRU->SetAttFloat(HruSNOW_BOX, (float)(1000. * pLayer->m_volumeWater / pHRU->m_HRUtotArea_m2)); break;
            case BOX_SURFACE_H2O: pHRU->SetAttFloat(HruBOXSURF_M3, (float)pLayer->m_volumeWater); break;

            case BOX_NAT_SOIL: 
            {   
               ASSERT(area_nat_soil_m2 > 0 || pLayer->m_volumeWater == 0);
               float nat_soil_mm = area_nat_soil_m2 > 0 ? ((float)(1000. * pLayer->m_volumeWater / area_nat_soil_m2)) : 0;
               pHRU->SetAttFloat(HruNAT_SOIL, nat_soil_mm);
            }
            break;

            case BOX_IRRIG_SOIL: 
            {
               ASSERT(area_irrig_soil_m2 > 0 || pLayer->m_volumeWater == 0);
               float irrig_soil_mm = area_irrig_soil_m2 > 0 ? ((float)(1000. * pLayer->m_volumeWater / area_irrig_soil_m2)) : 0;
               pHRU->SetAttFloat(HruIRRIG_SOIL, irrig_soil_mm);
            }
            break;
            
            case BOX_FAST_GW: pHRU->SetAttFloat(HruGW_FASTBOX, (float)(1000. * pLayer->m_volumeWater / pHRU->m_HRUtotArea_m2)); break;
            case BOX_SLOW_GW: pHRU->SetAttFloat(HruGW_SLOWBOX, (float)(1000. * pLayer->m_volumeWater / pHRU->m_HRUtotArea_m2)); break;
         }
         // pLayer->m_hru_layer_h2oWP = WaterParcel(natural_water_vol_m3, h2o_temp_degC); // eventually
         tot_HRU_natural_vol_m3 += pLayer->m_volumeWater;
         tot_HRU_other_vol_m3 += other_water_vol_m3;
         hru_water_m3 += pLayer->m_volumeWater;
      } // end of loop on HRU layers

      if (!CheckHRUwaterBalance(pHRU))
      {
         FixHRUwaterBalance(pHRU);
         water_balance_flag &= CheckHRUwaterBalance(pHRU);
      }
   } // end of loop on HRUs
   if (!water_balance_flag)
   {
      CString msg;
      msg.Format("ReadState() HRU water balance failed. You may need to do a spinup and create a new initial conditions file.");
      Report::WarningMsg(msg);
   }

   for (int i = 0; i < reachCount; i++)
   {
      Reach* pReach = m_reachArray[i];
      pReach->m_reach_volume_m3 = 0.;
      for (int j = 0; j < pReach->m_subnodeArray.GetSize(); j++)
      {
         ReachSubnode* pSubreach = pReach->GetReachSubnode(j);

         double discharge_as_double; // m_discharge is a float
         fread(&discharge_as_double, sizeof(double), 1, fp);
         if (isnan(discharge_as_double) || discharge_as_double <= 0.)
         {
            CString msg;
            msg.Format("ReadState() i = %d, j = %d, discharge_as_double (%lf) is a nan or <= 0,", i, j, discharge_as_double);
            discharge_as_double = pReach->NominalLowFlow_cms();
            CString msg2;
            msg2.Format(" replacing with %lf", discharge_as_double);
            msg = msg + msg2;
            Report::LogMsg(msg);
         }
         pSubreach->m_discharge = (float)discharge_as_double;
         pSubreach->m_dischargeWP = WaterParcel(SEC_PER_DAY * discharge_as_double, DEFAULT_REACH_H2O_TEMP_DEGC);
         pSubreach->m_dischargeDOY = -1;
         pSubreach->m_manning_depth_m = pReach->GetManningDepthFromQ(pSubreach->m_discharge, pReach->m_wdRatio);

         double subreach_volume;
         fread(&subreach_volume, sizeof(double), 1, fp);
         if (isnan(subreach_volume) || (subreach_volume < pSubreach->m_min_volume_m3 && !close_enough(subreach_volume, pSubreach->m_min_volume_m3, 0.0001)))
         {
            CString msg;
            msg.Format("ReadState() i = %d, j = %d, subreach_volume (%lf) is a nan or < pSubreach->m_min_volume_m3 (= %lf),", i, j, subreach_volume, pSubreach->m_min_volume_m3);
            subreach_volume = pSubreach->m_min_volume_m3;
            CString msg2;
            msg2.Format(" replacing with %lf", subreach_volume);
            msg = msg + msg2;
            Report::WarningMsg(msg);
         }
         pReach->m_reach_volume_m3 += subreach_volume;

         h2o_temp_degC = DEFAULT_REACH_H2O_TEMP_DEGC;
         if (m_ICincludesWP) fread(&h2o_temp_degC, sizeof(double), 1, fp);

         WaterParcel initialWP(subreach_volume, h2o_temp_degC);
         pSubreach->m_waterParcel = initialWP;
         pSubreach->m_previousWP = initialWP;
         pSubreach->m_manning_depth_m = GetManningDepthFromQ(pReach, pSubreach->m_discharge, pReach->m_wdRatio);
         pSubreach->SetSubreachGeometry(subreach_volume, pReach->m_wdRatio);
      } // end of subnode loop
      totReachVol += pReach->m_reach_volume_m3;
   } // end of reach loop

   int numNonzeroReservoirs = 0;
   for (int i = 0; i < reservoirCount; i++)
   {
      Reservoir* pRes = m_reservoirArray[i];
      fread(&pRes->m_volume, sizeof(double), 1, fp);
      h2o_temp_degC = DEFAULT_REACH_H2O_TEMP_DEGC;
      if (m_ICincludesWP) fread(&h2o_temp_degC, sizeof(double), 1, fp);

      pRes->m_resWP = WaterParcel(pRes->m_volume, h2o_temp_degC);
      if (pRes->m_volume > 0.f)
      {
         totReservoirVol += pRes->m_volume;
         numNonzeroReservoirs++;
      }

      Reach* pReach = pRes->m_pReach;
      if (pReach == NULL) continue;
      pReach->SetAtt(ReachRES_H2O, pRes->m_resWP.m_volume_m3);
      pReach->SetAtt(ReachRES_TEMP, pRes->m_resWP.WaterTemperature());

   } // end of loop on reservoirs

   fclose(fp);

   CString msg1;
   msg1.Format("FlowModel::ReadState() Initial Conditions file %s successfully read. modelStateVarCount = %d \ntot_HRU_natural_vol, tot_HRU_other_vol, totReachVol, totReservoirVol = %lf %lf %lf, %lf",
      (LPCTSTR)file, m_ICincludesWP ? model_state_var_countWP : modelStateVarCount, tot_HRU_natural_vol_m3, tot_HRU_other_vol_m3, totReachVol, totReservoirVol);
   Report::LogMsg(msg1);
   msg1.Format("FlowModel::ReadState() modelStateVarCount = %d, hruCount = %d, hruLayerCount = %d, reachCount = %d, reservoirCount = %d, numNonzeroReservoirs = %d",
      m_ICincludesWP ? model_state_var_countWP : modelStateVarCount, hruCount, hruLayerCount, reachCount, reservoirCount, numNonzeroReservoirs);
   Report::LogMsg(msg1);
   if (m_ICincludesWP)
   {
      CString msg;
      msg.Format("FlowModel::ReadState() IC file includes water temperatures.");
      Report::LogMsg(msg);
   }

   CString msg;
   WaterParcel totH2OinReachesWP = CalcTotH2OinReaches();
   WaterParcel totH2OinReservoirsWP = CalcTotH2OinReservoirs();
   msg.Format("*** FlowModel::ReadState() CalcTotH2OinReaches() = %f m3, %f degC, %f MJ, CalcTotH2OinReservoirs() = %f m3, %f degC, %f MJ, CalcTotH2OinHRUs = %f m3, CalcTotH2O() = %f m3, m_totArea = %f",
      totH2OinReachesWP.m_volume_m3, totH2OinReachesWP.m_temp_degC, totH2OinReachesWP.ThermalEnergy() / 1000.,
      totH2OinReservoirsWP.m_volume_m3, totH2OinReservoirsWP.m_temp_degC, totH2OinReservoirsWP.ThermalEnergy() / 1000.,
      CalcTotH2OinHRUs(), CalcTotH2O(), m_totArea);
   Report::LogMsg(msg);

   // Make sure the water volumes in the attributes are consistent with the water volumes in the HRUs.
   int num_hrus = (int)m_hruArray.GetSize();
   for (int hru_ndx = 0; hru_ndx < num_hrus; hru_ndx++)
   {
      HRU* pHRU = m_hruArray[hru_ndx];
      if (CheckHRUwaterBalance(pHRU)) continue;
      FixHRUwaterBalance(pHRU);
      ASSERT(CheckHRUwaterBalance(pHRU));
   } // end of loop thru HRUs

   return(true);
} // end of ReadState()


bool FlowModel::FixHRUwaterBalance(HRU* pHRU)
// Uses IDU LULC_A and AREA attribute values, the water volumes in 4 of the HRU water compartments, and pHRU->m_HRUtotArea_m2 as inputs.
// Smears the surface water uniformly across the HRU - as standing water in wetlands, and melt water elsewhere.
// Partitions the HRU snowpack and surface water among the IDUs which make up the HRU.
// Treats an HRU which consists entirely of wetland as an error.
// Lumps all the topsoil water together and puts it into the non-irrigated topsoil compartment.
// Turns off the IRRIGATION flag for all the IDUs.
// Stores into IDU attributes SNOW_SWE, WETNESS, H2O_MELT, and SM_DAY
// and HRU layer attributes HruSNOW_BOX (mmSWE), HruBOXSURF_M3, HruH2OMELT_M3, HruSTNDGM3, HruNAT_SOIL, and HruIRRIG_SOIL.
// Calculates pHRU->m_standingH2Oflag, pHRU->m_snowpackFlag, pHRU->m_wetlandArea_m2, and pHRU->m_snowpackArea_m2.
// Note that the IDU attribute SNOW_SWE has units of mmSWE and represents both frozen and liquid water in the snowpack,
// i.e. both BOX_SNOW and BOX_SURFACE_H2O. IDU attribute H2O_MELT, in mmH2O, is the liquid portion of SNOW_SWE. So H2O_MELT must
// always be <= SNOW_SWE.
// Note also that HRU attribute HruSNOW_BOX has units of mmSWE and represents just the frozen water in the snowpack,
// expressed as a depth of liquid water over the entire area of the HRU, not just the non-wetland fraction.
// Note that m_snowpackArea_m2 is the area of the non-wetland part of the HRU where there could be frozen snow or melt water.
// By definition, m_snowpackArea_m2 + m_wetlandArea_m2 = the total HRU area, even when there isn't currently any snow or meltwater.
// Theoretically, the frozen snow may melt on IDUs with low LAIs faster in the spring than
// it melts on IDUs with higher LAIs. As of August 2021, the model is not representing that level of spatial detail.
{ 
   // Variable names beginning with "box_" generally represent the volumes of water in the
   // various compartments (i.e. "boxes") of the HBV model.
   // Variable names beginning with "idu_" usually represent values for IDU layer attributes.
   // Variable names beginning with "hru_" usually represent values for HRU layer attributes.

   HRULayer* pHRULayer_snow = pHRU->GetLayer(BOX_SNOW);
   HRULayer* pHRULayer_surface_h2o = pHRU->GetLayer(BOX_SURFACE_H2O);
   HRULayer* pHRULayer_nat_soil = pHRU->GetLayer(BOX_NAT_SOIL);
   HRULayer* pHRULayer_irrig_soil = pHRU->GetLayer(BOX_IRRIG_SOIL);

   // Get the water volumes from the surface and topsoil compartments.
   double box_snow_m3SWE = pHRULayer_snow->m_volumeWater;
   double box_surface_h2o_m3 = pHRULayer_surface_h2o->m_volumeWater;
   double uniform_surface_h2o_depth_mm = (box_surface_h2o_m3 / pHRU->m_HRUtotArea_m2) * 1000.;
   double hru_nat_soil_h2o_m3 = pHRULayer_nat_soil->m_volumeWater;
   double hru_irrig_soil_h2o_m3 = pHRULayer_irrig_soil->m_volumeWater;

   // Lump all the topsoil water together and put it into the non-irrigated topsoil compartment.
   double hru_topsoil_h2o_m3 = hru_nat_soil_h2o_m3 + hru_irrig_soil_h2o_m3;
   pHRULayer_nat_soil->m_volumeWater = hru_topsoil_h2o_m3;
   pHRULayer_irrig_soil->m_volumeWater = 0.;
   // ??? are there any other members of the irrigated soil HRULayer object which need to be reset here?

   // Turn off the IRRIGATION flag for all the IDUs in this HRU, and 
   // partition the HRU area between wetland and non-wetland.
   // Then calculate pHRU->m_wetlandArea_m2, pHRU->m_snowpackArea_m2, wetland_frac,
   // pHRU->m_snowpackFlag, and pHRU->m_standingH2Oflag.
   double idu_area_accum_m2 = 0.;
   double wetland_area_accum_m2 = 0.;
   int num_idus_in_hru = (int)pHRU->m_polyIndexArray.GetSize();
   for (int idu_ndx_in_hru = 0; idu_ndx_in_hru < num_idus_in_hru; idu_ndx_in_hru++)
   {
      int idu_poly_ndx = pHRU->m_polyIndexArray[idu_ndx_in_hru];
      SetAttInt(idu_poly_ndx, IRRIGATION, 0);
      float idu_area_m2 = AttFloat(idu_poly_ndx, AREA);
      idu_area_accum_m2 += idu_area_m2;

      int lulc_a = AttInt(idu_poly_ndx, LULC_A);
      bool is_wetland = lulc_a == LULCA_WETLAND;
      if (is_wetland) wetland_area_accum_m2 += idu_area_m2;
   } // end of first loop thru IDUs
   ASSERT(close_enough(pHRU->m_HRUtotArea_m2, idu_area_accum_m2, 1e-6, 1));
   pHRU->m_wetlandArea_m2 = wetland_area_accum_m2;
   pHRU->m_snowpackArea_m2 = pHRU->m_HRUtotArea_m2 - pHRU->m_wetlandArea_m2;
   ASSERT(pHRU->m_snowpackArea_m2 > 0);
   double wetland_frac = wetland_area_accum_m2 / pHRU->m_HRUtotArea_m2;
   pHRU->m_snowpackFlag = (box_snow_m3SWE > 0) || (box_surface_h2o_m3 > 0);
   pHRU->m_standingH2Oflag = (pHRU->m_wetlandArea_m2 > 0.) && (uniform_surface_h2o_depth_mm > 0.);

   // Check for anomalous situations.
   if ((wetland_area_accum_m2 >= idu_area_accum_m2) || (pHRU->m_snowpackArea_m2 < 0))
   { // The entire HRU is a wetland.
      CString msg;
      msg.Format("FixHRUwaterBalance() HRU %d is composed entirely of wetland IDUs.",
         pHRU->m_id);
      Report::ErrorMsg(msg);
      return(false);
   }
   if (box_surface_h2o_m3 > 0 && (box_snow_m3SWE <= 0 && wetland_area_accum_m2 <= 0))
   {
      CString msg;
      msg.Format("FixHRUwaterBalance() HRU %d has %f m3 water in the surface water compartment, but no snow or wetlands.",
         pHRU->m_id, box_surface_h2o_m3);
      Report::WarningMsg(msg);
   }

   // Calculate HRU layer attribute HruSNOW_BOX (mmSWE) 
   // and IDU attribute SNOW_SWE for IDUs with snow or melt water.
   double hru_SNOW_BOX_mmSWE = 0.;
   double idu_SNOW_SWE_mm_for_idus_with_snow = 0.;
   if (pHRU->m_snowpackFlag)
   { // There is snow or meltwater.
      hru_SNOW_BOX_mmSWE = (box_snow_m3SWE / pHRU->m_HRUtotArea_m2) * 1000.;
      double frozen_snow_mm_for_idus_with_snow = (box_snow_m3SWE / pHRU->m_snowpackArea_m2) * 1000.;
      idu_SNOW_SWE_mm_for_idus_with_snow = frozen_snow_mm_for_idus_with_snow + uniform_surface_h2o_depth_mm;
   }
   else hru_SNOW_BOX_mmSWE = 0.; // There is no snow or melt water.
 
   // Calculate the soil moisture in mm in the topsoil, 
   double hru_topsoil_h2o_mm = (hru_topsoil_h2o_m3 / pHRU->m_HRUtotArea_m2) * 1000.;
 
   // Calculate HRU attributes HruBOXSURF_M3, HruH2OMELT_M3, and HruH2OSTNDGM3.
   double hru_BOXSURF_M3 = box_surface_h2o_m3;
   double hru_H2OMELT_M3 = (1. - wetland_frac) * box_surface_h2o_m3;
   double hru_H2OSTNDGM3 = wetland_frac * box_surface_h2o_m3;
   double hru_standing_h2o_mm = pHRU->m_standingH2Oflag ? uniform_surface_h2o_depth_mm : 0.;

   // Write the HRU attribute values to the HRU layer.
   pHRU->SetAttFloat(HruSNOW_BOX, (float)hru_SNOW_BOX_mmSWE);
   pHRU->SetAtt(HruBOXSURF_M3, hru_BOXSURF_M3);
   pHRU->SetAtt(HruH2OMELT_M3, hru_H2OMELT_M3);
   pHRU->SetAtt(HruH2OSTNDGM3, hru_H2OSTNDGM3);
   pHRU->SetAtt(HruNAT_SOIL, hru_topsoil_h2o_mm);
   pHRU->SetAtt(HruIRRIG_SOIL, 0.); ASSERT(pHRU->m_areaIrrigated == 0);

   // Finally, update the SNOW_SWE, H2O_MELT, WETNESS, FLOODDEPTH, and SM_DAY attributes for the IDUs in the HRU.
   for (int idu_ndx_in_hru = 0; idu_ndx_in_hru < num_idus_in_hru; idu_ndx_in_hru++)
   {
      int idu_poly_ndx = pHRU->m_polyIndexArray[idu_ndx_in_hru];
      double idu_SNOW_SWE_mm = 0.;
      double idu_WETNESS_mm = 0.;
      double idu_FLOODDEPTH_mm = 0;
      double idu_H2O_MELT_mm = 0.;
      double idu_SM_DAY_mm = hru_topsoil_h2o_mm;
      int lulc_a = AttInt(idu_poly_ndx, LULC_A);
      bool is_wetland = lulc_a == LULCA_WETLAND;

      if (is_wetland)
      { // This IDU is a wetland.
         idu_SNOW_SWE_mm = 0.;
         idu_H2O_MELT_mm = 0.;

         if (hru_standing_h2o_mm > 0.)
         { // This IDU is a wetland with standing water.
            // Divide the standing water between WETNESS and FLOODDEPTH.
            double idu_WETL_CAP_mm = Att(idu_poly_ndx, WETL_CAP);
            idu_WETNESS_mm = hru_standing_h2o_mm > idu_WETL_CAP_mm ? idu_WETL_CAP_mm : hru_standing_h2o_mm;
            idu_FLOODDEPTH_mm = hru_standing_h2o_mm > idu_WETNESS_mm ? hru_standing_h2o_mm - idu_WETNESS_mm : 0;
         }
         else
         { // This IDU is a wetland with no standing water.
            // Set its WETNESS attribute to minus the mm of water required to bring its topsoil to field capacity.
            // Field capacity here is a surrogate for water content at saturation.
            float field_capacity_mm = AttFloat(idu_poly_ndx, FIELD_CAP);
            idu_WETNESS_mm = -(field_capacity_mm - hru_topsoil_h2o_mm); if (idu_WETNESS_mm > 0.) idu_WETNESS_mm = 0.;
         }
      }
      else
      { // This IDU is not a wetland.
         idu_SNOW_SWE_mm = pHRU->m_snowpackFlag ? idu_SNOW_SWE_mm_for_idus_with_snow : 0.;
         idu_WETNESS_mm = NON_WETLAND_WETNESS_TOKEN;
         idu_H2O_MELT_mm = uniform_surface_h2o_depth_mm;
      }

      SetAttFloat(idu_poly_ndx, SNOW_SWE, (float)idu_SNOW_SWE_mm);
      SetAtt(idu_poly_ndx, WETNESS, idu_WETNESS_mm);
      SetAtt(idu_poly_ndx, FLOODDEPTH, idu_FLOODDEPTH_mm);
      SetAtt(idu_poly_ndx, H2O_MELT, idu_H2O_MELT_mm);
      SetAtt(idu_poly_ndx, SM_DAY, idu_SM_DAY_mm);
   } // end of second loop thru IDUs

   return(true);
} // end of FixHRUwaterBalance()


void FlowModel::ResetDataStorageArrays(EnvContext *pEnvContext)
   {
   // if model outputs are in use, set up the data object
   for ( int i=0; i < (int) m_modelOutputGroupArray.GetSize(); i++ )
      {
      ModelOutputGroup *pGroup = m_modelOutputGroupArray[ i ];

      if ( pGroup->m_pDataObj != NULL )
         {
         pGroup->m_pDataObj->ClearRows();
       //  pGroup->m_pDataObj->Clear();
         pGroup->m_pDataObj->SetSize( pGroup->m_pDataObj->GetColCount(), 0 );         
         }
      }
   //Insert code here for m_reservoirDataArray;
   }


// FlowModel::Run() runs the model for one year
bool FlowModel::Run( EnvContext *pEnvContext )
   {
   pEnvContext->daysInCurrentYear = pEnvContext->m_maxDaysInYear == 366 ? GetDaysInCalendarYear(pEnvContext->currentYear) : pEnvContext->m_maxDaysInYear;
   pEnvContext->m_doy0ofOct1 = pEnvContext->daysInCurrentYear == 366 ? 274 : 273;
   CString msgStart;
   msgStart.Format("Flow::Run() Starting simulation year %i on Jan 1, %i (using weather for %i). This simulation year has %i days.",
         pEnvContext->yearOfRun, pEnvContext->currentYear, pEnvContext->weatherYear, pEnvContext->daysInCurrentYear);
   Report::LogMsg(msgStart);
   int days_in_weather_year = pEnvContext->m_maxDaysInYear == 366 ? GetDaysInCalendarYear(pEnvContext->weatherYear) : pEnvContext->m_maxDaysInYear;
   if (days_in_weather_year < pEnvContext->daysInCurrentYear)
   {
      CString msg;
      msg.Format("days in weather year %i are less than days in current year %i, days in current year are being set to %i",
         days_in_weather_year, pEnvContext->daysInCurrentYear, days_in_weather_year);
      Report::LogMsg(msg);

      pEnvContext->daysInCurrentYear = days_in_weather_year;
   }

   MSG  msga;
   while (PeekMessage(&msga, NULL, NULL, NULL, PM_REMOVE))
      {
      TranslateMessage(&msga);
      DispatchMessage(&msga);
      }

   m_flowContext.Reset();
   m_flowContext.pFlowModel = this;
   m_flowContext.pEnvContext = pEnvContext;
   m_flowContext.timing = GMT_START_YEAR;

   m_pIDUlayer->SetColDataU(m_colET_DAY, 0);

   CString msg;

   clock_t start = clock();

   clock_t finish = clock();
   double duration = (float)(finish - start) / CLOCKS_PER_SEC;   
   m_loadClimateRunTime += (float) duration;   

   if (!StartYear( &m_flowContext )) return(false);     // calls GLobalMethods::StartYear(), initialize annual variable, accumulators

   // convert envision time to days
   m_yearStartTime = m_stopTime;
   m_timeInRun = m_currentTime = m_yearStartTime;
   m_flowContext.time = (float)m_yearStartTime;
   m_stopTime = m_yearStartTime + pEnvContext->daysInCurrentYear;

   m_flowContext.svCount = m_hruSvCount-1;  //???? 

   float stepSize = m_flowContext.timeStep = (float) m_timeStep;
   
   m_flowContext.Reset();   

   omp_set_num_threads(gpFlow->m_processorsUsed); 

   m_volumeMaxSWE = 0.0f;
   m_dateMaxSWE= 0;

   //-------------------------------------------------------
   // Main within-year FLOW simulation loop starts here
   //-------------------------------------------------------
   while ( ( m_currentTime + TIME_TOLERANCE ) < m_stopTime )
      {
      int dayOfYear = int(m_timeInRun - m_yearStartTime);  // zero based day of year
      // On the first day of the year, m_simDate has already been advanced in StartYear().
      if (dayOfYear > 0) pEnvContext->m_simDate.MoveToNextDay(pEnvContext->m_maxDaysInYear);
      {
         CString msg;
         msg.Format("FlowModel::Run() simDate = %d %d %d", pEnvContext->m_simDate.month, pEnvContext->m_simDate.day, pEnvContext->m_simDate.year);
         Report::LogMsg(msg);
      }
      
      m_flowContext.dayOfYear = dayOfYear;
      m_flowContext.timing = GMT_START_STEP;

      double timeOfDay = m_currentTime-(int)m_currentTime;

      if (!m_estimateParameters)
         msg.Format( "Flow: Time: %6.1f - Simulating HRUs...", m_timeInRun );
      else
         msg.Format( "Flow: Run %i of %i: Time: %6.1f of about %i days ", pEnvContext->run+1, m_numberOfRuns, m_timeInRun, m_numberOfYears*365 );
      
      Report::StatusMsg( msg );
      
      msg.Format("%s %i, %i %f", ::GetMonthStr(pEnvContext->m_simDate.month), pEnvContext->m_simDate.day, pEnvContext->m_simDate.year, m_currentTime);
      ::EnvSetLLMapText( msg );

      // reset accumlator rates
      m_totalWaterInputRate  = 0;
      m_totalWaterOutputRate = 0; 
      for ( int i=0; i < GetFluxInfoCount(); i++ )
         GetFluxInfo( i )->m_totalFluxRate = 0;

      if ( m_pGlobalFlowData )
         {
         start = clock();
         
         SetGlobalFlows();
         
         finish = clock();
         duration = (float)(finish - start) / CLOCKS_PER_SEC;   
         m_globalFlowsRunTime += (float) duration;   
         }

      // any pre-run external methods?
      start = clock();
         
      StartStep( &m_flowContext );
     
      finish = clock();
      duration = (float)(finish - start) / CLOCKS_PER_SEC;   
      m_externalMethodsRunTime += (float) duration;   
           
      start = clock();
      
      //if ( m_gwMethod != GW_EXTERNAL )
      //   ;  //m_reachBlock.Integrate( m_currentTime, m_currentTime+stepSize, GetReachDerivatives, &m_flowContext );  // NOTE: GetReachDerivatives does not work
      //else
      //   SolveGWDirect();
      
      finish = clock();
      duration = (float)(finish - start) / CLOCKS_PER_SEC;   
      m_gwRunTime += (float) duration;  

      SetGlobalReservoirFluxes();         // establish inflows and outflows for all Reservoirs(NOTE: MAY NEED TO MOVE THIS TO GetCatchmentDerivatives()!!!!

      start = clock();
      
      // GMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENT GMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENT
      m_flowContext.timing = GMT_CATCHMENT; // GMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENT GMT_CATCHMENTGMT_CATCHMENTGMT_CATCHMENT

      // Note: GetCatchmentDerivates() calls GlobalMethod::Run()'s for any method with (m_timing&GMT_CATCHMENT) != 0 
      // This is when HBV does its work.
      m_hruBlock.Integrate( (double)m_currentTime, (double)m_currentTime+stepSize, GetCatchmentDerivatives, &m_flowContext );  // update HRU swc and associated state variables

      // At this point, HRU water compartments have been updated for precip and infiltration, but not for
      // flows from reaches.
 
      for (int hru_ndx = 0; hru_ndx < m_hruArray.GetSize(); hru_ndx++)
      {
         HRU * pHRU = m_hruArray[hru_ndx];
         HRULayer* pBox_snow = pHRU->GetLayer(BOX_SNOW);
         HRULayer* pBox_surface_h2o = pHRU->GetLayer(BOX_SURFACE_H2O);
         HRULayer* pBox_nat_soil = pHRU->GetLayer(BOX_NAT_SOIL);
         HRULayer* pBox_irrig_soil = pHRU->GetLayer(BOX_IRRIG_SOIL);
         HRULayer* pBox_fast_gw = pHRU->GetLayer(BOX_FAST_GW);
         HRULayer* pBox_slow_gw = pHRU->GetLayer(BOX_SLOW_GW);
         
         pHRU->SetAttFloat(HruSNOW_BOX, (float)(1000. * pBox_snow->m_volumeWater / pHRU->m_HRUtotArea_m2)); 
         pHRU->SetAtt(HruBOXSURF_M3, pBox_surface_h2o->m_volumeWater); 

         float hru_irrigated_area_m2 = (float)pHRU->m_areaIrrigated;
         float hru_natural_area_m2 = pHRU->m_HRUeffArea_m2 - hru_irrigated_area_m2;
         ASSERT(hru_natural_area_m2 >= 0);
         double nat_soil_h2o_mm = 1000. * pBox_nat_soil->m_volumeWater / hru_natural_area_m2;
         pHRU->SetAttFloat(HruNAT_SOIL, (float)nat_soil_h2o_mm);

         double irrig_soil_h2o_mm = hru_irrigated_area_m2 > 0 ? (1000. * pBox_irrig_soil->m_volumeWater / hru_irrigated_area_m2) : 0.;
         pHRU->SetAttFloat(HruIRRIG_SOIL, (float)irrig_soil_h2o_mm);

         pHRU->SetAttFloat(HruGW_FASTBOX, (float)(1000. * pBox_fast_gw->m_volumeWater / pHRU->m_HRUtotArea_m2));
         pHRU->SetAttFloat(HruGW_SLOWBOX, (float)(1000. * pBox_slow_gw->m_volumeWater / pHRU->m_HRUtotArea_m2));

         double box_snow_m3 = pBox_snow->m_volumeWater;
//x         double surface_h2o_m3 = pBox_surface_h2o->m_volumeWater;

         // How much of the surface water is meltwater in the snowpack, and how much is standing water in wetlands?
         double hru_standing_h2o_m3 = 0.;
         double hru_standing_h2o_area_m2 = 0.;
         int hru_wetl_id = NON_WETLAND_TOKEN; 
         int count = (int)pHRU->m_polyIndexArray.GetSize();
         double wetl2q_accum_m3 = 0.;
         for (int i = 0; i < count; i++)
         {
            int idu_poly_ndx = pHRU->m_polyIndexArray[i];
            int lulc_a = AttInt(idu_poly_ndx, LULC_A);
            if (lulc_a != LULCA_WETLAND) continue;

            // Update the IDU attribute WETL2Q.
            double idu_wetl2q_cms = 0.;
            double wetness_mm = Att(idu_poly_ndx, WETNESS);
            double flooddepth_mm = Att(idu_poly_ndx, FLOODDEPTH);
            ASSERT(wetness_mm >= 0 || flooddepth_mm == 0);
            double standing_h2o_mm = wetness_mm + flooddepth_mm;
            if (standing_h2o_mm > 0.)
            {
               float idu_area_m2 = AttFloat(idu_poly_ndx, AREA);
               double wetl_cap_mm = Att(idu_poly_ndx, WETL_CAP);
               if (standing_h2o_mm > wetl_cap_mm)
               { // Standing water in excess of WETL_CAP should flow back to the reach as WETL2Q.
                  double idu_wetl2q_mm = standing_h2o_mm - wetl_cap_mm;
                  double idu_wetl2q_m3 = (idu_wetl2q_mm / 1000.) * idu_area_m2;
                  wetl2q_accum_m3 += idu_wetl2q_m3;
                  idu_wetl2q_cms = idu_wetl2q_m3 / SEC_PER_DAY;
                  wetness_mm = wetl_cap_mm;
                  flooddepth_mm = standing_h2o_mm - wetness_mm;
                  SetAtt(idu_poly_ndx, WETNESS, wetness_mm);
                  SetAtt(idu_poly_ndx, FLOODDEPTH, 0.);

                  // An HRU can can contain all or part of at most one wetland.
                  int idu_wetl_id = AttInt(idu_poly_ndx, WETL_ID);
                  if (hru_wetl_id == NON_WETLAND_TOKEN) hru_wetl_id = idu_wetl_id;
                  else ASSERT(hru_wetl_id == idu_wetl_id);
               } // end of if (standing_h2o_mm > wetl_cap_mm)

               double idu_standing_h2o_m3 = (wetness_mm / 1000.) * idu_area_m2;
               hru_standing_h2o_m3 += idu_standing_h2o_m3;
               hru_standing_h2o_area_m2 += idu_area_m2;
            } // end of if (standing_h2o_mm > 0.)

            SetAtt(idu_poly_ndx, WETL2Q, idu_wetl2q_cms);
         } // end of loop thru IDUs for this HRU
         ASSERT(!pHRU->m_standingH2Oflag || hru_standing_h2o_area_m2 > 0.);

         if (wetl2q_accum_m3 > 0.)
         {
            ASSERT(pBox_surface_h2o->GetFluxValue() == 0);

            int num_wetl = (int)m_wetlArray.GetSize();
            int wetl_ndx = 0;
            while (wetl_ndx < num_wetl && m_wetlArray[wetl_ndx]->m_wetlID != hru_wetl_id) wetl_ndx++;
            ASSERT(wetl_ndx < num_wetl);
            Wetland* pWetl = m_wetlArray[wetl_ndx];
            int reach_ndx = pWetl->m_wetlReachNdxArray[0];
            Reach* pReach = m_reachArray[reach_ndx];
            WaterParcel dischargeWP = pReach->GetReachDischargeWP();
            double reach_degC = dischargeWP.WaterTemperature(); // This neglects the effect on the water temperature of flowing through the wetland.
            WaterParcel wetl2qWP(wetl2q_accum_m3, reach_degC);
            pReach->AccumWPadditions(wetl2qWP);
         }

         double hru_h2o_melt_m3 = pBox_surface_h2o->m_volumeWater - hru_standing_h2o_m3;
         if (hru_h2o_melt_m3 < 0.)
         {
            if (pBox_surface_h2o->m_volumeWater == 0 || hru_standing_h2o_m3 == 0)
               ASSERT(close_enough(abs(hru_h2o_melt_m3), 0, 0.001, 1.0));
            else
            { // Meltwater, which is the difference between pBox_surface_h2o->m_volumeWater and hru_standing_h2o_m3, 
               // when it is negative should be a tiny fraction of hru_standing_h2o_m3 arising from roundoff.
               ASSERT(pBox_surface_h2o->m_volumeWater > 0);
               double fraction = hru_h2o_melt_m3 / pBox_surface_h2o->m_volumeWater;
               ASSERT(close_enough(fraction, 0, 1e-2, 1e-2));
            }
            hru_h2o_melt_m3 = 0.;
         }
         pHRU->SetAtt(HruH2OMELT_M3, hru_h2o_melt_m3);
         pHRU->SetAtt(HruH2OSTNDGM3, hru_standing_h2o_m3);

         // Update IDU attributes SM_DAY, H2O_MELT, and SNOW_SWE
         double hru_h2o_melt_area_m2 = pHRU->m_HRUtotArea_m2 - pHRU->m_wetlandArea_m2;
         ASSERT(hru_h2o_melt_area_m2 == pHRU->m_snowpackArea_m2);
         ASSERT(hru_h2o_melt_area_m2 > 0.);
         double hru_h2o_melt_mm = (hru_h2o_melt_area_m2 > 0.) ? (1000. * (hru_h2o_melt_m3 / hru_h2o_melt_area_m2)) : 0.;
         double frozen_snow_for_non_wetland_idus_mm = (box_snow_m3 / hru_h2o_melt_area_m2) * 1000.;
         double snow_swe_for_non_wetland_idus_mm = frozen_snow_for_non_wetland_idus_mm + hru_h2o_melt_mm;
         for (int i = 0; i < count; i++)
         { 
            int idu_ndx = pHRU->m_polyIndexArray[i];
            float idu_area_m2 = -1; m_pIDUlayer->GetData(idu_ndx, m_colAREA, idu_area_m2);
            int idu_irrigation = -1; m_pIDUlayer->GetData(idu_ndx, m_colIRRIGATION, idu_irrigation);
            int layer = (idu_irrigation == 1) ? BOX_IRRIG_SOIL : BOX_NAT_SOIL;
            float effective_hru_area_m2 = idu_irrigation == 1 ? hru_irrigated_area_m2 : hru_natural_area_m2;
            ASSERT(effective_hru_area_m2 > 0);
            float idu_frac_of_hru = idu_area_m2 / effective_hru_area_m2; 
            if (idu_frac_of_hru > 1.) { ASSERT(close_enough(idu_frac_of_hru, 1., 1e-6)); idu_frac_of_hru = 1.; }
            float sm_day_mm = (float)(pHRU->GetLayer(layer)->m_volumeWater * idu_frac_of_hru / idu_area_m2) * MM_PER_M;
            m_pIDUlayer->SetDataU(idu_ndx, m_colSM_DAY, sm_day_mm);

            int lulc_a = AttInt(idu_ndx, LULC_A);
            bool is_wetland = lulc_a == LULCA_WETLAND;
            if (is_wetland)
            { // WETNESS was set earlier this day in HBV
               double wetness = Att(idu_ndx, WETNESS); // for debugging
               SetAtt(idu_ndx, SNOW_SWE, 0);
               SetAtt(idu_ndx, H2O_MELT, 0);
            }
            else
            { // Not a wetland, may have a snowpack.
               SetAtt(idu_ndx, SNOW_SWE, snow_swe_for_non_wetland_idus_mm);
               SetAtt(idu_ndx, H2O_MELT, hru_h2o_melt_mm);
            }
         } // end of loop thru the IDUs contained in this HRU

         if (!CheckSurfaceH2O(pHRU, 0.))
         {
            CString msg;
            msg.Format("FlowModel::Run() For pHRU->m_id = %d, CheckSurfaceH2O() returned false.", pHRU->m_id);
            Report::LogWarning(msg);
         }
         
      } // end of loop thru HRUs

      finish = clock();
      duration = (float)(finish - start) / CLOCKS_PER_SEC;   
      
      m_hruIntegrationRunTime += (float) duration;   
      
      m_flowContext.Reset();

      // reservoirs
      start = clock();
      
      // Note: GetReservoirDerivatives() calls GlobalMethod::Run()'s for any method with 
      // (m_timing&GMT_RESERVOIR) != 0 
      m_flowContext.timing = GMT_RESERVOIR;
      m_reservoirBlock.Integrate( m_currentTime, m_currentTime+stepSize, GetReservoirDerivatives, &m_flowContext );     // update stream reaches
      
      finish = clock();
      duration = (float)(finish - start) / CLOCKS_PER_SEC;   
      m_reservoirIntegrationRunTime += (float) duration;   
      
      // check reservoirs for filling
      for ( int i=0; i < (int) m_reservoirArray.GetSize(); i++ )
         {
         Reservoir *pRes = m_reservoirArray[ i ];

         //CString msg;
         //msg.Format( "Reservoir check: Filled=%i, elevation=%f, dam top=%f", pRes->m_filled, pRes->m_elevation, pRes->m_dam_top_elev );
         //Report::LogMsg( msg );
         if ( pRes->m_filled == 0 && pRes->m_elevation >= ( pRes->m_dam_top_elev-2.0f ) )
            {
            //Report::LogMsg( "Reservoir filled!!!");
            pRes->m_filled = 1;   
            }
         }

      m_flowContext.Reset();
      
      start = clock();

      m_flowContext.timing = GMT_REACH;
      GlobalMethodManager::SetTimeStep(stepSize );
      GlobalMethodManager::Step( &m_flowContext );

      ApplyQ2WETL(); // Move water spilling over the stream banks into the wetlands.

      finish = clock();
      duration = (float)(finish - start) / CLOCKS_PER_SEC;   
      m_reachIntegrationRunTime += (float) duration;   
      
      // total refers to mass balance error ceheck
     // msg.Format( "Flow: Time: %6.1f - Computing Overall Mass Balances...", m_timeInRun );
     // Report::StatusMsg( msg );
      if ( m_checkMassBalance )
         {
         start = clock();
    
         m_totalBlock.Integrate( m_currentTime, m_currentTime+stepSize, GetTotalDerivatives, &m_flowContext );

         finish = clock();
         duration = (float)(finish - start) / CLOCKS_PER_SEC;   
         m_massBalanceIntegrationRunTime += (float) duration;   
         }

      start = clock();

      if (m_detailedOutputFlags & 1)  // daily IDU
         {
         if (!m_pGrid)
            SaveDetailedOutputIDU(m_iduDailyFilePtrArray);
        }
      if (m_detailedOutputFlags & 4)  // daily Reach
         SaveDetailedOutputReach(m_reachDailyFilePtrArray);
         

      CollectData( dayOfYear );

      finish = clock();
      duration = (float)(finish - start) / CLOCKS_PER_SEC;   
      m_collectDataRunTime += (float) duration;   

      GetMaxSnowPack(pEnvContext);

      EndStep( &m_flowContext );
      UpdateHRULevelVariables(pEnvContext);
      WriteDataToMap(pEnvContext);

      // any post-run external methods?
      m_flowContext.timing = GMT_END_STEP;
      GlobalMethodManager::EndStep( &m_flowContext );   // this invokes EndStep() for any internal global methods 

	  CollectModelOutput(); // Write the data out to the daily, monthly, and yearly FLOWreports.

      // update time...
      m_currentTime += stepSize;
      m_timeInRun += stepSize;

      // are we close to the end?
      double timeRemaining =  m_stopTime - m_currentTime;

      if ( timeRemaining  < ( m_timeStep + TIME_TOLERANCE ) )
         stepSize = float( m_stopTime - m_currentTime );
      
      if ( m_mapUpdate == 2 && !m_estimateParameters)
        RedrawMap( pEnvContext );

      if ( m_useRecorder && !m_estimateParameters )
         {
         for ( int i=0; i < (int) m_vrIDArray.GetSize(); i++  )
            ::EnvCaptureVideo( m_vrIDArray[ i ] );
         }
      
      if (dayOfYear == 90 && !m_estimateParameters)
         UpdateAprilDeltas(pEnvContext);

      if (dayOfYear == 274)
         ResetCumulativeWaterYearValues();        
      }  // end of: while ( m_time < m_stopTime )   // note - doesn't guarantee alignment with stop time

   // Done with the internal flow timestep loop, finish up...
   EndYear( &m_flowContext );
   
   if ( m_mapUpdate == 1 && !m_estimateParameters) RedrawMap(pEnvContext);   

   if (!m_estimateParameters)
      {
      UpdateYearlyDeltas(pEnvContext);
      ResetCumulativeYearlyValues();
      }

   // If we have already written out the daily binaries, there is no need to write out the annual binaries.
   if (!(m_detailedOutputFlags & 1) && (m_detailedOutputFlags & 2)) SaveDetailedOutputIDU(m_iduAnnualFilePtrArray);
   if (!(m_detailedOutputFlags & 4) && (m_detailedOutputFlags & 8)) SaveDetailedOutputReach(m_reachAnnualFilePtrArray);

    if (!m_estimateParameters)
      {
      for ( int i=0; i < m_reservoirArray.GetSize(); i++ )
         {
         //Reservoir *pRes = m_reservoirArray[ i ];
         //CString outputPath;
         //outputPath.Format("%s%sAppliedConstraints_%s_Run%i.csv", PathManager::GetPath(PM_OUTPUT_DIR), (LPCTSTR) pRes->m_name);
         //pRes->m_pResSimRuleCompare->WriteAscii(outputPath);         
         }

      float channel=0.0f;
      float terrestrial=0.0f;
      GetTotalStorage(channel, terrestrial);
      msg.Format( "Flow: TotalInput: %6.1f 10^6m3. TotalOutput: %6.1f 10^6m3. Terrestrial Storage: %6.1f 10^6m3. Channel Storage: %6.1f 10^6m3  Difference: %6.1f m3", float( m_totalWaterInput/1E6 ), float( m_totalWaterOutput/1E6 ), float( terrestrial/1E6 ), float( channel/1E6), float(m_totalWaterInput-m_totalWaterOutput-(channel+terrestrial)) );
      Report::LogMsg( msg );
      }

   return TRUE;
   } // end of FlowModel::Run()


   bool FlowModel::ApplyQ2WETL()
   { // Moves water from stream reaches to the surface water compartment, wetland by wetland.
      // Assumes FLOODDEPTH is zero on entry. 
      // Allows for the fact that a wetland may extend over more than one HRU.
      // Makes use of the assumption that, for all the IDUs in a single wetland, the elevation above sea level
      // of the surface water when the wetland is full is uniform (i.e. ELEV_MEAN + WETL_CAP/1000 is constant
      // for the IDUs in a given wetland).
      // Updates pHRU->m_standingH2Oflag and HRU attributes HruBOXSURF_M3 and HruH2OSTNDGM3, and IDU attributes WETNESS and FLOODDEPTH.

      int num_wetlands = (int)m_wetlArray.GetSize();
      for (int wetl_ndx = 0; wetl_ndx < num_wetlands; wetl_ndx++)
      {
         Wetland* pWetl = m_wetlArray[wetl_ndx];

         int first_idu_ndx = pWetl->m_wetlIDUndxArray[0];
         double full_wetl_elev_m = AttFloat(first_idu_ndx, ELEV_MEAN) + Att(first_idu_ndx, WETL_CAP) / 1000;

         // First calculate how much water in total we need to move from
         // the reaches to the IDUs in this wetland.
         double wetl_q2wetl_cms_accum = 0;
         int num_idus_in_wetl = (int)pWetl->m_wetlIDUndxArray.GetCount();
         for (int idu_ndx_in_wetl = 0; idu_ndx_in_wetl < num_idus_in_wetl; idu_ndx_in_wetl++)
         { // Since more than one IDU may drain to a given reach, this logic may be traversed more than once for a given reach.
            int idu_poly_ndx = pWetl->m_wetlIDUndxArray[idu_ndx_in_wetl];
            ASSERT(Att(idu_poly_ndx, FLOODDEPTH) == 0);
            float idu_elev_mean_m = AttFloat(idu_poly_ndx, ELEV_MEAN);
            double idu_wetl_cap_mm = Att(idu_poly_ndx, WETL_CAP);
            double test_val = ((idu_wetl_cap_mm / 1000) + idu_elev_mean_m) - full_wetl_elev_m;
            if (test_val != 0)
               ASSERT(close_enough(test_val, 0, .01, .01));

            int reach_ndx = pWetl->m_wetlReachNdxArray[idu_ndx_in_wetl];
            Reach* pReach = m_reachArray[reach_ndx];
            int comid = pReach->m_reachID;
            ASSERT(AttInt(idu_poly_ndx, COMID) == comid);
            if (pReach->m_q2wetl_cms <= 0) continue;

            // Have we already counted the q2wetl from this reach?
            bool already_counted_flag = false;
            int ndx_to_remaining_array = idu_ndx_in_wetl;
            while (ndx_to_remaining_array > 0 && !already_counted_flag)
            {
               ndx_to_remaining_array--;
               already_counted_flag = (reach_ndx == pWetl->m_wetlReachNdxArray[ndx_to_remaining_array]);
            }
            if (!already_counted_flag) wetl_q2wetl_cms_accum += pReach->m_q2wetl_cms;
         } // end of loop thru the IDUs in this wetland
         double wetl_q2wetl_m3 = wetl_q2wetl_cms_accum * SEC_PER_DAY;

         if (wetl_q2wetl_m3 <= 0) continue;

         // Now figure out how high the surface water will get and 
         // how many of the IDUs in the wetland will be affected by adding that water.
         double final_surf_h2o_elev_m = 0;
         int num_affected_idus = 0;
         double remaining_q2wetl_m3 = wetl_q2wetl_m3;
         double current_filling_area_m2 = 0;
         for (int idu_ndx_in_wetl = 0; idu_ndx_in_wetl < num_idus_in_wetl && remaining_q2wetl_m3 > 0; idu_ndx_in_wetl++)
         {
            int idu_poly_ndx = pWetl->m_wetlIDUndxArray[idu_ndx_in_wetl];
            float idu_area_m2 = AttFloat(idu_poly_ndx, AREA);
            double wetness_mm = Att(idu_poly_ndx, WETNESS);
            double h2o_for_soil_mm = wetness_mm >= 0 ? 0 : -wetness_mm;
            double vol_for_soil_m3 = (h2o_for_soil_mm / 1000) * idu_area_m2;

            // How much capacity for soil and surface water is there before the next higher IDU begins to fill?
            current_filling_area_m2 += idu_area_m2;
            double top_elev_m = idu_ndx_in_wetl + 1 >= num_idus_in_wetl ? full_wetl_elev_m :
               AttFloat(pWetl->m_wetlIDUndxArray[idu_ndx_in_wetl + (int)1], ELEV_MEAN);
            double already_filled_mm = wetness_mm > 0 ? wetness_mm : 0;
            double already_filled_m = already_filled_mm / 1000;
            double current_filling_depth_m = max(top_elev_m - AttFloat(idu_poly_ndx, ELEV_MEAN) - already_filled_m, 0);
            ASSERT(current_filling_depth_m >= 0);
            double current_filling_vol_m3 = vol_for_soil_m3 + current_filling_area_m2 * current_filling_depth_m;

            num_affected_idus++;
            if (current_filling_vol_m3 > remaining_q2wetl_m3)
            { // The remaining water is less than what it takes to fill the current IDU to the point 
               // where it begins to fill the next higher IDU. Calculate how high the water gets.
               double idu_surf_h2o_m3 = remaining_q2wetl_m3 > vol_for_soil_m3 ? (remaining_q2wetl_m3 - vol_for_soil_m3) : 0;
               double idu_surf_h2o_mm = 1000 * (idu_surf_h2o_m3 / idu_area_m2);
               final_surf_h2o_elev_m = Att(idu_poly_ndx, ELEV_MEAN) + (idu_surf_h2o_mm / 1000);
            }
            else remaining_q2wetl_m3 -= current_filling_vol_m3;
         } // end of loop thru the idus in this wetland

         // Is there flooding?
         if (remaining_q2wetl_m3 > 0)
         { // Yes, there is flooding.
            ASSERT(current_filling_area_m2 == pWetl->m_wetlArea_m2);
            double flooddepth_mm = 1000 * (remaining_q2wetl_m3 / pWetl->m_wetlArea_m2);
            for (int idu_ndx_in_wetl = 0; idu_ndx_in_wetl < num_idus_in_wetl; idu_ndx_in_wetl++)
            {
               int idu_poly_ndx = pWetl->m_wetlIDUndxArray[idu_ndx_in_wetl];
               double wetness_mm = Att(idu_poly_ndx, WETNESS);
               double h2o_for_soil_mm = wetness_mm >= 0 ? 0 : -wetness_mm;
               double current_idu_surf_h2o_mm = wetness_mm >= 0 ? wetness_mm : 0;
               double depth_to_add_mm = h2o_for_soil_mm + (Att(idu_poly_ndx, WETL_CAP) - current_idu_surf_h2o_mm) + flooddepth_mm;
               float idu_area_m2 = AttFloat(idu_poly_ndx, AREA);
               double idu_h2o_to_add_m3 = (depth_to_add_mm / 1000) * idu_area_m2;

               int comid = AttInt(idu_poly_ndx, COMID);
               bool ret_val = pWetl->ReachH2OtoWetlandIDU(comid, idu_h2o_to_add_m3, idu_poly_ndx);
            } // end of loop thru the idus in this wetland
         } // end of flooding block: if (remaining_q2wetl_m3 > 0) ...
         else
         { // no flooding
            ASSERT(final_surf_h2o_elev_m > Att(first_idu_ndx, ELEV_MEAN) && final_surf_h2o_elev_m <= full_wetl_elev_m);
            for (int idu_ndx_in_wetl = 0; idu_ndx_in_wetl < num_affected_idus; idu_ndx_in_wetl++)
            {
               int idu_poly_ndx = pWetl->m_wetlIDUndxArray[idu_ndx_in_wetl];
               double wetness_mm = Att(idu_poly_ndx, WETNESS);
               double h2o_for_soil_mm = wetness_mm >= 0 ? 0 : -wetness_mm;
               double current_idu_surf_h2o_mm = wetness_mm >= 0 ? wetness_mm : 0;
               double final_idu_surf_h2o_mm = 1000 * (final_surf_h2o_elev_m - Att(idu_poly_ndx, ELEV_MEAN));
               double idu_surf_h2o_to_add_mm = final_idu_surf_h2o_mm - current_idu_surf_h2o_mm;
               double idu_h2o_to_add_mm = h2o_for_soil_mm + idu_surf_h2o_to_add_mm;
               float idu_area_m2 = AttFloat(idu_poly_ndx, AREA);
               double idu_h2o_to_add_m3 = idu_area_m2 * (idu_h2o_to_add_mm / 1000);
               int comid = AttInt(idu_poly_ndx, COMID);
               bool ret_val = pWetl->ReachH2OtoWetlandIDU(comid, idu_h2o_to_add_m3, idu_poly_ndx);
            } // end of loop thru the idus in this wetland
         } // end of if (remaining_q2wetl_m3 > 0) else ...

      } // end of loop thru wetlands

      return(true);
   } // end of ApplyQ2WETL()


bool Wetland::ReachH2OtoWetlandIDU(int reachComid, double H2OtoWetl_m3, int iduPolyNdx)
// Updates HRU attributes HruBOXSURF_M3, HruH2OSTNDGM3, and IDU attributes WETNESS, FLOODDEPTH, and TEMP_WETL.
// Sets pHRU->m_standingH2Oflag to true when there is standing water.
// Assumes FLOODDEPTH for the IDU is 0 on entry.
{
   int idu_ndx = iduPolyNdx;
   double remaining_m3 = H2OtoWetl_m3;
   Reach* pReach = gFlowModel->FindReachFromID(reachComid);

   ASSERT(H2OtoWetl_m3 > 0.);
   if (H2OtoWetl_m3 <= 0.) return(0.);
   int idu_comid = gIDUs->AttInt(idu_ndx, COMID);
   ASSERT(idu_comid == reachComid);;

   double reach_temp_degC = pReach->Att(ReachTEMP_H2O);

   int hru_ndx = gIDUs->AttInt(idu_ndx, HRU_NDX);
   HRU* pHRU = gFlowModel->GetHRU(hru_ndx);
   double wetness_mm = gFlowModel->Att(idu_ndx, WETNESS);
   if (wetness_mm > 0)
      ASSERT(pHRU->m_standingH2Oflag);
   double idu_area_m2 = gFlowModel->Att(idu_ndx, AREA);
   double to_this_idu_m3 = H2OtoWetl_m3;
   double to_this_idu_mm = (to_this_idu_m3 / idu_area_m2) * 1000.;

   // If the soil is not yet saturated (i.e. wetness < 0), it can absorb some or all of the 
   // water coming from the reach.
   double to_this_idu_soil_m3 = 0.;
   double new_wetness_mm = wetness_mm;
   ASSERT(gIDUs->Att(idu_ndx, FLOODDEPTH) == 0);
   double new_flooddepth_mm = 0;
   if (wetness_mm < 0.)
   { // The soil can absorb more water.
      double idu_room_in_soil_mm = -wetness_mm;
      double idu_room_in_soil_m3 = (idu_room_in_soil_mm / 1000.) * idu_area_m2;
      to_this_idu_soil_m3 = min(idu_room_in_soil_m3, to_this_idu_m3);
      double to_this_idu_soil_mm = (to_this_idu_soil_m3 / idu_area_m2) * 1000.;
      HRULayer* pBOX_NAT_SOIL = pHRU->GetLayer(BOX_NAT_SOIL);
      pBOX_NAT_SOIL->AccumAdditions((float)to_this_idu_soil_m3);
      double hru_nat_soil_mm = pHRU->AttFloat(HruNAT_SOIL);
      float hru_area_m2 = pHRU->AttFloat(HruAREA_M2);
      float hru_area_irrig_m2 = pHRU->AttFloat(HruAREA_IRRIG);
      double hru_area_nat_soil_m2 = (double)(hru_area_m2 - hru_area_irrig_m2);
      double hru_nat_soil_m3 = (hru_nat_soil_mm / 1000) * hru_area_nat_soil_m2;
      hru_nat_soil_m3 += to_this_idu_soil_m3;
      ASSERT(hru_area_nat_soil_m2 > 0);
      hru_nat_soil_mm = hru_area_nat_soil_m2 > 0 ? (1000 * (hru_nat_soil_m3 / hru_area_nat_soil_m2)) : 0;
      pHRU->SetAttFloat(HruNAT_SOIL, (float)hru_nat_soil_mm);

      if (idu_room_in_soil_m3 > to_this_idu_soil_m3)
         new_wetness_mm = -(-wetness_mm - to_this_idu_soil_mm);
      else new_wetness_mm = 0;
   } // end of if (wetness_mm < 0.)

   // When the soil is saturated, then any additional water becomes standing water.
   double to_this_idu_standing_h2o_m3 = to_this_idu_m3 - to_this_idu_soil_m3;
   if (to_this_idu_standing_h2o_m3 > 0)
   {
      HRULayer* pBOX_SURFACE_H2O = pHRU->GetLayer(BOX_SURFACE_H2O);
      pBOX_SURFACE_H2O->AccumAdditions((float)to_this_idu_standing_h2o_m3);

      double standing_h2o_m3 = (wetness_mm > 0) ? ((wetness_mm / 1000.) * idu_area_m2) : 0.;
      double standing_h2o_degC = (standing_h2o_m3 > 0.) ? gIDUs->Att(idu_ndx, TEMP_WETL) : 0.;
      WaterParcel standingWP(standing_h2o_m3, standing_h2o_degC);
      WaterParcel to_this_iduWP = WaterParcel(to_this_idu_standing_h2o_m3, reach_temp_degC);
      standingWP.MixIn(to_this_iduWP); standing_h2o_m3 = standingWP.m_volume_m3;

      // Divide up the standing water between WETNESS and FLOODDEPTH.
      double standing_h2o_mm = 1000 * (standing_h2o_m3 / idu_area_m2);
      double wetl_cap_mm = gIDUs->Att(idu_ndx, WETL_CAP);
      new_wetness_mm = standing_h2o_mm > wetl_cap_mm ? wetl_cap_mm : standing_h2o_mm;
      new_flooddepth_mm = standing_h2o_mm > new_wetness_mm ? standing_h2o_mm - new_wetness_mm : 0;
      double new_standing_h2o_degC = standingWP.WaterTemperature();
      gIDUs->SetAtt(idu_ndx, TEMP_WETL, new_standing_h2o_degC);

      pHRU->m_standingH2Oflag = true;
      double hru_H2OSTNDGM3 = pHRU->Att(HruH2OSTNDGM3);
      hru_H2OSTNDGM3 += to_this_idu_standing_h2o_m3;
      pHRU->SetAtt(HruH2OSTNDGM3, hru_H2OSTNDGM3);
      double hru_BOXSURF_M3 = pHRU->Att(HruBOXSURF_M3);
      hru_BOXSURF_M3 += to_this_idu_standing_h2o_m3;
      pHRU->SetAtt(HruBOXSURF_M3, hru_BOXSURF_M3);
   }
   gFlowModel->SetAtt(idu_ndx, WETNESS, new_wetness_mm); 
   gFlowModel->SetAtt(idu_ndx, FLOODDEPTH, new_flooddepth_mm); 

   return(true);
} // end of ReachH2OtoWetlandIDU()


bool Wetland::InitWETL_CAPifNecessary() // Set nominal values of WETL_CAP. 
{
   float lowest_idu_elev_m = gIDUs->AttFloat(m_wetlIDUndxArray[0], ELEV_MEAN);
   int num_idus_in_wetl = (int)m_wetlIDUndxArray.GetSize();
   float highest_idu_elev_m = gIDUs->AttFloat(m_wetlIDUndxArray[num_idus_in_wetl - 1], ELEV_MEAN);

   double existing_wetl_cap_0_mm = gIDUs->Att(m_wetlIDUndxArray[0], WETL_CAP);
   double wetl_cap_0_mm = existing_wetl_cap_0_mm;
   if (existing_wetl_cap_0_mm <= 0.)
   {
      double nominal_wetl_cap_0_mm = (((double)highest_idu_elev_m - (double)lowest_idu_elev_m) + 1.) * MM_PER_M;
      wetl_cap_0_mm = nominal_wetl_cap_0_mm;
      gIDUs->SetAtt(m_wetlIDUndxArray[0], WETL_CAP, wetl_cap_0_mm);
   }

   int i = 1;
   while (i < num_idus_in_wetl)
   {
      double wetl_cap_i_mm = gIDUs->Att(m_wetlIDUndxArray[i], WETL_CAP);
      float idu_elev_m = gIDUs->AttFloat(m_wetlIDUndxArray[i], ELEV_MEAN);
      double elev_diff_mm = 1000. * ((double)idu_elev_m - (double)lowest_idu_elev_m);
      ASSERT(elev_diff_mm >= 0.);
      if (wetl_cap_i_mm <= 0.)
      {
         wetl_cap_i_mm = wetl_cap_0_mm - elev_diff_mm;
         ASSERT(wetl_cap_i_mm > 0.);
         gIDUs->SetAtt(m_wetlIDUndxArray[i], WETL_CAP, wetl_cap_i_mm);
      }
      else if (!close_enough(wetl_cap_i_mm + elev_diff_mm, wetl_cap_0_mm, 0.001, 1.))
      {
         CString msg;
         msg.Format("InitWETL_CAPifNecessary() wetl_id = %d, idu_id = %d, wetl_cap_i_mm = %f is inconsistent with wetl_cap_0_mm = %f",
            m_wetlID, gIDUs->AttInt(m_wetlIDUndxArray[i], IDU_ID), wetl_cap_i_mm, wetl_cap_0_mm);
         Report::WarningMsg(msg);
      }

      i++;
   } // end of loop thru additional IDUs in the wetland

   return(true);
} // end of InitWETL_CAPifNecessary()


bool Reach::CalcReachVegParamsIfNecessary()
{
   Scenario* pSimulationScenario = gpModel->m_flowContext.pEnvContext->pEnvModel->GetScenario();
   Shade_a_latorData* pSAL = &(pSimulationScenario->m_shadeAlatorData);

   // Calculate the reach vegetation parameters only on the first day of every year
   bool sal_reach = false; gpModel->m_pReachLayer->GetData(m_polyIndex, gpModel->m_colReachSAL_REACH, sal_reach);
   double veg_ht_l_m = 0.;
   double veg_ht_r_m = 0.;
   double veg_ht_m = 0.;
   double lai_reach = 0.;
   double width_reach_m = 0.;
   double width_given_m = Att(ReachWIDTHGIVEN);
   if (sal_reach)
   { // Shade-a-lator mode for this reach
      gpModel->m_pReachLayer->GetData(m_polyIndex, gpModel->m_colReachVEG_HT_L, veg_ht_l_m);
      gpModel->m_pReachLayer->GetData(m_polyIndex, gpModel->m_colReachVEG_HT_R, veg_ht_r_m);
      veg_ht_m = (veg_ht_l_m + veg_ht_r_m) / 2.;
      m_topo.m_vegDensity = pSAL->m_canopyDensity_pct / 100.;
      gpModel->m_pReachLayer->SetDataU(m_polyIndex, gpModel->m_colReachVGDNSGIVEN, m_topo.m_vegDensity);
      lai_reach = pSAL->m_SALlai;
      width_reach_m = width_given_m;
      ASSERT(width_reach_m >= Att(ReachWIDTH_MIN));
   }
   else 
   { // not Shade-a-lator mode for this reach
      int bank_l_idu_ndx = -1; gpModel->m_pReachLayer->GetData(m_polyIndex, gpModel->m_colReachBANK_L_IDU, bank_l_idu_ndx);
      int bank_r_idu_ndx = -1; gpModel->m_pReachLayer->GetData(m_polyIndex, gpModel->m_colReachBANK_R_IDU, bank_r_idu_ndx);
      gpModel->m_pIDUlayer->GetData(bank_l_idu_ndx, gpModel->m_colTREE_HT, veg_ht_l_m);
      gpModel->m_pIDUlayer->GetData(bank_r_idu_ndx, gpModel->m_colTREE_HT, veg_ht_r_m);
      veg_ht_m = (veg_ht_l_m + veg_ht_r_m) / 2.;

      double lai_l = -1; gpModel->m_pIDUlayer->GetData(bank_l_idu_ndx, gpModel->m_colLAI, lai_l);
      double lai_r = -1; gpModel->m_pIDUlayer->GetData(bank_r_idu_ndx, gpModel->m_colLAI, lai_r);
      lai_reach = (lai_l + lai_r) / 2.;
      m_topo.m_vegDensity = FlowModel::VegDensity(lai_reach);
      ASSERT(m_topo.m_vegDensity >= 0.);
      gpModel->m_pReachLayer->SetDataU(m_polyIndex, gpModel->m_colReachVGDNS_CALC, m_topo.m_vegDensity);

      double width_calc_m = Att(ReachWIDTH_CALC);
      double width_min_m = Att(ReachWIDTH_MIN);
      width_reach_m = (width_given_m > 0.) ? width_given_m : width_calc_m;
      if (width_reach_m < width_min_m) width_reach_m = width_min_m;
   }
   gpModel->m_pReachLayer->SetDataU(m_polyIndex, gpModel->m_colReachWIDTHREACH, width_reach_m);
   ASSERT(width_reach_m > 0.);

   double veghtgiven_m = Att(ReachVEGHTGIVEN);
   double veghtreach_m = (veghtgiven_m >= 0.) ? veghtgiven_m : veg_ht_m;
   gpModel->m_pReachLayer->SetDataU(m_polyIndex, gpModel->m_colReachVEGHTREACH, veghtreach_m);

   gpModel->m_pReachLayer->SetDataU(m_polyIndex, gpModel->m_colReachLAI_REACH, lai_reach);
   gpModel->m_pReachLayer->SetDataU(m_polyIndex, gpModel->m_colReachVGDNSREACH, m_topo.m_vegDensity);

   double direction_deg = 0.; gpModel->m_pReachLayer->GetData(m_polyIndex, gpModel->m_colReachDIRECTION, direction_deg);
   m_topo.m_vegElevEW_deg = m_topo.VegElevEW_deg(direction_deg, width_reach_m, veg_ht_m);
   m_topo.m_vegElevNS_deg = m_topo.VegElevNS_deg(direction_deg, width_reach_m, veg_ht_m);

   return(true);
} // end of CalcReachVegParamsIfNecessary()


bool FlowModel::ProcessShade_a_latorInputData(Shade_a_latorData* pSAL, FDataObj* pData, int dataRows, int reachCt)
{ // For each reach, average the Shade-a-lator data over all the segments which overlap it.
   // Column numbers in the Shade-a-lator input file
   // 0 Downstream_end - River km of downstream end of segment
   // 1 Elevation
   // 2 Bottom Width (m)
   // 3, 4, 5 West, South, East - topographic elevations, degrees
   // 6 Emergent Veg
   // 7-15 Veg_<i>_NE - i = 1...9, vegetation heights, NE from center of stream
   // 16-24 Veg_<i>_E - vegetation heights, E from center of stream
   // 25-33 Veg_<i>_SE
   // 34-42 Veg_<i>_S
   // 43-51 Veg_<i>_SW
   // 52-60 Veg_<i>_W
   // 61-69 Veg_<i>_NW
   // 70-78 Elv_<i>_NE - Elevations, NE from center of stream
   // 79-132 Elv_<i>_E, Elv_<i>_SE, ..., Elv_<i>_NW

   // The sum of the lengths of the reaches from the specified upstream reach to the specified downstream reach
   // may be different from the sum of the segment lengths in the data.
   double reaches_len_m = 0.;
   Reach * pReach = pSAL->m_pReach_upstream;
   for (int i = 0; i < reachCt; i++)
   {
      reaches_len_m += pReach->m_length;
      pReach = GetReachFromNode(pReach->m_pDown);
   } // end of loop through reaches
   pSAL->m_reachesLength_m = reaches_len_m;

   // Now, if there are any reaches below the specified downstream reach,
   // add up the lengths of all the reaches below the downstream reach
   // to determine the river km of the downstream end of the specified downstream reach.
   double river_km_at_lower_end = 0.;
   while (pReach != NULL)
   {
      pReach = GetReachFromNode(pReach->m_pDown);
      river_km_at_lower_end += pReach->m_length / 1000.;
   }
   pSAL->m_riverKmAtUpperEnd = river_km_at_lower_end + reaches_len_m / 1000.;
   
   // "final segment" here means the most upstream segment; its data is in row 0
   // river kilometers increase in the upstream direction
   float downstream_end_of_final_segment_km = 0.; pData->Get(0, 0, downstream_end_of_final_segment_km);
   float downstream_end_of_first_segment_km = 0.; pData->Get(0, dataRows - 1, downstream_end_of_first_segment_km);
   double adj_SAL_len_m = // adjusted Shade-a-lator length in meters = Shade-a-lator length not counting the final segment
      1000. * ((double)downstream_end_of_final_segment_km - (double)downstream_end_of_first_segment_km);
   CString msg;
   msg.Format("adj_SAL_len_m = %f, reaches_len_m = %f", adj_SAL_len_m, reaches_len_m);
   Report::LogMsg(msg);

   // Make sure the Shade-a-lator segments overlap the upstream reach entirely.
   // Since we don't know for sure how long the final segment in row 0 is, start with row 1.
   ASSERT(dataRows >= 2);
   int row = 1; // index to Shade-a-lator river segment
   // The upper end of the segment in row 1 is the lower end of the final segment in row 0.
   float segment_upstream_end_km = downstream_end_of_final_segment_km;
   float segment_downstream_end_km = 0.; pData->Get(0, 1, segment_downstream_end_km);
   double reach_upstream_end_km = pSAL->m_riverKmAtUpperEnd;
   pReach = pSAL->m_pReach_upstream;
   while (reach_upstream_end_km > segment_upstream_end_km)
   { // Move downstream by one reach.
      reach_upstream_end_km -= pReach->m_length / 1000.;
      pReach = GetReachFromNode(pReach->m_pDown);
      ASSERT(pReach != NULL);
   }
   double reach_downstream_end_km = reach_upstream_end_km - pReach->m_length / 1000.;
   // Now we have assured that the upstream segment goes at least as far upstream as the current reach.

   // For each reach, average the Shade-a-lator data over all the segments which overlap it.
   double below_the_bottom_m = 0.;
   double total_length_m = 0.;
   double segment_len_m = 0.;
   while (pReach != NULL && row < dataRows)
   { // This is the reach loop
      double topo_E_deg = 0., topo_S_deg = 0., topo_W_deg = 0.;
      double reach_veg_ht_m = 0;
      double elv_m[7] = {0., 0., 0., 0., 0., 0., 0.}; 
      double width_m = 0.;
      double elev_m = 0.;

      double frac_of_reach = 0.;
      total_length_m = 0.;
      while (segment_upstream_end_km > reach_downstream_end_km && row < dataRows) // frac_of_reach < 1. && row < data_rows)
      {
         segment_len_m = ((double)segment_upstream_end_km - (double)segment_downstream_end_km) * 1000.;
         // What part of this segment overlaps the current reach?
         double over_the_top_m = (segment_upstream_end_km > reach_upstream_end_km) ?
            (segment_upstream_end_km - reach_upstream_end_km) * 1000. : 0;
         below_the_bottom_m = (segment_downstream_end_km < reach_downstream_end_km) ?
            (reach_downstream_end_km - segment_downstream_end_km) * 1000. : 0;

         double overlap_m = segment_len_m - over_the_top_m - below_the_bottom_m;
         if (overlap_m < 0.) overlap_m = 0.;

         if (overlap_m > 0.)
         {
            total_length_m += overlap_m;
            double seg_frac_of_reach = overlap_m / pReach->m_length;
            frac_of_reach += seg_frac_of_reach;
            ASSERT(frac_of_reach < 1. || close_enough(frac_of_reach, 1., 1e-7));

            float seg_elev_m; pData->Get(1, row, seg_elev_m); elev_m += seg_frac_of_reach * seg_elev_m;
            float seg_width_m; pData->Get(2, row, seg_width_m); width_m += seg_frac_of_reach * seg_width_m;
            float seg_topo_elev_deg;
            pData->Get(3, row, seg_topo_elev_deg); topo_E_deg += seg_frac_of_reach * seg_topo_elev_deg;
            pData->Get(4, row, seg_topo_elev_deg); topo_S_deg += seg_frac_of_reach * seg_topo_elev_deg;
            pData->Get(5, row, seg_topo_elev_deg); topo_W_deg += seg_frac_of_reach * seg_topo_elev_deg;
            double seg_veg_ht_accum_m = 0.;

            for (int direction = 0; direction < 7; direction++) // NE, E, SE, S, SW, W, NW
            {
               float direction_veg_ht_accum_m = 0.;
               double elv_accum_m = 0.;
               for (int j = 0; j < 9; j++)
               {
                  float ht_m;
                  pData->Get(7 + direction * 9 + j, row, ht_m);
                  if (ht_m < pSAL->m_heightThreshold_m) ht_m = (float)pSAL->m_futureHeight_m;
                  direction_veg_ht_accum_m += ht_m;
                  float seg_elv_m = 0.f; pData->Get(70 + direction * 9 + j, row, seg_elv_m);
                  elv_m[direction] += seg_frac_of_reach * seg_elv_m;
               } // end of loop on j, sample position relative to center of stream
               seg_veg_ht_accum_m += direction_veg_ht_accum_m / 9.;
               elv_m[direction] = elv_accum_m / 9.;
            } // end of loop on direction
            reach_veg_ht_m += seg_frac_of_reach * (seg_veg_ht_accum_m / 7.);

         } // end of if (overlap_m > 0.), processing of data for one Shade-a-lator river segment for one reach

         if (segment_downstream_end_km >= reach_downstream_end_km) // (frac_of_reach < 1.)
         { // The downstream end of this segment is within the reach
            ASSERT(below_the_bottom_m == 0.);
            row++;
            ASSERT(row <= dataRows);
            segment_upstream_end_km = segment_downstream_end_km;
            if (row < dataRows) pData->Get(0, row, segment_downstream_end_km);
            else segment_downstream_end_km = segment_upstream_end_km;
         }
         else break; // go on to the next reach without advancing to the next segment.
      } // end of while (segment_upstream_end_km > reach_downstream_end_km && row < data_rows)

      // Now we have reach values for topographic elevations, width, and vegetation height.
      m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachWIDTHGIVEN, width_m);
      double width_min_m = pReach->Att(ReachWIDTH_MIN);
      ASSERT(width_m >= width_min_m);
      if (width_m < width_min_m) width_m = width_min_m;
      m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachWIDTHREACH, width_m);
      ASSERT(width_m > 0.);

      pReach->m_topo.m_topoElevE_deg = topo_E_deg;
      pReach->m_topo.m_topoElevS_deg = topo_S_deg;
      pReach->m_topo.m_topoElevW_deg = topo_W_deg;
      m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachTOPOELEV_E, topo_E_deg);
      m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachTOPOELEV_S, topo_S_deg);
      m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachTOPOELEV_W, topo_W_deg);
      m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachVEG_HT_R, reach_veg_ht_m);
      m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachVEG_HT_L, reach_veg_ht_m);

      ASSERT(close_enough(frac_of_reach, 1., 1e-7));
      ASSERT(row <= dataRows);

      if (pReach->m_reachID == pSAL->m_comid_downstream) pReach = NULL;
      else pReach = GetReachFromNode(pReach->m_pDown);
      if (pReach != NULL)
      {
         reach_upstream_end_km = reach_downstream_end_km;
         reach_downstream_end_km = reach_upstream_end_km - pReach->m_length / 1000.;
      }
   } // end of reach loop: while (pReach != NULL && row < data_rows)

      return(true);

} // end of ReadShade_a_latorInputFile()


bool FlowModel::StartYear( FlowContext *pFlowContext )
   {
   pFlowContext->pEnvContext->m_simDate.MoveToNextDay(pFlowContext->pEnvContext->m_maxDaysInYear);

   if (!OpenClimateDataFiles(m_flowContext.pEnvContext->weatherYear)) return(false);

   m_annualTotalET        = 0; // acre-ft
   m_annualTotalPrecip    = 0; // acre-ft
   m_annualTotalDischarge = 0; //acre-ft
   m_annualTotalRainfall  = 0; // acre-ft
   m_annualTotalSnowfall  = 0; // acre-ft

   m_volumeMaxSWE = 0.0f; // m3 H2O
   m_pIDUlayer->SetColDataU(MAXSNOW, 0.);

   m_totEvapFromReachesYr_m3 = 0.; // m3 H2O
   m_totEvapFromReservoirsYr_m3 = 0.;

   m_pIDUlayer->SetColDataU(m_colSM2ATM_YR, 0.f);

   // Initialize yearly values which accumulate from daily values
   m_pIDUlayer->SetColDataU(m_colPRECIP_YR, 0.f);
   m_pIDUlayer->SetColDataU(m_colET_YR, 0); // Accumulates ET_DAY.
   m_pReachLayer->SetColDataU(m_colReachXFLUX_Y, 0); // Accumulates XFLUX_D.

   GlobalMethodManager::StartYear( &m_flowContext );   // this invokes StartYear() for any internal global methods 
   
   // reset flux accumulators for this year
   for ( int i=0; i < GetFluxInfoCount(); i++ )
      {
      GetFluxInfo( i )->m_totalFluxRate = 0;
      GetFluxInfo( i )->m_annualFlux = 0;
      }

   for (int reach_ndx = 0; reach_ndx < m_reachArray.GetSize(); reach_ndx++)
   {
      Reach * pReach = m_reachArray[reach_ndx];
      pReach->m_addedDischarge_cms = 0;
      pReach->m_reachAddedVolumeWP = WaterParcel(0,0);
      pReach->m_nanOccurred = false;

      pReach->CalcReachVegParamsIfNecessary();

      for (int node_ndx = 0; node_ndx < pReach->GetSubnodeCount(); node_ndx++)
      {
         ReachSubnode * pNode = pReach->GetReachSubnode(node_ndx);
         pNode->m_addedDischarge_cms = 0.;
         pNode->m_addedVolYTD_WP = WaterParcel(0,0);
         pNode->m_nanOccurred = false;
         pNode->m_dischargeDOY = -1;
      } // end of loop thru subnodes
   } // end of loop thru reaches

   ClimateScenario *pScenario = m_scenarioArray.GetAt(m_currentFlowScenarioIndex);

   // reset reservoirs for this year
   for (int i = 0; i < (int)m_reservoirArray.GetSize(); i++)
      {
      Reservoir *pRes = m_reservoirArray[i];
      pRes->m_filled = 0;

      if (pRes->m_probMaintenance>-1)//the reservoir may be, or have been, taken offline
         {
         if (pRes->m_reservoirType == ResType_RiverRun)//it was taken off line last year
            pRes->m_reservoirType = ResType_FloodControl;//reset it back to FloodControl
         float value = (float)m_randUnif1.RandValue(0, 1);
         if (value < pRes->m_probMaintenance)
            {
            pRes->m_reservoirType = ResType_RiverRun;
            CString msg;
            msg.Format("Flow: Reservoir %s will be offline for the next year. Offline Probability %f, rolled %f", (LPCTSTR)pRes->m_name,pRes->m_probMaintenance,value);
            Report::LogMsg(msg);
            }
         else
            {
            CString msg;
            msg.Format("Flow: Reservoir %s will be ONline for the next year. Offline Probability %f, rolled %f", (LPCTSTR)pRes->m_name, pRes->m_probMaintenance, value);
            Report::LogMsg(msg);
            }
         }
      }

   const char* name = "IrrigatedSoil";
   if (m_hruLayerNames.GetSize()>3)
      {
      const char* label = m_hruLayerNames.GetAt(3).GetString();
      if (label[0] == name[0])
         {
         // fix up water transfers between irrigated and unirrigated polygons
         int hruCount = pFlowContext->pFlowModel->GetHRUCount();
         for (int h = 0; h < hruCount; h++)
            {
            HRU *pHRU = pFlowContext->pFlowModel->GetHRU(h);
            double targetIrrigatedArea = 0.0;
            for (int m = 0; m < pHRU->m_polyIndexArray.GetSize(); m++)
               {
               int idu_poly_ndx = pHRU->m_polyIndexArray[m];
               double area = 0.0;
               int irrigated = 0;
               pFlowContext->pEnvContext->pMapLayer->GetData(idu_poly_ndx, pFlowContext->pFlowModel->m_colIrrigation, irrigated);
               pFlowContext->pEnvContext->pMapLayer->GetData(idu_poly_ndx, pFlowContext->pFlowModel->m_colCatchmentArea, area);
               if (irrigated != 0)
                  targetIrrigatedArea += area * pHRU->m_frc_naturl;
               }
            if (targetIrrigatedArea > pHRU->m_HRUeffArea_m2) targetIrrigatedArea = pHRU->m_HRUeffArea_m2;
            double targetNonIrrigatedArea = pHRU->m_HRUeffArea_m2 - targetIrrigatedArea;
            pHRU->GetLayer(BOX_IRRIG_SOIL)->m_HRUareaFraction = (float)(targetIrrigatedArea / pHRU->m_HRUeffArea_m2); 
            pHRU->GetLayer(BOX_NAT_SOIL)->m_HRUareaFraction = 1.f - pHRU->GetLayer(BOX_IRRIG_SOIL)->m_HRUareaFraction;
            if (pHRU->GetLayer(BOX_NAT_SOIL)->m_HRUareaFraction < 0.f) pHRU->GetLayer(BOX_NAT_SOIL)->m_HRUareaFraction = 0.f;
            float nonIrrigatedVolume = (float)pHRU->GetLayer(BOX_NAT_SOIL)->m_volumeWater;
            float irrigatedVolume = (float)pHRU->GetLayer(BOX_IRRIG_SOIL)->m_volumeWater;

            // Reapportion water between irrigated and non-irrigated parts of the HRU at the beginning of each year, 
            // to reflect this year's decisions about which IDUs to irrigate (the output of the irrigation decision model).            if (pFlowContext->pEnvContext->yearOfRun <= 0)
            float totWater = irrigatedVolume + nonIrrigatedVolume;

            pHRU->m_areaIrrigated = targetIrrigatedArea;
            pHRU->m_fracIrrigated = targetIrrigatedArea / pHRU->m_HRUtotArea_m2;
            m_pHRUlayer->SetDataU(h, m_colHruAREA_IRRIG, pHRU->m_areaIrrigated);

            double layer2_volumeWater = pHRU->GetLayer(BOX_NAT_SOIL)->m_HRUareaFraction * totWater;
            pHRU->GetLayer(BOX_NAT_SOIL)->m_volumeWater = layer2_volumeWater; 
            double nat_soil_mm = targetNonIrrigatedArea > 0 ? (1000 * layer2_volumeWater / targetNonIrrigatedArea) : 0;
            pHRU->SetAttFloat(HruNAT_SOIL, (float)nat_soil_mm);

            pHRU->GetLayer(BOX_IRRIG_SOIL)->m_volumeWater = pHRU->GetLayer(BOX_IRRIG_SOIL)->m_HRUareaFraction * totWater;
            double irrig_mm = targetIrrigatedArea > 0 ? (1000 * layer2_volumeWater / targetIrrigatedArea) : 0;
            pHRU->SetAttFloat(HruIRRIG_SOIL, (float)irrig_mm);
         } // end of loop thru HRUs
         } // end of if (label[0] == name[0])
      } // end of if (m_hruLayerNames.GetSize()>3)

   m_StartYear_totH2O_m3 = CalcTotH2O();
   m_StartYear_totReaches_m3 = CalcTotH2OinReaches().m_volume_m3;
   m_StartYear_totReservoirs_m3 = CalcTotH2OinReservoirs().m_volume_m3;
   m_StartYear_totHRUs_m3 = CalcTotH2OinHRUs();

   CString msg;
   WaterParcel totH2OinReachesWP = CalcTotH2OinReaches();
   WaterParcel totH2OinReservoirsWP = CalcTotH2OinReservoirs();
   msg.Format("*** FlowModel::StartYear() CalcTotH2OinReaches() = %f m3, %f degC, %f MJ, CalcTotH2OinReservoirs() = %f m3, %f degC, %f MJ, CalcTotH2OinHRUs = %f m3, CalcTotH2O() = %f m3, m_totArea = %f",
      totH2OinReachesWP.m_volume_m3, totH2OinReachesWP.m_temp_degC, totH2OinReachesWP.ThermalEnergy() / 1000.,
      totH2OinReservoirsWP.m_volume_m3, totH2OinReservoirsWP.m_temp_degC, totH2OinReservoirsWP.ThermalEnergy() / 1000.,
      CalcTotH2OinHRUs(), CalcTotH2O(), m_totArea);
   Report::LogMsg(msg);

   int hruCount = pFlowContext->pFlowModel->GetHRUCount();
   for (int h = 0; h < hruCount; h++)
   {
      HRU * pHRU = m_hruArray[h];
      for (int box_ndx = 0; box_ndx < pHRU->m_layerArray.GetSize(); box_ndx++)
      {
         HRULayer * pBox = pHRU->GetLayer(box_ndx);
         pBox->m_nanOccurred = false;
         pBox->m_addedVolume_m3 = 0;
      } // end of loop thru compartments

      double wetland_area_accum_m2 = 0.;
      pHRU->m_standingH2Oflag = false;
      int num_idus = (int)pHRU->m_polyIndexArray.GetSize();
      for (int idu_in_hru = 0; idu_in_hru < num_idus; idu_in_hru++)
      {
         int idu_poly_ndx = pHRU->m_polyIndexArray[idu_in_hru];
         bool is_wetland = AttInt(idu_poly_ndx, LULC_A) == LULCA_WETLAND;
         if (is_wetland)
         {
            wetland_area_accum_m2 += AttFloat(idu_poly_ndx, AREA);
            double wetness_mm = Att(idu_poly_ndx, WETNESS);
            double flooddepth_mm = Att(idu_poly_ndx, FLOODDEPTH);
            ASSERT(wetness_mm > 0 || flooddepth_mm == 0);
            pHRU->m_standingH2Oflag = pHRU->m_standingH2Oflag || wetness_mm > 0 || flooddepth_mm > 0;
         }
      } // end of loop thru IDUs in this HRU
      pHRU->m_wetlandArea_m2 = wetland_area_accum_m2;
      pHRU->m_snowpackArea_m2 = pHRU->m_HRUtotArea_m2 - pHRU->m_wetlandArea_m2;
   } // end of loop thru HRUs

   return true;
   } // end of FlowModel::StartYear()


bool FlowModel::StartStep( FlowContext *pFlowContext )
   {
   int hruCount = GetHRUCount();
   for (int h = 0; h < hruCount; h++)
      {
      HRU *pHRU = GetHRU( h );
      pHRU->m_currentMaxET  = 0;
      pHRU->m_currentET     = 0;
      pHRU->m_currentRunoff = 0;
      pHRU->m_currentGWFlowOut = 0;
      pHRU->m_currentGWRecharge = 0;
      }

   for (int reach_ndx = 0; reach_ndx < m_reachArray.GetSize(); reach_ndx++)
   {
      Reach* pReach = m_reachArray[reach_ndx];
      int subnode_count = pReach->GetSubnodeCount();
      for (int subnode_ndx = 0; subnode_ndx < subnode_count; subnode_ndx++)
      {
         ReachSubnode* pNode = pReach->GetReachSubnode(subnode_ndx);
         pNode->m_addedVolTodayWP = WaterParcel(0, 0);
      } // end of loop thru subnodes
   } // end of loop thru reaches

   m_pReachLayer->SetColDataU(m_colReachXFLUX_D, 0);
   m_pReachLayer->SetColDataU(m_colReachSPRING_CMS, 0);
   m_pReachLayer->SetColDataU(m_colReachFLOOD_CMS, 0);
   m_pReachLayer->SetColDataU(m_colReachFLOOD_C, 0);

   // Make sure today's weather is loaded into the IDU and HRU layers.
   GetTodaysWeatherField(CDT_PRECIP);
   GetTodaysWeatherField(CDT_TMAX);
   GetTodaysWeatherField(CDT_TMIN);
   GetTodaysWeatherField(CDT_TMEAN);
   GetTodaysWeatherField(CDT_SOLARRAD);
   GetTodaysWeatherField(CDT_SPHUMIDITY);
   GetTodaysWeatherField(CDT_WINDSPEED);

   GlobalMethodManager::StartStep( &m_flowContext );  // Calls any global methods with ( m_timing & GMT_START_STEP ) == true

   SYSDATE today = gpModel->m_flowContext.pEnvContext->m_simDate;
   if (today.month == 1 && today.day == 1) 
      for (int reach_ndx = 0; reach_ndx < m_reachArray.GetSize(); reach_ndx++)
   {
      Reach* pReach = m_reachArray[reach_ndx];
      pReach->CalcReachVegParamsIfNecessary();
   } // end of loop thru reaches

   m_pReachLayer->SetColDataU(m_colReachRADSWGIVEN, 0);
   m_pReachLayer->SetColDataU(m_colReachKCAL_GIVEN, 0);
   Scenario* pSimulationScenario = m_flowContext.pEnvContext->pEnvModel->GetScenario();
   Shade_a_latorData* pSAL = &(pSimulationScenario->m_shadeAlatorData);
   if (pSAL->m_valid)
   {
      SYSDATE current_date = pFlowContext->pEnvContext->m_simDate;
      if (m_flowContext.m_SALmode
         && !DateComesBefore(current_date, pSAL->m_startDate)
         && !DateComesBefore(pSAL->m_endDate, current_date))
      { // Read Shade-a-lator output data files
         FDataObj* pEnergyData = &(pSAL->m_SALenergyOutputData);
         FDataObj* pRadiationData = &(pSAL->m_SALradiationOutputData);

         // Shade-a-lator "Processed_kcal_transposed" files have one row for each stream segment and one column for each day in the study period.
         // The row headers are the river kilometers of the downstream ends of the segments, in order from upstream to downstream.
         // The column headers are dates, for example "2019-11-01"
         // This block of code will read the column for the current date, aggregate the kcal values for the segments 
         // into kJ values for the reaches, and store the values into the Reach layer attribute KCAL_GIVEN.
         //
         // Shade-a-lator "Heat_SR4_transposed" files (the radiation output files) have one row for each stream segment and 
         // one column for each hour of each day in the study period.
         // The row headers are the river kilometers of the downstream ends of the segments, in order from upstream to downstream.
         // The column headers are dates and times, expressed as a day number and a fraction of a day, for example "43922.25"
         // for 0600 4/1/20.
         // This block of code will read the columns for the current date, aggregate the hourly W/m2 values for the segments 
         // both temporally for the current day and spatially into daily W/m2 values for the reaches, 
         // and store the values into the Reach layer attribute RADSWGIVEN.

         int days_since_start_date = DaysBetweenDates(pSAL->m_startDate, current_date);
         int energy_tgt_col = days_since_start_date + 1; // data for start date is in col 1
         ASSERT(energy_tgt_col >= 1 && pEnergyData->GetColCount() > energy_tgt_col);
         int radiation_tgt_col = 1 + 24 * days_since_start_date;
         ASSERT(radiation_tgt_col >= 1 && pRadiationData->GetColCount() > radiation_tgt_col);

         // Get the SAL output data for the current date 
         int data_rows = pEnergyData->GetRowCount();
         Reach* pReach_upstream = pSAL->m_pReach_upstream;
         int reach_ct = pSAL->m_reachCt;

         // "final segment" here means the most upstream segment; its data is in row 0
         // river kilometers increase in the upstream direction
         float downstream_end_of_final_segment_km; pEnergyData->Get(0, 0, downstream_end_of_final_segment_km);
         float downstream_end_of_first_segment_km; pEnergyData->Get(0, data_rows - 1, downstream_end_of_first_segment_km);
         double adj_SAL_len_m = // adjusted Shade-a-lator length in meters = Shade-a-lator length not counting the final segment
            1000. * ((double)downstream_end_of_final_segment_km - (double)downstream_end_of_first_segment_km);

            // Make sure the Shade-a-lator segments overlap the upstream reach entirely.
            // Since we don't know for sure how long the final segment in row 0 is, start with row 1.
         ASSERT(data_rows >= 2);
         int row = 1; // index to Shade-a-lator river segment
         // The upper end of the segment in row 1 is the lower end of the final segment in row 0.
         float segment_upstream_end_km = downstream_end_of_final_segment_km;
         float segment_downstream_end_km; pEnergyData->Get(0, 1, segment_downstream_end_km);

         double reach_upstream_end_km = pSAL->m_reachesLength_m / 1000.; // ??? This only works when the downstream end is at river mile 0.
         Reach* pReach = pReach_upstream;
         while (reach_upstream_end_km > segment_upstream_end_km)
         { // Move downstream by one reach.
            reach_upstream_end_km -= pReach->m_length / 1000.;
            pReach = GetReachFromNode(pReach->m_pDown);
            ASSERT(pReach != NULL);
         }
         double reach_downstream_end_km = reach_upstream_end_km - pReach->m_length / 1000.;
         // Now we have assured that the upstream segment goes at least as far upstream as the current reach.

         // For each reach, average the Shade-a-lator data over all the segments which overlap it.
         double below_the_bottom_m = 0.;
         double reach_kcal = 0.;
         double reach_W_m_accumulator = 0.;
         double total_length_m = 0.;
         float seg_kcal = 0.;
         float seg_W_m2 = 0.;
         double segment_len_m = 0.;
         while (pReach != NULL && row < data_rows)
         {
            double frac_of_reach = 0.;
            reach_kcal = 0.;
            reach_W_m_accumulator = 0.;
            total_length_m = 0.;
            while (segment_upstream_end_km > reach_downstream_end_km && row < data_rows) // frac_of_reach < 1. && row < data_rows)
            {
               segment_len_m = ((double)segment_upstream_end_km - (double)segment_downstream_end_km) * 1000.;
               // What part of this segment overlaps the current reach?
               double over_the_top_m = (segment_upstream_end_km > reach_upstream_end_km) ?
                  (segment_upstream_end_km - reach_upstream_end_km) * 1000. : 0;
               below_the_bottom_m = (segment_downstream_end_km < reach_downstream_end_km) ?
                  (reach_downstream_end_km - segment_downstream_end_km) * 1000. : 0;

               double overlap_m = segment_len_m - over_the_top_m - below_the_bottom_m;
               if (overlap_m < 0.) overlap_m = 0.;

               if (overlap_m > 0.)
               {
                  total_length_m += overlap_m;
                  pEnergyData->Get(energy_tgt_col, row, seg_kcal);
                  double seg_contrib_to_reach_kcal = (overlap_m / segment_len_m) * seg_kcal;
                  reach_kcal += seg_contrib_to_reach_kcal;

                  double seg_W_m2 = 0.;
                  for (int hour = 0; hour < 24; hour++)
                  {
                     float hour_W_m2 = 0.;
                     pRadiationData->Get(radiation_tgt_col + hour, row, hour_W_m2);
                     seg_W_m2 += hour_W_m2;
                  }
                  seg_W_m2 /= 24.;
                  double seg_contrib_to_reach_W_m = overlap_m * seg_W_m2;
                  reach_W_m_accumulator += seg_contrib_to_reach_W_m;

                  frac_of_reach += overlap_m / pReach->m_length;
               }

               if (segment_downstream_end_km >= reach_downstream_end_km) // (frac_of_reach < 1.)
               { // The downstream end of this segment is within the reach
                  ASSERT(below_the_bottom_m == 0.);
                  row++;
                  ASSERT(row <= data_rows);
                  segment_upstream_end_km = segment_downstream_end_km;
                  if (row < data_rows) pEnergyData->Get(0, row, segment_downstream_end_km);
                  else segment_downstream_end_km = segment_upstream_end_km;
               }
               else break; // go on to the next reach without advancing to the next segment.
            } // end of while (segment_upstream_end_km > reach_downstream_end_km && row < data_rows)
            ASSERT(close_enough(frac_of_reach, 1., 1e-7));
            ASSERT(row <= data_rows);

            m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachKCAL_GIVEN, reach_kcal);

            double reach_W_m2 = reach_W_m_accumulator / total_length_m;
            m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachRADSWGIVEN, reach_W_m2);

            Reach * pReach_down = GetReachFromNode(pReach->m_pDown);
            if (pReach->m_reachID == pSAL->m_comid_downstream) pReach_down = NULL;
            if (pReach_down != NULL)
            {
               reach_upstream_end_km = reach_downstream_end_km;
               reach_downstream_end_km = reach_upstream_end_km - pReach_down->m_length / 1000.;
            }
            pReach = pReach_down;
         } // end of while (pReach != NULL && row < data_rows)

      } // end of block to get the SAL output data for the current date
   } // end of if (pSAL->m_valid)

   return(true);
} // end of FlowModel::StartStep()


bool FlowModel::EndStep( FlowContext *pFlowContext )
{
   CArray< VData, VData > activeRules;
   activeRules.SetSize(1 + m_reservoirArray.GetSize());
   int day_ndx = pFlowContext->dayOfYear + (int)m_yearStartTime;
   CString day_str; day_str.Format("%d", day_ndx);
   char * day_str2 = (char *)malloc(day_str.GetLength() + 1);
//   m_DayStrings.Add(day_str2);
   strncpy_s(day_str2, day_str.GetLength() + 1, day_str, day_str.GetLength());
   day_str2[day_str.GetLength()] = 0;
   activeRules[0] = day_str2;
   for (int i = 0; i < (int)m_reservoirArray.GetSize(); i++)
   {
      Reservoir *pRes = m_reservoirArray[i];
      activeRules[1 + i] = pRes->m_activeRule;
   }
   m_ResActiveRuleReport.AppendRow(activeRules);

   // get discharge at pour points
   for ( int i=0; i < this->m_reachTree.GetRootCount(); i++ )
   {
      ReachNode *pRootNode = this->m_reachTree.GetRootNode( i );
      Reach *pReach = GetReachFromNode( pRootNode );
      double discharge = pReach->GetReachDischargeWP().m_volume_m3 / SEC_PER_DAY;   // m3/sec

      // convert to acreft/day
      discharge *= ACREFT_PER_M3 * SEC_PER_DAY;  // convert to acreft-day
      m_annualTotalDischarge += discharge;
   }

   // Make sure today's weather is loaded into the IDU and HRU layers.
   GetTodaysWeatherField(CDT_PRECIP);
   GetTodaysWeatherField(CDT_TMAX);
   GetTodaysWeatherField(CDT_TMIN);
   GetTodaysWeatherField(CDT_TMEAN);
   GetTodaysWeatherField(CDT_SOLARRAD);
   GetTodaysWeatherField(CDT_SPHUMIDITY);
   GetTodaysWeatherField(CDT_WINDSPEED);

   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu < m_pIDUlayer->End(); idu++)
   {
      float rad_sw_yr; m_pIDUlayer->GetData(idu, m_colRAD_SW_YR, rad_sw_yr);
      float rad_sw; m_pIDUlayer->GetData(idu, m_colRAD_SW, rad_sw);
      rad_sw_yr = pFlowContext->dayOfYear > 0 ? ((rad_sw_yr * (pFlowContext->dayOfYear - 1) + rad_sw) / pFlowContext->dayOfYear) : rad_sw;
      m_pIDUlayer->SetDataU(idu, m_colRAD_SW_YR, rad_sw_yr);

      double wetness_mm = Att(idu, WETNESS);
      if (wetness_mm > 0)
      {
         float area_m2 = AttFloat(idu, AREA);
         double wetl_vol_m3 = (wetness_mm / 1000.) * area_m2;
         SetAtt(idu, WETL_VOL, wetl_vol_m3);
      }
      else SetAtt(idu, WETL_VOL, 0);
   } // end of loop thru IDUs

   m_totEvapFromReachesYr_m3 += CalcTotDailyEvapFromReaches();

   for (MapLayer::Iterator reach = m_pReachLayer->Begin(); reach < m_pReachLayer->End(); reach++)
   {
      float xflux_d_cms;
      m_pReachLayer->GetData(reach, m_colReachXFLUX_D, xflux_d_cms);
      if (xflux_d_cms != 0)
      {
         float xflux_y_cms;
         m_pReachLayer->GetData(reach, m_colReachXFLUX_Y, xflux_y_cms);
         double accum_cms_x_days = xflux_y_cms * (double)pFlowContext->dayOfYear;
         xflux_y_cms = (float)((accum_cms_x_days + (double)xflux_d_cms) / ((double)pFlowContext->dayOfYear + 1));
         m_pReachLayer->SetDataU(reach, m_colReachXFLUX_Y, xflux_y_cms);
      }
   } // end of loop thru the Reach layer

   MagicReachWaterReport_m3(false); // Calculate ADDED_VOL and ADDED_Q.

   Scenario* pSimulationScenario = pFlowContext->pEnvContext->pEnvModel->GetScenario();
   Shade_a_latorData* pSAL = &(pSimulationScenario->m_shadeAlatorData);
   if (pSAL->m_valid) DumpReachInsolationData(pSAL);

   return true;
} // end of FlowModel::EndStep()


bool FlowModel::EndYear( FlowContext *pFlowContext )
   {
   m_annualTotalET = 0.0f;
   m_annualTotalPrecip = 0.0f;
   m_annualTotalRainfall = 0.0f;
   m_annualTotalSnowfall = 0.0f;

   for ( int i=0; i < (int) m_hruArray.GetSize(); i++ )
      {
      HRU *pHRU = m_hruArray[ i ];

      ASSERT(pHRU->m_et_yr >= 0.0f && pHRU->m_et_yr <= 1.0E10f);
      m_annualTotalET       += pHRU->m_et_yr       * pHRU->m_HRUeffArea_m2 * M_PER_MM * ACREFT_PER_M3;        // mm/year * area(m2) * m/mm * acreft/m3   = acre-ft
      m_annualTotalPrecip   += pHRU->m_precip_yr   * pHRU->m_HRUeffArea_m2 * M_PER_MM * ACREFT_PER_M3;        // mm/year * area(m2) * m/mm * acreft/m3   = acre-ft
      m_annualTotalRainfall += pHRU->m_rainfall_yr * pHRU->m_HRUeffArea_m2 * M_PER_MM * ACREFT_PER_M3;        // mm/year * area(m2) * m/mm * acreft/m3   = acre-ft
      m_annualTotalSnowfall += pHRU->m_snowfall_yr * pHRU->m_HRUeffArea_m2 * M_PER_MM * ACREFT_PER_M3;        // mm/year * area(m2) * m/mm * acreft/m3   = acre-ft
      }

   m_flowContext.timing = GMT_END_YEAR;
   GlobalMethodManager::EndYear( &m_flowContext );

   float row[ 7 ];
   row[ 0 ] = (float) pFlowContext->pEnvContext->currentYear;
   row[ 1 ] = m_annualTotalET;
   row[ 2 ] = m_annualTotalPrecip;
   row[ 3 ] = (float)m_annualTotalDischarge;
   row[ 4 ] = (float)(m_annualTotalPrecip - ( m_annualTotalET + m_annualTotalDischarge ));
   row[ 5 ] = m_annualTotalRainfall;
   row[ 6 ] = m_annualTotalSnowfall;

   m_pTotalFluxData->AppendRow( row, 7 );

   CString msg;
   WaterParcel totH2OinReachesWP = CalcTotH2OinReaches();
   WaterParcel totH2OinReservoirsWP = CalcTotH2OinReservoirs();
   msg.Format("*** FlowModel::EndYear() CalcTotH2OinReaches() = %f m3, %f degC, %f MJ, CalcTotH2OinReservoirs() = %f m3, %f degC, %f MJ, CalcTotH2OinHRUs = %f m3, CalcTotH2O() = %f m3, m_totArea = %f",
      totH2OinReachesWP.m_volume_m3, totH2OinReachesWP.m_temp_degC, totH2OinReachesWP.ThermalEnergy() / 1000.,
      totH2OinReservoirsWP.m_volume_m3, totH2OinReservoirsWP.m_temp_degC, totH2OinReservoirsWP.ThermalEnergy() / 1000.,
      CalcTotH2OinHRUs(), CalcTotH2O(), m_totArea);
   Report::LogMsg(msg);


   CArray< float, float > rowMetrics;
   rowMetrics.SetSize(10);
   rowMetrics[0] = (float)pFlowContext->pEnvContext->currentYear; 
   rowMetrics[1] = ((float)m_StartYear_totH2O_m3 / m_totArea)*1000.f;
   rowMetrics[2] = ((float)m_StartYear_totReaches_m3 / m_totArea)*1000.f;
   rowMetrics[3] = ((float)m_StartYear_totReservoirs_m3 / m_totArea)*1000.f;
   rowMetrics[4] = ((float)m_StartYear_totHRUs_m3 / m_totArea)*1000.f;
   rowMetrics[5] = ((float)CalcTotH2O() / m_totArea)*1000.f;
   rowMetrics[6] = ((float)CalcTotH2OinReaches().m_volume_m3 / m_totArea)*1000.f;
   rowMetrics[7] = ((float)CalcTotH2OinReservoirs().m_volume_m3 / m_totArea)*1000.f;
   rowMetrics[8] = ((float)CalcTotH2OinHRUs() / m_totArea)*1000.f;
   rowMetrics[9] = rowMetrics[5] - rowMetrics[1];
   this->m_FlowWaterBalanceReport.AppendRow(rowMetrics);

   // Update P_MINUS_ET, ARIDITYNDX, PRCP_Y_AVG, and AET_Y_AVG attributes.
   for (int idu = 0; idu < m_pIDUlayer->GetRowCount(); idu++)
      {
      float precip_yr = 0.f; m_pIDUlayer->GetData(idu, m_colPRECIP_YR, precip_yr);
      float et_yr = 0.f; m_pIDUlayer->GetData(idu, m_colET_YR, et_yr);
      float snowevap_y = 0.f; m_pIDUlayer->GetData(idu, m_colSNOWEVAP_Y, snowevap_y);
      m_pIDUlayer->SetDataU(idu, m_colP_MINUS_ET, precip_yr - (et_yr + snowevap_y));
      float max_et_yr = 0.f; m_pIDUlayer->GetData(idu, m_colMAX_ET_YR, max_et_yr);
      m_pIDUlayer->SetDataU(idu, m_colARIDITYNDX, precip_yr / max_et_yr);

      int num_yrs_in_avg = pFlowContext->pEnvContext->yearOfRun + 1;
      double prcp_y_avg = 0.; m_pIDUlayer->GetData(idu, m_colPRCP_Y_AVG, prcp_y_avg);
      prcp_y_avg = ((num_yrs_in_avg - 1)*prcp_y_avg + precip_yr) / num_yrs_in_avg;
      m_pIDUlayer->SetDataU(idu, m_colPRCP_Y_AVG, prcp_y_avg);
      double aet_y_avg = 0.; m_pIDUlayer->GetData(idu, m_colAET_Y_AVG, aet_y_avg);
      aet_y_avg = ((num_yrs_in_avg - 1)*aet_y_avg + et_yr) / num_yrs_in_avg;
      m_pIDUlayer->SetDataU(idu, m_colAET_Y_AVG, aet_y_avg);
   } // end of loop through IDUs

   MagicReachWaterReport_m3(true); // Report on NaNs and added amounts in reaches.
   MagicHRUwaterReport_m3(true); // Report on NaNs and added amounts in HRUs.

   return true;
} // end of FlowModel::EndYear()


double FlowModel::MagicHRUwaterReport_m3(bool msgFlag) // Report on NaNs and added amounts in HRUs.
{
   int nan_count = 0;
   int added_volume_count = 0;
   float added_volume_tot_m3 = 0;
   int comid_of_largest_added_volume_HRU = -1;
   float largest_added_volume_m3 = -1.;
   for (int hru_ndx = 0; hru_ndx < gpModel->m_hruArray.GetSize(); hru_ndx++)
   {
      HRU* pHRU = m_hruArray[hru_ndx];

      pHRU->m_addedVolume_m3 = 0;
      pHRU->m_nanOccurred = false;
      int box_count = (int)pHRU->m_layerArray.GetSize();
      for (int box_ndx = 0; box_ndx < box_count; box_ndx++)
      {
         HRULayer* pBox = pHRU->m_layerArray[box_ndx];
         pHRU->m_addedVolume_m3 += (float)pBox->m_addedVolume_m3;
         pHRU->m_nanOccurred = pHRU->m_nanOccurred || pBox->m_nanOccurred;
      } // end of loop thru boxs

      if (pHRU->m_nanOccurred) nan_count++;
      if (pHRU->m_addedVolume_m3 > 0)
      {
         added_volume_count++; added_volume_tot_m3 += pHRU->m_addedVolume_m3;
         if (pHRU->m_addedVolume_m3 > largest_added_volume_m3)
         {
            largest_added_volume_m3 = pHRU->m_addedVolume_m3;
            comid_of_largest_added_volume_HRU = pHRU->m_pCatchment->m_pReach->m_reachID;
         }
      }
   } // end of loop thru HRUs

   if (msgFlag)
   {
      CString msg;
      msg.Format("FlowModel::EndYear() HRUs... nan_count = %d, added_volume_count = %d, added_volume_tot_m3 = %f",
         nan_count, added_volume_count, added_volume_tot_m3);
      Report::LogMsg(msg);
      if (nan_count > 0 || added_volume_count > 0 || added_volume_tot_m3 > 0)
      {
         msg.Format("comid_of_largest_added_volume_HRU = %d, largest_added_volume_m3 = %f", comid_of_largest_added_volume_HRU, largest_added_volume_m3);
         if (comid_of_largest_added_volume_HRU > 0) Report::LogMsg(msg);
      }
   }

   return(added_volume_tot_m3);
} // end of MagicHRUwaterReport_m3()
   
   
   double FlowModel::MagicReachWaterReport_m3(bool msgFlag) // Report on NaNs and added amounts in reaches.
// Returns volume of added water in reaches
{
   int nan_count = 0;
   int added_volume_count = 0;
   double added_volume_tot_m3 = 0;
   int added_discharge_count = 0;
   double added_discharge_tot_cms = 0;
   int comid_of_largest_added_volume_reach = -1;
   int comid_of_largest_added_discharge_reach = -1;
   double largest_added_volume_m3 = -1.;
   double largest_added_discharge_cms = -1.;
   for (int reach_ndx = 0; reach_ndx < gpModel->m_reachArray.GetSize(); reach_ndx++)
   {
      Reach* pReach = m_reachArray[reach_ndx];

      pReach->m_addedDischarge_cms = 0;
      WaterParcel reachAddedVolumeThisYearBeforeTodayWP = pReach->m_reachAddedVolumeWP;
      pReach->m_reachAddedVolumeWP = WaterParcel(0, 0); 
      pReach->m_nanOccurred = false;
      int subnode_count = pReach->GetSubnodeCount();
      for (int subnode_ndx = 0; subnode_ndx < subnode_count; subnode_ndx++)
      {
         ReachSubnode* pNode = pReach->GetReachSubnode(subnode_ndx);
         pReach->m_addedDischarge_cms += pNode->m_addedDischarge_cms / pReach->GetSubnodeCount();
         pReach->m_reachAddedVolumeWP.MixIn(pNode->m_addedVolYTD_WP);
         pReach->m_nanOccurred = pReach->m_nanOccurred || pNode->m_nanOccurred;
      } // end of loop thru subnodes

      if (pReach->m_nanOccurred) nan_count++;
      if (pReach->m_reachAddedVolumeWP.m_volume_m3 > 0)
         {
         added_volume_count++; added_volume_tot_m3 += pReach->m_reachAddedVolumeWP.m_volume_m3;
         if (pReach->m_reachAddedVolumeWP.m_volume_m3 > largest_added_volume_m3)
            {
            largest_added_volume_m3 = pReach->m_reachAddedVolumeWP.m_volume_m3;
            comid_of_largest_added_volume_reach = pReach->m_reachID;
         }
      }
      if (pReach->m_addedDischarge_cms > 0)
      {
         added_discharge_count++; added_discharge_tot_cms += pReach->m_addedDischarge_cms;
         if (pReach->m_addedDischarge_cms > largest_added_discharge_cms)
         {
            largest_added_discharge_cms = pReach->m_addedDischarge_cms;
            comid_of_largest_added_discharge_reach = pReach->m_reachID;
         }
      }

      double reachAddedVolumeToday_m3 = pReach->m_reachAddedVolumeWP.m_volume_m3 - reachAddedVolumeThisYearBeforeTodayWP.m_volume_m3;
      m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachADDED_VOL, reachAddedVolumeToday_m3);
      m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachADDEDVOL_C, pReach->m_reachAddedVolumeWP.m_temp_degC);
      m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachADDED_Q, pReach->m_addedDischarge_cms);
   } // end of loop thru reaches

   if  (msgFlag)
   {
      CString msg;
      msg.Format("FlowModel::EndYear() Reaches... nan_count = %d, added_volume_count = %d, added_volume_tot_m3 = %f, added_discharge_count = %d, added_discharge_tot_cms = %f (%f m3)",
         nan_count, added_volume_count, added_volume_tot_m3, added_discharge_count, added_discharge_tot_cms, added_discharge_tot_cms * SEC_PER_DAY);
      Report::LogMsg(msg);
      if (nan_count > 0 || added_volume_count > 0 || added_volume_tot_m3 > 0 || added_discharge_count > 0 || added_discharge_tot_cms > 0)
      {
         msg.Format("comid_of_largest_added_volume_reach = %d, largest_added_volume_m3 = %f", comid_of_largest_added_volume_reach, largest_added_volume_m3);
         if (comid_of_largest_added_volume_reach > 0) Report::LogMsg(msg);
         msg.Format("comid_of_largest_added_discharge_reach = %d, largest_added_discharge_cms = %f", comid_of_largest_added_discharge_reach, largest_added_discharge_cms);
         if (comid_of_largest_added_discharge_reach > 0) Report::LogMsg(msg);
      }
   } // end of if (msgFlag)
   
   return(added_volume_tot_m3);
} // end of MagicReachWaterReport()


   double FlowModel::CalcTotDailyEvapFromReaches() // Returns m3 H2O
   {
      int reachCount = GetReachCount();
      double tot_reach_evap_m3 = 0.;

      for (int reach_ndx = 0; reach_ndx < reachCount; reach_ndx++)
      {
         Reach* pReach = m_reachArray[reach_ndx];
         tot_reach_evap_m3 += pReach->m_reach_evap_m3;
      } // end of loop thru reaches

      return(tot_reach_evap_m3);
   } // end of double CalcTotDailyEvapFromReaches()


WaterParcel FlowModel::CalcTotH2OinReaches() 
{
   int reachCount = GetReachCount();
   WaterParcel totReachH2O_WP(0, 0);

   for (int reach_ndx = 0; reach_ndx < reachCount; reach_ndx++)
   {
      Reach *pReach = m_reachArray[reach_ndx];
      WaterParcel reachH2O_WP(0, 0);
      for (int j = 0; j < pReach->m_subnodeArray.GetSize(); j++)
      {
         ReachSubnode *pNode = pReach->GetReachSubnode(j);
         reachH2O_WP.MixIn(pNode->m_waterParcel);
       }
      totReachH2O_WP.MixIn(reachH2O_WP);
   } // end of loop thru reaches

   return(totReachH2O_WP);
} // end of CalcTotH2OinReaches()


WaterParcel FlowModel::CalcTotH2OinReservoirs() 
{
   WaterParcel totReservoirH2O_WP(0, 0);
   int reservoirCount = (int)m_reservoirArray.GetSize();
   int numNonzeroReservoirs = 0;

   for (int i = 0; i < reservoirCount; i++)
   {
      Reservoir *pRes = m_reservoirArray[i];
      if (pRes!=NULL && pRes->m_in_use && pRes->m_volume > 0.f)
      {
         totReservoirH2O_WP.MixIn(pRes->m_resWP); 
         numNonzeroReservoirs++;
      }
   }
   return(totReservoirH2O_WP);
} // end of CalcTotH2OinReservoirs()


double FlowModel::CalcTotH2OinHRUs()
{
   double totH2O_m3 = 0.;
   for (int hru_ndx = 0; hru_ndx < GetHRUCount(); hru_ndx++)
   {
      HRU * pHRU = GetHRU(hru_ndx);
      double totH2OinHRU_m3 = 0.;
      for (int lyr_ndx = 0; lyr_ndx < pHRU->GetLayerCount(); lyr_ndx++) totH2OinHRU_m3 += pHRU->GetLayer(lyr_ndx)->m_volumeWater;
      totH2O_m3 += totH2OinHRU_m3;
   } // end of loop thru HRUs

   return(totH2O_m3);
} // end of CalcTotH2OinHRUs()

double FlowModel::CalcTotH2O()
{
   double totH2O_m3 = CalcTotH2OinHRUs();
   totH2O_m3 += CalcTotH2OinReaches().m_volume_m3;
   totH2O_m3 += CalcTotH2OinReservoirs().m_volume_m3;

   return(totH2O_m3);
} // end of CalcTotH2O()


// If estimating parameters, then we need to calculate some error, save the current
// parameters and resample a new set of parameters for the next model run.
// 
// When the model is finished for the last year of the current simulation, evaluate that model run over its entire period.
// if we are not in the last year, wait till the next Envision timestep
void FlowModel::UpdateMonteCarloOutput(EnvContext *pEnvContext, int runNumber)
   { 
   //Get the number of locations with measured values
   int numMeas=0;
   for ( int j=0; j < (int) m_modelOutputGroupArray.GetSize(); j++ )
      {
      ModelOutputGroup *pGroup = m_modelOutputGroupArray[ j ];

      for ( int i=0; i < (int) pGroup->GetSize(); i++ )
         {
         if (pGroup->GetAt( i )->m_inUse)
            {
            ModelOutput *pOutput = pGroup->GetAt( i );
            if (pOutput->m_pDataObjObs!=NULL && pOutput->m_queryStr)
               numMeas++;
            }
         }
      }
   //Get the objective function for each of the locations with measured values
   float *ns_=new float[numMeas];
   int c=0;
   float ns = -1.0f; float nsLN = -1.0f; float ve = 0.0f;
   for ( int j=0; j < (int) m_modelOutputGroupArray.GetSize(); j++ )
      {
      ModelOutputGroup *pGroup = m_modelOutputGroupArray[ j ];

      for ( int i=0; i < (int) pGroup->GetSize(); i++ )
         {
         if (pGroup->GetAt( i )->m_inUse)
            {
            ModelOutput *pOutput = pGroup->GetAt( i );
            if (pOutput->m_pDataObjObs!=NULL && pGroup->m_pDataObj!=NULL)
               {
               ns_[c] = GetObjectiveFunction(pGroup->m_pDataObj, ns, nsLN, ve);//this will include time meas model
               c++;
               }
            }
         }
      }
   //Get the highest value for the objective function
   float maxNS=-999999.0f;
   for (int i=0;i<numMeas;i++)
      if (ns_[i]>maxNS)
         maxNS=ns_[i];
   delete [] ns_;
   //Add objective functions and time series data to the dataobjects
   int count=0;
   for ( int j=0; j < (int) m_modelOutputGroupArray.GetSize(); j++ )
      {
      ModelOutputGroup *pGroup = m_modelOutputGroupArray[ j ];

      for ( int i=0; i < (int) pGroup->GetSize(); i++ )
         {
         if (pGroup->GetAt( i )->m_inUse)
            {
            ModelOutput *pOutput = pGroup->GetAt( i );
            if (pOutput->m_pDataObjObs!=NULL && pGroup->m_pDataObj!=NULL)
               {             
               FDataObj *pObj = m_mcOutputTables.GetAt(count);//objective
               count++;
               FDataObj *pData = m_mcOutputTables.GetAt(count);//timeseries
               count++;

               if (pEnvContext->run==0)//add the measured data to the mc measured file
                  {
                  pData->SetSize(2,pGroup->m_pDataObj->GetRowCount());
                  for (int i=0;i<pData->GetRowCount();i++)
                     {
                     pData->Set(0,i,i);//time
                     pData->Set(1,i,pGroup->m_pDataObj->Get(2,i));//measured value
                     }
                  }
                if (maxNS > m_nsThreshold)
                     {
                     pData->AppendCol("q");
                     for (int i=0;i<pData->GetRowCount();i++)
                        pData->Set(pData->GetColCount()-1,i, pGroup->m_pDataObj->Get(1,i));
                     float ns = -1.0f; float nsLN = -1.0f; float ve = -1.0f;
                     GetObjectiveFunction(pGroup->m_pDataObj, ns, nsLN, ve);//this will include time meas model
                     float *nsArray = new float[4];
                     nsArray[0] = (float) pEnvContext->run;
                     nsArray[1] = ns;
                     nsArray[2] = nsLN;
                     nsArray[3] = ve;
                     pObj->AppendRow( nsArray, 4 );
                     delete [] nsArray;
                    }
                }
            }
         }
      }
   //if the simulation wasn't behavioral for any of the measurement points, then remove that parameter set from the parameter array
   if (maxNS < m_nsThreshold)
      m_pParameterData->DeleteRow(m_pParameterData->GetRowCount()-1);
   CString path = PathManager::GetPath(3);
   // write the results to the disk
   if (fmod((double)pEnvContext->run,m_saveResultsEvery) < 0.01f)
      {  
      count=0;
      for ( int j=0; j < (int) m_modelOutputGroupArray.GetSize(); j++ )
         {
         ModelOutputGroup *pGroup = m_modelOutputGroupArray[ j ];

         for ( int i=0; i < (int) pGroup->GetSize(); i++ )
            {
            if (pGroup->GetAt( i )->m_inUse)
               {
               ModelOutput *pOutput = pGroup->GetAt( i );
               if (pOutput->m_pDataObjObs!=NULL && pGroup->m_pDataObj!=NULL)
                  {   
                  CString fileOut;
              
                  if ( m_rnSeed >= 0 )
                     fileOut.Format("%s\\output\\%sObj_%i.csv", (LPCTSTR) path,pGroup->m_name, (int) m_rnSeed );
                  else
                     fileOut.Format("%s\\output\\%sObj.csv", (LPCTSTR)path, (LPCTSTR)pGroup->m_name);
                  m_mcOutputTables.GetAt(count)->WriteAscii(fileOut);

                  count++;

                  if ( m_rnSeed >= 0 )
                    fileOut.Format("%s\\output\\%sObs_%i.csv", (LPCTSTR)path, (LPCTSTR)pGroup->m_name, (int)m_rnSeed);
                  else
                    fileOut.Format("%s\\output\\%sObs.csv", (LPCTSTR)path, (LPCTSTR)pGroup->m_name);
                  
                  m_mcOutputTables.GetAt(count)->WriteAscii(fileOut); 
                  count++;

                 if (m_rnSeed >= 0)
                    fileOut.Format("%s\\output\\%sparam_%i.csv", (LPCTSTR)path, pGroup->m_name, (int)m_rnSeed);
                 else
                    fileOut.Format("%s\\output\\%sparam.csv", (LPCTSTR)path, pGroup->m_name);

                 m_pParameterData->WriteAscii(fileOut);

                 }
               if (pEnvContext->run == 0)//for the first run, look for a climate data object and write it to a file.  
                  {
                  CString fileOut; CString name;
                  if (pGroup->m_pDataObj != NULL && pGroup->m_name == "Climate")
                     {
                     for (int j = 0; j < (int)m_modelOutputGroupArray.GetSize(); j++)//search the output array for a group that has a descriptive (place) m_name
                        {
                        ModelOutputGroup *pGroupName = m_modelOutputGroupArray[j];
                        for (int i = 0; i < (int)pGroupName->GetSize(); i++)
                           {
                           if (pGroupName->GetAt(i)->m_inUse)
                              {
                              ModelOutput *pOutput = pGroupName->GetAt(i);
                              if (pOutput->m_pDataObjObs != NULL && pGroupName->m_pDataObj != NULL)
                                 name = pGroupName->m_name;
                              continue;
                              }
                           }
                        }
                     fileOut.Format("%s\\output\\%sClimate.csv", (LPCTSTR)path, name);
                     pGroup->m_pDataObj->WriteAscii(fileOut);
                     }
                  }
              }
           }
        }
     }

   CString msg;
   msg.Format( "Flow: Results for Runs 1 to %i written to disk.  Check directory %soutput", pEnvContext->run+1, (LPCTSTR) path );
   Report::LogMsg( msg );
   }


void FlowModel::UpdateMonteCarloInput(EnvContext *pEnvContext,int runNumber)
   { 
   //update storage arrays (parameters, error  and also (perhaps...)modeled discharge)
   int paramCount =  (int) m_parameterArray.GetSize();
   float *paramValueData = new float[ paramCount+1 ];

   paramValueData[0] = (float) runNumber;

   for (int i=0; i < paramCount; i++)
      {
      ParameterValue *pParam = m_parameterArray.GetAt(i);

      float value= (float) m_randUnif1.RandValue(pParam->m_minValue,pParam->m_maxValue);
      
      //we have a value, and need to update the table with it.
      ParamTable *pTable = GetTable( pParam->m_table );  
      int col_name = pTable->GetFieldCol( pParam->m_name ); 
      float oldValue=0.0f;

      FDataObj *pTableData = (FDataObj*)pTable->GetDataObj();
      FDataObj *pInitialTableData = (FDataObj*)pTable->GetInitialDataObj();
      for (int j=0; j < pTableData->GetRowCount(); j++)
         {
         float oldValue = pInitialTableData->Get(col_name,j);
         float newValue = oldValue*value;
         if (pParam->m_distribute)
            pTableData->Set(col_name,j,newValue);   
         else
            pTableData->Set(col_name,j,value);  
         }

      paramValueData[i+1]=value;
      }

   m_pParameterData->AppendRow(paramValueData, int( m_parameterArray.GetSize())+1);
   delete [] paramValueData;
    
   }

bool FlowModel::InitHRULayers(EnvContext* pEnvContext)
{
   //MapLayer * pHRUlayer = (MapLayer *)pEnvContext->pHRUlayer;
   int col_hruHRU_ID = -1; m_pHRUlayer->CheckCol(col_hruHRU_ID, "HRU_ID", TYPE_INT, CC_MUST_EXIST);
   int col_hruCATCH_ID = -1; m_pHRUlayer->CheckCol(col_hruCATCH_ID, "CATCH_ID", TYPE_INT, CC_AUTOADD);
   int col_hruCATCH_NDX = -1; m_pHRUlayer->CheckCol(col_hruCATCH_NDX, "CATCH_NDX", TYPE_INT, CC_AUTOADD);
   int col_hruNDX_IN_C = -1; m_pHRUlayer->CheckCol(col_hruNDX_IN_C, "NDX_IN_C", TYPE_INT, CC_AUTOADD);
   bool HRUlayer_readOnly = m_pHRUlayer->m_readOnly; m_pHRUlayer->m_readOnly = false;

   int col_hruFRC_NATURL = -1; m_pHRUlayer->CheckCol(col_hruFRC_NATURL, "FRC_NATURL", TYPE_DOUBLE, CC_AUTOADD);
   int col_hruAREA_AC = -1; m_pHRUlayer->CheckCol(col_hruAREA_AC, "AREA_AC", TYPE_DOUBLE, CC_AUTOADD);
   int col_hruAREA_M2 = -1; m_pHRUlayer->CheckCol(col_hruAREA_M2, "AREA_M2", TYPE_DOUBLE, CC_AUTOADD);
   int catchmentCount = (int) m_catchmentArray.GetSize();
   int hruLayerCount = GetHRULayerCount();

   bool use_attribute_values = !pEnvContext->spinupFlag && IsICfileAvailable();
   CString msg;
   msg.Format("InitHRULayers() use_attribute_values = %d", use_attribute_values);
   Report::LogMsg(msg);

   for ( int i=0; i < catchmentCount; i++ )
   {
      Catchment *pCatchment = m_catchmentArray[ i ];
      ASSERT( pCatchment != NULL );

      int hruCount = pCatchment->GetHRUCountInCatchment();
      for ( int h=0; h < hruCount; h++ )
      {
         HRU *pHRU = pCatchment->GetHRUfromCatchment( h );
         ASSERT( pHRU != NULL );

         HRULayer* pBox_snow = pHRU->GetLayer(BOX_SNOW);
         HRULayer* pBox_nat_soil = pHRU->GetLayer(BOX_NAT_SOIL);
         HRULayer* pBox_irrig_soil = pHRU->GetLayer(BOX_IRRIG_SOIL);

         // Initialize m_frc_naturl to 1 and m_HRUeffArea_m2 to m_HRUtotArea_m2.  SWMM may calculate different values later. 
         pHRU->m_frc_naturl = 1.;
         pHRU->m_HRUeffArea_m2 = pHRU->m_HRUtotArea_m2;

         int hru_row = m_pHRUlayer->FindIndex(col_hruHRU_ID, pHRU->m_id);
         // Set FRC_NATURL, CATCH_NDX, NDX_IN_C, AREA_M2, and AREA_AC attributes in the HRU layer.
         m_pHRUlayer->SetData(hru_row, col_hruFRC_NATURL, pHRU->m_frc_naturl);
         m_pHRUlayer->SetData(hru_row, col_hruCATCH_NDX, i);
         m_pHRUlayer->SetData(hru_row, col_hruCATCH_ID, pCatchment->m_id);
         m_pHRUlayer->SetData(hru_row, col_hruNDX_IN_C, h);
         m_pHRUlayer->SetData(hru_row, col_hruAREA_M2, pHRU->m_HRUtotArea_m2);
         double area_ac = pHRU->m_HRUtotArea_m2 / M2_PER_ACRE;
         m_pHRUlayer->SetData(hru_row, col_hruAREA_AC, area_ac);

         int hbvcalib = pHRU->AttInt(HruHBVCALIB);
// We should only need these next few lines temporarily 4/18/21
         int hru_comid = pHRU->AttInt(HruCOMID);
         Reach* pReach = gFlowModel->FindReachFromID(hru_comid);
         hbvcalib = pReach->AttInt(ReachHBVCALIB);
         pHRU->SetAttInt(HruHBVCALIB, hbvcalib);
// end of temporary addition 4/18/21

         ParamTable* pHBVparams = GetTable("HBV");
         if (pHBVparams == NULL)
         {
            CString msg;
            msg.Format("InitHRULayers() pHBVparams is NULL. This can happen when the HBV CSV file is missing or is present, but open in Excel.");
            Report::ErrorMsg(msg);
            return(false);
         }

         int col_fc = pHBVparams->GetFieldCol("FC");
         float field_cap_mm = 0.;
         pHBVparams->Lookup(hbvcalib, col_fc, field_cap_mm);
         pHRU->SetAttFloat(HruFIELD_CAP, field_cap_mm);

         if (use_attribute_values)
         {
            double sm_day_accum_m3 = 0., snow_swe_accum_m3 = 0., h2o_melt_accum_m3 = 0.;
            double positive_wetness_accum_m3 = 0., negative_wetness_accum_m3 = 0., wetland_area_accum_m2 = 0.;
            double area_accum_m2 = 0.;
            for (int idu_HRU_ndx = 0; idu_HRU_ndx < pHRU->m_polyIndexArray.GetSize(); idu_HRU_ndx++)
            {
               int idu_poly_ndx = pHRU->m_polyIndexArray[idu_HRU_ndx];
               float idu_area_m2 = AttFloat(idu_poly_ndx, AREA);
               area_accum_m2 += idu_area_m2;

               double snow_swe_mm = Att(idu_poly_ndx, SNOW_SWE); snow_swe_accum_m3 += (snow_swe_mm / 1000.) * idu_area_m2;
               double h2o_melt_mm = Att(idu_poly_ndx, H2O_MELT); h2o_melt_accum_m3 += (h2o_melt_mm / 1000.) * idu_area_m2;
               double wetness_mm = Att(idu_poly_ndx, WETNESS);
               int lulc_a = AttInt(idu_poly_ndx, LULC_A);
               if (lulc_a != LULCA_WETLAND)
               { // This is not a wetland IDU.
                 if (wetness_mm != NON_WETLAND_WETNESS_TOKEN)
                  {
                     CString msg;
                     msg.Format("InitHRULayers() idu_poly_ndx = %d, lulc_a = %d, wetness = %lf"
                        " Setting WETNESS to NON_WETLAND_WETNESS_TOKEN now.", idu_poly_ndx, lulc_a, wetness_mm);
                     Report::WarningMsg(msg);
                     SetAtt(idu_poly_ndx, WETNESS, NON_WETLAND_WETNESS_TOKEN);
                  }

                  float sm_day_mm = AttFloat(idu_poly_ndx, SM_DAY); sm_day_accum_m3 += (sm_day_mm / 1000.) * idu_area_m2;
               }
               else
               { // This is a wetland IDU.
                  if (wetness_mm <= NON_WETLAND_WETNESS_TOKEN)
                  {
                     CString msg;
                     msg.Format("InitHRULayers() idu_poly_ndx = %d, lulc_a = %d, wetness = %lf"
                        " Setting WETNESS to 0. now.", idu_poly_ndx, lulc_a, wetness_mm);
                     Report::WarningMsg(msg);
                     SetAtt(idu_poly_ndx, WETNESS, 0.);
                  }

                  wetland_area_accum_m2 += idu_area_m2;
                  if (wetness_mm > 0.) positive_wetness_accum_m3 += (wetness_mm / 1000.) * idu_area_m2;
                  else if (wetness_mm < 0.) negative_wetness_accum_m3 += (wetness_mm / 1000.) * idu_area_m2;
               }
            } // end of loop through IDUs
            ASSERT(close_enough(area_accum_m2, pHRU->m_HRUtotArea_m2, 0.0001));

            // hruSNOW_BOX doesn't include melt water, and is a depth over the entire HRU area
            double hru_snow_box_mm = ((snow_swe_accum_m3 - h2o_melt_accum_m3) / pHRU->m_HRUtotArea_m2) * 1000.; 
//x            HRULayer* pBox_snow = pHRU->GetLayer(BOX_SNOW);
            pHRU->SetAtt(HruSNOW_BOX, hru_snow_box_mm);

            double surface_h2o_m3 = h2o_melt_accum_m3 + positive_wetness_accum_m3;
            HRULayer* pBox_surface_h2o = pHRU->GetLayer(BOX_SURFACE_H2O); pBox_surface_h2o->m_volumeWater = surface_h2o_m3;
            pHRU->SetAtt(HruBOXSURF_M3, surface_h2o_m3);

            double wetland_field_cap_m3 = (field_cap_mm / 1000.) * wetland_area_accum_m2;
            double wetland_nat_soil_h2o_m3 = wetland_field_cap_m3 + negative_wetness_accum_m3;
            double nat_soil_h2o_m3 = sm_day_accum_m3 + wetland_nat_soil_h2o_m3;
            HRULayer* pBox_nat_soil = pHRU->GetLayer(BOX_NAT_SOIL); pBox_nat_soil->m_volumeWater = nat_soil_h2o_m3;
            float nat_soil_h2o_mm = (float)(1000. * nat_soil_h2o_m3 / pHRU->m_HRUtotArea_m2);
            pHRU->SetAttFloat(HruNAT_SOIL, nat_soil_h2o_mm);

            HRULayer* pBox_irrig_soil = pHRU->GetLayer(BOX_IRRIG_SOIL); pBox_irrig_soil->m_volumeWater = 0.;
            pHRU->SetAttFloat(HruIRRIG_SOIL, 0.f);

            float gw_fastbox_mm = pHRU->AttFloat(HruGW_FASTBOX);
            double gw_fastbox_m3 = (gw_fastbox_mm / 1000.) * pHRU->m_HRUeffArea_m2;
            HRULayer* pBox_gw_fast = pHRU->GetLayer(BOX_FAST_GW); pBox_gw_fast->m_volumeWater = gw_fastbox_m3;

            float gw_slowbox_mm = pHRU->AttFloat(HruGW_SLOWBOX);
            double gw_slowbox_m3 = (gw_slowbox_mm / 1000.) * pHRU->m_HRUeffArea_m2;
            HRULayer* pBox_gw_slow = pHRU->GetLayer(BOX_SLOW_GW); pBox_gw_slow->m_volumeWater = gw_slowbox_m3;

            pHRU->m_snowpackFlag = snow_swe_accum_m3 > 0.;
            pHRU->m_standingH2Oflag = positive_wetness_accum_m3 > 0.;
         } // end of if (use_IDU_layer_values)
//         else Spinup or no IC file: Initialization gets done in InitializeSpinup()

         for (int l = 0; l < hruLayerCount; l++)
         {
            HRULayer* pBox = pHRU->GetLayer(l);
            pBox->m_contributionToReach = 0.0f;
            pBox->m_verticalDrainage = 0.0f;
            pBox->m_horizontalExchange = 0.0f;

            for (int k = 0; k < m_hruSvCount - 1; k++)
            {
               pBox->m_svArray[k] = 0.0f;//concentration = 1 kg/m3 ???????
               pBox->m_svArrayTrans[k] = 0.0f;//concentration = 1 kg/m3
            } // end of loop thru state variables in this layer
         } // end of loop thru layers in this HRU

         if (!use_attribute_values) SetAttributesFromHRUmemberData(pHRU);
      } // end of loop thru HRUs in this catchment
   } // end of loop thru catchments

   m_pHRUlayer->m_readOnly = HRUlayer_readOnly;

   return(true);
} // end of InitHRULayers()


void FlowModel::SetAttributesFromHRUmemberData(HRU* pHRU)
{
   double hru_area_m2 = pHRU->m_HRUtotArea_m2;

   float gw_fastbox_mm = (float)(1000. * (pHRU->GetLayer(BOX_FAST_GW)->m_volumeWater / hru_area_m2));
   pHRU->SetAttFloat(HruGW_FASTBOX, gw_fastbox_mm);

   float gw_slowbox_mm = (float)(1000. * (pHRU->GetLayer(BOX_SLOW_GW)->m_volumeWater / hru_area_m2));
   pHRU->SetAttFloat(HruGW_SLOWBOX, gw_slowbox_mm);

   HRULayer* pHRULayer_snowpack = pHRU->GetLayer(BOX_SNOWPACK);
   double hru_snowpack_m3swe = pHRULayer_snowpack->m_volumeWater;
   double hru_snowpack_mmswe = (hru_snowpack_m3swe / hru_area_m2) * 1000.;
   pHRU->m_snowpackFlag = hru_snowpack_mmswe > 0.;

   HRULayer* pHRULayer_surface_h2o = pHRU->GetLayer(BOX_SURFACE_H2O);
   double hru_surface_h2o_m3 = pHRULayer_surface_h2o->m_volumeWater;
   double hru_surface_h2o_mm = (hru_surface_h2o_m3 / hru_area_m2) * 1000.;
   pHRU->m_standingH2Oflag = false;

   HRULayer* pHRULayer_nat_soil = pHRU->GetLayer(BOX_NAT_SOIL);
   HRULayer* pHRULayer_irrig_soil = pHRU->GetLayer(BOX_IRRIG_SOIL);
   double hru_topsoil_h2o_m3 = pHRULayer_nat_soil->m_volumeWater + pHRULayer_irrig_soil->m_volumeWater;
   double hru_topsoil_h2o_mm = (hru_topsoil_h2o_m3 / hru_area_m2) * 1000.;

   int num_idus_in_hru = (int)pHRU->m_polyIndexArray.GetSize();
   double idu_area_accum_m2 = 0.;
   for (int idu_ndx_in_hru = 0; idu_ndx_in_hru < num_idus_in_hru; idu_ndx_in_hru++)
   {
      int idu_poly_ndx = pHRU->m_polyIndexArray[idu_ndx_in_hru];
      float idu_area_m2 = AttFloat(idu_poly_ndx, AREA);
      idu_area_accum_m2 += idu_area_m2;

      double wetness_mm = 0., h2o_melt_mm = 0.;
      if (hru_surface_h2o_mm > 0.)
      {
         int lulc_a = AttInt(idu_poly_ndx, LULC_A);
         if (lulc_a == LULCA_WETLAND)
         { // Wetland IDU and there is surface water => standing water.
            wetness_mm = hru_surface_h2o_mm;
            pHRU->m_standingH2Oflag = true;
         } // end of if (lulc_a == LULCA_WETLAND)
         else h2o_melt_mm = hru_surface_h2o_mm;
      } // end of if (hru_surface_h2o_mm > 0.)

      SetAtt(idu_poly_ndx, SNOW_SWE, hru_snowpack_mmswe + hru_surface_h2o_mm);
      SetAtt(idu_poly_ndx, H2O_MELT, h2o_melt_mm);
      SetAtt(idu_poly_ndx, WETNESS, wetness_mm);
      SetAtt(idu_poly_ndx, SM_DAY, hru_topsoil_h2o_mm);
   } // end of loop thru IDUs in this HRU
   ASSERT(close_enough(hru_area_m2, idu_area_accum_m2, 0.0001));

} // end of SetAttributesFromHRUmemberData()


bool FlowModel::CheckHRUwaterBalance(HRU* pHRU)
{
   double hru_area_m2 = pHRU->m_HRUtotArea_m2;

   HRULayer* pHRULayer_snowpack = pHRU->GetLayer(BOX_SNOWPACK);
   double hru_snowpack_m3swe = pHRULayer_snowpack->m_volumeWater;

   HRULayer* pHRULayer_surface_h2o = pHRU->GetLayer(BOX_SURFACE_H2O);
   double hru_surface_h2o_m3 = pHRULayer_surface_h2o->m_volumeWater;

   HRULayer* pHRULayer_nat_soil = pHRU->GetLayer(BOX_NAT_SOIL);
   HRULayer* pHRULayer_irrig_soil = pHRU->GetLayer(BOX_IRRIG_SOIL);
   double hru_topsoil_h2o_m3 = pHRULayer_nat_soil->m_volumeWater + pHRULayer_irrig_soil->m_volumeWater;
   double hru_topsoil_h2o_mm = (hru_topsoil_h2o_m3 / hru_area_m2) * 1000.;

   int num_idus_in_hru = (int)pHRU->m_polyIndexArray.GetSize();
   double idu_area_accum_m2 = 0.;
   double snow_swe_accum_m3 = 0.;
   double standing_h2o_accum_m3 = 0.;
   double surface_h2o_accum_m3 = 0.;
   double melt_h2o_accum_m3 = 0.;
   double sm_day_accum_m3 = 0.;
   for (int idu_ndx_in_hru = 0; idu_ndx_in_hru < num_idus_in_hru; idu_ndx_in_hru++)
   {
      int idu_poly_ndx = pHRU->m_polyIndexArray[idu_ndx_in_hru];
      float idu_area_m2 = AttFloat(idu_poly_ndx, AREA);
      int lulc_a = AttInt(idu_poly_ndx, LULC_A);
      double idu_snow_swe_mmSWE = Att(idu_poly_ndx, SNOW_SWE);
      double idu_melt_h2o_mm = Att(idu_poly_ndx, H2O_MELT);
      double idu_wetness_mm = Att(idu_poly_ndx, WETNESS);
      double idu_flooddepth_mm = Att(idu_poly_ndx, FLOODDEPTH);
      float sm_day_mm = AttFloat(idu_poly_ndx, SM_DAY);

      idu_area_accum_m2 += idu_area_m2;

      double sm_day_m3 = (sm_day_mm / 1000.) * idu_area_m2;
      sm_day_accum_m3 += sm_day_m3;

      double idu_standing_h2o_m3 = 0., idu_melt_h2o_m3 = 0.;
      bool is_wetland = lulc_a == LULCA_WETLAND;
      if (is_wetland)
      {
         double idu_standing_h2o_mm = idu_flooddepth_mm + max(idu_wetness_mm, 0);
         idu_standing_h2o_m3 = (idu_standing_h2o_mm / 1000) * idu_area_m2;
         standing_h2o_accum_m3 += idu_standing_h2o_m3;
         ASSERT((idu_snow_swe_mmSWE == 0.) && (idu_melt_h2o_mm == 0.));
      }
      else
      {
         ASSERT(idu_snow_swe_mmSWE >= 0.);
         double idu_snow_swe_m3 = (idu_snow_swe_mmSWE / 1000.) * idu_area_m2;
         snow_swe_accum_m3 += idu_snow_swe_m3;
         idu_melt_h2o_m3 = (idu_melt_h2o_mm / 1000.) * idu_area_m2;
         melt_h2o_accum_m3 += idu_melt_h2o_m3;
         ASSERT(idu_wetness_mm == NON_WETLAND_WETNESS_TOKEN);
      }

      ASSERT((idu_standing_h2o_m3 == 0.) || (idu_melt_h2o_m3 == 0.));
      surface_h2o_accum_m3 += (idu_standing_h2o_m3 + idu_melt_h2o_m3);
   } // end of loop thru idus for this hru
   
   bool rtnval = close_enough(hru_area_m2, idu_area_accum_m2, 0.0001, 0.01)
      && close_enough(snow_swe_accum_m3 - melt_h2o_accum_m3, hru_snowpack_m3swe, 0.0001, 0.01)
      && close_enough(surface_h2o_accum_m3, hru_surface_h2o_m3, 0.0001, 0.01)
      && close_enough(sm_day_accum_m3, hru_topsoil_h2o_m3, 0.0001, 0.01);
   ASSERT(rtnval);
   rtnval = rtnval && CheckSurfaceH2O(pHRU, 0.);

   if (!rtnval)
   {
      int hru_id = pHRU->AttInt(HruHRU_ID);
      CString msg;
      msg.Format("CheckHRUwaterBalance() HRU_ID = %d water volume doesn't balance with water in its IDUs.",
         hru_id);
      Report::WarningMsg(msg);
   }

   return(rtnval);
} // end of CheckHRUwaterBalance()


//bool FlowModel::SolveReachDirect( void )
//   {
//   switch( m_reachSolutionMethod )
//      {
//      case RSM_EXTERNAL:
//         {
//         if ( m_reachExtFn != NULL )
//            m_reachExtFn( &m_flowContext );
//         return true;
//         }
//
//      case RSM_KINEMATIC:
//         return SolveReachKinematicWave();
//      }
//
//   // anything else indicates a problem
//   ASSERT( 0 );
//   return false;
//   }


bool FlowModel::SolveGWDirect( void )
   {
//   switch( m_gwMethod )
//      {
//      case GW_EXTERNAL:
//         {
//         if ( m_gwExtFn != NULL )
//            m_gwExtFn( &m_flowContext );
//         return true;
//         }
//      }

   // anything else indicates a problem
   ASSERT( 0 );
   return false;
   }


//bool FlowModel::SetGlobalHruToReachExchanges( void )
//   {
//   switch( m_latExchMethod )
//      {
//      case HREX_EXTERNAL:
//         {
//         if ( m_latExchExtFn != NULL )
//            m_latExchExtFn( &m_flowContext );
//         return true;
//         }
//
//      case HREX_INFLUXHANDLER:
//         return 0.0f;
//
//      case HREX_LINEARRESERVOIR:
//         return SetGlobalHruToReachExchangesLinearRes();
//      }
//
//   // anything else indicates a problem
//   ASSERT( 0 );
//   return false;
//   }


//bool FlowModel::SetGlobalHruVertFluxes( void )
//   {
//   switch( m_hruVertFluxMethod )
//      {
//      case VD_EXTERNAL:
//         {
//         if ( m_hruVertFluxExtFn != NULL )
//            m_hruVertFluxExtFn( &m_flowContext );
//         return true;
//         }
//
//      case VD_INFLUXHANDLER:
//         return 0.0f;
//
//      case VD_BROOKSCOREY:
//         return SetGlobalHruVertFluxesBrooksCorey();
//      }
//
//   // anything else indicates a problem
//   ASSERT( 0 );
//   return false;
//   }



bool FlowModel::SetGlobalExtraSVRxn( void )
   {
   switch( m_extraSvRxnMethod )
      {
      case EXSV_EXTERNAL:
         {
         if ( m_extraSvRxnExtFn != NULL )
            m_extraSvRxnExtFn( &m_flowContext );
         return true;
         }

      case EXSV_INFLUXHANDLER:
         return 0.0f;
      }

   // anything else indicates a problem
   ASSERT( 0 );
   return false;
   }


bool FlowModel::SetGlobalReservoirFluxes( void  )
   {
   switch( m_reservoirFluxMethod )
      {
      case RES_EXTERNAL:
         {
         if ( m_reservoirFluxExtFn != NULL )
            m_reservoirFluxExtFn( &m_flowContext );
         return true;
         }

      case RES_RESSIMLITE:
         return SetGlobalReservoirFluxesResSimLite();
      }

   // anything else indicates a problem
   ASSERT( 0 );
   return false;
   }


bool FlowModel::InitReservoirControlPoints(void )
   {
   if (m_reservoirArray.GetSize() <= 0) return(true);

   int controlPointCount = (int) m_controlPointArray.GetSize();
    // Code here to step through each control point

   int colComID = m_pStreamLayer->GetFieldCol("comID");

   for ( int i=0; i < controlPointCount; i++ )
      {
      ControlPoint *pControl = m_controlPointArray.GetAt(i);
      ASSERT( pControl != NULL );

      TCHAR *r = NULL;
      TCHAR delim[] = ",";
      TCHAR res[256];
      TCHAR *next = NULL;
      CString resString = pControl->m_reservoirsInfluenced;  //list of reservoirs influenced...from XML file
      lstrcpy( res, resString);
      r = _tcstok_s( res, delim, &next );   //Get first value

      ASSERT( r != NULL );
      int res_influenced = atoi (r);       //convert to int

      // Loop through reservoirs and point to the ones that this control point has influence over
      while ( r != NULL )
         {
         for (int j=0; j < m_reservoirArray.GetSize(); j++)
            {
            Reservoir *pRes = m_reservoirArray.GetAt(j);
            ASSERT( pRes != NULL );

            int resID = pRes->m_id;

            if (resID == res_influenced)       //Is this reservoir influenced by this control point?
               {
               pControl->m_influencedReservoirsArray.Add( pRes );    //Add reservoir to array of influenced reservoirs
               r = strtok_s( NULL, delim, &next );   //Look for delimiter, Get next value in string
               if ( r == NULL )
                  break;

               res_influenced = atoi (r);
               }
            }

         /// Need code here (or somewhere, to associate a pReach with a control point)
         // iterate through reach network, looking control point ID's...  need to associate a pReach with each control point
         // Probably need a more efficient way to do this than cycling through the network for every control point
         int reachCount = m_pStreamLayer->GetRecordCount();

         for ( int k=0; k < m_reachArray.GetSize(); k++ )
            {
            Reach *pReach = m_reachArray.GetAt(k);
            if (pReach->m_reachID == pControl->m_location)     // Is this location a control point?
               {
                //If so, add a pointer to the reach to the control point class
               pControl->m_pReach = pReach; 
               break;
               }
            }
         }
      
      if ( pControl->m_pReach != NULL )
         {
         ASSERT( pControl->m_pResAllocation == NULL );
         pControl->m_pResAllocation = new FDataObj (int( pControl->m_influencedReservoirsArray.GetSize()), 0);

         CString cp_path(gpModel->m_path + pControl->m_dir + pControl->m_controlPointFileName);
         ApplyMacros(cp_path);
         //load table for control point constraints 
         LoadTable(cp_path, (DataObj**) &(pControl->m_pControlPointConstraintsTable),    0 );   
         }
      }
   
   return true;
   }


bool FlowModel::UpdateReservoirControlPoints( int doy )
   {
   int controlPointCount = (int) m_controlPointArray.GetSize();
    // Code here to step through each control point
     
   for ( int i=0; i < controlPointCount; i++ )
      {
      ControlPoint *pControl = m_controlPointArray.GetAt(i);
      ASSERT( pControl != NULL );

      if (pControl->InUse() == false )
         continue;

      CString filename = pControl->m_controlPointFileName;

      int location = pControl->m_location;     //Reach where control point is located

      //Get constraint type (m_type).  Assume 1st characters of filename until first underscore or space contain rule type.  
      //Parse rule name and determine m_type  ( IN the case of downstream control points, only maximum and minimum)
      TCHAR name[ 256 ];
      strcpy_s(name, filename);
      TCHAR* ruletype=NULL, *next=NULL;
      TCHAR delim[] = _T( " ,_" );  //allowable delimeters: , ; space ; underscore

      ruletype = strtok_s(name, delim, &next);  //Strip header.  should be cp_ for control points
      ruletype = strtok_s(NULL, delim, &next);  //Returns next string at head of file (max or min). 
         
      // Is this a min or max value
      if ( _stricmp(ruletype, "Max" ) == 0)
         pControl->m_type = RCT_MAX;
      else if (_stricmp(ruletype, "Min" ) == 0)
         pControl->m_type = RCT_MIN;
      
      // Get constraint value
      DataObj *constraintTable = pControl->m_pControlPointConstraintsTable;
      ASSERT( constraintTable != NULL );

     Reach *pReach = pControl->m_pReach;       //get reach at location of control point  
    
      CString xlabel = constraintTable->GetLabel(0);

      float xvalue = 0.0f;
      float yvalue = 0.0f;
      bool isFlowDiff = 0;

      if (_stricmp(xlabel, "Date") == 0)                         //Date based rule?  xvalue = current date.
         xvalue = (float)doy;
      else if (_stricmp(xlabel, "Pool_Elev_m") == 0)              //Pool elevation based rule?  xvalue = pool elevation (meters)
         xvalue = 110;   //workaround 11_15.  Need to point to Fern Ridge pool elev.  only one control point used this xvalue
      else if (_stricmp(xlabel, "Outflow_lagged_24h" ) == 0)
         {
         xvalue = (float)(pReach->GetReachDischargeWP().m_volume_m3 / SEC_PER_DAY);
        }
      else if (_stricmp(xlabel,"Inflow_cms" ) == 0)     
         {}//Code here for lagged and inflow based constraints
      else if  (_stricmp(xlabel,"date_water_year_type") == 0)         //Lookup based on two values...date and wateryeartype (storage in 13 USACE reservoirs on May 20th).
         {
         xvalue = (float) doy;
         yvalue = gpModel->m_waterYearType;
         }
      else                                                    //Unrecognized xvalue for constraint lookup table
         { 
         CString msg;
         msg.Format( "Flow:  Unrecognized x value for reservoir constraint lookup '%s', %s (id:%i) in stream network", (LPCTSTR) pControl->m_controlPointFileName, (LPCTSTR) pControl->m_name, pControl->m_id );
         Report::WarningMsg( msg );
         }
        
      
      // if the reach doesn't exist, ignore this
      if ( pReach != NULL )
         {       
         double currentdischarge = (pReach->GetReachDischargeWP().m_volume_m3 / SEC_PER_DAY);    // Get current flow at reach
         /*/////////////////////////////////////////////////////////
       ///Change flows with ResSim values.  Use only for testing in conjunction with low or zero precipitation
       int flowcol = 0;
       flowcol = pControl->localFlowCol;
       float localflow = 0.0f;

       if (flowcol >= 1)
          {
          localflow = m_pCpInflows->IGet(m_currentTime, 0, flowcol, IM_LINEAR);   //local flow value from ressim
          currentdischarge =+ localflow;
          }
       /////////////////////////////////////////////////////////////////////////*/

         float constraintValue = 0.0f;
         float addedConstraint = 0.0f;
       
         int influenced = (int) pControl->m_influencedReservoirsArray.GetSize();     // varies by control point
         if ( influenced > 0 )
            {
            ASSERT( pControl->m_pResAllocation != NULL );
            ASSERT( influenced == pControl->m_pResAllocation->GetColCount() );

            float *resallocation = new float[influenced]();   //array the size of all reservoirs influenced by this control point
   
            switch( pControl->m_type )
               {
               case RCT_MAX:    //maximum
                  {
                  ASSERT( pControl->m_pControlPointConstraintsTable != NULL );

                  if (yvalue > 0)  //Does the constraint depend on two values?  If so, use both xvalue and yvalue
                     constraintValue = pControl->m_pControlPointConstraintsTable->IGet(xvalue, yvalue, IM_LINEAR);
                  else             //If not, just use xvalue
                     constraintValue = pControl->m_pControlPointConstraintsTable->IGet(xvalue, 1, IM_LINEAR);
            
                  //Compare to current discharge and allocate flow increases or decreases
                  //Currently allocated evenly......need to update based on storage balance curves in ResSIM  mmc 11_15_2012
                  if (currentdischarge > constraintValue)   //Are we above the maximum flow?   
                     {
                     for (int j=0; j < influenced; j++)
                        resallocation[j] = (float)(((constraintValue - currentdischarge)/ influenced));// Allocate decrease in releases (should be negative) over "controlled" reservoirs if maximum, evenly for now
   
                     pControl->m_pResAllocation->ClearRows();     //Clear old data.  Only keep current allocation.
                     pControl->m_pResAllocation->AppendRow(resallocation, influenced);//Populate array with flows to be allocated
                     }
                  else                                      //If not above maximum, set maximum value high, so no constraint will be applied
                     {
                     for (int j=0; j < influenced; j++)
                        resallocation[ j ] = 0.0f;
                     
                     pControl->m_pResAllocation->ClearRows();     //Clear old data.  Only keep current allocation.
                     pControl->m_pResAllocation->AppendRow(resallocation, influenced );   //Populate array with flows to be allocated
                     }
                  }
                  break;

               case RCT_MIN:  //minimum
                  {
                  if (yvalue > 0)  //Does the constraint depend on two values?  If so, use both xvalue and yvalue
                     addedConstraint = constraintValue = pControl->m_pControlPointConstraintsTable->IGet(xvalue, yvalue, IM_LINEAR);
                  else             //If not, just use xvalue
                     addedConstraint = constraintValue = pControl->m_pControlPointConstraintsTable->IGet(xvalue, 1, IM_LINEAR);
                  
                  if (pControl->m_influencedReservoirsArray[0]->m_reservoirType == ResType_CtrlPointControl)
                  {
                     double resDischarge = pControl->m_influencedReservoirsArray[0]->m_pReach->GetReachDischargeWP().m_volume_m3 / SEC_PER_DAY;
                     addedConstraint = (float)(resDischarge + constraintValue);
                     int releaseFreq = 1;

                     for (int j = 0; j < influenced; j++)
                     {
                        resallocation[j] = (float)((addedConstraint - currentdischarge) / influenced); // Allocate increase in releases (should be positive) over "controlled" reservoirs if maximum, evenly for now                     }

                        releaseFreq = pControl->m_influencedReservoirsArray[j]->m_releaseFreq;

                        ASSERT(pControl->m_pResAllocation != NULL);                     
                        
                        if (resallocation[j] < 0) resallocation[j] = 0.0f;

                        if (releaseFreq == 1) pControl->m_pResAllocation->ClearRows();     //Clear old data.  Only keep current allocation.
                        pControl->m_pResAllocation->AppendRow(resallocation, influenced);   //Populate array with flows to be allocated
                     }
                  }
                  else
                  {
                     if (currentdischarge < constraintValue)   //Are we below the minimum flow?   
                     {
                        for (int j = 0; j < influenced; j++)
                           resallocation[j] = (float)((addedConstraint - currentdischarge) / influenced);// Allocate increase in releases (should be positive) over "controlled" reservoirs if maximum, evenly for now

                        ASSERT(pControl->m_pResAllocation != NULL);
                        pControl->m_pResAllocation->ClearRows();     //Clear old data.  Only keep current allocation.
                        pControl->m_pResAllocation->AppendRow(resallocation, influenced);   //Populate array with flows to be allocated
                     }
                     else                                      //If not below minimum, set min value at 0, so no constraint will be applied
                     {
                        for (int j = 0; j < influenced; j++)
                        {
                           resallocation[j] = 0.0f;
                        }

                        pControl->m_pResAllocation->ClearRows();     //Clear old data.  Only keep current allocation.
                        pControl->m_pResAllocation->AppendRow(resallocation, influenced);   //Populate array with flows to be allocated
                     }
                  }
                  }
                  break;
               }  // end of:  switch( pControl->m_type )

            delete [] resallocation;
            }  // end of: if influenced > 0 )
         }  // end of: if ( pReach != NULL )
      }  // end of:  for each control point
   
   return true;
}


void FlowModel::UpdateReservoirWaterYear( int dayOfYear )
{
   int reservoirCount = (int) m_reservoirArray.GetSize();
   float resVolumeBasin = 0.0;
   for ( int i=0; i < reservoirCount; i++ )
   {
      Reservoir *pRes = m_reservoirArray.GetAt(i);
      ASSERT( pRes != NULL );
         resVolumeBasin += (float) pRes->m_volume;   // This value in m3
   }

      resVolumeBasin = resVolumeBasin*M3_PER_ACREFT*1000000;   //convert volume to millions of acre-ft  (MAF)

      if (resVolumeBasin > 1.48f)                            //USACE defined "Abundant" water year in Willamette
         gpModel->m_waterYearType = 1.48f;
      else if (resVolumeBasin < 1.48f && resVolumeBasin > 1.2f)      //USACE defined "Adequate" water year in Willamette
         gpModel->m_waterYearType = 1.2f;
      else if (resVolumeBasin < 1.2f && resVolumeBasin > 0.9f)       //USACE defined "Insufficient"  water year in Willamette
         gpModel->m_waterYearType = 0.9f;
      else if (resVolumeBasin < 0.9f)                         //USACE defined "Deficit" water year in Willamette
         gpModel->m_waterYearType = 0;

}


bool FlowModel::SetGlobalReservoirFluxesResSimLite( void )
   {
   int reservoirCount = (int) m_reservoirArray.GetSize();

   int dayOfYear = int(m_timeInRun - m_yearStartTime);  // zero based day of year
   UpdateReservoirControlPoints(dayOfYear);   //Update current status of system control points

   //A Willamette specific ruletype that checks res volume in the basin on May 20th and adjusts rules based on water year classification.
   if (dayOfYear == 140)   //Is it May 20th?  Update basinwide reservoir volume and designate water year type
      {
      UpdateReservoirWaterYear( dayOfYear );
      }

   // iterate through reservoirs, calling fluxes as needed
   for ( int i=0; i < reservoirCount; i++ )
      {
     Reservoir *pRes = m_reservoirArray.GetAt(i);
      ASSERT( pRes != NULL );

      // first, inflow.  We get the inflow from any stream reaches flowing into the downstream reach this res is connected to.
      Reach *pReach = pRes->m_pReach;
      
      // no associated reach?
      if ( pReach == NULL )
         {
         pRes->m_inflow = pRes->m_outflow = 0;
         pRes->m_inflowWP = pRes->m_outflowWP = WaterParcel(0, 0);
         continue;
         }

      Reach *pLeft  = (Reach*) pReach->m_pLeft;
      Reach *pRight = (Reach*) pReach->m_pRight;

      //set inflow, outflows
      double inflow_cms = 0;
      WaterParcel inflowWP = WaterParcel(0, 0);
     
      //   To be re-implemented with remainder of FLOW hydrology - ignored currently to test ResSIMlite
      if (pLeft != NULL)
      {
         inflowWP = pLeft->GetReachDischargeWP();
         inflow_cms = inflowWP.m_volume_m3 / SEC_PER_DAY;
      }
      if (pRight != NULL)
      {
		   WaterParcel rightWP = pRight->GetReachDischargeWP();
         inflowWP.MixIn(rightWP);
         inflow_cms = inflowWP.m_volume_m3 / SEC_PER_DAY;
      }
     

      /*////////////////////////////////////////////////////////////////////////////////////////////
     //Get inflows exported from ResSIM model.  Used for testing in conjunction with 0 precip.
     int resID = pRes->m_id;
     
    inflow_cms = m_pResInflows->IGet(m_currentTime, resID);   
     
     //Hard code here for additional inflows into Lookout, Foster, Dexter and BigCliff from upstream dam releases

     //Lookout = Lookout + Hills Creek
      if (resID == 2)
       {
       int HCid= 0;
        Reservoir *pHC = m_reservoirArray.GetAt(HCid);
         inflow_cms += pHC->m_outflow/SEC_PER_DAY; 
        }
     //Foster = Foster + Green Peter
      if (resID == 11)
       {
        int GPid = 9;
       Reservoir *pGP = m_reservoirArray.GetAt(GPid);
         inflow_cms += pGP->m_outflow/SEC_PER_DAY;
        }

     //Dexter
     if (resID == 3)
        {
        int LOid = 1;
        Reservoir *pLO = m_reservoirArray.GetAt(LOid);
       inflow_cms = pLO->m_outflow/SEC_PER_DAY;
        }

     //Big Cliff
     if (resID == 13)
       {
        int DETid = 11;
        Reservoir *pDET = m_reservoirArray.GetAt(DETid);
       inflow_cms = pDET->m_outflow/SEC_PER_DAY;
        }
     ///////////////////////////////////////////////////////////////////////////////////////////*/ 
     

      pRes->m_inflow = inflow_cms*SEC_PER_DAY;  //m3   
      pRes->m_inflowWP = inflowWP;
          
      pRes->m_outflowWP = pRes->GetResOutflowWP(pRes, dayOfYear);
      pRes->m_outflow = pRes->m_outflowWP.m_volume_m3;    // m3 
      ASSERT(pRes->m_outflow >= 0);

      // Store today's hydropower generation (megawatts) for this reservoir into the reach attribute HYDRO_MW for the reach which receives the reservoir outflow.
     int reach_ndx = m_pReachLayer->FindIndex(m_colStreamCOMID, pReach->m_reachID, 0);
     m_pReachLayer->SetDataU(reach_ndx, m_colStreamHYDRO_MW, pRes->m_power);

      } // end of loop through reservoirs

   return true;
   } // end of SetGlobalReservoirFluxesResSimLite()


bool FlowModel::SetGlobalFlows( void )
   {
   if ( m_pGlobalFlowData == NULL )
      return false;

   // Basic idea:  The m_pGlobalFlowData (GeospatialDataObj) contains info about a collection
   // of points in the stream network that have flow data. A "slice" of the data object
   // contains flows for the collection of source points.
   //
   // Each point is connected to a value (default="COMID") in the reach coverage that defines
   // where in the reach network that flow applies.  (???Where is this coming from)
   //
   // Once the flow are set at the measured points, they are interpolated across the entire 
   // ReachTree containing the stream segment nodes, distributed based on contributing areas.
 /*
   CArray< int, int > m_globalFlowInputReachIndexArray;

   for ( int i=0; i < m_pGlobalFlowData->GetRowCount(); i++ )  // assumes 2 cols (COMID, FLOW)
      {
      int id;
      m_pGlobalFlowData->Get( 0, i, id );

      int reachIndex = m_pStreamLayer->FindIndex( m_colGlobalFlowInput, id );
      if ( reachIndex >= 0 )
         m_globalFlowInputReachIndexArray.Add( reachIndex );
      else
         {
         CString msg;
         msg.Format( "Flow: Reach '%s:%i referenced in flow specification file %s could not be located in the reach network.",
            (LPCTSTR) m_globalFlowReachFieldName, id, (LPCTSTR) m_globalFlowInputFile );
         Report::WarningMsg( msg );
         
         m_globalFlowInputReachIndexArray.Add( -1 );
         }
      }

   // write data to reachtree
   

   // zero out flows in the Reach network
   for ( int i=0; i < (int) this->m_reachArray.GetSize(); i++ )
      {
      Reach *pReach = m_reachArray[ i ];
      int subnodeCount = pReach->GetSubnodeCount();

      for ( int j=0; j < subnodeCount; j++ )
         {
         ReachSubnode *pNode = pReach->GetReachSubnode( i );
         pNode->m_discharge = pNode->m_volume = 0.0F;
         }
      }

      */
     
   return true;
   }


bool FlowModel::RedrawMap(EnvContext *pEnvContext)
   {
   int activeField = m_pCatchmentLayer->GetActiveField();
//   m_pStreamLayer->UseVarWidth(0, 200);
   m_pStreamLayer->ClassifyData();
   m_pCatchmentLayer->ClassifyData(activeField);

   if ( m_grid ) // m_buildCatchmentsMethod==2 )//a grid based simulation
      {
      if ( m_pGrid != NULL  )
         {
         if (m_gridClassification!=1)
            {
            m_pGrid->SetBins( -1, 0, 5, 20, TYPE_FLOAT );
            m_pGrid->SetBinColorFlag(BCF_BLUEGREEN);
            m_pGrid->ClassifyData(0);
            }
         if (m_gridClassification==0)
            {
            m_pGrid->SetBins( -1, 0.1f, 0.4f, 20, TYPE_FLOAT );
            m_pGrid->SetBinColorFlag(BCF_BLUEGREEN);
            m_pGrid->ClassifyData(0);
            }
         if (m_gridClassification==1) //elws
            {
            m_pGrid->SetBins( -1, -10, 100, 20, TYPE_FLOAT );
            m_pGrid->SetBinColorFlag(BCF_BLUEGREEN);
            m_pGrid->ClassifyData(0);
            }
         if (m_gridClassification == 2) //depthToWT
            {
            m_pGrid->SetBins(-1, 0, 30, 20, TYPE_FLOAT);
            m_pGrid->SetBinColorFlag(BCF_BLUEGREEN);
            m_pGrid->ClassifyData(0);
            }
         if (m_gridClassification==6)
            {
            m_pGrid->SetBins( -1, 0.0f, 0.4f, 20, TYPE_FLOAT );
            m_pGrid->SetBinColorFlag(BCF_BLUEGREEN);
            m_pGrid->ClassifyData(0);
            }
         
         if ( m_gridClassification==4)//SWE in mm
            {
            m_pGrid->SetBins( -1, 0.0f, 1200.0f, 20, TYPE_FLOAT );
            m_pGrid->SetBinColorFlag(BCF_BLUEGREEN);
            m_pGrid->ClassifyData(0);         
            }  
         if  (m_gridClassification==5)//SWE in mm
            {
            m_pGrid->SetBins( -1, -15.0f, 30.0f ,20, TYPE_FLOAT );
            m_pGrid->SetBinColorFlag(BCF_BLUERED);
            m_pGrid->ClassifyData(0);         
            }  
         }
      }
   
   ::EnvRedrawMap();

   //CDC *dc = m_pMap->GetDC();
   //m_pMap->DrawMap(*dc);
   //m_pMap->ReleaseDC(dc);
   //// yield control to other processes
   //MSG  msg;
   //while( PeekMessage( &msg, NULL, NULL, NULL , PM_REMOVE ) )
   //   {
   //   TranslateMessage(&msg); 
   //   DispatchMessage(& msg); 
   //   }

   return true;
   }


void FlowModel::UpdateYearlyDeltas(EnvContext *pEnvContext )
   {
   int activeField = m_pCatchmentLayer->GetActiveField();
   int catchmentCount = (int) m_catchmentArray.GetSize();
   float airTemp = -999.0f;
   
   int num_hrus = (int)m_hruArray.GetSize();
   for (int hru_ndx = 0; hru_ndx < num_hrus; hru_ndx++)
   {
      HRU* pHRU = m_hruArray[hru_ndx];
      pHRU->m_temp_yr /= pEnvContext->daysInCurrentYear;   
      pHRU->m_temp_10yr.AddValue(pHRU->m_temp_yr);
      pHRU->m_precip_10yr.AddValue(pHRU->m_precip_yr);

      // annual climate variables
      for (int k = 0; k < pHRU->m_polyIndexArray.GetSize(); k++)
      {
         int idu = pHRU->m_polyIndexArray[k];


         gpFlow->UpdateIDU(pEnvContext, idu, m_colHruTempYr, pHRU->m_temp_yr, true);   // C
         gpFlow->UpdateIDU(pEnvContext, idu, m_colHruTemp10Yr, pHRU->m_temp_10yr.GetValue(), true); // C
         gpFlow->UpdateIDU(pEnvContext, idu, m_colPRECIP_YR, pHRU->m_precip_yr, true);   // mm/year
         gpFlow->UpdateIDU(pEnvContext, idu, m_colHruPrecip10Yr, pHRU->m_precip_10yr.GetValue(), true); // mm/year
         gpFlow->UpdateIDU(pEnvContext, idu, m_colRunoff_yr, pHRU->m_runoff_yr, true);
         // ET_YR and MAX_ET_YR are accumulated IDU by IDU from daily values in EvapTrans::GetHruET().
      } // end of loop thru the IDUs in this HRU
   } // end of loop thru HRUs

} // end of UpdateYearlyDeltas()


void FlowModel::GetMaxSnowPack(EnvContext *pEnvContext)
{
   // First sum the total volume of snow for this day.
   double totalSnow_m3 = 0.0f;
   for (int j = 0; j < m_hruArray.GetSize(); j++)
   {
      HRU* pHRU = m_hruArray.GetAt(j);
      if (!pHRU->m_snowpackFlag) continue;
      totalSnow_m3 += pHRU->GetLayer(BOX_SNOW)->m_volumeWater;
      for (int idu_ndx_in_hru = 0; idu_ndx_in_hru < pHRU->m_polyIndexArray.GetSize(); idu_ndx_in_hru++)
      {
         int idu_ndx = pHRU->m_polyIndexArray[idu_ndx_in_hru];
         int lulc_a = AttInt(idu_ndx, LULC_A);
         if (lulc_a == LULCA_WETLAND) continue;
         double h2o_melt_mm = Att(idu_ndx, H2O_MELT);
         float area_m2 = AttFloat(idu_ndx, AREA);
         double h2o_melt_m3 = (h2o_melt_mm / 1000.) * area_m2;
         totalSnow_m3 += h2o_melt_m3;
      } // end of loop thru IDUs in HRU j
   } // end of loop thru HRUs

   // Then compare the total volume to the previously defined maximum volume.
   if (totalSnow_m3 > m_volumeMaxSWE)
   {
      m_volumeMaxSWE = (float)totalSnow_m3;
      int dayOfYear = int(m_timeInRun - m_yearStartTime);  // zero based day of year
      m_dateMaxSWE = dayOfYear;

      // Update the IDU MAXSNOW attribute values.
      for (int j = 0; j < m_hruArray.GetSize(); j++)
      {
         HRU* pHRU = m_hruArray.GetAt(j);
         for (int idu_ndx_in_hru = 0; idu_ndx_in_hru < pHRU->m_polyIndexArray.GetSize(); idu_ndx_in_hru++)
         {
            int idu_ndx = pHRU->m_polyIndexArray[idu_ndx_in_hru];
            int lulc_a = AttInt(idu_ndx, LULC_A);
            if (lulc_a == LULCA_WETLAND) continue;
            double snow_swe_mm = Att(idu_ndx, SNOW_SWE);
            double h2o_melt_mm = Att(idu_ndx, H2O_MELT);
            float maxsnow_mm = (float)(snow_swe_mm);
            SetAttFloat(idu_ndx, MAXSNOW, maxsnow_mm);
         } // end of loop thru IDUs in HRU j
      } // end of loop thru HRUs
   } // end of if (totalSnow > m_volumeMaxSWE)

} // end of GetMaxSnowPack()


void FlowModel::UpdateAprilDeltas(EnvContext *pEnvContext )
   {
  // int activeField = m_pCatchmentLayer->GetActiveField();
   int catchmentCount = (int) m_catchmentArray.GetSize();
   m_pCatchmentLayer->m_readOnly=false;
  // float airTemp=-999.0f;
   
   if ( !m_pGrid )   //m_buildCatchmentsMethod != 2 )    // not a grid based model?
      {
      for (int i=0; i< catchmentCount; i++)
         {
         Catchment *pCatchment = m_catchmentArray[ i ];
         for ( int j=0; j < pCatchment->GetHRUCountInCatchment(); j++ )
            {
            HRU *pHRU = pCatchment->GetHRUfromCatchment( j );
            for (int k=0; k< pHRU->m_polyIndexArray.GetSize();k++)
               {
               int idu = pHRU->m_polyIndexArray[k];

               float refSnow=0.0f;

               float _ref=((float)pHRU->GetLayer(BOX_SNOWPACK)->m_volumeWater+(float)pHRU->GetLayer(BOX_MELT)->m_volumeWater)/pHRU->m_HRUeffArea_m2*1000.0f ; //get the current year snowpack (mm)
               
           
               float snowIndex = 0.0f;
               if (refSnow > 0.0f)
                  snowIndex=_ref/(refSnow/10.0f);    // compare to avg over first decade


               // BAD!!!! assumes top two layers are snow!!!
               float apr1SWE = (((float)pHRU->GetLayer(BOX_SNOWPACK)->m_volumeWater + (float)pHRU->GetLayer(BOX_MELT)->m_volumeWater) / pHRU->m_HRUeffArea_m2)*1000.0f; // (mm)
               if ( apr1SWE < 0 )
                  apr1SWE = 0;
               
               gpFlow->UpdateIDU(pEnvContext, idu, m_colHruApr1SWE, apr1SWE, true );

               pHRU->m_apr1SWE_10yr.AddValue( apr1SWE );
               float apr1SWEMovAvg = pHRU->m_apr1SWE_10yr.GetValue();
               gpFlow->UpdateIDU( pEnvContext, idu, m_colHruApr1SWE10Yr, apr1SWEMovAvg, true );

               pHRU->m_precipCumApr1 = pHRU->m_precip_yr;      // mm of cumulative precip Jan 1 - Apr 1)
               }
            }
         }
      }

   m_pCatchmentLayer->m_readOnly=true;
   }


bool FlowModel::WriteDataToMap(EnvContext *pEnvContext )
   {
   int activeField = m_pCatchmentLayer->GetActiveField();
   m_pCatchmentLayer->m_readOnly=false;
   int reachCount = (int)m_reachArray.GetSize();
   for ( int i=0; i < reachCount; i++ )
      {
      Reach *pReach = m_reachArray.GetAt(i);
      ASSERT( pReach != NULL );
      if ( pReach->m_subnodeArray.GetSize() > 0 && pReach->m_polyIndex >= 0 )
      {
         WaterParcel dischargeWP = pReach->GetReachDischargeWP(); 
         double discharge_cms = dischargeWP.m_volume_m3 / SEC_PER_DAY;
         ASSERT(discharge_cms > 0.);
         double q2wetl_cms = pReach->Att(ReachQ2WETL);
         double downstream_cms = discharge_cms - q2wetl_cms;

         m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachQ_DISCHARG, downstream_cms);  // m3/sec        
         m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachLOG_Q, log10(downstream_cms));  // log10(m3/sec)
         m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachTEMP_H2O, dischargeWP.WaterTemperature());  // degC       

         Reservoir* pReservoir = pReach->m_pReservoir;
         if (pReservoir != NULL)
         {
            double res_h2o_m3 = pReservoir->m_resWP.m_volume_m3;
            m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachRES_H2O, res_h2o_m3);
            m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachRES_TEMP, pReservoir->m_resWP.WaterTemperature());
            double res_area_ha = pReservoir->GetPoolSurfaceAreaFromVolume_ha();
            m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachRESAREA_HA, res_area_ha);
         }

         double reachH2O_m3 = 0.;
         double reach_evap_m3 = 0.;
         double reach_surf_area_m2 = 0.;
         WaterParcel reach_in_runoffWP(0, 0);
         double reach_out_lateral_m3 = 0.;
         for (int j = 0; j < pReach->m_subnodeArray.GetSize(); j++)
         {
            ReachSubnode *pNode = pReach->GetReachSubnode(j);
            reachH2O_m3 += pNode->m_waterParcel.m_volume_m3;
            reach_evap_m3 += pNode->m_evap_m3;
            reach_surf_area_m2 += pNode->m_subreach_surf_area_m2;
            reach_in_runoffWP.MixIn(pNode->m_runoffWP);
            reach_out_lateral_m3 += pNode->m_withdrawal_m3;
         }
         double reach_evap_mm = (reach_evap_m3 / reach_surf_area_m2) * MM_PER_M;
         m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colStreamREACH_H2O, reachH2O_m3);
         m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachEVAP_MM, reach_evap_mm);
         float spring_cms = 0.; m_pReachLayer->GetData(pReach->m_polyIndex, m_colReachSPRING_CMS, spring_cms); 
         if (spring_cms > 0)
         { // Spring water is tracked separately in the reach attributes SPRING_CMS and SPRINGTEMP, so remove it before storing reach_in_runoffWP to IN_RUNOFF and IN_RUNOF_C.
            float springtemp_degC = 0.; m_pReachLayer->GetData(pReach->m_polyIndex, m_colReachSPRINGTEMP, springtemp_degC);
            WaterParcel springWP((double)spring_cms * SEC_PER_DAY, springtemp_degC);
            int evap_code = reach_in_runoffWP.Evaporate((double)spring_cms * SEC_PER_DAY, springWP.ThermalEnergy()); // Remove spring water from reach_in_runoffWP..
            switch (evap_code)
            {
               case -1: // temperature of WaterParcel after evaporation is > NOMINAL_MAX_WATER_TEMP_DEGC
               {
                  CString msg; 
                  msg.Format("WriteDataToMap()1 reach_in_runoffWP.m_temp_degC = %f", reach_in_runoffWP.m_temp_degC);
                  Report::LogWarning(msg);
               }
               break;

               case -2:
               case -3:
                  reach_in_runoffWP = WaterParcel(0, 0);
                  break;

               default:
                  break;
            }
         }

         // It is not necessary to separate out flood water from normal runoff because at this point
         // the flood water is in m_globalHandlerFluxValue.  It hasn't gotten to pNode->m_runoffWP yet
         // because ApplyQ2WETL() runs after ReachRouting::Step().

         double reach_in_runoff_cms = reach_in_runoffWP.m_volume_m3 / SEC_PER_DAY;
         m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachIN_RUNOFF, reach_in_runoff_cms);
         m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachIN_RUNOF_C, reach_in_runoffWP.WaterTemperature());
         double reach_out_lateral_cms = reach_out_lateral_m3 / SEC_PER_DAY;
         m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachOUT_LATERL, reach_out_lateral_cms);
      }
   }

   for (MapLayer::Iterator idu = m_pCatchmentLayer->Begin(); idu < m_pCatchmentLayer->End(); idu++)
   {
      float et_day = 0.f; m_pIDUlayer->GetData(idu, m_colET_DAY, et_day);
      float sm2atm = 0.f; m_pIDUlayer->GetData(idu, m_colSM2ATM, sm2atm);
      et_day += sm2atm;
      m_pIDUlayer->SetDataU(idu, m_colET_DAY, et_day);
      float et_yr = 0.f; m_pIDUlayer->GetData(idu, m_colET_YR, et_yr);
      et_yr += et_day;
      m_pIDUlayer->SetDataU(idu, m_colET_YR, et_yr);
   } // end of loop thru IDUs

   return true;
} // end of WriteDataToMap()


void FlowModel::ResetReachFluxes()
{
   for (int i = 0; i < m_reachArray.GetSize(); i++)
   {
      Reach* pReach = m_reachArray.GetAt(i);
      pReach->ResetFluxValue(); pReach->m_additionsWP = WaterParcel(0, 0); // pReach->m_withdrawals_m3 = 0.;

      ReachSubnode* subnode = pReach->GetReachSubnode(0);
      for (int k = 0; k < m_hruSvCount - 1; k++)
         subnode->ResetExtraSvFluxValue(k);
   }
   
   return;
} // end of ResetReachFluxes()


void FlowModel::ResetHRUfluxes() // Also resets catchment fluxes
{
   for (int i = 0; i < m_hruArray.GetSize(); i++)
   {
      HRU* pHRU = m_hruArray.GetAt(i);
      int neighborCount = (int)pHRU->m_neighborArray.GetSize();

      for (int j = 0; j < pHRU->m_layerArray.GetSize(); j++)
      {
         HRULayer* pLayer = pHRU->m_layerArray.GetAt(j);
         pLayer->ResetFluxValue();

         for (int k = 0; k < (int)pLayer->m_waterFluxArray.GetSize(); k++) //kbv 4/27/15   why would these two lines be commented out?
            pLayer->m_waterFluxArray[k] = 0.0f;

         for (int l = 0; l < m_hruSvCount - 1; l++)
            pLayer->ResetExtraSvFluxValue(l);
      } // end of loop thru HRULayers in this HRU
   } // end of loop thru HRUs

   for (int i = 0; i < m_catchmentArray.GetSize(); i++)
   {
      ASSERT(m_catchmentArray[i] != NULL);
      m_catchmentArray[i]->m_contributionToReach = 0.0f;
   } // end of loop through catchments

   return;
} // end of ResetHRUfluxes()


bool FlowModel::ResetCumulativeYearlyValues( )
   {
   EnvContext *pContext = m_flowContext.pEnvContext;
   
   for ( int i=0; i < m_hruArray.GetSize(); i++ )
   {
      HRU *pHRU = m_hruArray.GetAt(i);

      pHRU->m_temp_yr   = 0;
      pHRU->m_precip_yr = 0;
      pHRU->m_rainfall_yr = 0;
      pHRU->m_snowfall_yr = 0;
      pHRU->m_gwRecharge_yr = 0.0f;       // mm
      pHRU->m_gwFlowOut_yr = 0.0f;
      pHRU->m_precipCumApr1 = 0;
      pHRU->m_maxET_yr = 0.0f;
      pHRU->m_et_yr = 0.0f;
      pHRU->m_aquifer_recharge_yr_mm = 0.0f;
      pHRU->m_runoff_yr =0.0f;
   } // end of loop through HRUs

   return(true);
} // end of ResetCumulativeYearlyValues()


bool FlowModel::ResetCumulativeWaterYearValues()
   {
   for (int i = 0; i < m_hruArray.GetSize(); i++)
      {
      HRU *pHRU = m_hruArray.GetAt(i);
      pHRU->m_precip_wateryr = 0.0f;
      }
   return true;
   }


inline double HRU::Att(int hruCol)
{
   double attribute;
   gFlowModel->m_pHRUlayer->GetData(this->m_hruNdx, hruCol, attribute);
   return(attribute);
} // end of HRU::Att()


inline int HRU::AttInt(int hruCol)
{
   int attribute;
   gFlowModel->m_pHRUlayer->GetData(this->m_hruNdx, hruCol, attribute);
   return(attribute);
} // end of HRU::AttInt()


inline float HRU::AttFloat(int hruCol)
{
   float attribute;
   gFlowModel->m_pHRUlayer->GetData(this->m_hruNdx, hruCol, attribute);
   return(attribute);
} // end of HRU::AttFloat()


inline void HRU::SetAtt(int col, double attValue)
{
   gFlowModel->m_pHRUlayer->SetDataU(this->m_hruNdx, col, attValue);
} // end of HRU::SetAtt()


inline void HRU::SetAttInt(int col, int attValue)
{
   gFlowModel->m_pHRUlayer->SetDataU(this->m_hruNdx, col, attValue);
} // end of HRU::SetAttInt()


inline void HRU::SetAttFloat(int col, float attValue)
{
   gFlowModel->m_pHRUlayer->SetDataU(this->m_hruNdx, col, attValue);
} // end of HRU::SetAttFloat()


inline double Reach::Att(int col)
{
   double attribute;
   pLayer->GetData(this->m_polyIndex, col, attribute);
   return(attribute);
} // end of Reach::Att()


inline int Reach::AttInt(int col)
{
   int attribute;
   pLayer->GetData(this->m_polyIndex, col, attribute);
   return(attribute);
} // end of Reach::AttInt()


inline void Reach::SetAtt(int col, double attValue)
{
   pLayer->SetDataU(this->m_polyIndex, col, attValue);
} // end of Reach::SetAtt()


inline void Reach::SetAttInt(int col, int attValue)
{
   pLayer->SetDataU(this->m_polyIndex, col, attValue);
} // end of Reach::SetAttInt()


inline double FlowModel::Att(int IDUindex, int col) { return(gIDUs->Att(IDUindex, col)); }
inline float FlowModel::AttFloat(int IDUindex, int col) { return(gIDUs->AttFloat(IDUindex, col)); }
inline int FlowModel::AttInt(int IDUindex, int col) { return(gIDUs->AttInt(IDUindex, col)); }


inline void FlowModel::SetAtt(int IDUindex, int col, double attValue)
{
   m_pIDUlayer->SetDataU(IDUindex, col, attValue);
} // end of SetAtt()


inline void FlowModel::SetAttFloat(int IDUindex, int col, float attValue)
{
   m_pIDUlayer->SetDataU(IDUindex, col, attValue);
} // end of SetAttFloat()


inline void FlowModel::SetAttInt(int IDUindex, int col, int attValue)
{
   m_pIDUlayer->SetDataU(IDUindex, col, attValue);
} // end of SetAttInt()


Wetland::Wetland(int wetlID) : m_wetlNdx(-1), m_wetlArea_m2(0.)
{
   m_wetlID = wetlID; 
   m_wetlIDUndxArray.RemoveAll();
} // end of Wetland constructor


int FlowModel::InitWetlands() // Returns the number of wetlands.
{
   if (WETL_ID < 0) return(0); // If there isn't any WETL_ID attribute, then there aren't any wetlands.

   int reach_count = (int)m_reachArray.GetCount();
   int reach_array_ndx;
   for (reach_array_ndx = 0; reach_array_ndx < reach_count; reach_array_ndx++)
   {
      Reach * pReach = m_reachArray[reach_array_ndx];
      pReach->m_wetlNdx = NON_WETLAND_TOKEN;
   } // end of loop thru m_reachArray

   int num_wetlands = 0;
   m_wetlArray.RemoveAll();
   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      int wetl_id = AttInt(idu, WETL_ID);
      bool is_wetland = AttInt(idu, LULC_A) == LULCA_WETLAND;
      if (!is_wetland) continue;

      Wetland* pWetl = NULL;
      double idu_area_m2 = Att(idu, AREA);
      int wetl_ndx = 0; while (wetl_ndx < num_wetlands && m_wetlArray[wetl_ndx]->m_wetlID != wetl_id) wetl_ndx++;

      if (wetl_ndx >= num_wetlands)
      { // Add a wetland
         pWetl = new Wetland(wetl_id);
         m_wetlArray.Add(pWetl);
         pWetl->m_wetlNdx = wetl_ndx;
         num_wetlands++;
      }
      else pWetl = m_wetlArray[wetl_ndx];

      pWetl->m_wetlArea_m2 += idu_area_m2;
      pWetl->m_wetlIDUndxArray.Add(idu);

   } // end of loop through IDUs

   // A single wetland may extend across more than one HRU.
   // But a single HRU may contain IDUs from no more than one wetland.
   int num_HRUs = (int)m_hruArray.GetCount();
   for (int hru_ndx = 0; hru_ndx < num_HRUs; hru_ndx++)
   {
      int wetl_id = NON_WETLAND_TOKEN;
      HRU* pHRU = GetHRU(hru_ndx);
      int num_idus_in_hru = (int)pHRU->m_polyIndexArray.GetCount();
      bool hru_ok = true;
      for (int idu_ndx_in_hru = 0; hru_ok && idu_ndx_in_hru < num_idus_in_hru; idu_ndx_in_hru++)
      {
         int idu_poly_ndx = pHRU->m_polyIndexArray[idu_ndx_in_hru];
         bool is_wetland = AttInt(idu_poly_ndx, LULC_A) == LULCA_WETLAND;
         if (!is_wetland) continue;

         int idu_wetl_id = AttInt(idu_poly_ndx, WETL_ID);
         if ((idu_wetl_id != NON_WETLAND_TOKEN) && (idu_wetl_id != wetl_id))
         {
            if (wetl_id == NON_WETLAND_TOKEN) wetl_id = idu_wetl_id;
            else
            {
               CString msg;
               msg.Format("InitWetlands() HRU %d contains more than one wetland. IDU %d is in wetland %d, whereas another IDU is in wetland %d.",
                  pHRU->m_id, AttInt(idu_poly_ndx, IDU_ID), idu_wetl_id, wetl_id);
               Report::ErrorMsg(msg);
               hru_ok = false;
            } // end of if (wetl_id == NON_WETLAND_TOKEN) ... else
         } // end of if (idu_wetl_id != NON_WETLAND_TOKEN)
      } // end of loop thru IDUs in this HRU
   } // end of loop thru HRUs

   // Now, for each wetland, rearrange the entries in m_wetlIDUndxArray into an order 
   // representing the order in which the wetland IDUs will be inundated,
   // i.e. from the IDU at the lowest elevation to the one at the highest elevation.
   // Also, set pReach->m_wetlNdx for the reach that overflows into the wetland.
   for (int wetl_ndx = 0; wetl_ndx < num_wetlands; wetl_ndx++)
   {
      Wetland* pWetl = m_wetlArray[wetl_ndx];
      int num_idus = (int)pWetl->m_wetlIDUndxArray.GetCount();
      CArray <int, int> idus; idus.SetSize(num_idus);
      CArray <float, float> elevations; elevations.SetSize(num_idus);
      for (int i = 0; i < num_idus; i++)
      {
         int idu_ndx = pWetl->m_wetlIDUndxArray[i];
         idus[i] = idu_ndx;
         elevations[i] = AttFloat(idu_ndx, ELEV_MEAN);
      }

      // Unsorted IDU indices are in idus[] and their elevations are in elevations[].
      // Now sort them from lowest to highest elevation into m_wetlIDUndxArray.
      int num_remaining_idus = num_idus;
      while (num_remaining_idus > 0)
      {
         float min_elev_m = 10000.;
         int j_min_elev = -1;
         for (int j = 0; j < num_remaining_idus; j++)
         {
            if (elevations[j] < min_elev_m)
            {
               min_elev_m = elevations[j];
               j_min_elev = j;
            }
         }
         int idu_ndx = idus[j_min_elev];
         int i = num_idus - num_remaining_idus;
         pWetl->m_wetlIDUndxArray[i] = idu_ndx;

         idus.RemoveAt(j_min_elev);
         elevations.RemoveAt(j_min_elev);
         num_remaining_idus--;
      } // end of while (num_remaining_idus > 0)

      // Now load up pWetl->m_wetlHRUndxArray, pWetl->m_wetlReachNdxArray and pReach->m_wetlNdx.
      pWetl->m_wetlHRUndxArray.SetSize(num_idus);
      pWetl->m_wetlReachNdxArray.SetSize(num_idus);
      for (int i = 0; i < num_idus; i++)
      {
         int idu_ndx = pWetl->m_wetlIDUndxArray[i];
         int hru_ndx = AttInt(idu_ndx, HRU_NDX);
         pWetl->m_wetlHRUndxArray[i] = hru_ndx;
         HRU* pHRU = GetHRU(hru_ndx);
         int comid = pHRU->AttInt(HruCOMID);
// ??? This isn't necessarily the reach associated with the IDU, since a single HRU can be associated with more than one reach.
         Reach* pReach = GetReachFromCOMID(comid);
         pWetl->m_wetlReachNdxArray[i] = pReach->m_reachArrayNdx;
         // By convention, a reach may only be associated with a single wetland.
         if (pReach->m_wetlNdx != wetl_ndx)
         {
            ASSERT(pReach->m_wetlNdx == NON_WETLAND_TOKEN);
            pReach->m_wetlNdx = wetl_ndx;
         }
      } // end of loop through m_wetlIDUndxArray

      pWetl->InitWETL_CAPifNecessary(); // Set nominal values of WETL_CAP if needed. 

   } // end of loop through all wetlands

   CString msg;
   msg.Format("InitWetlands() initialized %d Wetland objects.", num_wetlands);
   Report::LogMsg(msg);

   return(num_wetlands);
} // end of InitWetlands()


// create, initialize reaches based on the stream layer
bool FlowModel::InitReaches(void)
   {
   ASSERT(m_pStreamLayer != NULL);
   Reach::pLayer = m_pStreamLayer; 

   m_colStreamFrom = m_pStreamLayer->GetFieldCol(_T("FNODE_"));
   m_colStreamTo = m_pStreamLayer->GetFieldCol(_T("TNODE_"));

   if (m_colStreamFrom < 0 || m_colStreamTo < 0)
      {
      Report::ErrorMsg("Flow: Stream layer is missing a required column (FNODE_ and/or TNODE_).  These are necessary to establish topology");
      return false;
      }

   // ??? This next part may be temporary.  If we ever get a source of real data other than Shade-a-lator for these parameters,
   // then we won't want to wipe out the real data here.
   if (m_flowContext.pEnvContext->spinupFlag)
   {
      m_pReachLayer->SetColDataU(ReachDEPTH_MIN, -1.);
      m_pReachLayer->SetColDataU(ReachWIDTH_MIN, -1.);
      m_pReachLayer->SetColDataU(ReachWIDTHGIVEN, -1.);
      m_pReachLayer->SetColDataU(ReachVGDNSGIVEN, -1.);
      m_pReachLayer->SetColDataU(ReachVEGHTGIVEN, -1.);
      m_pReachLayer->SetColDataU(ReachVEG_HT_L, -1.);
      m_pReachLayer->SetColDataU(ReachVEG_HT_R, -1.);
      m_pReachLayer->SetColDataU(ReachTOPOELEV_E, -1.);
      m_pReachLayer->SetColDataU(ReachTOPOELEV_S, -1.);
      m_pReachLayer->SetColDataU(ReachTOPOELEV_W, -1.);
      m_pReachLayer->SetColDataU(ReachRADSWGIVEN, -1.);
   } // end of if (...spinupFlag)
   // end of possibly temporary code


   /////////////////////
   //   FDataObj dataObj;
   //   m_reachTree.BuildDistanceMatrix( &dataObj );
   //   dataObj.WriteAscii( "C:\\envision\\studyareas\\wesp\\distance.csv" );
   /////////////////////

   // figure out which stream layer reaches to actually use in the model
   if (m_pStreamQE != NULL && m_streamQuery.GetLength() > 0)
      {
      CString source("Flow Stream Query");

      m_pStreamQuery = m_pStreamQE->ParseQuery(m_streamQuery, 0, source);

      if (m_pStreamQuery == NULL)
         {
         CString msg("Flow: Error parsing stream query: '");
         msg += m_streamQuery;
         msg += "' - the query will be ignored";
         Report::ErrorMsg(msg);
         }
      }

   // build the network topology for the stream reaches
   int nodeCount = m_reachTree.BuildTree(m_pStreamLayer, &Reach::CreateInstance, m_colStreamFrom, m_colStreamTo, m_colStreamJoin /*in*/, NULL /*m_pStreamQuery*/);   // jpb 10/7

   if (nodeCount < 0)
      {
      CString msg;
      msg.Format("Flow: Error building reach topology:  Error code %i", nodeCount);
      Report::ErrorMsg(msg);
      return false;
      }

   // populate stream order if column provided
   if (m_colReachSTRM_ORDER >= 0)
      m_reachTree.PopulateOrder(m_colReachSTRM_ORDER, true);

   // populate TreeID if specified
   if (m_colTreeID >= 0)
      {
      m_pStreamLayer->SetColNoData(m_colTreeID);
      int roots = m_reachTree.GetRootCount();

      for (int i = 0; i < roots; i++)
         {
         ReachNode *pRoot = m_reachTree.GetRootNode(i);
         PopulateTreeID(pRoot, i);
         }
      }

   // for each tree, add Reaches to our internal array of Reach ptrs
   int roots = m_reachTree.GetRootCount();

   for (int i = 0; i < roots; i++)
      {
      ReachNode *pRoot = m_reachTree.GetRootNode(i);
      ReachNode *pReachNode = (Reach*)m_reachTree.FindLeftLeaf(pRoot);  // find leftmost leaf of entire tree
      Reach* pReach = (Reach*)pReachNode;
      m_reachArray.Add(pReach);   // add leftmost node
      int reach_array_ndx = (int)m_reachArray.GetSize() - 1;
      pReach->m_reachArrayNdx = reach_array_ndx;

      while (pReachNode)
         {
         ReachNode *pDownNode = pReachNode->m_pDown;

         if (pDownNode == NULL) // root of tree reached?
            break;

         if ((pDownNode->m_pLeft == pReachNode) && (pDownNode->m_pRight != NULL))
            pReachNode = m_reachTree.FindLeftLeaf(pDownNode->m_pRight);
         else
            pReachNode = pDownNode;

         if (pReachNode->IsPhantomNode() == false)
            {
            pReach = (Reach*)pReachNode;
            m_reachArray.Add(pReach);   // add leftmost node
            int reach_array_ndx = (int)m_reachArray.GetSize() - 1;
            pReach->m_reachArrayNdx = reach_array_ndx;
            }
         }
      }

   // allocate subnodes to each reach
   m_reachTree.CreateSubnodes(&ReachSubnode::CreateInstance, m_subnodeLength);
   // Initialize subnode member data with values which will work if there is no initial conditions file.
   int reachCount = (int)m_reachArray.GetSize();
   for (int i = 0; i < reachCount; i++)
      {
      Reach *pReach = m_reachArray[i];
      m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachREACH_NDX, i);

      int num_subreaches = pReach->GetSubnodeCount();

      double reach_min_width_m = 0.;
      m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachWIDTH_MIN, reach_min_width_m);
      if (reach_min_width_m <= 0.)
      {
         reach_min_width_m = (double)pReach->NominalMinWidth_m();
         m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachWIDTH_MIN, reach_min_width_m);
      }

      double reach_min_depth_m = 0.;
      m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachDEPTH_MIN, reach_min_depth_m);
      if (reach_min_depth_m <= 0.)
      {
         reach_min_depth_m = reach_min_width_m / pReach->m_wdRatio;
         m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachDEPTH_MIN, reach_min_depth_m);
      }

      double reach_z_min_m = 0.;
      m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachZ_MIN, reach_z_min_m);
      ASSERT(-100. < reach_z_min_m && reach_z_min_m < 10000.);
      double reach_z_max_m = 0.;
      m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachZ_MAX, reach_z_max_m);
      ASSERT(-100. < reach_z_max_m && reach_z_max_m < 10000. && reach_z_min_m <= reach_z_max_m);
      double subreach_rise_m = (reach_z_max_m - reach_z_min_m) / num_subreaches;

      double reach_q_min_cms = 0.;
      m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachQ_MIN, reach_q_min_cms);
      if (reach_q_min_cms <= 0.)
      {
         reach_q_min_cms = pReach->NominalLowFlow_cms();
         m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachQ_MIN, reach_q_min_cms);
      }

      double Q = reach_q_min_cms;
      double manning_depth_m = GetManningDepthFromQ(pReach, Q, pReach->m_wdRatio);
      double width_x_length_accum_m2 = 0;

      for (int j = 0; j < num_subreaches; j++)
         { // Initialize state variables.
         ReachSubnode * pSubreach = pReach->GetReachSubnode(j);
         pSubreach->m_discharge = Q;
         pSubreach->m_dischargeWP = WaterParcel(Q * (double)SEC_PER_DAY, DEFAULT_REACH_H2O_TEMP_DEGC);
         pSubreach->m_dischargeDOY = -1;

         if (m_reachSvCount > 1)
            { // Initialize extra state variables
            pSubreach->AllocateStateVars(m_reachSvCount - 1);
            for (int k = 0; k < m_reachSvCount - 1; k++)
               {
               pSubreach->m_svArray[k] = 0.0f;
               pSubreach->m_svArrayTrans[k] = 0.0f;
               }
            } // end of logic for extra state variables

         pSubreach->m_subreach_length_m = pReach->m_length / num_subreaches; // ??? Ultimately we'll get this from the points which define the reach?
         pSubreach->m_min_width_m = reach_min_width_m;
         pSubreach->m_min_depth_m = reach_min_depth_m; 
         pSubreach->m_min_volume_m3 = pSubreach->m_subreach_length_m * pSubreach->m_min_width_m * pSubreach->m_min_depth_m;
         pSubreach->m_min_surf_area_m2 = pSubreach->m_subreach_length_m * pSubreach->m_min_width_m;
         pSubreach->m_wd_ratio = pReach->m_wdRatio;

         pSubreach->m_manning_depth_m = manning_depth_m;
         pSubreach->m_subreach_depth_m = pSubreach->m_min_depth_m + pSubreach->m_manning_depth_m;
         pSubreach->m_subreach_width_m = pReach->m_wdRatio * pSubreach->m_subreach_depth_m;
         width_x_length_accum_m2 += pSubreach->m_subreach_width_m * pSubreach->m_subreach_length_m;
         pSubreach->m_subreach_surf_area_m2 = pSubreach->m_subreach_width_m * pSubreach->m_subreach_length_m;
         double subreach_volume_m3 = pSubreach->m_subreach_length_m * pSubreach->m_subreach_width_m * pSubreach->m_subreach_depth_m;
         pSubreach->m_waterParcel = WaterParcel(subreach_volume_m3, DEFAULT_REACH_H2O_TEMP_DEGC);
         pSubreach->m_previousWP = pSubreach->m_waterParcel;

         pSubreach->m_midpt_elev_mASL = reach_z_min_m + (j + 0.5) * subreach_rise_m; // ??? placeholder
         pSubreach->m_aspect_deg = 90.; // ??? placeholder
         pSubreach->m_aspect_cat = 1; // ??? placeholder
         pSubreach->m_topo_shade = 0; // ??? placeholder
         pSubreach->m_veg_shade = 0; // ??? placeholder
         pSubreach->m_bank_shade = 0; // ??? placeholder
         }  // end of loop through subreaches of this reach

      double width_calc_m = width_x_length_accum_m2 / pReach->m_length;
      m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachWIDTH_CALC, width_calc_m);
      double width_given_m = pReach->Att(ReachWIDTHGIVEN);
      double width_reach_m = (width_given_m > 0. ) ? width_given_m : width_calc_m;
      pReach->SetAtt(ReachWIDTHREACH, width_reach_m);
      ASSERT(width_reach_m > 0.);

      } // end of loop through reaches

   int polyCount = m_pStreamLayer->GetPolygonCount();

   // Populate the 6 attributes in the stream layer which reflect how the reaches are connected.
   // Each reach has 3 possible connections: down (downstream), left (upstream left branch), and right (upstream right branch).
   // Root nodes lack a downstream connection.  Downstream connections are identified by the TNODE_ attribute, which contains
   // a node number matched in the FNODE_ attribute of the downstream reach.  Node numbers are not necessarily the same as reach
   // indices.
   // Headwater reaches lack left and right connections.
   // If there is only one upstream connection, it is, by convention, the left connection.
   // Stream.dbf is assumed to have its TNODE_ and COMID attributes already populated, since they are used to build the tree.
   // This logic will populate the COMID attributes for the down, left, and right connections, and also attributes holding
   // the OBJECTID values of the down, left, and right connections: 
   // COMID_DOWN, COMID_LEFT, COMID_RT
   // DOBJECTID, LOBJECTID, and ROBJECTID
      {
      CString msg;
      msg.Format("*** InitReaches(): Starting to populate the connection attributes now...");
      Report::LogMsg(msg);
      }

   bool readOnlyFlag = m_pStreamLayer->m_readOnly; m_pStreamLayer->m_readOnly = false;
   m_pStreamLayer->SetColData(m_colStreamDOBJECTID, -1, true);
   m_pStreamLayer->SetColData(m_colStreamLOBJECTID, -1, true);
   m_pStreamLayer->SetColData(m_colStreamROBJECTID, -1, true);
   m_pStreamLayer->SetColData(m_colStreamCOMID_DOWN, 0, true);
   m_pStreamLayer->SetColData(m_colStreamCOMID_LEFT, 0, true);
   m_pStreamLayer->SetColData(m_colStreamCOMID_RT, 0, true);
   m_pReachLayer->SetColDataU(m_colReachOUT_LATERL, 0);
   m_pReachLayer->SetColDataU(m_colReachFLOOD_CMS, 0);
   m_pReachLayer->SetColDataU(m_colReachFLOOD_C, 0);

   roots = m_reachTree.GetRootCount();
   for (int i = 0; i < roots; i++)
      {
      ReachNode *pRoot = m_reachTree.GetRootNode(i);
      ReachNode *pReachNode = (Reach*)m_reachTree.FindLeftLeaf(pRoot);  // find leftmost leaf of entire tree

      while (pReachNode)
         {
         ReachNode *pDownNode = pReachNode->m_pDown;
         int polyIndex = pReachNode->m_polyIndex;
         if (polyIndex >= 0 && polyIndex < polyCount)
            {
            int comid = -1; m_pStreamLayer->GetData(polyIndex, m_colStreamCOMID, comid);
            int downNdx = pDownNode != NULL ? pDownNode->m_polyIndex : -1; 
            int comid_down = 0;
            int orig_comid_down = 0; m_pStreamLayer->GetData(polyIndex, m_colStreamCOMID_DOWN, orig_comid_down);
            int dobjectid = -1;
            int orig_dobjectid = -1; m_pStreamLayer->GetData(polyIndex, m_colStreamDOBJECTID, orig_dobjectid);
            if (downNdx >= 0 && downNdx < polyCount)
               {

               int tnode = -1; m_pStreamLayer->GetData(polyIndex, m_colStreamTo, tnode);
               int down_fnode = -1; m_pStreamLayer->GetData(downNdx, m_colStreamFrom, down_fnode);
               if (down_fnode != tnode)
                  {
                  CString msg;
                  msg.Format("*** InitReaches(): tnode != down_fnode. i = %d, polyIndex = %d, tnode = %d, downNdx = %d, down_fnode = %d",
                     i, polyIndex, tnode, downNdx, down_fnode);
                  Report::LogMsg(msg);
                  }

               m_pStreamLayer->GetData(downNdx, m_colStreamOBJECTID, dobjectid);
               if (orig_dobjectid != -1)
                  {
                  CString msg; msg.Format("*** InitReaches(): orig_dobjectid != -1  polyIndex = %d, orig_dobjectid = %d, dobjectid = %d",
                     polyIndex, orig_dobjectid, dobjectid); Report::LogMsg(msg);
                  }

               m_pStreamLayer->GetData(downNdx, m_colStreamCOMID, comid_down);
               if (orig_comid_down != 0)
                  {
                  CString msg; msg.Format("*** InitReaches(): orig_comid_down != 0  polyIndex = %d, orig_comid_down = %d, comid_down = %d",
                     polyIndex, orig_comid_down, comid_down); Report::LogMsg(msg);
                  }
               }
            m_pStreamLayer->SetData(polyIndex, m_colStreamDOBJECTID, dobjectid);
            m_pStreamLayer->SetData(polyIndex, m_colStreamCOMID_DOWN, comid_down);

            ReachNode * pLeftNode = pReachNode->m_pLeft;
            int lobjectid = -1;
            int orig_lobjectid = -1; m_pStreamLayer->GetData(polyIndex, m_colStreamLOBJECTID, orig_lobjectid);
            int comid_left = 0;
            int orig_comid_left = 0; m_pStreamLayer->GetData(polyIndex, m_colStreamCOMID_LEFT, orig_comid_left);
            int leftNdx = pLeftNode != NULL ? pLeftNode->m_polyIndex : -1;
            if (leftNdx >= 0 && leftNdx < polyCount)
               {
               m_pStreamLayer->GetData(leftNdx, m_colStreamOBJECTID, lobjectid);
               if (orig_lobjectid != -1)
                  {
                  CString msg; msg.Format("*** InitReaches(): orig_lobjectid != -1  polyIndex = %d, orig_lobjectid = %d, lobjectid = %d",
                     polyIndex, orig_lobjectid, lobjectid); Report::LogMsg(msg); 
                  }
               m_pStreamLayer->GetData(leftNdx, m_colStreamCOMID, comid_left);
               if (orig_comid_left != 0)
                  {
                  CString msg; msg.Format("*** InitReaches(): orig_comid_left != 0  polyIndex = %d, orig_comid_left = %d, comid_left = %d",
                     polyIndex, orig_comid_left, comid_left); Report::LogMsg(msg);
                  }
               }
            m_pStreamLayer->SetData(polyIndex, m_colStreamLOBJECTID, lobjectid);
            m_pStreamLayer->SetData(polyIndex, m_colStreamCOMID_LEFT, comid_left);

            ReachNode * pRtNode = pReachNode->m_pRight;
            int robjectid = -1;
            int orig_robjectid = -1; m_pStreamLayer->GetData(polyIndex, m_colStreamROBJECTID, orig_robjectid);
            int comid_rt = 0;
            int orig_comid_rt = 0; m_pStreamLayer->GetData(polyIndex, m_colStreamCOMID_RT, orig_comid_rt);
            int rightNdx = pRtNode != NULL ? pRtNode->m_polyIndex : -1;
            if (rightNdx >= 0 && rightNdx < polyCount)
               {
               m_pStreamLayer->GetData(rightNdx, m_colStreamOBJECTID, robjectid);
               if (orig_robjectid != -1)
                  {
                  CString msg; msg.Format("*** InitReaches(): orig_robjectid != -1  polyIndex = %d, orig_robjectid = %d, robjectid = %d",
                     polyIndex, orig_robjectid, robjectid); Report::LogMsg(msg);
                  }
               m_pStreamLayer->GetData(rightNdx, m_colStreamCOMID, comid_rt);
               if (orig_comid_rt != 0)
                  {
                  CString msg; msg.Format("*** InitReaches(): orig_comid_rt != 0  polyIndex = %d, orig_comid_rt = %d, comid_rt = %d",
                     polyIndex, orig_comid_rt, comid_rt); Report::LogMsg(msg);
                  }
               }
            m_pStreamLayer->SetData(polyIndex, m_colStreamROBJECTID, robjectid);
            m_pStreamLayer->SetData(polyIndex, m_colStreamCOMID_RT, comid_rt);

            if ((comid_left <= 0 && comid_rt > 0) || (comid_left > 0 && comid_rt == comid_left))
               {
               CString msg; 
               msg.Format("InitReaches() comid %d has comid_left = %d and comid_rt = %d", comid, comid_left, comid_rt);
               Report::WarningMsg(msg);
               }
            }

         if (pDownNode == NULL) // root of tree reached?
            break;

         if ((pDownNode->m_pLeft == pReachNode) && (pDownNode->m_pRight != NULL))
            pReachNode = m_reachTree.FindLeftLeaf(pDownNode->m_pRight);
         else
            pReachNode = pDownNode;

         } // end of while (pReachNode), the loop that traverses all the branches in this one tree
      } // end of for (int i = 0; i < roots; i++), the loop through all the separate trees
   m_pStreamLayer->m_readOnly = readOnlyFlag;

   /* Look for non-unique COMIDs.
         {
         CString msg;
         msg.Format("*** InitReaches(): Starting to check for non-unique COMIDs now...");
         Report::LogMsg(msg);
         }
   int reachNdx1 = 0;
   while (reachNdx1 < polyCount - 1)
      {
      int comid1 = 0; m_pStreamLayer->GetData(reachNdx1, m_colStreamCOMID, comid1);
      int reachNdx2 = reachNdx1 + 1;
      while (reachNdx2 < polyCount)
         {
         int comid2 = 0; m_pStreamLayer->GetData(reachNdx2, m_colStreamCOMID, comid2);
         if (comid2 == comid1)
            {
            CString msg;
            msg.Format("InitReaches() 2 reaches have comid = %d. reachNdx1 = %d, reachNdx2 = %d", comid1, reachNdx1, reachNdx2);
            Report::ErrorMsg(msg);

            int r1X0, r1Y0, r2X0, r2Y0;
            m_pStreamLayer->GetData(reachNdx1, m_colStreamSTRMVERT0X, r1X0);
            m_pStreamLayer->GetData(reachNdx1, m_colStreamSTRMVERT0Y, r1Y0);
            m_pStreamLayer->GetData(reachNdx2, m_colStreamSTRMVERT0X, r2X0);
            m_pStreamLayer->GetData(reachNdx2, m_colStreamSTRMVERT0Y, r2Y0);
            float dx = (float)(r1X0 - r2X0);
            float dy = (float)(r1Y0 - r2Y0);
            float distance = sqrt(dx*dx + dy*dy);
            int object_id1 = -1; m_pStreamLayer->GetData(reachNdx1, m_colStreamOBJECTID, object_id1);
            int object_id2 = -1; m_pStreamLayer->GetData(reachNdx2, m_colStreamOBJECTID, object_id2);

            // Are they connected?
            // Is vertex 0 of reach 1 the same as vertex 0 of reach 2? and so on
            Poly *pReachPoly1 = m_pStreamLayer->GetPolygon(reachNdx1);
            Vertex r1v0 = pReachPoly1->m_vertexArray.GetAt(0);
            int r1last = pReachPoly1->GetVertexCount() - 1;
            Vertex r1vn = pReachPoly1->m_vertexArray.GetAt(r1last);
            Poly *pReachPoly2 = m_pStreamLayer->GetPolygon(reachNdx2);
            Vertex r2v0 = pReachPoly2->m_vertexArray.GetAt(0);
            int r2last = pReachPoly2->GetVertexCount() - 1;
            Vertex r2vn = pReachPoly2->m_vertexArray.GetAt(r2last);
            float distance2 = (float)pReachPoly2->NearestDistanceToPoly(pReachPoly1);
            bool connected = distance2 <= 1.f; // meter // r1v0 == r2v0 || r1v0 == r2vn || r1vn == r2v0 || r1vn == r2vn;
            bool downstream_branch = false;
            if (connected)
               { // They're connected.  Does the downstream reach of the two connected reaches with the same COMID have a branch?
               // Which is the downstream reach?
               int downstream_reachNdx = reachNdx1; // Assume the downstream reach is "reach1".
               int upstream_reachNdx = reachNdx2;
               int upstream_dobjectid = -1; m_pStreamLayer->GetData(upstream_reachNdx, m_colStreamDOBJECTID, upstream_dobjectid);
               int downstream_objectid = -1; m_pStreamLayer->GetData(downstream_reachNdx, m_colStreamOBJECTID, downstream_objectid);
               if (upstream_dobjectid != downstream_objectid)
                  { // The assumption was false, so now assume the downstream reach is "reach2".
                  int downstream_reachNdx = reachNdx2; // Assume the downstream reach is "reach1".
                  int upstream_reachNdx = reachNdx1;
                  int upstream_dobjectid = -1; m_pStreamLayer->GetData(upstream_reachNdx, m_colStreamDOBJECTID, upstream_dobjectid);
                  int downstream_objectid = -1; m_pStreamLayer->GetData(downstream_reachNdx, m_colStreamOBJECTID, downstream_objectid);
                  if (upstream_dobjectid != downstream_objectid) connected = false; // They're not connected after all.
                  }
               if (connected)
                  { // They are connected, and we know which one is downstream.  Does it have a branch?
                  int downstream_robjectid = -1; m_pStreamLayer->GetData(downstream_reachNdx, m_colStreamROBJECTID, downstream_robjectid);
                  downstream_branch = downstream_robjectid >= 0;
                  }
               }

            if (!connected || downstream_branch)
               {
               CString msg; msg.Format("*** InitReaches() Non-unique COMIDs.  COMID = %d, reachNdx1 = %d, OBJECTID1 = %d, reach1X = %d, reach1Y = %d, "
                  "reachNdx2 = %d, OBJECTID2 = %d, reach2X = %d, reach2Y = %d, distance = %f %f %s %s",
                  comid1, reachNdx1, object_id1, r1X0, r1Y0, reachNdx2, object_id2, r2X0, r2Y0, distance, distance2,
                  connected ? "connected" : "not connected", connected && downstream_branch ? "with downstream branch" : "");
               Report::LogMsg(msg);
               }
            } // end of if (comid2 == comid1)
         reachNdx2++;
         }
      reachNdx1++;
      }
*/

   // Make sure each reach is associated with an IDU, and populate the TopoSetting member object.
   if (m_flowContext.pEnvContext->coldStartFlag) m_pReachLayer->SetColDataU(m_colStreamIDU_ID, -1);
   for (int reach_array_ndx = 0; reach_array_ndx < reachCount; reach_array_ndx++)
   {
      Reach * pReach = m_reachArray[reach_array_ndx];
      int idu_id = -1; m_pReachLayer->GetData(pReach->m_polyIndex, m_colStreamIDU_ID, idu_id);
      if (idu_id < 0)
      {
         int idu_ndx = GetIDUndxForReach(pReach); // GetIDUndxForReach() determines IDU_ID and writes it to the Reach layer.
         ASSERT(idu_ndx >= 0);
      } // end of if (idu_id < 0)

      double topo_elev_E_deg; m_pReachLayer->GetData(pReach->m_polyIndex, m_colReachTOPOELEV_E, topo_elev_E_deg);
      double topo_elev_S_deg; m_pReachLayer->GetData(pReach->m_polyIndex, m_colReachTOPOELEV_S, topo_elev_S_deg);
      double topo_elev_W_deg; m_pReachLayer->GetData(pReach->m_polyIndex, m_colReachTOPOELEV_W, topo_elev_W_deg);

      double elev_m2; m_pReachLayer->GetData(pReach->m_polyIndex, m_colReachZ_MEAN, elev_m2);
      pReach->m_topo = TopoSetting(elev_m2, topo_elev_E_deg, topo_elev_S_deg, topo_elev_W_deg);
   } // end of loop thru m_reachArray

   if (m_flowContext.pEnvContext->coldStartFlag)
   { // Populate the RIVER_KM attribute.
      m_pStreamLayer->SetColDataU(m_colReachRIVER_KM, -1.);
      for (int i = 0; i < roots; i++)
      {
         ReachNode* pRoot = m_reachTree.GetRootNode(i);
         PopulateRIVER_KM(pRoot);
      }
   } // end of if (m_flowContext.pEnvContext->coldStartFlag)

   m_pReachLayer->SetColDataU(m_colReachSAL_REACH, false);

   return true;
   } // end of InitReaches()


   void FlowModel::PopulateRIVER_KM(ReachNode * pRoot)
   { // Assumes that RIVER_KM has been filled with negative place holders.
      Reach* pReach = GetReachFromNode(pRoot);
      double river_km = 0.; // the downstream end of pReach

      // Walk the reach tree, filling in the RIVER_KM attribute.
      while (pReach != NULL)
      {
         m_pStreamLayer->SetDataU(pReach->m_polyIndex, m_colReachRIVER_KM, river_km);

         // Go up the tree if possible
         if (pReach->m_pLeft != NULL)
         {
            river_km += pReach->m_length / 1000.;
            pReach = GetReachFromNode(pReach->m_pLeft);
            continue;
         }
         else if (pReach->m_pRight != NULL)
         {
            river_km += pReach->m_length / 1000.;
            pReach = GetReachFromNode(pReach->m_pRight);
            continue;
         }

         // Couldn't go up. Back down, looking for places where RIVER_KM hasn't been filled in yet.
         Reach* orig_pReach = pReach;
         pReach = GetReachFromNode(pReach->m_pDown);
         while (pReach != NULL)
         {
            double check_river_km = river_km - pReach->m_length / 1000.;

            m_pStreamLayer->GetData(pReach->m_polyIndex, m_colReachRIVER_KM, river_km);
            ASSERT(close_enough(check_river_km, river_km, 1e-7, 1e-7));

            Reach * new_pReach = GetReachFromNode(pReach->m_pLeft);
            if (new_pReach == NULL || new_pReach == orig_pReach) new_pReach = GetReachFromNode(pReach->m_pRight);

            if (new_pReach != NULL && new_pReach != orig_pReach)
            { // new_pReach might be a branch where RIVER_KM hasn't been filled in yet.
               double new_river_km = 0.; m_pStreamLayer->GetData(new_pReach->m_polyIndex, m_colReachRIVER_KM, new_river_km);
               if (new_river_km < 0.)
               { // RIVER_KM hasn't been filled in yet.
                  river_km += pReach->m_length / 1000.;
                  pReach = new_pReach;
                  break; // exit from inner while loop to outer while loop
               }
            }
            
            orig_pReach = pReach;
            pReach = GetReachFromNode(pReach->m_pDown); // back down one more level
         } // end of while (pReach != NULL) backing down the tree

      } // end of while (pReach != NULL) walking the tree

      ASSERT(river_km == 0.);
   } // end of PopulateRIVER_KM()


void FlowModel::PopulateTreeID( ReachNode *pNode, int treeID )
   {
   if ( pNode == NULL )
      return;

   if ( pNode->m_polyIndex < 0 )
      return;

   m_pStreamLayer->SetData( pNode->m_polyIndex, m_colTreeID, treeID );

   // recurse up tree
   PopulateTreeID( pNode->m_pLeft, treeID );
   PopulateTreeID( pNode->m_pRight, treeID );
   }


// create, initialize reaches based on the stream layer
bool FlowModel::InitCatchments( void )
   {  
   if (m_flowContext.pEnvContext->coldStartFlag) 
      {
      BuildCatchmentsFromQueries();
      CString msg; msg.Format("ColdStart: Calling BuildCatchmentsFromQueries() instead of BuildCatchmentsFromHRUlayer()");
      Report::LogMsg(msg);
      }
   else BuildCatchmentsFromHRUlayer();

  // SetAllCatchmentAttributes();     // populates internal HRU members (area, elevation, centroid) for all catchments

   // allocate any necessary "extra" state variables
   if ( m_hruSvCount > 1 )//then there are extra state variables..
      {
      int catchmentCount = (int) m_catchmentArray.GetSize();
      for ( int i=0; i < catchmentCount; i++)
         {
         Catchment *pCatchment = m_catchmentArray[ i ];
         HRU *pHRU = NULL;

         for ( int j=0; j < pCatchment->GetHRUCountInCatchment(); j++ )
            {
            HRU *pHRU = pCatchment->GetHRUfromCatchment( j );

            for (int k=0; k < pHRU->GetLayerCount(); k++ )
               {
               HRULayer *pLayer = pHRU->GetLayer( k );
               pLayer->AllocateStateVars( m_hruSvCount-1 );
               }
            }
         }
      }

   return true;
   }


bool FlowModel::SetAllCatchmentAttributes(void)
   {
   if ( !m_pGrid)   // if ( m_buildCatchmentsMethod < 2 )
      {
      double area_below_500m = 0.;
      double area_500to1200m = 0.;
      double area_at_or_above_1200m = 0.;

      int catchmentCount = (int) m_catchmentArray.GetSize();
      for (int i=0; i< catchmentCount; i++)
         {
         Catchment *pCatchment = m_catchmentArray[ i ];
         SetHRUAttributes( pCatchment );
         for (int j = 0; j < pCatchment->GetHRUCountInCatchment(); j++)
            {
            HRU *pHRU = pCatchment->GetHRUfromCatchment(j);

            if (pHRU->m_elevation < 500.) area_below_500m += pHRU->m_HRUtotArea_m2;
            else if (pHRU->m_elevation < 1200.) area_500to1200m += pHRU->m_HRUtotArea_m2;
            else area_at_or_above_1200m += pHRU->m_HRUtotArea_m2;
            } // end of loop thru HRUs in ith catchment 
         } // end of loop thru catchments

      CString msg;
      msg.Format("*** SetHRUAttributes() area_below_500m = %f, area_500to1200m = %f, area_at_or_above_1200m = %f", area_below_500m, area_500to1200m, area_at_or_above_1200m);
      Report::LogMsg(msg);
      } // end of if (!m_pGrid) ...

   return true;
   }


bool FlowModel::SetHRUAttributes( Catchment *pCatchment )
   {
   HRU *pHRU = NULL;
   pCatchment->m_area=0.0f;
   for ( int j=0; j < pCatchment->GetHRUCountInCatchment(); j++ )
      {
      float area=0.0f; float elevation=0.0f;float _elev=0.0f;
      HRU *pHRU = pCatchment->GetHRUfromCatchment( j );

      pHRU->m_HRUtotArea_m2 = 0;
      pHRU->m_wetlandArea_m2 = 0;

      for (int k=0; k < pHRU->m_polyIndexArray.GetSize();k++)
         {
         m_pCatchmentLayer->GetData( pHRU->m_polyIndexArray[k], m_colCatchmentArea, area );
         m_pCatchmentLayer->GetData( pHRU->m_polyIndexArray[k], m_colElev, _elev );
         pHRU->m_HRUtotArea_m2 += area;
         elevation    += _elev*area; 
         pCatchment->m_area += area;

         int lulc_a = AttInt(pHRU->m_polyIndexArray[k], LULC_A);
         bool is_wetland = lulc_a == LULCA_WETLAND;
         if (is_wetland) pHRU->m_wetlandArea_m2 += area;
      } // end of loop through the IDUs in this HRU

      pHRU->m_elevation = elevation / pHRU->m_HRUtotArea_m2;

      if (pHRU->m_polyIndexArray.GetSize() <= 0)
      {
         CString msg;
         msg.Format("SetHRUAttributes() pHRU->m_polyIndexArray.GetSize() = %d, pHRU->m_id = %d, pHRU->m_hruNdx = %d",
            pHRU->m_polyIndexArray.GetSize(), pHRU->m_id, pHRU->m_hruNdx);
         Report::ErrorMsg(msg);
      }
      Poly *pPoly = m_pHRUlayer->GetPolygon(pHRU->m_hruNdx);
      Vertex centroid = pPoly->GetCentroid();
      pHRU->m_centroid = centroid;
      }
  
   return true;
   }

   // If we are using grids as the basis for simulation, assume that each grid cell represents an HRU while each catchment is still a catchment
   // 
   // Assume that the DEM is consistent with the catchment delinations in the IDUs, so that we can take a grid cell centroid and associate
   // the catchment at that point with that particular grid cell
   // 
   // Catchments are collections of grid cells (HRUs) that still have HRULayers
   // 
   // 

bool FlowModel::BuildCatchmentsFromGrids( void )
   {
   ParseCols( m_pCatchmentLayer, m_catchmentAggCols, m_colsCatchmentAgg );
   ParseCols( m_pCatchmentLayer, m_hruAggCols, m_colsHruAgg );
   int colCatchID=m_pCatchmentLayer->m_pData->GetCol(m_catchIDCol);

   // first, do catchments
   CArray< Query*, Query* > catchmentQueryArray;
   int queryCount = GenerateAggregateQueries( m_pCatchmentLayer, m_colsCatchmentAgg, catchmentQueryArray );

   CArray< Query*, Query* > hruQueryArray;
   int hruQueryCount = GenerateAggregateQueries( m_pCatchmentLayer, m_colsHruAgg, hruQueryArray );

   // temporary allocations
   Catchment **catchments = new Catchment*[ queryCount ];  // placeholder for catchments
   memset( catchments, 0, queryCount*sizeof( Catchment* ) );  // initially, all NULL

   m_pCatchmentLayer->SetColNoData( m_colCatchmentCatchID );
   m_pCatchmentLayer->SetColNoData( m_colCatchmentHruID );

   int hruCols = (int) m_colsHruAgg.GetSize();
   VData *hruValueArray = new VData[ hruCols ];

   int polyCount = m_pCatchmentLayer->GetPolygonCount();
   int hruCount = 0;
   
   if ( m_pCatchmentQuery != NULL )
      polyCount = m_pCatchmentQuery->Select( true );

   // iterate though polygons that satisfy the catchment query.
   for ( int i=0; i < polyCount; i++ )
      {
      int polyIndex = m_pCatchmentQuery ? m_pCatchmentLayer->GetSelection( i ) : i;

      // get hru aggregate values from this poly
      for ( int h=0; h < hruCols; h++ )
         {
         bool ok = m_pCatchmentLayer->GetData( polyIndex, m_colsHruAgg[ h ], hruValueArray[ h ] );
         if ( ! ok )
            hruValueArray[ h ].SetNull();
         }

      // iterate through the catchment aggregate queries, to see if this polygon satisfies any of them
      for ( int j=0; j < queryCount; j++ )
         {
         Query *pQuery = catchmentQueryArray.GetAt( j );
         bool result = false;
         bool ok = pQuery->Run( polyIndex, result );

         if ( ok && result )  // is this polygon in a the specified Catchment (query?) - yes!
            {
            if ( catchments[ j ] == NULL )    // if first one, creaet a Catchment object for it
               {
               catchments[ j ] = new Catchment;
               catchments[ j ]->m_id = j;
               }
            m_pCatchmentLayer->SetData(i,colCatchID,j);
            }  // end of: if ( ok && result )  meaning adding polyIndex to catchment
         }  // end of: for ( j < queryCount )    
      }  // end of: for ( i < polyCount )

   // store catchments
   for ( int j=0; j < queryCount; j++ )
      {
      Catchment *pCatchment = catchments[ j ];
      if ( pCatchment != NULL )
         m_catchmentArray.Add( pCatchment );
      }

   // clean up
   delete [] catchments;
   delete [] hruValueArray;

   if ( m_pCatchmentQuery != NULL )
      m_pCatchmentLayer->ClearSelection();
  
   //Catchments are allocated, now we need to iterate the grid allocating an HRU for every grid cell.
   HRU *pHRU = NULL;
   hruCount=0;
   int foundHRUs=0;
   float minElev=1E10;

   m_pHruGrid = new IDataObj(m_pGrid->GetColCount(), m_pGrid->GetRowCount());

   for ( int i=0; i < m_pGrid->GetRowCount(); i++ )
      {
      for ( int j=0; j < m_pGrid->GetColCount(); j++ )
         {
         REAL x=0, y=0;
         int index=-1; 
         m_pGrid->GetGridCellCenter(i,j,x,y);
         Poly *pPoly = m_pCatchmentLayer->GetPolygonFromCoord(x,y,&index);
         float elev = 0.0f;
         
         m_pGrid->GetData( i, j, elev);
         if (elev == m_pGrid->GetNoDataValue())
            m_pHruGrid->Set(j, i, m_pGrid->GetNoDataValue());
         else 
            {
            if ( pPoly )      // found an IDU at the location provided, try to find a catchment 
               {
               for ( int k=0; k < (int) m_catchmentArray.GetSize(); k++ ) //figure out which catchment this HRU is in!
                  {
                  Catchment *pCatchment = m_catchmentArray[ k ];
                  ASSERT( pCatchment != NULL );

                  int catchID=-1;
                  m_pCatchmentLayer->GetData(index, colCatchID, catchID );
               
                  if ( catchID == pCatchment->m_id )//if the catchments ReachID equals this polygons reachID
                     {
                     pHRU = new HRU;
                     pHRU->m_id = hruCount;
                     pHRU->m_demRow = i;
                     pHRU->m_demCol = j;
                     pHRU->m_HRUtotArea_m2 = (float)(m_pGrid->GetGridCellWidth()*m_pGrid->GetGridCellWidth());
                     REAL x=0.0;REAL y=0.0;
                     m_pGrid->GetGridCellCenter(i,j,x,y);
                     pHRU->m_centroid.x=x;
                     pHRU->m_centroid.y=y;                  
                  
                     float elev = 0.0f;
                     m_pGrid->GetData( i, j, elev);
                     pHRU->m_elevation = elev;
                     if (elev<minElev)
                        minElev=elev;
                     pHRU->m_aquifer_recharge_mm = 0.f;

                     pHRU->AddLayers( m_soilLayerCount, m_snowLayerCount, m_vegLayerCount, 0, m_initTemperature, true );

                     hruCount++;
      
                     AddHRU( pHRU, pCatchment );
                     int offset = (int)m_hruArray.GetSize() - 1;
                     m_pHruGrid->Set(j, i, offset);
                     pHRU->m_polyIndexArray.Add( index );
                     foundHRUs++;
                     break;
                     }
                  }
               }  // end of: if ( poly found )
            else//we have a grid cell that includes data, but its centoid does not overlay the IDUs.  This can happen, particularly with coarsened grids
               m_pHruGrid->Set(j,i, m_pGrid->GetNoDataValue());
            }
         }
      }  // end of: iterating through elevation grid
   m_minElevation=minElev;
   hruCount = GetHRUCount();  


   /*
   for ( int h=0; h < hruCount; h++ )
      {
      HRU *pHRU = GetHRU(h);
      int neighborCount=0;
      for ( int k=0; k < hruCount; k++ )
         {
         HRU *pNeighborHRU = GetHRU( k );
         ASSERT( pNeighborHRU != NULL );
            
         if (pHRU == pNeighborHRU )
            int stop=1;
         else if (( pNeighborHRU->m_demRow == pHRU->m_demRow-1 || pNeighborHRU->m_demRow == pHRU->m_demRow || pNeighborHRU->m_demRow == pHRU->m_demRow+1 )
                  && pNeighborHRU->m_demCol == pHRU->m_demCol-1 ) 
            {
            pHRU->m_neighborArray.Add( pNeighborHRU );
            neighborCount++;
            }
         else if ((pNeighborHRU->m_demRow == pHRU->m_demRow-1 || pNeighborHRU->m_demRow == pHRU->m_demRow+1) 
                  && pNeighborHRU->m_demCol == pHRU->m_demCol ) 
            {
            pHRU->m_neighborArray.Add( pNeighborHRU );
            neighborCount++;
            }
         else if ((pNeighborHRU->m_demRow == pHRU->m_demRow-1 || pNeighborHRU->m_demRow == pHRU->m_demRow || pNeighborHRU->m_demRow == pHRU->m_demRow+1)
                  && pNeighborHRU->m_demCol == pHRU->m_demCol+1 ) 
            {
            pHRU->m_neighborArray.Add( pNeighborHRU );
            neighborCount++;
            }
        
         }
         */
   /*
   for (int h = 0; h < hruCount; h++)
      {
      HRU *pHRU = GetHRU(h);
      int neighborCount = 0;
      for (int k = 0; k < hruCount; k++)//For this HRU, go through other hrus and identify those that are adjacent (neighbors)
         {
         HRU *pNeighborHRU = GetHRU(k);
         ASSERT(pNeighborHRU != NULL);
         bool add = false;
         if (h != k)//don't add the current hru to its own neighbor array
            {
            if ((pNeighborHRU->m_demRow == pHRU->m_demRow - 1 || pNeighborHRU->m_demRow == pHRU->m_demRow || pNeighborHRU->m_demRow == pHRU->m_demRow + 1)
               && pNeighborHRU->m_demCol == pHRU->m_demCol - 1)
               add = true;

            else if ((pNeighborHRU->m_demRow == pHRU->m_demRow - 1 || pNeighborHRU->m_demRow == pHRU->m_demRow + 1)
               && pNeighborHRU->m_demCol == pHRU->m_demCol)
               add = true;

            else if ((pNeighborHRU->m_demRow == pHRU->m_demRow - 1 || pNeighborHRU->m_demRow == pHRU->m_demRow || pNeighborHRU->m_demRow == pHRU->m_demRow + 1)
               && pNeighborHRU->m_demCol == pHRU->m_demCol + 1)
               add = true;
            }

         if (add)//this grid cell is adjacent to the current grid cell
            neighborCount++;

         }//end of loop through neighbors, for this HRU. All we did was count the number of neighbors.

      pHRU->m_neighborArray.SetSize(neighborCount);  //pre-allocate the array that will hold pointers to the neighbors
      */
   for (int h = 0; h < hruCount; h++)
      {
      HRU *pHRU = GetHRU(h);
      int neighborCount = 0;

      for (int k = 0; k < hruCount; k++) // go back through and add neighbor pointers to the neighbor array.
         {
         HRU *pNeighborHRU = GetHRU(k);
         ASSERT(pNeighborHRU != NULL);
         if (h != k)//don't add the current hru to its own neighbor array
            {
            if ((pNeighborHRU->m_demRow == pHRU->m_demRow - 1 || pNeighborHRU->m_demRow == pHRU->m_demRow || pNeighborHRU->m_demRow == pHRU->m_demRow + 1)
               && pNeighborHRU->m_demCol == pHRU->m_demCol - 1)
               {
               neighborCount++;
               pHRU->m_neighborArray.Add(pNeighborHRU);
               }

            else if ((pNeighborHRU->m_demRow == pHRU->m_demRow - 1 || pNeighborHRU->m_demRow == pHRU->m_demRow + 1)
               && pNeighborHRU->m_demCol == pHRU->m_demCol)
               {
               neighborCount++;
               pHRU->m_neighborArray.Add(pNeighborHRU);
               }

            else if ((pNeighborHRU->m_demRow == pHRU->m_demRow - 1 || pNeighborHRU->m_demRow == pHRU->m_demRow || pNeighborHRU->m_demRow == pHRU->m_demRow + 1)
               && pNeighborHRU->m_demCol == pHRU->m_demCol + 1)
               {
               neighborCount++;
               pHRU->m_neighborArray.Add(pNeighborHRU);
               }
            }
         }//end of loop through neighbors, for this HRU.  Next, allocate the water flux
      
      ASSERT(neighborCount<9);


      int layerCount = pHRU->GetLayerCount();
      for (int i = 0; i< layerCount ; i++)                  //Assume all other layers include exchange only in upward and downward directions
         {
         HRULayer *pHRULayer = pHRU->GetLayer(i);

         pHRULayer->m_waterFluxArray.SetSize(neighborCount + 7);

         for (int i = 0; i <neighborCount + 7; i++)
            pHRULayer->m_waterFluxArray[i] = 0.0f;

         }

      /*
      pHRU->m_waterFluxArray.SetSize( 7 );//add 2 to accomodate upward and downward fluxes
      for (int j = 0; j < 7; j++)
         pHRU->m_waterFluxArray[j] = 0.0f;

      int layerCount = pHRU->GetLayerCount();
      
      HRULayer *pHRULayer = pHRU->GetLayer(layerCount-1); //this is the bottom layer.  Assume it includes exchange between horizontal neighbors
      pHRULayer->m_waterFluxArray.SetSize( neighborCount+7 );

      for (int i=0; i <neighborCount+7; i++)
         pHRULayer->m_waterFluxArray[i]=0.0f;
      
      for (int i=0; i< layerCount-1;i++)                  //Assume all other layers include exchange only in upward and downward directions
         {
         HRULayer *pHRULayer1 = pHRU->GetLayer(i);
         pHRULayer1->m_waterFluxArray.SetSize( 7 );
         for (int j=0; j < 7; j++)
            pHRULayer1->m_waterFluxArray[j]=0.0f;
         }
      
      */

      }
   // Build topology between different HRUs
/*   for ( int i=0; i < m_catchmentArray.GetSize(); i++ ) //catchments
      {
      Catchment *pCatchment = m_catchmentArray[ i ];

      for ( int j=0; j < pCatchment->GetHRUCount(); j++ )
         {
         HRU *pHRU = pCatchment->GetHRU( j );
         ASSERT( pHRU != NULL );
         int neighborCount=0;
         for ( int k=0; k < pCatchment->GetHRUCount(); k++ )
            {
            HRU *pNeighborHRU = pCatchment->GetHRU( k );
            ASSERT( pNeighborHRU != NULL );
            
            if (pHRU == pNeighborHRU )
               ;
            else if (( pNeighborHRU->m_demRow == pHRU->m_demRow-1 || pNeighborHRU->m_demRow == pHRU->m_demRow || pNeighborHRU->m_demRow == pHRU->m_demRow+1 )
                     && pNeighborHRU->m_demCol == pHRU->m_demCol-1 ) 
               {
               pHRU->m_neighborArray.Add( pNeighborHRU );
               neighborCount++;
               }
            else if ((pNeighborHRU->m_demRow == pHRU->m_demRow-1 || pNeighborHRU->m_demRow == pHRU->m_demRow+1) 
                     && pNeighborHRU->m_demCol == pHRU->m_demCol ) 
               {
               pHRU->m_neighborArray.Add( pNeighborHRU );
               neighborCount++;
               }
            else if ((pNeighborHRU->m_demRow == pHRU->m_demRow-1 || pNeighborHRU->m_demRow == pHRU->m_demRow || pNeighborHRU->m_demRow == pHRU->m_demRow+1)
                     && pNeighborHRU->m_demCol == pHRU->m_demCol+1 ) 
               {
               pHRU->m_neighborArray.Add( pNeighborHRU );
               neighborCount++;
               }
            }
         ASSERT(neighborCount<9);
         pHRU->m_waterFluxArray = new float[neighborCount];//add 2 to accomodate upward and downward fluxes
         int layerCount = pHRU->GetLayerCount();
         HRULayer *pHRULayer = pHRU->GetLayer(layerCount-1); //Assume the bottom layer includes exchange between horizontal neighbors
         pHRULayer->m_waterFluxArray = new float[neighborCount+5];
         for (int i=0;i<neighborCount+5;i++)
            pHRULayer->m_waterFluxArray[i]=0.0f;
         for (int i=0; i< layerCount-1;i++)                  //Assume all other layers include exchange only in upward and downward directions
            {
            pHRULayer = pHRU->GetLayer(i);
            pHRULayer->m_waterFluxArray = new float[5];
            for (int j=0;j<5;j++)
               pHRULayer->m_waterFluxArray[j]=0.0f;
            }
         
         }
      }*/

   CString msg;
   msg.Format( _T("Flow is Going Grid!!: %i Catchments, %i HRUs built from %i polygons.  %i HRUs were associated with Catchments" ), (int) m_catchmentArray.GetSize(), hruCount, polyCount, foundHRUs ); 
   Report::LogMsg( msg, RT_INFO );
   return true;

   }

bool FlowModel::BuildCatchmentsFromQueries( void )
   {
   // generate aggregates - these are the spatial units for running catchment-based models.  Two types of 
   // aggregates exist - catchments, and within catchments, collections of HRU's
   //
   // In both cases, the process is:
   //
   // 1) generate queries that indicate unique HRU poly collections, and unique Catchment poly collections.
   //    For Catchments, these are based on the catchment aggregation column(s), for HRUs, these are
   //    based on the catchment AND HRU columns, since HRU's are subsets of catchments
   // 2) iterate through the catchment layer polygons, identifying aggregates that represent HRUs and catchmentn
   //
   // Basic idea is that an aggregate is a collection of polygons that have the same attribute values
   // for the given field(s).  For example, if the coverage contains a HYDRO_ID value and that was specified 
   // as the aggregate col, then unique values of HYDRO_ID would be used to generate the  HRU's

   int colCOMID = -1; m_pIDUlayer->CheckCol(colCOMID, "COMID", TYPE_INT, CC_MUST_EXIST);
   int colONE_VAL = -1; m_pIDUlayer->CheckCol(colONE_VAL, "ONE_VAL", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->SetColDataU(colONE_VAL, 1);

   m_colhruHRU_ID = m_pHRUlayer->GetFieldCol("HRU_ID");
   bool hru_id_exists_flag = m_colhruHRU_ID >= 0;
   if (!hru_id_exists_flag) 
      {
      m_pHRUlayer->CheckCol(m_colhruHRU_ID, "HRU_ID", TYPE_INT, CC_AUTOADD);
      m_pHRUlayer->SetColDataU(m_colhruHRU_ID, -99);
      }

   ParseCols( m_pCatchmentLayer, m_catchmentAggCols, m_colsCatchmentAgg );
   ParseCols( m_pCatchmentLayer, m_hruAggCols, m_colsHruAgg );

   // first, do catchments
   CArray< Query*, Query* > catchmentQueryArray;
   int queryCount = GenerateAggregateQueries( m_pCatchmentLayer, m_colsCatchmentAgg, catchmentQueryArray );

   CArray< Query*, Query* > hruQueryArray;
   int hruQueryCount = GenerateAggregateQueries( m_pCatchmentLayer, m_colsHruAgg, hruQueryArray );

   // temporary allocations
   Catchment **catchments = new Catchment*[ queryCount ];  // placeholder for catchments
   memset( catchments, 0, queryCount*sizeof( Catchment* ) );  // initially, all NULL

   m_pCatchmentLayer->SetColNoData( m_colCatchmentCatchID );
   m_pCatchmentLayer->SetColNoData( m_colCatchmentHruID );

   int hruCols = (int) m_colsHruAgg.GetSize();
   VData *hruValueArray = new VData[ hruCols ];

   int polyCount = m_pCatchmentLayer->GetPolygonCount();
   int hruCount = 0;
   
   if ( m_pCatchmentQuery != NULL )
      polyCount = m_pCatchmentQuery->Select( true );

   // iterate though polygons that satisfy the catchment query.
   for ( int i=0; i < polyCount; i++ )
      {
      int polyIndex = m_pCatchmentQuery ? m_pCatchmentLayer->GetSelection( i ) : i;

      // get hru aggregate values from this poly
      for ( int h=0; h < hruCols; h++ )
         {
         bool ok = m_pCatchmentLayer->GetData( polyIndex, m_colsHruAgg[ h ], hruValueArray[ h ] );
         if ( ! ok )
            hruValueArray[ h ].SetNull();
         }

      // iterate through the catchment aggregate queries, to see if this polygon satisfies any of them
      for ( int j=0; j < queryCount; j++ )
         {
         Query *pQuery = catchmentQueryArray.GetAt( j );
         bool result = false;
         bool ok = pQuery->Run( polyIndex, result );

         if ( ok && result )  // is this polygon in a the specified Catchment (query?) - yes!
            {
            if ( catchments[ j ] == NULL )    // if first one, creaet a Catchment object for it
               {
               catchments[ j ] = new Catchment;
               catchments[ j ]->m_id = j;
               }

            // we know which catchment it's in, next figure out which HRU it is in
            Catchment *pCatchment = catchments[ j ];
            ASSERT( pCatchment != NULL );

            // store this in the catchment layer
            ASSERT( m_colCatchmentCatchID >= 0 );
            m_pCatchmentLayer->SetData( polyIndex, m_colCatchmentCatchID, pCatchment->m_id );

            // does it satisfy an existing HRU for this catchment?
            HRU *pHRU = NULL;
            for ( int k=0; k < pCatchment->GetHRUCountInCatchment(); k++ )
               {
               HRU *_pHRU = pCatchment->GetHRUfromCatchment( k );
              // _pHRU->m_hruValueArray.SetSize(hruCols);
               ASSERT( _pHRU != NULL );
               ASSERT( _pHRU->m_polyIndexArray.GetSize() > 0 );

               UINT hruPolyIndex = _pHRU->m_polyIndexArray[ 0 ];

               // does the poly being examined match this HRU?
               bool match = true;
               for ( int m=0; m < hruCols; m++ )
                  {
                  VData hruValue;
                  m_pCatchmentLayer->GetData( hruPolyIndex, m_colsHruAgg[ m ], hruValue );
                  int hruVal;
                  m_pCatchmentLayer->GetData( hruPolyIndex, m_colsHruAgg[ m ], hruVal );
                  
                  if ( hruValue != hruValueArray[ m ] )
                     {
                     match = false;
                     break;
                     }

                  }  // end of: for ( m < hruCols )

               if ( match )
                  {
                  pHRU = _pHRU;
                  break;
                  }
               }  // end of: for ( k < pCatchment->GetHRUCount() )

            if ( pHRU == NULL )  // not found in existing HRU's for this catchment?  Then add...
               {
               pHRU = new HRU;
               AddHRU( pHRU, pCatchment );

               pHRU->m_id = hruCount;
               int comid_from_IDU = -1; m_pIDUlayer->GetData(polyIndex, colCOMID, comid_from_IDU);
               int hru_row = m_pHRUlayer->FindIndex(m_colhruCOMID, comid_from_IDU);
               ASSERT(hru_row >= 0);
               if (hru_id_exists_flag)
               {
                  int preexisting_hru_id = -1; m_pHRUlayer->GetData(hru_row, m_colhruHRU_ID, preexisting_hru_id);
                  if (preexisting_hru_id >= 0 && preexisting_hru_id != pHRU->m_id)
                  {
                     CString msg; msg.Format("BuildCatchmentsFromQueries()1: hru_row = %d, preexisting_hru_id = %d, pHRU->m_id = %d", hru_row, preexisting_hru_id, pHRU->m_id);
                     Report::ErrorMsg(msg);
                  }
               }
               m_pHRUlayer->SetDataU(hru_row, m_colhruHRU_ID, pHRU->m_id);

               pHRU->AddLayers( m_soilLayerCount, m_snowLayerCount, m_vegLayerCount, 0, m_initTemperature, false );

               pHRU->m_aquifer_recharge_mm = 0.f;
               hruCount++;
               }

            ASSERT( m_colCatchmentHruID >= 0 );
            int preexisting_hru_id = -1; m_pCatchmentLayer->GetData(polyIndex, m_colCatchmentHruID, preexisting_hru_id);
            if (preexisting_hru_id >= 0 && preexisting_hru_id != pHRU->m_id)
               {
               CString msg; msg.Format("BuildCatchmentsFromQueries()2: polyIndex = %d, preexisting_hru_id = %d, pHRU->m_id = %d", polyIndex, preexisting_hru_id, pHRU->m_id);
               Report::ErrorMsg(msg);
               }
            m_pCatchmentLayer->SetData( polyIndex, m_colCatchmentHruID, pHRU->m_id );

            if (pHRU->m_id >= m_pHRUlayer->GetRowCount())
               {
               CString msg; 
               msg.Format("FlowModel::BuildCatchmentsFromQueries()3 pHRU->m_id = %d, m_pHRUlayer->GetRowCount() = %d.  There are more HRUs than there are rows in the HRU layer.",
                  pHRU->m_id, m_pHRUlayer->GetRowCount());
               Report::ErrorMsg(msg);
               }

            pHRU->m_polyIndexArray.Add( polyIndex );
            break;  // done processing queries for this polygon, since we found one that satisifies it, no need to look further
            }  // end of: if ( ok && result )  meaning adding polyIndex to catchment
         }  // end of: for ( j < queryCount )    
      }  // end of: for ( i < polyCount )

   // store catchments
   for ( int j=0; j < queryCount; j++ )
      {
      Catchment *pCatchment = catchments[ j ];

      if ( pCatchment != NULL )
         m_catchmentArray.Add( pCatchment );
      }

   // Reorder the m_hruArray to match the order in the HRU.shp layer.
   ASSERT(m_pHRUlayer->GetRowCount() == m_hruArray.GetSize());
   HRU * * hrus = new HRU * [m_pHRUlayer->GetRowCount()];  
   memset(hrus, NULL, m_pHRUlayer->GetRowCount() * sizeof(HRU *));  // initially, all NULL
   for (MapLayer::Iterator hru = m_pHRUlayer->Begin(); hru != m_pHRUlayer->End(); hru++)
      {
      int hru_id = -1; m_pHRUlayer->GetData(hru, m_colhruHRU_ID, hru_id);
      int orig_hru_ndx;
      for (orig_hru_ndx = 0; orig_hru_ndx < m_hruArray.GetSize(); orig_hru_ndx++)
         { 
         HRU * pHRU = m_hruArray[orig_hru_ndx];
         if (pHRU->m_id == hru_id) break;
      	 } // end of loop thru m_hruArray[] to find the HRU with m_id equal to hru_id
      ASSERT(orig_hru_ndx < m_hruArray.GetSize());
      hrus[(int)hru] = m_hruArray[orig_hru_ndx];
      } // end of loop thru HRU layer to build hrus[]
   for (int hru_ndx = 0; hru_ndx < m_hruArray.GetSize(); hru_ndx++) 
      {
      m_hruArray[hru_ndx] = hrus[hru_ndx];
      m_hruArray[hru_ndx]->m_hruNdx = hru_ndx;
      }

   // Store the HRU_NDX back to the IDU layer.
   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
      {
      int hru_id = -1; m_pIDUlayer->GetData(idu, m_colHRU_ID, hru_id);
      int hru_ndx = -1;
      if (hru_id >= 0) hru_ndx = m_pHRUlayer->FindIndex(m_colhruHRU_ID, hru_id);
      m_pIDUlayer->SetDataU(idu, m_colHRU_NDX, hru_ndx);
      }

   // clean up  
   delete [] hruValueArray;
   delete [] catchments;
   delete [] hrus;

   if ( m_pCatchmentQuery != NULL )
      m_pCatchmentLayer->ClearSelection();
   
   CString msg;
   msg.Format( _T("Flow: %i Catchments, %i HRUs built from %i polygons" ), (int) m_catchmentArray.GetSize(), hruCount, polyCount ); 
   Report::LogMsg( msg, RT_INFO );

   return true;
   } // end of BuildCatchmentsFromQueries()


// bool FlowModel::BuildCatchmentsFromIDUlayer( void ) -----------------------------
//
// builds Catchments and HRU's based on HRU ID's contained in the catchment layer
// Note that this method does NOT connect catchments to reaches
//-------------------------------------------------------------------------------

   bool FlowModel::BuildCatchmentsFromIDUlayer(void)
   {
      if (this->m_colCatchmentHruID < 0 || m_colCatchmentCatchID < 0)
         return false;

      int polyCount = m_pCatchmentLayer->GetPolygonCount();

      if (m_pCatchmentQuery != NULL)
         polyCount = m_pCatchmentQuery->Select(true);

      int catchID, hruID;
      int hruCount = 0;

      CMap< int, int, Catchment*, Catchment* > catchmentMap;

      // iterate though polygons that satisfy the catchment query.
      for (int i = 0; i < polyCount; i++)
      {
         int polyIndex = m_pCatchmentQuery ? m_pCatchmentLayer->GetSelection(i) : i;

         bool ok = m_pCatchmentLayer->GetData(polyIndex, m_colCatchmentCatchID, catchID);
         if (!ok)
            continue;

         ok = m_pCatchmentLayer->GetData(polyIndex, m_colCatchmentHruID, hruID);
         if (!ok)
            continue;

         Catchment *pCatchment = NULL;
         ok = catchmentMap.Lookup(catchID, pCatchment) ? true : false;
         if (!ok)
         {
            pCatchment = AddCatchment(catchID, NULL);
            catchmentMap.SetAt(catchID, pCatchment);
         }

         // catchment covered, now do HRU's
         HRU *pHRU = NULL;
         for (int j = 0; j < pCatchment->GetHRUCountInCatchment(); j++)
         {
            HRU *_pHRU = pCatchment->GetHRUfromCatchment(j);

            if (_pHRU->m_id == hruID)
            {
               pHRU = _pHRU;
               break;
            }
         }

         if (pHRU == NULL)
         {
            pHRU = new HRU;
            AddHRU(pHRU, pCatchment);
            // m_hruArray.Add(pHRU);
            // ???Flow HRU area should be initialized before AddLayers() is called.
            pHRU->AddLayers(m_soilLayerCount, m_snowLayerCount, m_vegLayerCount, 0, m_initTemperature, false);
            pHRU->m_id = hruCount;
            hruCount++;
            pHRU->m_aquifer_recharge_mm = 0.f;
         }

         pHRU->m_polyIndexArray.Add(polyIndex);
      }  // end of:  for ( i < polyCount )
      //pHRU->m_HRUeffArea_m2 = pHRU->m_HRUtotArea_m2 = area_m2;
      //pHRU->m_areaIrrigated = 0.f;

      CString msg;
      msg.Format(_T("Flow: %i Catchments, %i HRUs loaded from %i polygons"), (int)m_catchmentArray.GetSize(), hruCount, polyCount);
      Report::LogMsg(msg, RT_INFO);

      return true;
   } // end of BuildCatchmentsFromIDUlayer()


   bool FlowModel::BuildCatchmentsFromHRUlayer(void)
   // Assumption: Both Catchments and HRUs are aggregated on COMIDs, so there is a
   // 1-to-1 relationship between Catchments and HRUs, and a common index may be used.
   // Although the same index may be used for Catchments and HRUs, their IDs are not necessarily
   // the same.
   {
      int num_HRUs = m_pHRUlayer->GetRowCount();

      for (int common_ndx = 0; common_ndx < num_HRUs; common_ndx++)
      {
         int catch_id = -1; m_pHRUlayer->GetData(common_ndx, m_colhruCATCH_ID, catch_id);
         int hru_id = -1; m_pHRUlayer->GetData(common_ndx, m_colhruHRU_ID, hru_id);
         int comid = -1; m_pHRUlayer->GetData(common_ndx, m_colhruCOMID, comid);
         float area_m2 = 0.f; m_pHRUlayer->GetData(common_ndx, m_colHruAREA_M2, area_m2);

         Reach * pReach = FindReachFromID(comid);
         if (pReach == NULL)
         {
            CString msg;
            msg.Format("BuildCatchmentsFromHRUlayer() Can't find comid = %d in m_reachArray using FindReachFromID(). common_ndx = %d", 
               comid, common_ndx);
            Report::ErrorMsg(msg);
            continue;
         }

         Catchment * pCatch = AddCatchment(catch_id, pReach);
         m_pHRUlayer->SetDataU(common_ndx, m_colhruCATCH_NDX, common_ndx);

         HRU * pHRU = new HRU;
         AddHRU(pHRU, pCatch);
         pHRU->m_HRUeffArea_m2 = pHRU->m_HRUtotArea_m2 = area_m2;
         pHRU->m_areaIrrigated = 0.f;
         pHRU->AddLayers(m_soilLayerCount, m_snowLayerCount, m_vegLayerCount, 0, m_initTemperature, false);
         pHRU->m_id = hru_id;
         pHRU->m_hruNdx = common_ndx;
         pHRU->m_aquifer_recharge_mm = 0.f;
      }  // end of:  for ( common_ndx < num_HRUs )

      // Now loop thru the IDUs, add the index of each IDU to the pHRU->m_polyIndexArray of its HRU,
      // and populate the HRU_NDX attribute in the IDU layer.
      for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
      { 
         int hru_id = -1; m_pIDUlayer->GetData(idu, m_colHRU_ID, hru_id);
         int hru_ndx = -1;
         if (hru_id >= 0) 
         { 
            hru_ndx = m_pHRUlayer->FindIndex(m_colhruHRU_ID, hru_id);
            HRU * pHRU = GetHRU(hru_ndx);
            if (pHRU == NULL)
            {
               CString msg;
               msg.Format("BuildCatchmentsFromHRUlayer() pHRU is NULL for hru_id = %d, hru_ndx = %d.", hru_id, hru_ndx);
               Report::ErrorMsg(msg);
               continue;
            }
            pHRU->m_polyIndexArray.Add((int)idu);
         }
         m_pIDUlayer->SetDataU(idu, m_colHRU_NDX, hru_ndx);
      }

      CString msg;
      msg.Format(_T("Flow: %i Catchments, %i HRUs loaded from %i polygons"), (int)m_catchmentArray.GetSize(), m_hruArray.GetSize(), m_pIDUlayer->GetRowCount());
      Report::LogMsg(msg, RT_INFO);

      return true;
   } // end of BuildCatchmentsFromHRUlayer()


int FlowModel::BuildCatchmentFromPolygons( Catchment *pCatchment, int polyArray[], int polyCount )
   {
   // generate aggregates - these are the spatial units for running catchment-based models.  Two types of 
   // aggregates exist - catchments, and within catchments, collections of HRU's
   //
   // In both cases, the process is:
   //
   // 1) generate queries that indicate unique HRU poly collections, and unique Catchment poly collections.
   //    For Catchments, these are based on the catchment aggregation column(s), for HRUs, these are
   //    based on the catchment AND HRU columns, since HRU's are subsets of catchments
   // 2) iterate through the catchment layer polygons, identifying aggregates that represent HRUs and catchmentn
   //
   // Basic idea is that an aggregate is a collection of polygons that have the same attribute values
   // for the given field(s).  For example, if the coverage contains a HYDRO_ID value and that was specified 
   // as the aggregate col, then unique values of HYDRO_ID would be used to generate the  HRU's

   //ParseCols( m_pCatchmentLayer, m_hruAggCols, m_colsHruAgg ); not needed, already done in prior Buildxxx()

   CArray< Query*, Query* > hruQueryArray;   // will contain queries for all possible unique combination of HRU attributes
   int hruQueryCount = GenerateAggregateQueries( m_pCatchmentLayer, m_colsHruAgg, hruQueryArray );

   int hruCols = (int) m_colsHruAgg.GetSize();     // number of cols referenced in the queries
   VData *hruValueArray = new VData[ hruCols ];    // will store field values during queries

   int hruCount = 0;

   // iterate though polygons
   for ( int i=0; i < polyCount; i++ )
      {
      int polyIndex = polyArray[ i ];

      // get hru aggregate values from this poly
      for ( int h=0; h < hruCols; h++ )
         {
         bool ok = m_pCatchmentLayer->GetData( polyIndex, m_colsHruAgg[ h ], hruValueArray[ h ] );
         if ( ! ok )
            hruValueArray[ h ].SetNull();
         }

      // store the catchment ID in the poly 
      ASSERT( m_colCatchmentCatchID >= 0 );
      m_pCatchmentLayer->SetData( polyIndex, m_colCatchmentCatchID, pCatchment->m_id );

      // does it satisfy an existing HRU for this catchment?
      HRU *pHRU = NULL;
      for ( int k=0; k < pCatchment->GetHRUCountInCatchment(); k++ )
         {
         HRU *_pHRU = pCatchment->GetHRUfromCatchment( k );
         // _pHRU->m_hruValueArray.SetSize(hruCols);
         ASSERT( _pHRU != NULL );
         ASSERT( _pHRU->m_polyIndexArray.GetSize() > 0 );         
         UINT hruPolyIndex = _pHRU->m_polyIndexArray[ 0 ];

         // does the poly being examined match this HRU?
         bool match = true;
         for ( int m=0; m < hruCols; m++ )
            {
            VData hruValue;
            m_pCatchmentLayer->GetData( hruPolyIndex, m_colsHruAgg[ m ], hruValue );
            int hruVal;
            m_pCatchmentLayer->GetData( hruPolyIndex, m_colsHruAgg[ m ], hruVal );
            
            if ( hruValue != hruValueArray[ m ] )
               {
               match = false;
               break;
               }
            }  // end of: for ( m < hruCols )

         if ( match )
            {
            pHRU = _pHRU;
            break;
            }
         }  // end of: for ( k < pCatchment->GetHRUCount() )

      if ( pHRU == NULL )  // not found in existing HRU's for this catchment?  Then add...
         {
         pHRU = new HRU;
         AddHRU( pHRU, pCatchment );

         pHRU->m_id = 10000+m_numHRUs;
         m_numHRUs++;
         pHRU->AddLayers( m_soilLayerCount, m_snowLayerCount, m_vegLayerCount, 0, m_initTemperature, false );
         hruCount++;
         pHRU->m_aquifer_recharge_mm = 0.f;
         }

     ASSERT( m_colCatchmentHruID >= 0 );
     m_pCatchmentLayer->SetData( polyIndex, m_colCatchmentHruID, pHRU->m_id );

     pHRU->m_polyIndexArray.Add( polyIndex );
     }  // end of: for ( i < polyCount )
   
   // update hru info
   SetHRUAttributes( pCatchment );

   // clean up
   delete [] hruValueArray;
   
   //CString msg;
   //msg.Format( _T("Flow: Rebuild: %i HRUs built from %i polygons" ), hruCount, polyCount ); 
   //Report::LogMsg( msg, RT_INFO );
   return hruCount;
   }



int FlowModel::ParseCols( MapLayer *pLayer /*in*/, CString &aggCols /*in*/, CUIntArray &colsAgg /*out*/ )
   {
   // parse columns
   TCHAR *buffer = new TCHAR[ aggCols.GetLength()+1 ];
   TCHAR *nextToken = NULL;
   lstrcpy( buffer, (LPCTSTR) aggCols );

   TCHAR *col = _tcstok_s( buffer, ",", &nextToken );

   int colCount = 0;

   while( col != NULL )
      {
      int _col = pLayer->GetFieldCol( col );

      if ( _col < 0 )
         {
         CString msg( "Flow: Aggregation column " );
         msg += col;
         msg += " not found - it will be ignored";
         Report::ErrorMsg( msg );
         }
      else
         {
         colsAgg.Add( _col );
         colCount++;
         }

      col = _tcstok_s( NULL, ",", &nextToken );

      }

   delete [] buffer;

   return colCount;
   }



int FlowModel::GenerateAggregateQueries( MapLayer *pLayer /*in*/, CUIntArray &colsAgg /*in*/, CArray< Query*, Query* > &queryArray /*out*/ )
   {
   queryArray.RemoveAll();

   int colCount = (int) colsAgg.GetSize();

   if ( colCount == 0 )    // no column specified, no queries generatated
      return 0;

   // have columns, figure possible queries
   // for each column, get all possible values of the attributes in that column
   PtrArray< CStringArray > colStrArrays;  // one entry for each column, each entry has unique field values

   for ( int i=0; i < colCount; i++ )
      {
      CStringArray *attrValArray = new CStringArray;
      if ( pLayer->GetUniqueValues( colsAgg[ i ], *attrValArray ) > 0 )
         colStrArrays.Add( attrValArray );
     else
        delete attrValArray;

      }

   // now generate queries.  ASSUME AT MOST TWO COLUMNS FOR NOW!!!!  this needs to be improved!!!
   ASSERT( colCount <= 2 );

   if ( colCount == 1 )
      {
      CStringArray *attrValArray = colStrArrays[ 0 ];

      LPCTSTR colName = pLayer->GetFieldLabel( colsAgg[ 0 ] );

      for ( int j=0; j < attrValArray->GetSize(); j++ )
         {
         CString queryStr;
         queryStr.Format( "%s = %s and COMID > 0", colName, (LPCTSTR) attrValArray->GetAt( j ) );
         /*
         if ( m_catchmentQuery.GetLength() > 0 )
            {
            queryStr += _T(" and ");
            queryStr += m_catchmentQuery;
            } */        

         ASSERT( m_pCatchmentQE != NULL );
         Query *pQuery = m_pCatchmentQE->ParseQuery( queryStr, 0, "Flow Aggregate Query" );

         if ( pQuery == NULL )
            {
            CString msg( "Flow: Error parsing Aggregation query: '" );
            msg += queryStr;
            msg += "' - the query will be ignored";
            Report::ErrorMsg( msg );
            }
         else
            queryArray.Add( pQuery );
         }
      }

   else if ( colCount == 2 )
      {
      CStringArray *attrValArray0 = colStrArrays[ 0 ];
      CStringArray *attrValArray1 = colStrArrays[ 1 ];

      LPCTSTR colName0 = pLayer->GetFieldLabel( colsAgg[ 0 ] );
      LPCTSTR colName1 = pLayer->GetFieldLabel( colsAgg[ 1 ] );

      for ( int j=0; j < attrValArray0->GetSize(); j++ )
         {
         for ( int i=0; i < attrValArray1->GetSize(); i++ )
            {
            CString queryStr;
            queryStr.Format( "%s = %s and %s = %s", colName0, (LPCTSTR) attrValArray0->GetAt( j ), colName1, (LPCTSTR) attrValArray1->GetAt( i )  );
            /*
            if ( m_catchmentQuery.GetLength() > 0 )
               {
               queryStr += _T(" and ");
               queryStr += m_catchmentQuery;
               } */         

            ASSERT( m_pCatchmentQE != NULL );
            Query *pQuery = m_pCatchmentQE->ParseQuery( queryStr, 0, "Flow Aggregate Query" );

            if ( pQuery == NULL )
               {
               CString msg( "Flow: Error parsing Aggregation query: '" );
               msg += queryStr;
               msg += "' - the query will be ignored";
               Report::ErrorMsg( msg );
               }
            else
               queryArray.Add( pQuery );
            }
         }
      }

   return (int) queryArray.GetSize();
   }



bool FlowModel::ConnectCatchmentsToReaches(void)
{
   // This method determines which reach, if any, each catchment is connected to.  It does this by
   // iterate through the catchments, relating each catchment with it's associated reach.  We assume that
   // each polygon has a field indicated by the catchment join col that contains the index of the 
   // associated stream reach.

   INT_PTR catchmentCount = m_catchmentArray.GetSize();
   INT_PTR reachCount = m_reachArray.GetSize();

   int catchmentJoinID = 0;

   int unconnectedCatchmentCount = 0;
   int unconnectedReachCount = 0;

   m_pReachLayer->SetColDataU(m_colReachHRU_ID, -1);

   // iterate through each catchment, getting the associated field value that
   // maps into the stream coverage
   for (INT_PTR i = 0; i < catchmentCount; i++)
   {
      Catchment *pCatchment = m_catchmentArray[i];
      pCatchment->m_pReach = NULL;

      // have catchment, use the IDU associated with the first HRU in catchment to get
      // the value stored in the catchment join col in the catchment layer.  This is a lookup
      // key - if the corresponding value is found in the reach join col in the stream coverage,
      // we connect this catchment to the reach with the matching join ID
      //  ASSERT( pCatchment->GetHRUCount() > 0 );

      if (pCatchment->GetHRUCountInCatchment() > 0)
      {
         HRU *pHRU = pCatchment->GetHRUfromCatchment(0);
         m_pCatchmentLayer->GetData(pHRU->m_polyIndexArray[0], m_colCatchmentJoin, catchmentJoinID);

         // look for this catchID in the reaches
         for (INT_PTR j = 0; j < reachCount; j++)
         {
            Reach *pReach = m_reachArray[j];

            int streamJoinID = -1;
            m_pStreamLayer->GetData(pReach->m_polyIndex, this->m_colStreamJoin, streamJoinID);

            // try to join the stream and catchment join cols
            if (streamJoinID == catchmentJoinID)       // join col values match?
            {
               pCatchment->m_pReach = pReach;               // indicate that the catchment is connected to this reach
               pReach->m_catchmentArray.Add(pCatchment);  // add this catchment to the reach's catchment array
               int orig_hru_id = -2; m_pReachLayer->GetData(pReach->m_polyIndex, m_colReachHRU_ID, orig_hru_id);
               if (orig_hru_id != -1)
               {
                  CString msg;
                  msg.Format("ConnectCatchmentsToReaches() j = %d, pReach->m_reachID = %d, orig_hru_id = %d. Should be -1.",
                     j, pReach->m_reachID, orig_hru_id);
                  Report::WarningMsg(msg);
               }
               else
               {
                  int hru_ndx = m_pHRUlayer->FindIndex(m_colhruCOMID, streamJoinID);
                  int hru_id = -1; m_pHRUlayer->GetData(hru_ndx, m_colhruHRU_ID, hru_id);

                  int reach_comid = 0; m_pReachLayer->GetData(pReach->m_polyIndex, m_colStreamCOMID, reach_comid);
                  int hru_comid = 0; m_pHRUlayer->GetData(hru_ndx, m_colhruCOMID, hru_comid);
                  ASSERT(reach_comid == hru_comid);

                  m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachHRU_ID, hru_id);
               }
               break;
            }
         }

         if (pCatchment->m_pReach == NULL)
            unconnectedCatchmentCount++;
      }
   }

   /*
   // check for truncated stream orders.  Basic idea is to scan catchments look for connections
   // to stream reaches of order < m_minStreamOrder.  When found, re-route the catchment to
   // the first downstream reach that is >= m_minStreamOrder.
   if ( m_minStreamOrder > 1 )
   {
   for ( INT_PTR i=0; i < catchmentCount; i++ )
   {
   Catchment *pCatchment = m_catchmentArray[ i ];
   Reach *pReach = pCatchment->m_pReach;

   // have reach, check stream order
   if ( pReach && pReach->m_streamOrder < m_minStreamOrder )
   {
   Reach *pNewReach = pReach;

   // this one should be rerouted, so do it
   while ( pNewReach && pNewReach->m_streamOrder < m_minStreamOrder )
   {
   ReachNode *pDownNode = pNewReach->GetDownstreamNode( true );
   pNewReach = (Reach*) pDownNode;
   }

   if ( pNewReach == NULL )   // end of tree found?
   {
   pCatchment->m_pReach = NULL;
   reRoutedAndLost++;
   }
   else
   {
   pCatchment->m_pReach = pNewReach;
   pNewReach->m_catchmentArray.Add( pCatchment );
   reRoutedCatchmentCount++;
   }
   }  // end of:  if ( pReach->m_streamOrder < m_minStreamOrder )
   }  // end of: if ( pReach
   }

   */

   for (INT_PTR j = 0; j < reachCount; j++)
   {
      Reach *pReach = m_reachArray[j];

      if (pReach->m_catchmentArray.GetSize() > 1)
         Report::LogMsg("Flow: More than one catchment attached to a reach!");

      if (pReach->m_catchmentArray.GetSize() == 0)
         unconnectedReachCount++;
   }

   // populate the stream layers "CATCHID" field with the reach's catchment ID (if any)
   int colCatchmentID = -1;
   gpFlow->CheckCol(this->m_pStreamLayer, colCatchmentID, "CATCHID", TYPE_INT, CC_AUTOADD);

   for (INT_PTR j = 0; j < reachCount; j++)
   {
      Reach *pReach = m_reachArray[j];
      if (pReach->m_catchmentArray.GetSize() > 0)
      {
         Catchment *pCatchment = pReach->m_catchmentArray[0];
         m_pStreamLayer->SetData(pReach->m_polyIndex, colCatchmentID, pCatchment->m_id);
      }
   }
   ///////////////////////////////////////

   CString msg;
   msg.Format(_T("Flow: Topology established for %i catchments and %i reaches.  %i catchments are not connected to any reach, %i reaches are not connected to any catchments. "), (int)m_catchmentArray.GetSize(), (int)m_reachArray.GetSize(), unconnectedCatchmentCount, unconnectedReachCount);
   Report::LogMsg(msg);
   return true;
}


double Direction_deg(Vertex fromPt, Vertex toPt) // Returns compass direction from the first point to the second point in degrees.
{ 
   double direction_deg = 0;
   double easting = toPt.x - fromPt.x;
   double northing = toPt.y - fromPt.y;

   if (easting == 0 && northing == 0) direction_deg = 0; // the toPt is the same as the fromPt
   else if (easting == 0) direction_deg = northing > 0 ? 0 : 180;
   else
   {
      double radius = sqrt(northing * northing + easting * easting);
      double direction_rad = asin(easting / radius);
      direction_deg = (direction_rad / (2 * PI)) * 360;
      if (direction_deg < 0) direction_deg += 360;
      ASSERT(!isnan(direction_deg) && 0 <= direction_deg && direction_deg < 360);
   }

   return(direction_deg);
} // end of Direction_deg()


bool FlowModel::AssignIDUsToStreamBanks() // Populate reach attributes BANK_L_IDU, BANK_R_IDU, and DIRECTION
{
   m_pReachLayer->SetColDataU(m_colReachBANK_L_IDU, -1);
   m_pReachLayer->SetColDataU(m_colReachBANK_R_IDU, -1);

   for (INT_PTR reach_ndx = 0; reach_ndx < m_reachArray.GetSize(); reach_ndx++)
   {
      Reach* pReach = m_reachArray[reach_ndx];
      int hru_id = -1; m_pReachLayer->GetData(pReach->m_polyIndex, m_colReachHRU_ID, hru_id);
      int hru_ndx = m_pHRUlayer->FindIndex(m_colhruHRU_ID, hru_id);
      HRU* pHRU = m_hruArray[hru_ndx];

      // Calculate net compass direction of flow in deg, North = 0 
      Poly* pReachPoly = m_pReachLayer->GetPolygon(pReach->m_polyIndex);
      Vertex upstream_end = pReachPoly->m_vertexArray[0];
      int ndx_of_downstream_end = (int)pReachPoly->m_vertexArray.GetSize() - 1; ASSERT(ndx_of_downstream_end > 0);
      Vertex downstream_end = pReachPoly->m_vertexArray[ndx_of_downstream_end];
      Vertex nominal_midpt = Vertex((downstream_end.x + upstream_end.x) / 2, (downstream_end.y + upstream_end.y) / 2);
      double flow_direction_deg = Direction_deg(upstream_end, downstream_end);
      m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachDIRECTION, flow_direction_deg);

      // Find the IDU in the HRU closest to the right and left banks of the downstream end of the reach.
/*x   Could also try these
      int   GetNearestPoly(REAL x, REAL y) const;
      int   GetNearestPoly(Poly *pPoly, float maxDistance, SI_METHOD method = SIM_NEAREST, MapLayer *pToLayer = NULL) const;

      int   GetNearbyPolys(Poly *pPoly, int *neighbors, float *distances, int maxCount, float maxDistance,
         SI_METHOD method = SIM_NEAREST, MapLayer *pToLayer = NULL) const;

      int   GetNearbyPolysFromIndex(Poly *pPoly, int *neighbors, float *distances, int maxCount, float maxDistance,
         SI_METHOD method = SIM_NEAREST, MapLayer *pToLayer = NULL, void** ex = NULL) const;

      float ComputeAdjacentLength(Poly *pThisPoly, Poly *pSourcePoly);
x*/
      int num_idus_in_hru = (int)pHRU->m_polyIndexArray.GetSize(); ASSERT(num_idus_in_hru > 0);
      int closest_left_idu_ndx = -1, closest_right_idu_ndx = -1;
      REAL d_closest_left_idu = 1e9, d_closest_right_idu = 1e9;

      for (int idu_in_hru_ndx = 0; idu_in_hru_ndx < num_idus_in_hru; idu_in_hru_ndx++)
      {
         int idu_poly_ndx = pHRU->m_polyIndexArray[idu_in_hru_ndx];

         // Calculate the distance from the centroid of the IDU to nominal midpoint of the reach.
         REAL centroidx = 0.; m_pIDUlayer->GetData(idu_poly_ndx, m_colCENTROIDX, centroidx);
         REAL centroidy = 0.; m_pIDUlayer->GetData(idu_poly_ndx, m_colCENTROIDY, centroidy);
         Vertex idu_centroid(centroidx, centroidy);
         ASSERT(!isnan(idu_centroid.x) && !isnan(idu_centroid.y));
         REAL dx = idu_centroid.x - nominal_midpt.x;
         REAL dy = idu_centroid.y - nominal_midpt.y;
         REAL d_idu2reach = sqrt(dx * dx + dy * dy);

         // Looking downstream, which side of the reach is this IDU on?
         // Note that if the IDU straddles the reach, then it is on both sides.
         // ??? but this logic doesn't recognize that it might be on both sides
         double direction_to_idu = Direction_deg(nominal_midpt, idu_centroid);
         bool left_bank_idu = direction_to_idu > flow_direction_deg && direction_to_idu < (flow_direction_deg + 180);
         bool right_bank_idu = !left_bank_idu;

         if (left_bank_idu && d_idu2reach < d_closest_left_idu)
         {
            closest_left_idu_ndx = idu_poly_ndx;
            d_closest_left_idu = d_idu2reach;
         }
         if (right_bank_idu && d_idu2reach < d_closest_right_idu)
         {
            closest_right_idu_ndx = idu_poly_ndx;
            d_closest_right_idu = d_idu2reach;
         }
      } // end of loop thru IDUs in HRU
      ASSERT(closest_left_idu_ndx >= 0 || closest_right_idu_ndx >= 0);
      if (closest_right_idu_ndx < 0) closest_right_idu_ndx = closest_left_idu_ndx;
      if (closest_left_idu_ndx < 0) closest_left_idu_ndx = closest_right_idu_ndx;
      m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachBANK_L_IDU, closest_left_idu_ndx);
      m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachBANK_R_IDU, closest_right_idu_ndx);

   } // end of loop thru reaches

   return(true);
} // end of AssignIDUsToStreamBanks()


bool FlowModel::AssignReachesToHRUs() // Populate reach attributes HRU_ID and HRU_FRAC.
{
   // An "orphan reach" is a reach whose comid is not associated with any IDU.
   // First loop thru HRUs, fill in the HRU_ID for the reaches with the same COMID as the HRU, and add the reach index to the m_reachNdxArray for the HRU.
   // Then find the orphan reaches, set their HRU_IDs to the HRU_ID of the first downstream reach that is already associated with an HRU, and 
   // add the reach layer index to the m_reachNdxArray for the HRU.
   // Then loop thru all the reaches and populate an array of total reach lengths by HRU.
   // Finally, loop thru all the reaches one more time, and populate their HRU_FRAC attributes.

   CString msg = "AssignReachesToHRUs() starting now..."; Report::LogMsg(msg);

   m_pReachLayer->SetColDataU(m_colReachHRU_ID, -1);
   for (int hru_ndx = 0; hru_ndx < m_hruArray.GetSize(); hru_ndx++)
   {
      HRU * pHRU = m_hruArray[hru_ndx];
      Reach * pReach = pHRU->m_pCatchment->m_pReach;
      int polyIndex = pReach->m_polyIndex;
      int comid = pHRU->m_pCatchment->m_pReach->m_reachID;
      int reach_ndx_in_ReachLayer = m_pReachLayer->FindIndex(m_colStreamCOMID, comid);
      ASSERT(polyIndex == reach_ndx_in_ReachLayer);
      m_pReachLayer->SetDataU(reach_ndx_in_ReachLayer, m_colReachHRU_ID, pHRU->m_id);
      pHRU->m_reachNdxArray.RemoveAll();
   } // end of loop thru HRUs

   CArray <float, float> hru_tot_reach_length; hru_tot_reach_length.SetSize(m_hruArray.GetSize());
   for (int hru_ndx = 0; hru_ndx < hru_tot_reach_length.GetSize(); hru_ndx++) hru_tot_reach_length[hru_ndx] = 0;
   for (INT_PTR reach_ndx = 0; reach_ndx < m_reachArray.GetSize(); reach_ndx++)
   {
      Reach *pReach = m_reachArray[reach_ndx];
      int hru_id = -1; m_pReachLayer->GetData(pReach->m_polyIndex, m_colReachHRU_ID, hru_id);
      if (hru_id < 0)
      { // Look downstream to associate this reach with an HRU
         Reach * pDownStreamReach = GetReachFromNode(pReach->m_pDown);
         while (hru_id < 0 && pDownStreamReach != NULL)
         {
            m_pReachLayer->GetData(pDownStreamReach->m_polyIndex, m_colReachHRU_ID, hru_id);
            if (hru_id < 0) pDownStreamReach = GetReachFromNode(pDownStreamReach->m_pDown);
         } // end of while loop searching for a downstream reach already associated with an HRU
         if (hru_id >= 0) m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachHRU_ID, hru_id);
      } // end of looking downstream

      if (hru_id < 0)
      { // Didn't find an HRU-associated reach downstream. Look upstream instead of downstream to associate this reach with an HRU.
         Reach * pUpStreamReach = GetReachFromNode(pReach->m_pLeft);
         while (hru_id < 0 && pUpStreamReach != NULL)
         {
            m_pReachLayer->GetData(pUpStreamReach->m_polyIndex, m_colReachHRU_ID, hru_id);
            if (hru_id < 0) pUpStreamReach = GetReachFromNode(pUpStreamReach->m_pLeft);
         } // end of while loop searching for an ustream reach already associated with an HRU
         if (hru_id >= 0) m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colReachHRU_ID, hru_id);
      } // end of looking upstream

      int hru_ndx = m_pHRUlayer->FindIndex(m_colhruHRU_ID, hru_id);
      if (hru_ndx >= 0)
      {
         hru_tot_reach_length[hru_ndx] += pReach->m_length;
         HRU * pHRU = m_hruArray[hru_ndx];
         pHRU->m_reachNdxArray.Add((UINT)pReach->m_polyIndex);
      }
   } // end of loop thru reaches

   int missed_reaches = 0;
   for (INT_PTR reach_ndx = 0; reach_ndx < m_reachArray.GetSize(); reach_ndx++)
   {
      Reach *pReach = m_reachArray[reach_ndx];
      int reach_ndx_in_ReachLayer = pReach->m_polyIndex;
      int hru_id = -1; m_pReachLayer->GetData(reach_ndx_in_ReachLayer, m_colReachHRU_ID, hru_id);
      int hru_ndx = m_pHRUlayer->FindIndex(m_colhruHRU_ID, hru_id);
      if (hru_ndx >= 0)
      {
         float hru_frac = hru_tot_reach_length[hru_ndx] > 0 ? m_reachArray[reach_ndx]->m_length / hru_tot_reach_length[hru_ndx] : 0;
         m_pReachLayer->SetDataU(reach_ndx_in_ReachLayer, m_colReachHRU_FRAC, hru_frac);
      }
      else
      {
         missed_reaches++;
         CString msg;
         msg.Format("AssignReachesToHRUs() missed reach  reach_ndx = %d, reach_ndx_in_ReachLayer = %d, hru_id = %d, hru_ndx = %d",
            reach_ndx, reach_ndx_in_ReachLayer, hru_id, hru_ndx);
         Report::ErrorMsg(msg);
      }
   } // end of loop thru reaches to set HRU_FRAC attribute

   msg.Format("AssignReachesToHRUs() ending now.  missed_reaches = %d", missed_reaches); 
   if (missed_reaches > 0) Report::WarningMsg(msg); else Report::LogMsg(msg);

   return(missed_reaches == 0);
} // end of AssignReachesToHRUs()


bool FlowModel::InitReservoirs( void )
   {
   if ( m_colResID < 0 )
      return false;

   for (int i = 0; i < m_reservoirArray.GetSize(); i++)
   {
      Reservoir *pRes = m_reservoirArray[i];
      ASSERT(pRes != NULL);
      pRes->m_in_use = false;
   }

   // iterate through reach network, looking for reservoir ID's...
   int reachCount = m_pStreamLayer->GetRecordCount();

   for ( int i=0; i < reachCount; i++ )
      {
      int resID = -1;
      m_pStreamLayer->GetData( i, m_colResID, resID );

      if ( resID > 0 && resID < 100) // The reservoirs that we're interested in have IDs less than 100
         {
         Reservoir *pRes = FindReservoirFromID( resID );
      
         if ( pRes == NULL )
            {
            CString msg;
            msg.Format( "Flow: Unable to find reservoir ID '%i' in InitRes().", resID );
            Report::StatusMsg( msg );
            }
         else
            {
            if ( pRes->m_pReach == NULL )
               {
               Reach *pReach = (Reach*) m_reachTree.GetReachNodeFromPolyIndex( i );

               if ( pReach != NULL )
                  {
                  pRes->m_pReach = pReach;
                  pReach->m_pReservoir = pRes;
                  }
               else
                  {
                  CString msg;
                  msg.Format( "Flow:  Unable to locate reservoir (id:%i) in stream network", pRes->m_id );
                  Report::StatusMsg( msg );
                  }
               }  // end of: pRes->m_pReach
            }  // end of : else
         }  // end of: if ( resID > 0 )
      }  // end of: for each Reach


   /*///////////////////////////////////////////////////////////////////////////////////////////////////////////
   //Code here to load ResSIM inflows for testing...to be removed when discharge comes from FLOW  mmc 11_7_2012
   m_pResInflows = new FDataObj(0, 0 );
   LPCSTR ResSimInflows = "C:\\envision\\StudyAreas\\WW2100\\Reservoirs\\Inflows_from_ResSim\\ResSim_inflows_for_testing_cms.csv";
   
   m_pResInflows->ReadAscii( ResSimInflows );
   /*m_pCpInflows = new FDataObj(0, 0);
   LPCSTR CpLocalFlows = "C:\\envision\\StudyAreas\\WW2100\\Reservoirs\\Inflows_from_ResSim\\Control_point_local_flows_cms.csv";
   
   m_pCpInflows->ReadAscii( CpLocalFlows );
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////*/
   
   // for each reservoir, follow downstream until the first reach downstream from
   // the reaches associated with the reservoir is found
   int resCount = (int) m_reservoirArray.GetSize();
   int usedResCount = 0;

   for ( int i=0; i < resCount; i++ )
      {
      // for each reservoir, move the Reach pointer downstream
      // until a different resID found
      Reservoir *pRes = m_reservoirArray[ i ];
      ASSERT( pRes != NULL );

      Reach *pReach = pRes->m_pReach;   // initially , this points to a reach "in" the reservoir, if any
      
      if ( pReach != NULL )   // any associated reachs?
         {
         ReachNode *pNode = pReach;

         // traverse down until this node's resID != this one
         while ( pNode != NULL )
            {
            ReachNode *pDown = pNode->GetDownstreamNode( true );

            if ( pDown == NULL )  // root of tree?
               break;

            int reach_ndx = pDown->m_polyIndex;
            int resID = -1;
            m_pStreamLayer->GetData( reach_ndx, m_colResID, resID );

            // do we see no reservoir or a different reservoir? then quit the loop
            pNode = pDown;
            if ( resID != pRes->m_id )
               break;
            }

         // at this point, pNode points to the node below the last matching downstream reach, or NULL if there is none.
         // fix up the pointers
         if ( pNode != NULL )
            {
            Reach *pDownstreamReach = GetReachFromNode( pNode );
            pRes->m_pReach->m_pReservoir = NULL;
            pRes->m_pReach = pDownstreamReach;
            int reach_ndx = pDownstreamReach->m_polyIndex;
            m_pStreamLayer->SetDataU(reach_ndx, m_colStreamRESOUTFALL, pRes->m_id);

            // Write out the reservoir locations to the log.
               {
               CString msg;
               msg.Format("InitReservoirs() reservoir %2d comids: the downstream reach is \t%9d for %s",
                  pRes->m_id, pRes->m_pReach->m_reachID, pRes->m_name.GetString());
               Report::LogMsg(msg);

               pReach = pRes->m_pReach;
               int reach_comid = pReach->m_reachID; 
               int reach_ndx = m_pStreamLayer->FindIndex(m_colStreamCOMID, reach_comid);
               int comid_left = -1; m_pStreamLayer->GetData(reach_ndx, m_colStreamCOMID_LEFT, comid_left);
               msg.Format("\t\t%8d is an upstream comid (COMID_LEFT of %d)", comid_left, pRes->m_pReach->m_reachID);
               Report::LogMsg(msg);
               int comid_rt = -1; m_pStreamLayer->GetData(reach_ndx, m_colStreamCOMID_RT, comid_rt);
               if (comid_rt > 0)
               {
                  CString msg; msg.Format("\t\t%8d is an upstream comid (COMID_RT of %d)", comid_rt, pRes->m_pReach->m_reachID);
                  Report::LogMsg(msg);
               }

/*
               int next_comid = -1;
               int next_ndx = -1;
               
               int downstream_reach_comid = reach_comid;
               bool right_branch_flag = true;
               do
                  { 
                  // Is there a next_comid reach above reach_comid that we haven't already looked at?
                  next_comid = -1;
                  int comid_left = -1; m_pStreamLayer->GetData(reach_ndx, m_colStreamCOMID_LEFT, comid_left);
                  if (right_branch_flag)
                     {
                     m_pStreamLayer->GetData(reach_ndx, m_colStreamCOMID_RT, next_comid);
                     if (next_comid == comid_left) next_comid = 0; // Compensate for bad data in the reach shapefile.  For some reaches, the right and left branches are the same and non-zero.
                     if (next_comid > 0) next_ndx = m_pStreamLayer->FindIndex(m_colStreamCOMID, next_comid);
                     }
                  if (next_comid <= 0)
                     {
                     right_branch_flag = false;
                     next_comid = comid_left;
                     if (next_comid > 0) next_ndx = m_pStreamLayer->FindIndex(m_colStreamCOMID, next_comid);
                     }

                  if (next_comid <= 0) 
                     { // No, there is no next_comid reach above reach_comid that we haven't already looked at. Back up and see if we're done.                  
                     // We're done with reach_comid.  Back up past reach_comid, and keep backing up until
                     // we're on the right branch of some lower reach.
                     BackDownTheTree(&reach_comid, &reach_ndx, &right_branch_flag, downstream_reach_comid);
                     } // end of if (next_comid <= 0) - Logic for backing up.
                  else
                     { // Yes, next_comid is above reach_comid.  
                     // Is next_comid above the reservoir?
                     int next_res_id = -1; m_pStreamLayer->GetData(next_ndx, m_colStreamRES_ID, next_res_id);
                     if (next_res_id != pRes->m_id)
                        { // Yes, next_comid is just above the reservoir.
                        CString msg;
                        msg.Format("\t\t%8d is an upstream comid", next_comid);
                        Report::LogMsg(msg);

                        if (right_branch_flag) right_branch_flag = false; // Switch to the left branch.
                        else 
                           { // Back up and see if we're done.
                           // right_branch_flag was false, so we know the branch we're on is a left branch.
                           // At present, we're at next_comid, which is the left branch of reach_comid.
                           // So we're done with reach_comid.  Back up past reach_comid, and keep backing up until
                           // we're on the right branch of some lower reach.
                           BackDownTheTree(&reach_comid, &reach_ndx, &right_branch_flag, downstream_reach_comid);
                           } // end of if (right_branch_flag) ... else - The logic for backing up.
                        } // end of if (next_res_id != pRes->m_id) - The logic for recognizing an upstream reach.
                     else
                        { // No, this next_comid is still in the reservoir. Look for a reach further upstream.
                        reach_comid = next_comid;
                        reach_ndx = next_ndx;
                        right_branch_flag = true;
                        } // end of if (next_res_id != pRes->m_id) ... else - The logic for moving further upstream.
                     } // end of if (next_comid <= 0) ... else
                  } // end of do { ... }
               while (reach_comid != downstream_reach_comid);
*/
               } // end of block to write out reservoir locations to the log

            Reach* pReach = pDownstreamReach;
            ASSERT(pReach != NULL);
            if (pReach != NULL)
            {
               pReach->m_pReservoir = pRes;
               pReach->SetAtt(ReachRES_H2O, pRes->m_resWP.m_volume_m3);
               pReach->SetAtt(ReachRES_TEMP, pRes->m_resWP.WaterTemperature());
            }
            pRes->m_in_use = true;
            usedResCount++;

            //Initialize reservoir outflow to minimum outflow
            float minOutflow = pRes->m_minOutflow;
            pRes->m_outflow = minOutflow;
			pRes->m_outflowWP = WaterParcel(minOutflow, DEFAULT_REACH_H2O_TEMP_DEGC);

            //Initialize gate specific flow variables to zero
            pRes->m_maxPowerFlow = 0;
            pRes->m_minPowerFlow = 0;
            pRes->m_powerFlow = 0;
            
            pRes->m_maxRO_Flow = 0;
            pRes->m_minRO_Flow = 0;
            pRes->m_RO_flow = 0;
            
            pRes->m_maxSpillwayFlow = 0;
            pRes->m_minSpillwayFlow = 0;
            pRes->m_spillwayFlow = 0;

            //LoadTable(gpModel->m_path + pRes->m_dir + pRes->m_avdir + pRes->m_areaVolCurveFilename, (DataObj**)&(pRes->m_pAreaVolCurveTable), 0);

            // For all Reservoir Types but ControlPointControl
            //if (pRes->m_reservoirType == ResType_FloodControl || pRes->m_reservoirType == ResType_RiverRun)
            //   {
            //   LoadTable(gpModel->m_path + pRes->m_dir + pRes->m_redir + pRes->m_releaseCapacityFilename, (DataObj**)&(pRes->m_pCompositeReleaseCapacityTable), 0);
            //   LoadTable(gpModel->m_path + pRes->m_dir + pRes->m_redir + pRes->m_RO_CapacityFilename, (DataObj**)&(pRes->m_pRO_releaseCapacityTable), 0);
            //   LoadTable(gpModel->m_path + pRes->m_dir + pRes->m_redir + pRes->m_spillwayCapacityFilename, (DataObj**)&(pRes->m_pSpillwayReleaseCapacityTable), 0);
            //   }
            //
            //// Not Needed for RiverRun Reservoirs
            //if (pRes->m_reservoirType == ResType_FloodControl || pRes->m_reservoirType == ResType_CtrlPointControl)
            //   {
            //   LoadTable(gpModel->m_path + pRes->m_dir + pRes->m_rpdir + pRes->m_rulePriorityFilename, (DataObj**)&(pRes->m_pRulePriorityTable), 1);
            //
            //   if (pRes->m_reservoirType == ResType_FloodControl)
            //      {
            //      LoadTable(gpModel->m_path + pRes->m_dir + pRes->m_rcdir + pRes->m_ruleCurveFilename, (DataObj**)&(pRes->m_pRuleCurveTable), 0);
            //      LoadTable(gpModel->m_path + pRes->m_dir + pRes->m_rcdir + pRes->m_bufferZoneFilename, (DataObj**)&(pRes->m_pBufferZoneTable), 0);
            //
            //      //Load table with ResSIM outputs
            //      //LoadTable( pRes->m_dir+"Output_from_ResSIM\\"+pRes->m_ressimFlowOutputFilename,    (DataObj**) &(pRes->m_pResSimFlowOutput),    0 );
            //      LoadTable(pRes->m_dir + "Output_from_ResSIM\\" + pRes->m_ressimRuleOutputFilename, (DataObj**)&(pRes->m_pResSimRuleOutput), 1);
            //      }
            //   pRes->ProcessRulePriorityTable();
            //   }            
            }
         } // end of: if ( pReach != NULL )

      pRes->InitDataCollection();    // set up data object and add as output variable      
      }  // end of: for each Reservoir

   //if ( usedResCount > 0 )
   //   this->AddReservoirLayer();


   for (int i = 0; i < m_reservoirArray.GetSize(); i++)
      {
      Reservoir *pRes = m_reservoirArray.GetAt(i);
      gpFlow->AddInputVar(pRes->m_name + " ResMaintenance", pRes->m_probMaintenance, "Probability that the reservoir will need to be taken offline in any year");
      }
   


    // allocate any necessary "extra" state variables
   if (m_reservoirSvCount > 1)//then there are extra state variables..
      {
      int resCount = (int)m_reservoirArray.GetSize();
      for (int i = 0; i < resCount; i++)
         {
         Reservoir *pRes = m_reservoirArray[i];
         ASSERT(pRes != NULL);
         pRes->AllocateStateVars(m_reservoirSvCount - 1);
         }
      }
         
   CString msg;
   msg.Format( "Flow: Loaded %i reservoirs (%i were disconnected from the reach network and ignored)", usedResCount, resCount-usedResCount );
   Report::LogMsg( msg );
   
   return true;
   } // end of InitReservoirs()


void FlowModel::BackDownTheTree(int * pReachComid, int * pReachNdx, bool * pRightBranchFlag, int downstreamLimit)
// We're done with reach_comid.  Back up past reach_comid, and keep backing up until
// we're on the right branch of some lower reach.
{ 
   if (*pReachComid != downstreamLimit)
   {
      int comid_down = -1; m_pStreamLayer->GetData(*pReachNdx, m_colStreamCOMID_DOWN, comid_down);
      *pReachComid = comid_down;
      *pReachNdx = m_pStreamLayer->FindIndex(m_colStreamCOMID, *pReachComid);
   }

   // Now start looking for a reach which is a right branch, and go up the corresponding left branch.
   int comid_rt = -1;
   bool done = false;
   if (*pReachComid != downstreamLimit) do
   {
      int comid_down = -1; m_pStreamLayer->GetData(*pReachNdx, m_colStreamCOMID_DOWN, comid_down);
      int down_ndx = m_pStreamLayer->FindIndex(m_colStreamCOMID, comid_down);
      m_pStreamLayer->GetData(down_ndx, m_colStreamCOMID_RT, comid_rt);
      done = (comid_rt == *pReachComid);
      *pReachComid = comid_down;
      *pReachNdx = down_ndx;
   } while (!done && *pReachComid != downstreamLimit);
   *pRightBranchFlag = false; // Go up the left branch of the new reach_comid, or else quit altogether
} // end of BackDownTheTree()


bool FlowModel::InitRunReservoirs( EnvContext *pEnvContext )
   {
   // for each reservoir, follow downstream until the first reach downstream from
   // the reaches associated with the reservoir is found
   int resCount = (int) m_reservoirArray.GetSize();
   int usedResCount = 0;

   for ( int i=0; i < resCount; i++ )
      {
      // for each reservoir, move the Reach pointer downstream
      // until a different resID found
      Reservoir *pRes = m_reservoirArray[ i ];
      ASSERT( pRes != NULL );

      Reach *pReach = pRes->m_pReach;   // initially , this points to a reach "in" the reservoir, if any
      
      if ( pReach != NULL )   // any associated reaches?  if not, this Reservoir is ignored
         {
         //Initialize reservoir outflow to minimum outflow
         float minOutflow = pRes->m_minOutflow;
         pRes->m_outflow = minOutflow;
		 pRes->m_outflowWP = WaterParcel(minOutflow, DEFAULT_REACH_H2O_TEMP_DEGC);

         //Initialize gate specific flow variables to zero
         pRes->m_maxPowerFlow = 0;
         pRes->m_minPowerFlow = 0;
         pRes->m_powerFlow = 0;
         
         pRes->m_maxRO_Flow = 0;
         pRes->m_minRO_Flow = 0;
         pRes->m_RO_flow = 0;
         
         pRes->m_maxSpillwayFlow = 0;
         pRes->m_minSpillwayFlow = 0;
         pRes->m_spillwayFlow = 0;

         CString path = gpModel->m_path + pRes->m_dir + pRes->m_avdir + pRes->m_areaVolCurveFilename;

         LoadTable( path, (DataObj**)&(pRes->m_pAreaVolCurveTable), 0);

         // For all Reservoir Types but ControlPointControl
         if (pRes->m_reservoirType == ResType_FloodControl || pRes->m_reservoirType == ResType_RiverRun)
            {
            int nRows_CompositeReleaseCapacity = LoadTable(gpModel->m_path + pRes->m_dir + pRes->m_redir + pRes->m_releaseCapacityFilename, (DataObj**)&(pRes->m_pCompositeReleaseCapacityTable), 0);
            int nRows_RO_releaseCapacity = LoadTable(gpModel->m_path + pRes->m_dir + pRes->m_redir + pRes->m_RO_CapacityFilename, (DataObj**)&(pRes->m_pRO_releaseCapacityTable), 0);
            int nRows_SpillwayReleaseCapacity = LoadTable(gpModel->m_path + pRes->m_dir + pRes->m_redir + pRes->m_spillwayCapacityFilename, (DataObj**)&(pRes->m_pSpillwayReleaseCapacityTable), 0);
            if (nRows_CompositeReleaseCapacity <= 0 || nRows_RO_releaseCapacity <= 0 || nRows_SpillwayReleaseCapacity <= 0)
               {
               CString msg;
               msg.Format("*** InitRunReservoirs(): nRows_CompositeReleaseCapacity = %d, nRows_RO_releaseCapacity = %d, nRows_SpillwayReleaseCapacity = %d",
                  nRows_CompositeReleaseCapacity, nRows_RO_releaseCapacity, nRows_SpillwayReleaseCapacity);
               Report::LogMsg(msg);
               }
            }

         // Not Needed for RiverRun Reservoirs
         if (pRes->m_reservoirType == ResType_FloodControl || pRes->m_reservoirType == ResType_CtrlPointControl)
            {
            int row_count = LoadTable(gpModel->m_path + pRes->m_dir + pRes->m_rpdir + pRes->m_rulePriorityFilename, (DataObj**)&(pRes->m_pRulePriorityTable), 1);
            if (row_count <= 0) return(false);
            if (pRes->m_reservoirType == ResType_FloodControl)
               {
               row_count = LoadTable(gpModel->m_path + pRes->m_dir + pRes->m_rcdir + pRes->m_ruleCurveFilename, (DataObj**)&(pRes->m_pRuleCurveTable), 0);
               if (row_count <= 0) return(false);
               row_count = LoadTable(gpModel->m_path + pRes->m_dir + pRes->m_rcdir + pRes->m_bufferZoneFilename, (DataObj**)&(pRes->m_pBufferZoneTable), 0);
               if (row_count <= 0) return(false);

               //Load table with ResSIM outputs
               //LoadTable( pRes->m_dir+"Output_from_ResSIM\\"+pRes->m_ressimFlowOutputFilename,    (DataObj**) &(pRes->m_pResSimFlowOutput),    0 );
               row_count = LoadTable(pRes->m_dir + "Output_from_ResSIM\\" + pRes->m_ressimRuleOutputFilename, (DataObj**)&(pRes->m_pResSimRuleOutput), 1);
               if (row_count <= 0) return(false);
            }

            pRes->ProcessRulePriorityTable();
            }      
         } // end of: if ( pReach != NULL )

      ASSERT( pRes != NULL );
      ASSERT( pRes->m_pResData != NULL );
      ASSERT( pRes->m_pResSimFlowCompare != NULL ); 
      ASSERT( pRes->m_pResSimRuleCompare != NULL );

      pRes->m_filled = 0;

      pRes->m_pResData->ClearRows();
      //pRes->m_pResMetData->ClearRows();
      pRes->m_pResSimFlowCompare->ClearRows();
      pRes->m_pResSimRuleCompare->ClearRows();  

      //pRes->InitDataCollection();    // set up data object and add as output variable      
      }  // end of: for each Reservoir

   //if ( usedResCount > 0 )
   //   this->AddReservoirLayer();
   
   //CString msg;
   //msg.Format( "Flow: Loaded %i reservoirs (%i were disconnected from the reach network and ignored)", usedResCount, resCount-usedResCount );
   //Report::LogMsg( msg );
   
   return true;
   }



//--------------------------------------------------------------------------------------------------
// Network simplification process:
//
// basic idea: iterate through the the stream network starting with the leaves and
// working towards the roots.  For each reach, if it doesn't satisfy the stream query
// (meaning it is to be pruned), move its corresponding HRUs to the first downstream 
// that satisfies the query.
//
// The basic idea of simplification is to aggregate Catchments using two distinct methods: 
//   1) combining catchments that meet a set of constraints, typically focused on 
// 
//  returns number of combined catchments
//--------------------------------------------------------------------------------------------------

int FlowModel::SimplifyNetwork( void )
   {
   if ( m_pStreamQuery == NULL )
      return -1;

   int startReachCount = (int) m_reachArray.GetSize(); 
   int startCatchmentCount = (int) m_catchmentArray.GetSize();
   int startHRUCount=0;
   
   for ( int i=0; i < (int) m_catchmentArray.GetSize(); i++ )
      startHRUCount += m_catchmentArray[i]->GetHRUCountInCatchment();

   CString msg( "Flow: Simplifying Catchments/Reaches: " );

   if ( this->m_streamQuery.GetLength() > 0 )
      {
      msg += "Stream Query: '";
      msg += this->m_streamQuery;
      msg += "' ";
      }

   if ( this->m_minCatchmentArea > 0 )
      {
      CString _msg;
      _msg.Format( "Min Catchment Area: %.1g ha, Max Catchment Area: %.1g ha", (float) this->m_minCatchmentArea/M2_PER_HA, (float) this->m_maxCatchmentArea/M2_PER_HA );
      msg += _msg;
      }

   Report::LogMsg( msg );

   // iterate through the roots of the tree, applying catchment constraints
   try 
      {
      // now, starting with the leaves, move down stream, looking for excluded stream reaches
      for ( int i=0; i < m_reachTree.GetRootCount(); i++ )
         {
         ReachNode *pRoot = m_reachTree.GetRootNode( i );

         // algorithm:  start with the root.  Simplify the left side, then the right side,
         // recursively with the following rule:
         //   if the current reachnode satifies the query, recurse
         //   otherwise, relocate the HRUs from everything above and including it to the downstream
         //   reach node's catchment

         if ( this->m_streamQuery.GetLength() > 0 )
            {
            int catchmentCount = (int) m_catchmentArray.GetSize();

            SimplifyTree( pRoot );   // prune leaves based on reach query

            int newCatchmentCount = (int) m_catchmentArray.GetSize();
            
            CString msg;
            msg.Format( "Flow: Pruned leaves for root %i, %i Catchments reduced to %i (%i combined)", i, catchmentCount, newCatchmentCount, (catchmentCount-newCatchmentCount) );
            Report::LogMsg( msg );
            }

         if ( this->m_minCatchmentArea > 0 && this->m_maxCatchmentArea > 0 )
            {
            int catchmentCount = (int) m_catchmentArray.GetSize();

            ApplyCatchmentConstraints( pRoot );              // apply min area constraints
            
            int newCatchmentCount = (int) m_catchmentArray.GetSize();
   
            CString msg;
            msg.Format( "Flow: applying area constraints for root %i, %i Catchments reduced to %i (%i combined)", i, catchmentCount, newCatchmentCount, (catchmentCount-newCatchmentCount) );
            Report::LogMsg( msg );
            }         
         }
      }

   catch( CException *ex) // Catch all other CException derived exceptions 
      {
      TCHAR info[300]; 
      if ( ex->GetErrorMessage( info, 300 ) == TRUE )
         {
         CString msg( "Exception thrown in Flow during SimplifyTree() " );
         msg += info;
         Report::ErrorMsg( msg );
         }
      ex->Delete(); // Will delete exception object correctly 
      } 
   catch(...) 
      { 
      CString msg( "Unknown Exception thrown in Flow during SimplifyTree() " );
      Report::ErrorMsg( msg );
      }

   // update catchment/reach cumulative drainage areas
   // this->PopulateCatchmentCumulativeAreas();

   int finalReachCount = (int) this->m_reachArray.GetSize();
   int finalHRUCount=0;

   // update catchID and join cols to reflect the simplified network
   m_pCatchmentLayer->SetColData( this->m_colCatchmentJoin, VData( -1 ), true );
   m_pStreamLayer->SetColData( this->m_colStreamJoin, VData( -1 ), true );
   
   float areaTotal = 0.0f;
   
   for ( int i=0; i < (int) m_catchmentArray.GetSize(); i++ )
      {
      Catchment *pCatchment = m_catchmentArray[ i ];
      finalHRUCount += pCatchment->GetHRUCountInCatchment();

      for ( int j=0; j < pCatchment->GetHRUCountInCatchment(); j++ )
         {
         HRU *pHRU = pCatchment->GetHRUfromCatchment( j );
         ASSERT( pHRU != NULL );
         areaTotal += pHRU->m_HRUtotArea_m2;

         for ( int k=0; k < (int) pHRU->m_polyIndexArray.GetSize(); k++ )
            {
            // write new catchment ID to catchmentID col and catchment join col 
            m_pCatchmentLayer->SetData( pHRU->m_polyIndexArray[ k ], m_colCatchmentCatchID, pCatchment->m_id );
            m_pCatchmentLayer->SetData( pHRU->m_polyIndexArray[ k ], m_colCatchmentJoin, pCatchment->m_id );
            }
         }
      }

   // update stream join values
   for ( int i=0; i < this->m_reachArray.GetSize(); i++ )
      {
      Reach *pReach = m_reachArray[ i ];

      if ( pReach->m_catchmentArray.GetSize() > 0 )
         m_pStreamLayer->SetData( pReach->m_polyIndex, this->m_colStreamJoin, pReach->m_catchmentArray[0]->m_id );
      }
   
   msg.Format( "Flow: Reach Network Simplified from %i to %i Reaches, %i to %i Catchments and %i to %i HRUs", 
   startReachCount, finalReachCount, startCatchmentCount, (int) m_catchmentArray.GetSize(), startHRUCount, finalHRUCount);
   Report::LogMsg( msg );

   return startCatchmentCount - ((int) m_catchmentArray.GetSize() );
   }


// returns number of reaches in the simplified tree
int FlowModel::SimplifyTree( ReachNode *pNode )
   {
   if ( pNode == NULL )
      return 0;

   // skip phantom nodes - they have no associated catchments (they always succeed)
   if ( pNode->IsPhantomNode() || pNode->IsRootNode() )
      {
      int leftCount  = SimplifyTree( pNode->m_pLeft );
      int rightCount = SimplifyTree( pNode->m_pRight );
      return leftCount + rightCount;
      }

   // does this node satisfy the query?
   ASSERT( pNode->m_polyIndex >= 0 );
   bool result = false;
   bool ok = m_pStreamQE->Run( m_pStreamQuery, pNode->m_polyIndex, result );
   Reach *pReach = GetReachFromNode( pNode );
   if ( (ok && result) || pReach->m_isMeasured)
      {
      // it does, so we keep this one and move on to the upstream nodes
      int leftCount  = SimplifyTree( pNode->m_pLeft );
      int rightCount = SimplifyTree( pNode->m_pRight );    
      return 1 + leftCount + rightCount;
      }  

   else  // it fails, so this reach and all reaches of above it need to be rerouted to the reach below us
      {  // relocate this and upstream reaches to this nodes down stream pointer
      if ( pNode->IsRootNode() )    // if this is a root node, nothing required, the whole
         return 0;                  // tree will be deleted  NOTE: NOT IMPLEMENTED YET
      else
         {
         ReachNode *pDownNode = pNode->GetDownstreamNode( true );  // skips phantoms
         Reach *pDownReach = GetReachFromNode( pDownNode );
         if ( pDownReach == NULL  )
            {
            CString msg;
            msg.Format( "Flow: Warning - phantom root node encountered below Reach ID %i. Truncation can not proceed for this tree...", pNode->m_reachID );
            Report::WarningMsg( msg );
            }
         else
            {
            int beforeCatchCount = (int) this->m_catchmentArray.GetSize();
            MoveCatchments( pDownReach, pNode, true );   // moves all catchments for this and above (recursively)
            int afterCatchCount = (int) this->m_catchmentArray.GetSize();

            //CString msg;
            //msg.Format( "Flow: Pruning %i catchments that don't satisfy the Simplification query", (beforeCatchCount-afterCatchCount) );
            //Report::LogMsg( msg );

            RemoveReaches( pNode );   // removes all Reach and ReachNodes above and including this ReachNode
            }

         return 0;
         }
      }
   }


int FlowModel::ApplyCatchmentConstraints( ReachNode *pRoot )
   {
   // applies min/max size constraints on a catchment.  Basic rule is:
   // if the catchment for this reach is too small, combine it with another
   // reach using the following rules:
   //    1) if one of the upstream catchment is below the minimum size, add this reaches
   //       catchment to the upstream reach.  (Note that this works best with a root to leaf search)
   //    2) if there is no upstream capacity, combine this reach with the downstream reach (note
   //       that this will work best with a leaf to root, breadth first search.)

   // iterate through catchments. Assumes cumulative drainage areas are already calculated
   int removeCount = 1;
   int pass = 0;

   while ( removeCount > 0 && pass < 5 )
      {
      removeCount = 0;
      for ( int i=0; i < (int) m_reachArray.GetSize(); i++ )
         {
         Reach *pReach = m_reachArray[ i ];
         int removed = ApplyAreaConstraints( pReach ); 

         // if this reach was removed, then move i back one to compensate
        // i -= removed;

         removeCount += removed;
         }

      pass++;

      CString msg;
      msg.Format( "Flow: Applying catchment constraints (pass %i) - %i catchments consolidated", pass, removeCount );

      Report::LogMsg( msg );
      }

   return pass;
   }


// return 0 if no change, 1 if Reach is altered
int FlowModel::ApplyAreaConstraints( ReachNode *pNode )
   {
   // this method examines the current reach.  if the contributing area of the reach is too small, try to combine it with another catchments
   // Basic rule is:
   // if the catchment for this reach is too small, combine it with another
   // reach using the following rules:
   //    1) if one of the upstream catchment is below the minimum size, add this reaches
   //       catchment to the upstream reach.  (Note that this works best with a root to leaf search)
   //    2) if there is no upstream capacity, combine this reach with the downstream reach (note
   //       that this will work best with a leaf to root, breadth first search.)
   if ( pNode == NULL )   
      return 0;

   Reach *pReach = GetReachFromNode( pNode );

   if ( pReach == NULL )      // no reach exist no action required
      return 0;

   // does this Catchment  satisfy the min area constraint?  If so, we are done.
   float catchArea = pReach->GetCatchmentArea();

   if ( catchArea > m_minCatchmentArea )
      return 0;

   // catchment area for this reach is too small, combine it.
   // Step one, look above, left 
   ReachNode *pLeft = pNode->m_pLeft;
   while ( pLeft != NULL && GetReachFromNode( pLeft ) == NULL )
      pLeft = pLeft->m_pLeft;

   // does a left reach exist?  if so, check area and combine if minimum not satisfied
   if ( pLeft != NULL )
      {
      Reach *pLeftReach = GetReachFromNode( pLeft );
      ASSERT(  pLeftReach != NULL );
      if (!pLeftReach->m_isMeasured)
         {
         // is there available capacity upstream?
         if ( ( pLeftReach->GetCatchmentArea() + catchArea ) <= m_maxCatchmentArea)
            {
            MoveCatchments( pReach, pLeft, false );   // move the upstream reach to this one, no recursion
           // RemoveReach( pLeft ); //leave reaches in the network  KBV
            RemoveCatchment(pLeft);
            return 1;
            }
         }
      }
   
   // look above, right
   ReachNode *pRight = pNode->m_pRight;
   while ( pRight != NULL && GetReachFromNode( pRight ) == NULL )
      pRight = pRight->m_pRight;

   // does a right reach exist?  if so, check area and combine if minimum not satisfied
   if ( pRight != NULL )
      {
      Reach *pRightReach = GetReachFromNode( pRight );
      ASSERT(  pRightReach != NULL );
      if (!pRightReach->m_isMeasured)
         {
         // is there available capacity upstream?
         if ( ( pRightReach->GetCatchmentArea() + catchArea ) <= m_maxCatchmentArea )
            {
            MoveCatchments( pReach, pRight, false );   // move the upstream reach to this one, no recursion
           // RemoveReach( pRight );  //leave reaches in the network KBV
            RemoveCatchment(pRight);
            return 1;
            }
         }
      }

   // no capacity upstream, how about downstream?
   ReachNode *pDown = pNode->m_pDown;
   while ( pDown != NULL && GetReachFromNode( pDown ) == NULL )
      pDown = pDown->m_pDown;

   // does a down reach exist?  if so, check area and combine if minimum not satisfied
   if ( pDown != NULL && pReach->m_isMeasured==false )
      {
      Reach *pDownReach = GetReachFromNode( pDown );
      Reach *pReach = GetReachFromNode( pNode );
      ASSERT(  pDownReach != NULL );

      // is there available capacity upstream?
    //  if ( ( pDownReach->m_cumUpstreamArea + catchArea ) <= m_maxCatchmentArea )
     
      if (( pDownReach->GetCatchmentArea() + catchArea ) <= m_maxCatchmentArea )
         {
         MoveCatchments( pDownReach, pNode, false );   // move from this reach to the downstream reach
         //RemoveReach( pNode ); //leave reaches in the network  KBV
         RemoveCatchment(pNode);
         return 1;
         }
      }

   // else, no capacity anywhere adjacent to this reach, so give up
   return 0;
   }





float FlowModel::PopulateCatchmentCumulativeAreas( void )
   {
   float area = 0;
   for ( int i=0; i < m_reachTree.GetRootCount(); i++ )
      {
      ReachNode *pRoot = m_reachTree.GetRootNode( i );
      area += PopulateCatchmentCumulativeAreas( pRoot );
      }

   return area;
   }


// updates internal variables and (if columns are defined) the stream and catchment
// coverages with cumulative values.  Assumes internal catchment areas have already been 
// determined.

float FlowModel::PopulateCatchmentCumulativeAreas( ReachNode *pNode )
   {
   if ( pNode == NULL )
      return 0;

   float leftArea  = PopulateCatchmentCumulativeAreas( pNode->m_pLeft );
   float rightArea = PopulateCatchmentCumulativeAreas( pNode->m_pRight );

   Reach *pReach = GetReachFromNode( pNode );

   if ( pReach != NULL )
      {
      float area = 0;

      for ( int i=0; i < (int) pReach->m_catchmentArray.GetSize(); i++ )
         area += pReach->m_catchmentArray[ i ]->m_area;

      area += leftArea;
      area += rightArea;

      // write to coverages
      if ( m_colStreamCumArea >= 0 )
         m_pStreamLayer->SetData( pReach->m_polyIndex, m_colStreamCumArea, area );

      if ( m_colCatchmentCumArea >= 0 )
         {
         for ( int i=0; i < (int) pReach->m_catchmentArray.GetSize(); i++ )
            {
            Catchment *pCatch = pReach->m_catchmentArray[ i ];
            SetCatchmentData( pCatch, m_colCatchmentCumArea, area );
            }
         }

      pReach->m_cumUpstreamArea = area;
      return area;
      }

   return leftArea + rightArea;
   }



// FlowModel::MoveCatchments() takes the catchment HRUs associated wih the "from" node and moves them to the
//    "to" reach, and optionally repeats recursively up the network.
int FlowModel::MoveCatchments( Reach *pToReach, ReachNode *pFromNode, bool recurse )
   {
   if ( pFromNode == NULL || pToReach == NULL )
      return 0;

   if ( pFromNode->IsPhantomNode() )   // for phantom nodes, always recurse up
      {
      int leftCount  = MoveCatchments( pToReach, pFromNode->m_pLeft, recurse  );
      int rightCount = MoveCatchments( pToReach, pFromNode->m_pRight, recurse );
      return leftCount + rightCount;
      }
   
   Reach *pFromReach = GetReachFromNode( pFromNode );

   int fromCatchmentCount = (int) pFromReach->m_catchmentArray.GetSize();

   // SPECIAL CASE:  fromCatchmentCount = 0, then there are no catchments in the
   // upstream reach, so just rec

   // if the from reach is measured, we don't combine it's catchments.
   // or, if the from reach has no catchments, there isn't anything to do either
   if ( pFromReach->m_isMeasured == false && fromCatchmentCount > 0 )
      {
      // get the Catchments associated with this reach and combine them into the
      // first catchment in the "to" reach catchment list
      int toCatchmentCount = (int) pToReach->m_catchmentArray.GetSize();
   
      Catchment *pToCatchment = NULL;
      
      if ( toCatchmentCount == 0 )//KELLIE
         {
         m_numReachesNoCatchments++;
         pToCatchment = AddCatchment( 10000+m_numReachesNoCatchments, pToReach );
         }
      else
         pToCatchment = pToReach->m_catchmentArray[ 0 ];

      ASSERT( pToCatchment != NULL );

      for ( int i=0; i < fromCatchmentCount; i++ ) 
         {
         Catchment *pFromCatchment = pFromReach->m_catchmentArray.GetAt( i );
      
         pToReach->m_cumUpstreamArea   = pFromReach->m_cumUpstreamArea + pToReach->GetCatchmentArea();
         pToReach->m_cumUpstreamLength = pFromReach->m_cumUpstreamLength + pToReach->m_length;

         CombineCatchments( pToCatchment, pFromCatchment, true );   // deletes the original catchment
         }
      
      // add the from reach length to the to reach.  Note: this assumes the from reach will removed from Flow
      // pToReach->m_length += pFromReach->m_length;     // JPB KBV

      // remove dangling Catchment pointers from the From reach
      pFromReach->m_catchmentArray.RemoveAll();       // only ptrs deleted, not the catchments
      }
      
   // recurse if specified
   if ( recurse && !pFromReach->m_isMeasured) // we don't want to recurse farther if we have find a reach for which we are saving model output for comparison with measurements
      {
      ReachNode *pLeft  = pFromNode->m_pLeft;
      ReachNode *pRight = pFromNode->m_pRight;

      int leftCount  = MoveCatchments( pToReach, pLeft,  recurse );
      int rightCount = MoveCatchments( pToReach, pRight, recurse );

      return fromCatchmentCount + leftCount + rightCount;
      }
   else if ( recurse && pFromReach->m_isMeasured) // so skip the measured reach, and continue recursion above it
      {
      ReachNode *pLeft  = pFromNode->m_pLeft;
      ReachNode *pRight = pFromNode->m_pRight;

      int leftCount  = MoveCatchments( pFromReach, pLeft,  recurse );
      int rightCount = MoveCatchments( pFromReach, pRight, recurse );

      return fromCatchmentCount + leftCount + rightCount;
      }
   else
      return fromCatchmentCount;
   }


// Combines the two catchments into the target catchment, copying/combining HRUs in the process.
// Depending on the deleteSource flag, the source catchment is optionally deleted

int FlowModel::CombineCatchments( Catchment *pTargetCatchment, Catchment *pSourceCatchment, bool deleteSource )
   {
   // get list of polygons to move from soure to target
   int hruCount = pSourceCatchment->GetHRUCountInCatchment();

   CArray< int > polyArray;

   for ( int i=0; i < hruCount; i++ )
      {
      HRU *pHRU = pSourceCatchment->GetHRUfromCatchment( i );
      
      for ( int j=0; j < pHRU->m_polyIndexArray.GetSize(); j++ )
         polyArray.Add( pHRU->m_polyIndexArray[ j ] ); 
      }

   // have polygon list built, generate/add to target catchments HRUs
   BuildCatchmentFromPolygons( pTargetCatchment, polyArray.GetData(), (int) polyArray.GetSize() );
   
 //  pTargetCatchment->m_area += pSourceCatchment->m_area;

   // remove from source
   //pSourceCatchment->RemoveAllHRUs();     // this deletes the HRUs
   RemoveCatchmentHRUs( pSourceCatchment );
   
   if ( deleteSource )
      {
      // remove from global list and delete catchment
      for( int i=0; i < (int) m_catchmentArray.GetSize(); i++ )
         {
         if ( m_catchmentArray[ i ] == pSourceCatchment )
            {
            m_catchmentArray.RemoveAt( i );
            break;
            }     
         }
      }

   return (int) polyArray.GetSize();
   }


int FlowModel::RemoveCatchmentHRUs( Catchment *pCatchment )
   {
   int count = pCatchment->GetHRUCountInCatchment();
   // first, remove from FlowModel
   for ( int i=0; i < count; i++ )
      {
      HRU *pHRU = pCatchment->GetHRUfromCatchment( i );
      this->m_hruArray.Remove( pHRU );    // deletes the HHRU
      }

   // next, remove pointers from catchment
   pCatchment->m_catchmentHruArray.RemoveAll();
   pCatchment->m_area = 0;

   return count;
   }




// removes all reaches and reachnodes, starting with the passed in node
int FlowModel::RemoveReaches( ReachNode *pNode )
   {
   if ( pNode == NULL )
      return 0;

   int removed = 0;

   if ( pNode->IsPhantomNode() == false )
      {
      if ( m_colReachSTRM_ORDER >= 0 )
         {
         int order = 0;
         m_pStreamLayer->GetData( pNode->m_polyIndex, m_colReachSTRM_ORDER, order );
         m_pStreamLayer->SetData( pNode->m_polyIndex, m_colReachSTRM_ORDER, -order );
         }

      Reach *pReach = GetReachFromNode( pNode );
      removed++;

      // remove from internal list
      for ( int i=0; i < (int) m_reachArray.GetSize(); i++ )
         {
         if ( m_reachArray[ i ] == pReach )
            {
            m_reachArray.RemoveAt( i );    // deletes the Reach
            break;
            }
         }
      }

   // remove from reach tree (note this does not delete the ReachNode, it turns it into a phantom node
   m_reachTree.RemoveNode( pNode, false );    // don't delete upstream nodes, this will occur below

   // recurse
   int leftCount  = RemoveReaches( pNode->m_pLeft );
   int rightCount = RemoveReaches( pNode->m_pRight );

   return removed + leftCount + rightCount;
   }


bool FlowModel::RemoveCatchment( ReachNode *pNode )
   {
   Reach *pReach = GetReachFromNode( pNode ); 

   if ( pReach == NULL )    // NULL, phantom, or no associated Reach
      return 0;

   if ( pReach->IsPhantomNode() )
      return 0;

   if ( pReach->IsRootNode() )
      return 0;

   // delete any associated catchments
   for ( int i=0; i < (int) pReach->m_catchmentArray.GetSize(); i++ )
      {
      Catchment *pCatchment = pReach->m_catchmentArray[ i ];

      for ( int j=0; j < (int) m_catchmentArray.GetSize(); j++ )
         {
         if ( m_catchmentArray[ i ] == pCatchment )
            {  
            m_catchmentArray.RemoveAt( i );   // deletes the catchment
            break;
            }
         }
      }
   return true;
   }

// removes this reach and reachnode, converting the node into a phantom node
bool FlowModel::RemoveReach( ReachNode *pNode )
   {
   Reach *pReach = GetReachFromNode( pNode ); 

   if ( pReach == NULL )    // NULL, phantom, or no associated Reach
      return 0;

   if ( pReach->IsPhantomNode() )
      return 0;

   if ( pReach->IsRootNode() )
      return 0;

   // delete any associated catchments
   for ( int i=0; i < (int) pReach->m_catchmentArray.GetSize(); i++ )
      {
      Catchment *pCatchment = pReach->m_catchmentArray[ i ];

      for ( int j=0; j < (int) m_catchmentArray.GetSize(); j++ )
         {
         if ( m_catchmentArray[ i ] == pCatchment )
            {
            m_catchmentArray.RemoveAt( i );
            break;
            }
         }
      }

   // remove from internal list
   for ( int i=0; i < (int) m_reachArray.GetSize(); i++ )
      {
      if ( m_reachArray[ i ] == pReach )
         {
         m_reachArray.RemoveAt( i );    // does NOT delete Reach, just removes it from the list.  The
         break;                         // Reach object is managed by m_reachTree
         }
      }
 
   // remove from reach tree (actually, this just converts the node to a phantom node)
   m_reachTree.RemoveNode( pNode, false );    // don't delete upstream nodes (no recursion)

   return true;
   }



// load table take a filename, creates a new DataObject of the specified type, and fills it
// with the file-based table
int FlowModel::LoadTable( LPCTSTR filename, DataObj** pDataObj, int type )
   {
   if ( *pDataObj != NULL )
      {
      delete *pDataObj;
      *pDataObj = NULL;
      }

   if ( filename == NULL || lstrlen( filename ) == 0 )
      {
      CString msg( "Flow: bad table name specified" );
      if ( filename != NULL )
         {
         msg += ": ";
         msg += filename;
         }
      
      Report::ErrorMsg( msg);
      return 0;
      }

   CString path( filename );
   this->ApplyMacros( path );

   switch( type )
      {
      case 0:
         *pDataObj = new FDataObj;
         ASSERT( *pDataObj != NULL );
         break;

      case 1:
         *pDataObj = new VDataObj;
         ASSERT( *pDataObj != NULL );
         break;

      default:
         *pDataObj = NULL;
         return -1;
      }

   
   if ( (*pDataObj)->ReadAscii( path ) <= 0 )
      {
      delete *pDataObj;
      *pDataObj = NULL;
   
      CString msg( "Flow: Error reading table: " );
      msg += ": ";
      msg += path;
      Report::ErrorMsg( msg);
      return 0;
      }

   (*pDataObj)->SetName( path );

   return (*pDataObj)->GetRowCount();
   }
    
// deprecated
bool FlowModel::InitFluxes( void )
   {
   int fluxCount = 0;
   int fluxInfoCount = (int) m_fluxInfoArray.GetSize();

   for ( int i=0; i < fluxInfoCount; i++ )
      {
      FluxInfo *pFluxInfo = m_fluxInfoArray[ i ];

      if ( pFluxInfo->m_sourceQuery.IsEmpty() && pFluxInfo->m_sinkQuery.IsEmpty() )
         {
         pFluxInfo->m_use = false;
         continue;
         }

      // source syntax is:
      // <flux name="Precipitation" path="WHydro.dll" description="" 
      //   source_type="hrulayer:*" source_query="AREA > 0" sink_type="" 
      //   sink_query=""  flux_handler="PrecipFluxHandler" />

      // first, figure out what kind of flux it is
      // if path is a dll and there is a flux handler defined, then it a function
      // if the path has a database extension, it's a database of some type.

      LPTSTR buffer = pFluxInfo->m_path.GetBuffer();
      LPTSTR ext = _tcsrchr( buffer, '.' );
      pFluxInfo->m_path.ReleaseBuffer();

      if ( ext == NULL )
         {
         CString msg( _T("Flow: Error parsing flux path '" ) );
         msg += pFluxInfo->m_path;
         msg += _T( "'.  Invalid file extension.  The flux will be ignored." );
         Report::WarningMsg( msg );
         pFluxInfo->m_use = false;
         continue;
         }

      ext++;
      
      FLUXSOURCE fsType = FS_UNDEFINED;
      if ( _tcsicmp( ext, "dll" ) == 0 )
         {
         if ( pFluxInfo->m_fnName.IsEmpty() )
            {
            CString msg( "Flux: Missing 'flux_handler' attribute for DLL-based flux source ");
            msg += pFluxInfo->m_path;
            msg += ". This is a required attribute";
            Report::WarningMsg( msg );
               pFluxInfo->m_use = false;
            continue;
            }
         else     // load dll
            {
            if ( ( pFluxInfo->m_hDLL = AfxLoadLibrary( pFluxInfo->m_path ) ) == NULL )
               {
               CString msg( "Flux: Unable to load flux specified in ");
               msg += pFluxInfo->m_path;
               msg += " or a dependent DLL. This flux will be disabled";
               Report::WarningMsg( msg );
               pFluxInfo->m_use = false;
               continue;
               }

            pFluxInfo->m_pFluxFn = (FLUXFN) ::GetProcAddress( pFluxInfo->m_hDLL, pFluxInfo->m_fnName );

            if ( pFluxInfo->m_pFluxFn == NULL )
               {
               CString msg( "Flux: Unable to load function '" );
               msg += pFluxInfo->m_fnName;
               msg += "'specifed in the 'flux_handler' attribute in ";
               msg += pFluxInfo->m_path;
               msg += ".  This flux will be disabled";
               Report::WarningMsg( msg );
               continue;
               }

            if ( pFluxInfo->m_initFn.GetLength() > 0 )
               {
               pFluxInfo->m_pInitFn = (FLUXINITFN) ::GetProcAddress( pFluxInfo->m_hDLL, pFluxInfo->m_initFn );
               if ( pFluxInfo->m_pInitFn == NULL )
                  {
                  CString msg( "Flux: Unable to load function '" );
                  msg += pFluxInfo->m_initFn;
                  msg += "'specifed in the 'init_handler' attribute in ";
                  msg += pFluxInfo->m_path;
                  msg += ".";
                  Report::WarningMsg( msg );
                  }
               }

            if ( pFluxInfo->m_initRunFn.GetLength() > 0 )
               {
               pFluxInfo->m_pInitRunFn = (FLUXINITRUNFN) ::GetProcAddress( pFluxInfo->m_hDLL, pFluxInfo->m_initRunFn );
               if ( pFluxInfo->m_pInitRunFn == NULL )
                  {
                  CString msg( "Flux: Unable to load function '" );
                  msg += pFluxInfo->m_initRunFn;
                  msg += "'specifed in the 'initRun_handler' attribute in ";
                  msg += pFluxInfo->m_path;
                  msg += ".";
                  Report::WarningMsg( msg );
                  }
               }

            if ( pFluxInfo->m_endRunFn.GetLength() > 0 )
               {
               pFluxInfo->m_pEndRunFn = (FLUXENDRUNFN) ::GetProcAddress( pFluxInfo->m_hDLL, pFluxInfo->m_endRunFn );
               if ( pFluxInfo->m_pEndRunFn == NULL )
                  {
                  CString msg( "Flux: Unable to load function '" );
                  msg += pFluxInfo->m_endRunFn;
                  msg += "'specifed in the 'endRun_handler' attribute in ";
                  msg += pFluxInfo->m_path;
                  msg += ".";
                  Report::WarningMsg( msg );
                  }
               }
            }  // end of: load DLL

         pFluxInfo->m_dataSource = FS_FUNCTION;
         }  // end of: if ( ext is DLL )

      else if ( ( _tcsicmp( ext, "csv" ) == 0 )
             || ( _tcsicmp( ext, "xls" ) == 0 )
             || ( _tcsicmp( ext, "dbf" ) == 0 )
             || ( _tcsicmp( ext, "mdb" ) == 0 )
             || ( _tcsicmp( ext, "ncf" ) == 0 ) )
         {
         fsType = FS_DATABASE;

         TCHAR *buffer = new TCHAR[ pFluxInfo->m_fieldName.GetLength()+1 ];
         lstrcpy( buffer, pFluxInfo->m_fieldName );

         LPTSTR tableName = buffer;
         LPTSTR fieldName = _tcschr( buffer, ':' );
         if ( fieldName == NULL )
            {
            *fieldName = '\0';
            fieldName++;
            tableName = NULL;
            }
         else
            fieldName = buffer;

         // have database info, load it
         ASSERT( pFluxInfo->m_pFluxData == NULL );
         pFluxInfo->m_pFluxData = new FDataObj;
         ASSERT( pFluxInfo->m_pFluxData != NULL );

         int records = pFluxInfo->m_pFluxData->ReadAscii( pFluxInfo->m_path );
         if ( records <= 0 )
            {
            CString msg( "Flux: Unable to load database '" );
            msg += pFluxInfo->m_path;
            msg += "'specified in the 'path' attribute of a flux. This flux will be disabled";
            Report::WarningMsg( msg );
            pFluxInfo->m_use = false;
            delete [] buffer;
            continue;
            }

         delete [] buffer;
         }
      
      // basics handled for the source of the flux.  
      // Next, figure out where this applies. This is determined by the associated query.
      if ( pFluxInfo->m_use == false )
         continue;

      // first, source
      if ( pFluxInfo->m_sourceLocation != FL_UNDEFINED )
         {
         ASSERT( pFluxInfo->m_sourceLocation != NULL );

         pFluxInfo->m_pSourceQuery = BuildFluxQuery( pFluxInfo->m_sourceQuery, pFluxInfo->m_sourceLocation );
   
         if ( pFluxInfo->m_pSourceQuery == NULL )
            {
            CString msg( "Flow: Error parsing flux source query: '" );
            msg += pFluxInfo->m_sourceQuery;
            msg += " for flux '";
            msg += pFluxInfo->m_name;
            msg += "' - this flux will be ignored";
            Report::ErrorMsg( msg );
            pFluxInfo->m_use = false;
            continue;
            }
         }

      // then sinks
      if ( pFluxInfo->m_sinkLocation != FL_UNDEFINED )
         {
         pFluxInfo->m_pSinkQuery = BuildFluxQuery( pFluxInfo->m_sinkQuery, pFluxInfo->m_sinkLocation );
   
         if ( pFluxInfo->m_pSinkQuery == NULL )
            {
            CString msg( "Flow: Error parsing flux sink query: '" );
            msg += pFluxInfo->m_sinkQuery;
            msg += " for flux '";
            msg += pFluxInfo->m_name;
            msg += "' - this flux will be ignored";
            Report::ErrorMsg( msg );
            pFluxInfo->m_use = false;
            continue;
            }
         }

      // now, build the straws (connections) for the FluxInfo.  
      // Each of these are a flux, as opposed to a FluxInfo, are are the
      // "straws" that convey water around the system

      /// NOTE THIS IS PROBABLY INCORRECT FOR TWO_WAY STRAWS (both sources and sinks)
      if ( pFluxInfo->m_sourceLocation != FL_UNDEFINED )
         fluxCount += AllocateFluxes( pFluxInfo, pFluxInfo->m_pSourceQuery, pFluxInfo->m_sourceLocation, pFluxInfo->m_sourceLayer );
      
      if ( pFluxInfo->m_sinkLocation != FL_UNDEFINED )
         fluxCount += AllocateFluxes( pFluxInfo, pFluxInfo->m_pSinkQuery, pFluxInfo->m_sinkLocation, pFluxInfo->m_sinkLayer  );
      }

   //CString msg;
   //msg.Format( "Flow: %i fluxes generated from %i Flux definitions", fluxCount, (int) m_fluxInfoArray.GetSize() );
   //Report::LogMsg( msg );
   return true;
   }


bool FlowModel::InitRunFluxes( void )
   {
   int fluxInfoCount = (int) m_fluxInfoArray.GetSize();

   for ( int i=0; i < fluxInfoCount; i++ )
      {
      FluxInfo *pFluxInfo = m_fluxInfoArray[ i ];

      pFluxInfo->m_totalFluxRate = 0;
      pFluxInfo->m_annualFlux = 0;
      pFluxInfo->m_cumFlux = 0;
      }

   return true;
   }
   



int FlowModel::AllocateFluxes( FluxInfo *pFluxInfo, Query *pQuery, FLUXLOCATION type, int layer )
   {
   int fluxCount = 0;

   // query built, need to determine where it applies
   switch( type )
      {
      case FL_REACH:
         {
         for ( int i=0; i < m_reachArray.GetSize(); i++ )
            {
            Reach *pReach = m_reachArray[ i ];

            int polyIndex = pReach->m_polyIndex;
            bool result = false;
            bool ok = pQuery->Run( polyIndex, result );

            if ( ok && result )
               {
               for ( int j=0; j < (int) pFluxInfo->m_stateVars.GetSize(); j++ )
                  {
                  pReach->AddFlux( pFluxInfo, pFluxInfo->m_stateVars[ j ] );
                  fluxCount++;
                  }
               }
            }
         }
         break;

      case FL_HRULAYER:
         {
         int hruLayerCount = GetHRULayerCount();

         for ( int i=0; i < m_catchmentArray.GetSize(); i++ )
            {
            Catchment *pCatchment = m_catchmentArray[ i ];

            for ( int h=0; h < pCatchment->GetHRUCountInCatchment(); h++ )
               {
               HRU *pHRU = pCatchment->GetHRUfromCatchment( h );

               // assume that if a majority of idu's satisfy the query, the flux applies to the HRU
               int countSoFar = 0;
               int polyCount = (int) pHRU->m_polyIndexArray.GetSize();

               for ( int k=0; k < polyCount; k++ )
                  {
                  bool result = false;
                  int polyIndex = pHRU->m_polyIndexArray[ k ];
                  bool ok = pQuery->Run( polyIndex, result );

                  if ( ok && result )
                     countSoFar++;
                  
                  if ( countSoFar >= polyCount )
                     {
                     for ( int j=0; j < (int) pFluxInfo->m_stateVars.GetSize(); j++ )
                        {
                        if ( layer < 0 )
                           {
                           for ( int l=0; l < hruLayerCount; l++ )
                              {
                              pHRU->GetLayer( l )->AddFlux( pFluxInfo, pFluxInfo->m_stateVars[ j ] );
                              fluxCount++;
                              }
                           }
                        else
                           {
                           pHRU->GetLayer( layer )->AddFlux( pFluxInfo, pFluxInfo->m_stateVars[ j ] );
                           fluxCount++;
                           }
                        }
                          
                     break;
                     }
                  }  // end of: for ( k < polyCount )
               }  // end of: for ( h < hruCount )
            }  // end of: for ( i < catchmentCount )
         }
         break;

      case FL_RESERVOIR:
      case FL_GROUNDWATER:
         break;

      case FL_IDU:
         {/*
         for ( int i=0; i < m_catchmentArray.GetSize(); i++ )
            {
            Catchment *pCatchment = m_catchmentArray[ i ];

            for ( int h=0; h < pCatchment->GetHRUCount(); h++ )
               {
               HRU *pHRU = pCatchment->GetHRU( h );
               int polyCount = (int) pHRU->m_polyIndexArray.GetSize();

               for ( int k=0; k < polyCount; k++ )
                  {
                  bool result = false;
                  int polyIndex = pHRU->m_polyIndexArray[ k ];
                  bool ok = pQuery->Run( polyIndex, result );



                  if ( ok && result )
                     countSoFar++;
                  
                  if ( countSoFar >= polyCount )
                     {
                     for ( int j=0; j < (int) pFluxInfo->m_stateVars.GetSize(); j++ )
                        {
                        if ( layer < 0 )
                           {
                           for ( int l=0; l < hruLayerCount; l++ )
                              {
                              pHRU->GetLayer( l )->AddFlux( pFluxInfo, pFluxInfo->m_stateVars[ j ] );
                              fluxCount++;
                              }
                           }
                        else
                           {
                           pHRU->GetLayer( layer )->AddFlux( pFluxInfo, pFluxInfo->m_stateVars[ j ] );
                           fluxCount++;
                           }
                        }
                          
                     break;
                     }
                  }  // end of: for ( k < polyCount )
               }  // end of: for ( h < hruCount )
            }  // end of: for ( i < catchmentCount )
            */
         }
         break;
         
      default:
         ASSERT( 0 );
      }
   
   return fluxCount;
   }


Query *FlowModel::BuildFluxQuery( LPCTSTR queryStr, FLUXLOCATION type )
   {
   Query *pQuery = NULL;

   switch( type )
      {
      case FL_REACH:
         ASSERT( m_pStreamQE != NULL );
         pQuery = m_pStreamQE->ParseQuery( queryStr, 0, "Flow Flux Query (Reach)" );
         break;

      case FL_HRULAYER:
         ASSERT( m_pCatchmentQE != NULL );
         pQuery = m_pCatchmentQE->ParseQuery( queryStr, 0, "Flow Flux Query (HRU)" );
         break;

      case FL_RESERVOIR:
      case FL_GROUNDWATER:
         break;
         
      case FL_IDU:
         ASSERT( m_pCatchmentQE != NULL );
         pQuery = m_pCatchmentQE->ParseQuery( queryStr, 0, "Flow Flux Query (IDU)" );
         break;

      default:
         ASSERT( 0 );
      }

   return pQuery;
   }





bool FlowModel::InitIntegrationBlocks( void )
   {
   // allocates arrays necessary for integration method.
   // For reaches, this allocates a state variable for each subnode "discharge" member for each reach,
   //   plus any "extra" state variables
   // For catchments, this allocates a state variable for each hru, for water volume,
   //   plus any "extra" state variables

   // catchments first
   int hruLayerCount = GetHRULayerCount();
   int catchmentCount = (int) m_catchmentArray.GetSize();
   int hruSvCount = 0;
   for ( int i=0; i < catchmentCount; i++ )
      {
      int count = m_catchmentArray[ i ]->GetHRUCountInCatchment();
      hruSvCount += ( count * hruLayerCount );
      }

   // note: total size of m_hruBlock is ( total number of HRUs ) * ( number of layers ) * (number of sv's/HRU)
   
   m_hruBlock.Init( m_integrator, hruSvCount * m_hruSvCount, m_initTimeStep);   // ALWAYS RK4???. INT_METHOD::IM_RKF  Note: Derivatives are set in GetCatchmentDerivatives.
   if (m_integrator==IM_RKF)
      {
      //m_hruBlock.m_rkvInitTimeStep = m_initTimeStep;
      m_hruBlock.m_rkvMinTimeStep=m_minTimeStep;
      m_hruBlock.m_rkvMaxTimeStep=m_maxTimeStep;
      m_hruBlock.m_rkvTolerance=m_errorTolerance;
      }
   hruSvCount = 0;
   for ( int i=0; i < catchmentCount; i++ )
      {
      int count = m_catchmentArray[ i ]->GetHRUCountInCatchment();

      for ( int j=0; j < count; j++ )
         {
         HRU *pHRU = m_catchmentArray[i]->GetHRUfromCatchment( j );

         for ( int k=0; k < hruLayerCount; k++ )
            {
            HRULayer *pHRULayer = m_catchmentArray[ i ]->GetHRUfromCatchment( j )->GetLayer( k );

            pHRULayer->m_svIndex = hruSvCount;
            m_hruBlock.SetStateVar(  &(pHRULayer->m_volumeWater), hruSvCount++ );

            for ( int l=0; l < m_hruSvCount-1; l++ )
               m_hruBlock.SetStateVar( &(pHRULayer->m_svArray[ l ]), hruSvCount++ );
            }          
         }
      }
      
   // next, reaches.  Each subnode gets a set of state variables
   if (  m_pReachRouting->GetMethod() == GM_RK4 
      || m_pReachRouting->GetMethod() == GM_EULER
      || m_pReachRouting->GetMethod() == GM_RKF )
      {
      int reachCount = (int) m_reachArray.GetSize();
      int reachSvCount = 0;
      for ( int i=0; i < reachCount; i++ )
        {
        Reach *pReach = m_reachArray[ i ];
        reachSvCount += pReach->GetSubnodeCount();
        }

      INT_METHOD intMethod = IM_RK4;
      if ( m_pReachRouting->GetMethod() == GM_EULER )
         intMethod = IM_EULER;
      else if ( m_pReachRouting->GetMethod() == GM_RKF )
         intMethod = IM_RKF;

      m_reachBlock.Init( intMethod, reachSvCount * m_reachSvCount, m_initTimeStep );

      reachSvCount = 0;
      for ( int i=0; i < reachCount; i++ )
        {
        Reach *pReach = m_reachArray[ i ];

        for ( int j=0; j < pReach->GetSubnodeCount(); j++ )
          {
          // assign state variables
          ReachSubnode *pNode = (ReachSubnode*) pReach->m_subnodeArray[j];
          pNode->m_svIndex = reachSvCount;
          m_reachBlock.SetStateVar( &pNode->m_waterParcel.m_volume_m3, reachSvCount++ );

          for ( int k=0; k < m_reachSvCount-1; k++ )
            m_reachBlock.SetStateVar( &(pNode->m_svArray[ k ]), reachSvCount++ );
          }
        }
      }
   
   // next, reservoirs
   int reservoirCount = (int) m_reservoirArray.GetSize();
   m_reservoirBlock.Init(IM_RK4, reservoirCount * m_reservoirSvCount, m_initTimeStep);

   int resSvCount = 0;
   for ( int i=0; i < reservoirCount; i++ )
      {
      Reservoir *pRes = m_reservoirArray.GetAt(i);
      pRes->m_svIndex = resSvCount;
      m_reservoirBlock.SetStateVar(  &(pRes->m_volume), resSvCount++ );
      pRes->m_resWP = WaterParcel(pRes->m_volume, pRes->m_resWP.WaterTemperature());

      for ( int k=0; k < m_reservoirSvCount-1; k++ )
         m_reservoirBlock.SetStateVar( &(pRes->m_svArray[ k ]), resSvCount++ );
      }

   // next, totals (for error-check mass balance).  total includes:
   // 1) total input
   // 2) total output
   // 3) one entry for each flux info to accumulate annual fluxes
   int fluxInfoCount = GetFluxInfoCount();

   m_totalBlock.Init( IM_RK4, 2 + fluxInfoCount, m_timeStep );
   m_totalBlock.SetStateVar( &m_totalWaterInput, 0 );
   m_totalBlock.SetStateVar( &m_totalWaterOutput, 1 );

   for ( int i=0; i < fluxInfoCount; i++ )
      {
      FluxInfo *pFluxInfo = GetFluxInfo( i );
      m_totalBlock.SetStateVar( &pFluxInfo->m_annualFlux, 2+i );
      }
   
   return true;
   }


/*
void FlowModel::AllocateOutputVars()
   {
   m_pInstreamData = new FDataObj( 3 , 0 );   
   m_pInstreamData->SetLabel(0,_T("Time"));
   m_pInstreamData->SetLabel(1,_T("Mod Q (ft3/s)"));
   if (m_pMeasuredDischargeData != NULL)
      m_pInstreamData->SetLabel(2, _T("Meas Q (ft3/s)"));
   m_instreamDataObjArray.Add( m_pInstreamData );

   m_pTempData = new FDataObj( 2 , 0 );   
   m_pTempData->SetLabel(0,_T("Time"));
   m_pTempData->SetLabel(1,_T("Temp (C)"));
   m_tempDataObjArray.Add( m_pTempData );

   m_pPrecipData = new FDataObj( 2 , 0 );   
   m_pPrecipData->SetLabel(0,_T("Time"));
   m_pPrecipData->SetLabel(1,_T("Precip (mm/d)"));
   m_precipDataObjArray.Add( m_pPrecipData );
   
   m_pSoilMoistureData = new FDataObj( 2 , 0 );   
   m_pSoilMoistureData->SetLabel(0,_T("Time"));
   m_pSoilMoistureData->SetLabel(1,_T("SoilMoisture"));
   m_soilMoistureDataObjArray.Add( m_pSoilMoistureData );

   m_pGWDepthData = new FDataObj( 2 , 0 );   
   m_pGWDepthData->SetLabel(0,_T("Time"));
   m_pGWDepthData->SetLabel(1,_T("SoilMoisture"));
   m_gWDepthDataObjArray.Add( m_pGWDepthData );  
   }
*/
/*
void FlowModel::UpdateCatchmentFluxes( float time, float timeStep, FlowContext *pFlowContext )
   {
   FlowModel *pModel = pFlowContext->pFlowModel;
   
   // compute derivative values for catchment
   pFlowContext->timeStep = timeStep;
   pFlowContext->time = time;

   int catchmentCount = (int) pModel->m_catchmentArray.GetSize();
   int svIndex = 0;
   int hruLayerCount = pModel->GetHRULayerCount();

   // iterate through catchments/hrus/hrulayers, calling fluxes as needed
   for ( int i=0; i < catchmentCount; i++ )
      {
      Catchment *pCatchment = pModel->m_catchmentArray[ i ];
      int hruCount = pCatchment->GetHRUCount();
      for ( int h=0; h < hruCount; h++ )
         {
         HRU *pHRU = pCatchment->GetHRU( h );
         pFlowContext->pHRU = pHRU;

         for ( int l=0; l < hruLayerCount; l++ )
            {
            HRULayer *pHRULayer = pHRU->GetLayer( l );

            pFlowContext->pHRULayer = pHRULayer;

            // update all fluxes for this hruLayer
            for ( int k=0; k < pHRULayer->GetFluxCount(); k++ )
               {
               Flux *pFlux = pHRULayer->GetFlux( k );
               pFlowContext->pFlux = pFlux;

               float flux = pFlux->Evaluate( pFlowContext );
               }

            svIndex += m_hruSvCount;
            }
         }
      }
   }

 */


void FlowModel::GetCatchmentDerivatives(double time, double timeStep, int svCount, double* derivatives /*out*/, void* extra)
   {
   gFlowModel->SetGlobalExtraSVRxn();

   GlobalMethodManager::Step(&gFlowModel->m_flowContext);

   gFlowModel->m_flowContext.svCount = gpModel->m_hruSvCount - 1;

   int catchmentCount = (int)gFlowModel->m_catchmentArray.GetSize();
   int hruLayerCount = gFlowModel->GetHRULayerCount();
      
   // iterate through catchments/hrus/hrulayers, calling fluxes as needed
   clock_t start = clock();

   int hruCount = (int)gFlowModel->m_hruArray.GetSize();
   for ( int h=0; h < hruCount; h++ )
      {
      HRU* pHRU = gFlowModel->m_hruArray[h];

      for ( int l=0; l < hruLayerCount; l++ )
         {
         HRULayer *pHRULayer = pHRU->GetLayer( l );
         int svIndex = pHRULayer->m_svIndex;
         derivatives[svIndex] = pHRULayer->GetFluxValue();

         // initialize additional state variables=0
         for ( int k=1; k < gpModel->m_hruSvCount; k++ )
            derivatives[svIndex+k] = 0;

         // transport fluxes for the extra state variables
         for ( int k=1; k < gpModel->m_hruSvCount; k++ )
            derivatives[svIndex+k] = pHRULayer->GetExtraSvFluxValue(k-1);

         // add all fluxes for this hruLayer - this includes extra state variables!
         for ( int k=0; k < pHRULayer->GetFluxCount(); k++ )
            {
            Flux *pFlux = pHRULayer->GetFlux( k );
            // get the flux.  Note that this is a rate applied over a timestep
            float flux = pFlux->Evaluate(&gFlowModel->m_flowContext); 
            int sv = pFlux->m_pStateVar->m_index;
            if ( pFlux->IsSource() )
               derivatives[ svIndex + sv ] += flux;
            else
               derivatives[ svIndex + sv ] -= flux;
            }

         // Negative mass check 
         if (isnan(pHRULayer->m_volumeWater)) 
            {
            pHRULayer->m_nanOccurred = true;
            pHRULayer->m_volumeWater = pHRULayer->m_wc = pHRULayer->m_wDepth = 0;
            }
         else if (pHRULayer->m_volumeWater < 0)
            {
            float nominal_minimum_mm;
            if (pHRULayer->m_layer == BOX_SNOW || pHRULayer->m_layer == BOX_MELT) nominal_minimum_mm = 0.f;
            else nominal_minimum_mm = (float)(NOMINAL_MINIMUM_SOIL_WATER_CONTENT * pHRULayer->m_soilThickness_m);
            pHRULayer->m_addedVolume_m3 += (float)((pHRU->m_HRUeffArea_m2 * pHRULayer->m_HRUareaFraction * nominal_minimum_mm / MM_PER_M) - pHRULayer->m_volumeWater);
            pHRULayer->m_wDepth = nominal_minimum_mm;
            pHRULayer->m_wc = nominal_minimum_mm / pHRULayer->m_soilThickness_m;
            pHRULayer->m_volumeWater = (float)(pHRU->m_HRUeffArea_m2 * pHRULayer->m_HRUareaFraction * pHRULayer->m_soilThickness_m * nominal_minimum_mm / MM_PER_M);
            }
         } // end of loop thru HRULayers of this HRU
      } // end of loop thru HRUs
   gFlowModel->ResetHRUfluxes();

   if (gFlowModel->m_pReachRouting->GetMethod() == GM_RKF)
      {
      gFlowModel->m_flowContext.timing = GMT_REACH;
      GlobalMethodManager::SetTimeStep(gFlowModel->m_flowContext.timeStep);
      GlobalMethodManager::Step(&gFlowModel->m_flowContext);
      }

   clock_t finish = clock();
   double duration = (float)(finish - start) / CLOCKS_PER_SEC;   
   gpModel->m_hruFluxFnRunTime += (float) duration;   
   } // end of GetCatchmentDerivatives()


bool FlowModel::CheckSurfaceH2O(HRU * pHRU, double boxSurfaceH2Oadjustment_m3)
// Confirm that HruBOXSURF_M3 is consistent with HruH2OMELT_M3 and HruH2OSTNDGM3, with IDU attributes H2O_MELT, WETNESS, and FLOODDEPTH,
// and with m_standingH2Oflag, m_snowpackFlag, and m_waterVolume in the snowpack and surface water HRU compartments.
{
   double hru_snow_box_mm = pHRU->Att(HruSNOW_BOX);
   double hru_snow_box_m3 = (hru_snow_box_mm / 1000.) * pHRU->m_HRUtotArea_m2;
   double hru_box_surf_m3 = pHRU->Att(HruBOXSURF_M3);
   double hru_h2o_melt_m3 = pHRU->Att(HruH2OMELT_M3);
   double hru_h2o_stndg_m3 = pHRU->Att(HruH2OSTNDGM3);

   HRULayer* pBOX_SNOW = pHRU->GetLayer(BOX_SNOW);
   double box_snow_m3 = pBOX_SNOW->m_volumeWater;
   double flux_box_snow_m3 = pBOX_SNOW->m_globalHandlerFluxValue;

   HRULayer* pBOX_SURFACE_H2O = pHRU->GetLayer(BOX_SURFACE_H2O);
   double box_surface_h2o_m3 = pBOX_SURFACE_H2O->m_volumeWater;
   double flux_box_surface_h2o_m3 = pBOX_SURFACE_H2O->m_globalHandlerFluxValue; // ???

   HRULayer* pBOX_NAT_SOIL = pHRU->GetLayer(BOX_NAT_SOIL);
   double box_nat_soil_m3 = pBOX_NAT_SOIL->m_volumeWater;
   double flux_box_nat_soil_m3 = pBOX_NAT_SOIL->m_globalHandlerFluxValue; // ???

   double idu_melt_h2o_accum_m3 = 0.;
   double idu_standing_h2o_accum_m3 = 0.;
   double idu_snow_swe_accum_m3 = 0.;
   double idu_wetl2q_accum_cms = 0.; // ???
   double idu_ET_DAY_accum_m3 = 0;
   int num_idus = (int)pHRU->m_polyIndexArray.GetSize();
   for (int i = 0; i < num_idus; i++)
   {
      int idu_poly_ndx = pHRU->m_polyIndexArray[i];
      int lulc_a = AttInt(idu_poly_ndx, LULC_A);
      bool is_wetland = lulc_a == LULCA_WETLAND;
      if (is_wetland)
      {
         double idu_wetness_mm = Att(idu_poly_ndx, WETNESS);
         double idu_flooddepth_mm = Att(idu_poly_ndx, FLOODDEPTH);
         double idu_standing_h2o_mm = idu_flooddepth_mm + max(idu_wetness_mm, 0);
         float idu_area_m2 = AttFloat(idu_poly_ndx, AREA);
         double idu_standing_h2o_m3 = (idu_standing_h2o_mm / 1000) * idu_area_m2;
         idu_standing_h2o_accum_m3 += idu_standing_h2o_m3;

         double idu_ET_DAY_mm = AttFloat(idu_poly_ndx, ET_DAY);
         idu_ET_DAY_accum_m3 += (idu_ET_DAY_mm / 1000) * idu_area_m2;

         double idu_wetl2q_cms = gFlowModel->Att(idu_poly_ndx, WETL2Q); // ???
         idu_wetl2q_accum_cms += idu_wetl2q_cms; // ???
      }
      else
      {
         double idu_snow_swe_mm = gFlowModel->Att(idu_poly_ndx, SNOW_SWE);
         double idu_melt_h2o_mm = gFlowModel->Att(idu_poly_ndx, H2O_MELT);
         ASSERT(idu_melt_h2o_mm >= 0.);
         if ((idu_melt_h2o_mm > 0.) || (idu_snow_swe_mm > 0.))
         {
            float idu_area_m2 = gFlowModel->AttFloat(idu_poly_ndx, AREA);
            double idu_melt_h2o_m3 = (idu_melt_h2o_mm / 1000.) * idu_area_m2;
            idu_melt_h2o_accum_m3 += idu_melt_h2o_m3;
            double idu_snow_swe_m3 = (idu_snow_swe_mm / 1000.) * idu_area_m2;
            idu_snow_swe_accum_m3 += idu_snow_swe_m3;
         }
      }
   } // end of loop thru IDUs in this HRU

   double box_snow_flux_m3 = pBOX_SNOW->m_globalHandlerFluxValue;
   double box_surface_h2o_flux_m3 = pBOX_SURFACE_H2O->m_globalHandlerFluxValue;
   double idu_wetl2q_accum_m3 = idu_wetl2q_accum_cms * SEC_PER_DAY; // ???

   double adjusted_box_surface_h2o_m3 = box_surface_h2o_m3 + boxSurfaceH2Oadjustment_m3;

   bool is_close_enough = close_enough(hru_box_surf_m3, hru_h2o_melt_m3 + hru_h2o_stndg_m3, 1e-3, 1);
   is_close_enough = is_close_enough && close_enough(hru_box_surf_m3, idu_melt_h2o_accum_m3 + idu_standing_h2o_accum_m3, 1e-3, 1.);
   is_close_enough = is_close_enough && close_enough(hru_snow_box_m3, box_snow_m3, 1e-4, 1);
// ???  is_close_enough = is_close_enough && close_enough(hru_box_surf_m3, adjusted_box_surface_h2o_m3, 1e-4, 1);
   is_close_enough = is_close_enough && close_enough(box_snow_m3, idu_snow_swe_accum_m3 - idu_melt_h2o_accum_m3, 1e-4, 1);
   is_close_enough = is_close_enough && ((pHRU->m_snowpackFlag == (box_snow_m3 > 0)) || close_enough(box_snow_m3, 0, 1e-4, 1));
   is_close_enough = is_close_enough && (pHRU->m_standingH2Oflag == (hru_h2o_stndg_m3 > 0));

   if (!is_close_enough)
   {
      CString msg; 
      msg.Format("CheckSurfaceH2O() HRU id = %d. boxSurfaceH2Oadjustment = %f. is_close_enough is false", 
         pHRU->m_id, boxSurfaceH2Oadjustment_m3); 
      Report::LogWarning(msg);
      msg.Format("hru_box_surf_m3 = %f", hru_box_surf_m3); Report::LogMsg(msg);
      msg.Format("hru_h2o_melt_m3 = %f", hru_h2o_melt_m3); Report::LogMsg(msg);
      msg.Format("hru_h2o_stndg_m3 = %f", hru_h2o_stndg_m3); Report::LogMsg(msg);
      msg.Format("idu_melt_h2o_accum_m3 = %f", idu_melt_h2o_accum_m3); Report::LogMsg(msg);
      msg.Format("idu_standing_h2o_accum_m3 = %f", idu_standing_h2o_accum_m3); Report::LogMsg(msg);
      msg.Format("hru_snow_box_m3 = %f", hru_snow_box_m3); Report::LogMsg(msg);
      msg.Format("box_snow_m3 = %f", box_snow_m3); Report::LogMsg(msg);
      msg.Format("box_surface_h2o_m3 = %f", box_surface_h2o_m3); Report::LogMsg(msg);
      msg.Format("idu_snow_swe_accum_m3 = %f", idu_snow_swe_accum_m3); Report::LogMsg(msg);
      msg.Format("idu_melt_h2o_accum_m3 = %f", idu_melt_h2o_accum_m3); Report::LogMsg(msg);
      msg.Format("m_snowpackFlag = %s", pHRU->m_snowpackFlag ? "true" : "false"); Report::LogMsg(msg);
      msg.Format("m_standingH2Oflag = %s", pHRU->m_standingH2Oflag ? "true" : "false"); Report::LogMsg(msg);
   }

   return(is_close_enough);
} // end of CheckSurfaceH2O()


void FlowModel::GetReservoirDerivatives( double time, double timeStep, int svCount, double *derivatives /*out*/, void *extra )
   {
   FlowContext *pFlowContext = (FlowContext*) extra;
   FlowModel *pModel = pFlowContext->pFlowModel;
   
   // compute derivative values for catchment
   pFlowContext->timeStep = (float) timeStep;
   pFlowContext->time = (float) time;

   int reservoirCount = (int) pModel->m_reservoirArray.GetSize();

   // iterate through reservoirs, calling fluxes as needed
   //#pragma omp parallel for
   for ( int i=0; i < reservoirCount; i++ )
      {
      FlowContext flowContext( *pFlowContext );    // make a copy for openMP

      Reservoir *pRes = pModel->m_reservoirArray[ i ];

      int svIndex = pRes->m_svIndex;

      // basic inflow/outflow (assumes these were calculated prior to this point)
      derivatives[svIndex] = pRes->m_inflow - pRes->m_outflow;//m3/d;

      // include additional state variables
      for ( int k=1; k < pModel->m_reservoirSvCount; k++ )
         derivatives[svIndex+k] = 0;

      // add all fluxes for this reservoir...not sure we will define external fluxes for reservoirs, but perhaps...
      for ( int k=0; k < pRes->GetFluxCount(); k++ )
         {
         Flux *pFlux = pRes->GetFlux( k );
         pFlowContext->pFlux = pFlux;

         float flux = pFlux->Evaluate( pFlowContext );

         // figure out which state var this flux is associated with
         int sv = pFlux->m_pStateVar->m_index;

         // source or sink?  If sink, flip sign  
         // CHECK THIS???? - probably not correct for two-way straws 
         if ( pFlux->IsSource() )
            {
            derivatives[ svIndex + sv ] += flux;
            pModel->m_totalWaterInputRate += flux;
            }
         else
            {
            derivatives[ svIndex + sv ] -= flux;
            pModel->m_totalWaterOutputRate += flux;
            }
         }
      }
   }



void FlowModel::GetTotalDerivatives( double time, double timeStep, int svCount, double *derivatives /*out*/, void *extra )
   {
   FlowContext *pFlowContext = (FlowContext*) extra;

   // NOTE: NEED TO INITIALIZE TIME IN FlowContext before calling here...
   // NOTE:  Should probably do a breadth-first search of the tree(s) rather than what is here.  This could be accomplished by
   //  sorting the m_reachArray in InitReaches()
   FlowModel *pModel = pFlowContext->pFlowModel;

   int fluxInfoCount = pModel->GetFluxInfoCount();

   ASSERT( svCount == 2 + fluxInfoCount );
   derivatives[ 0 ] = pModel->m_totalWaterInputRate;
   derivatives[ 1 ] = pModel->m_totalWaterOutputRate;

   #pragma omp parallel for
   for ( int i=0; i < fluxInfoCount; i++ )
      {
      FluxInfo *pFluxInfo = pModel->GetFluxInfo( i );
      derivatives[ 2+i ]  = pFluxInfo->m_totalFluxRate;
      }
   }


ParamTable *FlowModel::GetTable( LPCTSTR name )
   {
   for ( int i=0; i < (int) m_tableArray.GetSize(); i++ )
      if ( lstrcmpi( m_tableArray[ i ]->m_pTable->GetName(), name ) == 0 )
         return m_tableArray[ i ];

   return NULL;
   }


bool FlowModel::GetTableValue( LPCTSTR name, int col, int row, float &value )
   {
   ParamTable *pTable = GetTable( name );

   if ( pTable == NULL )
      return false;

   return pTable->m_pTable->Get( col, row, value );
   }



void FlowModel::AddReservoirLayer( void )
   {
   Map *pMap = m_pCatchmentLayer->GetMapPtr();

   MapLayer *pResLayer = pMap->GetLayer( "Reservoirs" );

   if ( pResLayer != NULL )      // already exists?
      {
      return;
      }

   // doesn't already exist, so map existing reservoirs
   int resCount = (int) m_reservoirArray.GetSize();
   int usedResCount = 0;
   for ( int i=0; i < resCount; i++ )
      {
      if ( m_reservoirArray[ i ]->m_pReach != NULL )
         usedResCount++;
      }
   
   m_pResLayer = pMap->AddLayer( LT_POINT );

   DataObj *pData = m_pResLayer->CreateDataTable( usedResCount, 1, DOT_FLOAT );
   
   usedResCount = 0;
   for ( int i=0; i < resCount; i++ )
      {
      Reservoir *pRes = m_reservoirArray[ i ];

      if ( pRes->m_pReach != NULL )
         {
         int polyIndex = pRes->m_pReach->m_polyIndex;

         Poly *pPt = m_pStreamLayer->GetPolygon( polyIndex );
         int vertexCount = (int) pPt->m_vertexArray.GetSize();
         Vertex &v = pPt->m_vertexArray[ vertexCount-1 ];
      
         m_pResLayer->AddPoint( v.x, v.y );
         pData->Set( 0, usedResCount++, pRes->m_volume );
         }
      }

   // additional map setup
   m_pResLayer->m_name = "Reservoirs";
   m_pResLayer->SetFieldLabel( 0, "ResVol" );
   //m_pResLayer->AddBin( 0, RGB(0,0,127), "Reservoir Volume", TYPE_FLOAT, 0, 1000000 );
   m_pResLayer->SetActiveField( 0 );
   m_pResLayer->SetOutlineColor( RGB( 0, 0, 127 ) );
   m_pResLayer->SetSolidFill( RGB( 0,0, 255 ) );
//   m_pResLayer->UseVarWidth( 0, 100 );
   //pMap->RedrawWindow();
   }




////////////////////////////////////////////////////////////////////////////////////////////////
// F L O W    P R O C E S S 
////////////////////////////////////////////////////////////////////////////////////////////////

FlowProcess::FlowProcess()
: EnvAutoProcess()
, m_maxProcessors( -1 )
, m_processorsUsed( 0 )
   { }
  

FlowProcess::~FlowProcess(void)
   {
   m_flowModelArray.RemoveAll();
   }


BOOL FlowProcess::Setup( EnvContext *pEnvContext, HWND )
   {
   FlowModel *pModel = GetFlowModelFromID( pEnvContext->id );
   ASSERT( pModel != NULL );

   FlowDlg dlg( pModel, "Flow Setup" );

   dlg.DoModal();

   return TRUE;
   }



BOOL FlowProcess::Init( EnvContext *pEnvContext, LPCTSTR initStr  )
   {
   EnvExtension::Init( pEnvContext, initStr );

   m_pIDUlayer = (MapLayer*)pEnvContext->pMapLayer;
   m_pReachLayer = (MapLayer*)pEnvContext->pReachLayer;
   m_pHRUlayer = (MapLayer*)pEnvContext->pHRUlayer;
   m_pLinkLayer = (MapLayer*)pEnvContext->pLinkLayer;
   m_pNodeLayer = (MapLayer*)pEnvContext->pNodeLayer;
   m_pSubcatchLayer = (MapLayer*)pEnvContext->pSubcatchmentLayer;

   bool ok = LoadXml( initStr, pEnvContext );
   
   if ( ! ok )
      return FALSE;

   m_processorsUsed = omp_get_num_procs();

   m_processorsUsed = ( 3 * m_processorsUsed / 4) + 1;   // default uses 2/3 of the cores
   if ( m_maxProcessors > 0 && m_processorsUsed >= m_maxProcessors )
      m_processorsUsed = m_maxProcessors;

   omp_set_num_threads(m_processorsUsed); 

   FlowModel *pModel = m_flowModelArray.GetAt( m_flowModelArray.GetSize()-1 );
   ASSERT( pModel != NULL );

   ok = pModel->Init( pEnvContext );

   return ok ? TRUE : FALSE;
   }


BOOL FlowProcess::InitRun( EnvContext *pEnvContext  )
   {
   m_flowModelArray.GetAt( m_flowModelArray.GetSize()-1 );

   FlowModel *pModel = GetFlowModelFromID( pEnvContext->id );
   ASSERT( pModel != NULL );

   bool ok = pModel->InitRun( pEnvContext );
   
   return ok ? TRUE : FALSE;
   }


BOOL FlowProcess::Run( EnvContext *pEnvContext  )
   {
   m_flowModelArray.GetAt( m_flowModelArray.GetSize()-1 );

   FlowModel *pModel = GetFlowModelFromID( pEnvContext->id );
   ASSERT( pModel != NULL );

   bool ok = pModel->Run( pEnvContext );
   
   return ok ? TRUE : FALSE;
   }

BOOL FlowProcess::EndRun( EnvContext *pEnvContext  )
   {
   m_flowModelArray.GetAt( m_flowModelArray.GetSize()-1 );

   FlowModel *pModel = GetFlowModelFromID( pEnvContext->id );
   ASSERT( pModel != NULL );

   bool ok = pModel->EndRun( pEnvContext );
   
   return ok ? TRUE : FALSE;
   }


FlowModel *FlowProcess::GetFlowModelFromID( int id )
   {
   for ( int i=0; i < (int) m_flowModelArray.GetSize(); i++ )
      {
      FlowModel *pModel = m_flowModelArray.GetAt( i );
      
      if ( pModel->m_id == id )
         return pModel;
      }
   
   return NULL;
   }


// this method reads an xml input file
bool FlowProcess::LoadXml( LPCTSTR filename, EnvContext *pEnvContext)
   {
   MapLayer * pIDULayer = (MapLayer *)pEnvContext->pMapLayer; ASSERT(pIDULayer != NULL);
   MapLayer * pReachLayer = (MapLayer *)pEnvContext->pReachLayer; ASSERT(pReachLayer != NULL);
   MapLayer * pHRUlayer = (MapLayer *)pEnvContext->pHRUlayer; ASSERT(pHRUlayer != NULL);
   MapLayer * pLinkLayer = (MapLayer *)pEnvContext->pLinkLayer;
   MapLayer * pNodeLayer = (MapLayer *)pEnvContext->pNodeLayer;
   MapLayer * pSubcatchLayer = (MapLayer *)pEnvContext->pSubcatchmentLayer; 

   // start parsing input file
   TiXmlDocument doc;
   bool ok = doc.LoadFile( filename );

   bool loadSuccess = true;

   if ( ! ok )
      {
      CString msg;
      msg.Format("Flow: Error reading input file %s:  %s", filename, doc.ErrorDesc() );
      Report::ErrorMsg( msg );
      return false;
      }
   
   // start interating through the nodes
   TiXmlElement *pXmlRoot = doc.RootElement();  // <flow_model>

   FlowModel *pModel = new FlowModel;
   pModel->m_filename = filename;
   CString grid;
   CString integrator;
   float ts = 1.0f;
   grid.Format("none");

   int iduDailyOutput    = 0;
   int iduAnnualOutput   = 0;
   int reachDailyOutput  = 0;
   int reachAnnualOutput = 0;

   pModel->m_initConditionsFileName = "";
   XML_ATTR rootAttrs[] = {
      // attr                     type          address                                isReq  checkCol
      { "name",                    TYPE_CSTRING,  &(pModel->m_name),                   false,   0 },
      { "time_step",               TYPE_FLOAT,    &ts,                                 false,   0 },
      { "build_catchments_method", TYPE_INT,      &(pModel->m_buildCatchmentsMethod),  false,   0 },
      { "catchment_join_col",      TYPE_CSTRING,  &(pModel->m_catchmentJoinCol),       true,    0 },
      { "stream_join_col",         TYPE_CSTRING,  &(pModel->m_streamJoinCol),          true,    0 },
      { "simplify_query",          TYPE_CSTRING,  &(pModel->m_streamQuery),            false,   0 },
      { "simplify_min_area",       TYPE_FLOAT,    &(pModel->m_minCatchmentArea),       false,   0 },
      { "simplify_max_area",       TYPE_FLOAT,    &(pModel->m_maxCatchmentArea),       false,   0 },
      { "path",                    TYPE_CSTRING,  &(pModel->m_path),                   false,   0 },  
      { "check_mass_balance",      TYPE_INT,      &(pModel->m_checkMassBalance),       false,   0 },  
      { "max_processors",          TYPE_INT,      &m_maxProcessors,                    false,   0 },  
      { "grid",                    TYPE_CSTRING,  &grid,                               false,   0 },
      { "integrator",              TYPE_CSTRING,  &integrator,                         false,   0 },
      { "min_timestep",            TYPE_FLOAT,    &(pModel->m_minTimeStep),            false,   0 },
      { "max_timestep",            TYPE_FLOAT,    &(pModel->m_maxTimeStep),            false,   0 },
      { "error_tolerance",         TYPE_FLOAT,    &(pModel->m_errorTolerance),         false,   0 },
      { "init_timestep",           TYPE_FLOAT,    &(pModel->m_initTimeStep),           false,   0 },
      { "update_display",          TYPE_INT,      &(pModel->m_mapUpdate),              false,   0 },
      { "initial_conditions",      TYPE_CSTRING,  &(pModel->m_initConditionsFileName), false,   0 },
      { "daily_idu_output",        TYPE_INT,      &iduDailyOutput,                     false,   0 },
      { "annual_idu_output",       TYPE_INT,      &iduAnnualOutput,                    false,   0 },
      { "daily_reach_output",      TYPE_INT,      &reachDailyOutput,                   false,   0 },
      { "annual_reach_output",     TYPE_INT,      &reachAnnualOutput,                  false,   0 },
      { NULL,                      TYPE_NULL,     NULL,                                false,   0 } };

   ok = TiXmlGetAttributes( pXmlRoot, rootAttrs, filename, NULL );
   pModel->m_path = PathManager::GetPath(1);
   if ( ! ok )
      {
      CString msg; 
      msg.Format( _T("Flow: Misformed root element reading <flow> attributes in input file %s"), filename );
      Report::ErrorMsg( msg );
      delete pModel;
      return false;
      }
   pModel->m_timeStep = ts;
   if (grid.CompareNoCase("none"))
      {
      pModel->m_pGrid= pIDULayer->GetMapPtr()->GetLayer(grid);
     pModel->m_grid=grid;
      if (!pModel->m_pGrid)
         {
         CString msg; 
         msg.Format( _T("Flow: Can't find %s in the Map. Check the envx file"), (LPCTSTR) grid );
         Report::ErrorMsg( msg );
         return false;
         }
      }
   if ( lstrcmpi( integrator, "rk4" ) == 0 )
      pModel->m_integrator = IM_RK4;
   else if ( lstrcmpi( integrator, "rkf" ) == 0 )
      pModel->m_integrator = IM_RKF;
   else
      pModel->m_integrator = IM_EULER;

   pModel->m_detailedOutputFlags = 0;
   if ( iduDailyOutput    ) pModel->m_detailedOutputFlags += 1;
   if ( iduAnnualOutput   ) pModel->m_detailedOutputFlags += 2;
   if ( reachDailyOutput  ) pModel->m_detailedOutputFlags += 4;
   if ( reachAnnualOutput ) pModel->m_detailedOutputFlags += 8;

   //------------------- <scenario macros> ------------------
   TiXmlElement *pXmlMacros = pXmlRoot->FirstChildElement( "macros" ); 
   if ( pXmlMacros )
      {
      TiXmlElement *pXmlMacro = pXmlMacros->FirstChildElement( "macro" ); 

      while ( pXmlMacro != NULL )
         {
         ScenarioMacro *pMacro = new ScenarioMacro;
         XML_ATTR macroAttrs[] = {
            // attr                 type          address                     isReq  checkCol
            { "name",              TYPE_CSTRING,   &(pMacro->m_name),          true,   0 },
            { "default",           TYPE_CSTRING,   &(pMacro->m_defaultValue),  true,   0 },
            { NULL,                TYPE_NULL,      NULL,                       false,  0 } };
   
         ok = TiXmlGetAttributes( pXmlMacro, macroAttrs, filename, NULL );
         if ( ! ok )
            {
            CString msg; 
            msg.Format( _T("Flow: Misformed root element reading <macro> attributes in input file %s"), filename );
            Report::ErrorMsg( msg );
            delete pMacro;
            }
         else
            {
            pModel->m_macroArray.Add( pMacro );

            // parse any details
            TiXmlElement *pXmlScn = pXmlMacro->FirstChildElement( "scenario" ); 
            while( pXmlScn != NULL )
               {
               SCENARIO_MACRO *pScn = new SCENARIO_MACRO;
               CString scnName;
               XML_ATTR scnAttrs[] = {
                  // attr           type          address                  isReq  checkCol
                  { "name",       TYPE_CSTRING,   &(pScn->envScenarioName), true,   0 },
                  { "value",      TYPE_CSTRING,   &(pScn->value),           true,   0 },
                  { NULL,         TYPE_NULL,      NULL,                     false,  0 } };
   
               ok = TiXmlGetAttributes( pXmlScn, scnAttrs, filename, NULL );
               if ( ! ok )
                  {
                  CString msg; 
                  msg.Format( _T("Flow: Misformed element reading <macro><scenario> attributes in input file %s"), filename );
                  Report::ErrorMsg( msg );
                  delete pScn;
                  }
               else
                  {
                  Scenario *pScenario = NULL;  //::EnvGetScenarioFromName( scnName, &(pScn->envScenarioIndex) );   // this is the envision scenario index

                  //if ( pScenario != NULL )
                     pMacro->m_macroArray.Add( pScn );
                  //else
                  //   delete pScn;
                  }
               pXmlScn = pXmlScn->NextSiblingElement( "scenario" );
               }  // end of: while( pXmlScn != NULL )
            }  // end of: else (Reading <macro> xml succesful

         pXmlMacro = pXmlMacro->NextSiblingElement( "macro" ); 
         }  // end of: while ( pXmlMacro != NULL )
      }

   //------------------- <catchment> ------------------
   CString colNames;
   CString layerDepths;
   CString layerWC;
   TiXmlElement *pXmlCatchment = pXmlRoot->FirstChildElement( "catchments" ); 
   XML_ATTR catchmentAttrs[] = {
      // attr                 type          address                           isReq  checkCol
      { "layer",              TYPE_CSTRING,  &(pModel->m_catchmentLayer),     false,   0 },
      { "query",              TYPE_CSTRING,  &(pModel->m_catchmentQuery),     false,   0 },
      { "elev_col",           TYPE_CSTRING,  &(pModel->m_elevCol),            false,   0 },
      //{ "join_col",           TYPE_CSTRING,  &(pModel->m_catchmentJoinCol),   true,    0 },  // typically "COMID"
      { "catchment_agg_cols", TYPE_CSTRING,  &(pModel->m_catchmentAggCols),   true,   0 },
      { "hru_agg_cols",       TYPE_CSTRING,  &(pModel->m_hruAggCols),         true,    0 },
      { "hruID_col",          TYPE_CSTRING,  &(pModel->m_hruIDCol),           false,   0 },
      { "catchmentID_col",    TYPE_CSTRING,  &(pModel->m_catchIDCol),         false,   0 },
      { "soil_layers",        TYPE_INT,      &(pModel->m_soilLayerCount),     false,   0 },
      { "snow_layers",        TYPE_INT,      &(pModel->m_snowLayerCount),     false,   0 },
      { "veg_layers",         TYPE_INT,      &(pModel->m_vegLayerCount),      false,   0 },
      { "init_water_content", TYPE_CSTRING,  &(layerWC),                      false,   0 },
      { "layer_names",        TYPE_CSTRING,  &(colNames),                     false,   0 },
      { "layer_depths",       TYPE_CSTRING,  &(layerDepths),                  false,   0 },
      { NULL,                 TYPE_NULL,     NULL,                            false,   0 } };
            
   ok = TiXmlGetAttributes( pXmlCatchment, catchmentAttrs, filename, NULL );
   if ( ! ok )
      {
      CString msg; 
      msg.Format( _T("Flow: Misformed root element reading <catchments> attributes in input file %s"), filename );
      Report::ErrorMsg( msg );
      delete pModel;
      return false;
      }

   pModel->ParseHRULayerDetails(colNames,0);
   pModel->ParseHRULayerDetails(layerDepths,1);
   pModel->ParseHRULayerDetails(layerWC,2);
   pModel->m_pCatchmentLayer = pIDULayer->GetMapPtr()->GetLayer( pModel->m_catchmentLayer );
   // ERROR CHECK

   if ( pModel->m_hruIDCol.IsEmpty() )
      pModel->m_hruIDCol = _T( "HRU_ID" );

   if ( pModel->m_catchIDCol.IsEmpty() )
      pModel->m_catchIDCol = _T( "CATCH_ID" );
      
   if ( pModel->m_elevCol.IsEmpty() )
      pModel->m_elevCol = _T( "ELEV_MEAN" );

   pModel->m_colCatchmentArea = pModel->m_pCatchmentLayer->GetFieldCol(_T("AREA"));
   if (pModel->m_colCatchmentArea < 0)
      {
      CString msg; 
      msg.Format( _T("Flow: Bad Area col %s"), filename );
      Report::ErrorMsg( msg );
      }

   pModel->m_colElev = pModel->m_pCatchmentLayer->GetFieldCol(pModel->m_elevCol);
   if (pModel->m_colElev < 0)
      {
      CString msg; 
      msg.Format( _T("Flow:  Elevation col %s doesn't exist.  It will be added and, if using station met data, it's values will be set to the station elevation (it is used for lapse rate calculations)"), filename );
      Report::ErrorMsg( msg );
      }

   gpFlow->AddModel( pModel );

   //-------------- <stream> ------------------
   TiXmlElement *pXmlStream = pXmlRoot->FirstChildElement( "streams" ); 
   float reachTimeStep = 1.0f;
   XML_ATTR streamAttrs[] = {
      // attr              type           address                          isReq  checkCol
      { "layer",           TYPE_CSTRING,  &(pModel->m_streamLayer),        true,    0 },
      { "subnode_length",  TYPE_FLOAT,    &(pModel->m_subnodeLength),      false,   0 },
      { "wd_ratio",        TYPE_FLOAT,    &(pModel->m_wdRatio),            false,   0 },
      { "stepsize",        TYPE_FLOAT,    &reachTimeStep,                  false,   0 },    // not currently used!!!!
      { "treeID_col",      TYPE_CSTRING,  &(pModel->m_treeIDCol ),          false,   0 },
      { NULL,              TYPE_NULL,     NULL,                            false,   0 } };

   ok = TiXmlGetAttributes( pXmlStream, streamAttrs, filename, NULL );
   if ( ! ok )
      {
      CString msg; 
      msg.Format( _T("Flow: Misformed root element reading <streams> attributes in input file %s"), filename );
      Report::ErrorMsg( msg );
      delete pModel;
      return false;
      }

   // success - add model and get flux definitions
   pModel->m_pStreamLayer = pIDULayer->GetMapPtr()->GetLayer( pModel->m_streamLayer );

   if ( ! pModel->m_treeIDCol.IsEmpty() )
      CheckCol(pModel->m_pStreamLayer, pModel->m_colTreeID, pModel->m_treeIDCol, TYPE_LONG, CC_AUTOADD );
   
   //-------- <global_methods> ---------------///
   //pModel->m_reachSolutionMethod = RSM_KINEMATIC;     // if not defined, default to kinematic
   //pModel->m_latExchMethod = HREX_LINEARRESERVOIR;    // default to linear_reservoir
   //pModel->m_hruVertFluxMethod = VD_BROOKSCOREY;      // default to brooks-corey
  // pModel->m_gwMethod = GW_NONE;                    // defaults to no groundwater

   TiXmlElement *pXmlGlobal = pXmlRoot->FirstChildElement( "global_methods" ); 
   if ( pXmlGlobal != NULL )
      {
      // the following has been deprecated~!~!

      //LPCTSTR exchangeMethod = NULL;
      //LPCTSTR vertFluxMethod = NULL;
      //LPCTSTR gwMethod = NULL;
      //LPCTSTR svRxnMethod = NULL;
      //XML_ATTR globalAttrs[] = {
      //   // attr                    type            address            isReq  checkCol
      //   { "hru_vertical_exchange", TYPE_STRING,  &vertFluxMethod,     false,   0 },
      //   { "groundwater",           TYPE_STRING,  &gwMethod,           false,   0 },
      //   { "extraStateVarRxn",      TYPE_STRING,  &svRxnMethod,        false,   0 },
      //   { NULL,                     TYPE_NULL,    NULL,               false,   0 } };
      //
      //ok = TiXmlGetAttributes( pXmlGlobal, globalAttrs, filename, NULL );
      //if ( ! ok )
      //   {
      //   CString msg; 
      //   msg.Format( _T("Flow: Misformed root element reading <global_methods> attributes in input file %s"), filename );
      //   Report::ErrorMsg( msg );
      //   return false;
      //   }
      //
      //if ( svRxnMethod )
      //   {
      //   if ( lstrcmpi( svRxnMethod, "none" ) == 0 )
      //      pModel->m_extraSvRxnMethod = EXSV_INFLUXHANDLER;
      //   else
      //      {
      //      pModel->m_extraSvRxnMethod = EXSV_EXTERNAL;
      //      pModel->m_extraSvRxnExtSource = svRxnMethod;
      //      }
      //   }

      // process and child nodes
      TiXmlElement *pXmlGlobalChild = pXmlGlobal->FirstChildElement();

      while ( pXmlGlobalChild != NULL )
         {
         if ( pXmlGlobalChild->Type() == TiXmlNode::ELEMENT )
            {
            PCTSTR tagName = pXmlGlobalChild->Value();

            // Reach Routing
            if ( _tcsicmp( tagName, "reach_routing") == 0 )
               {
               pModel->m_pReachRouting = ReachRouting::LoadXml( pXmlGlobalChild, filename );
               if (pModel->m_pReachRouting != NULL) 
                  GlobalMethodManager::AddGlobalMethod(pModel->m_pReachRouting);
               }

            // lateral exchnage
            else if ( _tcsicmp( tagName, "lateral_exchange") == 0 )
               {
               pModel->m_pLateralExchange = LateralExchange::LoadXml( pXmlGlobalChild, filename );
               if (pModel->m_pLateralExchange != NULL) 
                  GlobalMethodManager::AddGlobalMethod(pModel->m_pLateralExchange);
               }

            // hru vertical exchange
            else if ( _tcsicmp( tagName, "hru_vertical_exchange") == 0 )
               {
               pModel->m_pHruVertExchange = HruVertExchange::LoadXml( pXmlGlobalChild, filename );
               if (pModel->m_pHruVertExchange != NULL) 
                  GlobalMethodManager::AddGlobalMethod(pModel->m_pHruVertExchange);
               }

            // external methods
            else if ( _tcsicmp( tagName, "external") == 0 )
               {
               GlobalMethod *pMethod = ExternalMethod::LoadXml(pXmlGlobalChild, filename);
               if (pMethod != NULL)
                  GlobalMethodManager::AddGlobalMethod(pMethod);
               }

            // evap_trans
            else if ( _tcsicmp( tagName, "evap_trans") == 0 )
               {
               GlobalMethod *pMethod = EvapTrans::LoadXml( pXmlGlobalChild, pIDULayer, filename );
               if ( pMethod != NULL) 
                  GlobalMethodManager::AddGlobalMethod( pMethod );
               }

            // daily urban water demand 
            else if ( _tcsicmp( tagName, "urban_water_demand") == 0 )
               {
               GlobalMethod *pMethod = DailyUrbanWaterDemand::LoadXml(pXmlGlobalChild, pIDULayer, filename);
               if (pMethod != NULL) 
                  GlobalMethodManager::AddGlobalMethod(pMethod);
               }
            // flux (multiple entries allowed)
            else if (_tcsicmp(tagName, "flux") == 0)
            {
               GlobalMethod* pMethod = FluxExpr::LoadXml(pXmlGlobalChild, pModel, pIDULayer, filename);
               if (pMethod != NULL)
                  GlobalMethodManager::AddGlobalMethod(pMethod);
            }

            // Spring (multiple entries allowed)
            else if (_tcsicmp(tagName, "Spring") == 0)
            {
               GlobalMethod * pMethod = Spring::LoadXml(pXmlGlobalChild, pModel, pIDULayer, filename);
               if (pMethod != NULL)
                  GlobalMethodManager::AddGlobalMethod(pMethod);
            }

            // allocations (multiple entries allowed)
            else if ( _tcsicmp( tagName, "allocation") == 0 )
               {
               GlobalMethod *pMethod = WaterAllocation::LoadXml( pXmlGlobalChild, filename );
               if (pMethod != NULL) 
                  GlobalMethodManager::AddGlobalMethod(pMethod);
               }
            }   // end of: if ( pXmlGlobalChild->Type() == TiXmlNode::ELEMENT )

         pXmlGlobalChild = pXmlGlobalChild->NextSiblingElement();
         }

      
      /*-- OLD

      // Reach Routing
      TiXmlElement *pXmlReachRouting = pXmlGlobal->FirstChildElement( "reach_routing" );
      pModel->m_pReachRouting = ReachRouting::LoadXml( pXmlReachRouting, filename );
      if (pModel->m_pReachRouting != NULL) 
         GlobalMethodManager::AddGlobalMethod(pModel->m_pReachRouting);

      // lateral (horizontal) exchange
      TiXmlElement *pXmlLatExch = pXmlGlobal->FirstChildElement( "lateral_exchange" );
      pModel->m_pLateralExchange = LateralExchange::LoadXml( pXmlLatExch, filename );
      if (pModel->m_pLateralExchange != NULL) 
         GlobalMethodManager::AddGlobalMethod(pModel->m_pLateralExchange);

      // hru vertical exchange
      TiXmlElement *pXmlHruVertExch = pXmlGlobal->FirstChildElement( "hru_vertical_exchange" );
      pModel->m_pHruVertExchange = HruVertExchange::LoadXml( pXmlHruVertExch, filename );
      if (pModel->m_pHruVertExchange != NULL) 
         GlobalMethodManager::AddGlobalMethod(pModel->m_pHruVertExchange);
      
      // any external methods?
      TiXmlElement *pXmlMethod = pXmlGlobal->FirstChildElement("external");
      while (pXmlMethod != NULL)
      {
         GlobalMethod *pMethod = ExternalMethod::LoadXml(pXmlMethod, filename);
         if (pMethod != NULL)
            GlobalMethodManager::AddGlobalMethod(pMethod);

         pXmlMethod = pXmlMethod->NextSiblingElement("external");
      }  // end of : while ( Method xml is not NULL )

      // evapotranspiration (multiple entries allowed)
      TiXmlElement *pXmlEvapTrans = pXmlGlobal->FirstChildElement( "evap_trans" ); 
      while ( pXmlEvapTrans != NULL )
         {
         GlobalMethod *pMethod = EvapTrans::LoadXml( pXmlEvapTrans, pIDULayer, filename );
         if ( pMethod != NULL) GlobalMethodManager::AddGlobalMethod( pMethod );

         pXmlEvapTrans = pXmlEvapTrans->NextSiblingElement( "evap_trans" );
         }

      // daily urban water demand 
      TiXmlElement *pXmlDailyUrbWaterDmd = pXmlGlobal->FirstChildElement("Urban_Water_Demand");
      while (pXmlDailyUrbWaterDmd != NULL)
         {
         GlobalMethod *pMethod = DailyUrbanWaterDemand::LoadXml(pXmlDailyUrbWaterDmd, pIDULayer, filename);
         if (pMethod != NULL) 
            GlobalMethodManager::AddGlobalMethod(pMethod);

         pXmlDailyUrbWaterDmd = pXmlDailyUrbWaterDmd->NextSiblingElement("Urban_Water_Demand");
         }

      // flux (multiple entries allowed)
      TiXmlElement *pXmlFlux = pXmlGlobal->FirstChildElement( "flux" ); 
      while ( pXmlFlux != NULL )
         {
         GlobalMethod *pMethod = FluxExpr::LoadXml( pXmlFlux, pModel, pIDULayer, filename );
         if (pMethod != NULL) 
            GlobalMethodManager::AddGlobalMethod(pMethod);

         pXmlFlux = pXmlFlux->NextSiblingElement( "flux" );
         }

      // allocations (multiple entries allowed)
      TiXmlElement *pXmlAlloc = pXmlGlobal->FirstChildElement( "allocation" ); 
      while ( pXmlAlloc != NULL )
         {
         GlobalMethod *pMethod = WaterAllocation::LoadXml( pXmlAlloc, filename );
         if (pMethod != NULL) 
            GlobalMethodManager::AddGlobalMethod(pMethod);

         pXmlAlloc = pXmlAlloc->NextSiblingElement( "allocation" );
         }
         */
      }
      
   // climate scenarios - now in a separate Climate Scenarios XML file, identified in the ENVX <scenarios> block
   TiXmlElement *pXmlScenarios = pXmlRoot->FirstChildElement( "climate_scenarios" );
   if (pXmlScenarios != NULL)
   {
      CString msg;
      msg.Format("FlowProcess::LoadXml() A <climate_scenarios> block was found in the Flow XML file. Climate scenario data "
         "is no longer read from the Flow XML file, but is in a separate file identified in the <scenarios> block of the ENVX file.");
      Report::ErrorMsg(msg);
      return(false);
   }

      //----------- <reservoirs> ------------------------------
   TiXmlElement *pXmlReservoirs = pXmlRoot->FirstChildElement( "reservoirs" ); 
   if ( pXmlReservoirs != NULL )
      {
      bool ok = true;

      LPCTSTR field = pXmlReservoirs->Attribute( "col" );
      if ( field == NULL )
         {
         CString msg; 
         msg.Format( _T("Flow: Missing 'col' attribute reading <reservoirs> tag in input file %s"), filename );
         Report::ErrorMsg( msg );
         }
      else
         ok = CheckCol( pModel->m_pStreamLayer, pModel->m_colResID, field, TYPE_INT, CC_MUST_EXIST );

      if ( ok )
         {
         TiXmlElement *pXmlRes = pXmlReservoirs->FirstChildElement( "reservoir" ); 

         while ( pXmlRes != NULL )
            {
            Reservoir *pRes = new Reservoir;
            LPCSTR reservoirType = NULL;
           
            float volume=0.0f;
            pRes->m_resSWmult = 1.f;
            XML_ATTR resAttrs[] = {
               // attr                  type          address                           isReq  checkCol
               { "id",               TYPE_INT,      &(pRes->m_id),                      true,   0 },
               { "name",             TYPE_CSTRING,  &(pRes->m_name),                    true,   0 },
               { "path",             TYPE_CSTRING,  &(pRes->m_dir),                     false,   0 },
               { "initVolume",       TYPE_FLOAT,    &(volume),                          false,   0 },
               { "area_vol_curve",   TYPE_CSTRING,  &(pRes->m_areaVolCurveFilename),    false,   0 },
               { "av_dir",           TYPE_CSTRING,  &(pRes->m_avdir),                   false,   0 },
               { "minOutflow",       TYPE_FLOAT,    &(pRes->m_minOutflow),              false,   0 },
               { "rule_curve",       TYPE_CSTRING,  &(pRes->m_ruleCurveFilename),       false,   0 },
               { "buffer_zone",      TYPE_CSTRING,  &(pRes->m_bufferZoneFilename),      false,   0 },
               { "rc_dir",           TYPE_CSTRING,  &(pRes->m_rcdir),                   false,   0 },
               { "maxVolume",        TYPE_FLOAT,    &(pRes->m_maxVolume),               false,   0 },
               { "composite_rc",     TYPE_CSTRING,  &(pRes->m_releaseCapacityFilename), false,   0 },
               { "re_dir",           TYPE_CSTRING,  &(pRes->m_redir),                   false,   0 },
               { "maxPowerFlow",     TYPE_FLOAT,    &(pRes->m_gateMaxPowerFlow),        false,   0 },
               { "RO_rc",            TYPE_CSTRING,  &(pRes->m_RO_CapacityFilename),     false,   0 },
               { "spillway_rc",      TYPE_CSTRING,  &(pRes->m_spillwayCapacityFilename),false,   0 },
               { "cp_dir",           TYPE_CSTRING,  &(pRes->m_cpdir),                   false,   0 },
               { "rule_priorities",  TYPE_CSTRING,  &(pRes->m_rulePriorityFilename),    false,   0 },
               { "ressim_output_f",  TYPE_CSTRING,  &(pRes->m_ressimFlowOutputFilename),false,   0 },
               { "ressim_output_r",  TYPE_CSTRING,  &(pRes->m_ressimRuleOutputFilename),false,   0 },
               { "rp_dir",           TYPE_CSTRING,  &(pRes->m_rpdir),                   false,   0 },
               { "td_elev",          TYPE_FLOAT,    &(pRes->m_dam_top_elev),            false,   0 },  // Top of dam elevation
               { "fc1_elev",         TYPE_FLOAT,    &(pRes->m_fc1_elev),                false,   0 },  // Top of primary flood control zone
               { "fc2_elev",         TYPE_FLOAT,    &(pRes->m_fc2_elev),                false,   0 },  // Seconary flood control zone (if applicable)
               { "fc3_elev",         TYPE_FLOAT,    &(pRes->m_fc3_elev),                false,   0 },  // Tertiary flood control zone (if applicable)
               { "tailwater_elev",   TYPE_FLOAT,    &(pRes->m_tailwater_elevation),     false,   0 },
               { "turbine_efficiency",TYPE_FLOAT,   &(pRes->m_turbine_efficiency),      false,   0 },
               { "inactive_elev",    TYPE_FLOAT,    &(pRes->m_inactive_elev),           true,    0 },
               { "reservoir_type",   TYPE_STRING,   &(reservoirType),                   true,    0 },
               { "release_freq",       TYPE_INT,      &(pRes->m_releaseFreq),           false,    0 },
               { "probabilityOfMaintenance", TYPE_FLOAT, &(pRes->m_probMaintenance),    false, 0 },
               { "solar_radiation_multiplier", TYPE_FLOAT, &(pRes->m_resSWmult),    false, 0 },
               { NULL,               TYPE_NULL,     NULL,                               false,   0 } };

            
            ok = TiXmlGetAttributes( pXmlRes, resAttrs, filename, NULL );
            if ( ! ok )
               {
               CString msg; 
               msg.Format( _T("Flow: Misformed element reading <reservoir> attributes in input file %s"), filename );
               Report::ErrorMsg( msg );
               delete pRes;
               pRes = NULL;
               }

            if (pRes->m_resSWmult <= 0 || pRes->m_resSWmult > 1)
            {
               CString msg;
               msg.Format("Flow: solar radiation multiplier for reservoir %s is <= 0 or > 1. Setting it to 1.0 now.",
                  pRes->m_name);
               Report::WarningMsg(msg);
               pRes->m_resSWmult = 1.f;
            }
           
            switch (reservoirType[0])
            {
               case 'F':
               {
                  pRes->m_reservoirType = ResType_FloodControl;
                  CString msg;
                  msg.Format("Flow: Reservoir %s is Flood Control", (LPCTSTR)pRes->m_name);
                  break;
               }
               case 'R':
               {
                  pRes->m_reservoirType = ResType_RiverRun;
                  CString msg;
                  msg.Format("Flow: Reservoir %s is River Run", (LPCTSTR)pRes->m_name);
                  break;
               }
               case 'C':
               {
                  pRes->m_reservoirType = ResType_CtrlPointControl;
                  CString msg;
                  msg.Format("Flow: Reservoir %s is Control Point Control", (LPCTSTR)pRes->m_name);
                  break;
               }
               default:
               {
                  CString msg("Flow: Unrecognized 'reservoir_type' attribute '");
                  msg += reservoirType;
                  msg += "' reading <reservoir> tag for Reservoir '";
                  msg += (LPCSTR)pRes->m_name;
                  //      msg += "'. This output will be ignored...";
                  Report::ErrorMsg(msg);
                  break;
               }
            }

            if (pRes != NULL)
            {
               pRes->InitializeReservoirVolume(volume);
               pModel->m_reservoirArray.Add(pRes);
            }

            pXmlRes = pXmlRes->NextSiblingElement( "reservoir" );
            }
         }
      }
   
   //----------- <control points> ------------------------------
   TiXmlElement *pXmlControlPoints = pXmlRoot->FirstChildElement( "controlPoints" ); 
   if ( pXmlControlPoints != NULL )
      {
      bool ok = true;

      if ( ok )
         {
         TiXmlElement *pXmlCP = pXmlControlPoints->FirstChildElement( "controlPoint" ); 

         while ( pXmlCP != NULL )
            {
            ControlPoint *pControl = new ControlPoint;

            XML_ATTR cpAttrs[] = {
               // attr               type         address                                isReq   checkCol
            { "name",               TYPE_CSTRING,   &(pControl->m_name),                 true,   0 },
            { "id",                 TYPE_INT,       &(pControl->m_id),                   false,  0 },
            { "path",               TYPE_CSTRING,   &(pControl->m_dir),                  true,   0 },
            { "control_point",      TYPE_CSTRING,   &(pControl->m_controlPointFileName), true,   0 },
            { "location",           TYPE_INT,       &(pControl->m_location),             true,   0 },
            { "reservoirs",         TYPE_CSTRING,   &(pControl->m_reservoirsInfluenced), true,   0 },
            { "local_col",          TYPE_INT,       &(pControl->localFlowCol),           false,  0 },
            { NULL,                  TYPE_NULL,     NULL,                                false,  0 } };
               
            ok = TiXmlGetAttributes( pXmlCP, cpAttrs, filename, NULL );
            if ( ! ok )
               {
               CString msg; 
               msg.Format( _T("Flow: Misformed element reading <controlPoint> attributes in input file %s"), filename );
               Report::ErrorMsg( msg );
               delete pControl;
               }
            else
               pModel->m_controlPointArray.Add( pControl );

            pXmlCP = pXmlCP->NextSiblingElement( "controlPoint" );
            }
         }
      }

   //-------- extra state variables next ------------
   StateVar *pSV = new StateVar;  // first is always "water"
   pSV->m_name  = "water";
   pSV->m_index = 0;
   pSV->m_uniqueID = -1;
   pModel->AddStateVar( pSV );
   LPTSTR appTablePath = NULL;
   LPTSTR paramTablePath = NULL;
   LPCTSTR fieldname = NULL;

   TiXmlElement *pXmlStateVars = pXmlRoot->FirstChildElement( "additional_state_vars" ); 
   if (pXmlStateVars != NULL)
      {
      TiXmlElement *pXmlStateVar = pXmlStateVars->FirstChildElement("state_var");

      while (pXmlStateVar != NULL)
         {
         pSV = new StateVar;

         XML_ATTR svAttrs[] = {
            // attr              type          address                   isReq  checkCol
            { "name", TYPE_CSTRING, &(pSV->m_name), true, 0 },
            { "unique_id", TYPE_INT, &(pSV->m_uniqueID), true, 0 },
            { "integrate", TYPE_INT, &(pSV->m_isIntegrated), false, 0 },

            { "fieldname", TYPE_STRING, &fieldname, false, 0 },

            { "applicationTable", TYPE_STRING, &appTablePath, false, 0 },
            { "parameterTable", TYPE_STRING, &paramTablePath, false, 0 },

            { "scalar", TYPE_FLOAT, &(pSV->m_scalar), false, 0 },



            { NULL, TYPE_NULL, NULL, false, 0 } };

         ok = TiXmlGetAttributes(pXmlStateVar, svAttrs, filename, NULL);
         if (!ok)
            {
            CString msg;
            msg.Format(_T("Flow: Misformed root element reading <state_var> attributes in input file %s"), filename);
            Report::ErrorMsg(msg);
            delete pSV;
            pSV = NULL;
            }

         if (pSV)
            pModel->AddStateVar(pSV);


         if (appTablePath != NULL)
         {
            DataObj *pDataObj;
            pDataObj = new FDataObj;

            int rows = pDataObj->ReadAscii(pModel->m_path + appTablePath);

            if (rows <= 0)
            {
               CString msg(_T("Flow: ESV table source error: Unable to open/read "));
               msg += appTablePath;
               Report::ErrorMsg(msg);
               delete pDataObj;
            }
            else
            {
               ParamTable *pTable = new ParamTable;

               pTable->m_pTable = pDataObj;
               pTable->m_type = pDataObj->GetDOType();

               if (fieldname != NULL)
               {
                  pTable->m_fieldName = fieldname;
                  pTable->m_iduCol = pIDULayer->GetFieldCol(fieldname);

                  if (pTable->m_iduCol < 0)
                  {
                     CString msg(_T("Flow: ESV table source error: column '"));
                     msg += fieldname;
                     msg += "; not found in IDU coverage";
                     Report::ErrorMsg(msg);
                  }
               }

               pTable->CreateMap();
               pSV->m_tableArray.Add(pTable);
            }
         }

         // read in esv parameter  Table
         //pSV->m_paramTablePath = paramTablePath;
         if (paramTablePath != NULL)
         {
            pSV->m_paramTable = new VDataObj;
            pSV->m_paramTable->ReadAscii(paramTablePath, ',');
         }

         pXmlStateVar = pXmlStateVar->NextSiblingElement("state_var");
         }
      }

   // set state variable count.  Assumes all pools have the same number of extra state vars

   pModel->m_hruSvCount = pModel->m_reachSvCount = pModel->m_reservoirSvCount = pModel->GetStateVarCount();
  
   //// fluxes next
   //TiXmlElement *pXmlFluxes = pXmlRoot->FirstChildElement( "fluxes" ); 
   //if ( pXmlFluxes != NULL )
   //   {
   //   // get fluxes next
   //   TiXmlElement *pXmlFlux = pXmlFluxes->FirstChildElement( "flux" );
   //   
   //   while ( pXmlFlux != NULL )
   //      {
   //      FluxInfo *pFluxInfo = new FluxInfo;
   //
   //      //  Specifies all fluxes.  
   //      //    type:   1 - reach, 2=catchment, 3=hru, 4=hruLayer
   //      //    query:  where does this flux apply
   //      //    source: fn:<dllpath:function> for functional, db:<datasourcepath:columnname> for datasets
   //      LPTSTR sourceLocation = NULL;
   //      LPTSTR sinkLocation   = NULL;
   //      LPTSTR stateVars  = NULL;
   //      LPTSTR fluxType   = NULL;
   //
   //      XML_ATTR attrs[] = {
   //         // attr             type            address                         isReq checkCol
   //         { "name",           TYPE_CSTRING,   &(pFluxInfo->m_name),          true,  0 },
   //         { "path",           TYPE_CSTRING,   &(pFluxInfo->m_path),          true,  0 },
   //         { "description",    TYPE_CSTRING,   &(pFluxInfo->m_description),   false, 0 },
   //         { "type",           TYPE_STRING,    &fluxType,                     false, 0 },
   //         { "source_type",    TYPE_STRING,    &sourceLocation,               false, 0 },
   //         { "source_query",   TYPE_CSTRING,   &(pFluxInfo->m_sourceQuery),   false, 0 },
   //         { "sink_type",      TYPE_STRING,    &sinkLocation,                 false, 0 },
   //         { "sink_query",     TYPE_CSTRING,   &(pFluxInfo->m_sinkQuery),     false, 0 },
   //         { "flux_handler",   TYPE_CSTRING,   &(pFluxInfo->m_fnName),        false, 0 },
   //         { "field",          TYPE_CSTRING,   &(pFluxInfo->m_fieldName),     false, 0 },
   //         { "state_vars",     TYPE_STRING,    &stateVars,                    false, 0 },
   //         { "init_handler",   TYPE_CSTRING,   &(pFluxInfo->m_initFn),        false, 0 },
   //         { "initRun_handler",TYPE_CSTRING,   &(pFluxInfo->m_initRunFn),     false, 0 },
   //         { "endRun_handler", TYPE_CSTRING,   &(pFluxInfo->m_endRunFn),      false, 0 },
   //         { "initInfo",       TYPE_CSTRING,   &(pFluxInfo->m_initInfo),      false, 0 },
   //         { NULL,             TYPE_NULL,      NULL,                          true,  0 }
   //         };
   //
   //      bool ok = TiXmlGetAttributes( pXmlFlux, attrs, filename );
   //
   //      if ( !ok )
   //         {
   //         CString msg; 
   //         msg.Format( _T("Flow: Misformed flux element reading <flux> attributes in input file %s"), filename );
   //         Report::ErrorMsg( msg );
   //         delete pFluxInfo;
   //         }
   //      else
   //         {
   //         pFluxInfo->m_type = FT_UNDEFINED;
   //
   //         if ( fluxType != NULL) 
   //            {
   //            if ( *fluxType =='p' || *fluxType == 'P' )
   //               pFluxInfo->m_type = FT_PRECIP;
   //            }
   //
   //         // good xml, figure everything out
   //         // start with source info
   //         bool okSource = true;
   //         bool okSink   = true;
   //
   //         if ( sourceLocation != NULL )
   //            {
   //            okSource = pModel->ParseFluxLocation( sourceLocation, pFluxInfo->m_sourceLocation, pFluxInfo->m_sourceLayer );
   //
   //            // trap errors
   //            if ( ! okSource )
   //               {
   //               CString msg; 
   //               msg.Format( _T("Flow: Unrecognized flux <source_type> attribute '%s' in input file %s - this should be 'hruLayer', 'reach', 'reservoir', or 'gw' "), sourceLocation, filename );
   //               Report::ErrorMsg( msg );
   //               }
   //
   //            if ( pFluxInfo->m_sourceLocation == FL_HRULAYER && pFluxInfo->m_sourceLayer >= pModel->GetHRULayerCount() )
   //               {
   //               CString msg;
   //               msg.Format( "Flow: layer specified in Flux '%s' exceeds layer count specfied in <catchment> definition.  This flux will be disabled.", (LPCTSTR) pFluxInfo->m_name );
   //               Report::WarningMsg( msg );
   //               pFluxInfo->m_use = false;
   //               }
   //            }
   //
   //         // next, sinks
   //         if ( sinkLocation != NULL )
   //            {
   //            okSink = pModel->ParseFluxLocation( sinkLocation, pFluxInfo->m_sinkLocation, pFluxInfo->m_sinkLayer );
   //            if ( ! okSink )
   //               {
   //               CString msg; 
   //               msg.Format( _T("Flow: Unrecognized flux <sink_type> attribute '%s' in input file %s - this should be 'hruLayer', 'reach', 'reservoir', or 'gw' "), sinkLocation, filename );
   //               Report::ErrorMsg( msg );
   //               }
   //
   //            if ( pFluxInfo->m_sinkLocation == FL_HRULAYER && pFluxInfo->m_sinkLayer >= pModel->GetHRULayerCount() )
   //               {
   //               CString msg;
   //               msg.Format( "Flow: layer specified in Flux '%s' exceeds layer count specfied in <catchment> definition.  This flux will be disabled.", (LPCTSTR) pFluxInfo->m_name );
   //               Report::WarningMsg( msg );
   //               pFluxInfo->m_use = false;
   //               }
   //            }
   //
   //         if ( okSource == false || okSink == false )
   //            delete pFluxInfo;
   //         else
   //            {
   //            // state vars next...
   //            if ( stateVars == NULL )
   //               {
   //               // assume default is "water" if not specified
   //               pFluxInfo->m_stateVars.Add( pModel->GetStateVar( 0 ) );
   //               }
   //            else
   //               {
   //               // parse state vars
   //               TCHAR *buff = new TCHAR[ lstrlen( stateVars )+1 ];
   //               lstrcpy( buff, stateVars );
   //               LPTSTR next;
   //               TCHAR *varName = _tcstok_s( buff, _T("|,"), &next );
   //
   //               while ( varName != NULL )
   //                  {
   //                  StateVar *pSV = pModel->FindStateVar( varName );
   //
   //                  if ( pSV != NULL )
   //                     pFluxInfo->m_stateVars.Add( pSV );
   //                  else
   //                     {
   //                     CString msg( "Flow: Unrecognized state variable '" );
   //                     msg += varName;
   //                     msg += "' found while reading <flux> '";
   //                     msg += pFluxInfo->m_name;
   //                     msg += "' attribute 'state_vars'";
   //                     Report::WarningMsg( msg );
   //                     }
   //
   //                  varName = _tcstok_s( NULL, _T( "|," ), &next );
   //                  }
   //
   //               delete [] buff;
   //               }
   //
   //            pModel->AddFluxInfo( pFluxInfo );
   //            }
   //         }
   //
   //      pXmlFlux = pXmlFlux->NextSiblingElement( "flux" );
   //      }
   //   }
   //
   // tables next
   TiXmlElement *pXmlTables = pXmlRoot->FirstChildElement( "tables" ); 
   if ( pXmlTables != NULL )
      {
      // get tables next
      TiXmlElement *pXmlTable = pXmlTables->FirstChildElement( "table" );
      while ( pXmlTable != NULL )
         {
         LPCTSTR name = NULL;
         LPCTSTR description = NULL;
         LPCTSTR fieldname = NULL;
         LPCTSTR type = NULL;
         LPCTSTR source = NULL;
         XML_ATTR attrs[] = {
            // attr             type            address                         isReq checkCol
            { "name",           TYPE_STRING,   &name,               true,  0 },
            { "description",    TYPE_STRING,   &description,        false, 0 },
            { "type",           TYPE_STRING,   &type,               true,  0 },
            { "col",            TYPE_STRING,   &fieldname,          false, 0 },
            { "source",         TYPE_STRING,   &source,             true,  0 },
            { NULL,             TYPE_NULL,     NULL,                true,  0 }
            };

         bool ok = TiXmlGetAttributes( pXmlTable, attrs, filename );

         if ( !ok )
            {
            CString msg; 
            msg.Format( _T("Flow: Misformed table element reading <table> attributes in input file %s"), filename );
            Report::ErrorMsg( msg );
            }
         else
            {
            DataObj *pDataObj;

            if ( *type == 'v' )
               pDataObj = new VDataObj;
            else if ( *type == 'i' )
               pDataObj = new IDataObj;
            else
               pDataObj = new FDataObj;

            int rows = pDataObj->ReadAscii( pModel->m_path+source );

            if ( rows <= 0 )
               {
               CString msg( _T( "Flow: Flux table source error: Unable to open/read ") );
               msg += source;
               Report::ErrorMsg( msg );
               delete pDataObj;
               }
            else
               {
               ParamTable *pTable = new ParamTable;

               pTable->m_pTable = pDataObj;
               pTable->m_pInitialValuesTable = pDataObj;  ///???? 
               pTable->m_pTable->SetName(name);
               pTable->m_type = pDataObj->GetDOType();
               pTable->m_description = description;
               
               if ( fieldname != NULL )
                  {
                  pTable->m_fieldName = fieldname;
                  pTable->m_iduCol = pIDULayer->GetFieldCol( fieldname );

                  if ( pTable->m_iduCol < 0 )
                     {
                     CString msg( _T( "Flow: Flux table source error: column '" ) );
                     msg += fieldname;
                     msg += "; not found in IDU coverage";
                     Report::ErrorMsg( msg );
                     }
                  }

               pTable->CreateMap();

               pModel->m_tableArray.Add( pTable );
               }
            }
    
         pXmlTable = pXmlTable->NextSiblingElement( "table" );
         }

         pModel->m_colHbvW2A_SLP = pModel->m_colHbvW2A_INT = -1;
         ParamTable* pHBVtable = pModel->GetTable("HBV");
         if (pHBVtable != NULL)
         {
            pModel->m_colHbvW2A_SLP = pHBVtable->GetFieldCol("W2A_SLP");
            pModel->m_colHbvW2A_INT = pHBVtable->GetFieldCol("W2A_INT");
         }
         if (pHBVtable == NULL || pModel->m_colHbvW2A_SLP < 1 || pModel->m_colHbvW2A_INT < 1)
         {
            CString msg;
            msg.Format("FlowProcess::LoadXml() Could not locate the W2A_SLP or W2A_INT columns in the HBV XML file .");
            Report::WarningMsg(msg);
         }
   }

// Parameter Estimation 
   TiXmlElement *pXmlParameters = pXmlRoot->FirstChildElement( "parameterEstimation" ); 
   if ( pXmlParameters != NULL )
      {
      float numberOfRuns;
      float numberOfYears;
      bool estimateParameters;
      float saveResultsEvery;
      float nsThreshold;
      long rnSeed = -1;

      XML_ATTR paramAttrs[] = {
         { "estimateParameters",TYPE_BOOL,    &estimateParameters, true,   0 },
         { "numberOfRuns",      TYPE_FLOAT,   &numberOfRuns,       true,   0 },
         { "numberOfYears",     TYPE_FLOAT,   &numberOfYears,      true,   0 },
         { "saveResultsEvery",  TYPE_FLOAT,   &saveResultsEvery,   true,   0 },
         { "nsThreshold",       TYPE_FLOAT,   &nsThreshold,        false,   0 },
         { "randomSeed",        TYPE_LONG,    &rnSeed,             false,  0 },
         { NULL,                TYPE_NULL,    NULL,                false,  0 } };
      
      bool ok = TiXmlGetAttributes( pXmlParameters, paramAttrs, filename );
      if ( ! ok )
         {
         CString msg; 
         msg.Format( _T("Flow: Misformed element reading <parameterEstimation> attributes in input file %s"), filename );
         Report::ErrorMsg( msg );
         }
      else
         {
         pModel->m_estimateParameters=estimateParameters;
         pModel->m_numberOfRuns=(int)numberOfRuns;
         pModel->m_numberOfYears = (int)numberOfYears;
         pModel->m_saveResultsEvery = (int)saveResultsEvery;
         pModel->m_nsThreshold = nsThreshold;
         pModel->m_rnSeed = rnSeed;

         if ( rnSeed >= 0 )
            pModel->m_randUnif1.SetSeed( rnSeed );

         //if ( outputPath.IsEmpty() )
         //   {
         //   pModel->m_paramEstOutputPath = pModel->m_path;
         //   pModel->m_paramEstOutputPath += "\\output\\";
         //   }
         //else
         //   pModel->m_paramEstOutputPath += outputPath;
         }
      // get tables next
      TiXmlElement *pXmlParameter = pXmlParameters->FirstChildElement( "parameter" );
      while ( pXmlParameter != NULL )
         {
         LPCTSTR name = NULL;
         LPCTSTR table = NULL;
         float value = 0.0f;
         float minValue = 0.0f;
         float maxValue = 0.0f;
         bool distributeSpatially = true;
         int id = NULL;
         XML_ATTR attrs[] = {
            // attr             type            address                         isReq checkCol
            { "table",    TYPE_STRING,  &table,        true,  0 },
            { "name",     TYPE_STRING,   &name,       false, 0 },
            { "value",    TYPE_FLOAT,   &value,       false, 0 },
            { "minValue", TYPE_FLOAT,   &minValue,    false, 0 },
            { "maxValue", TYPE_FLOAT,   &maxValue,    false, 0 },
            { "distributeSpatially",  TYPE_BOOL,   &distributeSpatially,    false, 0 },
            { NULL,       TYPE_NULL,     NULL,        true,  0 }
            };

         bool ok = TiXmlGetAttributes( pXmlParameter, attrs, filename );

         if ( !ok )
            {
            CString msg; 
            msg.Format( _T("Flow: Misformed element reading <reach_location> attributes in input file %s"), filename );
            Report::ErrorMsg( msg );
            }
         else
            {
            ParameterValue *pParam = new ParameterValue;
            if ( name != NULL )
               {
               pParam->m_name = name;
               pParam->m_table = table;
               pParam->m_value = value;
               pParam->m_minValue = minValue;
               pParam->m_maxValue = maxValue;
               pParam->m_distribute = distributeSpatially;

               }
            pModel->m_parameterArray.Add( pParam );
            }     
         pXmlParameter = pXmlParameter->NextSiblingElement( "parameter" );
         }
      }


   // output expressions
   TiXmlElement *pXmlModelOutputs = pXmlRoot->FirstChildElement( "outputs" ); 
   if ( pXmlModelOutputs != NULL ) 
   {
      CString output_groups_file;
      XML_ATTR output_groups_attrs[] = {
         {"output_groups_file", TYPE_CSTRING, &output_groups_file, true, 0},
         { NULL,                      TYPE_NULL,     NULL,                                false,   0 } };

      ok = TiXmlGetAttributes(pXmlModelOutputs, output_groups_attrs, filename, NULL);
      ::ApplySubstituteStrings(output_groups_file, pEnvContext->m_substituteStrings);
      CString output_groups_path;
      bool output_groups_ok = PathManager::FindPath(output_groups_file, output_groups_path) >= 0; // return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found
      if (output_groups_ok)
      {
         CString msg;
         msg.Format("Loading FLOW report specifications from file %s.", output_groups_path);
         Report::LogMsg(msg);
      }
      else
      {
         CString msg;
         msg.Format("FlowProcess::LoadXml() FLOW Reports file '%s' not found - no FLOW reports will be produced", output_groups_file.GetString());
         Report::LogMsg(msg);
      }

      TiXmlDocument doc;
      if (output_groups_ok)
      {
         output_groups_ok = doc.LoadFile(output_groups_path);
         if (!output_groups_ok)
         {
            CString msg;
            msg.Format("Error reading FLOW Reports file, %s", (LPCTSTR)output_groups_path);
            Report::ErrorMsg(msg);
         }
      }

      TiXmlElement* pXmlOutputGroupsRoot = NULL;
      TiXmlElement* pXmlGroup = NULL;
      if (output_groups_ok)
      {
         pXmlOutputGroupsRoot = doc.RootElement();
         pXmlGroup = pXmlOutputGroupsRoot->FirstChildElement("output_group");
         output_groups_ok = pXmlGroup != NULL;
      }

      while (output_groups_ok && pXmlGroup != NULL)
      {
         CString group_name = pXmlGroup->Attribute("name");
         if (group_name.IsEmpty())
         {
            output_groups_ok = false;
            CString msg = "FlowProcess::LoadXml() Output group name is an empty string.";
            Report::ErrorMsg(msg);
            continue;
         }

         int multiple_intervals = 0;
         CString yes_or_no;
         
         yes_or_no = pXmlGroup->Attribute("daily"); yes_or_no.MakeLower();
         if (!yes_or_no.IsEmpty() && (yes_or_no.GetAt(0) == 'y'))
            multiple_intervals = multiple_intervals | (int)(pEnvContext->m_useWaterYears ? pow(2., MOI_WATERYEAR_DAILY) : pow(2., MOI_DAILY));

         yes_or_no = pXmlGroup->Attribute("monthly"); yes_or_no.MakeLower();
         if (!yes_or_no.IsEmpty() && (yes_or_no.GetAt(0) == 'y'))
            multiple_intervals = multiple_intervals | (int)(pEnvContext->m_useWaterYears ? pow(2., MOI_WATERYEAR_MONTHLY) : pow(2., MOI_MONTHLY));

         yes_or_no = pXmlGroup->Attribute("yearly"); yes_or_no.MakeLower();
         if (!yes_or_no.IsEmpty() && (yes_or_no.GetAt(0) == 'y'))
            multiple_intervals = multiple_intervals | (int)(pEnvContext->m_useWaterYears ? pow(2., MOI_WATERYEAR_YEARLY) : pow(2., MOI_YEARLY));

         CString interval_name = pXmlGroup->Attribute("interval"); interval_name.MakeLower();
         if (!interval_name.IsEmpty())  switch (interval_name[0])
         {
            case 'd': // daily, DAILY
               multiple_intervals = multiple_intervals | (int)(pEnvContext->m_useWaterYears ? pow(2., MOI_WATERYEAR_DAILY) : pow(2., MOI_DAILY));
               break;
            case 'm': // monthly, MONTHLY
               multiple_intervals = multiple_intervals | (int)(pEnvContext->m_useWaterYears ? pow(2., MOI_WATERYEAR_MONTHLY) : pow(2., MOI_MONTHLY));
               break;
            case 'y': // yearly, YEARLY
               multiple_intervals = multiple_intervals | (int)(pEnvContext->m_useWaterYears ? pow(2., MOI_WATERYEAR_YEARLY) : pow(2., MOI_YEARLY));
               break;
            default:
            {
               CString msg("Flow: Unrecognized 'interval' attribute '");
               msg += interval_name;
               msg += "' reading <output_group> tag for '";
               msg += group_name;
               msg += "'. This output group will be ignored...";
               Report::ErrorMsg(msg);
               output_groups_ok = false;
               continue;
            }
         } // end of ... else switch (interval_name_[0])

         if (multiple_intervals == 0) multiple_intervals = (int)(pEnvContext->m_useWaterYears ? pow(2., MOI_WATERYEAR_DAILY) : pow(2., MOI_DAILY));

         bool first_interval = true;
         int bit_num = 0;
         ModelOutputGroup* pGroup = new ModelOutputGroup;
         CString observations_path;
         while (multiple_intervals != 0)
         {
            bit_num++;
            if ((multiple_intervals & (int)pow(2., bit_num)) == 0) continue;
            MOINTERVAL interval = (MOINTERVAL) bit_num;
            multiple_intervals = multiple_intervals - (int)pow(2., bit_num);
            CString group_name_with_interval;
            switch (interval)
            {
               case MOI_YEARLY:  group_name_with_interval = group_name + "_yearly"; break;
               case MOI_MONTHLY: group_name_with_interval = group_name + "_monthly"; break;
               case MOI_DAILY: group_name_with_interval = group_name + "_daily"; break;
               case MOI_WATERYEAR_YEARLY: group_name_with_interval = group_name + "_wateryear"; break;
               case MOI_WATERYEAR_MONTHLY: group_name_with_interval = group_name + "_monthly_in_wateryear"; break;
               case MOI_WATERYEAR_DAILY: group_name_with_interval = group_name + "_daily_in_wateryear"; break;
               default: ASSERT(0); break;
            } // end of switch (interval)

            if (first_interval)
            {
               pGroup->m_moInterval = interval;
               pGroup->m_name = group_name_with_interval;

               TiXmlElement* pXmlModelOutput = pXmlGroup->FirstChildElement("output");
               while (output_groups_ok && pXmlModelOutput != NULL)
               {
                  LPTSTR name = NULL;
                  LPTSTR query = NULL;
                  LPTSTR expr = NULL;
                  bool   inUse = true;
                  LPTSTR type = NULL;
                  CString domain;
                  LPCSTR obs = NULL;
                  LPCSTR format = NULL;

                  XML_ATTR attrs[] = {
                        // attr           type         address                 isReq checkCol
                        { "name",         TYPE_STRING, &name,                  true,   0 },
                        { "query",        TYPE_STRING, &query,                 true,  0 },
                        { "value",        TYPE_STRING, &expr,                  true,   0 },
                        { "in_use",       TYPE_BOOL,   &inUse,                 false,  0 },
                        { "type",         TYPE_STRING, &type,                  true,   0 },
                        { "domain",       TYPE_CSTRING, &domain,                true,   0 },
                        { "obs",          TYPE_STRING, &obs,                   false,  0 },
                        { "format",       TYPE_STRING, &format,                false,  0 },
                        { NULL,           TYPE_NULL,   NULL,                   false,  0 } };

                  bool ok = TiXmlGetAttributes(pXmlModelOutput, attrs, output_groups_file);
                  if (!ok)
                  {
                     CString msg;
                     msg.Format(_T("Flow: Misformed element reading <output> attributes in input file %s"), output_groups_file);
                     Report::ErrorMsg(msg);
                  }
                  else
                  {
                     ModelOutput* pOutput = new ModelOutput;

                     pOutput->m_name = name;
                     pOutput->m_queryStr = query;

                     pOutput->m_inUse = inUse;
                     if (obs != NULL)
                     {
                        pOutput->m_nameObs = obs;
                        int path_manager_rtn = PathManager::FindPath(obs, observations_path);
                        if ((path_manager_rtn < 0) || (format == NULL))
                        {
                           CString msg;
                           if (path_manager_rtn < 0)
                           {
                              msg.Format("Flow: Unable to find observation file '%s' specified for Model Output '%s'", obs, name);
                              Report::WarningMsg(msg);
                           }
                           else if (format == NULL)
                           {
                              msg.Format("FlowProcess::LoadXml() missing format for observation %s", name);
                              Report::WarningMsg(msg);
                           }
                           pOutput->m_inUse = false;
                        } // end of if ((path_manager_rtn < 0) || (format == NULL))
                        else 
                        {
                           pOutput->m_pDataObjObs = new FDataObj;
                           //CString inFile;
                           //inFile.Format("%s%s",pModel->m_path,obs);    // m_path is not slash-terminated
                           int rows = -1;
                           switch (format[0])
                           {
                              case 'E': // E<day number of observation on Jan 1, 1900>
                                 // E0 implies that observation index 0 is for Jan 1, 1900 (used when leap days are included)
                                 // E693500 implies that observation index 0 is for Jan 1 in the year 0000 (used for uniform 365-day years)
                                 // E by itself makes m_dayNumberOfObsFor1900Jan1 dependent on pEnvContext->m_maxDaysInYear
                                 pOutput->m_dayNumberOfObsFor1900Jan1 = pEnvContext->m_maxDaysInYear == 365 ? (1900 * 365) : 0;
                                 if (strlen(format) > 1)
                                 {
                                    if (!isdigit(format[1]))
                                    {
                                       CString msg;
                                       msg.Format("FlowModel::LoadXml() When reading <output> tag for %s, observation format %s cannot be interpreted as 'E' followed by an integer (the integer would represent the day number in the "
                                          "observation file of an observation on Jan 1, 1900).", name, format);
                                       Report::ErrorMsg(msg);
                                       pOutput->m_inUse = false;
                                    }
                                    pOutput->m_dayNumberOfObsFor1900Jan1 = atoi(format + 1);
                                 }
                                 rows = pOutput->m_pDataObjObs->ReadAscii(observations_path);
                                 break;
                              default:
                              {
                                 CString msg("Flow: Unrecognized observation 'format' attribute '");
                                 msg += type;
                                 msg += "' reading <output> tag for '";
                                 msg += name;
                                 msg += "'. This output will be ignored...";
                                 Report::ErrorMsg(msg);
                                 pOutput->m_inUse = false;
                              }
                           }

                           if (rows <= 0)
                           {
                              CString msg;
                              msg.Format("Flow: Unable to load observation file '%s' specified for Model Output '%s'", (LPCTSTR)observations_path, name);
                              Report::WarningMsg(msg);

                              delete pOutput->m_pDataObjObs;
                              pOutput->m_pDataObjObs = NULL;
                              pOutput->m_inUse = false;
                           }

                           pModel->m_numQMeasurements++;
                        }
                     } // end of if (obs != NULL)

                     pOutput->m_modelType = MOT_SUM;
                     if (type != NULL)
                     {
                        switch (type[0])
                        {
                           case 'S':
                           case 's':      pOutput->m_modelType = MOT_SUM;         break;
                           case 'A':
                           case 'a':      pOutput->m_modelType = MOT_AREAWTMEAN;  break;
                           case 'P':
                           case 'p':      pOutput->m_modelType = MOT_PCTAREA;     break;
                           default:
                           {
                              CString msg("Flow: Unrecognized 'type' attribute '");
                              msg += type;
                              msg += "' reading <output> tag for '";
                              msg += name;
                              msg += "'. This output will be ignored...";
                              Report::ErrorMsg(msg);
                              pOutput->m_inUse = false;
                           }
                        }
                     } // end of if (type != NULL)

                     pOutput->m_modelDomain = MOD_IDU;
                     if (!domain.IsEmpty())
                     {
                        if (domain.CompareNoCase("idu") == 0) pOutput->m_modelDomain = MOD_IDU;
                        else if (domain.CompareNoCase("hru") == 0) pOutput->m_modelDomain = MOD_HRU;
                        else if (domain.CompareNoCase("reach") == 0) pOutput->m_modelDomain = MOD_REACH;
                        else
                        {
                           CString msg("Flow: Unrecognized 'domain' attribute '");
                           msg += domain;
                           msg += "' reading <output> tag for '";
                           msg += name;
                           msg += "'. This output will be ignored...";
                           Report::ErrorMsg(msg);
                           pOutput->m_inUse = false;
                        }
                     } // end of if ( !domain.IsEmpty() )

                     if (expr != NULL)
                     {
                        pOutput->m_exprStr = expr;

                        // Do this next statement to ensure that the expression string can never be exactly
                        // the same as a variable name.  Without this statement, an expression consisting of
                        // a single variable name is not recognized as an error when the variable name is not 
                        // a column name.  For example, without this statement, value="NOT_A_COL" would not
                        // be recognized as uninterpretable.
                        pOutput->m_exprStr = "(" + pOutput->m_exprStr + ")";
                     } // end of if ( expr != NULL )

                     pOutput->InitModelOutput(pIDULayer);
                     bool pass = true;
                     switch (pOutput->m_modelDomain)
                     {
                        default:
                        case MOD_IDU:
                           pass = pIDULayer != NULL && pOutput->InitIDUdomain(pIDULayer);
                           break;
                        case MOD_REACH:
                           pass = pReachLayer != NULL && pOutput->InitReachDomain(pReachLayer);
                           break;
                        case MOD_HRU:
                           pass = pHRUlayer != NULL && pOutput->InitHRUdomain(pHRUlayer);
                           break;
                     }

                     if (pOutput->m_inUse && pass)
                        pGroup->Add(pOutput);
                     else
                        delete pOutput;
                  }

                  pXmlModelOutput = pXmlModelOutput->NextSiblingElement("output");
               } // end of loop to read <output> blocks

               gpModel->m_modelOutputGroupArray.Add(pGroup);
               first_interval = false;
            } // end of if (first_interval)
            else
            { // Replicate the output group with a different interval.
               int size_of_previous_group = (int)pGroup->GetSize();

               ModelOutputGroup* pReplicated_group = new ModelOutputGroup;
               pReplicated_group->m_name = group_name_with_interval;
               pReplicated_group->m_moInterval = interval;
               pReplicated_group->m_moCountIdu = pGroup->m_moCountIdu;
               pReplicated_group->m_moCountHru = pGroup->m_moCountHru;
               pReplicated_group->m_moCountHruLayer = pGroup->m_moCountHruLayer;
               pReplicated_group->m_moCountReach = pGroup->m_moCountReach;
               for (int output_ndx = 0; output_ndx < size_of_previous_group; output_ndx++)
               {
                  ModelOutput* pOriginal_output = pGroup->GetAt(output_ndx);
                  ModelOutput* pReplicated_output = new ModelOutput;
                  *(pReplicated_output) = *(pGroup->GetAt(output_ndx));

                  if (pOriginal_output->m_pDataObjObs != NULL )
                  {
                     pReplicated_output->m_pDataObjObs = new FDataObj;
                     pReplicated_output->m_nameObs = pOriginal_output->m_nameObs;
                     bool path_ok = (PathManager::FindPath(pReplicated_output->m_nameObs, observations_path) >= 0); ASSERT(path_ok);
                     int rows = pReplicated_output->m_pDataObjObs->ReadAscii(observations_path);
                     ASSERT(rows > 0);
                  }

                  pReplicated_output->InitModelOutput(pIDULayer);
                  bool pass = true;
                  switch (pReplicated_output->m_modelDomain)
                  {
                     default:
                     case MOD_IDU:
                        pass = pIDULayer != NULL && pReplicated_output->InitIDUdomain(pIDULayer);
                        break;
                     case MOD_REACH:
                        pass = pReachLayer != NULL && pReplicated_output->InitReachDomain(pReachLayer);
                        break;
                     case MOD_HRU:
                        pass = pHRUlayer != NULL && pReplicated_output->InitHRUdomain(pHRUlayer);
                        break;
                  } // end of (pReplicated_output->m_modelDomain)

                  if (pReplicated_output->m_inUse && pass)
                     pReplicated_group->Add(pReplicated_output);
                  else
                     delete pReplicated_output;
               } // end of loop through outputs in the output group

               gpModel->m_modelOutputGroupArray.Add(pReplicated_group);
            } // end of if (first_interval) ... else

         } // end of while (multiple_intervals != 0)

         pXmlGroup = pXmlGroup->NextSiblingElement("output_group");
      } // end of if (output_groups_ok && pXmlGroup != NULL)
   } // end of if ( pXmlModelOutputs != NULL ) 

   // video capture
   TiXmlElement *pXmlVideoCapture = pXmlRoot->FirstChildElement( "video_capture" ); 
   if ( pXmlVideoCapture != NULL )
      {
      //pModel->m_useRecorder = true;
      pModel->m_useRecorder = true;
      int frameRate = 30;

      XML_ATTR attrs[] = {
            // attr           type         address                 isReq checkCol
            { "use",          TYPE_BOOL,   &pModel->m_useRecorder, false,  0 },
            { "frameRate",    TYPE_INT,    &frameRate,             false,  0 },
            { NULL,           TYPE_NULL,   NULL,                   false,  0 } };

      bool ok = TiXmlGetAttributes( pXmlVideoCapture, attrs, filename );

      if ( !ok )
         {
         CString msg; 
         msg.Format( _T("Flow: Misformed element reading <video_capture> attributes in input file %s"), filename );
         Report::ErrorMsg( msg );
         }
      else
         {
         //pModel->m_pVideoRecorder = new VideoRecorder;
   
         TiXmlElement *pXmlMovie = pXmlVideoCapture->FirstChildElement( "movie" ); 

         while( pXmlMovie != NULL )
            {
            LPTSTR field = NULL;
            LPTSTR file = NULL;

            XML_ATTR attrs[] = {
               // attr      type            address   isReq checkCol
               { "col",     TYPE_STRING,   &field,    true,   1 },
               { "file",    TYPE_STRING,   &file,     true,   0 },
               { NULL,      TYPE_NULL,      NULL,     false,  0 } };

            bool ok = TiXmlGetAttributes( pXmlMovie, attrs, filename, pIDULayer );

            if ( !ok )
               {
               CString msg; 
               msg.Format( _T("Flow: Misformed element reading <movie> attributes in input file %s"), filename );
               Report::ErrorMsg( msg );
               }
            else
               {
               int col = pIDULayer->GetFieldCol( field );

               if ( col >= 0 )
                  {
                  int vrID = EnvAddVideoRecorder( /*VRT_MAPPANEL*/ 2, "Flow Results", file, 30, VRM_CALLDIRECT, col );
                  pModel->m_vrIDArray.Add( vrID );
                  }
               }

            pXmlMovie = pXmlMovie->NextSiblingElement( "movie" );
            }
         }
      }
   
   return true;
   }


bool FlowModel::ParseFluxLocation( LPCTSTR typeStr, FLUXLOCATION &type, int &layer )
   {
   if ( *typeStr == 'h' ) // hruLayer
      {
      type = FL_HRULAYER;
      LPCTSTR colon = _tcschr( typeStr, ':' );

      if ( colon )
         {
         if ( *(colon+1) == '*' )
            layer = -1;
         else
            layer = atoi( colon+1 );
         }
      else
         layer = -1;  // applies to all layers
      }
         
   else if ( lstrcmpi( typeStr, "reach" ) == 0 )
      type = FL_REACH;

   else if ( lstrcmpi( typeStr, "reservoir" ) == 0 )
      type = FL_RESERVOIR;

   else if ( lstrcmpi( typeStr, "gw" ) == 0 )
      type = FL_GROUNDWATER;

   else
      return false;

   return true;
   }
         


int FlowModel::IdentifyMeasuredReaches(void)
   {
   int count = 0;

   //for ( MapLayer::Iterator i=this->m_pStreamLayer->Begin(); i < m_pStreamLayer->End(); i++ )
   for ( int i=0; i < (int) m_reachArray.GetSize(); i++ )
       {
       Reach *pReach = m_reachArray[ i ];

       int polyIndex = pReach->m_polyIndex;

       for ( int k=0; k < (int) this->m_modelOutputGroupArray.GetSize(); k++ )
         {
         ModelOutputGroup *pGroup = m_modelOutputGroupArray[ k ];

         for ( int j=0; j < (int) pGroup->GetSize(); j++ )
            {
            ModelOutput *pOutput = pGroup->GetAt( j );

            if ( pOutput->m_modelDomain == MOD_REACH && pOutput->m_pQuery != NULL )
               {
               // does it pass the query?
               bool result = false;
               bool ok = pOutput->m_pQuery->Run( polyIndex, result );

            if (ok && result)
               {
              pReach->m_isMeasured = true;
              count++;
               }
               }
            }
         }
      }

   return count;
   }



int FlowModel::ParseHRULayerDetails(CString &aggCols, int detail)
   {
   CStringArray tokens;
   int count = ::Tokenize( aggCols, ":", tokens);

   for( int i=0; i < count; i++ )
      {
      switch( detail)
         {
         case 0:
            m_hruLayerNames.Add( tokens[ i ] );
            break;
         case 1:
            m_hruLayerDepths.Add( tokens[ i ] );
            break;
         case 2:
            m_initWaterContent.Add( tokens[ i ] );
            break;

         case 4:
            break;
         }
      }

   return count;
   }


bool FlowModel::InitializeParameterEstimationSampleArray(void)
   {
   int moCount = 0;//number of outputs with measured time series
   for ( int j=0; j < (int) m_modelOutputGroupArray.GetSize(); j++ )
      {
      ModelOutputGroup *pGroup = m_modelOutputGroupArray[ j ];   
      for ( int i=0; i < (int) pGroup->GetSize(); i++ )
         {
         if ( pGroup->GetAt( i )->m_pDataObjObs!=NULL)
            {
            moCount++;
            FDataObj *pErrorStatData = new FDataObj(4,0);
           // FDataObj *pParameterData = new FDataObj((int)m_parameterArray.GetSize(),0);
            FDataObj *pDischargeData = new FDataObj(1,1);
           // m_mcOutputTables.Add(pParameterData);
            m_mcOutputTables.Add(pErrorStatData);
            m_mcOutputTables.Add(pDischargeData);
            }
         }
      }
   m_pParameterData = new FDataObj((int)m_parameterArray.GetSize()+1,0);
   m_pParameterData->SetLabel( 0, "RunNumber"  );
   for ( int j=0; j < (int) m_parameterArray.GetSize(); j++)
      {
      CString name;
      name.Format("%s:%s", (LPCTSTR) m_parameterArray.GetAt(j)->m_table, (LPCTSTR) m_parameterArray.GetAt(j)->m_name);
      m_pParameterData->SetLabel( j+1, name  );
      }
   /*
   m_pErrorStatData = new FDataObj((int)m_reachMeasuredDataArray.GetSize()*2,0);
   m_pParameterData = new FDataObj((int)m_parameterArray.GetSize(),0);
   m_pDischargeData = new FDataObj((int)m_pReachDischargeData->GetRowCount(),1);
 
   CString msg;
   int meas=0;
   for ( int j=0; j < (int) m_reachSampleLocationArray.GetSize(); j++)
      {
      if ( m_reachSampleLocationArray.GetAt(j)->m_pMeasuredData != NULL )
         {
         m_pErrorStatData->SetLabel( meas, "NS_"+m_reachSampleLocationArray.GetAt(j)->m_name  );
         meas++;
         m_pErrorStatData->SetLabel( meas, "NS_LN_"+m_reachSampleLocationArray.GetAt(j)->m_name  );
         meas++;
         }
      }
   
   for ( int j=0; j < (int) m_parameterArray.GetSize(); j++)
         m_pParameterData->SetLabel( j, m_parameterArray.GetAt(j)->m_name  );
         */
   return true;
   }

   /*
bool FlowModel::InitializeHRULayerSampleArray(void)
   {
   int numLocations = (int) m_reachSampleLocationArray.GetSize()*(m_soilLayerCount+m_vegLayerCount);
   m_pHRUPrecipData = new FDataObj( int( m_reachSampleLocationArray.GetSize()) +1  , 0 );  
   m_pHRUPrecipData->SetLabel( 0, "Time (Days)" );

   m_pHRUETData = new FDataObj( int( m_reachSampleLocationArray.GetSize() )+1, 0 );  
   m_pHRUETData->SetLabel( 0, "Time (Days)" );
   CString label;

   // for each sample location, allocate and label an FDataObj for storing HRULayer water contents
   for (int i=0; i < (int) m_reachSampleLocationArray.GetSize();i++)
      {
      FDataObj *pHruLayerData = new FDataObj(m_soilLayerCount+m_vegLayerCount+1,0);
      pHruLayerData->SetLabel( 0, "Time (Days)" );

      for (int j=0; j < m_soilLayerCount+m_vegLayerCount; j++)
        {
        label.Format( "%s", (LPCTSTR) m_hruLayerNames.GetAt(j));
        pHruLayerData->SetLabel( j+1, (char*)(LPCSTR) label );
        }
      m_reachHruLayerDataArray.Add( pHruLayerData );
      }


   //next look for any extra state variables
   for (int l=0;l<m_hruSvCount-1;l++)
      {
      // for each sample location, allocate and label an FDataObj for storing HRULayer water contents
      for (int i=0; i < (int) m_reachSampleLocationArray.GetSize();i++)
         {
         FDataObj *pHruLayerData = new FDataObj(m_soilLayerCount+m_vegLayerCount+1,0);
         pHruLayerData->SetLabel( 0, "Time (Days)" );

         for (int j=0; j < m_soilLayerCount+m_vegLayerCount; j++)
            {
            label.Format( "%s", (LPCTSTR) m_hruLayerNames.GetAt(j));
            pHruLayerData->SetLabel( j+1, (char*)(LPCSTR) label );
            }
         m_hruLayerExtraSVDataArray.Add( pHruLayerData );
         }
      }


   // next, initialize the m_pHRUPrecipData object.  It contains a column for precipitation at each sample location,
   int count=0;
   for (int i=0; i < (int)m_reachSampleLocationArray.GetSize(); i++)
      {
     // for (int j=0; j < m_soilLayerCount+m_vegLayerCount; j++)
       //  { 
         ReachSampleLocation *pSampleLocation = m_reachSampleLocationArray.GetAt(i);
         label.Format( "%s", (LPCTSTR) pSampleLocation->m_name, i );
         m_pHRUPrecipData->SetLabel( count+1, label );
         m_pHRUETData->SetLabel( count+1, label );
         count++;
     //    }
      }

   for (int i=0; i < (int)m_reachSampleLocationArray.GetSize(); i++)
      {
      ReachSampleLocation *pSampleLocation = m_reachSampleLocationArray.GetAt(i);
      if ( pSampleLocation->m_pMeasuredData != NULL)
         {
         FDataObj *pData = new FDataObj(3,0);
         pData->SetLabel( 0, "Time (Days)" );
         label.Format( "%s Mod", pSampleLocation->m_name );
         pData->SetLabel( 1, label );
         label.Format( "%s Meas", pSampleLocation->m_name );
         pData->SetLabel( 2, label );
         m_reachMeasuredDataArray.Add( pData );
         }
      }
   
   int numFound=0;
   for ( INT_PTR j=0; j < m_reachArray.GetSize(); j++ )
      {
      Reach *pReach = m_reachArray[j];
      for (int i=0;i<(int)m_reachSampleLocationArray.GetSize();i++)
         {
        // for (int k=0;k<m_soilLayerCount+m_vegLayerCount;k++)
          //  {
            ReachSampleLocation *pSam = m_reachSampleLocationArray.GetAt(i);
            if ( pReach->m_reachID == pSam->m_id )
               {
               if ( pReach->m_catchmentArray.GetSize() > 0 )
                  {
                  numFound++;
                  pSam->m_pHRU = pReach->m_catchmentArray[ 0 ]->GetHRU( 0 );
                  }
               break;
               }
          //  }

         }
      //next look for gridded locations

      for (int i=0;i<(int)m_reachSampleLocationArray.GetSize();i++)
         {
         ReachSampleLocation *pSam = m_reachSampleLocationArray.GetAt(i);
         for ( INT_PTR j=0; j < m_catchmentArray.GetSize(); j++ )
            {
            Catchment *pCatch = m_catchmentArray[j];
            for (int k=0;k<pCatch->GetHRUCount();k++)
               {
               HRU *pHRU=pCatch->GetHRU(k);
               if ( pHRU->m_climateRow == pSam->row && pHRU->m_climateCol == pSam->col  )
                  {
                  pSam->m_pHRU = pHRU;                    
                  break;                
                  }
               }
            }
         }
      }
   bool ret = false;
   if (numFound==numLocations)
      ret = true;
   return ret;
   }
   */
  
void FlowModel::CollectData( int dayOfYear )
   {
   // reservoirs
   for (int i = 0; i < (int) this->m_reservoirArray.GetSize(); i++)
      {
      Reservoir *pRes = m_reservoirArray.GetAt(i);

      ASSERT(pRes->m_pResData != NULL);

      if (pRes->m_reservoirType == ResType_CtrlPointControl)
      {
         float *data = new float[4];
         data[0] = (float)m_timeInRun;
         data[1] = pRes->GetPoolElevationFromVolume();
         data[2] = (float)pRes->m_inflow / SEC_PER_DAY;
         data[3] = (float)pRes->m_outflow / SEC_PER_DAY;

         pRes->m_pResData->AppendRow(data, 4);

         delete[] data;
      }
      else
      {
         float *data = new float[14];
         data[0] = (float)m_timeInRun;
         data[1] = pRes->GetPoolElevationFromVolume();
         data[2] = pRes->GetTargetElevationFromRuleCurve(dayOfYear);
         data[3] = (float)pRes->m_inflow / SEC_PER_DAY;
         data[4] = (float)pRes->m_outflow / SEC_PER_DAY;
         data[5] = pRes->m_powerFlow;
         data[6] = pRes->m_RO_flow;
         data[7] = pRes->m_spillwayFlow;
		 data[8] = (float)pRes->m_zone;
		 data[9] = (float)pRes->m_daysInZoneBuffer;
		 data[10] = pRes->GetBufferZoneElevation(dayOfYear);
		 data[11] = gpModel->m_waterYearType;
		 data[12] = pRes->m_constraintValue;
       data[13] = (float)pRes->m_volume;

         pRes->m_pResData->AppendRow(data, 14);

         delete[] data;
      }

    if (pRes->m_pResSimRuleOutput != NULL)
         {       
         VData vtime = m_timeInRun;
         vtime.ChangeType(TYPE_INT);
         VData *Rules = new VData[ 7 ]; 

         VData activerule; 
         activerule.ChangeType(TYPE_CSTRING);
         activerule = pRes->m_activeRule;

         VData blank;
         blank.ChangeType(TYPE_CSTRING);
         blank = "none";

         VData zone;
         zone.ChangeType(TYPE_INT);
         zone = pRes->m_zone;

         VData outflow;
         outflow.ChangeType(TYPE_FLOAT);
         outflow = pRes->m_outflow / SEC_PER_DAY;

         VData inflow;
         inflow.ChangeType(TYPE_FLOAT);
         inflow = pRes->m_inflow / SEC_PER_DAY;

         VData elevation;
         elevation.ChangeType(TYPE_FLOAT);
         elevation = pRes->m_elevation;

         Rules[0] = vtime;
         Rules[1] = blank;// ressimrule;
         Rules[2] = inflow;
         Rules[3] = outflow;
         Rules[4] = elevation;
         Rules[5]  = zone;
         Rules[6] = activerule;
     
         pRes->m_pResSimRuleCompare->AppendRow(Rules, 7); 
         delete[] Rules;
         }  //end of if (pRes->m_pResSimRuleOutput != NULL)
      }   //end of  for ( int i=0; i < (int) this->m_reservoirArray.GetSize(); i++ )
   }
   

// called daily (or whatever timestep FLOW is running at)
void FlowModel::UpdateHRULevelVariables(EnvContext *pEnvContext)
   {
   m_pCatchmentLayer->m_readOnly=false;

//   #pragma omp parallel for 
   for ( int i=0; i < (int) this->m_hruArray.GetSize(); i++ )
      {
      HRU *pHRU = m_hruArray[ i ];

      // climate
      GetTodaysHRUclimate( CDT_PRECIP,pHRU, pHRU->m_currentPrecip);
      GetTodaysHRUclimate( CDT_TMEAN, pHRU, pHRU->m_currentAirTemp);
      GetTodaysHRUclimate(CDT_TMIN, pHRU, pHRU->m_currentMinTemp);
      
      pHRU->m_precip_yr      += pHRU->m_currentPrecip;         // mm
      pHRU->m_precip_wateryr += pHRU->m_currentPrecip;         // mm
      pHRU->m_rainfall_yr    += pHRU->m_rainThrufall_mm;       // mm
      pHRU->m_snowfall_yr    += pHRU->m_snowThrufall_mm;       // mm
      pHRU->m_temp_yr        += pHRU->m_currentAirTemp;
      pHRU->m_gwRecharge_yr  += pHRU->m_currentGWRecharge;       // mm
      pHRU->m_gwFlowOut_yr   += pHRU->m_currentGWFlowOut;

      // hydrology
      pHRU->m_depthMelt = float( pHRU->GetLayer(BOX_MELT)->m_volumeWater/pHRU->m_HRUeffArea_m2);  // volume of water in snow, expressed as a depth
      pHRU->m_depthSWE  = float( pHRU->GetLayer(BOX_SNOWPACK)->m_volumeWater/pHRU->m_HRUeffArea_m2 + pHRU->m_depthMelt );   // volume of ice in snow in SWE, expressed as a depth

      pHRU->m_maxET_yr  += pHRU->m_currentMaxET;
      pHRU->m_et_yr     += pHRU->m_currentET;
      pHRU->m_aquifer_recharge_yr_mm += pHRU->m_aquifer_recharge_mm;
      pHRU->m_runoff_yr += pHRU->m_currentRunoff;

      if (pEnvContext->yearOfRun == 0 && m_flowContext.dayOfYear == 0)
         {
            for (int k = 0; k < pHRU->m_polyIndexArray.GetSize(); k++)
            {
               int idu = pHRU->m_polyIndexArray[k];
               gpFlow->UpdateIDU(pEnvContext, idu, m_colCLIMATENDX, pHRU->m_climateIndex, false);  
            } // end of loop thru IDUs in this HRU
         } // end of if (pEnvContext->yearOfRun == 0 && m_flowContext.dayOfYear == 0)

      } // end of loop thru HRUs

   m_pCatchmentLayer->m_readOnly = true;

   int hruCount = GetHRUCount();
   for (int hru_ndx = 0; hru_ndx < hruCount; hru_ndx++)
      {
      HRU *pHRU = GetHRU(hru_ndx);
      float area_m2 = pHRU->m_HRUeffArea_m2;
      float irrigated_area_m2 = area_m2 * pHRU->GetLayer(BOX_IRRIG_SOIL)->m_HRUareaFraction;
      float unirrigated_area_m2 = area_m2 - irrigated_area_m2;

      float sm2atm = 0.f; m_pHRUlayer->GetData(hru_ndx, m_colHruSM2ATM, sm2atm);
      m_pHRUlayer->SetDataU(hru_ndx, m_colHruET_DAY, sm2atm + pHRU->m_currentET);
      pHRU->m_et_yr += sm2atm;

      HRULayer* pBOX_SURFACE_H2O = pHRU->GetLayer(BOX_SURFACE_H2O);
      double box_surface_h2o_flux_m3 = pBOX_SURFACE_H2O->m_globalHandlerFluxValue;
      HRULayer* pBOX_NAT_SOIL = pHRU->GetLayer(BOX_NAT_SOIL);
      double box_nat_soil_flux_m3 = pBOX_NAT_SOIL->m_globalHandlerFluxValue;
      double box_surface_h2o_adjustment_m3 = 0.;
      if (!CheckSurfaceH2O(pHRU, box_surface_h2o_adjustment_m3))
      {
         CString msg;
         msg.Format("UpdateHRULevelVariables() For pHRU->m_id = %d, CheckSurfaceH2O() returned false.", pHRU->m_id);
         Report::LogWarning(msg);
      }

      HRULayer * pNatSoilBox = pHRU->GetLayer(BOX_NAT_SOIL);
      m_pHRUlayer->SetDataU(hru_ndx, m_colHruNAT_SOIL, unirrigated_area_m2 > 0.f ? ((pNatSoilBox->m_volumeWater / unirrigated_area_m2) * 1000.f) : 0.f);
      HRULayer * pIrrigSoilBox = pHRU->GetLayer(BOX_IRRIG_SOIL);
      m_pHRUlayer->SetDataU(hru_ndx, m_colHruIRRIG_SOIL, irrigated_area_m2 > 0.f ? ((pIrrigSoilBox->m_volumeWater / irrigated_area_m2) * 1000.f) : 0.f);
      HRULayer * pGWfastBox = pHRU->GetLayer(BOX_FAST_GW);
      m_pHRUlayer->SetDataU(hru_ndx, m_colHruGW_FASTBOX, (pGWfastBox->m_volumeWater / area_m2) * 1000.f);
      HRULayer * pGWslowBox = pHRU->GetLayer(BOX_SLOW_GW);
      m_pHRUlayer->SetDataU(hru_ndx, m_colHruGW_SLOWBOX, (pGWslowBox->m_volumeWater / area_m2) * 1000.f);
      }

   } // end of UpdateHRULevelVariables()


bool FlowModel::GetHRUData( HRU *pHRU, int col, VData &value, DAMETHOD method )
   {
   if ( method == DAM_FIRST )
      {
      if ( pHRU->m_polyIndexArray.IsEmpty() )
         return false;

      return m_pCatchmentLayer->GetData( pHRU->m_polyIndexArray[0], col, value );
      }
   else
      {
      DataAggregator da( m_pCatchmentLayer );
      da.SetPolys( (int*)(pHRU->m_polyIndexArray.GetData()),
                   (int ) pHRU->m_polyIndexArray.GetSize() );
      da.SetAreaCol( m_colCatchmentArea );
      return da.GetData(col, value, method );
      }
   }


bool FlowModel::GetHRUData( HRU *pHRU, int col, bool  &value, DAMETHOD method )
   {
   VData _value;
   if ( GetHRUData( pHRU, col, _value, method ) == false )
      return false;

   return _value.GetAsBool( value );
   }


bool FlowModel::GetHRUData( HRU *pHRU, int col, int  &value, DAMETHOD method )
   {
   VData _value;
   if ( GetHRUData( pHRU, col, _value, method ) == false )
      return false;

   return _value.GetAsInt( value );
   }


bool FlowModel::GetHRUData( HRU *pHRU, int col, float &value, DAMETHOD method )
   {
   VData _value;
   if ( GetHRUData( pHRU, col, _value, method ) == false )
      return false;

   return _value.GetAsFloat( value );
   }



bool FlowModel::GetCatchmentData( Catchment *pCatchment, int col, VData &value, DAMETHOD method )
   {
   if ( method == DAM_FIRST )
      {
      HRU *pHRU = pCatchment->GetHRUfromCatchment( 0 );
      if ( pHRU == NULL || pHRU->m_polyIndexArray.IsEmpty() )
         return false;

      return m_pCatchmentLayer->GetData( pHRU->m_polyIndexArray[0], col, value );
      }
   else
      {
      DataAggregator da( m_pCatchmentLayer );

      int hruCount = pCatchment->GetHRUCountInCatchment();
      for ( int i=0; i < hruCount; i++ )
         {
         HRU *pHRU = pCatchment->GetHRUfromCatchment( i );

         da.AddPolys( (int*)(pHRU->m_polyIndexArray.GetData()),
                      (int ) pHRU->m_polyIndexArray.GetSize() );
         }

      da.SetAreaCol( m_colCatchmentArea );
      return da.GetData(col, value, method );
      }
   }


bool FlowModel::GetCatchmentData( Catchment *pCatchment, int col, int &value, DAMETHOD method )
   {
   VData _value;
   if ( GetCatchmentData( pCatchment, col, _value, method ) == false )
      return false;

   return _value.GetAsInt( value );
   }


bool FlowModel::GetCatchmentData( Catchment *pCatchment, int col, bool &value, DAMETHOD method )
   {
   VData _value;
   if ( GetCatchmentData( pCatchment, col, _value, method ) == false )
      return false;

   return _value.GetAsBool( value );
   }


bool FlowModel::GetCatchmentData( Catchment *pCatchment, int col, float &value, DAMETHOD method )
   {
   VData _value;
   if ( GetCatchmentData( pCatchment, col, _value, method ) == false )
      return false;

   return _value.GetAsFloat( value );
   }



bool FlowModel::SetCatchmentData( Catchment *pCatchment, int col, VData value )
   {
   if ( col < 0 )
      return false;

   for ( int i=0; i < pCatchment->GetHRUCountInCatchment(); i++ )
      {
      HRU *pHRU = pCatchment->GetHRUfromCatchment( i );
      
      for ( int j=0; j < (int) pHRU->m_polyIndexArray.GetSize(); j++ )
         m_pCatchmentLayer->SetData( pHRU->m_polyIndexArray[ j ], col, value );
      }
   return true;
   }


bool FlowModel::GetReachData( Reach *pReach, int col, VData &value )
   {
   int record = pReach->m_polyIndex;
   return m_pStreamLayer->GetData( record, col, value );
   }

bool FlowModel::GetReachData( Reach *pReach, int col, bool &value )
   {
   int record = pReach->m_polyIndex;
   return m_pStreamLayer->GetData( record, col, value );
   }

 
bool FlowModel::GetReachData( Reach *pReach, int col, int &value )
   {
   int record = pReach->m_polyIndex;
   return m_pStreamLayer->GetData( record, col, value );
   }

bool FlowModel::GetReachData( Reach *pReach, int col, float &value )
   {
   int record = pReach->m_polyIndex;
   return m_pStreamLayer->GetData( record, col, value );
   }


bool FlowModel::MoveFlux( Flux *pFlux, FluxContainer *pStartContainer, FluxContainer *pEndContainer )
   {
   // first, remove the flux from the start layer
   int index = pStartContainer->RemoveFlux( pFlux, false );
   
   if ( index < 0 )
      return false;

   return pEndContainer->AddFlux( pFlux ) >= 0 ? true : false;
   }


float FlowModel::GetObjectiveFunction(FDataObj *pData, float &ns, float &nsLN, float &ve)
   {
   int meas=0;      
   int offset=0;
   int n = pData->GetRowCount();
   float obsMean=0.0f, predMean=0.0f,obsMeanL=0.0f,predMeanL=0.0f;
   int firstSample = 200;
   for (int j=firstSample;j<n;j++) //50this skips the first 50 days. Assuming those values
      // might be unacceptable due to initial conditions...
      {
      float time = 0.0f; float obs=0.0f; float pred=0.0f;
      pData->Get(0, j,time);
      pData->Get(2, j,obs); //add up all the discharge values and then...
      obsMean+=obs;
      obsMeanL+=log10(obs);
      pData->Get(1,j,pred);
      predMean+=pred;
      predMeanL+=log10(pred);
      }
   obsMean = obsMean/((float)n-(float)firstSample); // divide them by the sample size to generate the average.
   //predMean = predMean/((float)n-(float)firstSample); // divide them by the sample size to generate the average.
   obsMeanL=obsMeanL/((float)n-(float)firstSample);
   //predMeanL=predMeanL/((float)n-(float)firstSample);

   double errorSum = 0.0f, errorSumDenom = 0.0f, errorSumLN = 0.0f, errorSumDenomLN = 0.0f, r2Denom1 = 0.0f, r2Denom2 = 0.0f; float ve_numerator = 0.0f; float ve_denominator = 0.0f;
   for (int row=firstSample; row < n; row++ ) 
      {
      float pred =0.0f, obs=0.0f;float time=0.0f;
      pData->Get(0, row,time);
      pData->Get(2, row,obs);
      pData->Get(1,row,pred); // Assuming col i holds data for the reach corresponding to measurement

      errorSum+=((obs-pred)*(obs-pred));
      errorSumDenom+=((obs-obsMean)*(obs-obsMean));

      ve_numerator += (obs - pred);
      ve_denominator += obs;
      
      double a=log10(obs)-log10(pred);
      double b=log10(obs)-obsMeanL;
     // double b = log10(obs) - log10(obsMean);
      errorSumLN+=pow(a,2);
      errorSumDenomLN+=pow(b,2);
      }

   if (errorSumDenom > 0.0f)
      {
      ns  = (float)(1.0f - (errorSum/errorSumDenom));
      nsLN  = (float)(1.0f - (errorSumLN/errorSumDenomLN));
      ve = (float)ve_numerator / ve_denominator;
      }
   return ns;
   }
  
void FlowModel::GetNashSutcliffe(float *ns)
   {
   int meas=0;      
   float _ns=1.0f; float _nsLN=1.0f;
   float *data = new float[ m_reachMeasuredDataArray.GetSize() ];
   int offset=0;
   for ( int l=0; l < (int) m_reachMeasuredDataArray.GetSize(); l++)
      {
      FDataObj *pData = m_reachMeasuredDataArray.GetAt(l);
      int n = pData->GetRowCount();
      float obsMean=0.0f, predMean=0.0f;
      int firstSample = 200;
      for (int j=firstSample;j<n;j++) //50this skips the first 50 days. Assuming those values
         // might be unacceptable due to initial conditions...
         {
         float time = pData->Get(0, j);
         obsMean += pData->Get(2, j); //add up all the discharge values and then...
         predMean += pData->Get(1,j);
         }
      obsMean = obsMean/((float)n-(float)firstSample); // divide them by the sample size to generate the average.
      predMean = predMean/((float)n-(float)firstSample); // divide them by the sample size to generate the average.

      double errorSum=0.0f, errorSumDenom=0.0f, errorSumLN=0.0f, errorSumDenomLN=0.0f, r2Denom1=0.0f, r2Denom2=0.0f;
      for (int row=firstSample; row < n; row++ ) 
         {
         float pred =0.0f, obs=0.0f;
         float time = pData->Get(0, row);
         obs = pData->Get(2, row);
         pData->Get(1,row,pred); // Assuming col i holds data for the reach corresponding to measurement

         errorSum+=((obs-pred)*(obs-pred));
         errorSumDenom+=((obs-obsMean)*(obs-obsMean));
         double a=log10(obs)-log10(pred);
         double b=log10(obs)-log10(obsMean);
         errorSumLN+=pow(a,2);
         errorSumDenomLN+=(b,2);
         }

      if (errorSumDenom > 0.0f)
         {
         _ns  = (float)(1.0f - (errorSum/errorSumDenom));
         _nsLN  = (float)(1.0f - (errorSumLN/errorSumDenomLN));
         }
      ns[offset] = _ns;
      offset++;
      ns[offset] = _nsLN; 
      offset++;
      }
   delete [] data;
   }


bool FlowModel::RestoreStateVariables(bool spinupFlag)
{
   bool OK_flag = spinupFlag ? InitializeSpinup() : ReadState();
   if (!OK_flag)
   {
      CString msg = "FlowModel::RestoreStateVariables() failed. If no initial conditions file is available, then you may have to run a spinup.";
      Report::ErrorMsg(msg);
      return(false);
   }

   bool hru_h2o_balance_flag = true;
   int hru_count = (int)m_hruArray.GetSize();
   for (int hru_ndx = 0; hru_ndx < hru_count; hru_ndx++)
   {
      HRU* pHRU = m_hruArray[hru_ndx];
      hru_h2o_balance_flag &= CheckHRUwaterBalance(pHRU);
   }

   if (!hru_h2o_balance_flag)
   {
      CString msg;
      msg.Format("RestoreStateVariables() HRU water balance failed.");
      Report::FatalMsg(msg);
   }

   return(hru_h2o_balance_flag);
} // end of RestoreStateVariables()


void FlowModel::GetTotalStorage(float &channel, float &terrestrial)
   {

   int catchmentCount = GetCatchmentCount();
   // iterate through catchments/hrus/hrulayers, calling fluxes as needed
   for ( int i=0; i < catchmentCount; i++ )
      {
      Catchment *pCatchment = GetCatchment(i);
      int hruCount = pCatchment->GetHRUCountInCatchment();
      for ( int h=0; h < hruCount; h++ )
         {
         HRU *pHRU = pCatchment->GetHRUfromCatchment( h );
         int hruLayerCount=pHRU->GetLayerCount();
         for ( int l=0; l < hruLayerCount; l++ )     
            {
            HRULayer *pHRULayer = pHRU->GetLayer( l );
            terrestrial+= float( pHRULayer->m_volumeWater );
            }
         }
      }

   int reachCount = (int) m_reachArray.GetSize();
   for ( int i=0; i < reachCount; i++ )
      {
      Reach *pReach = m_reachArray[ i ];
      channel+=(float)(pReach->m_reach_volume_m3);
      }
   }


bool FlowModel::InitPlugins( void )
   {
   //if (  m_pReachRouting->m_type = m_reachExtFnDLL )
   //   {
   //   FLOWINITFN initFn = (FLOWINITFN) ::GetProcAddress( m_reachExtFnDLL, "Init" );
   //
   //   if ( initFn != NULL )
   //      initFn( &m_flowContext, NULL );
   //   }

   //if (  m_latExchExtFnDLL )
   //   {
   //   FLOWINITFN initFn = (FLOWINITFN) ::GetProcAddress( m_latExchExtFnDLL, "Init" );
   //
   //   if ( initFn != NULL )
   //      initFn( &m_flowContext, NULL );
   //   }
   //
   //if (  m_hruVertFluxExtFnDLL )
   //   {
   //   FLOWINITFN initFn = (FLOWINITFN) ::GetProcAddress( m_hruVertFluxExtFnDLL, "Init" );
   //
   //   if ( initFn != NULL )
   //      initFn( &m_flowContext, NULL );
   //   }

   if (  m_extraSvRxnExtFnDLL )
      {
      FLOWINITFN initFn = (FLOWINITFN) ::GetProcAddress( m_extraSvRxnExtFnDLL, "Init" );

      if ( initFn != NULL )
         initFn( &m_flowContext, NULL );
      }

   if (  m_reservoirFluxExtFnDLL )
      {
      FLOWINITFN initFn = (FLOWINITFN) ::GetProcAddress( m_reservoirFluxExtFnDLL, "Init" );

      if ( initFn != NULL )
         initFn( &m_flowContext, NULL );
      }

   //if (  m_gwExtFnDLL )
   //   {
   //   FLOWINITFN initFn = (FLOWINITFN) ::GetProcAddress( m_gwExtFnDLL, "Init" );
   //
   //   if ( initFn != NULL )
   //      initFn( &m_flowContext, NULL );
   //   }

   //for ( INT_PTR i=0; i < this->m_extMethodArray.GetSize(); i++ )
   //   {
   //   ExternalMethod *pMethod = m_extMethodArray[ i ];
   //   pMethod->Init( &m_flowContext );
   //   }

   return true;
   }


bool FlowModel::InitRunPlugins( void )
   {
   //if (  m_reachExtFnDLL )  // TODO - this isn't currently supported!!!
   //   {
   //   FLOWINITRUNFN initFn = (FLOWINITRUNFN) ::GetProcAddress( m_reachExtFnDLL, "InitRun" );
   //
   //   if ( initFn != NULL )
   //      initFn( &m_flowContext );
   //   }

   //if (  m_latExchExtFnDLL )
   //   {
   //   FLOWINITRUNFN initFn = (FLOWINITRUNFN) ::GetProcAddress( m_latExchExtFnDLL, "InitRun" );
   //
   //   if ( initFn != NULL )
   //      initFn( &m_flowContext );
   //   }
   //
   //if (  m_hruVertFluxExtFnDLL )
   //   {
   //   FLOWINITRUNFN initFn = (FLOWINITRUNFN) ::GetProcAddress( m_hruVertFluxExtFnDLL, "InitRun" );
   //
   //   if ( initFn != NULL )
   //      initFn( &m_flowContext );
   //   }

   if (  m_extraSvRxnExtFnDLL )
      {
      FLOWINITRUNFN initFn = (FLOWINITRUNFN) ::GetProcAddress( m_extraSvRxnExtFnDLL, "InitRun" );

      if ( initFn != NULL )
         initFn( &m_flowContext );
      }

   if (  m_reservoirFluxExtFnDLL )
      {
      FLOWINITRUNFN initFn = (FLOWINITRUNFN) ::GetProcAddress( m_reservoirFluxExtFnDLL, "InitRun" );

      if ( initFn != NULL )
         initFn( &m_flowContext );
      }

   //if (  m_gwExtFnDLL )
   //   {
   //   FLOWINITRUNFN initFn = (FLOWINITRUNFN) ::GetProcAddress( m_gwExtFnDLL, "InitRun" );
   //
   //   if ( initFn != NULL )
   //      initFn( &m_flowContext );
   //   }   


   return true;
   }


bool FlowModel::EndRunPlugins( void )
   {
   //if (  m_reachExtFnDLL )
   //   {
   //   FLOWENDRUNFN initFn = (FLOWENDRUNFN) ::GetProcAddress( m_reachExtFnDLL, "EndRun" );
   //
   //   if ( initFn != NULL )
   //      initFn( &m_flowContext );
   //   }

   //if (  m_latExchExtFnDLL )
   //   {
   //   FLOWENDRUNFN initFn = (FLOWENDRUNFN) ::GetProcAddress( m_latExchExtFnDLL, "EndRun" );
   //
   //   if ( initFn != NULL )
   //      initFn( &m_flowContext );
   //   }
   //
   //if (  m_hruVertFluxExtFnDLL )
   //   {
   //   FLOWENDRUNFN initFn = (FLOWENDRUNFN) ::GetProcAddress( m_hruVertFluxExtFnDLL, "EndRun" );
   //
   //   if ( initFn != NULL )
   //      initFn( &m_flowContext );
   //   }

   if (  m_extraSvRxnExtFnDLL )
      {
      FLOWENDRUNFN initFn = (FLOWENDRUNFN) ::GetProcAddress( m_extraSvRxnExtFnDLL, "EndRun" );

      if ( initFn != NULL )
         initFn( &m_flowContext );
      }

   if (  m_reservoirFluxExtFnDLL )
      {
      FLOWENDRUNFN initFn = (FLOWENDRUNFN) ::GetProcAddress( m_reservoirFluxExtFnDLL, "EndRun" );

      if ( initFn != NULL )
         initFn( &m_flowContext );
      }

//   if (  m_gwExtFnDLL )
//      {
//      FLOWENDRUNFN initFn = (FLOWENDRUNFN) ::GetProcAddress( m_gwExtFnDLL, "EndRun" );
//
//      if ( initFn != NULL )
//         initFn( &m_flowContext );
//      }   

//   for ( INT_PTR i=0; i < this->m_extMethodArray.GetSize(); i++ )
//      {
//      ExternalMethod *pMethod = m_extMethodArray[ i ];
//      pMethod->EndRun( &m_flowContext );
//      }
//
   return true;
   }


int FlowModel::GetUpstreamCatchmentCount( ReachNode *pStartNode )
   {
   if ( pStartNode == NULL )
      return 0;

   int count = 0;
   if ( pStartNode->m_pLeft )
      {
      count += GetUpstreamCatchmentCount( pStartNode->m_pLeft );

      Reach *pReach = GetReachFromNode( pStartNode->m_pLeft );

      if ( pReach != NULL )
         count += (int) pReach->m_catchmentArray.GetSize();
      }

   if ( pStartNode->m_pRight)
      {
      count += GetUpstreamCatchmentCount( pStartNode->m_pRight );

      Reach *pReach = GetReachFromNode( pStartNode->m_pRight );

      if ( pReach != NULL )
         count += (int) pReach->m_catchmentArray.GetSize();
      }
      
   return count;
   }


/////////////////////////////////////////////////////////////////////////////////
// Climate methods
/////////////////////////////////////////////////////////////////////////////////


//void FlowModel::InitRunClimate( void )
//   {
//   for ( int i=0; i < (int) m_climateDataInfoArray.GetSize(); i++ )
//      {
//      ClimateDataInfo *pInfo = m_climateDataInfoArray[ i ];
//
//    //  if ( pInfo->m_pDataObj == NULL )
//         pInfo->m_pDataObj->InitLibraries();
//      }
//   }


bool chk_nc(int io_rtnval)
{
   if (io_rtnval == NC_NOERR) return(TRUE);

   CString msg;
   msg.Format("*** %s\n", nc_strerror(io_rtnval));
   Report::ErrorMsg(msg);

   return(FALSE);
} // end of chk_nc()


bool FlowModel::OpenClimateDataFiles(int tgtYear)
{
   ClimateScenario *pScenario = m_scenarioArray.GetAt(m_currentFlowScenarioIndex);
   for (int i = 0; i < (int)pScenario->m_climateInfoArray.GetSize(); i++)
   {
      ClimateDataInfo *pInfo = pScenario->m_climateInfoArray[i];
      if (pInfo->m_pDataObj == NULL) continue;

      if (pInfo->m_ncid >= 0)
      { // There is a file open.
         if ((pInfo->m_firstYear <= tgtYear) && (tgtYear <= pInfo->m_lastYear)) continue; // The open file is the one we want.
         nc_close(pInfo->m_ncid); // Close the one that's open, and open a new one.
      }

      CString filePathAndName = pInfo->m_path;
      // Is the .nc suffix already present?  If so, it is a multiyear file.
      bool multiyear_file = false;
      int pathAndName_length = filePathAndName.GetLength();
      if (pathAndName_length >= 4)
      {
         CString suffix = filePathAndName.Right(3);
         if (suffix == ".nc") multiyear_file = true;
      }

      if (!multiyear_file)
      {
         CString year_cstr;
         year_cstr.Format("_%i", tgtYear);
         filePathAndName = filePathAndName + year_cstr + ".nc";
         pInfo->m_firstYear = tgtYear;
         pInfo->m_lastYear = tgtYear;
      }

      // Open this input file.
      CString msg;
      msg.Format("Opening Climate file %s", filePathAndName.GetString());
      Report::LogMsg(msg);
      int rtnval = 0;
      pInfo->m_ncid = -1;
      pInfo->m_mostRecentIndexYear = -1; // A calendar year will go here
      pInfo->m_recentYearIndex = -1; // An index to a place in the file will go here
      rtnval = nc_open(filePathAndName, NC_NOWRITE, &pInfo->m_ncid);
      if (rtnval != NC_NOERR) Report::LogMsg(msg, RT_INFO);
      if (!chk_nc(rtnval)) return(false);

      pInfo->m_mostRecentIndexYear = pInfo->m_firstYear;
      pInfo->m_recentYearIndex = 0;

      pInfo->m_varid_data = -1;
      rtnval = nc_inq_varid(pInfo->m_ncid, pInfo->m_varName, &pInfo->m_varid_data); 
      if (!chk_nc(rtnval))
      {
         CString msg;
         msg.Format("OpenClimateDataFiles() File %s does not contain variable %s.", filePathAndName, pInfo->m_varName);
         Report::LogError(msg);
         return(false);
      }

      pInfo->m_nctype = NC_NAT;
      rtnval = nc_inq_vartype(pInfo->m_ncid, pInfo->m_varid_data, &pInfo->m_nctype); if (!chk_nc(rtnval)) return(false);
      if (pInfo->m_nctype != NC_FLOAT && pInfo->m_nctype != NC_SHORT)
      {
         CString msg;
         msg.Format("OpenClimateDataFiles() File %s, variable %s, has nctype = %d, which is neither NC_FLOAT nor NC_SHORT.", filePathAndName, pInfo->m_varName, pInfo->m_nctype);
         Report::LogError(msg);
         return(false);
      }

      float scale_factor = 1.f;
      rtnval = nc_get_att_float(pInfo->m_ncid, pInfo->m_varid_data, "scale_factor", &scale_factor);
      if (rtnval == NC_NOERR)
      {
         CString msg;
         msg.Format("OpenClimateDataFiles() File %s, variable %s, has scale_factor = %f", filePathAndName.GetString(), pInfo->m_varName, scale_factor);
         Report::LogMsg(msg);
      }
      else if (pInfo->m_firstYear < pInfo->m_lastYear)
      { // We only write this for multiyear files; single year files aren't expected to have scale factors.
         CString msg;
         msg.Format("OpenClimateDataFiles() Unable to read scale_factor attribute for file %s, variable %s.  Setting m_scaleFactor = 1.",
            filePathAndName, pInfo->m_varName);
         Report::LogMsg(msg);
         scale_factor = 1.f;
      }
      pInfo->m_scaleFactorAttribute = scale_factor;
      pInfo->m_scaleFactor = pInfo->m_scaleFactorAttribute;

      float add_offset_attribute = 0.f;
      rtnval = nc_get_att_float(pInfo->m_ncid, pInfo->m_varid_data, "add_offset", &add_offset_attribute);
      if (rtnval == NC_NOERR)
      {
         CString msg;
         msg.Format("OpenClimateDataFiles() File %s, variable %s, has 'add_offset' attribute = %f", filePathAndName.GetString(), pInfo->m_varName, add_offset_attribute);
         Report::LogMsg(msg);
      }
      pInfo->m_addOffsetAttribute = add_offset_attribute;
      // pInfo->m_offset is set in FlowModel::LoadXml() and is used for converting from input units to model units (e.g. from deg K to deg C)

      char units_attribute_str[81]; for (int i = 0; i < 81; i++) units_attribute_str[i] = 0;
      rtnval = nc_get_att_text(pInfo->m_ncid, pInfo->m_varid_data, "units", units_attribute_str);
      if (rtnval == NC_NOERR && strlen(units_attribute_str) > 0)
      {
         CString msg;
         msg.Format("OpenClimateDataFiles() File %s, variable %s, has 'units' attribute = %s", filePathAndName.GetString(), pInfo->m_varName, CString(units_attribute_str));
         Report::LogMsg(msg);

         if (pInfo->m_unitsAttribute.GetLength() > 0 && pInfo->m_unitsAttribute != units_attribute_str)
         {
            CString msg;
            msg.Format("OpenClimateDataFiles() Units mismatch in file %s, variable %s. Units specified in XML are %s. Units attribute in the NC file is %s.",
               filePathAndName.GetString(), pInfo->m_varName, pInfo->m_unitsAttribute.GetString(), units_attribute_str);
            Report::LogError(msg);
            return(false);
         }
         pInfo->m_unitsAttribute = units_attribute_str;
      }
      else if (pInfo->m_unitsAttribute.GetLength() > 0)
      {
         CString msg;
         msg.Format("OpenClimateDataFiles() XML specified units as %s, while the NC file does not have a units attribute.",
            pInfo->m_unitsAttribute.GetString());
         Report::LogMsg(msg);
      }

      // Is this netCDF file grid-based or polygon-based?
      // Does it have an "idu" dimension?
      pInfo->m_dimid_idu = -1;
      rtnval = nc_inq_dimid(pInfo->m_ncid, "idu", &pInfo->m_dimid_idu);
      pInfo->m_IDUbased = rtnval == NC_NOERR;

      if (!pInfo->m_IDUbased)
      { // grid-based
         pInfo->m_dimid_time = -1;
         rtnval = nc_inq_dimid(pInfo->m_ncid, "time", &pInfo->m_dimid_time); 
         if (rtnval != NC_NOERR)
         {
            rtnval = nc_inq_dimid(pInfo->m_ncid, "day", &pInfo->m_dimid_time); 
            if (!chk_nc(rtnval)) return(false);
         }

         pInfo->m_size_time = -1;
         rtnval = nc_inq_dimlen(pInfo->m_ncid, pInfo->m_dimid_time, &pInfo->m_size_time); if (!chk_nc(rtnval)) return(false);

         CString msg;
         msg.Format("For tgtYear = %d, opened Grid-based Climate file %s with %ld records", tgtYear, filePathAndName, pInfo->m_size_time);
         Report::LogMsg(msg, RT_INFO);
      }
      else
      { // IDU-based 
         CString msg;
         msg.Format("Opening IDU-based Climate file %s_%i.nc", (LPCTSTR)pInfo->m_path, tgtYear);
         Report::LogMsg(msg, RT_INFO);

         pInfo->m_varid_idu_id = -1;
         rtnval = nc_inq_varid(pInfo->m_ncid, "idu_id", &pInfo->m_varid_idu_id); if (!chk_nc(rtnval)) return(false);

         if (pInfo->m_nctype != NC_FLOAT)
         {
            CString msg; msg.Format("file = %s, vartype = %d vartype is not NC_FLOAT", filePathAndName, pInfo->m_nctype);
            Report::LogError(msg);
            return(false);
         }

         pInfo->m_size_idu = -1;
         rtnval = nc_inq_dimlen(pInfo->m_ncid, pInfo->m_dimid_idu, &pInfo->m_size_idu); if (!chk_nc(rtnval)) return(false);

         pInfo->m_dimid_time = -1;
         rtnval = nc_inq_dimid(pInfo->m_ncid, "time", &pInfo->m_dimid_time); if (!chk_nc(rtnval)) return(false);

         pInfo->m_size_time = -1;
         rtnval = nc_inq_dimlen(pInfo->m_ncid, pInfo->m_dimid_time, &pInfo->m_size_time); if (!chk_nc(rtnval)) return(false);
         if (pInfo->m_size_time != 360 && pInfo->m_size_time != 365 && pInfo->m_size_time != 366)
         {
            CString msg; msg.Format("file = %s, size_time = %d time dimension is none of 360, 365, 366", pInfo->m_size_time);
            Report::LogError(msg);
            return(false);
         }

      } // end of if (!pInfo->m_IDUbased) ... else ...

   } // end of for ( int i=0; i < (int) pScenario->m_climateInfoArray.GetSize(); i++ )

   return(true);
} // end of OpenClimateDataFiles()


  // called at the end of FlowModel::Run()
void FlowModel::CloseClimateData( void )
   {
   ClimateScenario *pScenario = m_scenarioArray[ m_currentFlowScenarioIndex ];

   for ( int i=0; i < (int) pScenario->m_climateInfoArray.GetSize(); i++ )
      {
      ClimateDataInfo *pInfo = pScenario->m_climateInfoArray[ i ];

      if ( pInfo->m_pDataObj != NULL )
         pInfo->m_pDataObj->Close();
      }
   }


void ClimateDataInfo::ScaleTheValues(float * fieldVals, size_t numVals) // Converts from the form of the data on disk to the form and units used by the model
{
   float net_offset = m_addOffsetAttribute + m_offset;

   if (m_scaleFactor != 1. && net_offset == 0.) for (int space_ndx = 0; space_ndx < numVals; space_ndx++)
      fieldVals[space_ndx] *= m_scaleFactor;

   else if (m_scaleFactor != 1. && net_offset != 0.) for (int space_ndx = 0; space_ndx < numVals; space_ndx++)
   {
      float raw_val = fieldVals[space_ndx];
      fieldVals[space_ndx] = m_scaleFactor * raw_val + net_offset;
   }

   else if (m_scaleFactor == 1.f && net_offset != 0.f) for (int space_ndx = 0; space_ndx < numVals; space_ndx++)
      fieldVals[space_ndx] += net_offset;
} // end of ScaleTheValues()


int ClimateDataInfo::GetTimeIndex(SYSDATE tgtDate, int maxDaysInYear)
{
   int start_year, time_index;

   if (m_ncid < 0) return(-2);
   if (tgtDate.year < m_firstYear || m_lastYear < tgtDate.year) return(-1);

   int jday0 = GetJulianDay(tgtDate.month, tgtDate.day, tgtDate.year, maxDaysInYear) - 1;
   if (tgtDate.year == m_mostRecentIndexYear) return(m_recentYearIndex + jday0);

   if (m_mostRecentIndexYear > tgtDate.year || m_mostRecentIndexYear < 0)
   {
      start_year = m_firstYear;
      time_index = 0;
   }
   else
   {
      start_year = m_mostRecentIndexYear;
      time_index = m_recentYearIndex;
   }

   int year = start_year;
   while (year != tgtDate.year)
   {
      time_index += maxDaysInYear == 366 ? GetDaysInCalendarYear(year) : maxDaysInYear;
      year++;
   }
   m_mostRecentIndexYear = year;
   m_recentYearIndex = time_index;

   time_index += jday0;
   if (time_index >= m_size_time)
   {
      CString msg;
      msg.Format("GetTimeIndex() time_index = %d is >= m_size_time = %d", time_index, m_size_time);
      Report::ErrorMsg(msg);
      time_index = -3;
   }

   return(time_index);
} // end of ClimateDataInfo::GetTimeIndex()


bool FlowModel::GetTodaysHRUclimate(CDTYPE type, HRU * pHRU, float &value)
{
   return(GetHRUClimate(type, pHRU, m_flowContext.dayOfYear, value));
}


bool FlowModel::GetHRUClimate(CDTYPE type, HRU *pHRU, int doyTgt, float &value) // doyTgt is 0-based
{
   bool rtn_val = true;
   int colHRU = -1, colIDU = -1;
   int * pCurr_doy = NULL;
   switch (type)
   {
      case CDT_TMAX: colHRU = m_colHruTMAX; colIDU = m_colTMAX; pCurr_doy = &m_doyTMAX; break;
      case CDT_TMIN: colHRU = m_colHruTMIN; colIDU = m_colTMIN; pCurr_doy = &m_doyTMIN; break;
      case CDT_TMEAN: colHRU = m_colHruTEMP; colIDU = m_colTEMP; pCurr_doy = &m_doyTEMP; break;
      case CDT_PRECIP: colHRU = m_colHruPRECIP; colIDU = m_colPRECIP; pCurr_doy = &m_doyPRECIP; break;
      case CDT_SOLARRAD: colHRU = m_colHruRAD_SW; colIDU = m_colRAD_SW; pCurr_doy = &m_doyRAD_SW; break;
      case CDT_SPHUMIDITY: colHRU = m_colHruSPHUMIDITY; colIDU = m_colSPHUMIDITY; pCurr_doy = &m_doySPHUMIDITY; break;
      case CDT_RELHUMIDITY: colHRU = m_colHruRH; colIDU = m_colRH; pCurr_doy = &m_doyRH; break;
      case CDT_WINDSPEED: colHRU = m_colHruWINDSPEED; colIDU = m_colWINDSPEED; pCurr_doy = &m_doyWINDSPEED; break;
      default:
      {
         CString msg;
         msg.Format("GetHRUClimate(): type = %d is not supported", type);
         Report::ErrorMsg(msg);
         return(false);
      }
   } // end of switch (type)

   if (*pCurr_doy != doyTgt) rtn_val = GetCurrentYearDailyWeatherField(type, doyTgt);

   if (rtn_val) m_pHRUlayer->GetData(pHRU->m_hruNdx, colHRU, value);

   return(rtn_val);
} // end of GetHRUClimate(CDTYPE type, HRU *pHRU, int dayOfYear, float &pValue)


bool FlowModel::GetTodaysWeatherField(CDTYPE type)
{
   return(GetDailyWeatherField(type, m_flowContext.dayOfYear, m_flowContext.pEnvContext->weatherYear));
} // end of GetTodaysWeatherField()


bool FlowModel::GetCurrentYearDailyWeatherField(CDTYPE type, int tgtDoy0)
{
   return(GetDailyWeatherField(type, tgtDoy0, m_flowContext.pEnvContext->weatherYear));
} // end of GetCurrentYearDailyWeatherField()


bool FlowModel::GetDailyWeatherField(CDTYPE type, int tgtDoy0, int tgtYear)
// Here we assume that the right climate data files have already been opened by OpenClimateDataFiles().
{
   if (m_useStationData || m_estimateParameters) 
   {
      CString msg = "GetDailyWeatherField(): m_useStationData or m_estimateParameters is set; they are unsupported.";
      Report::ErrorMsg(msg);
      return(false);
   }

   int colHRU = -1, colIDU = -1, colReach = -1;
   CUIntArray * pNdx_array = NULL; // For polygon-based climate data, this will point to an array which contains, for each IDU, the index to that IDU's data in the climate file.
   int * pCurr_doy = NULL;
   switch (type)
   {
      case CDT_TMAX: colHRU = m_colHruTMAX; colIDU = m_colTMAX; colReach = m_colStreamTMAX_AIR;  pNdx_array = &m_tmaxNdx; pCurr_doy = &m_doyTMAX; break;
      case CDT_TMIN: colHRU = m_colHruTMIN; colIDU = m_colTMIN; colReach = m_colStreamTMIN_AIR;  pNdx_array = &m_tminNdx; pCurr_doy = &m_doyTMIN; break;
      case CDT_TMEAN: colHRU = m_colHruTEMP; colIDU = m_colTEMP; colReach = m_colStreamTEMP_AIR;  pNdx_array = &m_tmeanNdx; pCurr_doy = &m_doyTEMP; break;
      case CDT_PRECIP: colHRU = m_colHruPRECIP; colIDU = m_colPRECIP; colReach = m_colStreamPRECIP; pNdx_array = &m_precipNdx; pCurr_doy = &m_doyPRECIP; break;
      case CDT_SOLARRAD: colHRU = m_colHruRAD_SW; colIDU = m_colRAD_SW; colReach = m_colStreamRAD_SW; pNdx_array = &m_solarradNdx; pCurr_doy = &m_doyRAD_SW; break;
      case CDT_SPHUMIDITY: colHRU = m_colHruSPHUMIDITY; colIDU = m_colSPHUMIDITY; colReach = m_colStreamSPHUMIDITY; pNdx_array = &m_sphumidityNdx; pCurr_doy = &m_doySPHUMIDITY; break;
      case CDT_RELHUMIDITY: colHRU = m_colHruRH; colIDU = m_colRH; colReach = m_colStreamRH; pNdx_array = &m_rhNdx; pCurr_doy = &m_doyRH; break;
      case CDT_WINDSPEED: colHRU = m_colHruWINDSPEED; colIDU = m_colWINDSPEED; colReach = m_colStreamWINDSPEED; pNdx_array = &m_windspeedNdx; pCurr_doy = &m_doyWINDSPEED; break;

      default:
      {
         CString msg;
         msg.Format("GetDailyWeatherField(): type = %d is not supported", type);
         Report::ErrorMsg(msg);
         return(false);
      }
   }
   if (*pCurr_doy == tgtDoy0) return(true); // The data for the target day has already been loaded.

   *pCurr_doy = -1;
   ClimateDataInfo *pInfo = GetClimateDataInfo(type);
   if (pInfo != NULL)
   { // There is data for this field
      if (pInfo->m_IDUbased)
      { // polygon-based climate data

         // read m_ncid varid[dayOfYear, 0:m_sizeIDU-1] into field_vals[]
         float * field_vals = new float[pInfo->m_size_idu];
         const long start[2] = { tgtDoy0, 0 };
         const long len[2] = { 1, (long)pInfo->m_size_idu };
         int rtnval = ncvarget(pInfo->m_ncid, pInfo->m_varid_data, start, len, field_vals);
         if (!chk_nc(rtnval)) return(false);

         if (pInfo->m_scaleFactor != 1. || pInfo->m_addOffsetAttribute != 0 || pInfo->m_offset != 0.) pInfo->ScaleTheValues(field_vals, pInfo->m_size_idu);

         if (pNdx_array->IsEmpty())
         { // initialize the index array
            (*pNdx_array).SetSize(m_pIDUlayer->GetRowCount());

            int * idu_ids = new int[pInfo->m_size_idu];
            // read m_ncid varid_idu[0:m_sizeIDU-1] into idu_ids[]
            rtnval = nc_get_var_int(pInfo->m_ncid, pInfo->m_varid_idu_id, idu_ids); if (!chk_nc(rtnval)) return(false);

            for (MapLayer::Iterator idu_ndx = m_pIDUlayer->Begin(); idu_ndx != m_pIDUlayer->End(); idu_ndx++)
            { // for every IDU, find the IDU's data in the climate data array, and remember its location in the index array
               float field_val = -9999.f;
               int idu_id = -1; m_pIDUlayer->GetData(idu_ndx, m_colIDU_ID, idu_id);
               int i = -1;
               for (i = 0; idu_ids[i] != idu_id && i < pInfo->m_size_idu; i++);
               if (i >= pInfo->m_size_idu)
               {
                  CString msg;
                  msg.Format("GetDailyWeatherField(): Unable to find data for idu_id = %d for type = %d in file %s", idu_id, type, pInfo->m_path);
                  Report::ErrorMsg(msg);
                  return(false);
               }

               (*pNdx_array)[idu_ndx] = i;
            }
         } // end of logic to initialize the index array

         // Copy the data from today's climate data into the IDU layer attributes.
         for (MapLayer::Iterator idu_ndx = m_pIDUlayer->Begin(); idu_ndx != m_pIDUlayer->End(); idu_ndx++)
         {
            int idu_index = (int)idu_ndx; // This facilitates setting breakpoints for specific IDUs.
            float field_value = field_vals[(*pNdx_array)[idu_ndx]]; // This facilitates examining values for specific IDUs in debug mode.
            m_pIDUlayer->SetDataU(idu_index, colIDU, field_value);
         } // end of loop to copy the field values into their IDU attributes
      } // end of logic for polygon-based climate data
      else 
      { // grid-based climate data
         int days_in_year = m_flowContext.pEnvContext->m_maxDaysInYear > 365 ? GetDaysInCalendarYear(tgtYear) : m_flowContext.pEnvContext->m_maxDaysInYear;
         int tgt_month, tgt_day; GetCalDate0(tgtDoy0, &tgt_month, &tgt_day, days_in_year);
         SYSDATE tgtDate(tgt_month, tgt_day, tgtYear);
         int time_ndx = pInfo->GetTimeIndex(tgtDate, m_flowContext.pEnvContext->m_maxDaysInYear); // m_flowContext.pEnvContext->m_simDate, m_flowContext.pEnvContext->m_maxDaysInYear
         if (time_ndx < 0)
         {
            CString msg;
            switch (time_ndx)
            {
               case -1:
                  msg.Format("GetDailyWeatherField() GetTimeIndex() returned %d. The target year %d is earlier than the first year of the climate data.",
                     time_ndx, tgtDate.year);
                  break;
               case -3:
                  msg.Format("GetDailyWeatherField() GetTimeIndex() returned %d. The target date %d %d %d is off the end of the climate data.",
                     time_ndx, tgtDate.year, tgtDate.month, tgtDate.day);
                  break;
               default:
                  msg.Format("GetDailyWeatherField() GetTimeIndex() returned %d.", time_ndx);
                  break;
            } // end of switch(time_ndx)
            Report::ErrorMsg(msg);
            return(false);
         }

         float * field_vals = new float[NUM_OF_CLIMATE_GRIDCELLS];
         const long start[3] = { time_ndx, 0, 0 };
         const long len[3] = { 1, (long)NUM_OF_CLIMATE_GRID_ROWS, (long)NUM_OF_CLIMATE_GRID_COLUMNS };
         int rtnval = NC_NOERR;;

         switch (pInfo->m_nctype)
         {
            case NC_FLOAT: 
               rtnval = ncvarget(pInfo->m_ncid, pInfo->m_varid_data, start, len, field_vals);
               if (!chk_nc(rtnval)) { delete field_vals;  return(false); }
               break;

            case NC_SHORT:
            {
               __int16 * field_vals_int16 = new __int16[NUM_OF_CLIMATE_GRIDCELLS];
               rtnval = ncvarget(pInfo->m_ncid, pInfo->m_varid_data, start, len, field_vals_int16);
               if (!chk_nc(rtnval)) { delete field_vals_int16;  return(false); }
               for (int i = 0; i < NUM_OF_CLIMATE_GRIDCELLS; i++) field_vals[i] = (float)field_vals_int16[i];
               delete field_vals_int16;
            }
            break;

            default: ASSERT(false); return(false);
         }

         if (pInfo->m_scaleFactor != 1. || pInfo->m_addOffsetAttribute != 0 || pInfo->m_offset != 0.) pInfo->ScaleTheValues(field_vals, NUM_OF_CLIMATE_GRIDCELLS);

         for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu < m_pIDUlayer->End(); idu++)
         {
            int grid_index; m_pIDUlayer->GetData(idu, m_colGRID_INDEX, grid_index);
            if (!(grid_index >= LOWEST_POPULATED_GRIDCELL_INDEX && grid_index <= HIGHEST_POPULATED_GRIDCELL_INDEX))
            {
               CString msg;
               msg.Format("GetDailyWeatherField() grid_index = %d is out of bounds. Run with years_to_run = 0 using a climate dataset with single year files to populate GRID_INDEX.",
                  grid_index);
               Report::FatalMsg(msg);
            }

            if (pInfo->m_row0inSouth)
            { // In the NetCDF file itself, row 0 of the gridded data is along the south edge of the grid.
               int column = grid_index % NUM_OF_CLIMATE_GRID_COLUMNS;
               int orig_row = grid_index / NUM_OF_CLIMATE_GRID_COLUMNS;
               int new_row = (NUM_OF_CLIMATE_GRID_ROWS - 1) - orig_row;
               int adjusted_grid_index = new_row * NUM_OF_CLIMATE_GRID_COLUMNS + column;
               m_pIDUlayer->SetDataU(idu, colIDU, field_vals[adjusted_grid_index]);
            }
            else m_pIDUlayer->SetDataU(idu, colIDU, field_vals[grid_index]);            
         } // end of loop to populate this IDU weather attribute
      } // end of logic for grid-based climate data
   }
   else
   { // There is no data for this field.
      switch (type)
      {
         case CDT_TMEAN:
            if (tgtDoy0 != m_doyTMAX) if (!GetDailyWeatherField(CDT_TMAX, tgtDoy0, tgtYear)) return(false);
            if (tgtDoy0 != m_doyTMIN) if (!GetDailyWeatherField(CDT_TMIN, tgtDoy0, tgtYear)) return(false);
            for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
            {
               float tmin = -9999.f, tmax = -9999.f;
               m_pIDUlayer->GetData(idu, m_colTMIN, tmin);
               m_pIDUlayer->GetData(idu, m_colTMAX, tmax);
               float tmean = (tmin + tmax) / 2.f;
               m_pIDUlayer->SetDataU(idu, m_colTEMP, tmean);
            }
            GetHRUweatherFieldFromIDUs(type, colHRU, colIDU);
            m_doyTEMP = tgtDoy0;
            break;

         case CDT_RELHUMIDITY: // Try to construct RH
         {
            float rhmin[NUM_OF_CLIMATE_GRIDCELLS];
            float rhmax[NUM_OF_CLIMATE_GRIDCELLS];
            float rh[NUM_OF_CLIMATE_GRIDCELLS];
            bool rh_available = GetWeatherField(CDT_RHMIN, tgtDoy0, tgtYear, rhmin) && GetWeatherField(CDT_RHMAX, tgtDoy0, tgtYear, rhmax);
            if (rh_available)
            {
               for (int i = 0; i < NUM_OF_CLIMATE_GRIDCELLS; i++) rh[i] = (rhmin[i] + rhmax[i]) / 2;
               for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
               {
                  int grid_index; m_pIDUlayer->GetData(idu, m_colGRID_INDEX, grid_index);
                  ASSERT(grid_index >= LOWEST_POPULATED_GRIDCELL_INDEX && grid_index <= HIGHEST_POPULATED_GRIDCELL_INDEX);
                  m_pIDUlayer->SetDataU(idu, colIDU, rh[grid_index]);
               }
            }
            else
            {
               m_pIDUlayer->SetColDataU(colIDU, -1.f); 
               if (tgtDoy0 == 0 && tgtYear == m_flowContext.pEnvContext->startYear)
               {
                  CString msg;
                  msg.Format("GetDailyWeatherField() relative humidity and one or both of rhmin and rhmax are unavailable. Setting RH = -1.");
                  Report::ErrorMsg(msg);
                  return(false);
               }
            } // end of rh not available
         }
         break;

         case CDT_WINDSPEED: // Try to construct WINDSPEED from UAS and VAS
            {
               float uas[NUM_OF_CLIMATE_GRIDCELLS];
               float vas[NUM_OF_CLIMATE_GRIDCELLS];
               float ws[NUM_OF_CLIMATE_GRIDCELLS];
               bool wind_available = GetWeatherField(CDT_UAS, tgtDoy0, tgtYear, uas) && GetWeatherField(CDT_VAS, tgtDoy0, tgtYear, vas);
               if (wind_available)
               {
                  for (int i = 0; i < NUM_OF_CLIMATE_GRIDCELLS; i++) ws[i] = sqrtf(uas[i] * uas[i] + vas[i] * vas[i]);
                  for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
                  {
                     int grid_index; m_pIDUlayer->GetData(idu, m_colGRID_INDEX, grid_index);
                     ASSERT(grid_index >= LOWEST_POPULATED_GRIDCELL_INDEX && grid_index <= HIGHEST_POPULATED_GRIDCELL_INDEX);
                     m_pIDUlayer->SetDataU(idu, colIDU, ws[grid_index]);
                  }
               }
               else
               {
                  m_pIDUlayer->SetColDataU(colIDU, 1.f); // constant windspeed at 1 m/s
                  if (tgtDoy0 == 0 && tgtYear == m_flowContext.pEnvContext->startYear)
                  {
                     CString msg;
                     msg.Format("GetDailyWeatherField() Can't read windspeed. Using windspeed = 1 m/s.");
                     Report::WarningMsg(msg);
                  }
               } // end of wind not available
            }
            break;
         default:
         {
            CString msg;
            msg.Format("GetDailyWeatherField(): Unable to get data for type = %d", type);
            Report::ErrorMsg(msg);
            return(false);
         }
      } // end of switch (type)
   } // end of if (pInfo != NULL) ... else

   GetHRUweatherFieldFromIDUs(type, colHRU, colIDU);
   GetReachWeatherFieldFromIDUs(type, colReach, colIDU);
   *pCurr_doy = tgtDoy0;
   return(true);
} // end of GetDailyWeatherField()


bool FlowModel::GetWeatherField(CDTYPE type, int tgtDoy0, int tgtYear, float field_vals[])
{
   ClimateDataInfo *pInfo = GetClimateDataInfo(type); 
   if (pInfo == NULL) return(false);

   int tgt_month, tgt_day; GetCalDate0(tgtDoy0, &tgt_month, &tgt_day, GetDaysInCalendarYear(tgtYear));
   SYSDATE tgtDate(tgt_month, tgt_day, tgtYear);
   int time_ndx = pInfo->GetTimeIndex(tgtDate, m_flowContext.pEnvContext->m_maxDaysInYear); // m_flowContext.pEnvContext->m_simDate, m_flowContext.pEnvContext->m_maxDaysInYear
   if (time_ndx < 0)
   {
      CString msg;
      msg.Format("GetWeatherField(%d, %d, %d, ...): time_ndx = %d", type, tgtDoy0, tgtYear, time_ndx);
      Report::ErrorMsg(msg);
      return(false);
   }
   const long start[3] = { time_ndx, 0, 0 };
   const long len[3] = { 1, (long)NUM_OF_CLIMATE_GRID_ROWS, (long)NUM_OF_CLIMATE_GRID_COLUMNS };
   bool rtnval = ncvarget(pInfo->m_ncid, pInfo->m_varid_data, start, len, field_vals);
   if (!chk_nc(rtnval)) return(false); 

   if (pInfo->m_scaleFactor != 1. || pInfo->m_addOffsetAttribute != 0 || pInfo->m_offset != 0.) pInfo->ScaleTheValues(field_vals, NUM_OF_CLIMATE_GRIDCELLS);

   return(true);
} // end of GetWeatherField()


bool FlowModel::InitGridIndex() // Read a netCDF file to initialize the GRID_INDEX IDU attribute
{
   CString msg;

   ClimateScenario *pScenario = m_scenarioArray.GetAt(m_currentFlowScenarioIndex);
   ClimateDataInfo *pInfo = NULL;
   CString file_path_and_name;
   bool found_a_file = false;
   for (int i = 0; !found_a_file && i < (int)pScenario->m_climateInfoArray.GetSize(); i++)
   {
      pInfo = pScenario->m_climateInfoArray[i];
      if (pInfo->m_pDataObj == NULL) continue;
      file_path_and_name = pInfo->m_path;

      // Is the .nc suffix already present?  If so, it is a multiyear file.
      if (file_path_and_name.GetLength() >= 4)
      {
         CString suffix = file_path_and_name.Right(3);
         if (suffix == ".nc") continue; // Multiyear files can take too long to read with ReadSpatialData().
      }
      found_a_file = true;
   } // end of loop through the climate fields to find a single year file
   if (!found_a_file) return(false);

   CString year_cstr;
   year_cstr.Format("_%i", m_flowContext.pEnvContext->startYear);
   file_path_and_name = file_path_and_name + year_cstr + ".nc";
   if (!pInfo->m_pDataObj->Open(file_path_and_name, pInfo->m_varName))
      return(false);

   pInfo->m_pDataObj->ReadSpatialData();

   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      if (((int)idu / 25000) * 25000 == (int)idu)
      {
         msg.Format("InitGridIndex(): idu = %d", (int)idu);
         Report::LogMsg(msg);
      }

      Poly *pPoly = m_pIDUlayer->GetPolygon(idu);
      Vertex centroid = pPoly->GetCentroid();
      if ((centroid.x != centroid.x || centroid.y != centroid.y) || (centroid.x == 0 && centroid.y == 0)) // detect NaNs
      {
         msg.Format("InitGridIndex(): bad centroid at idu = %d", (int)idu);
         Report::ErrorMsg(msg);
         return(false);
      }

      int grid_index; int &r_grid_index = grid_index;
      bool OKflag = true;
      float field_val = pInfo->m_pDataObj->Get(centroid.x, centroid.y, r_grid_index, 0, m_projectionWKT, false, &OKflag);
      if (!OKflag)
      {
         msg.Format("InitGridIndex(): pInfo->m_pDataObj->Get() failed. idu = %d, centroid = (%f,%f)", (int)idu, centroid.x, centroid.y);
         Report::ErrorMsg(msg);
         return(false);
      }
      m_pIDUlayer->SetDataU(idu, m_colGRID_INDEX, grid_index);
   } // end IDU loop
   msg.Format("InitGridIndex() - initialized GRID_INDEX");
   Report::LogMsg(msg);

   return(true);
} // end of InitGridIndex()


bool FlowModel::GetHRUweatherFieldFromIDUs(CDTYPE type, int colHRU, int colIDU)
{
   for (MapLayer::Iterator hru_poly = m_pHRUlayer->Begin(); hru_poly < m_pHRUlayer->End(); hru_poly++)
   {
      HRU *pHRU = GetHRU((int)hru_poly); // Relies on HRU array to be ordered the same as the HRUs in the HRU layer
      ASSERT(pHRU->m_hruNdx == (int)hru_poly);
      float area_weighted_accumulator = 0;
      for (int idu_in_HRU = 0; idu_in_HRU < pHRU->m_polyIndexArray.GetSize(); idu_in_HRU++)
      {
         float idu_area; m_pIDUlayer->GetData((int)pHRU->m_polyIndexArray[idu_in_HRU], m_colAREA, idu_area);
         float field_val; m_pIDUlayer->GetData((int)pHRU->m_polyIndexArray[idu_in_HRU], colIDU, field_val);
         area_weighted_accumulator += idu_area * field_val;
      }
      float hru_field_val = area_weighted_accumulator / pHRU->m_HRUtotArea_m2;

      m_pHRUlayer->SetDataU((int)hru_poly, colHRU, hru_field_val);

      switch (type)
      {
         case CDT_TMIN: pHRU->m_currentMinTemp = hru_field_val; break;
         case CDT_TMEAN: pHRU->m_currentAirTemp = hru_field_val; break;
         case CDT_PRECIP: pHRU->m_currentPrecip = hru_field_val; break;
         case CDT_TMAX: pHRU->m_TMAX = hru_field_val; break;
         case CDT_SOLARRAD: pHRU->m_SOLARRAD = hru_field_val; break;
         case CDT_SPHUMIDITY: pHRU->m_SPHUMIDITY = hru_field_val; break;
         case CDT_WINDSPEED: pHRU->m_WINDSPEED = hru_field_val; break;
         case CDT_RELHUMIDITY: pHRU->m_RELHUMIDITY = hru_field_val; break;

         default: break;
      }
   } // end of loop to populate this HRU weather attribute

   return(true);
} // end of GetHRUweatherFieldFromIDUs()


bool FlowModel::GetReachWeatherFieldFromIDUs(CDTYPE type, int colReach, int colIDU)
{
   int reach_count = GetReachCount();
   for (int reach_ndx = 0; reach_ndx < reach_count; reach_ndx++)
   {
      Reach * pReach = GetReach(reach_ndx);

   if (pReach->m_IDUndxForReach < 0)
   { 
         pReach->m_IDUndxForReach = GetIDUndxForReach(pReach);
         if (pReach->m_IDUndxForReach < 0) return(false);
      }

      float idu_field_val = -9999.f;
      m_pIDUlayer->GetData(pReach->m_IDUndxForReach, colIDU, idu_field_val);
      m_pReachLayer->SetDataU(pReach->m_polyIndex, colReach, idu_field_val);
   } // end of loop through reach array

   return(true);
} // end of GetReachWeatherFieldFromIDUs()


float FlowModel::GetTodaysReachTEMP_AIR(Reach * pReach)
{
   float temp_air; m_pReachLayer->GetData(pReach->m_polyIndex, m_colStreamTEMP_AIR, temp_air);
   return(temp_air);
} // end of GetTodaysReachTEMP_AIR()


float FlowModel::GetTodaysReachTMAX_AIR(Reach * pReach)
{
   float tmax_air = m_pReachLayer->GetData(pReach->m_polyIndex, m_colStreamTMAX_AIR, tmax_air);
   return(tmax_air);
} // end of GetTodaysReachTMAX_AIR()


float FlowModel::GetTodaysReachTMIN_AIR(Reach * pReach)
{
   float tmin_air; m_pReachLayer->GetData(pReach->m_polyIndex, m_colStreamTMIN_AIR, tmin_air);
   return(tmin_air);
} // end of GetTodaysReachTMIN_AIR()


float FlowModel::GetTodaysReachPRECIP(Reach * pReach)
{
   float precip; m_pReachLayer->GetData(pReach->m_polyIndex, m_colStreamPRECIP, precip);
   return(precip);
} // end of GetTodaysReachPRECIP()


float FlowModel::GetTodaysReachSPHUMIDITY(Reach * pReach)
{
   float sphumidity; m_pReachLayer->GetData(pReach->m_polyIndex, m_colStreamSPHUMIDITY, sphumidity);
   return(sphumidity);
} // end of GetTodaysReachSPHUMIDITY()


float FlowModel::GetTodaysReachRAD_SW(Reach * pReach)
{
   float rad_sw; m_pReachLayer->GetData(pReach->m_polyIndex, m_colStreamRAD_SW, rad_sw);
   return(rad_sw);
} // end of GetTodaysReachRAD_SW()


float FlowModel::GetTodaysReachWINDSPEED(Reach * pReach)
{
   float windspeed; m_pReachLayer->GetData(pReach->m_polyIndex, m_colStreamWINDSPEED, windspeed);
   return(windspeed);
} // end of GetTodaysReachWINDSPEED()


int FlowModel::GetIDUndxForReach(Reach * pReach)
{
   int idu_ndx = -1;
   int idu_id; m_pReachLayer->GetData(pReach->m_polyIndex, m_colStreamIDU_ID, idu_id);

   if (idu_id >= 0)
   {
      idu_ndx = m_pIDUlayer->FindIndex(m_colIDU_ID, idu_id);
   }
   else
   {
      double xCoord = 0.f, yCoord = 0.f;
      Poly *pPoly = m_pStreamLayer->GetPolygon(pReach->m_polyIndex);
      int num_vertices = pPoly->GetVertexCount();
      int vertex_ndx = 0;
      bool found_flag = false;
      while (vertex_ndx < num_vertices && !found_flag)
      {
         xCoord = pPoly->GetVertex(vertex_ndx).x;  
         yCoord = pPoly->GetVertex(vertex_ndx).y;   //pPoly->GetCentroid().y;
         for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); !found_flag && idu != m_pIDUlayer->End(); idu++)
         {
            Poly *pPoly = m_pIDUlayer->GetPolygon(idu);
            Vertex v((REAL)xCoord, (REAL)yCoord);

            idu_ndx = (int)idu; // Facilitates setting breakpoints on particular IDUs.
            found_flag = pPoly->IsPointInPoly(v);
         } // end of loop thru IDUs
         vertex_ndx++;
      } // end of while (vertex_ndx < num_vertices && !found_flag)
      if (!found_flag)
      {
         CString msg;
         msg.Format("GetIDUndxForReach(): Reach at index = %d has no vertices in any IDU polygon. ", pReach->m_polyIndex);
         Report::ErrorMsg(msg);
      }

      pReach->m_IDUndxForReach = idu_ndx;
      int idu_id; m_pIDUlayer->GetData(idu_ndx, m_colIDU_ID, idu_id);
      m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colStreamIDU_ID, idu_id);
   }

   return(idu_ndx);
} // end of GetIDUndxForReach()


float FlowModel::GetTotalReachLength( void )
   {
   float length = 0;

   for ( int i=0; i < this->GetReachCount(); i++ )
      {
      Reach *pReach = m_reachArray[ i ];

      length += pReach->m_length;
      }

   return length;
   }


bool FlowModel::CollectModelOutput(void)
{
   clock_t start = clock();

   // any outputs to collect?
   int moCount = 0;
   int moCountIdu = 0;
   int moCountHru = 0;
   int moCountXHRU = 0;
   int moCountReach = 0;
   int moCountLink = 0;
   int moCountNode = 0;
   int moCountSubcatch = 0;
   int moCountHruIdu = 0;
   int moCountReservoir = 0;

   for (int j = 0; j < (int)m_modelOutputGroupArray.GetSize(); j++)
   {
      ModelOutputGroup* pGroup = m_modelOutputGroupArray[j];

      for (int i = 0; i < (int)pGroup->GetSize(); i++)
      {
         ModelOutput* pOutput = pGroup->GetAt(i);

         // initialize ModelOutput
         pOutput->m_value = 0;
         pOutput->m_totalQueryArea = 0;

         if (pOutput->m_inUse)
         {
            moCount++;

            switch (pOutput->m_modelDomain)
            {
               case MOD_IDU:        moCountIdu++;          break;
               case MOD_HRU:        moCountHru++;          break;
               case MOD_XHRU:   moCountXHRU++;     break;
               case MOD_REACH:      moCountReach++;        break;
               case MOD_LINK:      moCountLink++;        break;
               case MOD_NODE: moCountNode++; break;
               case MOD_SUBCATCH: moCountSubcatch++; break;
               case MOD_HRU_IDU:    moCountHruIdu++;       break;
               case MOD_RESERVOIR:   moCountReservoir++;    break;
            }
         }
      }
   }

   if (moCount == 0)
      return true;

   float totalIDUArea = 0;

   // iterate through IDU's to collect output if any of the expressions are IDU-domain
   if (moCountIdu > 0)
   {
      int colArea = m_pCatchmentLayer->GetFieldCol("AREA");

      for (MapLayer::Iterator idu = this->m_pCatchmentLayer->Begin(); idu < m_pCatchmentLayer->End(); idu++)
      {
         if (m_pQE_IDU)
            m_pQE_IDU->SetCurrentRecord(idu);

         m_pME_IDU->SetCurrentRecord(idu);

         float area = 0;
         m_pCatchmentLayer->GetData(idu, colArea, area);
         totalIDUArea += area;

         for (int j = 0; j < (int)m_modelOutputGroupArray.GetSize(); j++)
         {
            ModelOutputGroup* pGroup = m_modelOutputGroupArray[j];

            for (int i = 0; i < (int)pGroup->GetSize(); i++)
            {
               ModelOutput* pOutput = pGroup->GetAt(i);

               if (pOutput->m_inUse == false || pOutput->m_modelDomain != MOD_IDU)
                  continue;

               bool passConstraints = true;

               if (pOutput->m_pQuery)
               {
                  bool result = false;
                  bool ok = pOutput->m_pQuery->Run(idu, result);

                  if (result == false)
                     passConstraints = false;
               }

            // did we pass the constraints?  If so, evaluate preferences
               if (passConstraints)
               {
                  pOutput->m_totalQueryArea += area;
                  float value = 0;

                  if (pOutput->m_pMapExpr != NULL)
                  {
                     bool ok = m_pME_IDU->EvaluateExpr(pOutput->m_pMapExpr, pOutput->m_pQuery ? true : false);
                     if (ok)
                        value = (float)pOutput->m_pMapExpr->GetValue();
                  }

               // we have a value for this IDU, accumulate it based on the model type
                  switch (pOutput->m_modelType)
                  {
                     case MOT_SUM:
                        pOutput->m_value += value;
                        break;

                     case MOT_AREAWTMEAN:
                        pOutput->m_value += value * area;
                        break;

                     case MOT_PCTAREA:
                        pOutput->m_value += area;
                        break;
                     //case MO_PCT_CONTRIB_AREA:
                     //   return true;
                  }
               }  // end of: if( passed constraints)
            }  // end of: for ( each model )
         }  // end of: for ( each group )
      }  // end of: for ( each IDU )
   }  // end of: if ( moCountIdu > 0 )

// Next, do HRU-level variables
   float totalHRUarea = 0;
   if (moCountHru > 0)
   {
      int colAREA_AC = m_pHRUlayer->GetFieldCol("AREA_AC");
      int colHRU_ID = m_pHRUlayer->GetFieldCol("HRU_ID");

      for (MapLayer::Iterator hru = m_pHRUlayer->Begin(); hru < m_pHRUlayer->End(); hru++)
      {
         if (m_pQE_HRU)
            m_pQE_HRU->SetCurrentRecord(hru);

         m_pME_HRU->SetCurrentRecord(hru);

         float area_ac = 0;
         m_pHRUlayer->GetData(hru, colAREA_AC, area_ac);
         float area = area_ac * M2_PER_ACRE;
         totalHRUarea += area;

         HRU* pHRU = GetHRU((int)hru);

         // update static HRU variables
         HRU::m_mvDepthMelt = pHRU->m_depthMelt * 1000.0f;  // volume of water in snow (converted from m to mm)
         HRU::m_mvDepthSWE_mm = pHRU->m_depthSWE * 1000.0f;   // snow water equivalent, mm (includes both ice and meltwater in the snowpack)
         double layer2_volumeWater = pHRU->GetLayer(BOX_NAT_SOIL)->m_volumeWater;
         double layer3_volumeWater = pHRU->GetLayer(BOX_IRRIG_SOIL)->m_volumeWater;
         HRU::m_mvTopSoilH2O_mm = ((layer2_volumeWater + layer3_volumeWater) / pHRU->m_HRUeffArea_m2) * 1000.f;
         HRU::m_mvShallowGW_mm = (pHRU->GetLayer(BOX_FAST_GW)->m_volumeWater / pHRU->m_HRUeffArea_m2) * 1000.f;
         HRU::m_mvDeepGW_mm = (pHRU->GetLayer(BOX_SLOW_GW)->m_volumeWater / pHRU->m_HRUeffArea_m2) * 1000.f;
         HRU::m_mvCurrentPrecip = pHRU->m_currentPrecip;
         HRU::m_mvRainThrufall_mm = pHRU->m_rainThrufall_mm;
         HRU::m_mvRainEvap_mm = pHRU->m_rainEvap_mm;
         HRU::m_mvSnowThrufall_mmSWE = pHRU->m_snowThrufall_mm;
         HRU::m_mvSnowEvap_mmSWE = pHRU->m_snowEvap_mm;
         HRU::m_mvMelt_mm = pHRU->m_melt_mm;
         HRU::m_mvRefreezing_mm = pHRU->m_refreezing_mm;
         HRU::m_mvInfiltration_mm = pHRU->m_infiltration_mm;
         HRU::m_mvRechargeToIrrigatedSoil_mm = pHRU->m_rechargeToIrrigatedSoil_mm;
         HRU::m_mvRechargeToNonIrrigatedSoil_mm = pHRU->m_rechargeToNonIrrigatedSoil_mm;
         HRU::m_mvRechargeTopSoil_mm = pHRU->m_rechargeTopSoil_mm;
         HRU::m_mvRechargeToUpperGW_mm = pHRU->m_rechargeToUpperGW_mm;
         HRU::m_mvQ0_mm = pHRU->m_Q0_mm;
         HRU::m_mvQ2_mm = pHRU->m_Q2_mm;
         HRU::m_mvPercolation_mm = pHRU->m_percolation_mm;
         HRU::m_mvCurrentAirTemp = pHRU->m_currentAirTemp;
         HRU::m_mvCurrentMinTemp = pHRU->m_currentMinTemp;
         HRU::m_mvHRU_TO_AQ_mm = pHRU->m_aquifer_recharge_mm;

         HRU::m_mvCurrentRecharge = pHRU->m_currentGWRecharge;
         HRU::m_mvCurrentGwFlowOut = pHRU->m_currentGWFlowOut;

         HRU::m_mvCurrentET = pHRU->m_currentET;
         HRU::m_mvCurrentMaxET = pHRU->m_currentMaxET;
         HRU::m_mvCurrentSediment = pHRU->m_currentSediment;
         HRU::m_mvCumP = pHRU->m_precip_yr;
         HRU::m_mvCumRunoff = pHRU->m_runoff_yr;
         HRU::m_mvCumET = pHRU->m_et_yr;
         HRU::m_mvCumMaxET = pHRU->m_maxET_yr;
         HRU::m_mvCumRecharge = pHRU->m_gwRecharge_yr;
         HRU::m_mvCumGwFlowOut = pHRU->m_gwFlowOut_yr;

         HRU::m_mvElws = pHRU->m_elws;

         for (int j = 0; j < (int)m_modelOutputGroupArray.GetSize(); j++)
         {
            ModelOutputGroup* pGroup = m_modelOutputGroupArray[j];

            for (int i = 0; i < (int)pGroup->GetSize(); i++)
            {
               ModelOutput* pOutput = pGroup->GetAt(i);

               if (pOutput->m_inUse == false || pOutput->m_modelDomain != MOD_HRU)
                  continue;

               bool passConstraints = true;

               if (pOutput->m_pQuery)
               {
                  bool result = false;
                  bool ok = pOutput->m_pQuery->Run(hru, result);

                  if (result == false)
                     passConstraints = false;
               }

            // did we pass the constraints?  If so, evaluate preferences
               if (passConstraints)
               {
                  pOutput->m_totalQueryArea += area;
                  float value = 0;

                  if (pOutput->m_pMapExpr != NULL)
                  {
                     bool ok = m_pME_HRU->EvaluateExpr(pOutput->m_pMapExpr, pOutput->m_pQuery ? true : false);
                     if (ok)
                        value = (float)pOutput->m_pMapExpr->GetValue();
                  }

               // we have a value for this IDU, accumulate it based on the model type
                  switch (pOutput->m_modelType)
                  {
                     case MOT_SUM:
                        pOutput->m_value += value;
                        break;

                     case MOT_AREAWTMEAN:
                        pOutput->m_value += value * area;
                        break;

                     case MOT_PCTAREA:
                        pOutput->m_value += area;
                        break;
                     //case MO_PCT_CONTRIB_AREA:
                     //   return true;
                  }
               }  // end of: if( passed constraints)
            }  // end of: for ( each model )
         }  // end of: for ( each output group )
      }  // end of: for ( each HRU )
   }  // end of: if ( moCountHru > 0 )


// Next, do XHRU-level variables
   float totalXHRUarea = 0;

// iterate through rows in HRU layer to collect output if any of the expressions are XHRU-domain
   if (moCountXHRU > 0)
   {
      //int colAREA = m_pHRUlayer->GetFieldCol("AREA");

      for (MapLayer::Iterator HRUrow = m_pHRUlayer->Begin(); HRUrow < m_pHRUlayer->End(); HRUrow++)
      {
         if (m_pQE_XHRU)
            m_pQE_XHRU->SetCurrentRecord(HRUrow);

         m_pME_XHRU->SetCurrentRecord(HRUrow);

         float area = 0;
         m_pHRUlayer->GetData(HRUrow, m_colHruAREA, area);
         totalXHRUarea += area;

         for (int j = 0; j < (int)m_modelOutputGroupArray.GetSize(); j++)
         {
            ModelOutputGroup* pGroup = m_modelOutputGroupArray[j];

            for (int i = 0; i < (int)pGroup->GetSize(); i++)
            {
               ModelOutput* pOutput = pGroup->GetAt(i);

               if (pOutput->m_inUse == false || pOutput->m_modelDomain != MOD_XHRU)
                  continue;

               bool passConstraints = true;

               if (pOutput->m_pQuery)
               {
                  bool result = false;
                  bool ok = pOutput->m_pQuery->Run(HRUrow, result);

                  if (result == false)
                     passConstraints = false;
               }

               // did we pass the constraints?  If so, evaluate preferences
               if (passConstraints)
               {
                  pOutput->m_totalQueryArea += area;
                  float value = 0;

                  if (pOutput->m_pMapExpr != NULL)
                  {
                     bool ok = m_pME_XHRU->EvaluateExpr(pOutput->m_pMapExpr, pOutput->m_pQuery ? true : false);
                     if (ok)
                        value = (float)pOutput->m_pMapExpr->GetValue();
                  }

                  // we have a value for this HRUrow, accumulate it based on the model type
                  switch (pOutput->m_modelType)
                  {
                     case MOT_SUM:
                        pOutput->m_value += value;
                        break;

                     case MOT_AREAWTMEAN:
                        pOutput->m_value += value * area;
                        break;

                     case MOT_PCTAREA:
                        pOutput->m_value += area;
                        break;
                        //case MO_PCT_CONTRIB_AREA:
                        //   return true;
                  }
               }  // end of: if( passed constraints)
            }  // end of: for ( each model )
         }  // end of: for ( each group )
      }  // end of: for ( each HRUrow )
   }  // end of: if ( moCountXHRU > 0 )


     // Next, do Reach-level variables
   if (moCountReach > 0)
   {
      for (int h = 0; h < (int)this->m_reachArray.GetSize(); h++)
      {
         Reach* pReach = m_reachArray[h];
         ReachSubnode* pNode = pReach->GetReachSubnode(0);

         if (m_pQE_Reach) m_pQE_Reach->SetCurrentRecord(pReach->m_polyIndex);
         m_pME_Reach->SetCurrentRecord(pReach->m_polyIndex);

         // update static HRU variables
         Reach::m_mvCurrentStreamFlow = (pReach->GetReachDischargeWP().m_volume_m3 / SEC_PER_DAY);
         Reach::m_mvInstreamWaterRightUse = pReach->m_instreamWaterRightUse;

         MapLayer* pStreamLayer = this->m_pStreamLayer;
         int streamNdx = pReach->m_polyIndex;
         float q_div_wrq = 0.f; pStreamLayer->GetData(streamNdx, m_colStreamQ_DIV_WRQ, q_div_wrq);
         Reach::m_mvQ_DIV_WRQ = q_div_wrq;
         float instrm_req = 0.f; pStreamLayer->GetData(streamNdx, m_colStreamINSTRM_REQ, instrm_req);
         Reach::m_mvINSTRM_REQ = instrm_req;

         Reservoir* pReservoir = pReach->GetReservoir();
         Reach::m_mvRESVR_H2O = pReservoir == NULL ? 0. : pReservoir->m_volume;

         for (int j = 0; j < (int)m_modelOutputGroupArray.GetSize(); j++)
         {
            ModelOutputGroup* pGroup = m_modelOutputGroupArray[j];

            for (int i = 0; i < (int)pGroup->GetSize(); i++)
            {
               ModelOutput* pOutput = pGroup->GetAt(i);

               if (pOutput->m_inUse == false || pOutput->m_modelDomain != MOD_REACH)
                  continue;
               if (pNode->m_svArray != NULL)
                  Reach::m_mvCurrentTracer = pNode->m_svArray[pOutput->m_esvNumber] / pNode->m_waterParcel.m_volume_m3;  // volume of water        
               bool passConstraints = true;

               if (pOutput->m_pQuery)
               {
                  bool result = false;
                  bool ok = pOutput->m_pQuery->Run(pReach->m_polyIndex, result);

                  if (result == false)
                     passConstraints = false;
               }

               // did we pass the constraints (we found the correct reach, or reaches)?  If so, evaluate preferences 
               if (passConstraints)
               {
                  float value = 0;

                  if (pOutput->m_pMapExpr != NULL)
                  {
                     bool ok = m_pME_Reach->EvaluateExpr(pOutput->m_pMapExpr, pOutput->m_pQuery ? true : false);
                     if (ok)
                        value = (float)pOutput->m_pMapExpr->GetValue();
                  }

                  // we have a value for this Reach, accumulate it based on the model type
                  switch (pOutput->m_modelType)
                  {
                     case MOT_SUM:
                        pOutput->m_value += value;
                        break;

                        //case MO_PCT_CONTRIB_AREA:
                        //   return true;
                  }
               }  // end of: if( passed constraints)
            }  // end of: for ( each model )
         }  // end of: for ( each output group )
      }  // end of: for ( each reach )
   }  // end of: if ( moCountReach > 0 )

      // Next, do Link-level variables
   if (moCountLink > 0)
   {
      for (MapLayer::Iterator row = this->m_pLinkLayer->Begin(); row < m_pLinkLayer->End(); row++)
      {
         if (m_pQE_Link)
            m_pQE_Link->SetCurrentRecord(row);

         m_pME_Link->SetCurrentRecord(row);

         for (int j = 0; j < (int)m_modelOutputGroupArray.GetSize(); j++)
         {
            ModelOutputGroup* pGroup = m_modelOutputGroupArray[j];

            for (int i = 0; i < (int)pGroup->GetSize(); i++)
            {
               ModelOutput* pOutput = pGroup->GetAt(i);

               if (pOutput->m_inUse == false || pOutput->m_modelDomain != MOD_LINK)
                  continue;

               bool passConstraints = true;

               if (pOutput->m_pQuery)
               {
                  bool result = false;
                  bool ok = pOutput->m_pQuery->Run(row, result);

                  if (result == false)
                     passConstraints = false;
               }

               // did we pass the constraints?  If so, evaluate preferences
               if (passConstraints)
               {
                  float value = 0;

                  if (pOutput->m_pMapExpr != NULL)
                  {
                     bool ok = m_pME_Link->EvaluateExpr(pOutput->m_pMapExpr, pOutput->m_pQuery ? true : false);
                     if (ok)
                        value = (float)pOutput->m_pMapExpr->GetValue();
                  }

                  // we have a value for this row, accumulate it based on the model type
                  switch (pOutput->m_modelType)
                  {
                     case MOT_SUM:
                        pOutput->m_value += value;
                        break;
                        /*
                        case MOT_AREAWTMEAN:
                        pOutput->m_value += value * area;
                        break;

                        case MOT_PCTAREA:
                        pOutput->m_value += area;
                        break;
                        */
                  }
               }  // end of: if( passed constraints)
            }  // end of: for ( each model )
         }  // end of: for ( each group )
      }  // end of: for ( each row )
   }  // end of: if ( moCountLink > 0 )

   // Next, do Node-level variables
   if (moCountNode > 0)
   {
      for (MapLayer::Iterator row = m_pNodeLayer->Begin(); row < m_pNodeLayer->End(); row++)
      {
         if (m_pQE_Node)
            m_pQE_Node->SetCurrentRecord(row);

         m_pME_Node->SetCurrentRecord(row);

         for (int j = 0; j < (int)m_modelOutputGroupArray.GetSize(); j++)
         {
            ModelOutputGroup* pGroup = m_modelOutputGroupArray[j];

            for (int i = 0; i < (int)pGroup->GetSize(); i++)
            {
               ModelOutput* pOutput = pGroup->GetAt(i);

               if (pOutput->m_inUse == false || pOutput->m_modelDomain != MOD_NODE)
                  continue;

               bool passConstraints = true;

               if (pOutput->m_pQuery)
               {
                  bool result = false;
                  bool ok = pOutput->m_pQuery->Run(row, result);

                  if (result == false)
                     passConstraints = false;
               }

               // did we pass the constraints?  If so, evaluate preferences
               if (passConstraints)
               {
                  float value = 0;

                  if (pOutput->m_pMapExpr != NULL)
                  {
                     bool ok = m_pME_Node->EvaluateExpr(pOutput->m_pMapExpr, pOutput->m_pQuery ? true : false);
                     if (ok)
                        value = (float)pOutput->m_pMapExpr->GetValue();
                  }

                  // we have a value for this row, accumulate it based on the model type
                  switch (pOutput->m_modelType)
                  {
                     case MOT_SUM:
                        pOutput->m_value += value;
                        break;
                        /*
                        case MOT_AREAWTMEAN:
                        pOutput->m_value += value * area;
                        break;

                        case MOT_PCTAREA:
                        pOutput->m_value += area;
                        break;
                        */
                  }
               }  // end of: if( passed constraints)
            }  // end of: for ( each model )
         }  // end of: for ( each group )
      }  // end of: for ( each row )
   }  // end of: if ( moCountNode > 0 )

   float totalSubcatchArea = 0;

   // iterate through Subcatchments to collect output if any of the expressions are Subcatch-domain
   if (moCountSubcatch > 0)
   {
      int colAREA = m_pSubcatchLayer->GetFieldCol("AREA");

      for (MapLayer::Iterator subcatch = m_pSubcatchLayer->Begin(); subcatch < m_pSubcatchLayer->End(); subcatch++)
      {
         if (m_pQE_Subcatch)
            m_pQE_Subcatch->SetCurrentRecord(subcatch);

         m_pME_Subcatch->SetCurrentRecord(subcatch);

         float area = 0;
         m_pSubcatchLayer->GetData(subcatch, colAREA, area);
         totalSubcatchArea += area;

         for (int j = 0; j < (int)m_modelOutputGroupArray.GetSize(); j++)
         {
            ModelOutputGroup* pGroup = m_modelOutputGroupArray[j];

            for (int i = 0; i < (int)pGroup->GetSize(); i++)
            {
               ModelOutput* pOutput = pGroup->GetAt(i);

               if (pOutput->m_inUse == false || pOutput->m_modelDomain != MOD_SUBCATCH)
                  continue;

               bool passConstraints = true;

               if (pOutput->m_pQuery)
               {
                  bool result = false;
                  bool ok = pOutput->m_pQuery->Run(subcatch, result);

                  if (result == false)
                     passConstraints = false;
               }

               // did we pass the constraints?  If so, evaluate preferences
               if (passConstraints)
               {
                  pOutput->m_totalQueryArea += area;
                  float value = 0;

                  if (pOutput->m_pMapExpr != NULL)
                  {
                     bool ok = m_pME_Subcatch->EvaluateExpr(pOutput->m_pMapExpr, pOutput->m_pQuery ? true : false);
                     if (ok)
                        value = (float)pOutput->m_pMapExpr->GetValue();
                  }

                  // we have a value for this Subcatch, accumulate it based on the model type
                  switch (pOutput->m_modelType)
                  {
                     case MOT_SUM:
                        pOutput->m_value += value;
                        break;

                     case MOT_AREAWTMEAN:
                        pOutput->m_value += value * area;
                        break;

                     case MOT_PCTAREA:
                        pOutput->m_value += area;
                        break;
                        //case MO_PCT_CONTRIB_AREA:
                        //   return true;
                  }
               }  // end of: if( passed constraints)
            }  // end of: for ( each model )
         }  // end of: for ( each group )
      }  // end of: for ( each Subcatch )
   }  // end of: if ( moCountSubcatch > 0 )

// Last, look for IDU specific values that are only stored in the HRUs.
// This should be used if we have IDU specific queries AND model results that are not being written from HRUs to IDUs.
// This will occur anytime we do not write to the screen
   totalIDUArea = 0;
   if (moCountHruIdu > 0)
   {
      for (int h = 0; h < (int)this->m_hruArray.GetSize(); h++)
      {
         HRU* pHRU = m_hruArray[h];

         for (int z = 0; z < pHRU->m_polyIndexArray.GetSize(); z++)
         {
            int idu = pHRU->m_polyIndexArray[z];       // Here evaluate all the polygons - 

            if (m_pQE_IDU)
               m_pQE_IDU->SetCurrentRecord(idu);

            m_pME_IDU->SetCurrentRecord(idu);

            float area = pHRU->m_HRUeffArea_m2;
            totalIDUArea += area;

            // update static HRU variables
            HRU::m_mvDepthMelt = pHRU->m_depthMelt * 1000.0f;  // volume of water in snow (converted from m to mm)
            HRU::m_mvDepthSWE_mm = pHRU->m_depthSWE * 1000.0f;   // snow water equivalent, mm (includes both ice and meltwater in the snowpack)
            HRU::m_mvTopSoilH2O_mm = pHRU->m_HRUeffArea_m2 > 0 ? (((pHRU->GetLayer(BOX_NAT_SOIL)->m_volumeWater + pHRU->GetLayer(BOX_IRRIG_SOIL)->m_volumeWater) / pHRU->m_HRUeffArea_m2) * 1000.f) : 0.;
            HRU::m_mvShallowGW_mm = pHRU->m_HRUeffArea_m2 > 0 ? ((pHRU->GetLayer(BOX_FAST_GW)->m_volumeWater / pHRU->m_HRUeffArea_m2) * 1000.f) : 0.;
            HRU::m_mvDeepGW_mm = pHRU->m_HRUeffArea_m2 > 0 ? ((pHRU->GetLayer(BOX_SLOW_GW)->m_volumeWater / pHRU->m_HRUeffArea_m2) * 1000.f) : 0.;
            HRU::m_mvCurrentPrecip = pHRU->m_currentPrecip;
            HRU::m_mvRainThrufall_mm = pHRU->m_rainThrufall_mm;
            HRU::m_mvRainEvap_mm = pHRU->m_rainEvap_mm;
            HRU::m_mvSnowThrufall_mmSWE = pHRU->m_snowThrufall_mm;
            HRU::m_mvSnowEvap_mmSWE = pHRU->m_snowEvap_mm;
            HRU::m_mvMelt_mm = pHRU->m_melt_mm;
            HRU::m_mvRefreezing_mm = pHRU->m_refreezing_mm;
            HRU::m_mvInfiltration_mm = pHRU->m_infiltration_mm;
            HRU::m_mvRechargeToIrrigatedSoil_mm = pHRU->m_rechargeToIrrigatedSoil_mm;
            HRU::m_mvRechargeToNonIrrigatedSoil_mm = pHRU->m_rechargeToNonIrrigatedSoil_mm;
            HRU::m_mvRechargeTopSoil_mm = pHRU->m_rechargeTopSoil_mm;
            HRU::m_mvRechargeToUpperGW_mm = pHRU->m_rechargeToUpperGW_mm;
            HRU::m_mvQ0_mm = pHRU->m_Q0_mm;
            HRU::m_mvQ2_mm = pHRU->m_Q2_mm;
            HRU::m_mvPercolation_mm = pHRU->m_percolation_mm;
            HRU::m_mvCurrentAirTemp = pHRU->m_currentAirTemp;
            HRU::m_mvCurrentMinTemp = pHRU->m_currentMinTemp;
            HRU::m_mvCurrentET = pHRU->m_currentET;
            HRU::m_mvCurrentMaxET = pHRU->m_currentMaxET;
            HRU::m_mvCurrentRecharge = pHRU->m_currentGWRecharge;
            HRU::m_mvCurrentGwFlowOut = pHRU->m_currentGWFlowOut;
            HRU::m_mvCumRunoff = pHRU->m_runoff_yr;
            HRU::m_mvCumP = pHRU->m_precip_yr;
            HRU::m_mvCumET = pHRU->m_et_yr;
            HRU::m_mvCumMaxET = pHRU->m_maxET_yr;
            HRU::m_mvCumRecharge = pHRU->m_gwRecharge_yr;
            HRU::m_mvCumGwFlowOut = pHRU->m_gwFlowOut_yr;
            HRU::m_mvHRU_TO_AQ_mm = pHRU->m_aquifer_recharge_mm;

            for (int j = 0; j < (int)m_modelOutputGroupArray.GetSize(); j++)
            {
               ModelOutputGroup* pGroup = m_modelOutputGroupArray[j];

               for (int i = 0; i < (int)pGroup->GetSize(); i++)
               {
                  ModelOutput* pOutput = pGroup->GetAt(i);

                  if (pOutput->m_inUse == false || pOutput->m_modelDomain != MOD_HRU_IDU)
                     continue;

                  bool passConstraints = true;

                  if (pOutput->m_pQuery)
                  {
                     bool result = false;
                     bool ok = pOutput->m_pQuery->Run(idu, result);

                     if (result == false)
                        passConstraints = false;
                  }

               // did we pass the constraints?  If so, evaluate preferences
                  if (passConstraints)
                  {
                     pOutput->m_totalQueryArea += area;
                     float value = 0;

                     if (pOutput->m_pMapExpr != NULL)
                     {
                        bool ok = m_pME_IDU->EvaluateExpr(pOutput->m_pMapExpr, pOutput->m_pQuery ? true : false);
                        if (ok)
                           value = (float)pOutput->m_pMapExpr->GetValue();
                     }

                  // we have a value for this IDU, accumulate it based on the model type
                     switch (pOutput->m_modelType)
                     {
                        case MOT_SUM:
                           pOutput->m_value += value;
                           break;

                        case MOT_AREAWTMEAN:
                           pOutput->m_value += value * area;
                           break;

                        case MOT_PCTAREA:
                           pOutput->m_value += area;
                           break;
                        //case MO_PCT_CONTRIB_AREA:
                        //   return true;
                     }
                  }
               }  // end of: if( passed constraints)
            }  // end of: for ( each model )
         }  // end of: for ( each output group )
      }  // end of: for ( each HRU )
   }  // end of: if ( moCountHru > 0 )



   // do any fixup needed and add to data object for each group
   int day_index = (int)m_timeInRun;
   int year_index = m_flowContext.pEnvContext->yearOfRun;
   int doy0 = (int)(m_timeInRun - m_yearStartTime);
   bool last_day_of_year = doy0 == (m_flowContext.pEnvContext->daysInCurrentYear - 1);
   bool last_day_of_run = false;
   if (last_day_of_year) last_day_of_run = (m_flowContext.pEnvContext->currentYear + 1) >= m_flowContext.pEnvContext->endYear;
   int moy0 = -1;
   int days_left = doy0; while (days_left >= 0 && moy0 < 11) { moy0++; days_left -= GetDaysInMonth0(moy0, m_flowContext.pEnvContext->daysInCurrentYear); }
   int dom0 = m_flowContext.pEnvContext->m_simDate.day - 1;
   int month_index = 12 * m_flowContext.pEnvContext->yearOfRun + moy0;
   bool last_day_of_month = days_left == -1;
   bool last_day_of_water_year = false;
   if (last_day_of_month && m_flowContext.pEnvContext->m_simDate.month == 9) last_day_of_water_year = true;
   for (int j = 0; j < (int)m_modelOutputGroupArray.GetSize(); j++)
   {
      ModelOutputGroup* pGroup = m_modelOutputGroupArray[j];

      // any outputs?
      int moCount = 0;
      for (int i = 0; i < (int)pGroup->GetSize(); i++)
      {
         ModelOutput* pOutput = pGroup->GetAt(i);
         if (pOutput->m_inUse)
            moCount++;
         if (pOutput->m_pDataObjObs != NULL) //add 1 column if we have observations for this point.
            moCount++;
      }

      if (moCount == 0)
         continue;

      int cols_to_skip = pGroup->m_moInterval == MOI_UNDEFINED ? 1 : 4;
      float* outputArray = new float[moCount + cols_to_skip];
      outputArray[0] = (float)m_timeInRun;
      if (cols_to_skip == 4)
      {
         outputArray[1] = (float)GetCurrentYear();
         outputArray[2] = (float)(moy0 + 1);
         outputArray[3] = (float)(((pGroup->m_moInterval == MOI_YEARLY || pGroup->m_moInterval == MOI_WATERYEAR_YEARLY) ? doy0 : dom0) + 1);

         switch (pGroup->m_moInterval)
         {
            case MOI_YEARLY:
               outputArray[1] = (float)GetCurrentYear();
               outputArray[2] = (float)(moy0 + 1);
               outputArray[3] = (float)(doy0 + 1);
               break;
            case MOI_MONTHLY:
            case MOI_DAILY:
               outputArray[1] = (float)GetCurrentYear();
               outputArray[2] = (float)(moy0 + 1);
               outputArray[3] = (float)(dom0 + 1);
               break;
            case MOI_WATERYEAR_YEARLY:
               outputArray[1] = (float)GetCurrentYear();
               outputArray[2] = (float)(moy0 + 1);
               outputArray[3] = (float)(doy0 + 1);
               if (moy0 <= 8)
               { // Jan-Sep are months 4-12 of the water year.
                  if (m_flowContext.pEnvContext->yearOfRun > 0)
                  { // This is not the first year of the run
                     outputArray[2] += 3;
                     outputArray[3] += 92; // Days in Oct-Nov of the preceding year
                  }
               }
               else
               { // Oct-Dec. We're in the next water year. Oct-Dec are months 1-3 of the water year.
                  outputArray[1]++; // Water years are named by the year in which they end.
                  outputArray[2] -= 9;
                  outputArray[3] -= m_flowContext.pEnvContext->m_doy0ofOct1;
               }
               break;
            case MOI_WATERYEAR_MONTHLY:
            case MOI_WATERYEAR_DAILY:
               outputArray[1] = (float)GetCurrentYear();
               outputArray[2] = (float)(moy0 + 1);
               outputArray[3] = (float)(dom0 + 1);
               if (moy0 <= 8)
               { // Jan-Sep are months 4-12 of the water year.
                  outputArray[2] += 3;
               }
               else
               { // Oct-Dec. We're in the next water year. Oct-Dec are months 1-3 of the water year.
                  outputArray[1]++; // Water years are named by the year in which they end.
                  outputArray[2] -= 9;
               }
               break;
            case MOI_END_OF_YEAR:
               outputArray[2] = 12;
               outputArray[3] = 31;
               break;
            default: ASSERT(0); break;
         } // end of switch(pGroup->m_moInterval)
      } // end of if (cols_to_skip == 4)

      int index = cols_to_skip;

      for (int i = 0; i < (int)pGroup->GetSize(); i++)
      {
         ModelOutput* pOutput = pGroup->GetAt(i);

         if (pOutput->m_inUse)
         {
            switch (pOutput->m_modelType)
            {
               case MOT_SUM:
                  break;      // no fixup required

               case MOT_AREAWTMEAN:
                  pOutput->m_value /= pOutput->m_totalQueryArea;      //totalArea;
                  break;

               case MOT_PCTAREA:
                  pOutput->m_value /= totalIDUArea;
            }

            outputArray[index++] = pOutput->m_value;
         }
         if (pOutput->m_pDataObjObs != NULL)
         {
            ASSERT(m_flowContext.pEnvContext->currentYear >= 1900);
            int days_since_1900Jan1 = 0;
            int year = 1900;
            while (year < m_flowContext.pEnvContext->currentYear)
            {
               days_since_1900Jan1 += m_flowContext.pEnvContext->m_maxDaysInYear > 365 ? GetDaysInCalendarYear(year) : m_flowContext.pEnvContext->m_maxDaysInYear;
               year++;
            }
            days_since_1900Jan1 += m_flowContext.dayOfYear;
            int obs_day_number = pOutput->m_dayNumberOfObsFor1900Jan1 + days_since_1900Jan1;
            outputArray[index++] = pOutput->m_pDataObjObs->IGet((float)obs_day_number);

            if (m_flowContext.dayOfYear == 0 && m_flowContext.pEnvContext->yearOfRun==0)
            { // Warn once if calendar of observations might be inconsistent with calendar of climate data
               if (!((m_flowContext.pEnvContext->m_maxDaysInYear == 365 && pOutput->m_dayNumberOfObsFor1900Jan1 == 693500) ||
                  (m_flowContext.pEnvContext->m_maxDaysInYear == 366 && pOutput->m_dayNumberOfObsFor1900Jan1 == 0)))
               {
                  CString msg;
                  msg.Format("Flow: 'format' attribute in <output> tag for %s: the year lengths in observations may be incompatible with the year lengths in the climate data",
                     pOutput->m_name);
                  Report::WarningMsg(msg);
               }
            }
         }
      }

      pGroup->m_pDataObj->AppendRow(outputArray, moCount + cols_to_skip);

      int rows_to_aggregate = 1;
      float col0_val = (float)m_timeInRun;
      switch (pGroup->m_moInterval)
      {
         case MOI_YEARLY:
            if (last_day_of_year) rows_to_aggregate = m_flowContext.pEnvContext->daysInCurrentYear;
            col0_val = (float)year_index;
            break;
         case MOI_MONTHLY:
            if (last_day_of_month) rows_to_aggregate = ::GetDaysInMonth0(moy0, m_flowContext.pEnvContext->daysInCurrentYear);
            col0_val = (float)month_index;
            break;
         case MOI_WATERYEAR_YEARLY:
            if (last_day_of_water_year)
            {
               rows_to_aggregate = min(m_flowContext.pEnvContext->daysInCurrentYear, pGroup->m_pDataObj->GetRowCount());
               col0_val = (float)year_index;
               CString msg;
               msg.Format("CollectModelOutput() on last_day_of_water_year: rows_to_aggregate = %d, m_flowContext.pEnvContext->daysInCurrentYear = %d, pGroup->m_pDataObj->GetRowCount() = %d",
                  rows_to_aggregate, m_flowContext.pEnvContext->daysInCurrentYear, pGroup->m_pDataObj->GetRowCount());
               Report::LogMsg(msg);
            }
            else if (last_day_of_year && last_day_of_run)
            {
               rows_to_aggregate = 92; // 92 days in Oct-Nov-Dec
               col0_val = (float)year_index + 1;
            }
            break;
         case MOI_WATERYEAR_MONTHLY:
            if (last_day_of_month) rows_to_aggregate = ::GetDaysInMonth0(moy0, m_flowContext.pEnvContext->daysInCurrentYear);
            col0_val = (float)month_index;
            break;
         default:
            rows_to_aggregate = 1;
            col0_val = (float)m_timeInRun;
            break;
      }

      if (rows_to_aggregate > 1)
      { // Go back and aggregate the last n rows to make monthly and yearly outputs
         outputArray[0] = col0_val;
         for (int col_ndx = cols_to_skip; col_ndx < moCount + cols_to_skip; col_ndx++) outputArray[col_ndx] = 0.f;
         int rows_in_DataObj = pGroup->m_pDataObj->GetRowCount();
         float* final_values = new float[moCount + cols_to_skip];
         for (int col_ndx = cols_to_skip; col_ndx < moCount + cols_to_skip; col_ndx++)
            final_values[col_ndx] = pGroup->m_pDataObj->GetAsFloat(col_ndx, rows_in_DataObj - 1);

         for (int row_ndx = rows_in_DataObj - 1; row_ndx > ((rows_in_DataObj - 1) - rows_to_aggregate); row_ndx--)
         {
            if (pGroup->m_moInterval != MOI_END_OF_YEAR) for (int col_ndx = cols_to_skip; col_ndx < moCount + cols_to_skip; col_ndx++)
               outputArray[col_ndx] += pGroup->m_pDataObj->GetAsFloat(col_ndx, row_ndx);
            pGroup->m_pDataObj->DeleteRow(row_ndx);
         }
         for (int col_ndx = cols_to_skip; col_ndx < moCount + cols_to_skip; col_ndx++)
            if (pGroup->m_moInterval == MOI_END_OF_YEAR) outputArray[col_ndx] = final_values[col_ndx];
            else outputArray[col_ndx] /= rows_to_aggregate;
         pGroup->m_pDataObj->AppendRow(outputArray, moCount + cols_to_skip);
         delete[] final_values;
      }

      delete[] outputArray;
   } // end of for ( int j=0; j < (int) m_modelOutputGroupArray.GetSize(); j++ )



   clock_t finish = clock();

   float duration = (float)(finish - start) / CLOCKS_PER_SEC;
   gpModel->m_outputDataRunTime += duration;

   return true;
} // end of CollectModelOutput()

int FlowModel::AddModelVar( LPCTSTR label, LPCTSTR varName, MTDOUBLE *value )
   {
   int rtnval = AddModelVar(m_pME_IDU, label, varName, value);
   return(rtnval);
   }
    
int FlowModel::AddModelVar(MapExprEngine * pME, LPCTSTR label, LPCTSTR varName, MTDOUBLE *value)
{
   ASSERT(pME != NULL);

   ModelVar *pModelVar = new ModelVar(label);

   pME->AddVariable(varName, value);

   return (int)m_modelVarArray.Add(pModelVar);
}


void FlowModel::ApplyMacros( CString &str )
   {
   if ( str.Find( '{' ) < 0 )
      return;

   // get the current scenario
   int scnIndex = this->m_currentEnvisionScenarioIndex;

   if ( scnIndex < 0 )
      return;

   // iterate through the macros, applying substitutions as needed
   for ( int i=0; i < (int) this->m_macroArray.GetSize(); i++ )
      {
      ScenarioMacro *pMacro = this->m_macroArray[ i ];
      
      // for this macro, see if a value is defined for the current scenario.
      // if so, use it; if not, use the default value
      CString value( pMacro->m_defaultValue );
      for ( int j=0; j < (int) pMacro->m_macroArray.GetSize(); j++ )
         {
         SCENARIO_MACRO *pScn = pMacro->m_macroArray[ j ];

         int index = -1;
         if (::EnvGetScenarioFromName(pScn->envScenarioName, &index) != NULL)
            {
            if (index == scnIndex)
               {
               value = pScn->value;
               break;
               }
            }
         }

      CString strToReplace = "{" + pMacro->m_name + "}";
      str.Replace( strToReplace, value );
      }
   }
   

/* Here is what was in WW2100 ver. 471 ReachRouting.cpp on 12/8/20

void Reach::SetGeometry( float wdRatio )
   {
   //ASSERT( pReach->m_qArray != NULL );
   ReachSubnode *pNode = (ReachSubnode*) m_subnodeArray[ 0 ];
   ASSERT( pNode != NULL );

   m_depth = GetDepthFromQ( pNode->m_discharge, wdRatio );
   m_width = wdRatio * m_depth;
   m_wdRatio = wdRatio;
   }


//???? CHECK/DOCUMENT UNITS!!!!
float Reach::GetDepthFromQ( float Q, float wdRatio )  // ASSUMES A SPECIFIC CHANNEL GEOMETRY
   {
   float depth;
   if (Q > 0.f)
      {
      // Q=m3/sec
      // from kellie:  d = ( 5/9 Q n s^2 ) ^ 3/8   -- assumes width = 3*depth, rectangular channel
      float wdterm = (float)pow((wdRatio / (2 + wdRatio)), 2.0f / 3.0f)*wdRatio;
      depth = (float)pow(((Q*m_n) / ((float)sqrt(m_slope)*wdterm)), 3.0f / 8.0f);
      }
   else depth = 0.f;
   return depth;
   } // end of GetDepthFromQ()

///////////////////////////////////////////////////////////////////
//  additional functions

void SetGeometry( Reach *pReach, float discharge )
   {
   ASSERT( pReach != NULL );
   pReach->m_depth = GetDepthFromQ(pReach, discharge, pReach->m_wdRatio );
   pReach->m_width = pReach->m_wdRatio * pReach->m_depth;
   }

float GetDepthFromQ( Reach *pReach, float Q, float wdRatio )  // ASSUMES A SPECIFIC CHANNEL GEOMETRY
   {
  // from kellie:  d = ( 5/9 Q n s^2 ) ^ 3/8   -- assumes width = 3*depth, rectangular channel
   float wdterm = (float) pow( (wdRatio/( 2 + wdRatio )), 2.0f/3.0f)*wdRatio;
   float depth  = (float) pow(((Q*pReach->m_n)/((float)sqrt(pReach->m_slope)*wdterm)), 3.0f/8.0f);
   if (depth != depth) depth = 0.00001f;
   return depth;
   }

*/

void ReachSubnode::SetSubreachGeometry(double volume_m3, double wdRatio)
{
   double xsec_m2 = volume_m3 / m_subreach_length_m;
   m_subreach_depth_m = xsec_m2 / (1. + wdRatio);
   m_subreach_width_m = m_subreach_depth_m * wdRatio;
   m_subreach_surf_area_m2 = m_subreach_width_m * m_subreach_length_m;
} // end of SetSubreachGeometry(volume_m3, wdRatio)


void ReachSubnode::SetSubreachGeometry(double volume_m3, double dummy, double widthGiven_m)
{
   m_subreach_width_m = widthGiven_m;
   m_subreach_surf_area_m2 = widthGiven_m * m_subreach_length_m;
   m_subreach_depth_m = volume_m3 / m_subreach_surf_area_m2;
} // end of SetSubreachGeometry(volume_m3, dummy, widthGiven_m)


float Reach::GetManningDepthFromQ(double Q, float wdRatio)  // ASSUMES A SPECIFIC CHANNEL GEOMETRY
{
   float depth;
   if (Q > 0.f)
   {
   // Q=m3/sec
   // from kellie:  d = ( 5/9 Q n s^2 ) ^ 3/8   -- assumes width = 3*depth, rectangular channel
      float wdterm = (float)pow((wdRatio / (2 + wdRatio)), 2.0f / 3.0f) * wdRatio;
      depth = (float)pow(((Q * m_n) / ((double)sqrt(m_slope) * wdterm)), 3.0 / 8.0);
   }
   else depth = 0.f;
   return depth;
} // end of GetManningDepthFromQ()


float GetManningDepthFromQ(Reach* pReach, double Q, float wdRatio)  // ASSUMES A SPECIFIC CHANNEL GEOMETRY
{
// from kellie:  d = ( 5/9 Q n s^2 ) ^ 3/8   -- assumes width = 3*depth, rectangular channel
   float wdterm = (float)pow((wdRatio / (2 + wdRatio)), 2.0f / 3.0f) * wdRatio;
   float depth = (float)pow(((Q * pReach->m_n) / ((float)sqrt(pReach->m_slope) * wdterm)), 3.0f / 8.0f);
   if (depth != depth) depth = 0.00001f;
   return depth;
}


float GetManningDepthFromQ(double Q, float wdRatio, float n, float slope)  // ASSUMES A SPECIFIC CHANNEL GEOMETRY
{
// from kellie:  d = ( 5/9 Q n s^2 ) ^ 3/8   -- assumes width = 3*depth, rectangular channel
   float wdterm = (float)pow((wdRatio / (2 + wdRatio)), 2.0f / 3.0f) * wdRatio;
   float depth = (float)pow(((Q * n) / ((float)sqrt(slope) * wdterm)), 3.0f / 8.0f);
   if (depth != depth) depth = 0.00001f;
   return depth;
} // end of GetManningDepthFromQ(double Q, float wdRatio, float n, float slope)


// figure out what kind of flux it is
// source string syntax= fn:<dllpath:function> for functional, db:<datasourcepath:columnname> for datasets
FLUXSOURCE ParseSource( LPCTSTR _sourceStr, CString &path, CString &source, HINSTANCE &hDLL, FLUXFN &fn )
   {
   TCHAR sourceStr[ 256 ];
   lstrcpy( sourceStr, _sourceStr );
      
   LPTSTR equal = _tcschr( sourceStr, '=' );
   if ( equal == NULL )
      {
      CString msg;
      msg.Format( "Flow: Error parsing source string '%s'.  The source string must start with either 'fn=' or 'db=' for functional or database sources, respectively", 
            sourceStr );
      Report::ErrorMsg( msg );
      return  FS_UNDEFINED;
      }

   if ( *sourceStr == 'f' )
      {
      //pFluxInfo->m_sourceLocation = FS_FUNCTION;
      LPTSTR _path = equal+1;
      LPTSTR colon = _tcschr( _path, ':' );

      if ( colon == NULL )
         {
         CString msg;
         msg.Format( "Flow: Error parsing source string '%s'.  The source string must be of the form 'fn=dllpath:entrypoint' ", 
                sourceStr );
         Report::ErrorMsg( msg );
         return  FS_UNDEFINED;
         }

      *colon = NULL;
      path = _path;
      path.Trim();
      source = colon+1;
      source.Trim();

      hDLL = AfxLoadLibrary( path ); 
      if ( hDLL == NULL )
         {
         CString msg( _T( "Flow error: Unable to load ") );
         msg += path;
         msg += _T(" or a dependent DLL" );
         Report::ErrorMsg( msg );
         return FS_UNDEFINED;
         }

      fn = (FLUXFN) GetProcAddress( hDLL, source );
      if ( fn == NULL )
         {
         CString msg( "Flow: Unable to find Flow function '");
         msg += source;
         msg += "' in ";
         msg += path;
         Report::ErrorMsg( msg );
         AfxFreeLibrary( hDLL );
         hDLL = NULL;
         return FS_UNDEFINED;         
         }
      return FS_FUNCTION;
      }
   else if ( *sourceStr =='d' )
      {
      //m_sourceLocation = FS_DATABASE;
      LPTSTR _path = equal+1;
      LPTSTR colon = _tcschr( _path, ':' );

      if ( colon == NULL )
         {
         CString msg;
         msg.Format( "Flow: Error parsing Flow source string '%s'.  The source string must be of the form 'db=dllpath:entrypoint' ", 
               sourceStr );
         Report::ErrorMsg( msg );
         return FS_UNDEFINED;
         }

      *colon = NULL;
      path = path;
      path.Trim();
      source = colon+1;
      source.Trim();

      /*
      //int rows = pFluxInfo->m_pData->ReadAscii( pFluxInfo->m_path );  // ASSUMES CSV FOR NOW????
      //if ( rows <= 0  )
      //   {
      //   CString msg( _T( "Flux datasource error: Unable to open/read ") );
      //   msg += path;
      //   Report::ErrorMsg( msg );
      //   pFluxInfo->m_use = false;
      //   continue;
      //   }

      col = pFluxInfo->m_pData->GetCol( pFluxInfo->m_fieldName );
      if ( pFluxInfo->m_col < 0 )
         {
         CString msg( "Unable to find Flux database field '");
         msg += pFluxInfo->m_fieldName;
         msg += "' in ";
         msg += path;
         Report::ErrorMsg( msg );
         pFluxInfo->m_use = false;
         continue;
         } */
      }

   return FS_UNDEFINED;   
   }



float GetVerticalDrainage( float wc )
   {
   //  For Loamy Sand (taken from Handbook of Hydrology).  .
   float wp = 0.035f; 
   float porosity = 0.437f;
   float lamda = 0.553f ; 
   float kSat = 1.0f;//m/d
   float ratio = (wc - wp)/(porosity-wp);
   float power = 3.0f+(2.0f/lamda);
   float kTheta = (kSat) * pow(ratio,power);
   
   if (wc < wp) //no flux if too dry
      kTheta=0.0f;

   return kTheta ;//m/d 
   }


float GetBaseflow(float wc)
   {
   float k1 = 0.01f;
   float baseflow = wc*k1;
   return baseflow;//m/d
   }


float CalcRelHumidity(float specHumid, float tMean, float elevation)
   {
   // Atmospheric Pressure; kPa
   // p28, eq34
   float P = 101.3f * (float) pow(((293 - 0.0065 * elevation) / 293), 5.26);
   float e = specHumid/0.622f * P;
   // temp in deg C
   float E = 6.112f * (float) exp(17.62*tMean/(243.12+tMean));
   // rel. humidity [-]
   float ret = e/E;

   return ret;
   }


Reach * FlowModel::GetReachFromCOMID(int comid)
{
   Reach* pReach = NULL;

   int reach_count = (int)m_reachArray.GetSize();
   int reach_array_ndx;
   for (reach_array_ndx = 0; reach_array_ndx < reach_count; reach_array_ndx++)
   {
      pReach = m_reachArray[reach_array_ndx];
      if (pReach->m_reachID == comid) break;
   } // end of loop thru m_reachArray

   if (reach_array_ndx >= reach_count) pReach = NULL;

   return(pReach);
} // end of GetReachFromCOMID()


float GroundWaterRechargeFraction(float waterDepth, float FC, float Beta)
{
   float value = 0.0f;
   float lossFraction = (pow((waterDepth / FC), Beta));
   if (lossFraction > 1.0f)
      lossFraction = 1.0f;
   return lossFraction;
}


bool HRU::WetlSurfH2Ofluxes(double precip_mm, double fc, double Beta, 
   double * pPrecip2WetlSurfH2O_m3, double* pWetl2TopSoil_m3, double * pWetl2SubSoil_m3, double* pWetl2Reach_m3) 
// Updates IDU attributes WETNESS, FLOODDEPTH, and SM_DAY.
{
   *pPrecip2WetlSurfH2O_m3 = 0.;
   *pWetl2TopSoil_m3 = 0.;
   *pWetl2SubSoil_m3 = 0.;
   *pWetl2Reach_m3 = 0.;
   if (m_wetlandArea_m2 <= 0.) 
      return(false);
   bool ret_val = true;

   double hru_to_topsoil_m3 = 0.;
   double hru_to_subsoil_m3 = 0.;
   double hru_to_reach_m3 = 0.;
   int idus_in_hru = (int)m_polyIndexArray.GetSize();
   m_standingH2Oflag = false;

   for (int i = 0; i < idus_in_hru; i++)
   {
      int idu_poly_ndx = m_polyIndexArray[i];
      int lulc_a = gIDUs->AttInt(idu_poly_ndx, LULC_A);
      bool is_wetland = lulc_a == LULCA_WETLAND;
      if (!is_wetland) continue;

      double idu_to_topsoil_mm = 0.;
      double idu_to_subsoil_mm = 0.;
      double idu_to_reach_mm = 0.;

      // In this IDU, how much potential surface water is there?
      float sm_day_mm = gIDUs->AttFloat(idu_poly_ndx, SM_DAY); // This is the water in the NAT_SOIL compartment for this IDU as of yesterday.
      double wetness_mm = gIDUs->Att(idu_poly_ndx, WETNESS);
      double flooddepth_mm = gIDUs->Att(idu_poly_ndx, FLOODDEPTH);
      ASSERT(wetness_mm > 0 || flooddepth_mm == 0);
      ASSERT(flooddepth_mm == 0 || wetness_mm == gIDUs->Att(idu_poly_ndx, WETL_CAP));
      double idu_surf_h2o_mm = precip_mm + (wetness_mm > 0. ? wetness_mm : 0.) + flooddepth_mm;
      if (idu_surf_h2o_mm <= 0.)
      { // There isn't any water available in this IDU to infiltrate into the soil.
          wetness_mm = -(fc - sm_day_mm);
         if (wetness_mm > 0.)
         {
            ASSERT(close_enough(0, wetness_mm, 1e-4, 1));
            wetness_mm = 0.;
         }
         gFlowModel->SetAtt(idu_poly_ndx, WETNESS, wetness_mm);
         ASSERT(flooddepth_mm == 0);
         continue; 
      }

      // How will the water that drains from the surface into the soil be
      // divided between the topsoil and the subsoil?
      // frac_to_subsoil is the proportion of surface water that bypasses the topsoil bucket, and is added directly to the subsoil
      double frac_to_subsoil = GroundWaterRechargeFraction((float)sm_day_mm, (float)fc, (float)Beta);
      ASSERT(0. <= frac_to_subsoil && frac_to_subsoil <= 1.);

      // In this IDU, how much room is there in the soil for more water to infiltrate??
      double idu_topsoil_room_mm = fc - sm_day_mm; if (idu_topsoil_room_mm < 0.) idu_topsoil_room_mm = 0.;
      double idu_total_soil_room_mm = frac_to_subsoil >= 1 ? 0 : idu_topsoil_room_mm / (1. - frac_to_subsoil);

      if (idu_total_soil_room_mm > idu_surf_h2o_mm)
      { // There is more room in the soil than there is standing water.
         // Drain all the surface water out of this IDU into the soil.
         idu_to_topsoil_mm = idu_surf_h2o_mm * (1. - frac_to_subsoil);
         idu_to_subsoil_mm = idu_surf_h2o_mm - idu_to_topsoil_mm;
         idu_surf_h2o_mm = 0.;
         idu_topsoil_room_mm -= idu_to_topsoil_mm;
         wetness_mm = -idu_topsoil_room_mm;
         ASSERT(wetness_mm < 0.);
      } // end of if (idu_total_soil_room_mm > idu_surf_h2o_mm)
      else
      { // There is enough standing water to fill up the room in the soil.
         // Drain enough surface water out of this IDU to saturate the soil.
         idu_to_topsoil_mm = idu_topsoil_room_mm;
         idu_to_subsoil_mm = idu_total_soil_room_mm - idu_topsoil_room_mm;
         idu_surf_h2o_mm -= idu_total_soil_room_mm;

         // Divide up the surface water between WETNESS and FLOODDEPTH.
         double wetl_cap_mm = gIDUs->Att(idu_poly_ndx, WETL_CAP);
         wetness_mm = idu_surf_h2o_mm > wetl_cap_mm ? wetl_cap_mm : idu_surf_h2o_mm;
         flooddepth_mm = idu_surf_h2o_mm > wetl_cap_mm ? idu_surf_h2o_mm - wetness_mm : 0;
//x         wetness_mm = idu_surf_h2o_mm; 
         ASSERT(wetness_mm >= 0.);
      } // end of if (idu_total_soil_room_mm > idu_surf_h2o_mm) ... else

      // Update the IDU attributes WETNESS, FLOODDEPTH, and SM_DAY.
      ASSERT(wetness_mm > 0 || flooddepth_mm == 0);
      ASSERT(flooddepth_mm == 0 || wetness_mm == gIDUs->Att(idu_poly_ndx, WETL_CAP));
      gFlowModel->SetAtt(idu_poly_ndx, WETNESS, wetness_mm);
      gFlowModel->SetAtt(idu_poly_ndx, FLOODDEPTH, flooddepth_mm);
      if (wetness_mm > 0.) m_standingH2Oflag = true;
      sm_day_mm += (float)idu_to_topsoil_mm;
      gFlowModel->SetAttFloat(idu_poly_ndx, SM_DAY, (float)sm_day_mm);

      // Convert water depths to water volumes.
      float idu_area_m2 = gIDUs->AttFloat(idu_poly_ndx, AREA);
      double idu_surf_h2o_m3 = (idu_surf_h2o_mm / 1000.) * idu_area_m2;
      double idu_to_topsoil_m3 = (idu_to_topsoil_mm / 1000.) * idu_area_m2;
      double idu_to_subsoil_m3 = (idu_to_subsoil_mm / 1000.) * idu_area_m2;
      double flooddepth_m3 = (flooddepth_mm / 1000.) * idu_area_m2;

      // Add to the HRU fluxes.
      hru_to_topsoil_m3 += idu_to_topsoil_m3;
      hru_to_subsoil_m3 += idu_to_subsoil_m3;
      hru_to_reach_m3 += flooddepth_m3; 

      // Add the flood water to the reach.
      int comid = gIDUs->AttInt(idu_poly_ndx, COMID);
      Reach* pReach = gFlowModel->FindReachFromID(comid);
      pReach->AddRunoffToReach(flooddepth_m3);
   } // end of loop through IDUs

   *pPrecip2WetlSurfH2O_m3 = (precip_mm / 1000.) * m_wetlandArea_m2;
   *pWetl2TopSoil_m3 = hru_to_topsoil_m3;
   *pWetl2SubSoil_m3 = hru_to_subsoil_m3;
   *pWetl2Reach_m3 = hru_to_reach_m3;
 
   return(ret_val);
} // end of WetlSurfaceH2Ofluxes()


Wetland* FlowModel::GetWetlandFromID(int wetlID)
{
   int num_wetlands = (int)m_wetlArray.GetSize();
   for (int wetl_ndx = 0; wetl_ndx < num_wetlands; wetl_ndx++)
   {

      Wetland* pWetl = m_wetlArray[wetl_ndx];
      if (pWetl->m_wetlID == wetlID) return(pWetl);
   } // end of loop through wetlands

   return((Wetland*)NULL);
} // end of GetWetlandFromID()


bool HRULayer::AccumAdditions(float h2oEnteringHRUlayer_m3)
{
   bool rtn_val = AddFluxFromGlobalHandler(h2oEnteringHRUlayer_m3, FL_TOP_SOURCE);
   return(rtn_val);
} // end of HRULayer::AccumAdditions()

/*
bool HRULayer::AccumWithdrawals(float h2oLeavingHRUlayer_m3)
{
   bool rtn_val = AddFluxFromGlobalHandler(h2oLeavingHRUlayer_m3, FL_SINK);
   return(rtn_val);
} // end of HRULayer::AccumWithdrawals()
*/


