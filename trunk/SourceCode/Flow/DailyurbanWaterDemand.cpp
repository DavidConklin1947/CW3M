// DailyUrbanWaterDemand.cpp : Implements global daily urban water demands methods for Flow
//

#include "stdafx.h"
#pragma hdrstop

#include "GlobalMethods.h"
#include "Flow.h"
#include "FlowInterface.h"
#include <UNITCONV.H>

using namespace std;

extern FlowProcess *gpFlow;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DailyUrbanWaterDemand::DailyUrbanWaterDemand(LPCTSTR name)
: GlobalMethod(name, GM_URBAN_DEMAND_DY)
, m_colDailyUrbanDemand(-1)// Calculated Daily Urband Water Demand m3/day
, m_colH2OResidnt(-1)		// Annual Residential Demand ccf/day/acre
, m_colH2OIndComm(-1)		// Annual Industrial & commercial Demand ccf/day/acre
, m_colIDUArea(-1)			// IDU area from IDU layer
, m_colUGB(-1)					// IDU UGB from IDU layer	
, m_colWrexist(-1)         // IDU attribute from IDU layer with WR information	
, m_colPop(-1)
, m_iduMetroWaterDmdDy(-1)  // UGB 40 Metro daily urban water demand ccf/day
, m_iduEugSpWaterDmdDy(-1)	 // UGB 22 EugSp daily urban water demand ccf/day
, m_iduSalKiWaterDmdDy(-1)	 // UGB 51 Salki daily urban water demand ccf/day
, m_iduCorvlWaterDmdDy(-1)	 // UGB 12 Corvl daily urban water demand ccf/da
, m_iduAlbnyWaterDmdDy(-1)	 // UGB 02 Albny daily urban water demand ccf/day
, m_iduMcminWaterDmdDy(-1)	 // UGB 39 Mcmin daily urban water demand ccf/day
, m_iduNewbrWaterDmdDy(-1)	 // UGB 47 Newbr daily urban water demand ccf/day
, m_iduWoodbWaterDmdDy(-1)	 // UGB 71 Woodb daily urban water demand ccf/day


{
	this->m_timing = GMT_START_STEP;
}


DailyUrbanWaterDemand::~DailyUrbanWaterDemand()
{

}

bool DailyUrbanWaterDemand::Init(FlowContext *pFlowContext)
{
	MapLayer *pIDULayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

	int iduCount = pIDULayer->GetRecordCount();

	pIDULayer->CheckCol(m_colH2OResidnt, "H2ORESIDNT", TYPE_FLOAT, CC_MUST_EXIST);
	pIDULayer->CheckCol(m_colH2OIndComm, "H2OINDCOMM", TYPE_FLOAT, CC_MUST_EXIST);
	pIDULayer->CheckCol(m_colDailyUrbanDemand, "UDMAND_DY", TYPE_FLOAT, CC_MUST_EXIST);
	pIDULayer->CheckCol(m_colIDUArea, "AREA", TYPE_FLOAT, CC_MUST_EXIST);
	pIDULayer->CheckCol(m_colUGB, "UGB", TYPE_INT, CC_MUST_EXIST);   // urban water 
   pIDULayer->CheckCol(m_colWrexist, "WREXISTS", TYPE_INT, CC_MUST_EXIST);
   pIDULayer->CheckCol(m_colPop, "POP", TYPE_FLOAT, CC_MUST_EXIST);

	bool readOnlyFlag = pIDULayer->m_readOnly;
	pIDULayer->m_readOnly = false;
	pIDULayer->SetColData(m_colDailyUrbanDemand, VData(0), true);
	pIDULayer->m_readOnly = readOnlyFlag;

	this->m_timeSeriesMunDemandSummaries.SetName("Daily Urban Water Demand Summary");
	this->m_timeSeriesMunDemandSummaries.SetSize(9, 0);
	this->m_timeSeriesMunDemandSummaries.SetLabel(0, "Time (days)");
	this->m_timeSeriesMunDemandSummaries.SetLabel(1, "Daily Metro Water Demand Summary (ccf/day)");
	this->m_timeSeriesMunDemandSummaries.SetLabel(2, "Daily Eug-Spr Water Demand Summary (ccf/day)");
	this->m_timeSeriesMunDemandSummaries.SetLabel(3, "Daily Salem-Kei Water Demand Summary (ccf/day)");
	this->m_timeSeriesMunDemandSummaries.SetLabel(4, "Daily Corval Water Demand Summary (ccf/day)");
	this->m_timeSeriesMunDemandSummaries.SetLabel(5, "Daily Albany Water Demand Summary (ccf/day)");
	this->m_timeSeriesMunDemandSummaries.SetLabel(6, "Daily Mcminn Water Demand Summary (ccf/day)");
	this->m_timeSeriesMunDemandSummaries.SetLabel(7, "Daily Newberg Water Demand Summary (ccf/day)");
	this->m_timeSeriesMunDemandSummaries.SetLabel(8, "Daily Woodburn Water Demand Summary (ccf/day)");

	gpFlow->AddOutputVar("Daily Urban Water Demand Summary", &m_timeSeriesMunDemandSummaries, "");

   return TRUE;
}

bool DailyUrbanWaterDemand::StartYear(FlowContext *pFlowContext)
	{
	MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

	m_iduMetroWaterDmdDy = 0.f;
	m_iduEugSpWaterDmdDy = 0.f;
	m_iduSalKiWaterDmdDy = 0.f;
	m_iduCorvlWaterDmdDy = 0.f;
	m_iduAlbnyWaterDmdDy = 0.f;
	m_iduMcminWaterDmdDy = 0.f;
	m_iduNewbrWaterDmdDy = 0.f;
	m_iduWoodbWaterDmdDy = 0.f;
		
	return true;
	}


bool DailyUrbanWaterDemand::StartStep(FlowContext *pFlowContext)
{
	// handle NONE, EXTERNAL cases if defined
	//if (GlobalMethod::Run(pFlowContext) == true)
	//	return true;
	
	switch (m_method)
	{
	case GM_URBAN_DEMAND_DY:

		m_iduMetroWaterDmdDy = 0.f;
		m_iduEugSpWaterDmdDy = 0.f;
		m_iduSalKiWaterDmdDy = 0.f;
		m_iduCorvlWaterDmdDy = 0.f;
		m_iduAlbnyWaterDmdDy = 0.f;
		m_iduMcminWaterDmdDy = 0.f;
		m_iduNewbrWaterDmdDy = 0.f;
		m_iduWoodbWaterDmdDy = 0.f;
	
		return CalcDailyUrbanWaterDemand(pFlowContext);

	case GM_NONE:
		return true;

	default:
		ASSERT(0);
	}

	return true;
}


bool DailyUrbanWaterDemand::EndStep(FlowContext *pFlowContext)
	{

	CArray< float, float > rowMuni;
	
   float time = (float)pFlowContext->dayOfYear;

	rowMuni.SetSize(9);
	
	rowMuni[0] = time;
	rowMuni[1] = m_iduMetroWaterDmdDy;
	rowMuni[2] = m_iduEugSpWaterDmdDy;
	rowMuni[3] = m_iduSalKiWaterDmdDy;
	rowMuni[4] = m_iduCorvlWaterDmdDy;
	rowMuni[5] = m_iduAlbnyWaterDmdDy;
	rowMuni[6] = m_iduMcminWaterDmdDy;
	rowMuni[7] = m_iduNewbrWaterDmdDy;
	rowMuni[8] = m_iduWoodbWaterDmdDy;

	this->m_timeSeriesMunDemandSummaries.AppendRow(rowMuni);
	
	return true;	
	}

bool DailyUrbanWaterDemand::CalcDailyUrbanWaterDemand(FlowContext *pFlowContext)
   {
	MapLayer *pIDULayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

	float ETtG = 1.f; // average summer ET (april-October) for urban lawn grass in current time step
	float ET0G = 1.f; // average summer ET (april-October) for urban lawn grass in time step 0
	int   doy = pFlowContext->dayOfYear; // Jan 1 = 0

	for (MapLayer::Iterator idu = pIDULayer->Begin(); idu != pIDULayer->End(); idu++)
		{
      float adjustedTotDmd_ccf = 0.f; // ccf/day
      float iduPop = 0.f; pIDULayer->GetData(idu, m_colPop, iduPop);
      if (iduPop > 0.f) 
         { 
         float aveResDmd_ccf = 0.f; // ccf/day
         pIDULayer->GetData(idu, m_colH2OResidnt, aveResDmd_ccf); // ccf/day         

         // Add in the commercial and industrial demand, which may be zero.
         float aveIndCommDmd_ccf = 0.f; // ccf/day
         pIDULayer->GetData(idu, m_colH2OIndComm, aveIndCommDmd_ccf);

         adjustedTotDmd_ccf = (aveResDmd_ccf + aveIndCommDmd_ccf) * UWseasonalVolumeAdjustment(doy); // ccf/day

         int UGBid = 0; pIDULayer->GetData(idu, m_colUGB, UGBid);
         if (UGBid == 40) m_iduMetroWaterDmdDy += adjustedTotDmd_ccf;
         if (UGBid == 22) m_iduEugSpWaterDmdDy += adjustedTotDmd_ccf;
         if (UGBid == 51) m_iduSalKiWaterDmdDy += adjustedTotDmd_ccf;
         if (UGBid == 12) m_iduCorvlWaterDmdDy += adjustedTotDmd_ccf;
         if (UGBid == 2) m_iduAlbnyWaterDmdDy += adjustedTotDmd_ccf;
         if (UGBid == 39) m_iduMcminWaterDmdDy += adjustedTotDmd_ccf;
         if (UGBid == 47) m_iduNewbrWaterDmdDy += adjustedTotDmd_ccf;
         if (UGBid == 71) m_iduWoodbWaterDmdDy += adjustedTotDmd_ccf;

         } // end of if (iduPop > 0.f)
      else adjustedTotDmd_ccf = 0.f; // Where there are no people, the demand is zero.

      // ccf = 100 cf = 100 ft3
      // m3 = 35.3147 ft3
      // day = 86400 seconds
      // m3/sec = cf/day * 100 / 35.3147 / 86400
      float adjustedTotDmd_m3_per_sec = adjustedTotDmd_ccf * 100 / FT3_PER_M3 / SEC_PER_DAY;

      bool readOnlyFlag = pIDULayer->m_readOnly;
      pIDULayer->m_readOnly = false;
      pIDULayer->SetData(idu, m_colDailyUrbanDemand, adjustedTotDmd_m3_per_sec);
      pIDULayer->m_readOnly = readOnlyFlag;
      }

	return TRUE;
} // end of CalcDailyUrbanWaterDemand()

DailyUrbanWaterDemand *DailyUrbanWaterDemand::LoadXml(TiXmlElement *pXmlDailyUrbWaterDmd, MapLayer *pIDULayer, LPCTSTR filename)
{
	if (pXmlDailyUrbWaterDmd == NULL)
		return NULL;

	// get attributes
	LPTSTR name = NULL;
	LPTSTR method = NULL;
	bool use = true;

	XML_ATTR attrs[] = {
		// attr                 type          address               isReq  checkCol
		{ "name", TYPE_STRING, &name, false, 0 },
		{ "method", TYPE_STRING, &method, true, 0 },
		{ "use", TYPE_BOOL, &use, false, 0 },
		{ NULL, TYPE_NULL, NULL, false, 0 } };

	bool ok = TiXmlGetAttributes(pXmlDailyUrbWaterDmd, attrs, filename, pIDULayer);
	if (!ok)
	{
		CString msg;
		msg.Format(_T("Flow: Misformed element reading <DailyUrbanWaterDemand> attributes in input file %s"), filename );
		Report::ErrorMsg(msg);
		return NULL;
	}

	DailyUrbanWaterDemand *pDayUrbDemand = new DailyUrbanWaterDemand(name);

	if (method != NULL)
	{
		switch (method[0])
		{
		case 'd':
		case 'D':
			pDayUrbDemand->SetMethod(GM_URBAN_DEMAND_DY);
			break;
		default:
			pDayUrbDemand->SetMethod(GM_NONE);
		}
	}

	return pDayUrbDemand;
}

// Julian day (Jan 1 = 1), Total daily water as fraction of mean daily water, Daily indoor water as fraction of mean daily water
// from "seasonal_water_distribution_urban_demand_v2_DRC.xlsx"
double seasonal_adjustment[366][3] = {
      { 1, 0.837699, 0.833957 },
      { 2, 0.837913, 0.833957 },
      { 3, 0.838149, 0.833957 },
      { 4, 0.838577, 0.833957 },
      { 5, 0.839047, 0.833957 },
      { 6, 0.839540, 0.833957 },
      { 7, 0.840034, 0.833957 },
      { 8, 0.840478, 0.833957 },
      { 9, 0.840844, 0.833957 },
      { 10, 0.841110, 0.833957 },
      { 11, 0.841257, 0.833957 },
      { 12, 0.841280, 0.833957 },
      { 13, 0.841181, 0.833957 },
      { 14, 0.840968, 0.833957 },
      { 15, 0.840656, 0.833957 },
      { 16, 0.840263, 0.833957 },
      { 17, 0.839810, 0.833957 },
      { 18, 0.839317, 0.833957 },
      { 19, 0.838805, 0.833957 },
      { 20, 0.838291, 0.833957 },
      { 21, 0.837794, 0.833957 },
      { 22, 0.837328, 0.833957 },
      { 23, 0.836904, 0.833957 },
      { 24, 0.836532, 0.833957 },
      { 25, 0.836217, 0.833957 },
      { 26, 0.835963, 0.833957 },
      { 27, 0.835770, 0.833957 },
      { 28, 0.835634, 0.833957 },
      { 29, 0.835549, 0.833957 },
      { 30, 0.835506, 0.833957 },
      { 31, 0.835495, 0.833957 },
      { 32, 0.835506, 0.833957 },
      { 33, 0.835527, 0.833957 },
      { 34, 0.835550, 0.833957 },
      { 35, 0.835568, 0.833957 },
      { 36, 0.835577, 0.833957 },
      { 37, 0.835577, 0.833957 },
      { 38, 0.835571, 0.833957 },
      { 39, 0.835564, 0.833957 },
      { 40, 0.835565, 0.833957 },
      { 41, 0.835584, 0.833957 },
      { 42, 0.835629, 0.833957 },
      { 43, 0.835712, 0.833957 },
      { 44, 0.835840, 0.833957 },
      { 45, 0.836019, 0.833957 },
      { 46, 0.836251, 0.833957 },
      { 47, 0.836538, 0.833957 },
      { 48, 0.836877, 0.833957 },
      { 49, 0.837262, 0.833957 },
      { 50, 0.837686, 0.833957 },
      { 51, 0.838137, 0.833957 },
      { 52, 0.838605, 0.833957 },
      { 53, 0.839076, 0.833957 },
      { 54, 0.839536, 0.833957 },
      { 55, 0.839973, 0.833957 },
      { 56, 0.840375, 0.833957 },
      { 57, 0.840730, 0.833957 },
      { 58, 0.841030, 0.833957 },
      { 59, 0.841270, 0.833957 },
      { 60, 0.841559, 0.833957 },
      { 61, 0.841610, 0.833957 },
      { 62, 0.841601, 0.833957 },
      { 63, 0.841538, 0.833957 },
      { 64, 0.841424, 0.833957 },
      { 65, 0.841263, 0.833957 },
      { 66, 0.841057, 0.833957 },
      { 67, 0.840808, 0.833957 },
      { 68, 0.840517, 0.833957 },
      { 69, 0.840184, 0.833957 },
      { 70, 0.839811, 0.833957 },
      { 71, 0.839399, 0.833957 },
      { 72, 0.838957, 0.833957 },
      { 73, 0.838493, 0.833957 },
      { 74, 0.838024, 0.833957 },
      { 75, 0.837568, 0.833957 },
      { 76, 0.837151, 0.833957 },
      { 77, 0.836801, 0.833957 },
      { 78, 0.836548, 0.833957 },
      { 79, 0.836423, 0.833957 },
      { 80, 0.836455, 0.833957 },
      { 81, 0.836669, 0.833957 },
      { 82, 0.837086, 0.833957 },
      { 83, 0.837716, 0.833957 },
      { 84, 0.838566, 0.833957 },
      { 85, 0.839629, 0.833957 },
      { 86, 0.840895, 0.833957 },
      { 87, 0.842342, 0.833957 },
      { 88, 0.843941, 0.833957 },
      { 89, 0.845661, 0.833957 },
      { 90, 0.847462, 0.833957 },
      { 91, 0.849307, 0.833957 },
      { 92, 0.851156, 0.833957 },
      { 93, 0.852971, 0.833957 },
      { 94, 0.854717, 0.833957 },
      { 95, 0.856364, 0.833957 },
      { 96, 0.857891, 0.833957 },
      { 97, 0.859280, 0.833957 },
      { 98, 0.860526, 0.833957 },
      { 99, 0.861630, 0.833957 },
      { 100, 0.862603, 0.833957 },
      { 101, 0.863464, 0.833957 },
      { 102, 0.864242, 0.833957 },
      { 103, 0.864969, 0.833957 },
      { 104, 0.865685, 0.833957 },
      { 105, 0.866433, 0.833957 },
      { 106, 0.867254, 0.833957 },
      { 107, 0.868193, 0.833957 },
      { 108, 0.869288, 0.833957 },
      { 109, 0.870578, 0.833957 },
      { 110, 0.872094, 0.833957 },
      { 111, 0.873861, 0.833957 },
      { 112, 0.875902, 0.833957 },
      { 113, 0.878228, 0.833957 },
      { 114, 0.880850, 0.833957 },
      { 115, 0.883771, 0.833957 },
      { 116, 0.886989, 0.833957 },
      { 117, 0.890500, 0.833957 },
      { 118, 0.894297, 0.833957 },
      { 119, 0.898368, 0.833957 },
      { 120, 0.902702, 0.833957 },
      { 121, 0.907284, 0.833957 },
      { 122, 0.912097, 0.833957 },
      { 123, 0.917121, 0.833957 },
      { 124, 0.922334, 0.833957 },
      { 125, 0.927707, 0.833957 },
      { 126, 0.933209, 0.833957 },
      { 127, 0.938804, 0.833957 },
      { 128, 0.944449, 0.833957 },
      { 129, 0.950103, 0.833957 },
      { 130, 0.955719, 0.833957 },
      { 131, 0.961255, 0.833957 },
      { 132, 0.966672, 0.833957 },
      { 133, 0.971938, 0.833957 },
      { 134, 0.977032, 0.833957 },
      { 135, 0.981945, 0.833957 },
      { 136, 0.986683, 0.833957 },
      { 137, 0.991265, 0.833957 },
      { 138, 0.995723, 0.833957 },
      { 139, 1.000099, 0.833957 },
      { 140, 1.004443, 0.833957 },
      { 141, 1.008803, 0.833957 },
      { 142, 1.013229, 0.833957 },
      { 143, 1.017759, 0.833957 },
      { 144, 1.022422, 0.833957 },
      { 145, 1.027230, 0.833957 },
      { 146, 1.032181, 0.833957 },
      { 147, 1.037257, 0.833957 },
      { 148, 1.042426, 0.833957 },
      { 149, 1.047650, 0.833957 },
      { 150, 1.052884, 0.833957 },
      { 151, 1.058086, 0.833957 },
      { 152, 1.063225, 0.833957 },
      { 153, 1.068279, 0.833957 },
      { 154, 1.073246, 0.833957 },
      { 155, 1.078141, 0.833957 },
      { 156, 1.083000, 0.833957 },
      { 157, 1.087875, 0.833957 },
      { 158, 1.092829, 0.833957 },
      { 159, 1.097935, 0.833957 },
      { 160, 1.103264, 0.833957 },
      { 161, 1.108884, 0.833957 },
      { 162, 1.114848, 0.833957 },
      { 163, 1.121193, 0.833957 },
      { 164, 1.127935, 0.833957 },
      { 165, 1.135069, 0.833957 },
      { 166, 1.142567, 0.833957 },
      { 167, 1.150382, 0.833957 },
      { 168, 1.158454, 0.833957 },
      { 169, 1.166711, 0.833957 },
      { 170, 1.175081, 0.833957 },
      { 171, 1.183493, 0.833957 },
      { 172, 1.191889, 0.833957 },
      { 173, 1.200223, 0.833957 },
      { 174, 1.208473, 0.833957 },
      { 175, 1.216637, 0.833957 },
      { 176, 1.224735, 0.833957 },
      { 177, 1.232811, 0.833957 },
      { 178, 1.240921, 0.833957 },
      { 179, 1.249134, 0.833957 },
      { 180, 1.257523, 0.833957 },
      { 181, 1.266153, 0.833957 },
      { 182, 1.275076, 0.833957 },
      { 183, 1.284325, 0.833957 },
      { 184, 1.293908, 0.833957 },
      { 185, 1.303802, 0.833957 },
      { 186, 1.313957, 0.833957 },
      { 187, 1.324301, 0.833957 },
      { 188, 1.334738, 0.833957 },
      { 189, 1.345163, 0.833957 },
      { 190, 1.355464, 0.833957 },
      { 191, 1.365536, 0.833957 },
      { 192, 1.375283, 0.833957 },
      { 193, 1.384624, 0.833957 },
      { 194, 1.393501, 0.833957 },
      { 195, 1.401870, 0.833957 },
      { 196, 1.409709, 0.833957 },
      { 197, 1.417007, 0.833957 },
      { 198, 1.423762, 0.833957 },
      { 199, 1.429977, 0.833957 },
      { 200, 1.435654, 0.833957 },
      { 201, 1.440792, 0.833957 },
      { 202, 1.445383, 0.833957 },
      { 203, 1.449414, 0.833957 },
      { 204, 1.452869, 0.833957 },
      { 205, 1.455730, 0.833957 },
      { 206, 1.457982, 0.833957 },
      { 207, 1.459617, 0.833957 },
      { 208, 1.460638, 0.833957 },
      { 209, 1.461056, 0.833957 },
      { 210, 1.460899, 0.833957 },
      { 211, 1.460203, 0.833957 },
      { 212, 1.459012, 0.833957 },
      { 213, 1.457376, 0.833957 },
      { 214, 1.455341, 0.833957 },
      { 215, 1.452945, 0.833957 },
      { 216, 1.450217, 0.833957 },
      { 217, 1.447168, 0.833957 },
      { 218, 1.443791, 0.833957 },
      { 219, 1.440063, 0.833957 },
      { 220, 1.435946, 0.833957 },
      { 221, 1.431391, 0.833957 },
      { 222, 1.426350, 0.833957 },
      { 223, 1.420773, 0.833957 },
      { 224, 1.414625, 0.833957 },
      { 225, 1.407887, 0.833957 },
      { 226, 1.400561, 0.833957 },
      { 227, 1.392673, 0.833957 },
      { 228, 1.384272, 0.833957 },
      { 229, 1.375427, 0.833957 },
      { 230, 1.366224, 0.833957 },
      { 231, 1.356759, 0.833957 },
      { 232, 1.347133, 0.833957 },
      { 233, 1.337442, 0.833957 },
      { 234, 1.327775, 0.833957 },
      { 235, 1.318208, 0.833957 },
      { 236, 1.308802, 0.833957 },
      { 237, 1.299601, 0.833957 },
      { 238, 1.290633, 0.833957 },
      { 239, 1.281908, 0.833957 },
      { 240, 1.273426, 0.833957 },
      { 241, 1.265174, 0.833957 },
      { 242, 1.257130, 0.833957 },
      { 243, 1.249266, 0.833957 },
      { 244, 1.241552, 0.833957 },
      { 245, 1.233954, 0.833957 },
      { 246, 1.226439, 0.833957 },
      { 247, 1.218980, 0.833957 },
      { 248, 1.211550, 0.833957 },
      { 249, 1.204131, 0.833957 },
      { 250, 1.196713, 0.833957 },
      { 251, 1.189291, 0.833957 },
      { 252, 1.181870, 0.833957 },
      { 253, 1.174461, 0.833957 },
      { 254, 1.167078, 0.833957 },
      { 255, 1.159736, 0.833957 },
      { 256, 1.152447, 0.833957 },
      { 257, 1.145218, 0.833957 },
      { 258, 1.138044, 0.833957 },
      { 259, 1.130910, 0.833957 },
      { 260, 1.123785, 0.833957 },
      { 261, 1.116629, 0.833957 },
      { 262, 1.109387, 0.833957 },
      { 263, 1.102001, 0.833957 },
      { 264, 1.094407, 0.833957 },
      { 265, 1.086548, 0.833957 },
      { 266, 1.078374, 0.833957 },
      { 267, 1.069848, 0.833957 },
      { 268, 1.060954, 0.833957 },
      { 269, 1.051694, 0.833957 },
      { 270, 1.042094, 0.833957 },
      { 271, 1.032199, 0.833957 },
      { 272, 1.022073, 0.833957 },
      { 273, 1.011795, 0.833957 },
      { 274, 1.001454, 0.833957 },
      { 275, 0.991143, 0.833957 },
      { 276, 0.980957, 0.833957 },
      { 277, 0.970984, 0.833957 },
      { 278, 0.961306, 0.833957 },
      { 279, 0.951992, 0.833957 },
      { 280, 0.943099, 0.833957 },
      { 281, 0.934669, 0.833957 },
      { 282, 0.926732, 0.833957 },
      { 283, 0.919302, 0.833957 },
      { 284, 0.912383, 0.833957 },
      { 285, 0.905968, 0.833957 },
      { 286, 0.900043, 0.833957 },
      { 287, 0.894585, 0.833957 },
      { 288, 0.889565, 0.833957 },
      { 289, 0.884952, 0.833957 },
      { 290, 0.880710, 0.833957 },
      { 291, 0.876802, 0.833957 },
      { 292, 0.873189, 0.833957 },
      { 293, 0.869833, 0.833957 },
      { 294, 0.866696, 0.833957 },
      { 295, 0.863742, 0.833957 },
      { 296, 0.860938, 0.833957 },
      { 297, 0.858257, 0.833957 },
      { 298, 0.855674, 0.833957 },
      { 299, 0.853171, 0.833957 },
      { 300, 0.850738, 0.833957 },
      { 301, 0.848369, 0.833957 },
      { 302, 0.846064, 0.833957 },
      { 303, 0.843828, 0.833957 },
      { 304, 0.841672, 0.833957 },
      { 305, 0.839607, 0.833957 },
      { 306, 0.837647, 0.833957 },
      { 307, 0.835806, 0.833957 },
      { 308, 0.834096, 0.833957 },
      { 309, 0.832526, 0.833957 },
      { 310, 0.831100, 0.833957 },
      { 311, 0.829820, 0.833957 },
      { 312, 0.828680, 0.833957 },
      { 313, 0.827672, 0.833957 },
      { 314, 0.826780, 0.833957 },
      { 315, 0.825987, 0.833957 },
      { 316, 0.825274, 0.833957 },
      { 317, 0.824620, 0.833957 },
      { 318, 0.824009, 0.833957 },
      { 319, 0.823426, 0.833957 },
      { 320, 0.822862, 0.833957 },
      { 321, 0.822317, 0.833957 },
      { 322, 0.821797, 0.833957 },
      { 323, 0.821316, 0.833957 },
      { 324, 0.820894, 0.833957 },
      { 325, 0.820558, 0.833957 },
      { 326, 0.820335, 0.833957 },
      { 327, 0.820255, 0.833957 },
      { 328, 0.820343, 0.833957 },
      { 329, 0.820619, 0.833957 },
      { 330, 0.821095, 0.833957 },
      { 331, 0.821774, 0.833957 },
      { 332, 0.822648, 0.833957 },
      { 333, 0.823697, 0.833957 },
      { 334, 0.824895, 0.833957 },
      { 335, 0.826203, 0.833957 },
      { 336, 0.827582, 0.833957 },
      { 337, 0.828985, 0.833957 },
      { 338, 0.830368, 0.833957 },
      { 339, 0.831687, 0.833957 },
      { 340, 0.832905, 0.833957 },
      { 341, 0.833991, 0.833957 },
      { 342, 0.834919, 0.833957 },
      { 343, 0.835674, 0.833957 },
      { 344, 0.836249, 0.833957 },
      { 345, 0.836641, 0.833957 },
      { 346, 0.836859, 0.833957 },
      { 347, 0.836911, 0.833957 },
      { 348, 0.836813, 0.833957 },
      { 349, 0.836582, 0.833957 },
      { 350, 0.836237, 0.833957 },
      { 351, 0.835799, 0.833957 },
      { 352, 0.835288, 0.833957 },
      { 353, 0.834725, 0.833957 },
      { 354, 0.834132, 0.833957 },
      { 355, 0.833531, 0.833957 },
      { 356, 0.832942, 0.833957 },
      { 357, 0.832383, 0.833957 },
      { 358, 0.831884, 0.833957 },
      { 359, 0.831451, 0.833957 },
      { 360, 0.831086, 0.833957 },
      { 361, 0.830889, 0.833957 },
      { 362, 0.830719, 0.833957 },
      { 363, 0.830582, 0.833957 },
      { 364, 0.830582, 0.833957 },
      { 365, 0.830582, 0.833957 },
      { 366, 0.830582, 0.833957 } };

float DailyUrbanWaterDemand::UWseasonalVolumeAdjustment(int doy) { return((float)seasonal_adjustment[doy][1]); }

float DailyUrbanWaterDemand::UWindoorFraction(int doy) { return((float)(min(1., seasonal_adjustment[doy][2] / seasonal_adjustment[doy][1]))); }
