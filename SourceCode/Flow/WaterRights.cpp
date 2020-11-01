// WaterRights.cpp
//
#include "StdAfx.h"
#pragma hdrstop

#include "WaterRights.h"
#include "Flow.h"
#include "../APs/APs.h"
#include <Maplayer.h>
#include <string.h>
#include <EnvExtension.h>
#include <EnvInterface.h>
#include <VDataTable.h>
#include <UNITCONV.H>
#include "GlobalMethods.h"
#include "AlgLib\AlgLib.h"
#include "AlgLib\ap.h"
#include "GDALWrapper.h"
#include <iostream>
#include <fstream>
#include <afxtempl.h>
#include "FlowContext.h"
#include <EnvEngine\EnvModel.h>
#include "GeoSpatialDataObj.h"

using namespace alglib_impl;


extern FlowProcess *gpFlow;
extern FlowModel   *gpModel;

int SortWrData(const void *e0, const void *e1);


AltWaterMaster::AltWaterMaster(WaterAllocation *pWaterAllocation)
: m_pWaterAllocation(pWaterAllocation)
, m_typesInUse(0)
, m_colDemand(-1)							// desired demand (m3/sec)
, m_colWaterUse(-1)						// actual use
, m_colXcoord(-1)							// WR UTM zone 10 x coordinate
, m_colYcoord(-1)							// WR UTM zone 10 y coordinate
, m_colPodID(-1)							// Water Right POD ID
, m_colPDPouID(-1)						// Water Right POU ID in POD input data file
, m_colPermitCode(-1)					// WR Permit Code http://www.oregon.gov/owrd/pages/wr/wrisuse.aspx
, m_colPodRate(-1)						// WR point of diversion max rate (m3/sec)
, m_colUseCode(-1)						// WR Use Code http://www.oregon.gov/owrd/pages/wr/wrisuse.aspx
, m_colPriorDoy(-1)						// WR priority date day of year
, m_colPriorYr(-1)						// WR priority date year
, m_colBeginDoy(-1)						// WR seasonal begin day of year
, m_colEndDoy(-1)							// WR season end day year
, m_colPouRate(-1)						// WR point of use max rate (m3/sec)
, m_colPouPOU_ID(-1)						// WR ID POU ID in POU data input file
, m_colPouIDU_ID(-1)					// IDU_INDEX column in POU data input file
, m_colPouPERCENT_POU(-1)							// The percentage of the POU, for a SnapID, that over laps the IDU_INDEX
, m_colPouPERCENT_IDU(-1)
, m_colPouDBNdx(-1)						// a zero based index for the POU input data file itself
, m_colPouUSECODE(-1)
, m_colPouPERMITCODE(-1)
, m_colReachComid(-1)					// WR Reach comid, relates to COMID in reach layer
, m_colIrrigate(-1)						// Economic decision to irrigat,e 0=false no irrigate, 1=true yes irrigate
, m_colAREA(-1)							// IDU column for area
, m_colUGB(-1)
, m_colPOP(-1)
, m_colGAL_CAP_DY(-1)
, m_colET_DAY(-1)
, m_colH2ORESIDNT(-1)
, m_colH2OINDCOMM(-1)
, m_colWaterDeficit(-1)					// water deficit, from ET calculations, mm/d
, m_colRealUnSatisfiedDemand(-1)		// Real UnSatisfied Demand (m3/sec)
, m_colVirtUnSatisfiedDemand(-1)		// virtual UnSatisfied Demand (m3/sec)
, m_colUnExercisedWR(-1)				// Un-exercised WR
, m_weekOfYear(0)							// the week of the year (zero based 0-51)
, m_envTimeStep(0)						// 0 based year of run
, m_nSWLevelOneMunWRSO(0)				// number of municipal IDUs with level one conflict, water right shut off
, m_nSWLevelTwoMunWRSO(0)				// number of municipal IDUs with level two conflict, water right shut off
, m_minFlow(0.f)							// minimum flow threshold for stream reach
, m_colStreamCOMID(-1)					// column number of COMID in Stream Layer
, m_colStreamSTRM_ORDER(-1)
, m_colWRShortG(-1)						// WR shut off in IDU layer.  1=level 1 shutoff, 2=level 2 shutoff, 0=has not been shut off
, m_SWIrrWaterMmDy(0.0f)					// daily allocated surface water irrigation mm/day
, m_GWIrrWaterMmDy(0.0f)					// daily allocated ground water irrigation mm/day
, m_SWIrrDuty(0.0f)						// daily surface water irrigation duty m3
, m_GWIrrDuty(0.0f)						// daily ground water irrigation duty m3
, m_remainingIrrRequest(0.0f)        // as multiple PODs satisfy Irrigation Request in IDU this is what is remaining
, m_SWMuniWater(0.0f)						// daily allocated municipal water
, m_fromBullRunDy_m3_per_s(0.f)
, m_fromBullRunYr_m3(0.f)
, m_fromSpringHillDy_m3_per_s(0.f)
, m_fromSpringHillYr_m3(0.f)
, m_fromBarneyDy_m3_per_s(0.f)
, m_fromBarneyYr_m3(0.f)
, m_fromClackamasDy_m3_per_s(0.f)
, m_fromClackamasYr_m3(0.f)
, m_fromLakeOswegoDy_m3_per_s(0.f)
, m_fromLakeOswegoYr_m3(0.f)
, m_toOutsideBasinYr_m3(0.f)
, m_SWunallocatedIrrWater(0.0f)      // daily unallocated ag water
, m_SWunsatIrrWaterDemand(0.0f)      // daily unsatisfied ag water demand
, m_GWUnSatIrr(0.0f)						// daily unsatisfied irrigation from ground water m3/sec
, m_SWUnSatIrr(0.0f)						// daily unsatisfied irrigation from stream m3/sec
, m_SWUnExIrr(0.0f)						// daily unexercised irrigation from surface water m3/sec
, m_GWUnExIrr(0.0f)						// daily unexercised irrigation from ground water m3/sec
, m_SWUnAllocIrr(0.0f)					// daily unallocated irrigatin from surface water m3/sec 
, m_GWUnAllocIrr(0.0f)					// daily unallocated irrigatin from ground water m3/sec
, m_colAllocatedIrrigation(-1)      // irrigation allocation
, m_colDemandFraction(-1)
, m_wrIrrigateFlag(true)				// for each flow time step if right exist and demand is zero, flag is false. default true.
, m_demandIrrigateFlag(true)			// If economic to irrigate at envision time step is 1, then true. elseif  0 then false.
, m_colUDMAND_DY(-1)			// Daily Urban water demand from the IDU layer m3/sec
, m_colPLANTDATE(-1)						// Crop planting date in IDU layer
, m_colHarvDate(-1)					   // Crop harvest date in IDU layer
, m_irrDefaultBeginDoy(-1)			   // The default irrigation begin Day of Year if not specified in Water Right. Set in .xml file (1 base)
, m_irrDefaultEndDoy(-1)				// The default irrigation end Day of Year if not specified in Water Right.  Set in .xml file (1 base)
, m_SWCorrFactor(0.0f)				   // correction factor for surface water POD rate diversion (unitless)
, m_GWCorrFactor(0.0f)				   //correction factor for ground water POD rate diversion (unitless)
, m_colSWIrrDy(-1)						// Daily surface water applied to IDU (mm/day)
, m_colGWIrrDy(-1)						// Daily ground water applied to IDU (mm/day)
, m_areaDutyExceeds(0.0f)				// sum area that have been shut off after WR exceeds it's allowed maximum
, m_colIrrApplied(-1)
, m_colWRID(-1)
, m_colLulc_A(-1)							// Land Use/Land Cover (coarse). used for finding Agriculture IDUs
, m_maxRate_cfs_per_ac(IRRIG_MAX_RATE_DEFAULT) // For irrigable IDUs, default maximum rate, cfs per acre
, m_maxDuty_ft_per_yr(IRRIG_MAX_DUTY_DEFAULT) // For irrigable IDUs, default maximum duty, ft H2O per year
, m_maxIrrDiversion_mm_per_day(7.5f)				// The maximum irrigation diversion allowed, mm/day
, m_SWIrrUnSatDemand(0.0f)          // if available surface water is less than half irrigation request m3/sec
, m_GWIrrUnSatDemand(0.0f)          // if available ground water is less than half irrigation request m3/sec
, m_colWRConflictYr(-1)             // Attribute in stream layer that types a reach as being in "conflict" (not satisfying WR) 0- no conflict, 1-available < demand, 2-available < demand/2
, m_colWRConflictDy(-1)             // Attribute in stream layer that types a reach as being in "conflict" (not satisfying WR) 0- no conflict, 1-level one, 2-level two
, m_irrWaterRequestYr(0.0f) 			// Total Irrgation Request (acre-ft per year)
, m_irrigableIrrWaterRequestYr(0.0f)// Irrigable Irrigation Request (acre-ft per year)
, m_irrigatedWaterYr(0.0f)				// Allocated irrigation Water (acre-ft per year)
, m_irrigatedSurfaceYr(0.0f)			// Allocated surface water for irrigation (acre-ft per year)
, m_irrigatedGroundYr(0.0f)			// Allocated ground water for irrigation (acre-ft per year)
, m_unSatisfiedIrrigationYr(0.0f)	// Unsatisified Irrigation Water (acre-ft per year)
, m_colIrrRequestYr(-1)					// Annual Irrigation Request column in IDU layer (acre-ft per year)
, m_colIrrRequestDy(-1)					// Daily Irrigation Request column in IDU layer (acre-ft per year)
, m_colMaxTotDailyIrr(-1) 			   // column in IDU layer for Maximum total daily applied irrigation water (acre-ft)
, m_colaveTotDailyIrr(-1) 		      // column in IDU layer for average total daily applied irrigation water (acre-ft)
, m_GWMuniWater(0.0f)				   // daily allocated municipal water from well m3/sec
, m_colReachLength(-1)              // column number of the attrigute "LENGTH" in the stream layer
, m_basinReachLength(0.0f)				// total lenght of stream reaches in the study area
, m_colNInConflict(-1)					// column number in stream layer for number of days reach in conflict
, m_exportDistPodComid(0)				// If set equal to 1, export .csv file with distances between stream centroid and POD xy coordinates
, m_maxBullRunAllocationToday(-1)  
, m_maxSpringHillAllocationToday(-1)
, m_maxClackamasAllocationToday(-1)
, m_maxBarneyAllocationToday(-1)
, m_maxLakeOswegoAllocationToday(-1)
, m_irrigateDecision(1)					// decision to irrigate 1-yes, 0-no.  default yes
, m_maxDutyHalt(0)						// If 1-yes then halt irrigation to IDU if maximum duty is reached
, m_colWRShutOffMun(-1)             // WR municipal shut off in IDU layer.  1=level 1 shutoff, 2=level 2 shutoff, 0=has not been shut off
, m_debug(0)								// if 1-yes then will invoke helpful debugging stuff
, m_dyGTmaxPodArea21(0.0f)				// vineyards and tree farms area > maxPOD(acres)"); //21
, m_dyGTmaxPodArea22(0.0f)				// Grass seed area > maxPOD(acres)"); //22
, m_dyGTmaxPodArea23(0.0f)				// Pasture area > maxPOD(acres)"); //23
, m_dyGTmaxPodArea24(0.0f)				// Wheat area > maxPOD(acres)"); //24
, m_dyGTmaxPodArea25(0.0f)				// Fallow area > maxPOD(acres)"); //25
, m_dyGTmaxPodArea26(0.0f)				// Corn area > maxPOD(acres)"); //26
, m_dyGTmaxPodArea27(0.0f)				// Clover area > maxPOD(acres)"); //27
, m_dyGTmaxPodArea28(0.0f)				// Hay area > maxPOD(acres)"); //28
, m_dyGTmaxPodArea29(0.0f)				// Other crops area > maxPOD(acres)"); //29	
, m_anGTmaxDutyArea21(0.0f)			// vineyards and tree farms area > maxDuty(acres)"); //21
, m_anGTmaxDutyArea22(0.0f)			// Grass seed area > maxDuty(acres)"); //22
, m_anGTmaxDutyArea23(0.0f)			// Pasture area > maxDuty(acres)"); //23
, m_anGTmaxDutyArea24(0.0f)			// Wheat area > maxDuty(acres)"); //24
, m_anGTmaxDutyArea25(0.0f)			// Fallow area > maxDuty(acres)"); //25
, m_anGTmaxDutyArea26(0.0f)			// Corn area > maxDuty(acres)"); //26
, m_anGTmaxDutyArea27(0.0f)			// Clover area > maxDuty(acres)"); //27
, m_anGTmaxDutyArea28(0.0f)			// Hay area > maxDuty(acres)"); //28
, m_anGTmaxDutyArea29(0.0f)			// Other crops area > maxDuty(acres)"); //29	
, m_colLulc_B(-1)						   // Land Use/Land Cover (medium). used for finding crop type
, m_colDailyAllocatedIrrigation(-1)	// Water Allocated for irrigation use (mm per day)
, m_colUnSatIrrRequest(-1)          // Unsatisfied irrigation request (mm per day)
, m_colActualIrrRequestDy(-1)       // Daily Actual irrigation request ( mm per day )
, m_IrrFromAllocationArrayDy(0.0f)	// check to see if array used to flux is same as values written to map ( mm/day ) action item remove after development
, m_IrrFromAllocationArrayDyBeginStep(0.0f) // check to see if array used to flux is same as values written to map ( mm/day ) action item to eventually remove
, m_pastureIDUGTmaxDuty(0)          // a pasture IDU_INDEX where max duty is exceeded
, m_pastureIDUGTmaxDutyArea(0.0f)   // a pasture area of IDU_INDEX where max duty is exceeded (m2)
, m_colSWMUNALL_D(-1)       // column in IDU layer for daily municipal surface water allocations (m3/sec)
, m_colSWMUNALL_Y(-1)               // column in IDU layer for annual municipal surface water allocations (m3 H2O)
, m_colGWMUNALL_D(-1)       // column in IDU layer for daily municipal ground water allocations (m3/sec)
, m_colGWMUNALL_Y(-1)               // column in IDU layer for annual municipal ground water allocations (m3 H2O)
, m_colUWOUTDOORD(-1) // today's urban water used outdoors, mm
, m_colSWUnexIrr(-1)						// when appropriated surface water POD rate is greater than request, POD rate - demand (m3/sec)
, m_colGWUnexIrr(-1)						// when appropriated ground water POD rate is greater than request, POD rate - demand (m3/sec)
, m_colSWUnexMun(-1) 					// when appropriated municipal surface water POD rate is greater than request, POD rate - demand (m3/sec)
, m_colSWUnSatMunDemandDy(-1)			// daily unsatisfied municipal demand (m3/sec)
, m_colSWUnSatIrrRequestDy(-1)      // daily surface water unsatisfied irrigation request (m3/sec)
, m_colWastedIrrDy(-1)					// 0.0125 cfs - daily allocated irrigation water, summed over irrigated IDUs (m3/sec)
, m_colExcessIrrDy(-1)					// (m3/sec)
, m_colAllocatedIrrigationAf(-1)			// Water Allocated for irrigation use (acre-ft per year)
, m_colSWAllocatedIrrigationAf(-1)  // Surface Water Allocated for irrigation use (acre-ft per year)
, m_colIRGWAF_Y(-1)  // Ground Water Allocated for irrigation use (acre-ft per year)
, m_colUnsatIrrigationAf(-1)			// Unsatisfied irrigation request (acre-ft per year) 
, m_colGWUnsatIrrigationAf(-1)		// Unsatisfied ground irrigation request (acre-ft per year)
, m_colWastedIrrYr(-1)              // 0.0125 cfs - annual allocated irrigation water, summed over irrigated IDUs (m3/sec)
, m_colExcessIrrYr(-1)					// (m3/sec)
, m_colIrrExceedMaxDutyYr(-1)       // 1- yes, IDU exceeded maximum annual irrigation duty, 0-no
, m_colGWUnSatMunDemandDy(-1)			// daily ground water unsatisfied municipal demand (m3/sec)
, m_colDynamTimeStep(-1)				// column for time step specified in dynamic WR input file
, m_colDynamPermitCode(-1)				// column for permit code specified in dynamic WR input file
, m_colDynamUseCode(-1)					// column for use code specified in dynamic WR input file
, m_colDynamIsLease(-1)					// column if water right is a lease specified in dynamic WR input file
, m_DynamicWRs(false)				   // a dynamic water rights input file has loaded
, m_colWRZone(-1) 				      // column number of WRZONE in IDU layer
, m_colDSReservoir(-1)              // 1- yes, 0-no reach is downstream from reservoir
, m_colCOMID(-1)					      // column ID of COMID in IDU layer
, m_colSTRM_INDEX(-1)					
, m_dynamicWRType(0)						// input variable index
, m_useNewInstreamWR(0)
, m_dynamicWRRadius(1000000)			// the maximum radius from a reach to consider adding a dynamic water right (m). default is a larger number. set in .xml
, m_maxDaysShortage(366)				// the maximum number of shortage days before a Water Right is canceled for the growing season.
, m_recursiveDepth(1)					// in times of shortage, the recursive depth of upstream reaches to possibly suspend junior water rights
, m_pctIDUPOUIntersection(10.0)     // minimum % of IDU's area which overlaps with the POU, in order for an irrigation water right to be associated with the IDU
, m_regulationType(0)               // in times of shortage, index of type of regulation 0 - no regulation, >0 see manual regulation types
, m_colWRShutOff(-1)                // WR irrigation unsatisfied irrigaton request in IDU layer.  1=level, 2=level , 0=request met
, m_colWR_SH_DAY(-1)
, m_dynamicWRAppropriationDate(-1)  // Dynamic water right appropriation date.  If set to -1, then equals year of run
, m_colWR_MUNI(-1)
, m_colWR_INSTRM(-1)
, m_colWR_IRRIG_S(-1)
, m_colWR_IRRIG_G(-1)
, m_colWR_PROB(-1)
, m_colWR_PCT_POU(-1)
, m_colWR_PCT_IDU(-1)
, m_colECC0(-1)
, m_colECC(-1)
, m_colSTRM_ORDER(-1)
, m_colSOILH2OEST(-1)
, m_colIRR_STATE(-1)
, m_colPRECIP_YR(-1)
, m_colPRECIP( -1 )
, m_colTEMP(-1)
, m_colSNOWEVAP_Y(-1)
, m_colHRU_ID(-1)
, m_colHRU_NDX(-1)
, m_colWRPRIORITY(-1)
, m_SWirrigWRpriority(-1)
, m_colCENTROIDX(-1)
, m_colCENTROIDY(-1)
, m_colMIN_Q_WRQ(-1)
, m_colWATERRIGHT(-1)
, m_colCOMID_POD(-1)
, m_colStreamSTRMVERT0X(-1)
, m_colStreamSTRMVERT0Y(-1)
, m_colUGAPOCD(-1)
, m_colUGAMUNIPOD(-1)
, m_colStreamINSTRM_WR(-1)
, m_colStreamINSTRMWRID(-1)
, m_colStreamINSTRM_WRQ(-1)
, m_colStreamQ_DIV_WRQ(-1)
, m_colStreamINSTRM_REQ(-1) // regulatory flow requirement for this reach, cms (requirement of most junior instream WR, if more than one applies)
, m_colStreamHBVCALIB(-1)
, m_colReachSUB_AREA(-1)
, m_colStreamXFLUX_Y(-1)
, m_colStreamOUT_IRRIG(-1)
, m_colStreamOUT_MUNI(-1)
, m_colStreamIN_MUNI(-1)
, m_colHBVCALIB(-1)
, m_colSUB_AREA(-1)
, m_colCOUNTYID(-1)
, m_colIDU_ID(-1)
, m_totH2OlastYr_m3(0.)
, m_colXIRR_DUTY(-1)
, m_colXIRR_RATE(-1)
, m_colSEWER_NODE(-1)
, m_colSEWER_NDX(-1)
, m_colNodeDWF_D(-1)

, m_colHruET_DAY(-1)
, m_colHruIRR2SOIL(-1)
, m_colHruIRR2FASTGW(-1)
, m_colHruMUNI2SOIL(-1)
{ 
gpFlow->AddInputVar( "Dynamic Water Right type", m_dynamicWRType, "0=none, 1=unused, 2=unused, 3=NewIrrig Scenario, 4=Extreme Scenario" );
gpFlow->AddInputVar("Regulation type", m_regulationType, "0=default, 1=suspendJuniors");
gpFlow->AddInputVar("Irrig max rate", m_maxRate_cfs_per_ac, "irrigation max rate (cfs per ac)");
gpFlow->AddInputVar("Irrig max duty", m_maxDuty_ft_per_yr, "irrigation duty (ft H2O per year)");
gpFlow->AddInputVar("Use new instream water rights", m_useNewInstreamWR, "0=don't, 1=do");
   }

AltWaterMaster::~AltWaterMaster(void)
{
m_podArray.RemoveAll();
delete[] m_myGeo;
}


// {EASTING, NORTHING, COMID, HBVCALIB attribute value}, 
#define NUM_HBV_CALIB_PTS 46
double HBVcalibPts[NUM_HBV_CALIB_PTS][4] =
{ // These entries should be ordered so that upstream points in a drainage come before downstream points.
// Coast Fork Willamette basin
   { 0, 0, 23759312, 6 }, //", 6, Cottage Grove, ,
   { 0, 0, 23759588, 5 }, //", 5, Dorena, ,
   { 503899, 4868172, 23759226, 39 }, // 39 Coast Fork near Goshen GOSO
   { 0, 0, 23759222, 17 }, // 17 Coast Fork outlet
// Middle Fork Willamette basin
   { 0, 0, 23751946, 1 }, //", 1, Hills Creek, ,
   { 0, 0, 23752598, 4 }, //", 4, Fall Creek, ,
   { 507544, 4871675, 23751778, 38 }, // 38 Middle Fork near Jasper
   { 0, 0, 23751752, 18 }, // 18 Middle Fork outlet
// McKenzie basin
   { 0, 0, 23773407, 9 }, //", 9, Blue River, ,
   { 0, 0, 0, 14 }, // now spare14 { 553414.7513, 4890138.574, 23773405, 14 }, //", 14, Blue River, 44.162500, -122.331940
   { 0, 0, 0, 33 }, // now spare33 { 575933.4287, 4902090.896, -99, 33 }, //", 33, Trail Bridge, 44.268100, -122.048600
   { 0, 0, 23773011, 8 }, //", 8, Cougar, ,
   { 518418.3531, 4879673.347, 23772801, 34 }, //", 34, Walterville, 44.070000, -122.770000
   { 0, 0, 23773507, 25 }, // Mohawk outlet into the McKenzie
   { 0, 0, 23765583, 16 }, // McKenzie outlet
// Upper Willamette basin
   { 0, 0, 23763395, 15 }, // upper Willamette mainstem at the confluence with the McKenzie
// Long Tom basin
   { 0, 0, 23763141, 7 }, //", 7, Fern Ridge, ,
   { 476354.78, 4906669.65, 23763077, 35 }, // 35 Long Tom at Monroe
   { 0, 0, 23763069, 24 }, // Long Tom outlet into the Willamette
// Marys basin
   { 0, 0, 23762881, 23 }, // Marys outlet into the Willamette
// Calapooia basin
   { 0, 0, 23763517, 22 }, // Calapooia outlet into the Willamette
// Luckiamute basin
   { 0, 0, 23762647, 21 }, // Luckiamute outlet into the Willamette
// South Santiam basin
   { 0, 0, 23785925, 10 }, //", 10, Green Peter, ,
   { 514578, 4925728, 23785687, 36 }, // 36 S. Santiam near Waterloo WTLO
   { 0, 0, 23785607, 43}, // 43 lower S. Santiam
// North Santiam basin
   { 0, 0, 23780525, 12 }, //", 12, Detroit, ,
   { 530169, 4959583, 23780481, 37 }, // 37 Mehama MEHO
   { 0, 0, 23780877, 44 }, // 44 lower N Santiam
// Lower Santiam
   { 0, 0, 23780405, 45 }, // 45, lower Santiam basin, below the confluence of NSantiam and SSantiam
// Willamette River mainstem
   { 497372.4074, 4975541.458, 23791093, 29 }, //", 29, Salem, 44.933300, -123.033300
// Molalla basin
   { 0, 0, 23800564, 20 }, // Pudding outlet into the Molalla
   { 0, 0, 23800560, 19 }, // Molalla outlet into the Willamette
// Clackamas basin
   { 0, 0, 0, 27 }, // now spare27 { 572965.6503, 4997255.136, 23809158, 27 }, //", 27, Oak Grove, 45.125000, -122.072200
   { 0, 0, 0, 40 }, // now spare40 { 572870, 4997237, -99, 40 }, // 40 Above Three Lynx Creek
   { 550742.3607, 5016481.187, 23809080, 28 }, //", 28, River Mill, 45.300000, -122.352800
   { 0, 0, 23809000, 26 }, // Clackamas outlet into the Willamette
// Yamhill basin
   { 485660, 5005815, 23796627, 42 }, // 42 McMinnville
   { 0, 0, 23791899, 31 }, // Yamhill outlet into the Willamette
// Tualatin basin
   { 511704, 5025253, 23806034, 13 }, // 13 Chicken Creek
   { 525364, 5021958, 23804832, 41 }, // 41 West Linn
   { 0, 0, 23792815, 30 }, // Tualatin outlet into the Willamette
   { 0, 0, 0, 0 }, //", 0, spare0, ,
// Johnson Creek
   { 0, 0, 23815060, 27 }, // 27 Johnson Creek
   { 0, 0, 0, 3 }, //", 3, spare3, ,
   { 0, 0, 0, 11 }, //", 11, spare11, ,
// Willamette River mainstem
   { 0, 0, 23735691, 32 }, // Willamette outlet into the Columbia
};

#define NUM_SUB_AREAS 14
double SUB_AREApourPts[NUM_SUB_AREAS][2] = 
{ // These entries should be ordered so that upstream points in a drainage come before downstream points.
   // entries are {sub_area number, comid of pour point}
{ 8, 23792815 }, // Tualatin
{ 11, 23791899 }, // Yamhill
{ 5, 23762881 }, // Marys
{ 4, 23763071 }, // Long Tom
{ 9, 23759222 }, // Coast Fork Willamette
{ 10, 23751752 }, // Middle Fork Willamette
{ 1, 23765583 }, // McKenzie basin
{ 12, 23763395}, // Upper Willamette mainstem
{ 7, 23785607 }, // South Santiam
{ 6, 23780877 }, // North Santiam
{ 13, 23780405 }, // Lower Santiam
{ 2, 23800560 }, // Molalla and Pudding
{ 3, 23809000 }, // Clackamas
{ 0, 23735691 } }; // Willamette mainstem


void AltWaterMaster::SetHBVCALIB(FlowContext *pFlowContext, Reach * pReach, int newHBVcalib, int origHBVcalib)
{
   SetUpstreamReachAttributes(pReach, newHBVcalib, origHBVcalib, m_colStreamHBVCALIB);
} // end of SetHBVCALIB()


void AltWaterMaster::SetUpstreamReachAttributes(Reach * pReach, int newValue, int origValue, int reachColNum)
{
   int polyNdx = pReach->m_polyIndex;
   int currentValue = -1; m_pReachLayer->GetData(polyNdx, reachColNum, currentValue);
   if (currentValue != origValue) return;

   m_pReachLayer->SetDataU(polyNdx, reachColNum, newValue);

   Reach * pUpstreamReachLeft = m_pFlowModel->GetReachFromNode(pReach->m_pLeft);
   Reach * pUpstreamReachRight = m_pFlowModel->GetReachFromNode(pReach->m_pRight);

   if (pUpstreamReachLeft != NULL)
   {
      SetUpstreamReachAttributes(pUpstreamReachLeft, newValue, origValue, reachColNum);
      if (pUpstreamReachRight != NULL) SetUpstreamReachAttributes(pUpstreamReachRight, newValue, origValue, reachColNum);
   }
} // end of SetUpstreamReachAttributes()


bool AltWaterMaster::PopulateHBVCALIB(FlowContext *pFlowContext)
{
   MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;
   for (int iPt = 0; iPt < NUM_HBV_CALIB_PTS; iPt++) if (HBVcalibPts[iPt][2] == -99)
   {
      int idu_index = IDUatXY(HBVcalibPts[iPt][0], HBVcalibPts[iPt][1], pLayer);
      int comid = -99; pLayer->GetData(idu_index, m_colCOMID, comid);
      HBVcalibPts[iPt][2] = (double)comid;
      CString msg; msg.Format("HBVcalibPts[%d] comid = %d HBVCALIB = %d", iPt, (int)HBVcalibPts[iPt][2], (int)HBVcalibPts[iPt][3]); Report::LogMsg(msg);
   }
   int iduCount = pLayer->GetRecordCount();
   MapLayer* pStreamLayer = (MapLayer*)pFlowContext->pFlowModel->m_pStreamLayer;
   int reachCount = pFlowContext->pFlowModel->GetReachCount();

   bool readOnlyFlag = pStreamLayer->m_readOnly;
   pStreamLayer->m_readOnly = false; pStreamLayer->SetColData(m_colStreamHBVCALIB, 0, true);

   pStreamLayer->m_readOnly = false;
   for (int iPt = 0; iPt < NUM_HBV_CALIB_PTS; iPt++) if (HBVcalibPts[iPt][2] != 0)
   {
      CString msg; msg.Format("PopulateHBVCALIB(): Processing iPt = %d", iPt); Report::LogMsg(msg);
      int comid = (int)HBVcalibPts[iPt][2];
      int polyNdx = pStreamLayer->FindIndex(m_colStreamCOMID, comid);
      if (polyNdx < 0) continue;
      int origHBVcalib = -1; pStreamLayer->GetData(polyNdx, m_colStreamHBVCALIB, origHBVcalib);
      Reach * pReach = pFlowContext->pFlowModel->FindReachFromPolyIndex(polyNdx);

      SetHBVCALIB(pFlowContext, pReach, (int)HBVcalibPts[iPt][3], origHBVcalib);
   }
   pStreamLayer->m_readOnly = readOnlyFlag;

   readOnlyFlag = pLayer->m_readOnly; pLayer->m_readOnly = false;
   int missing_reach_count = 0;
   for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
   {
      int idu_comid = -1; pLayer->GetData(idu, m_colCOMID, idu_comid);
      int reachNdx = pStreamLayer->FindIndex(m_colStreamCOMID, idu_comid);
      int HBVcalib = -1;
      if (reachNdx >= 0) pStreamLayer->GetData(reachNdx, m_colStreamHBVCALIB, HBVcalib);
      else
      {
         missing_reach_count++;
         CString msg; msg.Format("PopulateHBVCalib() idu = %d, idu_comid = %d, reachNdx = %d, missing_reach_count = %d; HBVCALIB will be set to %d for this IDU.",
            (int)idu, idu_comid, reachNdx, missing_reach_count, HBVcalib);
         Report::WarningMsg(msg);
      }
      pLayer->SetData(idu, m_colHBVCALIB, HBVcalib);
   }
   pLayer->m_readOnly = readOnlyFlag;

   return(true);
} // end of PopulateHBVCALIB()


bool AltWaterMaster::PopulateSUB_AREA(FlowContext *pFlowContext)
{
   m_pReachLayer->SetColDataU(m_colReachSUB_AREA, 0);

   for (int iPt = 0; iPt < NUM_SUB_AREAS; iPt++)
   {
      CString msg; msg.Format("PopulateSUB_AREA(): Processing iPt = %d", iPt); Report::LogMsg(msg);
      int comid = (int)SUB_AREApourPts[iPt][1];
      int polyNdx = m_pReachLayer->FindIndex(m_colStreamCOMID, comid);
      if (polyNdx < 0) continue;
      int orig_sub_area = -1; m_pReachLayer->GetData(polyNdx, m_colReachSUB_AREA, orig_sub_area);
      Reach * pReach = pFlowContext->pFlowModel->FindReachFromPolyIndex(polyNdx);

      SetUpstreamReachAttributes(pReach, (int)SUB_AREApourPts[iPt][0], orig_sub_area, m_colReachSUB_AREA);
   }

   int missing_reach_count = 0;
   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      int idu_comid = -1; m_pIDUlayer->GetData(idu, m_colCOMID, idu_comid);
      int reachNdx = m_pReachLayer->FindIndex(m_colStreamCOMID, idu_comid);
      int sub_area = -1;
      if (reachNdx >= 0) m_pReachLayer->GetData(reachNdx, m_colReachSUB_AREA, sub_area);
      else
      {
         missing_reach_count++;
         CString msg; msg.Format("PopulateSUB_AREA() idu = %d, idu_comid = %d, reachNdx = %d, missing_reach_count = %d; SUB_AREA will be set to %d for this IDU.",
            (int)idu, idu_comid, reachNdx, missing_reach_count, sub_area);
         Report::WarningMsg(msg);
      }
      m_pIDUlayer->SetDataU(idu, m_colSUB_AREA, sub_area);
   }
 
   return(true);
} // end of PopulateSUB_AREA()


bool AltWaterMaster::Init(FlowContext *pFlowContext)
   {
	Report::LogMsg("Initializing AltWaterMaster");

   m_pFlowContext = pFlowContext;
   m_pFlowModel = m_pFlowContext->pFlowModel;
   m_pEnvContext = m_pFlowContext->pEnvContext;

   m_pIDUlayer = (MapLayer *)m_pEnvContext->pMapLayer;
   m_pReachLayer = (MapLayer*)m_pFlowModel->m_pStreamLayer;
   m_pHRUlayer = (MapLayer*)m_pFlowModel->m_pHRUlayer;
   m_pNodeLayer = (MapLayer *)m_pEnvContext->pNodeLayer;

   int iduCount = m_pIDUlayer->GetRecordCount();
   int reachCount = m_pFlowModel->GetReachCount();

   bool readOnlyFlag = m_pIDUlayer->m_readOnly; m_pIDUlayer->m_readOnly = false;
   m_pIDUlayer->CheckCol(m_colUGAPOCD, "UGAPOCD", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->SetColData(m_colUGAPOCD, VData(0), true);
   m_pIDUlayer->CheckCol(m_colUGAMUNIPOD, "UGAMUNIPOD", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->SetColData(m_colUGAMUNIPOD, VData(0), true);
   
   m_pIDUlayer->CheckCol(m_colCENTROIDX, "CENTROIDX", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colCENTROIDY, "CENTROIDY", TYPE_INT, CC_AUTOADD);
   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
      {
      Poly *pIDUPoly = m_pIDUlayer->GetPolygon(idu);
      Vertex vIDU = pIDUPoly->GetCentroid();
      m_pIDUlayer->SetData(idu, m_colCENTROIDX, (int)vIDU.x);
      m_pIDUlayer->SetData(idu, m_colCENTROIDY, (int)vIDU.y);
      }
   
   m_pIDUlayer->CheckCol(m_colCENTROIDX, "CENTROIDX", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colCENTROIDY, "CENTROIDY", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colMIN_Q_WRQ, "MIN_Q_WRQ", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colWATERRIGHT, "WATERRIGHT", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colCOMID_POD, "COMID_POD", TYPE_INT, CC_AUTOADD);

   bool streamReadOnlyFlag = m_pReachLayer->m_readOnly; m_pReachLayer->m_readOnly = false;
   m_pReachLayer->CheckCol(m_colStreamINSTRM_WR, "INSTRM_WR", TYPE_INT, CC_AUTOADD);
   m_pReachLayer->SetColData(m_colStreamINSTRM_WR, VData(0), true);
   m_pReachLayer->CheckCol(m_colStreamINSTRMWRID, "INSTRMWRID", TYPE_INT, CC_AUTOADD);
   m_pReachLayer->SetColData(m_colStreamINSTRMWRID, VData(0), true);
   m_pReachLayer->CheckCol(m_colStreamINSTRM_WRQ, "INSTRM_WRQ", TYPE_FLOAT, CC_AUTOADD);
   m_pReachLayer->SetColData(m_colStreamINSTRM_WRQ, VData(0), true);
   m_pReachLayer->CheckCol(m_colStreamQ_DIV_WRQ, "Q_DIV_WRQ", TYPE_FLOAT, CC_AUTOADD);
   m_pReachLayer->SetColData(m_colStreamQ_DIV_WRQ, VData(-1.e-20), true);
   m_pReachLayer->CheckCol(m_colStreamINSTRM_REQ, _T("INSTRM_REQ"), TYPE_FLOAT, CC_AUTOADD);
   m_pReachLayer->SetColData(m_colStreamINSTRM_REQ, VData(0), true);
   m_pReachLayer->CheckCol(m_colStreamXFLUX_Y, "XFLUX_Y", TYPE_FLOAT, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colStreamOUT_IRRIG, "OUT_IRRIG", TYPE_FLOAT, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colStreamOUT_MUNI, "OUT_MUNI", TYPE_FLOAT, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colStreamIN_MUNI, "IN_MUNI", TYPE_FLOAT, CC_AUTOADD);

   m_pIDUlayer->CheckCol(m_colCOMID, "COMID", TYPE_INT, CC_MUST_EXIST);
   m_pReachLayer->CheckCol(m_colStreamCOMID, "COMID", TYPE_INT, CC_MUST_EXIST);
   m_pReachLayer->CheckCol(m_colStreamSTRM_ORDER, "STRM_ORDER", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colHBVCALIB, "HBVCALIB", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colSUB_AREA, "SUB_AREA", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colCOUNTYID, "COUNTYID", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colIDU_ID, "IDU_ID", TYPE_INT, CC_MUST_EXIST);
   m_pReachLayer->CheckCol(m_colStreamHBVCALIB, "HBVCALIB", TYPE_INT, CC_AUTOADD);
   m_pReachLayer->CheckCol(m_colReachSUB_AREA, "SUB_AREA", TYPE_INT, CC_AUTOADD);

   m_pIDUlayer->CheckCol(m_colUGB, "UGB", TYPE_INT, CC_MUST_EXIST); // m_colUGB is needed for IDUatXY() in the loop just below
   
   int colStreamECO = -1; m_pReachLayer->CheckCol(colStreamECO, "ECO", TYPE_INT, CC_MUST_EXIST);
   int colECOREGION = -1; m_pIDUlayer->CheckCol(colECOREGION, "ECOREGION", TYPE_INT, CC_MUST_EXIST);

   if (pFlowContext->pEnvContext->coldStartFlag)
      {
      CString msg; msg.Format("ColdStart: Populating Reach attributes STRMVERT0X, STRMVERT0Y, ECO, and calling PopulateHBVCALIB() and PopulateSUB_AREA().");
      Report::LogMsg(msg);

      m_pReachLayer->CheckCol(m_colStreamSTRMVERT0X, "STRMVERT0X", TYPE_INT, CC_AUTOADD);
      m_pReachLayer->CheckCol(m_colStreamSTRMVERT0Y, "STRMVERT0Y", TYPE_INT, CC_AUTOADD);

      for (MapLayer::Iterator reachPolyNdx = m_pReachLayer->Begin(); reachPolyNdx != m_pReachLayer->End(); reachPolyNdx++)
         {
         Poly *pReachPoly = m_pReachLayer->GetPolygon(reachPolyNdx);
         // Vertex vReach = pReachPoly->GetCentroid(); GetCentroid() doesn't always work
         Vertex vReach = pReachPoly->m_vertexArray.GetAt(0);
         m_pReachLayer->SetData(reachPolyNdx, m_colStreamSTRMVERT0X, (int)vReach.x);
         m_pReachLayer->SetData(reachPolyNdx, m_colStreamSTRMVERT0Y, (int)vReach.y);

         int reachPolyNdx_int = (int)reachPolyNdx; // Facilitates setting breakpoints on particular reaches.

         int idu_index = IDUatXY(vReach.x, vReach.y, m_pIDUlayer);
         int ecoregion = -1; 
         if (idu_index >= 0) m_pIDUlayer->GetData(idu_index, colECOREGION, ecoregion);
         else
            { // vReach is not in an IDU
            m_pReachLayer->GetData(reachPolyNdx, colStreamECO, ecoregion);
            int comid = -1; m_pReachLayer->GetData(reachPolyNdx, m_colStreamCOMID, comid);
            CString msg;
            msg.Format("AltWaterMaster::Init() comid = %d, reachPolyNdx = %d, vertex[0] at %f, %f; IDUatXY() returns %d. ECO will be left unchanged at %d for this reach.",
               comid, (int)reachPolyNdx, vReach.x, vReach.y, idu_index, ecoregion);
            Report::WarningMsg(msg);
            }
         m_pReachLayer->SetData(reachPolyNdx, colStreamECO, ecoregion);
         }
   
      PopulateHBVCALIB(pFlowContext);
      PopulateSUB_AREA(pFlowContext);
      } // end of coldstart logic
   else
      { 
      m_pReachLayer->CheckCol(m_colStreamSTRMVERT0X, "STRMVERT0X", TYPE_INT, CC_MUST_EXIST);
      m_pReachLayer->CheckCol(m_colStreamSTRMVERT0Y, "STRMVERT0Y", TYPE_INT, CC_MUST_EXIST);
      }

   m_pReachLayer->m_readOnly = streamReadOnlyFlag;

	// Irrigation attributes in IDU layer
	m_pIDUlayer->CheckCol( m_colUnSatIrrRequest, "UNSATIRQ_D", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colAllocatedIrrigation, "IRRALLO_Y", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colAllocatedIrrigationAf, "IRRALAF_Y", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colSWAllocatedIrrigationAf, "IRSWAF_Y", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colIRGWAF_Y, "IRGWAF_Y", TYPE_FLOAT, CC_MUST_EXIST );
	
	m_pIDUlayer->CheckCol( m_colUnsatIrrigationAf,   "USIRAF_Y", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colGWUnsatIrrigationAf, "USIRGWAF_Y", TYPE_FLOAT, CC_MUST_EXIST );

	m_pIDUlayer->CheckCol( m_colDailyAllocatedIrrigation, "IRRALLO_D", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colSWIrrDy,"SWIRR_DY", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colGWIrrDy,"GWIRR_DY", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colMaxTotDailyIrr, "MAXIRR_D", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colaveTotDailyIrr, "AVEIRR_D", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colSWUnexIrr, "SWUXIRR_DY", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colSWUnSatIrrRequestDy, "UNSWIRQ_D", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colWastedIrrDy, "WASTEIRR_D" , TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colWastedIrrYr, "WASTEIRR_Y" , TYPE_FLOAT, CC_MUST_EXIST );					
	m_pIDUlayer->CheckCol( m_colExcessIrrDy, "EXCESIRR_D" , TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colExcessIrrYr, "EXCESIRR_Y" , TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colIrrExceedMaxDutyYr, "EXCEDDTY_Y" , TYPE_FLOAT, CC_MUST_EXIST ); 

	// Irrigation request attributes in IDU layer
	m_pIDUlayer->CheckCol( m_colIrrRequestYr, "IRRRQST_Y", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colIrrRequestDy, "IRRRQST_D", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colActualIrrRequestDy, "IRRACRQ_D", TYPE_FLOAT, CC_MUST_EXIST );
	
	// Municipal attributes in the IDU layer
	m_pIDUlayer->CheckCol( m_colSWMUNALL_D, "SWMUNALL_D" , TYPE_FLOAT, CC_MUST_EXIST );
   m_pIDUlayer->CheckCol(m_colSWMUNALL_Y, "SWMUNALL_Y", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colGWMUNALL_D, "GWMUNALL_D", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colGWMUNALL_Y, "GWMUNALL_Y", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colUWOUTDOORD, "UWOUTDOORD", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colSWUnexMun, "SWUXMUN_DY", TYPE_FLOAT, CC_MUST_EXIST);
	m_pIDUlayer->CheckCol( m_colSWUnSatMunDemandDy, "UNSWMDMD_D", TYPE_FLOAT, CC_MUST_EXIST );
	m_pIDUlayer->CheckCol( m_colGWUnSatMunDemandDy, "UNGWMDMD_D", TYPE_FLOAT, CC_MUST_EXIST );

	m_pIDUlayer->CheckCol(m_colUDMAND_DY, "UDMAND_DY", TYPE_FLOAT, CC_MUST_EXIST);

	// Scarcity attributes in the IDU and Stream layer
	m_pReachLayer->CheckCol(m_colWRConflictDy, "CONFLCTDy", TYPE_INT, CC_AUTOADD);
	m_pReachLayer->CheckCol(m_colWRConflictYr, "CONFLCTYr", TYPE_INT, CC_AUTOADD);
	m_pReachLayer->CheckCol(m_colNInConflict, "NCONFLT", TYPE_INT, CC_AUTOADD);
	m_pIDUlayer->CheckCol(m_colWRShutOffMun, "SHUTOFFMUN", TYPE_INT, CC_MUST_EXIST);
	m_pIDUlayer->CheckCol(m_colWRShortG, "WR_SHORTG", TYPE_INT, CC_AUTOADD);
	m_pIDUlayer->CheckCol(m_colWRShutOff, "WR_SHUTOFF", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colWR_SH_DAY, "WR_SH_DAY", TYPE_INT, CC_AUTOADD);
	m_pReachLayer->CheckCol( m_colDSReservoir, "DS_RES", TYPE_INT, CC_MUST_EXIST);            
	// Other attributes in the IDU and stream layers used in the AltWaterMaster Method
	m_pIDUlayer->CheckCol(m_colAREA, "AREA", TYPE_FLOAT, CC_MUST_EXIST);	
   m_pIDUlayer->CheckCol(m_colPOP, "POP", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colGAL_CAP_DY, "GAL_CAP_DY", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colET_DAY, "ET_DAY", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colH2ORESIDNT, "H2ORESIDNT", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colH2OINDCOMM, "H2OINDCOMM", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colWaterDeficit, "DEFICIT", TYPE_FLOAT, CC_MUST_EXIST);
	m_pIDUlayer->CheckCol(m_colDemandFraction, "DemandFrac", TYPE_FLOAT, CC_MUST_EXIST);	
   m_pIDUlayer->CheckCol(m_colPLANTDATE,   "PLANTDATE", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colHarvDate,    "HARVDATE", TYPE_INT, CC_MUST_EXIST);
	m_pIDUlayer->CheckCol(m_colLulc_A, "LULC_A", TYPE_LONG, CC_MUST_EXIST);
	m_pIDUlayer->CheckCol(m_colLulc_B, "LULC_B", TYPE_LONG, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol( m_colIrrig_yr, "Irrig_yr", TYPE_FLOAT, CC_MUST_EXIST);	
   m_pIDUlayer->CheckCol(m_colWR_MUNI, "WR_MUNI", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colWR_INSTRM, "WR_INSTRM", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colWR_IRRIG_S, "WR_IRRIG_S", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colWR_IRRIG_G, "WR_IRRIG_G", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colWR_PROB, "WR_PROB", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colWR_PCT_POU, "WR_PCT_POU", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colWR_PCT_IDU, "WR_PCT_IDU", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colECC0, "ECC0", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colECC, "ECC", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colSTRM_ORDER, "STRM_ORDER", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colSOILH2OEST, "SOILH2OEST", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colIRR_STATE, "IRR_STATE", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colPRECIP_YR, "PRECIP_YR", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colPRECIP, "PRECIP", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colTEMP, "TEMP", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colHRU_ID, "HRU_ID", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colHRU_NDX, "HRU_NDX", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colWRPRIORITY, "WRPRIORITY", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colSNOWEVAP_Y, "SNOWEVAP_Y", TYPE_FLOAT, CC_AUTOADD);

   m_pIDUlayer->CheckCol(m_colSTRM_INDEX, "STRM_INDEX", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->SetColData(m_colSTRM_INDEX, VData(-99), true);
   m_pReachLayer->CheckCol(m_colReachLength, "LENGTH", TYPE_FLOAT, CC_MUST_EXIST);

   m_pIDUlayer->CheckCol(m_colXIRR_DUTY, "XIRR_DUTY", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->SetColData(m_colXIRR_DUTY, 1.0f, true);
   m_pIDUlayer->CheckCol(m_colXIRR_RATE, "XIRR_RATE", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->SetColData(m_colXIRR_RATE, 1.0f, true);

   m_pIDUlayer->CheckCol(m_colSEWER_NODE, "SEWER_NODE", TYPE_STRING, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colSEWER_NDX, "SEWER_NDX", TYPE_INT, CC_AUTOADD); m_pIDUlayer->SetColDataU(m_colSEWER_NDX, -1);

   if (m_pNodeLayer != NULL) m_pNodeLayer->CheckCol(m_colNodeDWF_D, "DWF_D", TYPE_DOUBLE, CC_AUTOADD);

   m_pHRUlayer->CheckCol(m_colHruET_DAY, "ET_DAY", TYPE_FLOAT, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruIRR2SOIL, "IRR2SOIL", TYPE_FLOAT, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruIRR2FASTGW, "IRR2FASTGW", TYPE_FLOAT, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruMUNI2SOIL, "MUNI2SOIL", TYPE_FLOAT, CC_AUTOADD);

	// column names specified in .xml file, may or may not exist in IDU or Stream Layer
   // m_pIDUlayer->CheckCol(m_colWREXISTS, "WREXISTS", TYPE_INT, CC_MUST_EXIST);
   m_colWREXISTS = m_pIDUlayer->GetFieldCol(m_wrExistsColName);

	m_colIrrigate = m_pIDUlayer->GetFieldCol(m_irrigateColName);

   int num_PODrecords = LoadWRDatabase(pFlowContext);
   if (num_PODrecords <= 0)
      {
      CString msg;
      msg.Format("*** AltWaterMaster::Init() num_PODrecords = %d", num_PODrecords);
      Report::ErrorMsg(msg);
      m_pIDUlayer->m_readOnly = readOnlyFlag;
      return(false);
      }

   m_iduConsecutiveShortages.SetSize(iduCount);

   // conflict data obj
	m_iduSWIrrWRSOLastWeek.SetSize(iduCount);
	m_iduSWMunWRSOLastWeek.SetSize(iduCount);
	m_iduSWLevelOneIrrWRSOAreaArray.SetSize(iduCount);
	m_iduSWLevelOneMunWRSOAreaArray.SetSize(iduCount);
	m_pouSWLevelOneIrrWRSOAreaArray.SetSize(iduCount);
	m_pouSWLevelOneMunWRSOAreaArray.SetSize(iduCount);
	m_iduSWLevelTwoIrrWRSOAreaArray.SetSize(iduCount);
	m_iduSWLevelTwoMunWRSOAreaArray.SetSize(iduCount);
	m_pouSWLevelTwoIrrWRSOAreaArray.SetSize(iduCount);
	m_pouSWLevelTwoMunWRSOAreaArray.SetSize(iduCount);
	m_iduSWIrrWRSOIndex.SetSize(iduCount);
	m_iduSWMunWRSOIndex.SetSize(iduCount);
	m_iduSWIrrWRSOWeek.SetSize(iduCount);
	m_iduSWMunWRSOWeek.SetSize(iduCount);
	m_iduLocalIrrRequestArray.SetSize(iduCount);
	m_iduActualIrrRequestDy.SetSize(iduCount);
	m_iduWastedIrr_Dy.SetSize(iduCount);	
	m_iduExceededIrr_Dy.SetSize(iduCount);

   m_iduWR.SetSize(iduCount);

	// surface water irrigation data obj
	m_iduSWUnAllocatedArray.SetSize(iduCount);
	m_iduSWIrrArrayDy.SetSize(iduCount);
	m_iduSWIrrArrayYr.SetSize(iduCount);
	m_iduSWMuniArrayDy.SetSize(iduCount);
	m_iduSWMuniArrayYr.SetSize(iduCount);
   m_iduUnsatIrrReqst_Yr.SetSize(iduCount);
	m_iduSWUnsatMunDmdYr.SetSize(iduCount);
	m_iduSWAppUnSatDemandArray.SetSize(iduCount);
	m_iduSWUnExerIrrArrayDy.SetSize(iduCount);
	m_iduSWUnExerMunArrayDy.SetSize(iduCount);
	m_iduAnnualIrrigationDutyArray.SetSize(iduCount);
	m_maxTotDailyIrr.SetSize(iduCount);
	m_aveTotDailyIrr.SetSize(iduCount);
	m_nIrrPODsPerIDUYr.SetSize(iduCount);
	m_iduExceedDutyLog.SetSize(iduCount);
	m_iduSWUnSatIrrArrayDy.SetSize(iduCount);
	m_iduSWUnSatMunArrayDy.SetSize(iduCount);	//m_iduGWUnSatIrrArrayDy.SetSize(iduCount);
	m_iduGWUnSatMunArrayDy.SetSize(iduCount);
	m_iduWastedIrr_Yr.SetSize(iduCount);
	m_iduExceededIrr_Yr.SetSize(iduCount);

	// Ground water irrigation data obj
	m_iduGWUnAllocatedArray.SetSize(iduCount);
	m_iduGWIrrArrayDy.SetSize(iduCount);
	m_iduGWIrrArrayYr.SetSize(iduCount);
	m_iduGWMuniArrayDy.SetSize(iduCount);
	m_iduGWMuniArrayYr.SetSize(iduCount);
	m_iduGWUnsatIrrReqYr.SetSize(iduCount);
	m_iduGWUnsatMunDmdYr.SetSize(iduCount);
	m_iduGWAppUnSatDemandArray.SetSize(iduCount);
	m_iduGWUnExerIrrArrayDy.SetSize(iduCount);
	m_iduGWUnExerMunArrayDy.SetSize(iduCount);
	m_iduIrrWaterRequestYr.SetSize(iduCount);

	// Stream Layer arrays
	m_reachDaysInConflict.SetSize(reachCount);
	m_dailyConflictCnt.SetSize(reachCount);

	//AltWaterMaster Daily Metrics
	this->m_dailyMetrics.SetName("ALTWM Daily Metrics");
	this->m_dailyMetrics.SetSize(15, 0);
	this->m_dailyMetrics.SetLabel(0, "Time (day)");
	this->m_dailyMetrics.SetLabel(1, "Allocated surface municipal water (m3 H2O)"); //m_SWMuniWater*SEC_PER_DAY
	this->m_dailyMetrics.SetLabel(2, "Allocated ground municipal water (m3 H2O)"); //m_GWMuniWater*SEC_PER_DAY
	this->m_dailyMetrics.SetLabel(3, "Unexercised surface irrigation water right (m3 per sec)"); //m_SWUnExIrr
	this->m_dailyMetrics.SetLabel(4, "Allocated surface irrigation water (m3 H2O)");//m_SWIrrDuty
	this->m_dailyMetrics.SetLabel(5, "Allocated ground irrigation water (m3 H2O)");//m_GWIrrDuty
   this->m_dailyMetrics.SetLabel(6, "total from out of basin (m3 per s)"); // m3/sec
   this->m_dailyMetrics.SetLabel(7, "from Bull Run WR 145069 (m3 per s)");
   this->m_dailyMetrics.SetLabel(8, "from Barney WR 142088 (m3 per s)");
   this->m_dailyMetrics.SetLabel(9, "from Spring Hill WR 173101 (m3 per s)");
   this->m_dailyMetrics.SetLabel(10, "from Clackamas WR 133611 (m3 per s)");
   this->m_dailyMetrics.SetLabel(11, "from Lake Oswego WR 135322 (m3 per s)");
   this->m_dailyMetrics.SetLabel(12, "water in reaches (m3 H2O)"); // total volume of water in reaches, m3 H2O
   this->m_dailyMetrics.SetLabel(13, "year-to-date number of IDUs whose irrigation rights have been regulated off");
   this->m_dailyMetrics.SetLabel(14, "year-to-date area of IDUs whose irrigation rights have been regulated off (ac)");
	gpFlow->AddOutputVar("ALTWM Daily Metrics", &m_dailyMetrics, "ALTWM Daily Metrics");

   //AltWaterMaster Annual Metrics
   this->m_annualMetrics.SetName("ALTWM Annual Metrics");
   this->m_annualMetrics.SetSize(18, 0);
   this->m_annualMetrics.SetLabel(0, "Year");
   this->m_annualMetrics.SetLabel(1, "Total Irrigation Water Requested (ac-ft)"); //m_irrigableIrrWaterRequestYr
   this->m_annualMetrics.SetLabel(2, "Allocated irrigation Water (ac-ft)"); //m_irrigatedWaterYr
   this->m_annualMetrics.SetLabel(3, "Allocated surface irrigation water (ac-ft)"); //m_irrigatedSurfaceYr
   this->m_annualMetrics.SetLabel(4, "Allocated ground irrigation water (ac-ft)"); //m_irrigatedGroundYr
   this->m_annualMetrics.SetLabel(5, "groundwater for rural domestic use without water right (ac-ft)"); // groundwater for rural domestic use with no water right // m_GWnoWR_Yr_m3, but in ac-ft
   this->m_annualMetrics.SetLabel(6, "num of PODs shut off"); // number of water rights suspended by regulatory action (not the number of affected IDUs)
   this->m_annualMetrics.SetLabel(7, "Unsatisfied irrigation request (ac-ft )"); //m_unSatisfiedIrrigationYr
   this->m_annualMetrics.SetLabel(8, "num of IDUs whose water rights have been shut off"); // the number of IDUs affected by water rights shutoffs
   this->m_annualMetrics.SetLabel(9, "area of IDUs whose water rights have been shut off (ac)"); 
   this->m_annualMetrics.SetLabel(10, "Irrigated area at max duty (ac)"); // m_areaDutyExceeds
   this->m_annualMetrics.SetLabel(11, "num of water rights shut off"); // the number of distinct water rights shut off (not the number of affected IDUs or affected PODs)
   this->m_annualMetrics.SetLabel(12, "total from out of basin (ac-ft)"); 
   this->m_annualMetrics.SetLabel(13, "from Bull Run (ac-ft)");
   this->m_annualMetrics.SetLabel(14, "from Barney (ac-ft)");
   this->m_annualMetrics.SetLabel(15, "from Spring Hill (ac-ft)");
   this->m_annualMetrics.SetLabel(16, "from Clackamas (ac-ft)");
   this->m_annualMetrics.SetLabel(17, "from Lake Oswego (ac-ft)");
   gpFlow->AddOutputVar("ALTWM Annual Metrics", &m_annualMetrics, "ALTWM Annual Metrics");

   // Annual Quick Check metrics
   this->m_QuickCheckMetrics.SetName("ALTWM Quick Check");
   this->m_QuickCheckMetrics.SetSize(17, 0);
   this->m_QuickCheckMetrics.SetLabel(0, "Year");
   this->m_QuickCheckMetrics.SetLabel(1, "tot in HRUs reaches and reservoirs at end of last year (mm H2O)"); // m_totH2OlastYr (m3)
   this->m_QuickCheckMetrics.SetLabel(2, "Precip (mm H2O)"); // PRECIP_YR
   this->m_QuickCheckMetrics.SetLabel(3, "GW pumping (mm H2O)"); // IRGWAF_Y (ac-ft) + GWMUNALL_Y (m3)
   this->m_QuickCheckMetrics.SetLabel(4, "High Cascades groundwater contribution mm H2O");
   this->m_QuickCheckMetrics.SetLabel(5, "from outside the basin (mm H2O)"); // m_fromOutsideBasinYr (m3)
   this->m_QuickCheckMetrics.SetLabel(6, "water added by FlowModel (mm)");
   this->m_QuickCheckMetrics.SetLabel(7, "to outside the basin (mm H2O)"); // m_toOutsideBasinYr_m3 (m3)
   this->m_QuickCheckMetrics.SetLabel(8, "AET (mm H2O)"); // ET_YR
   this->m_QuickCheckMetrics.SetLabel(9, "SNOW_EVAP (mm H2O)"); // SNOWEVAP_Y
   this->m_QuickCheckMetrics.SetLabel(10, "basin discharge (mm H2O)");
   this->m_QuickCheckMetrics.SetLabel(11, "tot in HRUs reaches and reservoirs at end of this year (mm H2O)"); // CalcTotH2O()
   this->m_QuickCheckMetrics.SetLabel(12, "irrigation (ac-ft)");
   this->m_QuickCheckMetrics.SetLabel(13, "municipal and rural domestic (ac-ft)");
   this->m_QuickCheckMetrics.SetLabel(14, "mass balance discrepancy (mm H2O)");
   this->m_QuickCheckMetrics.SetLabel(15, "mass balance discrepancy (fraction)");
   this->m_QuickCheckMetrics.SetLabel(16, "weather year");
   gpFlow->AddOutputVar("ALTWM Quick Check", &m_QuickCheckMetrics, "ALTWM Quick Check");

   // Daily Municipal Allocations = m_ugaUWallocatedDay[uga]
   this->m_DailyMunicipalAllocations.SetName("ALTWM Daily Municipal Allocations (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetSize(55, 0);
   this->m_DailyMunicipalAllocations.SetLabel(0, "Day");
   this->m_DailyMunicipalAllocations.SetLabel(1, "Metro total (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(2, "Metro from SW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(3, "Metro from GW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(4, "Metro from out-of-basin (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(5, "Metro outdoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(6, "Metro indoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(7, "Eugene-Springfield total (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(8, "Eugene-Springfield from SW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(9, "Eugene-Springfield from GW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(10, "Eugene-Springfield from out-of-basin (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(11, "Eugene-Springfield outdoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(12, "Eugene-Springfield indoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(13, "Salem total (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(14, "Salem from SW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(15, "Salem from GW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(16, "Salem from out-of-basin (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(17, "Salem outdoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(18, "Salem indoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(19, "Corvallis total (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(20, "Corvallis from SW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(21, "Corvallis from GW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(22, "Corvallis from out-of-basin (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(23, "Corvallis outdoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(24, "Corvallis indoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(25, "Albany total (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(26, "Albany from SW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(27, "Albany from GW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(28, "Albany from out-of-basin (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(29, "Albany outdoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(30, "Albany indoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(31, "McMinnville total (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(32, "McMinnville from SW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(33, "McMinnville from GW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(34, "McMinnville from out-of-basin (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(35, "McMinnville outdoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(36, "McMinnville indoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(37, "Newberg total (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(38, "Newberg from SW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(39, "Newberg from GW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(40, "Newberg from out-of-basin (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(41, "Newberg outdoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(42, "Newberg indoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(43, "Woodburn total (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(44, "Woodburn from SW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(45, "Woodburn from GW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(46, "Woodburn from out-of-basin (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(47, "Woodburn outdoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(48, "Woodburn indoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(49, "All other UGAs total (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(50, "All other UGAs from SW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(51, "All other UGAs from GW (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(52, "All other UGAs from out-of-basin (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(53, "All other UGAs outdoor use (m3 per sec)");
   this->m_DailyMunicipalAllocations.SetLabel(54, "All other UGAs indoor use (m3 per sec)");
   gpFlow->AddOutputVar("ALTWM Daily Municipal Allocations (m3 per sec)", &m_DailyMunicipalAllocations, "ALTWM Daily Municipal Allocations (m3 per sec)");

   // Annual Muni Backup Water Right Use = m_ugaMuniBackupWRuse_acft[uga]
   this->m_AnnualMuniBackupWRuse_acft.SetName("ALTWM Annual Muni Backup Water Right Use (ac-ft)");
   this->m_AnnualMuniBackupWRuse_acft.SetSize(9, 0);
   this->m_AnnualMuniBackupWRuse_acft.SetLabel(0, "Year");
   this->m_AnnualMuniBackupWRuse_acft.SetLabel(1, "Metro (ac-ft)");
   this->m_AnnualMuniBackupWRuse_acft.SetLabel(2, "Eugene-Springfield");
   this->m_AnnualMuniBackupWRuse_acft.SetLabel(3, "Salem-Keiser");
   this->m_AnnualMuniBackupWRuse_acft.SetLabel(4, "Corvallis-Philomath");
   this->m_AnnualMuniBackupWRuse_acft.SetLabel(5, "Albany");
   this->m_AnnualMuniBackupWRuse_acft.SetLabel(6, "McMinnville");
   this->m_AnnualMuniBackupWRuse_acft.SetLabel(7, "Newberg");
   this->m_AnnualMuniBackupWRuse_acft.SetLabel(8, "Woodburn");
   gpFlow->AddOutputVar("ALTWM Annual Muni Backup Water Right Use (ac-ft)", &m_AnnualMuniBackupWRuse_acft, "ALTWM Annual Muni Backup Water Right Use (ac-ft)");

   if (m_debug)
		{
	   //AltWaterMaster Annual Metrics Debug
		this->m_annualMetricsDebug.SetName("AltWaterMaster Annual Metrics Debug");
		this->m_annualMetricsDebug.SetSize(12, 0);
		this->m_annualMetricsDebug.SetLabel(0, "Year");	
		this->m_annualMetricsDebug.SetLabel(1, "Orchards vineyards and tree farms area > maxDuty (acres)"); //21 m_anGTmaxDutyArea21
		this->m_annualMetricsDebug.SetLabel(2, "Grass seed area > maxDuty (acres)"); //22 m_anGTmaxDutyArea22
		this->m_annualMetricsDebug.SetLabel(3, "Pasture area > maxDuty (acres)"); //23 m_anGTmaxDutyArea23
		this->m_annualMetricsDebug.SetLabel(4, "Wheat area > maxDuty (acres)"); //24 m_anGTmaxDutyArea24
		this->m_annualMetricsDebug.SetLabel(5, "Fallow area > maxDuty (acres)"); //25	m_anGTmaxDutyArea25
		this->m_annualMetricsDebug.SetLabel(6, "Corn area > maxDuty (acres)"); //26 m_anGTmaxDutyArea26
		this->m_annualMetricsDebug.SetLabel(7, "Clover area > maxDuty (acres)"); //27 m_anGTmaxDutyArea27
		this->m_annualMetricsDebug.SetLabel(8, "Hay area > maxDuty (acres)"); //28 m_anGTmaxDutyArea28
		this->m_annualMetricsDebug.SetLabel(9, "Other crops area > maxDuty (acres)"); //29 m_anGTmaxDutyArea29
		this->m_annualMetricsDebug.SetLabel(10, "A clover IDU where > maxDuty "); //23 m_pastureIDUGTmaxDuty
		this->m_annualMetricsDebug.SetLabel(11, "A clover IDU area where > maxDuty "); //23 m_pastureIDUGTmaxDutyArea
		gpFlow->AddOutputVar("Annual AltWM Metrics Debug", &m_annualMetricsDebug, "Annual AltWM Metrics Debug");

		//AltWM Daily Metrics Debug
		this->m_dailyMetricsDebug.SetName("AltWM Daily Metrics Debug"); 
		this->m_dailyMetricsDebug.SetSize(10, 0);
		this->m_dailyMetricsDebug.SetLabel(0, "Year");		
		this->m_dailyMetricsDebug.SetLabel(1, "Orchards vineyards and tree farms area > maxPOD (acres)"); //21 m_dyGTmaxPodArea21
		this->m_dailyMetricsDebug.SetLabel(2, "Grass seed area > maxPOD (acres)"); //22 m_dyGTmaxPodArea22
		this->m_dailyMetricsDebug.SetLabel(3, "Pasture area > maxPOD (acres)"); //23 m_dyGTmaxPodArea23
		this->m_dailyMetricsDebug.SetLabel(4, "Wheat area > maxPOD (acres)"); //24	m_dyGTmaxPodArea24
		this->m_dailyMetricsDebug.SetLabel(5, "Fallow area > maxPOD (acres)"); //25 m_dyGTmaxPodArea25
		this->m_dailyMetricsDebug.SetLabel(6, "Corn area > maxPOD (acres)"); //26	m_dyGTmaxPodArea26
		this->m_dailyMetricsDebug.SetLabel(7, "Clover area > maxPOD (acres)"); //27 m_dyGTmaxPodArea27
		this->m_dailyMetricsDebug.SetLabel(8, "Hay area > maxPOD (acres)"); //28 m_dyGTmaxPodArea28
		this->m_dailyMetricsDebug.SetLabel(9, "Other crops area > maxPOD (acres)"); //29	m_dyGTmaxPodArea29
		gpFlow->AddOutputVar("Daily AltWM Metrics Debug", &m_dailyMetricsDebug, "Daily AltWM Metrics Debug");
		}

	char units = 'm';

	// if ( m_exportDistPodComid == 1 ) ExportDistPodComid(pFlowContext, units);

   int podArrayLen = (int)m_podArray.GetCount();
   CString msg;
   msg.Format("length of podArray = %d\n", podArrayLen);
   Report::LogMsg(msg);

   m_pIDUlayer->m_readOnly = readOnlyFlag;

   if (pFlowContext->pEnvContext->coldStartFlag) 
   {
      CString msg;
      if (WriteWRreport())
      {
         msg = "Water Rights reports were written successfully.";
         Report::LogMsg(msg);
      }
      else
      {
         msg = "WriteWRreport() failed.";
         Report::WarningMsg(msg);
      }
   }

   /* Create IDUcentroids.txt //
   PCTSTR centroid_file_name = (PCTSTR)"IDUcentroids.txt";
   FILE *  oFileCentroids = NULL;
   int errNo = fopen_s(&oFileCentroids, centroid_file_name, "w");
   if (errNo != 0)
   {
      CString msg; msg.Format("WaterRights: Could not open %s output file ", centroid_file_name);
      Report::ErrorMsg(msg);
      return false;
   }
   fprintf(oFileCentroids, "IDU_ID\tCENTROIDX\tCENTROIDY\tMEAN_ELEV\n");

   int colELEV_MEAN = -1; m_pIDUlayer->CheckCol(colELEV_MEAN, "ELEV_MEAN", TYPE_FLOAT, CC_MUST_EXIST);
   int colIDU_ID = -1; m_pIDUlayer->CheckCol(colIDU_ID, "IDU_ID", TYPE_INT, CC_MUST_EXIST);
   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      int idu_id = -1; m_pIDUlayer->GetData(idu, colIDU_ID, idu_id);
      float centroidx = 0; m_pIDUlayer->GetData(idu, m_colCENTROIDX, centroidx);
      float centroidy = 0; m_pIDUlayer->GetData(idu, m_colCENTROIDY, centroidy);
      float elev_mean = 0; m_pIDUlayer->GetData(idu, colELEV_MEAN, elev_mean);
      fprintf(oFileCentroids, "%d\t%f\t%f\t%f\n", idu_id, centroidx, centroidy, elev_mean);
   }

   fclose(oFileCentroids);
   // end of block to create IDUcentroids.txt */

   /* Create "COMIDstreamNames.csv"
   PCTSTR Comid_file = (PCTSTR)"COMIDstreamNames.csv";
   FILE *oFileComid = NULL;
   int errNo = fopen_s(&oFileComid, Comid_file, "w");
   if (errNo != 0)
      {
      CString msg(" AltWaterMaster: Could not open COMIDstreamNames.csv output file ");
      msg += Comid_file;
      Report::ErrorMsg(msg);
      return false;
      }
   fprintf(oFileComid, "COMID, GNIS_NAME\n");
   int colStreamGNIS_NAME = -1; m_pReachLayer->CheckCol(colStreamGNIS_NAME, "GNIS_NAME", TYPE_STRING, CC_MUST_EXIST);
   for (MapLayer::Iterator reachNdx = m_pReachLayer->Begin(); reachNdx != m_pReachLayer->End(); reachNdx++)
      {
      Reach * pReach = pFlowContext->pFlowModel->FindReachFromIndex(reachNdx);
      if (pReach != NULL)
         {       
         int comid = pReach->m_reachID;
         CString streamName;
         m_pReachLayer->GetData(reachNdx, colStreamGNIS_NAME, streamName);
         fprintf(oFileComid, "%d, %s\n", comid, streamName);
         }
      }

   fclose(oFileComid);
   */ // end of block to create COMIDstreamNames.csv

   /* Create "WaterRights.csv"
   PCTSTR WR_file = (PCTSTR)"WaterRights.csv";
   FILE *oFile = NULL;
   int errNo = fopen_s(&oFile, WR_file, "w");
   if (errNo != 0)
      {
      CString msg(" AltWaterMaster: Could not open WaterRights.csv output file ");
      msg += WR_file;
      Report::ErrorMsg(msg);
      return false;
      }
   fprintf(oFile, "IDU_INDEX, IDU_AREA_AC, POUID, OVERLAP_AREA_AC, POU_PERCENT, WATERRIGHTID, PODID, USECODE, YEAR, IN_USE, POD_STATUS, WREXISTS, PERMITCODE, POU_ROW\n");

   int podArrayLen = (int)m_podArray.GetCount();

   // loop thru the IDUs: for each IDU with WREXISTS!=0, find the active WaterRights objects associated with it.
   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
      {
      CString msg;
      int count = 0;
      int wr_exists = 0;
      m_pIDUlayer->GetData(idu, m_colWRExists, wr_exists);    //(idu, m_wrExistsColName, &wr_exists);
      if (wr_exists == 0) continue;

      int idu_id = -1; m_pIDUlayer->GetData(idu, m_colIDU_ID, idu_id);
      float idu_area = 0.f; m_pIDUlayer->GetData(idu, m_colIDUArea, idu_area);

      int pod_count = 0;
      int tot_pou_count_for_idu = 0;
      int waterrightid_changes = 0;
      int prev_waterrightid = -1;
      for (int podNdx = 0; podNdx < podArrayLen; podNdx++)
         {
         int pouID = m_podArray[podNdx]->m_pouID;
         PouKeyClass pouLookupKey; pouLookupKey.pouID = pouID;
         vector<int> *pouIDUs = 0; pouIDUs = &m_pouInputMap[pouLookupKey];
         if (pouIDUs->size() <= 0) continue;
         int pouIDUs_ndx, tempIDU_ID;
         int pou_count = 0;
         for (pouIDUs_ndx = 0; pouIDUs_ndx < pouIDUs->size(); pouIDUs_ndx++)
            {
            tempIDU_ID = m_pouDb.GetAsInt(m_colPouIDU_ID, pouIDUs->at(pouIDUs_ndx));
            if (idu_id != tempIDU_ID) continue;
            pou_count++;
            int pouRow = pouIDUs->at(pouIDUs_ndx);
            int pouid = -1; pouid = m_pouDb.GetAsInt(m_colPouPOU_ID, pouRow);
            float overlap_area = -1.f; overlap_area = m_pouDb.GetAsFloat(m_colPouArea, pouRow);
            float percent = -1.f; percent = m_pouDb.GetAsFloat(m_colPouPERCENT_POU, pouRow);
            int this_waterrightid = m_podArray[podNdx]->m_wrID;
            if (prev_waterrightid != -1 && this_waterrightid != prev_waterrightid) waterrightid_changes++;
            prev_waterrightid = this_waterrightid;
            int podid = m_podArray[podNdx]->m_podID;
			WR_USE useCode = m_podArray[podNdx]->m_useCode;
			WR_PERMIT permitCode = m_podArray[podNdx]->m_permitCode;
			int year = m_podArray[podNdx]->m_priorYr;
            bool in_use = m_podArray[podNdx]->m_inUse;
            WR_PODSTATUS pod_status = m_podArray[podNdx]->m_podStatus;

			float m2_per_ac = 4046.85642f;
			float idu_area_ac = idu_area / m2_per_ac;
			float overlap_area_ac = overlap_area / m2_per_ac;
			fprintf(oFile, "%d, %f, %d, %f, %f, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
               idu_index, idu_area_ac, pouid, overlap_area_ac, percent, this_waterrightid, podid, (int)useCode, year, in_use, (int)pod_status, wr_exists, permitCode, pouRow);
			if (overlap_area_ac > 1.01f*idu_area_ac)
				{
				CString msg;
				msg.Format(" AltWM::Init() overlap area > 101% of IDU area. IDU_INDEX = %d, POUID = %d, idu_area_ac = %f, overlap_area_ac = %f, pouRow = %d",
					idu_index, pouid, idu_area_ac, overlap_area_ac, pouRow);
				Report::WarningMsg(msg);
				}

            } // end of loop thru pouIDUs
         if (pou_count < 1) continue;

         if (pou_count > 1)
            {
            msg.Format("idu_index %d for podNdx %d has %d POUs.", idu_index, podNdx, pou_count);
            Report::LogMsg(msg);
            }
         pod_count++;
         tot_pou_count_for_idu += pou_count;
         } // end of loop on podNdx

      if (pod_count <= 0)
         {
         msg.Format("WW2100AP: idu %d had non-zero WREXISTS but no active water right data.  Clearing WREXISTS etc now.", idu_index);
         Report::LogMsg(msg);
		 m_pIDUlayer->SetData(idu_index, m_colWRExists, 0);
		 m_pIDUlayer->SetData(idu, m_colWR_MUNI, 0); 
		 m_pIDUlayer->SetData(idu, m_colWR_INSTRM, 0); 
		 m_pIDUlayer->SetData(idu, m_colWR_IRRIG_S, 0); 
		 m_pIDUlayer->SetData(idu, m_colWR_IRRIG_G, 0); 
		 }
      else if ((pod_count > 1 || tot_pou_count_for_idu>1) && waterrightid_changes>1)
         {
         msg.Format("WW2100AP: idu %d has %d PODs and a total of %d POUs with %d WATERRIGHTID changes\n",
            idu_index, pod_count, tot_pou_count_for_idu, waterrightid_changes);
         Report::LogMsg(msg);
         }

      } // end of loop thru idus

   // close the output file
   fclose(oFile);
   */
   
   // Initialize m_availableDischarge for use by CalcTotH2OinReaches()
   for (int i = 0; i < reachCount; i++)
      {
      Reach *pReach = pFlowContext->pFlowModel->GetReach(i); 
      pReach->m_availableDischarge = pReach->GetDischarge();
      }

   msg.Format("*** AltWaterMaster::Init() CalcTotH2OinReaches() = %f, CalcTotH2OinReservoirs() = %f, CalcTotH2OinHRUs = %f, CalcTotH2O() = %f",
      pFlowContext->pFlowModel->CalcTotH2OinReaches(), pFlowContext->pFlowModel->CalcTotH2OinReservoirs(), pFlowContext->pFlowModel->CalcTotH2OinHRUs(), pFlowContext->pFlowModel->CalcTotH2O());
   Report::LogMsg(msg);

   /*
   // Dump the POD array and the reach array.
   PCTSTR WR_file = (PCTSTR)"PODarray_internal.csv";
   FILE *oFile = NULL;
   int errNo = fopen_s(&oFile, WR_file, "w");
   if (errNo != 0)
      {
      CString msg; msg.Format(" AltWaterMaster::Init() -  ERROR: Could not open output file %s", WR_file);
      Report::ErrorMsg(msg);
      return false;
      }
   fprintf(oFile, "POD_INDEX (index to m_podArray[]), "
      "wrID, "
      "xCoord, "
      "yCoord, "
      "podID, "
      "pouID, "
      "appCode, "
      "permitCode, "
      "podRate_cfs, "
      "podUseRate_cfs, "
      "useCode, "
      "supp, "
      "priorDoy, "
      "priorYr, "
      "beginDoy, "
      "endDoy, "
      "pouRate, "
      "pouArea, "
      "podStatus, "
      "reachComid, "
      "streamIndex (aka polyIndex in streams.dbf), "
      "wrAnnualDuty, "
      "distanceToReach, "
      "maxDutyPOD, "
      "stepRequest, "
      "instreamWRlength_m, "
      "specialCode\n");

   for (int podNdx = 0; podNdx < podArrayLen; podNdx++) // if (m_podArray[podNdx]->m_useCode==WRU_INSTREAM)
      fprintf(oFile,
      "%d, %d, %f, %f, %d, %d, %d, %d, %f, %f, %d, %d, "
      "%d, %d, %d, %d, %f, %f, %d, %d, %d, %f, %f, %f, "
      "%f, %f, %d\n",
      podNdx,
      m_podArray[podNdx]->m_wrID,
      m_podArray[podNdx]->m_xCoord,
      m_podArray[podNdx]->m_yCoord,
      m_podArray[podNdx]->m_podID,
      m_podArray[podNdx]->m_pouID,
      m_podArray[podNdx]->m_appCode,
      m_podArray[podNdx]->m_permitCode,
      m_podArray[podNdx]->m_podRate_cfs,
      m_podArray[podNdx]->m_podUseRate_cfs,
      m_podArray[podNdx]->m_useCode,
      m_podArray[podNdx]->m_supp,
      m_podArray[podNdx]->m_priorDoy,
      m_podArray[podNdx]->m_priorYr,
      m_podArray[podNdx]->m_beginDoy,
      m_podArray[podNdx]->m_endDoy,
      m_podArray[podNdx]->m_pouRate,
      m_podArray[podNdx]->m_pouArea,
      m_podArray[podNdx]->m_podStatus,
      m_podArray[podNdx]->m_reachComid,
      m_podArray[podNdx]->m_streamIndex,
      m_podArray[podNdx]->m_wrAnnualDuty,
      m_podArray[podNdx]->m_distanceToReach,
      m_podArray[podNdx]->m_maxDutyPOD,
      m_podArray[podNdx]->m_stepRequest,
      m_podArray[podNdx]->m_instreamWRlength_m,
      m_podArray[podNdx]->m_specialCode);

   fclose(oFile);

   WR_file = (PCTSTR)"ReachArray_internal.csv";
   oFile = NULL;
   errNo = fopen_s(&oFile, WR_file, "w");
   if (errNo != 0)
      {
      CString msg; msg.Format(" AltWaterMaster::Init() -  ERROR: Could not open output file %s", WR_file);
      Report::ErrorMsg(msg);
      return false;
      }
   fprintf(oFile, 
      "ReachArrayIndex, "
      "reachIndex, "
      "polyIndex, "
      "pDown->polyIndex, "
      "pLeft->polyIndex, "
      "pRight->polyIndex, "
      "reachID, "
      "length, "
      "slope, "
      "deltaX, "
      "streamOrder\n");

   for (int i = 0; i < reachCount; i++)
      {
      Reach *pReach = pFlowContext->pFlowModel->GetReach(i);
      fprintf(oFile,
         "%d, %d, %d, %d, %d, %d, %d, %f, %f, %f, %d\n",
         i, 
         pReach->m_reachIndex, 
         pReach->m_polyIndex, 
         pReach->m_pDown != NULL ? pReach->m_pDown->m_polyIndex : -1, 
         pReach->m_pLeft != NULL ? pReach->m_pLeft->m_polyIndex : -1,
         pReach->m_pRight != NULL ? pReach->m_pRight->m_polyIndex : -1,
         pReach->m_reachID, 
         pReach->m_length, 
         pReach->m_slope, 
         pReach->m_deltaX, 
         pReach->m_streamOrder);
      }
   fclose(oFile);
   */

   /* Logic to read "HUC4_86_Subarea0.csv" from Kathleen Moore and switch IDUs from subarea 0 to subarea 11
   // i.e. from mainstem to Yamhill.
   VDataObj toYamhill;
   int numRecords = toYamhill.ReadAscii("HUC4_86_SUBAREA0.csv"); 

   if (numRecords <= 0)
      {
      CString msg;
      msg.Format("AltWM::Init() could not load HUC4_86_SUBAREA0.csv\n");
      Report::InfoMsg(msg);
      return 0;
      }

   int toYamhillColIDU_INDEX = toYamhill.GetCol("IDU_INDEX");

   if (toYamhillColIDU_INDEX < 0)
      {
      CString msg;
      msg.Format("AltWM::Init() Couldn't find IDU_INDEX column in HUC4_86_SUBAREA0.csv\n");
      Report::ErrorMsg(msg);
      return 0;
      }

   int colSUB_AREA = -1; m_pIDUlayer->CheckCol(colSUB_AREA, "SUB_AREA", TYPE_INT, CC_MUST_EXIST);
   if (colSUB_AREA < 0)
      {
      CString msg;
      msg.Format("AltWM::Init() Couldn't find SUB_AREA column in idu layer\n");
      Report::ErrorMsg(msg);
      return 0;
      }

   int readOnly_flag = m_pIDUlayer->m_readOnly;  m_pIDUlayer->m_readOnly = false;
   for (int i_rec = 0; i_rec < numRecords; i_rec++)
      {
      int idu_index = toYamhill.GetAsInt(toYamhillColIDU_INDEX, i_rec);
      int idu_rec = m_pIDUlayer->FindIndex(m_colIDUIndex, idu_index);
      int sub_area = -1; m_pIDUlayer->GetData(idu_rec, colSUB_AREA, sub_area);
      if (sub_area != 0)
         {
         CString msg; msg.Format("*** AltWM::Init() idu_index = %d, sub_area = %d is not zero.", idu_index, sub_area); 
         Report::LogMsg(msg);
         }
      m_pIDUlayer->SetData(idu_rec, colSUB_AREA, 11); // 11 = Yamhill sub_area
      }
   m_pIDUlayer->m_readOnly = readOnly_flag;

      {
      CString msg; msg.Format("*** AltWM::Init() Wrote 11 to SUB_AREA for %d IDUs", numRecords);
      Report::LogMsg(msg);
      }
   */ // end of logic to read "HUC4_86_Subarea0.csv"


   return true;
	}

bool AltWaterMaster::InitRun(FlowContext *pFlowContext)
	{
   MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;
   MapLayer* pStreamLayer = (MapLayer*)pFlowContext->pFlowModel->m_pStreamLayer;

   pLayer->SetColData(m_colXIRR_DUTY, 1.0f, true);
   pLayer->SetColData(m_colXIRR_RATE, 1.0f, true);
      
// Convert max rate in cfs per ac to max diversion in mm per day
   m_maxIrrDiversion_mm_per_day = (((m_maxRate_cfs_per_ac * (1.f / FT3_PER_M3)) / M2_PER_ACRE) / (1.f / SEC_PER_DAY)) * 1000.f;
      {
      CString msg; msg.Format("*** AltWaterMaster::InitRun() m_maxIrrDiversion_mm_per_day = %f", m_maxIrrDiversion_mm_per_day); Report::LogMsg(msg);
      }

   m_QuickCheckMetrics.ClearRows();
   m_dailyMetrics.ClearRows();
   m_annualMetrics.ClearRows();
   m_DailyMunicipalAllocations.ClearRows();
   m_AnnualMuniBackupWRuse_acft.ClearRows();
   // m_annualMetricsDebug.ClearRows();
   // m_dailyMetricsDebug.ClearRows();
   
   for (int podNdx = 0; podNdx < m_podArray.GetCount(); podNdx++)
      {
      WR_SPECIALCODE wrsc = m_podArray[podNdx]->m_specialCode;
      switch (wrsc)
         {
         case WRSC_NONE:
         case WRSC_MUNIBACKUP:
            m_podArray[podNdx]->m_podStatus = WRPS_NONCANCELED;
            break;
         case WRSC_NEWINSTREAM:
            m_podArray[podNdx]->m_podStatus = (m_useNewInstreamWR == 1) ? WRPS_NONCANCELED : WRPS_CANCELED;
            break;
         case WRSC_NOT_NEWINSTREAM_ONLY:
            m_podArray[podNdx]->m_podStatus = (m_useNewInstreamWR == 1) ? WRPS_CANCELED : WRPS_NONCANCELED;
            break;
         case WRSC_OBSOLETE:
            m_podArray[podNdx]->m_podStatus = WRPS_CANCELED;
            break;
         default:
            {
            CString msg;
            msg.Format("AltWaterMaster::InitRun() podNdx = %d, wrsc = %d is unrecognized.", podNdx, wrsc);
            Report::ErrorMsg(msg);
			return(false);
            }
            break;
         }
      }

	for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
		{
		//Initiate arrays for water right conflict metrics and allocation arrays
		m_iduSWLevelOneIrrWRSOAreaArray[idu] = 0.0f;
		m_iduSWLevelOneMunWRSOAreaArray[idu] = 0.0f;
		m_pouSWLevelOneIrrWRSOAreaArray[idu] = 0.0f;
		m_pouSWLevelOneMunWRSOAreaArray[idu] = 0.0f;
		m_iduSWLevelTwoIrrWRSOAreaArray[idu] = 0.0f;
		m_iduSWLevelTwoMunWRSOAreaArray[idu] = 0.0f;
		m_pouSWLevelTwoIrrWRSOAreaArray[idu] = 0.0f;
		m_pouSWLevelTwoMunWRSOAreaArray[idu] = 0.0f;
		m_iduSWIrrWRSOIndex[idu] = -1;
		m_iduSWMunWRSOIndex[idu] = -1;
		m_iduSWIrrWRSOWeek[idu] = -1;
		m_iduSWMunWRSOWeek[idu] = -1;
		m_iduLocalIrrRequestArray[idu] = 0.0f;
		m_iduSWIrrWRSOLastWeek[idu] = -999;
	   m_iduSWMunWRSOLastWeek[idu] = -999;
		m_iduAnnualIrrigationDutyArray[idu] = 0.f;
		m_iduActualIrrRequestDy[idu] = 0.0f;

		// surface water irrigation 
		m_iduSWUnAllocatedArray[idu] = 0.0f;
		m_iduSWIrrArrayDy[idu] = 0.0f;
		m_iduSWIrrArrayYr[idu] = 0.0f;
		m_iduSWMuniArrayYr[idu] = 0.0f;
		m_iduSWUnsatMunDmdYr[idu] = 0.0f;
		m_iduSWAppUnSatDemandArray[idu] = 0.0f;
		m_iduSWUnExerIrrArrayDy[idu] = 0.0f;
		m_iduSWUnExerMunArrayDy[idu] = 0.0f;
		m_iduSWUnSatIrrArrayDy[idu] = 0.0f;
		m_iduSWUnSatMunArrayDy[idu] = 0.0f;
	   m_iduGWUnSatMunArrayDy[idu] = 0.0f;

		// ground water irrigation 
		m_iduGWUnAllocatedArray[idu] = 0.0f;
		m_iduGWIrrArrayDy[idu] = 0.0f;
		m_iduGWIrrArrayYr[idu] = 0.0f;
		m_iduGWMuniArrayYr[idu] = 0.0f;
		m_iduGWUnsatIrrReqYr[idu] = 0.0f;
		m_iduGWUnsatMunDmdYr[idu] = 0.0f;
		m_iduGWAppUnSatDemandArray[idu] = 0.0f;
		m_iduGWUnExerIrrArrayDy[idu] = 0.0f;
		m_iduGWUnExerMunArrayDy[idu] = 0.0f;
		m_iduIrrWaterRequestYr[idu] = 0.0f;

		//total irrigation 
		m_maxTotDailyIrr[idu] = 0.0f;
		m_aveTotDailyIrr[idu] = 0.0f;
		m_nIrrPODsPerIDUYr[idu] = 0;
		m_iduWastedIrr_Yr[idu] = 0.0f;
		m_iduExceededIrr_Yr[idu] = 0.0f;
		m_iduWastedIrr_Dy[idu] = 0.0f;
		m_iduExceededIrr_Dy[idu] =0.0f;

		//Duty
		m_iduExceedDutyLog[idu] = 0;

		}
	
   bool readOnlyFlag = pLayer->m_readOnly;
   pLayer->m_readOnly = false;
   pLayer->SetColData(m_colGAL_CAP_DY, VData(0), true);
   pLayer->m_readOnly = readOnlyFlag;

	int reachCount = pFlowContext->pFlowModel->GetReachCount();

	for (int i = 0; i < reachCount; i++)
		{

		float reachLength = 0.0f;
			
		m_reachDaysInConflict[i] = 0;

		m_dailyConflictCnt[i] = 0;

		pStreamLayer->GetData( i, m_colReachLength, reachLength);
									
		m_basinReachLength += reachLength;

		} // endfor reach

   m_totH2OlastYr_m3 = pFlowContext->pFlowModel->CalcTotH2O();

   CString msg;
   msg.Format("*** AltWaterMaster::InitRun() CalcTotH2OinReaches() = %f, CalcTotH2OinReservoirs() = %f, CalcTotH2OinHRUs = %f, CalcTotH2O() = %f",
      pFlowContext->pFlowModel->CalcTotH2OinReaches(), pFlowContext->pFlowModel->CalcTotH2OinReservoirs(), pFlowContext->pFlowModel->CalcTotH2OinHRUs(), pFlowContext->pFlowModel->CalcTotH2O());
   Report::LogMsg(msg);

   return true;
   } // end of AltWaterMaster::InitRun()


/* Commented out because it isn't getting reached at run time.  I don't know why. Dave Conklin 11/18/18
The Water Rights report stuff is duplicated at the end of EndYear(), for now.
bool AltWaterMaster::EndRun(FlowContext *pFlowContext)
{
   if (m_pFlowModel->m_waterRightsReportInputVar > 0)
   {
      CString msg;
      if (WriteWRreport())
      {
         msg = "Water Rights report was written successfully.";
         Report::LogMsg(msg);
      }
      else
      {
         msg = "WriteWRreport() failed.";
         Report::WarningMsg(msg);
      }
   }

   return(true);
} // end of AltWaterMaster::EndRun()
*/


bool AltWaterMaster::Step(FlowContext *pFlowContext)
	{
	// basic idea.  
	// 1. The Point of Diversion (POD) input data file should be sorted asending by 
	//    priority year, priority day of year (doy), season begin doy, and season end doy
	// 2. Iterate through POD water rights input data file
	// 3. For each water right (WR), allocate the right if possible, based on demand, and 
	//    the Point of Use data input file.

   int   plantDate = 0;
	int   harvDate = 0;
	vector<int> *iduNdxVec = 0;

	MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;
   MapLayer* pStreamLayer = (MapLayer*)pFlowContext->pFlowModel->m_pStreamLayer;

   m_SWirrigWRpriority = 0; 

	// 0 based year of run
	m_envTimeStep = pFlowContext->pEnvContext->yearOfRun;
/*
   {
   CString msg;
   int doy = pFlowContext->dayOfYear;
   msg.Format("*** AltWM::Step() year = %d, day = %d", pFlowContext->pEnvContext->yearOfRun, doy);
   if (((float)doy/10.f)*10.f==doy) 
      Report::LogMsg(msg);
   }
*/
   // set to zero at start of processing for each daily time step
   for (int uga = 0; uga <= MAX_UGA_NDX; uga++) 
      {
      m_ugaUWallocatedDayFromSW[uga] = 0.f;
      m_ugaUWallocatedDayFromGW[uga] = 0.f;
      m_ugaLocalUWdemandArray[uga] = 0.f;
      m_fromOutsideBasinDy[uga] = 0.f;
      m_ugaUWoutdoorDay_m3_per_sec[uga] = 0.f;
      m_ugaUWindoorDay_m3_per_sec[uga] = 0.f;
      } // end of loop on UGAs
   m_fromBullRunDy_m3_per_s = 0.f;
   m_fromSpringHillDy_m3_per_s = 0.f;
   m_fromBarneyDy_m3_per_s = 0.f;
   m_fromClackamasDy_m3_per_s = 0.f;
   m_fromLakeOswegoDy_m3_per_s = 0.f;

	m_SWUnExIrr = 0.0f;
	m_SWIrrDuty = 0.0f;
	m_GWIrrDuty = 0.0f;
	m_SWIrrWaterMmDy = 0.0f;
	m_GWIrrWaterMmDy = 0.0f;
	m_SWMuniWater=0.0f;
	m_GWMuniWater=0.0f;
	m_IrrFromAllocationArrayDy = 0.0f;
	m_IrrFromAllocationArrayDyBeginStep = 0.0f;
	m_SWIrrWaterMmDy = 0.0f;
   m_SWMuniWater = 0.0f;
	m_SWIrrUnSatDemand = 0.0f;
	m_GWIrrWaterMmDy = 0.0f;
	m_GWMuniWater = 0.0f;
	m_GWIrrUnSatDemand = 0.0f;
	m_SWunallocatedIrrWater = 0.0f;
	m_SWunsatIrrWaterDemand = 0.0f;  

	int reachCount = pFlowContext->pFlowModel->GetReachCount();

	#pragma omp parallel for
	for (int i = 0; i < reachCount; i++)
		{
		Reach *pReach = pFlowContext->pFlowModel->GetReach(i); // Note: these are guaranteed to be non-phantom
		pReach->m_availableDischarge = pReach->GetDischarge();
		pReach->m_instreamWaterRightUse = 0.;
		pFlowContext->pFlowModel->m_pStreamLayer->m_readOnly = false;
		pFlowContext->pFlowModel->m_pStreamLayer->SetData(i, m_colWRConflictDy, 0);
		pFlowContext->pFlowModel->m_pStreamLayer->m_readOnly = true;
		m_dailyConflictCnt[i] = 0;
		}

   pStreamLayer->m_readOnly = false;
   pStreamLayer->SetColData(m_colStreamQ_DIV_WRQ, VData(-1.e-20f), true);
   pStreamLayer->SetColData(m_colStreamINSTRM_REQ, VData(0), true);
   pStreamLayer->m_readOnly = true;

   pStreamLayer->SetColDataU(m_colStreamOUT_IRRIG, 0);
   pStreamLayer->SetColDataU(m_colStreamOUT_MUNI, 0);
   pStreamLayer->SetColDataU(m_colStreamIN_MUNI, 0);


   if (m_pNodeLayer != NULL) m_pNodeLayer->SetColDataU(m_colNodeDWF_D, 0.);

   for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
      {
		m_iduSWIrrArrayDy[idu] = 0.f;
		m_iduGWIrrArrayDy[idu] = 0.f;
		m_iduSWMuniArrayDy[idu] = 0.f;
		m_iduGWMuniArrayDy[idu] = 0.f;
		m_nIrrPODsPerIDUYr[idu] = 0;
		m_iduActualIrrRequestDy[idu] = 0.0f;
		m_iduSWUnExerIrrArrayDy[idu] = 0.f;
		m_iduGWUnExerIrrArrayDy[idu] = 0.0f;
		m_iduSWUnExerMunArrayDy[idu] = 0.f;
		m_iduGWUnExerMunArrayDy[idu] = 0.0f;
		m_iduSWUnSatIrrArrayDy[idu] = 0.0f;
		m_iduSWUnSatMunArrayDy[idu] = 0.0f;
		//m_iduGWUnSatIrrArrayDy[idu] = 0.0f;
	   m_iduGWUnSatMunArrayDy[idu] = 0.0f;
		m_iduWastedIrr_Dy[idu] = 0.0f;
		m_iduExceededIrr_Dy[idu] =0.0f;

		m_IrrFromAllocationArrayDyBeginStep += m_pWaterAllocation->m_iduIrrAllocationArray[idu];

		int lulcA = 0;
		float idu_area = 0.0f;

		pLayer->GetData( idu, m_colLulc_A, lulcA );
		
      // float biological_water_demand = m_pWaterAllocation->m_iduIrrRequestArray[idu];
		pLayer->GetData(idu, m_colIrrigate, m_irrigateDecision);
      float smoothed_irr_req = m_pWaterAllocation->m_iduIrrRequestArray[idu]; // 0.f;
/*
      int doy0 = pFlowContext->dayOfYear; // day of year, Jan 1 = 0
      if (m_irrigateDecision == 1 && 
         (doy0 + 1) >= m_irrDefaultBeginDoy && (doy0 + 1) <= m_irrDefaultEndDoy && 
         biological_water_demand > 0.f)
         {
         float unsmoothed_irr_req = biological_water_demand < m_maxIrrDiversion_mm_per_day ? 
            biological_water_demand : m_maxIrrDiversion_mm_per_day;

         // Update smoothed_irr_req.
         if ((doy0 + 1) == m_irrDefaultBeginDoy) smoothed_irr_req = unsmoothed_irr_req;
         else if ((doy0 + 1) > m_irrDefaultBeginDoy && (doy0 + 1) <= m_irrDefaultEndDoy)
            {
            float smoothing_days = 7.f; // smoothing time constant, as specified by Bill Jaeger
            float smoothing_days_eff = ((doy0 + 1) - m_irrDefaultBeginDoy) < smoothing_days ?
               ((doy0 + 1) - m_irrDefaultBeginDoy) : smoothing_days;

            float prev_smoothed_irr_req = 0.f;
            pLayer->GetData(idu, m_colActualIrrRequestDy, prev_smoothed_irr_req); // mm/day; read yesterday's IRRACRQ_D

            smoothed_irr_req = unsmoothed_irr_req*(1.f / smoothing_days) + prev_smoothed_irr_req*((smoothing_days - 1.f)/smoothing_days);
            }
         else smoothed_irr_req = 0.f;
         }
      else smoothed_irr_req = 0.f;
*/
      m_iduLocalIrrRequestArray[idu] = smoothed_irr_req;

		pLayer->GetData( idu, m_colPLANTDATE, plantDate );
      pLayer->GetData( idu, m_colHarvDate,  harvDate );
		pLayer->GetData(idu, m_colAREA, idu_area);

		//Reset arrays for next envision time step
		if (pFlowContext->dayOfYear == 0)
			{
			// conflict
			m_iduSWIrrWRSOWeek[idu] = -1;
			m_iduSWMunWRSOWeek[idu] = -1;
			m_iduSWLevelOneIrrWRSOAreaArray[idu] = 0.0f;
			m_iduSWLevelOneMunWRSOAreaArray[idu] = 0.0f;
			m_pouSWLevelOneIrrWRSOAreaArray[idu] = 0.0f;
			m_pouSWLevelOneMunWRSOAreaArray[idu] = 0.0f;
			m_iduSWLevelTwoIrrWRSOAreaArray[idu] = 0.0f;
			m_iduSWLevelTwoMunWRSOAreaArray[idu] = 0.0f;
			m_pouSWLevelTwoIrrWRSOAreaArray[idu] = 0.0f;
			m_pouSWLevelTwoMunWRSOAreaArray[idu] = 0.0f;
			m_iduAnnualIrrigationDutyArray[idu] = 0.0f;

			//surface water
			m_iduSWUnAllocatedArray[idu] = 0.0f;
			m_iduSWUnsatMunDmdYr[idu] = 0.0f;

			m_iduSWAppUnSatDemandArray[idu] = 0.0f;			
			m_iduSWIrrArrayYr[idu] = 0.0f;
			m_iduSWMuniArrayYr[idu] = 0.0f;

		   //ground water
			m_iduGWUnAllocatedArray[idu] = 0.0f;
			m_iduGWUnsatIrrReqYr[idu] = 0.0f;
			m_iduGWUnsatMunDmdYr[idu] = 0.0f;

			m_iduGWAppUnSatDemandArray[idu] = 0.0f;
			m_iduGWIrrArrayYr[idu] = 0.0f;
			m_iduGWMuniArrayYr[idu] = 0.0f;
			
			//totals
			m_iduIrrWaterRequestYr[idu] = 0.0f;
			m_maxTotDailyIrr[idu] = 0.0f;
			m_aveTotDailyIrr[idu] = 0.0f;

			//duty
			m_iduExceedDutyLog[idu] = 0;
			}

		// m_iduIrrRequestArray is in units mm/day, conversion reference http://www.fao.org/docrep/x0490e/x0490e04.htm
		// ET mm/day = 10 m3/ha/day
		// AREA in IDU layer is assumed to be meters squared m2
		// availSourceFlow = m3/sec
		// 10000 m2/ha

		float iduAreaM2 = 0.0f;

		pLayer->GetData(idu, m_colAREA, iduAreaM2);

		float iduAreaAc = iduAreaM2 * ACRE_PER_M2;

      float iduIrrRequest_acft = (m_iduLocalIrrRequestArray[idu] / 1000.f) * iduAreaM2 / M3_PER_ACREFT; // acre-feet per day

		if (m_irrigateDecision == 1) m_irrigableIrrWaterRequestYr += iduIrrRequest_acft; // irrigation water requested, ac-ft

		m_irrWaterRequestYr += iduIrrRequest_acft;  // acre-feet per day
		m_iduIrrWaterRequestYr[idu] += iduIrrRequest_acft; // acre-feet per day
				
      float dailyUWdemand_m3_per_sec;
      pLayer->GetData(idu, m_colUDMAND_DY, dailyUWdemand_m3_per_sec); // UDMAND_DY is calculated and stored by DailyUrbanWaterDemand::CalcDailyUrbanWaterDemand().
      int uga = 0; pLayer->GetData(idu, m_colUGB, uga);
      if (uga > 0 && uga <= MAX_UGA_NDX) 
         m_ugaLocalUWdemandArray[uga] += dailyUWdemand_m3_per_sec; // m3/sec

		}//end IDU

   m_maxBullRunAllocationToday = 0.67f*m_ugaLocalUWdemandArray[UGA_Metro];
   m_maxClackamasAllocationToday = 0.10f*m_ugaLocalUWdemandArray[UGA_Metro];
   m_maxLakeOswegoAllocationToday = 0.10f*m_ugaLocalUWdemandArray[UGA_Metro];

   int jday1 = pFlowContext->dayOfYear + 1; // Julian day using Jan 1 = 1
   if (167 <= jday1 && jday1 <= 319) // if June 15 <= jday1 <= Nov 15
      { 
	   m_maxSpringHillAllocationToday = 0.05f*m_ugaLocalUWdemandArray[UGA_Metro];
	   m_maxBarneyAllocationToday = 0.20f*m_ugaLocalUWdemandArray[UGA_Metro];
      }
   else
      { 
      m_maxSpringHillAllocationToday = 0.20f*m_ugaLocalUWdemandArray[UGA_Metro];
      m_maxBarneyAllocationToday = 0.05f*m_ugaLocalUWdemandArray[UGA_Metro];
      }
   
   /*
   if (pFlowContext->dayOfYear == 0)
      {
      for (int uga = 0; uga <= MAX_UGA_NDX; uga++)
         {
         CString msg;
         msg.Format("*** AltWaterMaster::Step() uga = %d, m_ugaLocalUWdemandArray[uga] = %f", uga, m_ugaLocalUWdemandArray[uga]);
         Report::LogMsg(msg);
         } // end of loop on UGAs
      } // end of if day of year == 0 
*/
	// Begin looping through POD input data file ===================================================================================
   int currentCalendarYear = pFlowContext->pEnvContext->currentYear;
	for (int i = 0; i < (int)m_podArray.GetSize(); i++)
		{
		vector<int> *pouIDUs = 0;

		// Get current WR
      WaterRight *pRight = m_podArray[i];
      if (pRight->m_podStatus == WRPS_CANCELED) continue;
      if ((pRight->m_priorYr > currentCalendarYear) || ((pRight->m_priorYr == currentCalendarYear) && (pRight->m_priorDoy > pFlowContext->dayOfYear))) break;

      pRight->m_stepRequest = 0.f;

		// initiate water right variables at first of year
		if (pFlowContext->dayOfYear == 0)
			{
			pRight->m_nDaysPerYearSO = 0;
         pRight->m_allocatedYesterday_cms = pRight->m_allocatedToday_cms = 0.f;
			}
      else
         {
         pRight->m_allocatedYesterday_cms = pRight->m_allocatedToday_cms;
         pRight->m_allocatedToday_cms = 0.f;
         }

      if (pRight->m_suspendForYear) continue; // Skip if the water right has been shut off by regulatory action.

		// Get POUID (aka Place of Use) for current Permit
		int pouID = pRight->m_pouID;
      if (pRight->m_useCode == WRU_MUNICIPAL && pouID < 0)
         { // Interpret the pouID as -UGB
         int uga = -pouID;
         if (uga > MAX_UGA_NDX) continue;
/*
         if (pFlowContext->dayOfYear == 0)
            {
            CString msg;
            msg.Format("*** AltWaterMaster::Step() WR PODindex = %d is associated with UGA %d ", i, uga);
            Report::LogMsg(msg);
            }
*/
         // Satisfy as much of the UGA's remaining municipal water demand as possible from this WR.
         float maxPODrate = pRight->m_podRate_cfs / (FT3_PER_M3); // Convert PODRATE in POD file from cfs to maxPODrate in m3/s
         float request = min(m_ugaLocalUWdemandArray[uga], maxPODrate); // m3/sec H2O
         if (request <= 0.f) continue;
         float allocated_water = 0.f; // m3 H2O
         switch (pRight->m_permitCode)
            {
            case WRP_SURFACE:
               AllocateSWtoUGA(pFlowContext, pRight, pStreamLayer, uga, request, &allocated_water);
/*             if (uga == UGA_Metro)
                  {
                  CString msg; msg.Format("*** Metro UW request = %f, allocated_water = %f, pRight->m_reachComid = %d, pRight->m_wrID = %d",
                     request, allocated_water, pRight->m_reachComid, pRight->m_wrID);
                  Report::LogMsg(msg);
                  }
*/
               break;
            case WRP_GROUNDWATER:
               AllocateGWtoUGA(pRight, uga, request, &allocated_water);
               break;
            default: break;
            } // end of switch on permitCode

         if (pFlowContext->dayOfYear == 0)
            {
            CString msg;
            msg.Format("*** AltWaterMaster::Step() uga = %d, request = %f, allocated_water = %f ", uga, request, allocated_water);
            Report::LogMsg(msg);
            }

         m_ugaLocalUWdemandArray[uga] -= allocated_water;
         pRight->m_allocatedToday_cms = allocated_water;

         continue; // We're done with this point of diversion for this water right.
         }
      else if (pRight->m_useCode == WRU_INSTREAM)
         { 
         int jday_1 = pFlowContext->dayOfYear + 1; // Calculate 1-based Julian day for comparison to water right data 
         if (jday_1 < pRight->m_beginDoy || pRight->m_endDoy < jday_1) continue;
         float current_rate_m3_per_s = pRight->m_podRate_cfs / FT3_PER_M3;
         Reach *pReach = pRight->m_pReach;
         if (pReach == NULL)
            {
            CString msg; msg.Format("*** AltWaterMaster::Step() instream water right with no associated reach at PODarray index = %d, "
               "pRight->m_wrID = %d", i, pRight->m_wrID);
            if (jday_1 == 1) Report::WarningMsg(msg);
            continue; // We're done with this point of diversion for this water right.
            }

         // Apply this WR to the current reach and to additional downstream reaches per the length or downstream COMID fields.
         float remaining_length = pRight->m_instreamWRlength_m > 0.f ? pRight->m_instreamWRlength_m : pReach->m_length;
         int recursive_depth = m_recursiveDepth;
         int streamNdx = pRight->m_streamIndex;
         float max_deficit_cms = 0.f;
         while (pReach != NULL)
            {
            remaining_length -= pReach->m_length;

            m_minFlow = DEFAULT_REACH_MINFLOW;

            float availSourceFlow = (float)(pReach->m_availableDischarge); // m3/sec
            if (availSourceFlow < 0.0f) availSourceFlow = 0.f;

            float current_rate_for_this_reach_cms = pReach->m_instreamWaterRightUse > 0.f ? 
               current_rate_m3_per_s - pReach->m_instreamWaterRightUse : current_rate_m3_per_s;
            if (current_rate_for_this_reach_cms < 0.f) current_rate_for_this_reach_cms = 0.f;

            float deficit = max(0.f, current_rate_for_this_reach_cms - availSourceFlow);
            max_deficit_cms = max(max_deficit_cms, deficit);
            float allocation_m3_per_s = deficit > 0.f ? availSourceFlow : current_rate_for_this_reach_cms;
            /*
            if ((pReach->m_instreamWaterRightUse > 0.f) && (pFlowContext->pEnvContext->yearOfRun == 0))
               {
               CString msg;
               msg.Format("*** AltWaterMaster::Step() for pRight->m_podID = %d, more than one instream water right is associated with reach %d",
               pRight->m_podID, pReach->m_reachID);
               Report::WarningMsg(msg);
               }
            //*/
            double wrq = pReach->m_instreamWaterRightUse + current_rate_for_this_reach_cms;
            double q_div_wrq = wrq > 0.f ? (pReach->GetDischarge() / wrq) : 100.f;
            pReach->m_instreamWaterRightUse += allocation_m3_per_s;
            /*
            if (jday_1 == 1)
               {
               CString msg;
               msg.Format("*** AltWaterMaster::Step() for pRight->m_podID = %d, reach %d, pReach->m_availableDischarge = %f, current_rate_m3_per_s = %f, q_div_wrq = %f",
                  pRight->m_podID, pReach->m_reachID, pReach->m_availableDischarge, current_rate_m3_per_s, q_div_wrq);
               Report::LogMsg(msg);
               }
            */
            if (streamNdx < 0)
               {
               CString msg; msg.Format("*** streamNdx < 0 streamNdx = %d", streamNdx); Report::LogMsg(msg);
               }
            else
               {
               pStreamLayer->m_readOnly = false;
               pStreamLayer->SetData(streamNdx, m_colStreamQ_DIV_WRQ, q_div_wrq);
               pStreamLayer->SetData(streamNdx, m_colStreamINSTRM_REQ, current_rate_m3_per_s);
               pStreamLayer->m_readOnly = true;
               }

            if (deficit > 0.f)
               {
               int numPODsShutOff = RegulatoryShutOff(pFlowContext, pRight, recursive_depth, deficit, pReach);
               if (numPODsShutOff > 0)
                  {
                  CString msg;
                  msg.Format("*** AltWaterMaster::Step() on day %d shut off water right(s) for %d POD(s) as a result of insufficient flow for senior instream WR %d (%d day %d COMID %d) %s", 
                     pFlowContext->dayOfYear, numPODsShutOff, pRight->m_wrID, pRight->m_priorYr, pRight->m_priorDoy, pRight->m_reachComid,
                     pRight->m_useCode == WRU_IRRIGATION ? "irrigation" : (pRight->m_useCode == WRU_INSTREAM ? "instream" : 
                     (pRight->m_useCode == WRU_MUNICIPAL ? "municipal" : "neither irrigation, instream, nor municipal")));
                  Report::LogMsg(msg);
                  }
               recursive_depth = 0; // On any subsequent downstream reaches, look only for junior water rights in the current reach.
               }

            if (pReach->m_pDown == NULL || pRight->m_downstreamComid == pReach->m_reachID || (pRight->m_downstreamComid == -99 && remaining_length <= 0.f)) 
               { // Stop moving downstream.
               remaining_length = 0.f;
               pReach = NULL;
               }
            else
               {
               pReach = m_pFlowModel->GetReachFromNode(pReach->m_pDown); // Move to the next downstream reach.
               streamNdx = pReach->m_polyIndex;
               } // end of if (pReach->m_pDown == NULL || ...) ... else 
            } // end of while (pReach != NULL)
         
         pRight->m_allocatedToday_cms = max(current_rate_m3_per_s - max_deficit_cms, 0.f);

         continue; // We're done with this point of diversion for this water right.
         }

      // Interpret the pouID as a collection of IDUs

		// Build lookup key into point of use (POU) map/lookup table
		m_pouLookupKey.pouID = pouID;

		// Returns vector of indices into the POU input data file. Used for relating to current water right POU to polygons in IDU layer
		pouIDUs = &m_pouInputMap[m_pouLookupKey];

		// if a PODID does not have a POUID, consider next WR
		if (pouIDUs->size() == 0) continue;
			
		// Diversion is a go. Begin looping through IDUs Associated with WRID and PLace of Use (POU)
      // float remaining_PODrate_cms = pRight->m_podRate_cfs * FT3_PER_M3;
      // for (int j = 0; (remaining_PODrate_cms > 0.f) && (j < pouIDUs->size()); j++)
      for (int j = 0; j < pouIDUs->size(); j++)
         {
         int pou_row = pouIDUs->at(j);
         if (pou_row < 0 || pou_row >= m_pouDb.GetRowCount()) continue;

         float percent_idu = m_pouDb.GetAsFloat(m_colPouPERCENT_IDU, pou_row);
         if (percent_idu < m_pctIDUPOUIntersection) continue;

         // gets an IDU associated with this water right
         int tempIDU_ID = m_pouDb.GetAsInt(m_colPouIDU_ID, pou_row);

			//build key into idu index map
			m_iduIndexLookupKey.idu_id = tempIDU_ID;

			//returns vector with the idu index for idu layer
			iduNdxVec = &m_IDUIndexMap[m_iduIndexLookupKey];

			// If no index is return, then the idu that the POU is associated with is not in current layer.
			if (iduNdxVec->size() == 0) continue;

			int iduNdx = iduNdxVec->at(0);
         int wrExists = 0; pLayer->GetData(iduNdx, m_colWREXISTS, wrExists);
         if (wrExists == 0) continue;

         int uga = -1; pLayer->GetData(iduNdx, m_colUGB, uga);

		 pLayer->GetData(iduNdx, m_colIrrigate, m_irrigateDecision);
         m_wrIrrigateFlag = m_irrigateDecision != 0;

			// Get the areal percent of the POU for current WRid and IDU_INDEX
			float percent_pou = m_pouDb.GetAsFloat(m_colPouPERCENT_POU, pouIDUs->at(j));

			float fracPou = percent_pou/100.f;

			// Get the area of the POU for current WRid and IDU_INDEX

			float iduAreaM2 = 0.0f;

			pLayer->GetData( iduNdx, m_colAREA, iduAreaM2 );
		
			unsigned __int16 iduUse = GetUse( wrExists ); // get the use code from the WREXISTS code
				
         if (pFlowContext->dayOfYear == 0 && IsIrrigation(iduUse) && pRight->m_useCode == WRU_IRRIGATION && pRight->m_permitCode==WRP_SURFACE)
				{					
               m_SWirrigWRpriority++;
               pLayer->SetData(iduNdx, m_colWRPRIORITY, m_SWirrigWRpriority);
				} // end of if (pFlowContext->dayOfYear == 0 && IsIrrigation(iduUse) && pRight->m_useCode == WRU_IRRIGATION )

         if ((pFlowContext->dayOfYear + 1 >= pRight->m_beginDoy) && (pFlowContext->dayOfYear + 1 <= pRight->m_endDoy))
				{
            switch (pRight->m_permitCode)
					{

					case WRP_SURFACE:
                  if (pRight->m_useCode == WRU_MUNICIPAL && 1 <= uga && uga <= MAX_UGA_NDX) break; // muni water in UGAs handled elsewhere
                  if (pRight->m_useCode != WRU_IRRIGATION || (m_wrIrrigateFlag && m_iduLocalIrrRequestArray[iduNdx] > 0.f))
                     AllocateSurfaceWR(pFlowContext, pRight, iduNdx, fracPou);
						break;

					case WRP_GROUNDWATER:
                  if (pRight->m_useCode == WRU_MUNICIPAL && 1 <= uga && uga <= MAX_UGA_NDX) break; // muni water in UGAs handled elsewhere
                  AllocateWellWR(pFlowContext, pRight, iduNdx, fracPou);
						break;

					default: // ignore everything else
						break;
					}
				} // end water right in season

         // remaining_PODrate_cms = max((pRight->m_podRate_cfs * FT3_PER_M3) - pRight->m_allocatedToday_cms, 0.f);
			} // end for idus per pou

		} // end for POD prior appropriation list

   // Tally up unsatisfied irrigation demand and distribute UGA urban water use to the IDUs
   bool readOnlyFlag = pLayer->m_readOnly;
   pLayer->m_readOnly = false;
   for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
      {
      float iduPop = 0; pLayer->GetData(idu, m_colPOP, iduPop);
      int wr_muni = 0; pLayer->GetData(idu, m_colWR_MUNI, wr_muni);
      int uga = 0; pLayer->GetData(idu, m_colUGB, uga);
      float iduUWdemand = 0.f; pLayer->GetData(idu, m_colUDMAND_DY, iduUWdemand); // m3/sec
      if (iduPop > 0.f)
         {
         if (0 < uga && uga <= MAX_UGA_NDX)
            {
            double popFrac = iduPop / m_ugaPop[uga];
            m_iduSWMuniArrayDy[idu] = (float)(popFrac * m_ugaUWallocatedDayFromSW[uga]); // m3/sec
            m_iduSWMuniArrayYr[idu] += m_iduSWMuniArrayDy[idu] * SEC_PER_DAY; // m3 H2O
            m_iduGWMuniArrayDy[idu] = (float)(popFrac * m_ugaUWallocatedDayFromGW[uga]); // m3/sec
            m_iduGWMuniArrayYr[idu] += m_iduGWMuniArrayDy[idu] * SEC_PER_DAY; // m3 H2O
            }
         else if (wr_muni == 0 && iduUWdemand > 0.f)
            { // This IDU is outside all the UGAs, has people, doesn't have a municipal water right, but does have positive urban water demand, so assume it has a well.
            m_iduGWMuniArrayDy[idu] += iduUWdemand; // m3/sec 
            m_iduGWMuniArrayYr[idu] += iduUWdemand * SEC_PER_DAY; // m3 H2O
            m_GWMuniWater += iduUWdemand; // m3/sec 
            m_GWnoWR_Yr_m3 += iduUWdemand * SEC_PER_DAY; // m3 H2O
            m_pWaterAllocation->m_iduYrMuniArray[idu] += iduUWdemand; //m3/sec;
            FateOfIDU_RuralResidentialWater((int)idu, iduUWdemand);
            }
         } // end of if (iduPop>0)
      else m_iduSWMuniArrayDy[idu] = m_iduGWMuniArrayDy[idu] = 0.f;

      pLayer->SetData(idu, m_colSWMUNALL_D, m_iduSWMuniArrayDy[idu]);
      pLayer->SetData(idu, m_colGWMUNALL_D, m_iduGWMuniArrayDy[idu]);

      float unsatisfied_request_mm = 0.f;

      int irr_decision = 0; pLayer->GetData(idu, m_colIrrigate, irr_decision);
      if (irr_decision == 1) 
         { // Irrigation is desired today.
         int wr_shutoff = 0; pLayer->GetData(idu, m_colWRShutOff, wr_shutoff);
         bool shortageFlag = false;
         unsatisfied_request_mm = m_iduLocalIrrRequestArray[idu];
         if (unsatisfied_request_mm > 0.01f)
         {
            // Don't count it as a shortage if the IDU's water right has been shut off by regulatory action.
            shortageFlag = wr_shutoff == 0;
            if (shortageFlag)
            {
               float idu_area_m2 = 0.f; pLayer->GetData(idu, m_colAREA, idu_area_m2);
               float unsatisfied_request_m3 = (unsatisfied_request_mm / 1000.f) * idu_area_m2;
               float unsatisfied_request_acft = unsatisfied_request_m3 / M3_PER_ACREFT;
               m_iduUnsatIrrReqst_Yr[idu] += unsatisfied_request_acft;
               m_unSatisfiedIrrigationYr += unsatisfied_request_acft;
            } // end of if (shortageFlag)

            // Decrement SOILH2OEST by the amount of the request which went unfulfilled.
            float soilwater_est_orig = 0.f; pLayer->GetData(idu, m_colSOILH2OEST, soilwater_est_orig); // mm
            float soilwater_est_new = soilwater_est_orig - (unsatisfied_request_mm * 0.80f /*(1.f - m_irrigLossFactor) */);
            float wp = 10.f; // wilting point = 10 mm
            if (soilwater_est_new < wp) soilwater_est_new = wp;
            pLayer->SetDataU(idu, m_colSOILH2OEST, soilwater_est_new);

            int irr_state_int = -1; pLayer->GetData(idu, m_colIRR_STATE, irr_state_int);
            if (((IrrigationState)irr_state_int) == IRR_FULL) pLayer->SetData(idu, m_colIRR_STATE, IRR_PARTIAL);

         } // end of if (unsatisfied_request_mm > 0.01f)

         LogWeeklySurfaceWaterIrrigationShortagesByIDU( idu, shortageFlag);

         } // end of block for IDUs for which irrigation was desired

      pLayer->SetDataU(idu, m_colUnSatIrrRequest, unsatisfied_request_mm);

      } // end of loop through IDUs to tally up unsatisfied irrigation demand and distribute UGA urban water use to the IDUs
   pLayer->m_readOnly = readOnlyFlag;

   for (int uga = 1; uga <= MAX_UGA_NDX; uga++) m_ugaUWshortageYr[uga] += m_ugaLocalUWdemandArray[uga] * SEC_PER_DAY;

   FateOfUGA_UrbanWaterUsingSewers(); // Dispose of the urban water.

   //all water rights have been exercised for relevant IDUs, now aggregate use per IDU to HRU Layer
   AggregateIDU2HRU(pFlowContext);

   return true;
	} // end of Step()


void AltWaterMaster::FateOfIDU_RuralResidentialWater(int iduNdx, float iduUWdemand_cms)
// The portion of rural residential demand which is used indoors is assumed to go back to the shallow subsoil in septic fields.
// The portion of rural residential demand which is used outdoors is also applied to the shallow subsoil.
// Then the outdoor ET fraction is sent from the shallow subsoil to the atmosphere.
   {
   ASSERT(iduUWdemand_cms > 0.);

   int doy = m_pFlowContext->dayOfYear; // Jan 1 = 0
   float indoorFrac = DailyUrbanWaterDemand::UWindoorFraction(doy);
   float indoor_use_cms = iduUWdemand_cms * indoorFrac;
   float outdoor_use_cms = iduUWdemand_cms - indoor_use_cms;
   float et_cms = UW_OUTDOOR_ET_FRAC * outdoor_use_cms;

   m_pWaterAllocation->m_iduNonIrrAllocationArray[iduNdx] += iduUWdemand_cms; // Add both indoor use and outdoor use to shallow subsoil.

   // Move ET from shallow subsoil to atmosphere.
   int hru_ndx = -1; m_pIDUlayer->GetData(iduNdx, m_colHRU_NDX, hru_ndx);
   if (hru_ndx >= 0)
      {
      HRU * pHRU = m_pFlowModel->GetHRU(hru_ndx);
      if (pHRU != NULL) 
         {
         HRULayer * pHRU_ShallowSubsoilLayer = pHRU->GetLayer(BOX_FAST_GW);
         float et_m3 = et_cms * SEC_PER_DAY;
         pHRU_ShallowSubsoilLayer->CheckForNaNs("FateOfIDU_RuralResidentialWater", pHRU_ShallowSubsoilLayer->AddFluxFromGlobalHandler(et_m3, FL_TOP_SINK));
         float idu_area_m2 = -1.f; m_pIDUlayer->GetData(iduNdx, m_colAREA, idu_area_m2);
         float et_mm = (et_m3 / idu_area_m2) * 1000.f;
         float et_day = -1.; m_pIDUlayer->GetData(iduNdx, m_colET_DAY, et_day);
         et_day += et_mm;
         m_pIDUlayer->SetDataU(iduNdx, m_colET_DAY, et_day);
         if (pHRU->m_HRUeffArea_m2 > 0) pHRU->m_et_yr += et_mm * (idu_area_m2 / pHRU->m_HRUeffArea_m2);
         } // end of if (pHRU != NULL)
      } // end of if (hru_id >= 0)

   } // end of FateOfIDU_RuralResidentialWater()


float AltWaterMaster::AllocateIrrigationWater(WaterRight *pRight, int iduNdx, float pctPou, float availSourceFlow_cms, float * pUnusedAmt_cms)
// Returns amount allocated in cms.  Places the amount available, over and above the amount allocated, in *pUnusedAmt_cms.
{
   float amount_allocated_cms = 0.f;

   int wr_exists = 0;  m_pIDUlayer->GetData(iduNdx, m_colWREXISTS, wr_exists);
   unsigned __int16 iduUse = GetUse(wr_exists); // get the use code from the WREXISTS code
   if (!IsIrrigation(iduUse))
   {
      *pUnusedAmt_cms = 0.f;
      return(0.f);
   }

   float idu_irr_rate_factor = 1.f; m_pIDUlayer->GetData(iduNdx, m_colXIRR_RATE, idu_irr_rate_factor);
   float adjusted_PODrate_cms = idu_irr_rate_factor * pRight->m_podRate_cfs / FT3_PER_M3;
   float remaining_PODrate_cms = max(adjusted_PODrate_cms - pRight->m_allocatedToday_cms, 0.f);
   float avail_PODrate_for_idu_cms = remaining_PODrate_cms * pctPou;
   float irrRequest_mm = m_iduLocalIrrRequestArray[iduNdx]; //mm/d
   if (!m_wrIrrigateFlag || irrRequest_mm <= 0.0)
   {
      *pUnusedAmt_cms = avail_PODrate_for_idu_cms;
      return(0);
   }

   float idu_area_m2 = 0.f; m_pIDUlayer->GetData(iduNdx, m_colAREA, idu_area_m2);
   float idu_area_ac = idu_area_m2 / M2_PER_ACRE;
   float iduDemand_cms = irrRequest_mm / 1000.0f * idu_area_m2 / SEC_PER_DAY;
   pRight->m_stepRequest += iduDemand_cms;

   if (irrRequest_mm < 0.0) irrRequest_mm = 0.0;
   float actualIrrRequest = m_iduActualIrrRequestDy[iduNdx];
   // just do once per step at the first encounter with this idu
   if (actualIrrRequest == 0.0) m_iduActualIrrRequestDy[iduNdx] = irrRequest_mm;

   float totIrrMMYr = m_iduSWIrrArrayYr[iduNdx] + m_iduGWIrrArrayYr[iduNdx]; // mm
   float idu_duty_ytd_ft = (totIrrMMYr / 1000.f) * FT_PER_M;
   //if (totIrrMMYr > 0.0f) totIrrAcreFt = totIrrMMYr / 1000.0f * iduAreaM2 / M3_PER_ACREFT; // acre-feet
   //if (totIrrAcreFt > 0.0f) iduDuty = totIrrAcreFt / iduAreaAc; // acre-feet per acre

   float irr_duty_factor = 0.f; m_pIDUlayer->GetData(iduNdx, m_colXIRR_DUTY, irr_duty_factor);
   if (idu_duty_ytd_ft >= (m_maxDuty_ft_per_yr*irr_duty_factor))
   { // This water right has already been used up this year for this IDU.
      if (m_iduExceedDutyLog[iduNdx] != 1)
      {
         m_iduExceedDutyLog[iduNdx] = 1;
         /*
         CString msg; msg.Format("***AllocateIrrigationWater(): iduDuty >= m_maxDuty_ft_per_yr.  iduNdx = %d, iduDuty = %f, LULC_B = %d",
         iduNdx, iduDuty, agClass);
         Report::LogMsg(msg);
         */
         m_areaDutyExceeds += idu_area_ac;
      }
      *pUnusedAmt_cms = 0.f;
      return(0);
   } // end of block for when the water right has already been used up

   amount_allocated_cms = min(min(availSourceFlow_cms, avail_PODrate_for_idu_cms), iduDemand_cms);
   ASSERT(amount_allocated_cms >= 0.f);
   pRight->m_allocatedToday_cms += amount_allocated_cms;
   m_pWaterAllocation->m_iduIrrAllocationArray[iduNdx] += amount_allocated_cms;

   float amount_allocated_mm = (amount_allocated_cms * SEC_PER_DAY / idu_area_m2) * 1000.f;
   m_IrrFromAllocationArrayDy += amount_allocated_mm;
   m_iduLocalIrrRequestArray[iduNdx] -= amount_allocated_mm;

   float amount_allocated_af = amount_allocated_cms * SEC_PER_DAY / M3_PER_ACREFT;
   m_irrigatedWaterYr += amount_allocated_af; // acre-feet

   if (availSourceFlow_cms < iduDemand_cms && availSourceFlow_cms < avail_PODrate_for_idu_cms)
   {
      float deficit = avail_PODrate_for_idu_cms - availSourceFlow_cms;
      m_pReachLayer->SetDataU(pRight->m_pReach->m_polyIndex, m_colWRConflictYr, 1);
      m_pReachLayer->SetDataU(pRight->m_pReach->m_polyIndex, m_colWRConflictDy, 1);

      int numPODsShutOff = ApplyRegulation(m_pFlowContext, m_regulationType, pRight, m_recursiveDepth, deficit);
      if (numPODsShutOff > 0)
      {
         CString msg;
         msg.Format("*** AltWaterMaster::AllocateSurfaceWR() on day %d shut off water right(s) for %d POD(s) as a result of insufficient flow for senior irrigation WR %d (%d day %d COMID %d) %s",
            m_pFlowContext->dayOfYear, numPODsShutOff, pRight->m_wrID, pRight->m_priorYr, pRight->m_priorDoy, pRight->m_reachComid,
            pRight->m_useCode == WRU_IRRIGATION ? "irrigation" : (pRight->m_useCode == WRU_INSTREAM ? "instream" :
            (pRight->m_useCode == WRU_MUNICIPAL ? "municipal" : "neither irrigation, instream, nor municipal")));
         Report::LogMsg(msg);
      }
   }

   // The "unused amount" here is the amount, of what is actually present and allocable in the stream, that this IDU is entitled to use,
   // but is not actually using on this day.
   if (avail_PODrate_for_idu_cms > iduDemand_cms && availSourceFlow_cms > iduDemand_cms)
      *pUnusedAmt_cms = min(avail_PODrate_for_idu_cms, availSourceFlow_cms) - iduDemand_cms;
   return(amount_allocated_cms);
} // end of AllocateIrrigationWater()


   bool AltWaterMaster::AllocateSWtoUGA(FlowContext * pFlowContext, WaterRight *pRight, MapLayer *pStreamLayer, int uga, float request, float *pAllocated_water) // quantities in m3/sec H2O
   {
   *pAllocated_water = 0.f;
   Reach *pReach = pRight->m_pReach;
   if (pRight->m_reachComid < 0)
      { // Source is outside the basin, e.g. Bull Run reservoir.
      *pAllocated_water = request;
      if (uga == UGA_Metro)
         {
         switch (pRight->m_wrID)
            {
            case BULL_RUN_WR:
               if (*pAllocated_water > m_maxBullRunAllocationToday) *pAllocated_water = m_maxBullRunAllocationToday;
               m_fromBullRunDy_m3_per_s = *pAllocated_water;
               break;
            case BARNEY_WR: 
			   if (*pAllocated_water > m_maxBarneyAllocationToday) *pAllocated_water = m_maxBarneyAllocationToday;
			   m_fromBarneyDy_m3_per_s = *pAllocated_water; 
			   break;
			default: break;
            }
         }
      m_fromOutsideBasinDy[uga] += *pAllocated_water;
      }
   else if (pReach != NULL)
      {
      float availSourceFlow;
      m_minFlow = DEFAULT_REACH_MINFLOW;

      availSourceFlow = (float)(pReach->m_availableDischarge - pReach->m_instreamWaterRightUse); // m3/sec
      if (availSourceFlow <= 0.0) return(false);

      float podRate = pRight->m_podRate_cfs / FT3_PER_M3;
      float request_from_this_WR = min(request, podRate);

      *pAllocated_water = min(request_from_this_WR, availSourceFlow);
      pReach->CheckForNaNs("AllocateSWtoUGA", pReach->AddFluxFromGlobalHandler(-(*pAllocated_water * SEC_PER_DAY))); //m3/d
      pReach->m_availableDischarge -= *pAllocated_water;

      float out_muni_cms; m_pReachLayer->GetData(pReach->m_polyIndex, m_colStreamOUT_MUNI, out_muni_cms);
      out_muni_cms += *pAllocated_water;
      m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colStreamOUT_MUNI, out_muni_cms);

      if (uga == UGA_Metro)
      {
         switch (pRight->m_wrID)
         {
         case SPRING_HILL_WR: 
			 if (*pAllocated_water > m_maxSpringHillAllocationToday) *pAllocated_water = m_maxSpringHillAllocationToday;
			 m_fromSpringHillDy_m3_per_s = *pAllocated_water;
			 break;
         case CLACKAMAS_WR: 
			 if (*pAllocated_water > m_maxClackamasAllocationToday) *pAllocated_water = m_maxClackamasAllocationToday;
			 m_fromClackamasDy_m3_per_s = *pAllocated_water;
			 break;
		 case LAKE_OSWEGO_WR:
			 if (*pAllocated_water > m_maxLakeOswegoAllocationToday) *pAllocated_water = m_maxLakeOswegoAllocationToday;
			 m_fromLakeOswegoDy_m3_per_s = *pAllocated_water;
			 break;
		 default: break;
         }
      }

      if (request_from_this_WR < request && request_from_this_WR < podRate)
         { // There is a shortage of water for exercising this water right.
         float deficit = podRate - request_from_this_WR;
         int numPODsShutOff = ApplyRegulation(pFlowContext, m_regulationType, pRight, m_recursiveDepth, deficit);
         if (numPODsShutOff > 0)
            {
            CString msg;
            msg.Format("*** AltWaterMaster::AllocateSWtoUGA() on day %d shut off water right(s) for %d POD(s) as a result of insufficient flow for senior muni WR %d (%d day %d COMID %d) %s",
               pFlowContext->dayOfYear, numPODsShutOff, pRight->m_wrID, pRight->m_priorYr, pRight->m_priorDoy, pRight->m_reachComid,
               pRight->m_useCode == WRU_IRRIGATION ? "irrigation" : (pRight->m_useCode == WRU_INSTREAM ? "instream" :
               (pRight->m_useCode == WRU_MUNICIPAL ? "municipal" : "neither irrigation, instream, nor municipal")));
            Report::LogMsg(msg);
            }
         }
      }
   else return(false);

   m_ugaUWallocatedDayFromSW[uga] += *pAllocated_water;
   m_SWMuniWater += *pAllocated_water; // m3/sec 
   m_ugaUWfromSW[uga] += *pAllocated_water * SEC_PER_DAY; // m3 H2O
   if (pRight->m_specialCode == WRSC_MUNIBACKUP) m_ugaMuniBackupWRuse_acft[uga] += (*pAllocated_water * SEC_PER_DAY) / M3_PER_ACREFT;
   return(true);
   } // end of AllocateSWtoUGA()


bool AltWaterMaster::AllocateGWtoUGA(WaterRight *pRight, int uga, float request, float *pAllocated_water) // quantities in m3/sec H2O
   {
   *pAllocated_water = request;

   m_ugaUWallocatedDayFromGW[uga] += *pAllocated_water;
   m_GWMuniWater += *pAllocated_water; // m3/sec 
   m_ugaUWfromGW[uga] += *pAllocated_water * SEC_PER_DAY; // m3 H2O
   return(true);
   } // end of AllocateGWtoUGA()


void AltWaterMaster::FateOfUGA_UrbanWaterUsingSewers()
{
   float indoor_frac = DailyUrbanWaterDemand::UWindoorFraction(m_pFlowContext->dayOfYear); 
   double frac2pocd[MAX_UGA_NDX + 1]; 
   double uga_allocated_cms[MAX_UGA_NDX + 1];

   for (int uga = 0; uga <= MAX_UGA_NDX; uga++) 
   {
      frac2pocd[uga] = 0.;
      m_ugaUWindoorDay_m3_per_sec[uga] = 0.f;
      m_ugaUWoutdoorDay_m3_per_sec[uga] = 0.f;

      uga_allocated_cms[uga] = m_ugaUWallocatedDayFromSW[uga] + m_ugaUWallocatedDayFromGW[uga];
      m_ugaUWallocatedYr[uga] += (float)(uga_allocated_cms[uga] * SEC_PER_DAY);
   } // end of loop thru UGAs

   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      float idu_pop = 0; m_pIDUlayer->GetData(idu, m_colPOP, idu_pop);
      if (idu_pop <= 0.f) continue;

      int uga = 0; m_pIDUlayer->GetData(idu, m_colUGB, uga);
      if (uga < 1 || uga > MAX_UGA_NDX) continue;
      ASSERT(m_ugaPop[uga] > 0); if (m_ugaPop[uga] <= 0) continue;

      double idu_pop_frac = idu_pop / m_ugaPop[uga];
      int sewer_ndx; m_pIDUlayer->GetData(idu, m_colSEWER_NDX, sewer_ndx);
      if (sewer_ndx < 0) frac2pocd[uga] += idu_pop_frac * indoor_frac;
      else
      { // Send the indoor use urban water for this IDU to the sewer.
         double idu_indoor_use_cms = uga_allocated_cms[uga] * indoor_frac *  idu_pop_frac;
         double idu_indoor_use_cfs = idu_indoor_use_cms * FT3_PER_M3;
         if (m_pNodeLayer != NULL) m_pNodeLayer->SetDataU(sewer_ndx, m_colNodeDWF_D, idu_indoor_use_cfs);
         m_ugaUWindoorDay_m3_per_sec[uga] += (float)idu_indoor_use_cms;
      } 

      double idu_outdoor_use_cms = (uga_allocated_cms[uga] - indoor_frac * uga_allocated_cms[uga]) * idu_pop_frac;
      m_ugaUWoutdoorDay_m3_per_sec[uga] += (float)idu_outdoor_use_cms;

      // First send the whole outdoor amount to the soil; then send part of it from the soil to the atmosphere.
      m_pWaterAllocation->m_iduNonIrrAllocationArray[idu] += (float)idu_outdoor_use_cms; // This sends it all to the shallow subsoil.
      float idu_area_m2 = -1.f; m_pIDUlayer->GetData(idu, m_colAREA, idu_area_m2);
      float idu_outdoor_use_mm = (float)((idu_outdoor_use_cms * SEC_PER_DAY / idu_area_m2) * 1000.f);
      m_pIDUlayer->SetData(idu, m_colUWOUTDOORD, idu_outdoor_use_mm);

      const float UW_outdoor_ET_frac = 0.8f; // fraction of UW used outside which goes to evapotranspiration
      double idu_UWtoET_cms = UW_outdoor_ET_frac * idu_outdoor_use_cms;
      double idu_UWtoET_m3_per_day = idu_outdoor_use_cms * SEC_PER_DAY;

      // Add the ET part to the idu's ET accumulation.
      double idu_UWtoET_mm = (idu_UWtoET_m3_per_day / idu_area_m2) * 1000.f;
      float idu_aet_mm = 0.f; m_pIDUlayer->GetData(idu, m_colET_DAY, idu_aet_mm);
      idu_aet_mm += (float)idu_UWtoET_mm;
      m_pIDUlayer->SetDataU(idu, m_colET_DAY, idu_aet_mm);  // mm/day

      int hru_ndx = -1; m_pIDUlayer->GetData(idu, m_colHRU_NDX, hru_ndx);
      if (hru_ndx >= 0)
         { 
         HRU * pHRU = m_pFlowModel->GetHRU(hru_ndx);
         HRULayer *pMunHRULayer = pHRU->GetLayer(BOX_FAST_GW);
         pMunHRULayer->CheckForNaNs("FateOfUGA_UrbanWaterUsingSewers 1", pMunHRULayer->AddFluxFromGlobalHandler((float)idu_UWtoET_m3_per_day, FL_TOP_SINK)); // This sends the ET part from the soil to the atmosphere.
         if (pHRU->m_HRUeffArea_m2 > 0) pHRU->m_et_yr += (float)(idu_UWtoET_mm * (idu_area_m2 / pHRU->m_HRUeffArea_m2));
         }
   } // end of loop thru IDUs to divide up the water that doesn't go back to a POCD

   // Now send the rest to the POCDs
   for (int uga = 1; uga <= MAX_UGA_NDX; uga++) 
   {
      double discharge2stream_cms = uga_allocated_cms[uga] * frac2pocd[uga];
      if (discharge2stream_cms <= 0.) continue;
      m_ugaUWindoorDay_m3_per_sec[uga] += (float)discharge2stream_cms;

      double total_pct = 0.f;
      for (int pocd_ndx = 0; total_pct < 100.f && pocd_ndx < m_POCDs.GetCount(); pocd_ndx++)
      {
         if (m_POCDs[pocd_ndx]->uga != uga) continue;

         double discharge2POCD_cms = (m_POCDs[pocd_ndx]->ugaDischargePct / 100.f)*discharge2stream_cms;
         double discharge2POCD_m3_today = discharge2POCD_cms * SEC_PER_DAY;
         m_POCDs[pocd_ndx]->pocdOutfallAmtYr_m3 += (float)discharge2POCD_m3_today;

         total_pct += m_POCDs[pocd_ndx]->ugaDischargePct;
         /*
         if (pFlowContext->dayOfYear == 0)
         {
         CString msg;
         msg.Format("*** FateOfUGA_UrbanWaterUsingSewers(): dayOfYear = %d, uga = %d, total_pct = %f, pocd_ndx = %d, pocdOutfallAmtYr_m3 = %f",
         pFlowContext->dayOfYear, uga, total_pct, pocd_ndx, m_POCDs[pocd_ndx]->pocdOutfallAmtYr_m3);
         Report::LogMsg(msg);
         }
         */

         // If the outfall is in the study area, put the water back in the stream.
         if (!m_POCDs[pocd_ndx]->outsideBasin_flag && m_POCDs[pocd_ndx]->reachPolyIndex >= 0) 
         {
            Reach *pReach = m_pFlowModel->FindReachFromPolyIndex(m_POCDs[pocd_ndx]->reachPolyIndex);
            pReach->CheckForNaNs("FateOfUGA_UrbanWaterUsingSewers 2", pReach->AddFluxFromGlobalHandler((float)discharge2POCD_m3_today));
         }
         else
         { // Even if the outsideBasin_flag is not set, the streamIndex could be missing if we're simulating a subbasin instead of the whole study area.
            m_toOutsideBasinDy_m3[m_pFlowContext->dayOfYear] += discharge2POCD_m3_today;
         }
      } // end of loop thru POCDs

      /*
      if (pFlowContext->dayOfYear == 0)
      {
      CString msg;
      msg.Format("*** FateOfUGA_UrbanWaterUsingSewers(): dayOfYear = %d, uga = %d, allocated_m3_per_sec = %f, uga_UWoutdoor_m3_per_sec[uga] = %f",
      pFlowContext->dayOfYear, uga, allocated_m3_per_sec, uga_UWoutdoor_m3_per_sec[uga]);
      Report::LogMsg(msg);
      }
      */
   } // end of loop thru UGAs

} // end of FateOfUGA_UrbanWaterUsingSewers()


 bool AltWaterMaster::AllocateSurfaceWR(FlowContext *pFlowContext, WaterRight *pRight, int iduNdx, float pctPou)
   {
   MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

   MapLayer* pStreamLayer = (MapLayer*)pFlowContext->pFlowModel->m_pStreamLayer;

   // each water right is associated with a reach in the stream layer. This relationship is stored in the POD lookup table and assigned
   // to pRight when loading the lookup table.
   Reach *pReach = pRight->m_pReach;
   int reachNdx = pRight->m_streamIndex;
   float availSourceFlow = 0.0f; // m3/sec

   if (pReach != NULL)
      {	// A Point of Diversion in the Basin being modeled
      m_minFlow = DEFAULT_REACH_MINFLOW;

      availSourceFlow = (float)(pReach->m_availableDischarge - pReach->m_instreamWaterRightUse - NOMINAL_LOW_FLOW_CMS); // m3/sec

      if (availSourceFlow < 0.0) availSourceFlow = 0.0;
      }
   else availSourceFlow = 100000.0f; // This is a POD outside of the Basin being modeled.  

   int wrExists = 0;
   int plantDate = -1;
   int harvDate = -1;
   int nPOUSperIDU = -1;
   int ugb = -1;
   int ugbCol = -1;
   float checkSumMuni = 0;
   float sumMunPodRateIDU = 0.0; // sum of all potential municipal POD rates for this IDU (m3/sec)

   float iduAreaHa = 0.f; // hetares
   float iduAreaM2 = 0.0f; // m2
   float iduAreaAc = 0.f;// Acres

   pLayer->GetData(iduNdx, m_colAREA, iduAreaM2);
   pLayer->GetData(iduNdx, m_colWREXISTS, wrExists);
   pLayer->GetData(iduNdx, m_colPLANTDATE, plantDate);
   pLayer->GetData(iduNdx, m_colHarvDate, harvDate);

   unsigned __int16 iduUse = pRight->m_useCode;

   iduAreaHa = iduAreaM2 * HA_PER_M2;

   iduAreaAc = iduAreaM2 * ACRE_PER_M2;
   float amt_allocated_cms = 0.f;
   float unused_amt_cms = -1.f;
   switch (pRight->m_useCode)
   {

      case WRU_INSTREAM:
         break;

      case WRU_IRRIGATION:
         {
            amt_allocated_cms = AllocateIrrigationWater(pRight, iduNdx, pctPou, availSourceFlow, &unused_amt_cms);
            if (amt_allocated_cms > 0.f)
            { // add a negative flux (sink) to the reach. converting from m3/sec to m3/day
               float flux = amt_allocated_cms * SEC_PER_DAY; // m3
               pReach->CheckForNaNs("AllocateSurfaceWR 1", pReach->AddFluxFromGlobalHandler(-flux)); //m3/d
               pReach->m_availableDischarge -= amt_allocated_cms;
               m_SWIrrDuty += flux; // m3 

               float amount_allocated_mm = (flux / iduAreaM2) * 1000.f;
               m_iduSWIrrArrayDy[iduNdx] += amount_allocated_mm;// mm/d for a map
               m_iduSWIrrArrayYr[iduNdx] += amount_allocated_mm;// mm/d for a map					

               float amount_allocated_af = flux / M3_PER_ACREFT;
               m_irrigatedSurfaceYr += amount_allocated_af; // acre-feet
            }
            if (unused_amt_cms > 0.f)
            {
               ASSERT(unused_amt_cms >= 0.f);
               m_SWUnExIrr += unused_amt_cms;
               m_iduSWUnExerIrrArrayDy[iduNdx] += unused_amt_cms;
            }
            float out_irrig_cms; m_pReachLayer->GetData(pReach->m_polyIndex, m_colStreamOUT_IRRIG, out_irrig_cms);
            out_irrig_cms += amt_allocated_cms;
            m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colStreamOUT_IRRIG, out_irrig_cms);
         }
         break;

      case WRU_MUNICIPAL:
         {
            // if this IDU does not have a municipal Water right then break         
            if (IsMunicipal(iduUse))
            {
               ugbCol = pLayer->GetFieldCol("UGB");
               ugb = -1;
               pLayer->GetData(iduNdx, ugbCol, ugb);

               if (1 <= ugb && ugb <= MAX_UGA_NDX)
               {
                  CString msg;
                  msg.Format("*** AllocateSurfaceWR() ugb = %d, iduNdx = %d, IsMunicipal(iduUse) = %d, iduUse = %d",
                     ugb, iduNdx, IsMunicipal(iduUse), iduUse);
                  Report::LogMsg(msg);
                  break;
               }

               nPOUSperIDU = GetMuni(wrExists);

               // m_iduIrrRequestArray is in units mm/day, conversion reference http://www.fao.org/docrep/x0490e/x0490e04.htm
               // ET mm/day = 10 m3/ha/day
               // AREA in IDU layer is assumed to be meters squared m2
               // availSourceFlow = m3/sec
               // 10000 m2/ha

               float iduAreaHa = 0.f;
               float iduAreaM2 = 0.f;
               float iduDemand = 0.f;

               pLayer->GetData(iduNdx, m_colAREA, iduAreaM2); // m2

               pLayer->GetData(iduNdx, m_colUDMAND_DY, iduDemand); // m3/sec

               //pRight->m_stepRequest = iduDemand;

               //m2 -> ha
               iduAreaHa = iduAreaM2 * HA_PER_M2;

               if (iduDemand <= 0.f)
               {
               // POD rate m3/sec * areal percentage of the POU that intersects the IDU
                  m_iduSWUnAllocatedArray[iduNdx] += (pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou;
                  m_unallocatedMuniWater += (pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou;
                  m_iduSWUnExerMunArrayDy[iduNdx] += (pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou;
                  break;
               }

            // if available source flow is greater than or equal to the demand, satisfy all of demand
               if (availSourceFlow >= iduDemand)
               {
                  amt_allocated_cms = iduDemand;

                  // add a negative flux (sink) to the reach. converting from m3/sec to m3/day
                  float flux = iduDemand * SEC_PER_DAY;

                  pReach->CheckForNaNs("AllocateSurfaceWR 3", pReach->AddFluxFromGlobalHandler(-flux)); //m3/d
                  pReach->m_availableDischarge -= iduDemand;

                  m_iduSWUnAllocatedArray[iduNdx] += availSourceFlow - iduDemand;  // m3/sec?

                  if (((pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou) > iduDemand)
                  {
                     m_iduSWUnExerMunArrayDy[iduNdx] += ((pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou) - iduDemand;
                  }

               // daily allocated and unallocated municipal water m3/sec    
                  m_iduSWMuniArrayDy[iduNdx] += iduDemand; // m3/sec for a map
                  m_iduSWMuniArrayYr[iduNdx] += iduDemand * (60 * 60 * 24); // m3 H2O

                  m_SWMuniWater += iduDemand; // m3/sec for graph 

                  m_pWaterAllocation->m_iduNonIrrAllocationArray[iduNdx] += iduDemand; // m3/s
                  //sum the irrigation addition for each polygon to arrive at the total yearly irrigation amount
                  m_pWaterAllocation->m_iduYrMuniArray[iduNdx] += iduDemand; //m3/s;

               }
               else // the available source flow is less than demand, allocate all of the available source flow
               {
                  amt_allocated_cms = availSourceFlow;

                  // add a negative value to the reach. converting from m3/sec to m3/day
                  float flux = availSourceFlow * SEC_PER_DAY;
                  float ratio = 0.000001f;

                  if (iduDemand > 0.0)
                     ratio = availSourceFlow / iduDemand;

                  pReach->CheckForNaNs("AllocateSurfaceWR 4", pReach->AddFluxFromGlobalHandler(-flux)); // m3/day
                  pReach->m_availableDischarge = 0.0f;

                  m_iduSWUnsatMunDmdYr[iduNdx] += iduDemand - availSourceFlow;

                  m_iduSWUnSatMunArrayDy[iduNdx] += iduDemand - availSourceFlow;

                  if (((pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou) > iduDemand)
                  {
                     m_iduSWUnExerMunArrayDy[iduNdx] += ((pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou) - availSourceFlow;
                  }

                  m_iduSWMuniArrayDy[iduNdx] += availSourceFlow; // m3/sec for a map
                  m_iduSWMuniArrayYr[iduNdx] += availSourceFlow * (60 * 60 * 24); // m3 H2O

                  m_SWMuniWater += availSourceFlow; // m3/sec for graph

                  m_pWaterAllocation->m_iduNonIrrAllocationArray[iduNdx] += availSourceFlow; // m3/s
                  //sum the irrigation addition for each polygon to arrive at the total yearly irrigation amount
                  m_pWaterAllocation->m_iduYrMuniArray[iduNdx] += availSourceFlow; // m3/s;

               }

               float out_muni_cms; m_pReachLayer->GetData(pReach->m_polyIndex, m_colStreamOUT_MUNI, out_muni_cms);
               out_muni_cms += amt_allocated_cms;
               m_pReachLayer->SetDataU(pReach->m_polyIndex, m_colStreamOUT_MUNI, out_muni_cms);
            }
         }
         break;

      default: // ignore everything else

         break;

      }

   return true;
   } // end of AllocateSurfaceWR()

bool AltWaterMaster::AllocateWellWR(FlowContext *pFlowContext, WaterRight *pRight, int iduNdx, float pctPou)
	{
	MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

	int wrExists = 0;
	int plantDate = -1;
   int harvDate = -1;
	int nPOUSperIDU = -1;
	int ugb = -1;
	int ugbCol = -1;
	float checkSumMuni = 0;
	float sumMunPodRateIDU = 0.0; // sum of all potential municipal POD rates for this IDU (m3/sec)
	float iduAreaHa = 0.f;
	float iduAreaM2 = 0.0f;
	float iduAreaAc = 0.f;

	pLayer->GetData(iduNdx, m_colAREA, iduAreaM2);
	pLayer->GetData( iduNdx, m_colWREXISTS,  wrExists );
	pLayer->GetData( iduNdx, m_colPLANTDATE, plantDate );
   pLayer->GetData( iduNdx, m_colHarvDate,  harvDate );

	//m2 -> ha
	iduAreaHa = iduAreaM2 * HA_PER_M2;
	// m2 -> acres
	iduAreaAc = iduAreaM2 * ACRE_PER_M2;
	
	unsigned __int16 iduUse = pRight->m_useCode; 

	//*************************** Begin Temporary for development **********************************************
	// Assumes that irrigation source from well is infinite
	float availSourceFlow = 1000000.0; // m3/sec
	float wellHead = 0.0; // generic for now, may have to define later. 
	//*************************** End Temporary for development ************************************************

   switch (pRight->m_useCode)
		{

		case WRU_IRRIGATION:

			// This means that a place of use intersects this IDU, thus in pou.csv file. However, it did not meet the area
			// intersected threshold used to define WREXISTS for this IDU
			if ( IsIrrigation(iduUse) )
				{
				//if (pFlowContext->dayOfYear < plantDate || pFlowContext->dayOfYear > harvDate ) break;

				if ( m_wrIrrigateFlag )
					{
                    // m_iduIrrRequestArray is in units mm/day, conversion reference http://www.fao.org/docrep/x0490e/x0490e04.htm
					// ET mm/day = 10 m3/ha/day
					// AREA in IDU layer is assumed to be meters squared m2
					// availSourceFlow = m3/sec
					// 10000 m2/ha

					float iduAreaHa = 0.f;
					float iduAreaM2 = 0.0f;
					float iduDuty = 0.0f;
					float totIrrAcreFt = 0.0f;

					// m2
					pLayer->GetData(iduNdx, m_colAREA, iduAreaM2);

					//m2 -> ha
					iduAreaHa = iduAreaM2 * HA_PER_M2;

					float irrRequest = m_iduLocalIrrRequestArray[iduNdx];//mm/d

					if ( irrRequest < 0.0 ) irrRequest = 0.0;

					// m3/sec = m3/ha/day * ha / sec/day
					// float iduDemand = m_iduLocalIrrRequestArray[ iduNdx ] * iduAreaHa / SEC_PER_DAY;

					float iduDemand = irrRequest / 1000.0f * iduAreaM2 / SEC_PER_DAY; // m3/s
					
					//pRight->m_stepRequest = iduDemand;
					float iduDemandAcreFt = irrRequest / 1000.0f * iduAreaM2 / M3_PER_ACREFT; // acre-feet

					float actualIrrRequest = m_iduActualIrrRequestDy[iduNdx]; // mm/day

					// just do once per step at the first encounter with this idu
					if ( actualIrrRequest == 0.0 ) m_iduActualIrrRequestDy[iduNdx] = irrRequest;

					float totIrrMMYr = m_iduSWIrrArrayYr[iduNdx] + m_iduGWIrrArrayYr[iduNdx]; // mm/day

					if ( totIrrMMYr > 0.0f )
						totIrrAcreFt = totIrrMMYr / 1000.0f * iduAreaM2 / M3_PER_ACREFT; // acre-feet

					if ( totIrrAcreFt > 0.0f )
						iduDuty = totIrrAcreFt / iduAreaAc; // acre-feet per acre

               float irr_duty_factor = 0.f; pLayer->GetData(iduNdx, m_colXIRR_DUTY, irr_duty_factor);
               if (iduDuty >= (m_maxDuty_ft_per_yr*irr_duty_factor))
						{
						int agClass = 0;
						pLayer->GetData(iduNdx, m_colLulc_B, agClass);

						if (m_iduExceedDutyLog[iduNdx] != 1)
							{	
                     /*
                     CString msg; msg.Format("***AllocateWellWR(): iduDuty >= m_maxDuty_ft_per_yr.  iduNdx = %d, iduDuty = %f, LULC_B = %d",
                        iduNdx, iduDuty, agClass);
                     Report::LogMsg(msg);
                     */
							m_areaDutyExceeds += iduAreaAc;

							if ( m_debug )
								{
								if ( agClass == 21 )
									m_anGTmaxDutyArea21 += iduAreaAc;
								if ( agClass == 22 )
									m_anGTmaxDutyArea22 += iduAreaAc;
								if ( agClass == 23 )																									
									m_anGTmaxDutyArea23 += iduAreaAc;	
								if ( agClass == 24 )
									m_anGTmaxDutyArea24 += iduAreaAc;
								if ( agClass == 25 )
									m_anGTmaxDutyArea25 += iduAreaAc;
								if ( agClass == 26 )
									m_anGTmaxDutyArea26 += iduAreaAc;
								if (agClass == 27)
									{	
									m_anGTmaxDutyArea27 += iduAreaAc;
									int idu_id = -1;
									pLayer->GetData( iduNdx, m_colIDU_ID, idu_id);
									m_pastureIDUGTmaxDuty = idu_id;
									m_pastureIDUGTmaxDutyArea = iduAreaM2;
									}
								if ( agClass == 28 )
									m_anGTmaxDutyArea28 += iduAreaAc;
								if ( agClass == 29 )
									m_anGTmaxDutyArea29 += iduAreaAc;
								}						
							}

						m_iduExceedDutyLog[iduNdx] = 1;

						// this will halt irrigation to IDU if max duty is exceeded, and the switch do to so is set in the .xml file
                  if (m_maxDutyHalt == 1) break;
						}

					float weightFactor = 1.0f;
											
					// if available source flow is greater than half of the demand, satisfy all of demand
					if ( availSourceFlow >= iduDemand  )
						{		
                  pRight->m_allocatedToday_cms += iduDemand;

						// add a negative flux (sink) to the reach. converting from m3/sec to m3/day
						float flux = ( iduDemand * SEC_PER_DAY ) ;

						m_iduGWUnAllocatedArray[iduNdx] += availSourceFlow - (iduDemand / pRight->m_nPODSperWR);

                  if (((pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou) > (iduDemand))
                     m_iduGWUnExerIrrArrayDy[iduNdx] += ((pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou) - (iduDemand);
						
						//daily allocated and unallocated ag water mm/d 
						m_iduGWIrrArrayDy[iduNdx] += irrRequest ;// mm/d for a map
						m_iduGWIrrArrayYr[iduNdx] += irrRequest ;// mm/d for a map
						m_GWIrrDuty += iduDemand   * SEC_PER_DAY; //m3 for graph
											 
						m_GWIrrWaterMmDy += irrRequest ; // mm/d for graph
						m_irrigatedWaterYr += iduDemandAcreFt ; // acre-feet
						m_irrigatedGroundYr += iduDemandAcreFt ; // acre-feet
						int tempNPods = m_nIrrPODsPerIDUYr[iduNdx];
						tempNPods++;
						m_nIrrPODsPerIDUYr[iduNdx] = tempNPods;

						m_pWaterAllocation->m_iduIrrAllocationArray[iduNdx] += iduDemand ; // m3/s
						m_IrrFromAllocationArrayDy += iduDemand  * 1000 / iduAreaM2 * SEC_PER_DAY; //mm/day

                  m_iduLocalIrrRequestArray[iduNdx] -= irrRequest; //mm/day

						}
					else // the available source flow is less than demand, allocate all of the available source flow
						{
                  pRight->m_allocatedToday_cms += availSourceFlow;

                  //add a negative value to the reach. converting from m3/sec to m3/day
						float flux = availSourceFlow * SEC_PER_DAY;
						float ratio = 0.000001f;
						
						if ( iduDemand > 0.0 ) 
							ratio = availSourceFlow / iduDemand;

						m_iduGWUnsatIrrReqYr[iduNdx] += ( iduDemand  ) - availSourceFlow;

						m_GWIrrUnSatDemand += (iduDemand ) - availSourceFlow;

						m_unSatisfiedIrrGroundYr += ((iduDemand ) - availSourceFlow)* SEC_PER_DAY / M3_PER_ACREFT; // acre-feet
						
						float unsatDemandM3Sec = ( iduDemand  ) - availSourceFlow; // m3/day

						float unsatDemandMMDY = unsatDemandM3Sec * 1000 / iduAreaM2 * SEC_PER_DAY; //mm/day
						
                  if (((pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou) > (iduDemand))
                     m_iduGWUnExerIrrArrayDy[iduNdx] += ((pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou) - (iduDemand);

						float irrRequest = availSourceFlow * 1000 / iduAreaM2 * SEC_PER_DAY; //mm/day

						//daily allocated irrigation water m3/sec 
						m_iduGWIrrArrayDy[iduNdx] += irrRequest;// mm/d for a map
						m_iduGWIrrArrayYr[iduNdx] += irrRequest;// mm/d for a map
						m_GWIrrDuty += availSourceFlow * SEC_PER_DAY; //m3 for graph
						
						m_GWIrrWaterMmDy += irrRequest; // mm/d for graph
						m_irrigatedWaterYr += availSourceFlow * SEC_PER_DAY / M3_PER_ACREFT; // acre-feet
						m_irrigatedGroundYr += availSourceFlow * SEC_PER_DAY / M3_PER_ACREFT; // acre-feet
						int tempNPods = m_nIrrPODsPerIDUYr[iduNdx];
						tempNPods++;
						m_nIrrPODsPerIDUYr[iduNdx] = tempNPods;

						m_pWaterAllocation->m_iduIrrAllocationArray[iduNdx] += availSourceFlow; // m3/s
						m_IrrFromAllocationArrayDy += availSourceFlow * 1000 / iduAreaM2 * SEC_PER_DAY; //mm/day

						m_iduLocalIrrRequestArray[iduNdx] -= irrRequest; //mm/day
						}
					}
				}
			
			break;

		case WRU_MUNICIPAL:

			// This IDU do not have a municipal Water right then break         
			if ( IsMunicipal(iduUse) )
				{
			
				ugbCol = pLayer->GetFieldCol("UGB");
				ugb = -1;  
				pLayer->GetData( iduNdx, ugbCol, ugb );

            if (1 <= ugb && ugb <= MAX_UGA_NDX)
               {
               CString msg;
               msg.Format("*** AllocateWellWR() ugb = %d, iduNdx = %d, IsMunicipal(iduUse) = %d, iduUse = %d",
                  ugb, iduNdx, IsMunicipal(iduUse), iduUse);
               Report::LogMsg(msg);
               break;
               }

				nPOUSperIDU = GetMuni(wrExists);

				// m_iduIrrRequestArray is in units mm/day, conversion reference http://www.fao.org/docrep/x0490e/x0490e04.htm
				// ET mm/day = 10 m3/ha/day
				// AREA in IDU layer is assumed to be meters squared m2
				// availSourceFlow = m3/sec
				// 10000 m2/ha

				float iduAreaHa = 0.f;
				float iduAreaM2 = 0.f;
				float iduDemand = 0.f;

				pLayer->GetData(iduNdx, m_colAREA, iduAreaM2); // m2
           
            pLayer->GetData(iduNdx, m_colUDMAND_DY, iduDemand); // m3/sec

				//pRight->m_stepRequest = iduDemand;
				//m2 -> ha
				iduAreaHa = iduAreaM2 * HA_PER_M2;

				if ( iduDemand <= 0.f )
					{
					// POD rate m3/sec * areal percentage of the POU that intersects the IDU
               m_iduGWUnAllocatedArray[iduNdx] += (pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou;
               m_iduGWUnExerMunArrayDy[iduNdx] += (pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou;
               m_unallocatedMuniWater += (pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou;
					//break;
					}

				// if available source flow is greater than demand, satisfy all of demand
				if (availSourceFlow >= (iduDemand  / 2 ) )
					{
					// add a negative flux (sink) to the reach. converting from m3/sec to m3/day
					float flux = (iduDemand * SEC_PER_DAY) ;

					m_iduGWUnAllocatedArray[iduNdx] += availSourceFlow - ( iduDemand  );  // m3/sec?

               if (((pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou) > (iduDemand))
                  m_iduGWUnExerMunArrayDy[iduNdx] += ((pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou) - (iduDemand);

					// daily allocated and unallocated municipal water m3/sec    
					m_iduGWMuniArrayDy[iduNdx] += iduDemand ; // m3/sec for a map
					m_iduGWMuniArrayYr[iduNdx] += (iduDemand)*(60*60*24) ; // m3 H2O
					
					m_GWMuniWater += iduDemand ; // m3/sec for graph 
					
					m_pWaterAllocation->m_iduNonIrrAllocationArray[iduNdx] += iduDemand ; // m3/s
					//sum the irrigation addition for each polygon to arrive at the total yearly irrigation amount
					m_pWaterAllocation->m_iduYrMuniArray[iduNdx] += iduDemand ; //m3/sec;

					}
				else // the available source flow is less than demand, allocate all of the available source flow
					{
					// add a negative value to the reach. converting from m3/sec to m3/day
					float flux = availSourceFlow * SEC_PER_DAY;
					float ratio = 0.000001f;
					
					if ( iduDemand > 0.0 )
						ratio = availSourceFlow / iduDemand;

					m_iduGWUnsatMunDmdYr[iduNdx] += ( iduDemand  ) - availSourceFlow;
					
					m_iduGWUnSatMunArrayDy[iduNdx] += ( iduDemand  ) - availSourceFlow;
					

               if (((pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou) > (iduDemand))
                  m_iduGWUnExerMunArrayDy[iduNdx] += ((pRight->m_podRate_cfs / (FT3_PER_M3))* pctPou) - (iduDemand);
					
					m_iduGWMuniArrayDy[iduNdx] += availSourceFlow; // m3/sec for a map
               m_iduGWMuniArrayYr[iduNdx] += availSourceFlow * SEC_PER_DAY; // m3 H2O

					m_GWMuniWater += availSourceFlow; // m3/sec for graph
					
					m_pWaterAllocation->m_iduNonIrrAllocationArray[iduNdx] += availSourceFlow; // m3/s
					//sum the irrigation addition for each polygon to arrive at the total yearly irrigation amount
					m_pWaterAllocation->m_iduYrMuniArray[iduNdx] += iduDemand; // m3/s;

					}
				}

			break;

		default: // ignore everything else

			break;

		}

	return true;
}


bool AltWaterMaster::AggregateIDU2HRU(FlowContext *pFlowContext)
{
	int hruCount = pFlowContext->pFlowModel->GetHRUCount();
   MapLayer  *pIDULayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;
   MapLayer  *pHRUlayer = (MapLayer*)pFlowContext->pEnvContext->pHRUlayer;
   //float hruWaterAllotment;

   for (int h = 0; h < hruCount; h++)
   {
      //hruWaterAllotment = 0.0f;

      HRU *pHRU = pFlowContext->pFlowModel->GetHRU(h);

      HRULayer *pIrrHRULayer = pHRU->GetLayer(BOX_IRRIG_SOIL); //use "irrigated" layer

      HRULayer * pBoxFastGW = pHRU->GetLayer(BOX_FAST_GW);  

      // Find the ShallowGround layer: it's the one with pHRULayer->m_layer == 4
      HRULayer *pShallowGroundLayer = NULL;
      for (int l = 0; pShallowGroundLayer == NULL && l < pHRU->GetLayerCount(); l++)
      {
         HRULayer *pHRULayer = pHRU->GetLayer(l);
         if (pHRULayer->m_layer == 4) pShallowGroundLayer = pHRULayer;
      }

      int polyCount = int(pHRU->m_polyIndexArray.GetCount());
      ASSERT(polyCount > 0);
      float agg_irr2soil = 0.f, agg_irr2fastgw = 0.f, agg_muni2soil = 0.f;
      for (int p = 0; p < polyCount; p++)
      {

         int iduNdx = pHRU->m_polyIndexArray[p];

         float irrFlux = m_pWaterAllocation->m_iduIrrAllocationArray[iduNdx] * SEC_PER_DAY; // m3/day
         float irrigLossFactor = 0.20f; // Should be m_irrigLossFactor, but that's inaccessible from here.

         if (irrFlux > 0.0)
         { // Irrigation water goes mostly to topsoil, but there are also losses to subsoil and hence back to the streams. 
            pIrrHRULayer->CheckForNaNs("AggregateIDU2HRU 1", pIrrHRULayer->AddFluxFromGlobalHandler(irrFlux, FL_TOP_SOURCE)); //m3/d - add entire irrigation amount to top of soil
            pIrrHRULayer->CheckForNaNs("AggregateIDU2HRU 2", pIrrHRULayer->AddFluxFromGlobalHandler(irrFlux * irrigLossFactor, FL_BOTTOM_SINK)); // m3/d - move the loss out of the irrigated soil layer
            pShallowGroundLayer->CheckForNaNs("AggregateIDU2HRU 3", pShallowGroundLayer->AddFluxFromGlobalHandler(irrFlux * irrigLossFactor, FL_TOP_SOURCE)); //m3/d - put the loss into the ShallowGround layer
            float idu_irr2soil = (1.f - irrigLossFactor) * irrFlux;
            agg_irr2soil += idu_irr2soil;
            agg_irr2fastgw += irrFlux - idu_irr2soil;
         }

         float munFlux = m_pWaterAllocation->m_iduNonIrrAllocationArray[iduNdx] * SEC_PER_DAY; // m3/day
         if (munFlux > 0.0)
         { // Use the fast groundwater box, because the corresponding ET will be drawn from that compartment
            pBoxFastGW->CheckForNaNs("AggregateIDU2HRU 4", pBoxFastGW->AddFluxFromGlobalHandler(munFlux, FL_TOP_SOURCE)); // m3/d 
            agg_muni2soil += munFlux;
         }

      } // end of loop thru IDUs which make up the HRU
      pHRUlayer->SetDataU(h, m_colHruIRR2SOIL, agg_irr2soil);
      pHRUlayer->SetDataU(h, m_colHruIRR2FASTGW, agg_irr2fastgw);
      pHRUlayer->SetDataU(h, m_colHruMUNI2SOIL, agg_muni2soil);
   } // of loop thru HRUs

   return(true);
} // end of AggregateIDU2HRU()



void AltWaterMaster::LogWeeklySurfaceWaterIrrigationShortagesByIDU(int iduNdx, bool shortageFlag )
	{
   if (shortageFlag)
      {
      m_iduConsecutiveShortages[iduNdx]++;
      if (m_iduSWIrrWRSOIndex[iduNdx] == 1 && m_iduConsecutiveShortages[iduNdx] >= m_nDaysWRConflict2) m_iduSWIrrWRSOIndex[iduNdx] = 2;
      else if (m_iduSWIrrWRSOIndex[iduNdx] < 1 && m_iduConsecutiveShortages[iduNdx] >= m_nDaysWRConflict1) m_iduSWIrrWRSOIndex[iduNdx] = 1;
      }
   else m_iduConsecutiveShortages[iduNdx] = 0;
	} // end of LogWeeklySurfaceWaterIrrigationShortagesByIDU()

	bool AltWaterMaster::LogWeeklySurfaceWaterMunicipalConflicts(FlowContext *pFlowContext, WaterRight *pRight, int iduNdx, float areaPou)
	{
	// available water is below minimum. CONFLICT  Water Right Shut Off (WRSO).
	// Narative: a.	Weeks that water deliveries are shut off in a year.  
	// One delivery of one water-right is shut-off in one week, the index = 1. 
	// If the water right is shut off again in a subsequent week the index would be 2.  
	// 2 would also be 2 water rights shut off in one week. 

	MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

	// number days shut off (SO)
	pRight->m_nDaysPerWeekSO++;

	// at the end of each week, check for conflict
	if ( pFlowContext->dayOfYear == m_weekInterval )
		{
		if ( pRight->m_nDaysPerWeekSO == m_nDaysWRConflict1 ) //defined in .xml
			{
			m_iduSWMunWRSOIndex[iduNdx] = 1;
			m_iduSWMunWRSOWeek[iduNdx] = pFlowContext->dayOfYear;
			m_pouSWLevelOneMunWRSOAreaArray[iduNdx] = areaPou;
			pLayer->GetData( iduNdx, m_colAREA, m_iduSWLevelOneMunWRSOAreaArray[iduNdx] );
			}

		if ( pRight->m_nDaysPerWeekSO >= m_nDaysWRConflict2 ) //defined in .xml
			{
			m_iduSWMunWRSOIndex[iduNdx] = 2;
			m_iduSWMunWRSOWeek[iduNdx] = pFlowContext->dayOfYear;
			m_pouSWLevelTwoMunWRSOAreaArray[iduNdx] = areaPou;
			pLayer->GetData( iduNdx, m_colAREA, m_iduSWLevelTwoMunWRSOAreaArray[iduNdx] );
			}

		// reset number of days per week shut off to zero
		pRight->m_nDaysPerWeekSO = 0;

		}

	return true;
	}


bool AltWaterMaster::EndStep(FlowContext *pFlowContext)
{
	MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;
   MapLayer* pStreamLayer = (MapLayer*)pFlowContext->pFlowModel->m_pStreamLayer;

   bool readOnlyFlag = pLayer->m_readOnly;
   pLayer->m_readOnly = false;
   
   float time = (float)(pFlowContext->time - pFlowContext->pFlowModel->m_yearStartTime);

   m_toOutsideBasinYr_m3 += m_toOutsideBasinDy_m3[m_pFlowContext->dayOfYear];

   for (int uga = 1; uga <= MAX_UGA_NDX; uga++) m_fromOutsideBasinYr[uga] += m_fromOutsideBasinDy[uga] * SEC_PER_DAY;
   m_fromBullRunYr_m3 += m_fromBullRunDy_m3_per_s * SEC_PER_DAY;
   m_fromSpringHillYr_m3 += m_fromSpringHillDy_m3_per_s * SEC_PER_DAY;
   m_fromBarneyYr_m3 += m_fromBarneyDy_m3_per_s * SEC_PER_DAY;
   m_fromClackamasYr_m3 += m_fromClackamasDy_m3_per_s * SEC_PER_DAY;
   m_fromLakeOswegoYr_m3 += m_fromLakeOswegoDy_m3_per_s * SEC_PER_DAY;

	if ( m_debug )
		{
		CArray< float, float > rowDailyMetricsDebug;
		
		rowDailyMetricsDebug.SetSize(10);

		rowDailyMetricsDebug[0] = time;
		rowDailyMetricsDebug[1] = m_dyGTmaxPodArea21;
		rowDailyMetricsDebug[2] = m_dyGTmaxPodArea22;
		rowDailyMetricsDebug[3] = m_dyGTmaxPodArea23;
		rowDailyMetricsDebug[4] = m_dyGTmaxPodArea24;
		rowDailyMetricsDebug[5] = m_dyGTmaxPodArea25;
		rowDailyMetricsDebug[6] = m_dyGTmaxPodArea26;
		rowDailyMetricsDebug[7] = m_dyGTmaxPodArea27;
		rowDailyMetricsDebug[8] = m_dyGTmaxPodArea28;
		rowDailyMetricsDebug[9] = m_dyGTmaxPodArea29;
		this->m_dailyMetricsDebug.AppendRow(rowDailyMetricsDebug);

		m_dyGTmaxPodArea21 =0.0f;
		m_dyGTmaxPodArea22 =0.0f;
		m_dyGTmaxPodArea23 =0.0f;
		m_dyGTmaxPodArea24 =0.0f;
		m_dyGTmaxPodArea25 =0.0f;
		m_dyGTmaxPodArea26 =0.0f;
		m_dyGTmaxPodArea27 =0.0f;
		m_dyGTmaxPodArea28 =0.0f;
		m_dyGTmaxPodArea29 =0.0f;
		}

		// Reset a few things associated with a WR.
		for ( int i = 0; i < (int)m_podArray.GetSize(); i++ )
   		{
			// Get current WR or POD
			WaterRight *pRight = m_podArray[i];
			pRight->m_stepShortageFlag = false;
			}
	
	// write values to map
	MapLayer *pIDULayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

	int iduCount = pIDULayer->GetRecordCount();
	int colArea = pIDULayer->GetFieldCol("AREA");
         
   float acreage_of_IDUs_shut_off = 0.f;
   int num_of_IDUs_shut_off = 0;
   for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
   	{

		//float applied = m_pWaterAllocation->m_iduAllocationArray[ i ];  // m3/sec
		float swApplied = m_iduSWIrrArrayDy[idu];   // mm/day
		float gwApplied = m_iduGWIrrArrayDy[idu];   // mm/day
		float deficit   = m_iduLocalIrrRequestArray[idu];
		float fracDemand = 0.0f;
		// float actualIrrRequest = m_iduActualIrrRequestDy[idu];
		float allocatedIrr = m_iduSWIrrArrayDy[idu] + m_iduGWIrrArrayDy[idu]; 
		float unexercisedSWirr = m_iduSWUnExerIrrArrayDy[idu];
		float unexercisedSWmun = m_iduSWUnExerMunArrayDy[idu];
		float unsatSWMunDemand = m_iduSWUnSatMunArrayDy[idu];
		float unsatSWIrrRequest = m_iduSWUnSatIrrArrayDy[idu];
		float unsatGWMunDemand = m_iduGWUnSatMunArrayDy[idu];
      int wr_shortg; pIDULayer->GetData(idu, m_colWRShortG, wr_shortg);
      int wr_shutoff; pIDULayer->GetData(idu, m_colWRShutOff, wr_shutoff);

      float iduPRECIP = 0.f;
      float iduTEMP = 0.f;

		pIDULayer->SetData(idu, m_colSWIrrDy, swApplied);
		pIDULayer->SetData(idu, m_colGWIrrDy, gwApplied);
		pIDULayer->SetData(idu, m_colWaterDeficit, deficit);
		// pIDULayer->SetData(idu, m_colActualIrrRequestDy, actualIrrRequest); this is done in EvapTrans now
		pIDULayer->SetData(idu, m_colDailyAllocatedIrrigation, allocatedIrr);
		pIDULayer->SetData(idu, m_colSWUnexIrr, unexercisedSWirr);
		pIDULayer->SetData(idu, m_colSWUnexMun, unexercisedSWmun);
		pIDULayer->SetData(idu, m_colSWUnSatMunDemandDy, unsatSWMunDemand);
		pIDULayer->SetData(idu, m_colSWUnSatIrrRequestDy, unsatSWIrrRequest);
		pIDULayer->SetData(idu, m_colGWUnSatMunDemandDy, unsatGWMunDemand);

		int irrDecision = 1;

	   if ( m_colIrrigate != -1 )
			{
			pLayer->GetData(idu, m_colIrrigate, irrDecision);
			}
		else
			{
			irrDecision = 1; //default 1 =  yes irrigate
			}

/*
      float irrRequest = m_pWaterAllocation->m_iduIrrRequestArray[idu];

      if (irrDecision == 1)
         {
         pIDULayer->SetData(idu, m_colIrrRequestDy, irrRequest);
         }
*/
		if (alglib::fp_eq(deficit, 0.0))
			fracDemand = -1;
		else
			fracDemand = ((swApplied + gwApplied) / deficit);  // unitless ratio

      float totDyIrrmmDy = m_iduSWIrrArrayDy[idu] + m_iduGWIrrArrayDy[idu]; //mm/day

		// m_iduIrrRequestArray is in units mm/day, conversion reference http://www.fao.org/docrep/x0490e/x0490e04.htm
		// ET mm/day = 10 m3/ha/day
		// AREA in IDU layer is assumed to be meters squared m2
		// availSourceFlow = m3/sec
		// 10000 m2/ha

		float iduAreaM2 = 0.0f;

		// m2
		pLayer->GetData(idu, m_colAREA, iduAreaM2);

		float totDyIrrAcft = totDyIrrmmDy / 1000.0f * iduAreaM2 / M3_PER_ACREFT; // acre-feet

		if ( totDyIrrAcft >  m_maxTotDailyIrr[idu] ) 
			m_maxTotDailyIrr[idu] = totDyIrrAcft; // acre-ft
		pIDULayer->SetData(idu, m_colDemandFraction, fracDemand);

      float iduAreaAc = iduAreaM2 * ACRE_PER_M2;

      // additional tracking
      float idu_irr_rate_factor = 1.f; pLayer->GetData(idu, m_colXIRR_RATE, idu_irr_rate_factor);
      float maxRate_m3_per_sec = ((m_maxRate_cfs_per_ac * iduAreaAc) / FT3_PER_M3)*idu_irr_rate_factor;    // m3/sec for IDU area
      float totalIduIrrRate = ( totDyIrrmmDy / SEC_PER_DAY ) * M_PER_MM * iduAreaM2;  // m3/sec for IDU area for this day

      int irrigated = 0;
      pIDULayer->GetData( idu, this->m_colIrrigate, irrigated );

      if ( irrigated == 1 )
         {
         int startDate = 366;
         pIDULayer->GetData( idu, this->m_colPLANTDATE, startDate );
   
         int endDate = 0;
         pIDULayer->GetData( idu, this->m_colHarvDate, endDate );

         if ( pFlowContext->dayOfYear >= startDate && pFlowContext->dayOfYear <= endDate )
            {
            // float maxRate_m3_per_sec = (m_maxRate_cfs_per_ac * FT3_PER_M3) * iduAreaAc; already calculated above
            float wastedWater =  maxRate_m3_per_sec - totalIduIrrRate;   // m3/sec of wasted water for the IDU

            if ( totalIduIrrRate > maxRate_m3_per_sec * 1.1 )
               {
					m_iduExceededIrr_Yr[idu] += ( -wastedWater * SEC_PER_DAY * ACREFT_PER_M3 );
					m_iduExceededIrr_Dy[idu] = (-wastedWater);												
               wastedWater = 0;
               }
   
				m_iduWastedIrr_Yr[idu] += wastedWater * SEC_PER_DAY * ACREFT_PER_M3;  // acre-ft for this day
				m_iduWastedIrr_Dy[idu] = wastedWater;			
            }
         }

		pIDULayer->SetData( idu, m_colExcessIrrDy,m_iduExceededIrr_Dy[idu]);			
		pIDULayer->SetData( idu, m_colWastedIrrDy,m_iduWastedIrr_Dy[idu] );	

      if (wr_shutoff == 1)
         {
         num_of_IDUs_shut_off++; // year-to-date, not just today
         acreage_of_IDUs_shut_off += iduAreaM2 / M2_PER_ACRE; // year-to-date, not just today
         }

      int wr_instrm = 0; pIDULayer->GetData(idu, m_colWR_INSTRM, wr_instrm);
      if ((wr_instrm == 1) || ((wr_instrm == 2) && (m_useNewInstreamWR == 1)))
         {
         int streamNdx = -99; pIDULayer->GetData(idu, m_colSTRM_INDEX, streamNdx);
         if (streamNdx >= 0)
            { 
            float q_div_wrq = 0.f; pStreamLayer->GetData(streamNdx, m_colStreamQ_DIV_WRQ, q_div_wrq);
            float min_q_wrq = 0.f; pIDULayer->GetData(idu, m_colMIN_Q_WRQ, min_q_wrq);
            if ((min_q_wrq < 0.f && q_div_wrq >= 0.f) || (min_q_wrq >= 0. && q_div_wrq >= 0.f && q_div_wrq < min_q_wrq))
               pIDULayer->SetData(idu, m_colMIN_Q_WRQ, q_div_wrq);
            }
         }
      }  // end of: for each ( idu )

      pStreamLayer->m_readOnly = false;
      float accumulator = 0.f;
      for (MapLayer::Iterator streamNdx = pStreamLayer->Begin(); streamNdx != pStreamLayer->End(); streamNdx++)         
      {
      Reach *pReach = pFlowContext->pFlowModel->GetReachFromStreamIndex(streamNdx);
      if (pReach == NULL)
         {
         CString msg;
         msg.Format("*** AltWaterMaster::EndStep() streamNdx = %d  pReach is NULL", streamNdx); Report::LogMsg(msg);
         }
      else if (pReach->m_instreamWaterRightUse != 0.f)
         {
         // CString msg;
         // msg.Format("*** streamNdx = %d  pReach->m_instreamWaterRightUse = %f", streamNdx, pReach->m_instreamWaterRightUse);
         // Report::LogMsg(msg);
         accumulator += (float)pReach->m_instreamWaterRightUse;
         pStreamLayer->SetData(streamNdx, m_colStreamINSTRM_WRQ, pReach->m_instreamWaterRightUse);
         }
      } // end of loop thru the reach array

   pStreamLayer->m_readOnly = true;

   pIDULayer->m_readOnly = readOnlyFlag;

   // summary for all irrigation
   CArray< float, float > rowDailyMetrics;

   rowDailyMetrics.SetSize(15);

   rowDailyMetrics[0] = time;

   rowDailyMetrics[1] = m_SWMuniWater*SEC_PER_DAY;
   rowDailyMetrics[2] = m_GWMuniWater*SEC_PER_DAY;
   rowDailyMetrics[3] = m_SWUnExIrr;
   rowDailyMetrics[4] = m_SWIrrDuty;
   rowDailyMetrics[5] = m_GWIrrDuty;

   float outside_basin_accum_m3_per_sec = 0.f; for (int uga = 1; uga <= MAX_UGA_NDX; uga++) outside_basin_accum_m3_per_sec += m_fromOutsideBasinDy[uga];
   rowDailyMetrics[6] = outside_basin_accum_m3_per_sec;

   rowDailyMetrics[7] = m_fromBullRunDy_m3_per_s;
   rowDailyMetrics[8] = m_fromBarneyDy_m3_per_s;
   rowDailyMetrics[9] = m_fromSpringHillDy_m3_per_s;
   rowDailyMetrics[10] = m_fromClackamasDy_m3_per_s;
   rowDailyMetrics[11] = m_fromLakeOswegoDy_m3_per_s;

   float totH2OinReaches_m3 = (float)pFlowContext->pFlowModel->CalcTotH2OinReaches();
   rowDailyMetrics[12] = totH2OinReaches_m3;
   rowDailyMetrics[13] = (float)num_of_IDUs_shut_off; // year-to-date, not just today
   rowDailyMetrics[14] = acreage_of_IDUs_shut_off; // year-to-date, not just today

   this->m_dailyMetrics.AppendRow(rowDailyMetrics);

   rowDailyMetrics.RemoveAll();
   rowDailyMetrics.SetSize(55); for (int col = 0; col < 55; col++) rowDailyMetrics[col] = 0.f;
   rowDailyMetrics[0] = time;
   for (int uga = 1; uga <= MAX_UGA_NDX; uga++)
      {
      switch (uga)
         {
         case UGA_Metro: 
            rowDailyMetrics[1] = m_ugaUWallocatedDayFromSW[UGA_Metro] + m_ugaUWallocatedDayFromGW[UGA_Metro]; 
            rowDailyMetrics[2] = m_ugaUWallocatedDayFromSW[UGA_Metro];
            rowDailyMetrics[3] = m_ugaUWallocatedDayFromGW[UGA_Metro];
            rowDailyMetrics[4] = m_fromOutsideBasinDy[UGA_Metro]; // out-of-basin
            rowDailyMetrics[5] = m_ugaUWoutdoorDay_m3_per_sec[UGA_Metro]; // outdoor
            rowDailyMetrics[6] = m_ugaUWindoorDay_m3_per_sec[UGA_Metro]; // indoor
            break;
         case UGA_EugeneSpringfield: 
            rowDailyMetrics[7] = m_ugaUWallocatedDayFromSW[UGA_EugeneSpringfield] + m_ugaUWallocatedDayFromGW[UGA_EugeneSpringfield]; 
            rowDailyMetrics[8] = m_ugaUWallocatedDayFromSW[UGA_EugeneSpringfield];
            rowDailyMetrics[9] = m_ugaUWallocatedDayFromGW[UGA_EugeneSpringfield];
            rowDailyMetrics[10] = m_fromOutsideBasinDy[UGA_EugeneSpringfield];
            rowDailyMetrics[11] = m_ugaUWoutdoorDay_m3_per_sec[UGA_EugeneSpringfield];
            rowDailyMetrics[12] = m_ugaUWindoorDay_m3_per_sec[UGA_EugeneSpringfield];
            break;
         case UGA_SalemKeizer: 
            rowDailyMetrics[13] = m_ugaUWallocatedDayFromSW[UGA_SalemKeizer] + m_ugaUWallocatedDayFromGW[UGA_SalemKeizer]; 
            rowDailyMetrics[14] = m_ugaUWallocatedDayFromSW[UGA_SalemKeizer];
            rowDailyMetrics[15] = m_ugaUWallocatedDayFromGW[UGA_SalemKeizer];
            rowDailyMetrics[16] = m_fromOutsideBasinDy[UGA_SalemKeizer];
            rowDailyMetrics[17] = m_ugaUWoutdoorDay_m3_per_sec[UGA_SalemKeizer];
            rowDailyMetrics[18] = m_ugaUWindoorDay_m3_per_sec[UGA_SalemKeizer];
            break;
         case UGA_Corvallis: 
            rowDailyMetrics[19] = m_ugaUWallocatedDayFromSW[UGA_Corvallis] + m_ugaUWallocatedDayFromGW[UGA_Corvallis]; 
            rowDailyMetrics[20] = m_ugaUWallocatedDayFromSW[UGA_Corvallis];
            rowDailyMetrics[21] = m_ugaUWallocatedDayFromGW[UGA_Corvallis];
            rowDailyMetrics[22] = m_fromOutsideBasinDy[UGA_Corvallis];
            rowDailyMetrics[23] = m_ugaUWoutdoorDay_m3_per_sec[UGA_Corvallis];
            rowDailyMetrics[24] = m_ugaUWindoorDay_m3_per_sec[UGA_Corvallis];
            break;
         case UGA_Albany: 
            rowDailyMetrics[25] = m_ugaUWallocatedDayFromSW[UGA_Albany] + m_ugaUWallocatedDayFromGW[UGA_Albany]; 
            rowDailyMetrics[26] = m_ugaUWallocatedDayFromSW[UGA_Albany];
            rowDailyMetrics[27] = m_ugaUWallocatedDayFromGW[UGA_Albany];
            rowDailyMetrics[28] = m_fromOutsideBasinDy[UGA_Albany];
            rowDailyMetrics[29] = m_ugaUWoutdoorDay_m3_per_sec[UGA_Albany];
            rowDailyMetrics[30] = m_ugaUWindoorDay_m3_per_sec[UGA_Albany];
            break;
         case UGA_McMinnville: 
            rowDailyMetrics[31] = m_ugaUWallocatedDayFromSW[UGA_McMinnville] + m_ugaUWallocatedDayFromGW[UGA_McMinnville]; 
            rowDailyMetrics[32] = m_ugaUWallocatedDayFromSW[UGA_McMinnville];
            rowDailyMetrics[33] = m_ugaUWallocatedDayFromGW[UGA_McMinnville];
            rowDailyMetrics[34] = m_fromOutsideBasinDy[UGA_McMinnville];
            rowDailyMetrics[35] = m_ugaUWoutdoorDay_m3_per_sec[UGA_McMinnville];
            rowDailyMetrics[36] = m_ugaUWindoorDay_m3_per_sec[UGA_McMinnville];
            break;
         case UGA_Newberg: 
            rowDailyMetrics[37] = m_ugaUWallocatedDayFromSW[UGA_Newberg] + m_ugaUWallocatedDayFromGW[UGA_Newberg]; 
            rowDailyMetrics[38] = m_ugaUWallocatedDayFromSW[UGA_Newberg];
            rowDailyMetrics[39] = m_ugaUWallocatedDayFromGW[UGA_Newberg];
            rowDailyMetrics[40] = m_fromOutsideBasinDy[UGA_Newberg];
            rowDailyMetrics[41] = m_ugaUWoutdoorDay_m3_per_sec[UGA_Newberg];
            rowDailyMetrics[42] = m_ugaUWindoorDay_m3_per_sec[UGA_Newberg];
            break;
         case UGA_Woodburn: 
            rowDailyMetrics[43] = m_ugaUWallocatedDayFromSW[UGA_Woodburn] + m_ugaUWallocatedDayFromGW[UGA_Woodburn]; 
            rowDailyMetrics[44] = m_ugaUWallocatedDayFromSW[UGA_Woodburn];
            rowDailyMetrics[45] = m_ugaUWallocatedDayFromGW[UGA_Woodburn];
            rowDailyMetrics[46] = m_fromOutsideBasinDy[UGA_Woodburn];
            rowDailyMetrics[47] = m_ugaUWoutdoorDay_m3_per_sec[UGA_Woodburn];
            rowDailyMetrics[48] = m_ugaUWindoorDay_m3_per_sec[UGA_Woodburn];
            break;
         default: 
            rowDailyMetrics[49] += m_ugaUWallocatedDayFromSW[uga] + m_ugaUWallocatedDayFromGW[uga]; 
            rowDailyMetrics[50] += m_ugaUWallocatedDayFromSW[uga];
            rowDailyMetrics[51] += m_ugaUWallocatedDayFromGW[uga];
            rowDailyMetrics[52] += m_fromOutsideBasinDy[uga];
            rowDailyMetrics[53] += m_ugaUWoutdoorDay_m3_per_sec[uga];
            rowDailyMetrics[54] += m_ugaUWindoorDay_m3_per_sec[uga];
            break;
         }
      }
   this->m_DailyMunicipalAllocations.AppendRow(rowDailyMetrics);

	// keep track of which week of the year we are in for conflict metrics
	if (pFlowContext->dayOfYear == m_weekInterval)
	   {
		m_weekOfYear++;
		m_weekInterval = m_weekInterval + 7;
	}

   return true;
   } // end of EndStep()


	bool AltWaterMaster::StartYear(FlowContext *pFlowContext)
		{

		MapLayer* pStreamLayer = (MapLayer*)pFlowContext->pFlowModel->m_pStreamLayer;

		MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

      if (pFlowContext->pEnvContext->yearOfRun == 0)
         {
         m_totH2OlastYr_m3 = pFlowContext->pFlowModel->CalcTotH2O(); // Calculate starting water in the HRUs.
         CString msg;
         msg.Format("*** AltWaterMaster::StartYear() yearOfRun 0: CalcTotH2OinReaches() = %f, CalcTotH2OinReservoirs() = %f, CalcTotH2OinHRUs = %f, CalcTotH2O() = %f",
            pFlowContext->pFlowModel->CalcTotH2OinReaches(), pFlowContext->pFlowModel->CalcTotH2OinReservoirs(), pFlowContext->pFlowModel->CalcTotH2OinHRUs(), pFlowContext->pFlowModel->CalcTotH2O());
         Report::LogMsg(msg);
         }

      if (m_dynamicWRType == 3 || m_dynamicWRType==4) for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
         {
         int lulc_a = -1; pLayer->GetData(idu, m_colLulc_A, lulc_a); if (lulc_a != LULCA_AGRICULTURE) continue;
         int wr_irrig_s = 0; pLayer->GetData(idu, m_colWR_IRRIG_S, wr_irrig_s); if (wr_irrig_s != 0) continue;
         int wr_irrig_g = 0; pLayer->GetData(idu, m_colWR_IRRIG_G, wr_irrig_g); if (wr_irrig_g != 0) continue;
         float wr_prob = 0.f; pLayer->GetData(idu, m_colWR_PROB, wr_prob); if (wr_prob <= 0.f) continue;
         double random_pick = m_WMuniformDist.RandValue(0., 1.);
         if (wr_prob < random_pick) continue;

         // Add surface water irrigation water right for this IDU.
         gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colWR_PROB, -wr_prob, true); // WR_PROB<0 signals that this IDU has had a WR added.
         int comid_pod = -99; pLayer->GetData(idu, m_colCOMID_POD, comid_pod); 
         AddWaterRight(pFlowContext, idu, comid_pod);
         float ecc0 = 0.f; pLayer->GetData(idu, m_colECC0, ecc0);
         gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colECC, ecc0, true);
         }

		m_nSWLevelOneMunWRSO = 0;
		m_nSWLevelTwoMunWRSO = 0;

		m_irrWaterRequestYr = 0.f;
		m_irrigableIrrWaterRequestYr = 0.f;
		m_irrigatedWaterYr = 0.f;
		m_irrigatedSurfaceYr = 0.f;
		m_irrigatedGroundYr = 0.f;
		m_unSatisfiedIrrigationYr = 0.f;

		m_areaDutyExceeds = 0.0f;
		m_pastureIDUGTmaxDuty = -1;
	   m_pastureIDUGTmaxDutyArea = 0.0f; 

      for (int jday = 0; jday < 366; jday++) m_toOutsideBasinDy_m3[jday] = 0.;
      m_toOutsideBasinYr_m3 = 0.;

      for (int uga = 0; uga <= MAX_UGA_NDX; uga++)
         {
         m_ugaUWallocatedYr[uga] = 0.f;
         m_ugaUWshortageYr[uga] = 0.f;
         m_ugaUWfromSW[uga] = 0.f;
         m_ugaUWfromGW[uga] = 0.f;
         m_ugaPop[uga] = 0.;
         m_ugaMuniBackupWRuse_acft[uga] = 0.f;
         m_fromOutsideBasinYr[uga] = 0.f;
         } // end of loop on UGAs
      m_fromBullRunYr_m3 = 0.f;
      m_fromSpringHillYr_m3 = 0.f;
      m_fromBarneyYr_m3 = 0.f;
      m_fromClackamasYr_m3 = 0.f;
      m_GWnoWR_Yr_m3 = 0.f;

      for (int pocd_ndx = 0; pocd_ndx < m_POCDs.GetCount(); pocd_ndx++) m_POCDs[pocd_ndx]->pocdOutfallAmtYr_m3 = 0.f;

      pLayer->m_readOnly = false;

      pLayer->SetColData(m_colGWMUNALL_Y, VData(0), true);
      pLayer->SetColData(m_colGWMUNALL_D, VData(0), true);
      pLayer->SetColData(m_colSWMUNALL_Y, VData(0), true);
      pLayer->SetColData(m_colSWMUNALL_D, VData(0), true);
      pLayer->SetColData(m_colMIN_Q_WRQ, VData(-1.e-20f), true);
      pLayer->SetColData(m_colWRShutOff, VData(0), true);
      pLayer->SetColData(m_colWR_SH_DAY, VData(366), true);
      pLayer->SetColData(m_colIRR_STATE, VData(IRR_NOT_STARTED_OR_OFF_SEASON), true);
      pLayer->SetColData(m_colUWOUTDOORD, VData(0), true);

		for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
			{
         int uga = 0; pLayer->GetData(idu, m_colUGB, uga);
         if (uga>0 && uga <= MAX_UGA_NDX)
            {
            float iduPop = 0.f; pLayer->GetData(idu, m_colPOP, iduPop);
            m_ugaPop[uga] += iduPop;
            }

			pLayer->SetData( idu,m_colIrrRequestYr , 0 );
         pLayer->SetData(idu, m_colWRShutOff, 0);
         pLayer->SetData(idu, m_colWRShutOffMun, 0);
         pLayer->SetData(idu, m_colWRShortG, 0);
 			m_iduWastedIrr_Yr[idu] = 0.0f;
			m_iduExceededIrr_Yr[idu] = 0.0f;

         m_iduSWLevelOneIrrWRSOAreaArray[idu] = 0.0f;
         m_iduSWLevelTwoIrrWRSOAreaArray[idu] = 0.0f;
         m_iduSWIrrWRSOIndex[idu] = -1;
         m_iduSWIrrWRSOWeek[idu] = -1;
         m_iduSWIrrWRSOLastWeek[idu] = -999;
         m_iduUnsatIrrReqst_Yr[idu] = 0.0f;

         m_iduExceedDutyLog[idu] = 0;

         m_iduSWIrrArrayYr[idu] = 0.f;
         m_iduGWIrrArrayYr[idu] = 0.f;
      } // end of IDU loop

   int reachCount = pFlowContext->pFlowModel->GetReachCount();
	
	// reset conflict flags in stream layer
   pFlowContext->pFlowModel->m_pStreamLayer->m_readOnly = false;
	for (int reachIndex = 0; reachIndex < reachCount; reachIndex++)
		{	
		pFlowContext->pFlowModel->m_pStreamLayer->SetData(reachIndex, m_colWRConflictYr, 0);
		pFlowContext->pFlowModel->m_pStreamLayer->SetData(reachIndex,  m_colNInConflict, 0);
		m_reachDaysInConflict[reachIndex] = 0;
		}
	pFlowContext->pFlowModel->m_pStreamLayer->m_readOnly = true;

   for (int i = 0; i < (int)m_podArray.GetSize(); i++)
      {
      // Get current WR
      WaterRight *pRight = m_podArray[i];
      pRight->m_stepRequest = 0.f;
      } // end of loop thru POD array

   m_weekInterval = 6; // 0 based, expressed as day of year
   pLayer->SetColData(m_colAllocatedIrrigation, VData(0), true); 
   for (int idu = 0; idu<m_iduConsecutiveShortages.GetSize(); idu++) m_iduConsecutiveShortages[idu] = 0;

   pLayer->m_readOnly = true;

   return true;
	} // end of AltWaterMaster::StartYear()


bool AltWaterMaster::EndYear(FlowContext *pFlowContext)
   {
	MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;	
	MapLayer* pStreamLayer = (MapLayer*)pFlowContext->pFlowModel->m_pStreamLayer;

   // accumulators for the Quick Check metrics
   double precip_accumulator_m3 = 0.f; // accumulates m3 H2O
   double groundwater_accumulator_ac_ft = 0.f; // accumulates acre-feet H2O
   double aquifer_recharge_accumulator_m3 = 0.f;
   double aet_accumulator_m3 = 0.f; // accumulates m3 H2O
   double snow_evap_accumulator_m3 = 0.f;
   double rain_evap_accumulator_m3 = 0.f;
   double tot_area = 0.f;
	int iduCount = pLayer->GetRecordCount();
	vector<int> *iduNdxVec = 0;

   double tot_ground_water_to_reach_cms = 0.;
   for (MapLayer::Iterator reach = m_pReachLayer->Begin(); reach < m_pReachLayer->End(); reach++)
      {
      double neg_ground_water_to_reach_cms = 0.; m_pReachLayer->GetData(reach, m_colStreamXFLUX_Y, neg_ground_water_to_reach_cms);
      tot_ground_water_to_reach_cms -= neg_ground_water_to_reach_cms;
      }

	for (int idu = 0; idu < iduCount; idu++)
	   {
		float iduAreaHa = 0.f;
		float iduAreaM2 = 0.0f;
		float iduAreaAc = 0.f;
		int plantDate = 0;
		int harvDate = 0;
		int iduNdx = -1;

		pLayer->GetData( idu, m_colAREA, iduAreaM2 );
      tot_area += iduAreaM2;

		pLayer->GetData( idu, m_colPLANTDATE, plantDate );
      pLayer->GetData( idu, m_colHarvDate,  harvDate );
		//m2 -> ha
		iduAreaHa = iduAreaM2 * HA_PER_M2;
		// m2 -> acres
		iduAreaAc = iduAreaM2 * ACRE_PER_M2;

      if ( !pFlowContext->pFlowModel->m_estimateParameters )
         {
         if (m_iduSWIrrWRSOIndex[idu] == 1)
					 {					 
				    gpFlow->AddDelta( pFlowContext->pEnvContext, idu, m_colWRShortG, 1 );
					 }
				 if (m_iduSWIrrWRSOIndex[idu] == 2)
					 {
				    gpFlow->AddDelta( pFlowContext->pEnvContext, idu, m_colWRShortG, 2 );
					 }
         }

		// check week before for water right shut off, if so, then index = 2
		if ( m_iduSWMunWRSOWeek[idu] == 6 )
		   {
			if ( m_iduSWMunWRSOWeek[idu] - m_iduSWMunWRSOLastWeek[idu] == -358 )
				m_iduSWMunWRSOIndex[idu] = 2;
		   }
		else
		   {
			if ( m_iduSWMunWRSOWeek[idu] - m_iduSWMunWRSOLastWeek[idu] == 7 )
				m_iduSWMunWRSOIndex[idu] = 2;
		   }

		// Quantify water right shut off metrics
		if ( m_iduSWMunWRSOIndex[idu] == 1 )
			m_nSWLevelOneMunWRSO++;

		if ( m_iduSWMunWRSOIndex[idu] == 2 )
			m_nSWLevelTwoMunWRSO++;

      if ( !pFlowContext->pFlowModel->m_estimateParameters )
         {
		   if ( m_colWRShutOffMun != -1 )
		       {
			    if ( m_iduSWMunWRSOIndex[idu] == 1 )
				    gpFlow->AddDelta( pFlowContext->pEnvContext, idu, m_colWRShutOffMun, 1 );

			    if ( m_iduSWMunWRSOIndex[idu] == 2 )
				    gpFlow->AddDelta( pFlowContext->pEnvContext, idu, m_colWRShutOffMun, 2 );
		       }
         }

	   // m_iduIrrRequestArray is in units mm/day, conversion reference http://www.fao.org/docrep/x0490e/x0490e04.htm
		// ET mm/day = 10 m3/ha/day
		// AREA in IDU layer is assumed to be meters squared m2
		// availSourceFlow = m3/sec
		// 10000 m2/ha

		float totDyIrrmmDy = m_iduSWIrrArrayYr[idu] + m_iduGWIrrArrayYr[idu];
		float totDyIrrAcft = totDyIrrmmDy / 1000.0f * iduAreaM2 / M3_PER_ACREFT; // acre-feet
		float swDyIrrAcft = m_iduSWIrrArrayYr[idu] / 1000.0f * iduAreaM2 / M3_PER_ACREFT; // acre-feet
		float unsatIrrAcft =  m_iduUnsatIrrReqst_Yr[idu]; // acre-feet
		float swUnsatMunAcft =  m_iduSWUnsatMunDmdYr[idu] / 1000.0f * iduAreaM2 / M3_PER_ACREFT; // acre-feet
		float gwUnsatMunAcft =  m_iduGWUnsatMunDmdYr[idu] / 1000.0f * iduAreaM2 / M3_PER_ACREFT; // acre-feet
		float totUnsatMunAcft =  swUnsatMunAcft + gwUnsatMunAcft;
		float wastedIrrWaterAcft = m_iduWastedIrr_Yr[idu]; // acre-ft
		float excessIrrWaterAcft = m_iduExceededIrr_Yr[idu]; // acre-ft
		int   isMaxDutyExceeded = m_iduExceedDutyLog[idu]; // binary
		
		int daysInGrowingSeason  = ABS(harvDate) - plantDate; // defaults are harvdate = -999 and plantdate = 999

		if ( daysInGrowingSeason != 0 )
			m_aveTotDailyIrr[idu] = totDyIrrAcft / daysInGrowingSeason; // acre-feet

      gpFlow->UpdateIDU( pFlowContext->pEnvContext, idu, m_colAllocatedIrrigation, totDyIrrmmDy, true );
		gpFlow->UpdateIDU( pFlowContext->pEnvContext, idu, m_colAllocatedIrrigationAf, totDyIrrAcft, true );
		gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colUnsatIrrigationAf, unsatIrrAcft, true );
		gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colGWUnsatIrrigationAf, 0, true );		 
		gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colSWAllocatedIrrigationAf, swDyIrrAcft, true );
		gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colIrrRequestYr, m_iduIrrWaterRequestYr[idu], true ) ;
		gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colMaxTotDailyIrr, m_maxTotDailyIrr[idu], true ) ;
		gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colaveTotDailyIrr, m_aveTotDailyIrr[idu], true ) ;
		gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colWastedIrrYr, wastedIrrWaterAcft, true ) ;
		gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colExcessIrrYr, excessIrrWaterAcft, true ) ;
		gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colIrrExceedMaxDutyYr, isMaxDutyExceeded, true );

      float iduPRECIP_YR = 0.f;
      int hru_ndx = -1; pLayer->GetData(idu, m_colHRU_NDX, hru_ndx);
      if (hru_ndx >= 0)
         {
         HRU * pHRU = m_pFlowModel->GetHRU(hru_ndx);
         aet_accumulator_m3 += (pHRU->m_et_yr / 1000.f) * iduAreaM2;
         aquifer_recharge_accumulator_m3 += (pHRU->m_aquifer_recharge_yr_mm / 1000.) * iduAreaM2;

         // pHRU->m_precip_yr may be larger than the sum of pHRU->m_rainfall_yr + pHRU->m_snowfall_yr
         // because m_snowfall_yr doesn't include snow intercepted by the canopy and evaporated or ablated back
         // to the atmosphere (i.e. SNOW_EVAP).  Moreover, SNOW_EVAP varies from IDU to IDU because it depends
         // on LAI.
         iduPRECIP_YR = pHRU->m_precip_yr;
         }

      gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colPRECIP_YR, iduPRECIP_YR, true);
      precip_accumulator_m3 += (iduPRECIP_YR / 1000.f) * iduAreaM2;

      float iduSnowEvap_mm = 0.f; pLayer->GetData(idu, m_colSNOWEVAP_Y, iduSnowEvap_mm);
      snow_evap_accumulator_m3 += (iduSnowEvap_mm / 1000.f) * iduAreaM2;

      float iduRainEvap_mm = 0.f; 
      rain_evap_accumulator_m3 += (iduRainEvap_mm / 1000.f) * iduAreaM2;

      } // end IDU loop

   // Reset a few things associated with a WR, and test for Use it or lose it.
   for (int i = 0; i < (int)m_podArray.GetSize(); i++)
      {
      // Get current WR or POD
      WaterRight *pRight = m_podArray[i];

      // reset for next year
      pRight->m_inConflict = false;
      pRight->m_wrAnnualDuty = 0.0f;
      pRight->m_stepsSuspended = 0; // this is reset if need be in a step, just here for last day of year
      } // for POD input table */
     
   float ugaGalPerPersonPerDay[MAX_UGA_NDX + 1];
   float muni_accumulator_m3 = 0.f;
   float from_outside_basin_accumulator_m3 = 0.f; 

   for (int uga = 1; uga <= MAX_UGA_NDX; uga++) 
      {
      if (m_ugaPop[uga] > 0)
         {
         muni_accumulator_m3 += m_ugaUWallocatedYr[uga];
         from_outside_basin_accumulator_m3 += m_fromOutsideBasinYr[uga];
         ugaGalPerPersonPerDay[uga] = (float)(((m_ugaUWallocatedYr[uga] / M3_PER_GAL) / m_ugaPop[uga]) / pFlowContext->pEnvContext->daysInCurrentYear); // Convert m3/yr to gallons per capita per day.
         /*
            {
            CString msg;
            msg.Format("*** AltWaterMaster::EndYear() uga = %d, m_ugaUWallocatedYr[uga] = %f (m3), ugaPop[uga] = %f, ugaGalPerPersonPerDay[uga] = %f",
               uga, m_ugaUWallocatedYr[uga], (float)ugaPop[uga], ugaGalPerPersonPerDay[uga]);
            Report::LogMsg(msg);
            }
         */
         if (m_ugaUWshortageYr[uga] > 0.f)
            {
            CString msg;
            msg.Format("*** AltWaterMaster::EndYear() UW shortage in UGA. uga = %d, m_ugaUWallocatedYr[uga] = %f (m3), m_ugaPop[uga] = %f, ugaGalPerPersonPerDay[uga] = %f, m_ugaUWshortageYr[uga] = %f (m3)",
               uga, m_ugaUWallocatedYr[uga], (float)m_ugaPop[uga], ugaGalPerPersonPerDay[uga], m_ugaUWshortageYr[uga]);
            Report::LogMsg(msg);
            }
         }
      else ugaGalPerPersonPerDay[uga] = 0.f;
   } // end of loop on UGAs

   int num_of_IDUs_shut_off = 0;
   float acreage_of_IDUs_shut_off = 0.f;
   for (int idu = 0; idu < iduCount; idu++)
      {
      float gals_per_capita_per_day = 0.f;
      float iduUWfromSW_m3 = m_iduSWMuniArrayYr[idu];
      float iduUWfromGW_m3 = m_iduGWMuniArrayYr[idu];
      float iduUW_m3 = iduUWfromSW_m3 + iduUWfromGW_m3; // m3
      float iduPop = 0.f; pLayer->GetData(idu, m_colPOP, iduPop);
      int uga = -1; pLayer->GetData(idu, m_colUGB, uga);
      bool errFlag = false;
      if (uga >= 1 && uga <= MAX_UGA_NDX)
         { // Urban water use was accumulated by UGA.
         gals_per_capita_per_day = ugaGalPerPersonPerDay[uga];
         if (m_ugaPop[uga] > 0.f)
            {
            float pop_frac = (float)(iduPop / m_ugaPop[uga]);
            iduUWfromSW_m3 = m_ugaUWfromSW[uga] * pop_frac; // m3 H2O
            iduUWfromGW_m3 = m_ugaUWfromGW[uga] * pop_frac; // m3 H2O
            }
         else errFlag = (iduUW_m3 > 0.f);
         }
      else
         { // Urban water use was accumulated by IDU.
         if (iduUW_m3 > 0.f && iduPop > 0.f)
            {
            gals_per_capita_per_day = ((iduUW_m3 / M3_PER_GAL) / iduPop) / pFlowContext->pEnvContext->daysInCurrentYear; // Convert from m3 to gals per capita per day.
            iduUWfromGW_m3 = m_iduGWMuniArrayYr[idu];
            }
         else errFlag = (iduUW_m3 > 0.f);
         }
      gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colGAL_CAP_DY, gals_per_capita_per_day, true);
      gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colSWMUNALL_Y, iduUWfromSW_m3, true);
      gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colGWMUNALL_Y, iduUWfromGW_m3, true);

      float iduArea_m2 = 0.f; pLayer->GetData(idu, m_colAREA, iduArea_m2);
      float gwDyIrrAcft = ((m_iduGWIrrArrayYr[idu] / 1000.0f) * iduArea_m2) / M3_PER_ACREFT; // Convert mm to acre-feet
      gpFlow->UpdateIDU(pFlowContext->pEnvContext, idu, m_colIRGWAF_Y, gwDyIrrAcft, true);

      float iduGWMUNALL_Y_ac_ft = iduUWfromGW_m3 / M3_PER_ACREFT;
      groundwater_accumulator_ac_ft += gwDyIrrAcft + iduGWMUNALL_Y_ac_ft;

      if (errFlag)
         {
         CString msg; msg.Format("*** AltWaterMaster::EndYear() urban water use > 0 where pop <= 0. idu = %d, uga = %d, iduUW_m3 = %f, iduPop = %f", idu, uga, iduUW_m3, iduPop);
         Report::WarningMsg(msg);
         gals_per_capita_per_day = 0.f;
         }

      int wr_shutoff = 0; pLayer->GetData(idu, m_colWRShutOff, wr_shutoff);
      if (wr_shutoff == 1)
         {
         num_of_IDUs_shut_off++;
         acreage_of_IDUs_shut_off += iduArea_m2 / M2_PER_ACRE;
         } 
      } // end of IDU loop

   int num_PODs_shut_off = 0;
   CArray< int, int > waterRightIDs_shutoff;
   for (int i = 0; i < (int)m_podArray.GetSize(); i++) if (m_podArray[i]->m_suspendForYear)
      {
      num_PODs_shut_off++;
      int waterRightID = m_podArray[i]->m_wrID;
      int j = 0;
      boolean found = false;
      while (!found && (j < waterRightIDs_shutoff.GetCount())) 
         {
         found = waterRightID == waterRightIDs_shutoff[j];
         j++;
         }
      if (!found) waterRightIDs_shutoff.Add(waterRightID);
      m_podArray[i]->m_suspendForYear = false;
      } // end of loop thru the POD array

   m_iduSWIrrWRSOLastWeek.Copy(m_iduSWIrrWRSOWeek);

	m_iduSWMunWRSOLastWeek.Copy(m_iduSWMunWRSOWeek);

	int time = pFlowContext->pEnvContext->currentYear;

	// summary for all irrigation
   CArray< float, float > rowAnnualMetrics;

   rowAnnualMetrics.SetSize(18);

   rowAnnualMetrics[0] = (float)time;

   //rowAnnualMetrics[1] = m_irrWaterRequestYr;
   rowAnnualMetrics[1] = m_irrigableIrrWaterRequestYr;
   rowAnnualMetrics[2] = m_irrigatedWaterYr;
   rowAnnualMetrics[3] = m_irrigatedSurfaceYr;
   rowAnnualMetrics[4] = m_irrigatedGroundYr;
   rowAnnualMetrics[5] = m_GWnoWR_Yr_m3 * ACREFT_PER_M3;
   rowAnnualMetrics[6] = (float)num_PODs_shut_off;
   rowAnnualMetrics[7] = m_unSatisfiedIrrigationYr;
   rowAnnualMetrics[8] = (float)num_of_IDUs_shut_off;  // acre-ft/year
   rowAnnualMetrics[9] = acreage_of_IDUs_shut_off;  
   rowAnnualMetrics[10] = m_areaDutyExceeds;  // acres
   rowAnnualMetrics[11] = (float)waterRightIDs_shutoff.GetCount(); 
   rowAnnualMetrics[12] = from_outside_basin_accumulator_m3 / M3_PER_ACREFT;
   rowAnnualMetrics[13] = m_fromBullRunYr_m3 / M3_PER_ACREFT;
   rowAnnualMetrics[14] = m_fromBarneyYr_m3 / M3_PER_ACREFT;
   rowAnnualMetrics[15] = m_fromSpringHillYr_m3 / M3_PER_ACREFT;
   rowAnnualMetrics[16] = m_fromClackamasYr_m3 / M3_PER_ACREFT;
   rowAnnualMetrics[17] = m_fromLakeOswegoYr_m3 / M3_PER_ACREFT;
   this->m_annualMetrics.AppendRow(rowAnnualMetrics);

   float precip_yr_mm = (float)((precip_accumulator_m3 / tot_area) * 1000.f);
   float snow_evap_mm = (float)((snow_evap_accumulator_m3 / tot_area) * 1000.f);
   float rain_evap_mm = (float)((rain_evap_accumulator_m3 / tot_area) * 1000.f);
   float aquifer_recharge_yr_mm = (float)((aquifer_recharge_accumulator_m3 / tot_area) * 1000.f);
   float reach_evap_mm = (float)((pFlowContext->pFlowModel->m_totEvapFromReachesYr_m3 / tot_area) * 1000.f);
   float aet_yr_mm = reach_evap_mm + (float)((aet_accumulator_m3 / tot_area) * 1000.f);

   float groundwater_pumped_mm = (float)((groundwater_accumulator_ac_ft * M3_PER_ACREFT) / tot_area) * 1000.f;
   float from_outside_basin_mm = (float)((from_outside_basin_accumulator_m3 / tot_area) * 1000.f);

   double volume_added_by_flow_model_m3 = m_pFlowModel->MagicReachWaterReport_m3() + m_pFlowModel->MagicHRUwaterReport_m3();
   double volume_added_by_flow_model_mm = 1000 * (volume_added_by_flow_model_m3 / tot_area);

   float to_outside_basin_mm = (float)((m_toOutsideBasinYr_m3 / tot_area) * 1000.f);

   double basin_discharge_ac_ft = pFlowContext->pFlowModel->m_annualTotalDischarge;
   float tot_area_ac = ((float)tot_area / 10000.f) * ACRE_PER_HA;
   float basin_discharge_mm = (float)(basin_discharge_ac_ft / tot_area_ac) * M_PER_FT * 1000.f;

   float tot_H2O_last_year_mm = (float)((m_totH2OlastYr_m3 / tot_area) * 1000.);
   m_totH2OlastYr_m3 = pFlowContext->pFlowModel->CalcTotH2O();
   float tot_H2O_this_year_mm = (float)((m_totH2OlastYr_m3 / tot_area) * 1000.);

   CString msg;
   msg.Format("*** AltWaterMaster::EndYear() CalcTotH2OinReaches() = %f, CalcTotH2OinReservoirs() = %f, CalcTotH2OinHRUs = %f, tot_H2O_this_year_mm = %f",
      pFlowContext->pFlowModel->CalcTotH2OinReaches(), pFlowContext->pFlowModel->CalcTotH2OinReservoirs(), pFlowContext->pFlowModel->CalcTotH2OinHRUs(), tot_H2O_this_year_mm);
   if (tot_H2O_this_year_mm != tot_H2O_this_year_mm) Report::ErrorMsg(msg); // Nan!
   else Report::LogMsg(msg);

   float tot_input_mm = tot_H2O_last_year_mm + precip_yr_mm + groundwater_pumped_mm + from_outside_basin_mm;
   float tot_output_mm = to_outside_basin_mm + aet_yr_mm + snow_evap_mm + rain_evap_mm + basin_discharge_mm + tot_H2O_this_year_mm;
   float change_in_total_H2O_mm = tot_output_mm - tot_input_mm;
   float muni_ac_ft = (muni_accumulator_m3 + m_GWnoWR_Yr_m3) * ACREFT_PER_M3;

   CArray< float, float > rowQuickCheckMetrics;
   rowQuickCheckMetrics.SetSize(17);
   rowQuickCheckMetrics[0] = (float)time;
   rowQuickCheckMetrics[1] = tot_H2O_last_year_mm;
   rowQuickCheckMetrics[2] = precip_yr_mm;
   rowQuickCheckMetrics[3] = groundwater_pumped_mm;
   double tot_ground_water_to_reach_m3 = tot_ground_water_to_reach_cms * SEC_PER_DAY * pFlowContext->pEnvContext->daysInCurrentYear;
   rowQuickCheckMetrics[4] = (float)((tot_ground_water_to_reach_m3 / tot_area) * 1000.); // High Cascades groundwater contribution
   rowQuickCheckMetrics[5] = from_outside_basin_mm;
   rowQuickCheckMetrics[6] = (float)volume_added_by_flow_model_mm;
   double year_in_mm = 0.; for (int i = 1; i <= 6; i++) year_in_mm += rowQuickCheckMetrics[i];

   rowQuickCheckMetrics[7] = to_outside_basin_mm;
   rowQuickCheckMetrics[8] = aet_yr_mm;
   rowQuickCheckMetrics[9] = snow_evap_mm;
   rowQuickCheckMetrics[10] = basin_discharge_mm;
   rowQuickCheckMetrics[11] = tot_H2O_this_year_mm;
   double year_out_mm = 0.; for (int i = 7; i <= 11; i++) year_out_mm += rowQuickCheckMetrics[i];

   rowQuickCheckMetrics[12] = m_irrigatedWaterYr; // ac-ft
   rowQuickCheckMetrics[13] = muni_ac_ft;

   double discrepancy_mm = year_out_mm - year_in_mm;
   rowQuickCheckMetrics[14] = (float)discrepancy_mm;
   rowQuickCheckMetrics[15] = (float)(discrepancy_mm / year_in_mm);

   rowQuickCheckMetrics[16] = (float)(m_pEnvContext->weatherYear);

   this->m_QuickCheckMetrics.AppendRow(rowQuickCheckMetrics);

   CArray< float, float > rowMuniBackupWRuse;
   rowMuniBackupWRuse.SetSize(9);
   rowMuniBackupWRuse[0] = (float)time;
   rowMuniBackupWRuse[1] = m_ugaMuniBackupWRuse_acft[UGA_Metro];
   rowMuniBackupWRuse[2] = m_ugaMuniBackupWRuse_acft[UGA_EugeneSpringfield];
   rowMuniBackupWRuse[3] = m_ugaMuniBackupWRuse_acft[UGA_SalemKeizer];
   rowMuniBackupWRuse[4] = m_ugaMuniBackupWRuse_acft[UGA_Corvallis];
   rowMuniBackupWRuse[5] = m_ugaMuniBackupWRuse_acft[UGA_Albany];
   rowMuniBackupWRuse[6] = m_ugaMuniBackupWRuse_acft[UGA_McMinnville];
   rowMuniBackupWRuse[7] = m_ugaMuniBackupWRuse_acft[UGA_Newberg];
   rowMuniBackupWRuse[8] = m_ugaMuniBackupWRuse_acft[UGA_Woodburn];
   this->m_AnnualMuniBackupWRuse_acft.AppendRow(rowMuniBackupWRuse);

/*
	if ( m_debug )
		{
		// summary for all irrigation
		CArray< float, float > rowAnnualMetricsDebug;

		rowAnnualMetricsDebug.SetSize(12);
		rowAnnualMetricsDebug[0] = (float) time; // "Year"
		rowAnnualMetricsDebug[1] = m_anGTmaxDutyArea21;
		rowAnnualMetricsDebug[2] = m_anGTmaxDutyArea22;
		rowAnnualMetricsDebug[3] = m_anGTmaxDutyArea23;
		rowAnnualMetricsDebug[4] = m_anGTmaxDutyArea24;
		rowAnnualMetricsDebug[5] = m_anGTmaxDutyArea25;
		rowAnnualMetricsDebug[6] = m_anGTmaxDutyArea26;
		rowAnnualMetricsDebug[7] = m_anGTmaxDutyArea27;
		rowAnnualMetricsDebug[8] = m_anGTmaxDutyArea28;
		rowAnnualMetricsDebug[9] = m_anGTmaxDutyArea29;
		rowAnnualMetricsDebug[10]= (float) m_pastureIDUGTmaxDuty;
		rowAnnualMetricsDebug[11]= m_pastureIDUGTmaxDutyArea;
		this->m_annualMetricsDebug.AppendRow(rowAnnualMetricsDebug);

		//reset for next year
		m_anGTmaxDutyArea21 = 0.0f;
		m_anGTmaxDutyArea22 = 0.0f;
		m_anGTmaxDutyArea23 = 0.0f;
		m_anGTmaxDutyArea24 = 0.0f;
		m_anGTmaxDutyArea25 = 0.0f;
		m_anGTmaxDutyArea26 = 0.0f;
		m_anGTmaxDutyArea27 = 0.0f;
		m_anGTmaxDutyArea28 = 0.0f;
		m_anGTmaxDutyArea29 = 0.0f;
		}
*/

   if (pFlowContext->pEnvContext->startYear + pFlowContext->pEnvContext->yearOfRun + 1 == pFlowContext->pEnvContext->endYear)
      { // This is the end of the run.  
      // Set the input variables back to their default values.
      m_maxRate_cfs_per_ac = IRRIG_MAX_RATE_DEFAULT; 
      m_maxDuty_ft_per_yr = IRRIG_MAX_DUTY_DEFAULT;
      m_dynamicWRType = 0;
      m_regulationType = 0;

      if (m_pFlowModel->m_waterRightsReportInputVar > 0)
      {
         CString msg;
         if (WriteWRreport())
         {
            msg = "Water Rights report was written successfully.";
            Report::LogMsg(msg);
         }
         else
         {
            msg = "WriteWRreport() failed.";
            Report::WarningMsg(msg);
         }
      }
   }
  	return true;
   } // end of EndYear()


int AltWaterMaster::IDUatXY(double xCoord, double yCoord, MapLayer *pLayer)
{
   int num_IDUs;
   return(IDUatXY(xCoord, yCoord, pLayer, &num_IDUs, false));
} // end of IDUatXY(double xCoord, double yCoord, MapLayer *pLayer)


int AltWaterMaster::IDUatXY(double xCoord, double yCoord, MapLayer *pLayer, int * pNumIDUs)
{
   return(IDUatXY(xCoord, yCoord, pLayer, pNumIDUs, true));
} // end of IDUatXY(double xCoord, double yCoord, MapLayer *pLayer, int * pNumIDUs)


int AltWaterMaster::IDUatXY(double xCoord, double yCoord, MapLayer *pLayer, int * pNumIDUs, bool multipleFlag)
// Return the local index of the IDU containing the point (xCoord, yCoord), or -1 if the point is outside the active study area.
{
   int idu_index = -1;
   *pNumIDUs = 0;
   for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
   {
      Poly *pPoly = pLayer->GetPolygon(idu);
      Vertex v((REAL)xCoord, (REAL)yCoord);

      int idu_int = (int)idu; // Facilitates setting breakpoints on particular IDUs.

      if (pPoly->IsPointInPoly(v))
      {
         if (idu_index == -1) idu_index = idu_int;
         *pNumIDUs = *pNumIDUs + 1;
         if (!multipleFlag) break;
      }
   } // end of loop through IDU layer

   return(idu_index);
} // end of IDUatXY(double xCoord, double yCoord, MapLayer *pLayer, int * pNumIDUs, bool multipleFlag)


double AltWaterMaster::GetAvailableSourceFlow(Reach *pReach)
	{
	// this method calculates the amount of demand that can be extracted from this stream reach

	double flow = (pReach->GetDischarge()); // m3/day

	float instreamWRUse = (float)pReach->m_instreamWaterRightUse;

	if (flow > m_minFlow)
		return flow - m_minFlow - instreamWRUse;
	else
		return m_minFlow;
	}

int AltWaterMaster::LoadWRDatabase(FlowContext *pFlowContext)
   {

   MapLayer *pStreamLayer = (MapLayer*)pFlowContext->pFlowModel->m_pStreamLayer;

   MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;
   
   // UGA code, POCD easting, POCD northing, discharge %, UGA name   
   double POCDlocations[][4] = { // Ideally this would be read in from a .csv file or embedded in HBV.xml.
      // UGA code,POCD easting,POCD northing,discharge %
      // from POCD_May8.xlsx, from Sara Wynn
      { 1, 487787.0, 4942742.1, 100 }, //   UGA_AdairVillage
      { 2, 494276.2, 4943679.6, 100 }, //   UGA_Albany
      { 3, 483441.8, 4996285.8, 100 }, //   UGA_Amity
      { 4, 508934.6, 4966663.4, 100 }, //   UGA_Aumsville
      { 5, 519689.0, 5008309.4, 100 }, //   UGA_Aurora
      { 6, 500742.4, 5038514.2, 100 }, //   UGA_Banks
      //{7,,,100}, //   UGA_Barlow
      { 8, 499986.9, 4915826.9, 100 }, //   UGA_Brownsville
      { 9, 525334.8, 5014985.6, 100 }, //   UGA_Canby
      { 10, 485321.8, 5014611.9, 100 }, //   UGA_Carlton
      { 11, 495521.7, 4888757.0, 100 }, //   UGA_Coburg
      { 12, 479895.8, 4935724.9, 100 }, //   UGA_Corvallis
      { 13, 496058.4, 4850579.0, 100 }, //   UGA_CottageGrove
      { 14, 497045.0, 4863919.6, 100 }, //   UGA_Creswell
      { 15, 481071.6, 4975071.5, 100 }, //   UGA_Dallas
      { 16, 494489.0, 5007607.4, 100 }, //   UGA_Dayton
      //{17,,,100}, //   UGA_Detroit
      //{18,,,100}, //   UGA_Donald
      { 19, 500333.3, 5012413.8, 100 }, //   UGA_Dundee
      { 20, 550800.5, 5016499.8, 100 }, //   UGA_Estacada
      //{21,,,100 }, //   UGA_UnknownLaneCo
      { 22, 490529.7, 4881303.7, 100 }, //   UGA_EugeneSpringfield
      { 23, 467087.9, 4967167.2, 100 }, //   UGA_FallsCity
      { 24, 494126.3, 5037913.3, 100 }, //   UGA_ForestGrove
      { 25, 494126.3, 5037913.3, 100 }, //   UGA_Gaston
      //{26,,,100}, //   UGA_Gates
      { 27, 512715.3, 4994953.6, 100 }, //   UGA_Gervais
      { 28, 488938.3, 4914573.5, 100 }, //   UGA_Halsey
      { 29, 484496.8, 4904461.6, 100 }, //   UGA_Harrisburg
      { 30, 514607.8, 5003592.9, 100 }, //   UGA_Hubbard
      //{31,,,100}, //   UGA_Idanha
      { 32, 486074.0, 4967673.9, 100 }, //   UGA_Independence
      { 33, 498634.9, 4952214.5, 100 }, //   UGA_Jefferson
      { 34, 481484.3, 4896718.2, 100 }, //   UGA_JunctionCity
      { 35, 491124.1, 5009475.0, 100 }, //   UGA_Lafayette
      { 36, 508974.7, 4930334.5, 100 }, //   UGA_Lebanon
      { 37, 515669.9, 4863471.5, 100 }, //   UGA_Lowell
      //{38,,,100}, //   UGA_Lyons
      { 39, 488717.2, 5006919.3, 100 }, //   UGA_McMinnville
      { 40, 523914.7, 5052460.0, -50.95541401 }, //   UGA_Metro on the Columbia, negative % means outside the basin
      { 40, 526775.1, 5029967.2, 5.874026893 }, //
      { 40, 518822.9, 5027401.3, 18.40056617 }, //
      { 40, 504027.0, 5037944.2, 24.76999292 }, //
      { 40, 500742.4, 5038514.2, 0 }, //
      { 40, 494126.3, 5037913.3, 0 }, //
      //{41,,,100}, //   UGA_MillCity
      { 42, 494276.2, 4943679.6, 100 }, //   UGA_Millersburg
      { 43, 536450.3, 4997734.2, 100 }, //   UGA_Molalla
      { 44, 486074.0, 4967673.9, 100 }, //   UGA_Monmouth
      { 45, 476526.2, 4907573.9, 100 }, //   UGA_Monroe
      { 46, 513521.0, 4990531.6, 100 }, //   UGA_MtAngel
      { 47, 503080.2, 5014432.6, 100 }, //   UGA_Newberg
      { 48, 500742.4, 5038514.2, 100 }, //   UGA_NorthPlains
      { 49, 541187.0, 4843390.0, 100 }, //   UGA_Oakridge
      //{50,,,100}, ////   UGA_Philomath
      { 51, 494417.3, 4983360.1, 100 }, //   UGA_SalemKeizer
      { 52, 552286.9, 5028129.5, 100 }, //   UGA_Sandy
      //{53,,,100}, ////   UGA_
      //{54,,,100}, ////   UGA_
      { 55, 511022.3, 4949787.1, 100 }, //   UGA_Scio
      //{56,,,100}, //   UGA_ScottsMills
      { 57, 470099.8, 4993712.7, 100 }, //   UGA_Sheridan
      { 58, 515483.3, 4983910.3, 100 }, //   UGA_Silverton
      //{59,,,100}, //   UGA_Sodaville
      //{60,,,100}, ////   UGA_
      //{61,,,100}, //   UGA_StPaul
      { 62, 514999.4, 4959117.9, 100 }, //   UGA_Stayton
      //{63,,,100}, ////   UGA_Sublimity
      { 64, 520894.9, 4916456.2, 100 }, //   UGA_SweetHome
      { 65, 488367.7, 4933247.4, 100 }, //   UGA_Tangent
      { 66, 494417.3, 4983360.1, 100 }, //   UGA_Turner
      { 67, 470198.5, 4877745.4, 100 }, //   UGA_Veneta
      //{68,,,100}, //   UGA_Waterloo
      { 69, 538526.1, 4845190.3, 100 }, //   UGA_Westfir
      { 70, 462397.2, 4991494.2, 100 }, //   UGA_Willamina
      { 71, 515353.9, 4999685.8, 100 }, //   UGA_Woodburn
      { 72, 485063.1, 5018447.2, 100 }, //   UGA_Yamhill
      };
   m_POCDs.RemoveAll();
   int numPOCDs = sizeof(POCDlocations) / (4 * sizeof(double));
      {
      CString msg;
      msg.Format("AltWM::LoadWRDatabase() numPOCDs = %d", numPOCDs);
      Report::LogMsg(msg);
      }

   int num_pocds_outside_basin = 0;
   int expected_num_pocds_outside_basin = 0;
   for (int pocd_ndx = 0; pocd_ndx < numPOCDs; pocd_ndx++)
      {
      PtOfCentralDischarge * pPOCD = new PtOfCentralDischarge;
      pPOCD->uga = (int)POCDlocations[pocd_ndx][0];
      pPOCD->ugaDischargePct = (float)fabs(POCDlocations[pocd_ndx][3]);
      if (POCDlocations[pocd_ndx][3] < 0.f)
         {
         pPOCD->outsideBasin_flag = true;
         expected_num_pocds_outside_basin++;
         }
      else pPOCD->outsideBasin_flag = false;
      int reachComID = -99;
      pPOCD->reachPolyIndex = -1;
      int idu = -1;

      if (!pPOCD->outsideBasin_flag)
         { 
         double xCoord = POCDlocations[pocd_ndx][1];
         double yCoord = POCDlocations[pocd_ndx][2];
         idu = IDUatXY(xCoord, yCoord, pLayer);
         if (idu < 0) 
            pPOCD->outsideBasin_flag = true;

         if (!pPOCD->outsideBasin_flag)
            { // Even if the POCD is not outside the whole WRB, it might still be outside the currently active subset of the WRB.
            Vertex v((float)xCoord, (float)yCoord);
            Poly * pPoly = pLayer->GetPolygon(idu);
            if (!(pPoly->IsPointInPoly(v)))
               pPOCD->outsideBasin_flag = true; 
            }

         if (!pPOCD->outsideBasin_flag)
            { 
            pLayer->GetData(idu, m_colCOMID, reachComID);
            pLayer->SetDataU(idu, m_colUGAPOCD, (VData)pPOCD->uga);
            }

         }

      if (pPOCD->outsideBasin_flag) num_pocds_outside_basin++;
      
      if (!pPOCD->outsideBasin_flag && reachComID > 0) 
         { 
         pPOCD->reachComID = reachComID;
         pPOCD->reachPolyIndex = pStreamLayer->FindIndex(m_colStreamCOMID, reachComID);
         }

      if (!pPOCD->outsideBasin_flag && (reachComID <= 0 || pPOCD->reachPolyIndex < 0))
         {
         pPOCD->reachComID = -99;
         pPOCD->reachPolyIndex = -99;
         CString msg; msg.Format("*** LoadWRDatabase() Unable to find the COMID or reachPolyIndex for pocd_ndx = %d, pPOCD->uga = %d, xCoord = %f, yCoord = %f; setting pPOCD->outsideBasin_flag now.",
               pocd_ndx, pPOCD->uga, POCDlocations[pocd_ndx][1], POCDlocations[pocd_ndx][2]);
         pPOCD->outsideBasin_flag = true;
         Report::WarningMsg(msg);
         }

      m_POCDs.Add(pPOCD);
      } // end of loop thru POCDs

   if (num_pocds_outside_basin != expected_num_pocds_outside_basin)
      {
      CString msg; 
      msg.Format("*** LoadWRDatabase() num_pocds_outside_basin = %d, while expected_num_pocds_outside_basin = %d.  This could be due to simulating only a portion of the study area.",
         num_pocds_outside_basin, expected_num_pocds_outside_basin);
      Report::LogMsg(msg); 
      }

   // *************** Begin loading Point of Diversion input file ****************************************

	int podRecords = m_podDb.ReadAscii(m_podTablePath); // Pre-sorted data file. sorted by PriorityYear, PriorityMonth, PriorityDOY, and BeginDOY respectively

	if (podRecords == 0)
		{
		CString msg;
		msg.Format("AltWM::LoadWRDatabase could not load Point of Diversion .csv file \n");
		Report::ErrorMsg(msg);
		return 0;
		}

   // http://www.oregon.gov/owrd/pages/wr/wrisuse.aspx
	m_colWRID = m_podDb.GetCol("WATERRIGHTID");
	m_colXcoord = m_podDb.GetCol("x");
	m_colYcoord = m_podDb.GetCol("y");
	m_colPodID = m_podDb.GetCol("PODID");
	m_colPDPouID = m_podDb.GetCol("POUID");
	m_colPermitCode = m_podDb.GetCol("PERMITCODE");
	m_colPodRate = m_podDb.GetCol("PODRATE");
	m_colUseCode = m_podDb.GetCol("USECODE");
	m_colPriorDoy = m_podDb.GetCol("PRIORITYDOY");
	m_colPriorYr = m_podDb.GetCol("YEAR");
	m_colBeginDoy = m_podDb.GetCol("BEGINDOY");
	m_colEndDoy = m_podDb.GetCol("ENDDOY");
	m_colReachComid = m_podDb.GetCol("REACHCOMID");
   int colPodLENGTH_OR_COMID = m_podDb.GetCol("LENGTH_OR_COMID");
   int colPodSPECIAL = m_podDb.GetCol("SPECIAL");

	if ( m_colXcoord   < 0 || m_colPermitCode < 0 || m_colPriorDoy < 0 || m_colWRID        < 0 ||
		  m_colYcoord   < 0 || m_colPodRate    < 0 || m_colPriorYr  < 0 || m_colPDPouID     < 0 ||
		  m_colPodID    < 0 || m_colUseCode    < 0 || m_colBeginDoy < 0 || 
        m_colPodRate     < 0 || colPodLENGTH_OR_COMID < 0 || colPodSPECIAL < 0)
		{
		CString msg;
		msg.Format("AltWM::LoadWRDatabase: One or more column headings are incorrect in Point of Diversion data input file\n");
		Report::ErrorMsg(msg);
      msg.Format("m_podTablePath = %s\n"
         "m_colXcoord = %d, m_colPermitCode = %d, m_colPriorDoy = %d, m_colWRID = %d\n"
         "m_colYcoord = %d, m_colPodRate = %d, m_colPriorYr = %d, m_colPDPouID = %d\n"
         "m_colPodID = %d, m_colUseCode = %d, m_colBeginDoy = %d, m_colEndDoy = %d\n"
         "m_colPodRate = %d, colPodLENGTH_OR_COMID = %d, colPodSPECIAL = %d",
         m_podTablePath,
         m_colXcoord, m_colPermitCode, m_colPriorDoy, m_colWRID,
         m_colYcoord, m_colPodRate, m_colPriorYr, m_colPDPouID,
         m_colPodID, m_colUseCode, m_colBeginDoy, m_colEndDoy,
         m_colPodRate, colPodLENGTH_OR_COMID, colPodSPECIAL);
      Report::InfoMsg(msg);
		return 0;
		}

  int notFound_count = 0;
   int i_ary = -1;
   int i_prev_rec = -1;
	for (int i_rec = 0; i_rec < podRecords; i_rec++)
		{
      int wrID = m_podDb.GetAsInt(m_colWRID, i_rec);
      if (wrID < 0) continue; // Skip over placeholder entries in the POD file.
      float podRate_cfs = m_podDb.GetAsFloat(m_colPodRate, i_rec); // cubic feet per second
      // if (podRate_cfs <= 0.f) continue; // Skip over PODs with zero rate.

      WaterRight *pRight = new WaterRight;

		// set up water right from table
		pRight->m_wrID = wrID;
		pRight->m_xCoord = m_podDb.GetAsDouble(m_colXcoord, i_rec);
		pRight->m_yCoord = m_podDb.GetAsDouble(m_colYcoord, i_rec);
		pRight->m_podID = m_podDb.GetAsInt(m_colPodID, i_rec);
		pRight->m_pouID = m_podDb.GetAsInt(m_colPDPouID, i_rec);
		pRight->m_appCode = -99; // m_podDb.GetAsInt(m_colAppCode, i_rec); 
		pRight->m_permitCode = (WR_PERMIT)m_podDb.GetAsInt(m_colPermitCode, i_rec);
		pRight->m_podRate_cfs = podRate_cfs; // cubic feet per second
		pRight->m_useCode = (WR_USE)m_podDb.GetAsInt(m_colUseCode, i_rec);
		pRight->m_supp = -99; // m_podDb.GetAsInt(m_colSupp, i_rec); 
		pRight->m_priorDoy = m_podDb.GetAsInt(m_colPriorDoy, i_rec);
		pRight->m_priorYr = m_podDb.GetAsInt(m_colPriorYr, i_rec);
		pRight->m_beginDoy = m_podDb.GetAsInt(m_colBeginDoy, i_rec);
		pRight->m_endDoy = m_podDb.GetAsInt(m_colEndDoy, i_rec);
      if (pRight->m_useCode == WRU_IRRIGATION)
         {
         pRight->m_beginDoy = m_irrDefaultBeginDoy; // 60; // March 1st, where Jan 1 = 1 and no leap year
         pRight->m_endDoy = m_irrDefaultEndDoy; // 304; // Oct 31st, where Jan 1 = 1 and no leap year
         }
		int comID = m_podDb.GetAsInt(m_colReachComid, i_rec);
/*
      // For surface irrigation and muni water rights, test whether the comID is consistent with the X,Y coordinates.
      if (comID > 0 && (pRight->m_useCode == WRU_IRRIGATION || pRight->m_useCode == WRU_MUNICIPAL) && pRight->m_permitCode == WRP_SURFACE)
         {
         int idu = IDUatXY(pRight->m_xCoord, pRight->m_yCoord, pLayer);
         int IDUcomid = -99;
         if (idu >= 0) pLayer->GetData(idu, m_colCOMID, IDUcomid);
         int reachX = 0;
         int reachY = 0;
         int IDUreachX = 0;
         int IDUreachY = 0;
         if (idu<0 || comID!=IDUcomid)
            {
            float distance = -99.f;
            int streamIndex = pStreamLayer->FindIndex(m_colStreamCOMID, comID);
            pStreamLayer->GetData(streamIndex, m_colStreamSTRMVERT0X, reachX);
            pStreamLayer->GetData(streamIndex, m_colStreamSTRMVERT0Y, reachY);
            float dx = (float)pRight->m_xCoord - reachX;
            float dy = (float)pRight->m_yCoord - reachY;
            distance = sqrt(dx*dx + dy*dy);
            float IDUcomidDistance = distance;
            if (idu >= 0)
               {
               int IDUstreamIndex = pStreamLayer->FindIndex(m_colStreamCOMID, IDUcomid);
               pStreamLayer->GetData(IDUstreamIndex, m_colStreamSTRMVERT0X, IDUreachX);
               pStreamLayer->GetData(IDUstreamIndex, m_colStreamSTRMVERT0Y, IDUreachY);
               dx = (float)pRight->m_xCoord - IDUreachX;
               dy = (float)pRight->m_yCoord - IDUreachY;
               IDUcomidDistance = sqrt(dx*dx + dy*dy);
               }
            
            float diff = IDUcomidDistance - distance;
            if (diff < -500.f && pRight->m_useCode == WRU_MUNICIPAL)
               {
               CString msg;
               msg.Format("*** LoadWRDatabase(): m_wrID = %d, %s, m_xCoord = %f, m_yCoord = %f, m_podID = %d, m_pouID = %d, "
                  "comID = %d, distance = %d, %d, %f, idu = %d, IDUcomid = %d, IDUcomidDistance = %d, %d, %f, diff = %f",
                  pRight->m_wrID, pRight->m_useCode == WRU_MUNICIPAL ? "muni" : (pRight->m_useCode == WRU_IRRIGATION ? "irrig" : "neither muni nor irrig"), 
                  pRight->m_xCoord, pRight->m_yCoord, pRight->m_podID, pRight->m_pouID, comID, reachX, reachY, distance,
                  idu, IDUcomid, IDUreachX, IDUreachY, IDUcomidDistance, diff);
               Report::LogMsg(msg);
               } // end of if (diff < -100.f)

            } // end of if (idu<0 || comID!=IDUcomid)
         } // end of if (comID > 0)
*/
      pRight->m_reachComid = comID;

      pRight->m_streamIndex = comID >= 0 ? pStreamLayer->FindIndex(m_colStreamCOMID, comID) : -99;
      
      pRight->m_instreamWRlength_m = 0.f;
      pRight->m_downstreamComid = -99;
      if (pRight->m_useCode == WRU_INSTREAM)
         {
         float instreamWRlength_ft = m_podDb.GetAsFloat(colPodLENGTH_OR_COMID, i_rec);
         if (instreamWRlength_ft < 1.e7f) pRight->m_instreamWRlength_m = instreamWRlength_ft * M_PER_FT; // Interpret as length.
         else pRight->m_downstreamComid = m_podDb.GetAsInt(colPodLENGTH_OR_COMID, i_rec); // Interpret as downstream COMID.
         } // end of if (pRight->m_useCode == WRU_INSTREAM)

      pRight->m_specialCode = (WR_SPECIALCODE)m_podDb.GetAsInt(colPodSPECIAL, i_rec);
      pRight->m_podStatus = WRPS_UNKNOWN;

      if (pRight->m_pouID < 0) 
         { // The "POU" for this WR is a UGA or this is an instream water right.
         if (pRight->m_useCode == WRU_INSTREAM)
            { // This is an instream water right.  Set the INSTRM_WR and INSTRMWRID attributes for the affected reach in the stream layer.
            if (pRight->m_reachComid < 0)
               { // This is an instream water right without an associated reach; if possible, fill in m_reachComid from the COMID of the enclosing IDU.
               int idu = IDUatXY(pRight->m_xCoord, pRight->m_yCoord, pLayer);
               if (idu >= 0)
                  {  
                  int comID = -99; pLayer->GetData(idu, m_colCOMID, comID);
                  pRight->m_reachComid = comID;
                  pRight->m_streamIndex = comID > 0 ? pStreamLayer->FindIndex(m_colStreamCOMID, pRight->m_reachComid) : -99;
                  } // end of if (idu>=0)
               else pRight->m_streamIndex = -99;
               } // end of (pRight->m_reachComid < 0) 
/*
            if (pRight->m_reachComid == -99)
               { // The reachComid is missing; maybe this POD is outside the study area, or at least outside the subset of the study area now being simulated.
               CString msg;
               msg.Format("*** LoadWRDatabase(): This instream water right seems to be outside the basin. pRight->m_wrID = %d, pRight->m_podID = %d", pRight->m_wrID, pRight->m_podID);
               Report::LogMsg(msg);
               }
*/
            if (pRight->m_reachComid >= 0)
               { // Set the INSTRM_WR and INSTRMWRID attributes for the affected reaches in the stream layer.
               int attribute_bitval;
               switch (pRight->m_specialCode)
                  {
                  case WRSC_NEWINSTREAM: attribute_bitval = 2; break;
                  case WRSC_NOT_NEWINSTREAM_ONLY: attribute_bitval = 4; break;
                  default: attribute_bitval = 1; break;
                  } // end of switch (pRight->m_specialCode)
               int reachPolyNdx = pRight->m_streamIndex;
               Reach * pReach = pRight->m_pReach = pFlowContext->pFlowModel->FindReachFromPolyIndex(reachPolyNdx);
               float remaining_length = pRight->m_instreamWRlength_m;
               float calculated_length = 0.f;
               pStreamLayer->m_readOnly = false;
               int finalComid = pRight->m_reachComid;
               int num_reaches = 0;
               while (pReach != NULL)
                  {
                  num_reaches++;
                  remaining_length -= pReach->m_length;
                  calculated_length += pReach->m_length;
                  int instrm_wr = 0;  pStreamLayer->GetData(reachPolyNdx, m_colStreamINSTRM_WR, instrm_wr);
                  /*
                  if (instrm_wr!=0)
                     { // We've already encountered an instream water right for this reach. 
                     int first_WRid = 0; pStreamLayer->GetData(reachNdx, m_colStreamINSTRMWRID, first_WRid);
                     if ((first_WRid != pRight->m_wrID) && ((instrm_wr & attribute_bitval)!=0))
                        {
                        CString msg;
                        msg.Format("*** More than one instream water right applies to reach %d. first_WRid = %d, first instrm_wr = %d, "
                              "second WRid = %d, second attribute_bitval = %d",
                              pRight->m_reachComid, first_WRid, instrm_wr, pRight->m_wrID, attribute_bitval);
                        Report::WarningMsg(msg);
                        }
                     }
                  */
                  instrm_wr |= attribute_bitval;
                  pStreamLayer->SetData(reachPolyNdx, m_colStreamINSTRM_WR, instrm_wr);
                  pStreamLayer->SetData(reachPolyNdx, m_colStreamINSTRMWRID, pRight->m_wrID);

                  if (pReach->m_pDown == NULL || pRight->m_downstreamComid == pReach->m_reachID || (pRight->m_downstreamComid == -99 && remaining_length <= 0.f))
                     pReach = NULL; // Stop moving downstream.
                  else
                     { // Move to the next downstream reach.
                     pReach = m_pFlowModel->GetReachFromNode(pReach->m_pDown);
                     reachPolyNdx = pReach->m_polyIndex;
                     finalComid = pReach->m_reachID;
                     } // end of if ... else move to the next downstream reach
                  } // end of while (pReach != NULL)
               /*
               CString msg; msg.Format("Instream WR %d ( %d day %d ) runs for %d reach(es), from reach %d to reach %d , a stream length of %f m, WR_INSTRM = %d ",
                  pRight->m_wrID, pRight->m_priorYr, pRight->m_priorDoy, num_reaches, pRight->m_reachComid, finalComid, 
                  calculated_length, attribute_bitval);
               Report::LogMsg(msg);
               */
               pStreamLayer->m_readOnly = true;
               } // end of if (pRight->m_reachComid >= 0)
            } // end of logic for an instream water right 

         else if (pRight->m_useCode == WRU_MUNICIPAL)
            { // The "POU" for this WR is a UGA
            int uga = -pRight->m_pouID;
            int idu = IDUatXY(pRight->m_xCoord, pRight->m_yCoord, pLayer);
            if (idu >= 0)
               {
               pLayer->SetDataU(idu, m_colUGAMUNIPOD, uga); // Set UGAMUNIPOD attribute = UGB
               if (pRight->m_reachComid == -99)
                  {
                  int comID = -99; pLayer->GetData(idu, m_colCOMID, comID);
                  pRight->m_reachComid = comID;
                  pRight->m_streamIndex = comID > 0 ? pStreamLayer->FindIndex(m_colStreamCOMID, pRight->m_reachComid) : -99;
                  }  // end of if (pRight->m_reachComid == -99)
               } // end of if (idu >= 0)
            else 
               {
               pRight->m_reachComid = -99;
               pRight->m_streamIndex = -99;
               } // end of if (idu >= 0) ... else
/*
            if (pRight->m_reachComid < 0)
               { // This POD is outside the study area, or at least outside the portion of the study area currently being simulated.
               CString msg;
               msg.Format("*** LoadWRDatabase(): This point of diversion is outside the basin. pRight->m_wrID = %d, pRight->m_podID = %d", pRight->m_wrID, pRight->m_podID);
               Report::LogMsg(msg);
               }
*/
            } // end of logic for m_useCode == WRU_MUNICIPAL
         } // end of POU-is-UGA and instream WR logic
      
		int reachID = pRight->m_reachComid;     // unique reach identifier stored in WR table, use COMID

		if (i_rec < (podRecords - 1))
			{
			int nextWRID = m_podDb.GetAsInt(m_colWRID, i_rec + 1);
			int nextPODID = m_podDb.GetAsInt(m_colPodID, i_rec + 1);
			int nextPOUID = m_podDb.GetAsInt(m_colPDPouID, i_rec + 1);
			int nextUse = m_podDb.GetAsInt(m_colUseCode, i_rec + 1);

			// count how many PODs per Water Right, per POU, per use. The default is 1
			if (pRight->m_wrID == nextWRID &&
				pRight->m_pouID == nextPOUID &&
				pRight->m_useCode == nextUse &&
				pRight->m_podID != nextPODID)
				{
				pRight->m_nPODSperWR++;
				}
			}

		// find corresponding reach.  Note that reachID < 0 is Point of Diversion out of Basin
      bool foundReach_flag = false;
      pRight->m_pReach = NULL;
      if (reachID < 0) foundReach_flag = true;
      else 
			{
         int polyIndex = pRight->m_streamIndex;
         if (polyIndex < 0) polyIndex = pRight->m_streamIndex = pStreamLayer->FindIndex(m_colStreamCOMID, reachID);
         if (polyIndex < 0) notFound_count++;
			else
				{ // found reach in stream layer, now find reach object
			   pRight->m_pReach = pFlowContext->pFlowModel->FindReachFromPolyIndex(polyIndex);
            if (pRight->m_pReach != NULL) foundReach_flag = true;
            else notFound_count++;
            }
			}
		
      // If we found the reach and it is in the basin, or if not in the basin it is not an instream water right, add to our array.
      if (foundReach_flag && (reachID >= 0 || pRight->m_useCode != WRU_INSTREAM))
      {
         m_podArray.Add(pRight);
         i_ary++;
      }

      // Check that POD file is in sorted order.
      if (i_ary > 0)
         {
         int yr, yrPrev, doy, doyPrev, begin_doy, begin_doyPrev, end_doy, end_doyPrev;
         yr = m_podArray[i_ary]->m_priorYr; yrPrev = m_podArray[i_ary - 1]->m_priorYr;
         doy = m_podArray[i_ary]->m_priorDoy ; doyPrev = m_podArray[i_ary - 1]->m_priorDoy;
         begin_doy = m_podArray[i_ary]->m_beginDoy ; begin_doyPrev = m_podArray[i_ary - 1]->m_beginDoy;
         end_doy = m_podArray[i_ary]->m_endDoy; end_doyPrev = m_podArray[i_ary - 1]->m_endDoy;
         if (!(yr > yrPrev
            || (yr == yrPrev && doy >= doyPrev)))
            {
            CString msg;
            msg.Format("AltWM::LoadWRDatabase: PODs out of order in input file %s at record #s %d and %d. yr = %d, yrPrev = %d, doy = %d, doyPrev = %d, "
               "begin_doy = %d, begin_doyPrev=%d, end_doy = %d, end_doyPrev = %d", 
               m_podTablePath, i_rec, i_prev_rec, yr, yrPrev, doy, doyPrev, begin_doy, begin_doyPrev, end_doy, end_doyPrev);
            Report::WarningMsg(msg);
            }
         }

      if (foundReach_flag) i_prev_rec = i_rec;

		} // end of loop thru the records in the POD data file

   if (notFound_count > 0)
      {
      CString msg;
      msg.Format("AltWaterMaster::LoadWRDatabase() There were %d records in %s for which the reach ComID was not found in the stream layer.", 
         notFound_count, m_podTablePath);
      Report::LogMsg(msg);
      }



	// *************** End loading Point of Diversion input file ****************************************

	// *************** Begin loading Place of Use input file ****************************************

	int pouDBNdx;

	int pouRecords = m_pouDb.ReadAscii(m_pouTablePath); // Point of Use (POU) data input file

	if (pouRecords == 0)
		{
		CString msg;
		msg.Format("AltWM::LoadWRDatabase could not load Point of Use .csv file \n");
		Report::ErrorMsg(msg);
		return 0;
		}

	m_colPouPOU_ID = m_pouDb.GetCol("POU_ID");        // WR POU ID column number in POU input data file
	m_colPouIDU_ID = m_pouDb.GetCol("IDU_ID");    // Relates to the IDU_INDEX attribute in the IDU layer
	m_colPouPERCENT_POU = m_pouDb.GetCol("PERCENT_POU");      // The areal percentage of the POU, for a POUID, that over laps the IDU/IDU_INDEX
	m_colPouDBNdx = m_pouDb.GetCol("POU_INDEX");    // a zero based index for the POU input data file itself
    m_colPouPERCENT_IDU = m_pouDb.GetCol("PERCENT_IDU");
	m_colPouUSECODE = m_pouDb.GetCol("USECODE");
	m_colPouPERMITCODE = m_pouDb.GetCol("PERMITCODE");

  if (m_colPouPOU_ID   < 0 || m_colPouIDU_ID < 0 || m_colPouPERCENT_POU < 0 || m_colPouDBNdx < 0 || 
       m_colPouPERCENT_IDU, m_colPouUSECODE < 0 || m_colPouPERMITCODE < 0)
	{
	CString msg;
	msg.Format("AltWM::LoadWRDatabase: One or more column headings are incorrect in Point of Use data input file\n");
	Report::ErrorMsg(msg);
	return 0;
	}

   for (int i = 0; i < pouRecords; i++)
      {

      //Build the Key for the map lookup
      m_pouDb.Get(m_colPouPOU_ID, i, m_pouInsertKey.pouID);

      //Result from the Map lookup
      m_pouDb.Get(m_colPouDBNdx, i, pouDBNdx);

      //Build the Map
      m_pouInputMap[m_pouInsertKey].push_back(pouDBNdx);

      }
      // *************** End loading Place of Use input file ****************************************

   int nIDUs = IDUIndexLookupMap(pFlowContext); // Create an IDU index map, used for sub basins and partial IDU layer loading.
   {
      CString msg;
      msg.Format("AltWM::LoadWRDatabase() nIDUs = %d", nIDUs);
      Report::LogMsg(msg);
   }

   if (m_pEnvContext->coldStartFlag)
   {
      CalculateWRattributes(pFlowContext, pLayer, pStreamLayer, pouRecords);

      CString fileName = "wr_pous.csv";
      FILE * oFile = NULL;

      int colPouPOU_ID = m_pouDb.GetCol("POU_ID");
      int colPouIDU_ID = m_pouDb.GetCol("IDU_ID");
      int colPouWRIS_ID = m_pouDb.GetCol("WRIS_ID");
      int colPouAREA_POU = m_pouDb.GetCol("AREA_POU");
      int colPouPERCENT_POU = m_pouDb.GetCol("PERCENT_POU");
      int colPouAREA_IDU = m_pouDb.GetCol("AREA_IDU");
      int colPouPERCENT_IDU = m_pouDb.GetCol("PERCENT_IDU");
      int colPouUSECODE = m_pouDb.GetCol("USECODE");
      int colPouPERMITCODE = m_pouDb.GetCol("PERMITCODE");
      int colPouXCOORD = m_pouDb.GetCol("XCOORD");
      int colPouYCOORD = m_pouDb.GetCol("YCOORD");
      int colPouCERTIFICATE = m_pouDb.GetCol("CERTIFICATE");

      int colPouIDU_WW2100 = m_pouDb.GetCol("IDU_WW2100");
      int colIduIDU_WW2100 = colPouIDU_WW2100 >= 0 ? m_pIDUlayer->GetFieldCol("IDU_WW2100") : -99;

      if (OpenFileInDocumentsFolder(fileName, &oFile))
      { // Write out a copy of the POUs file with the coordinates filled in.
         fprintf(oFile, "POU_INDEX, POU_ID, IDU_ID, IDU_WW2100, WRIS_ID, AREA_POU, PERCENT_POU, AREA_IDU, PERCENT_IDU, USECODE, PERMITCODE, XCOORD, YCOORD, CERTIFICATE\n");

         int num_of_outside_centroids = 0;
         int num_of_missing_idu_ids = 0;
         for (int pou_index = 0; pou_index < pouRecords; pou_index++)
         {
            int pou_id, idu_id, wris_id, usecode, permitcode, idu_ww2100;
            float area_pou, percent_pou, area_idu, percent_idu;
            double xcoord, ycoord;
            CString certificate;
            pou_id = colPouPOU_ID >= 0 ? m_pouDb.GetAsInt(colPouPOU_ID, pou_index) : -99;
            idu_id = colPouIDU_ID >= 0 ? m_pouDb.GetAsInt(colPouIDU_ID, pou_index) : -99;
            wris_id = colPouWRIS_ID >= 0 ? m_pouDb.GetAsInt(colPouWRIS_ID, pou_index) : -99;
            area_pou = colPouAREA_POU >= 0 ? m_pouDb.GetAsFloat(colPouAREA_POU, pou_index) : -99;
            percent_pou = colPouPERCENT_POU >= 0 ? m_pouDb.GetAsFloat(colPouPERCENT_POU, pou_index) : -99;
            area_idu = colPouAREA_IDU >= 0 ? m_pouDb.GetAsFloat(colPouAREA_IDU, pou_index) : -99;
            percent_idu = colPouPERCENT_IDU >= 0 ? m_pouDb.GetAsFloat(colPouPERCENT_IDU, pou_index) : -99;
            usecode = colPouUSECODE >= 0 ? m_pouDb.GetAsInt(colPouUSECODE, pou_index) : -99;
            permitcode = colPouPERMITCODE >= 0 ? m_pouDb.GetAsInt(colPouPERMITCODE, pou_index) : -99;
            xcoord = colPouXCOORD >= 0 ? m_pouDb.GetAsDouble(colPouXCOORD, pou_index) : -99;
            ycoord = colPouYCOORD >= 0 ? m_pouDb.GetAsDouble(colPouYCOORD, pou_index) : -99;
            idu_ww2100 = colPouIDU_WW2100 >= 0 ? m_pouDb.GetAsInt(colPouIDU_WW2100, pou_index) : -99;
            certificate = colPouCERTIFICATE >= 0 ? m_pouDb.GetAsString(colPouCERTIFICATE, pou_index) : "-99";

            int idu_index = idu_id >=0 ? m_pIDUlayer->FindIndex(m_colIDU_ID, idu_id) : -99;

            // If the IDU_ID is missing, try to fill it in using the WW2100 IDU_ID value.
            if (idu_id == -99 && idu_ww2100 >= 0 && colIduIDU_WW2100 >= 0)
            { 
               idu_index = m_pIDUlayer->FindIndex(colIduIDU_WW2100, idu_ww2100);
               m_pIDUlayer->GetData(idu_index, m_colIDU_ID, idu_id);
            }

            if (idu_id < 0) num_of_missing_idu_ids++;
            else if (xcoord == -99 && ycoord == -99)
            { // Fill in xcoord and ycoord from the centroid of the IDU.
               ASSERT(idu_index >= 0);
               Poly * pPoly = m_pIDUlayer->GetPolygon(idu_index);
               Vertex v = pPoly->GetCentroid();
               xcoord = v.x;
               ycoord = v.y;
               if (!(pPoly->IsPointInPoly(v))) num_of_outside_centroids++;
            }

            /* This next bit doesn't work because about one out of 8 centroids lie outside the polygon.
            This can happen when the polygon is concave.
            if (idu_id == -99)
            { // Fill in idu_id from xcoord and ycoord
               int num_idus_containing_pt = 0;
               int idu_index_at_xy = IDUatXY(xcoord, ycoord, m_pIDUlayer, &num_idus_containing_pt);
               m_pIDUlayer->GetData(idu_index_at_xy, m_colIDU_ID, idu_id);
               ASSERT(idu_id >= 0);
               if (num_idus_containing_pt > 1) num_of_pts_in_multiple_idus++;
            }
            */

            fprintf(oFile, "%d, %d, %d, %d, %d, %f, %f, %f, %f, %d, %d, %f, %f, %s\n",
               pou_index, pou_id, idu_id, idu_ww2100, wris_id, area_pou, percent_pou, area_idu, percent_idu, usecode, permitcode, xcoord, ycoord, certificate.GetString());
         } // end of loop thru the records in the wr_pous.csv file

         fclose(oFile); oFile = NULL;

         CString msg;
         msg.Format("LoadWRDatabase() pouRecords = %d, num_of_outside_centroids = %d, num_of_missing_idu_ids = %d",
            pouRecords, num_of_outside_centroids, num_of_missing_idu_ids);
         if (num_of_missing_idu_ids > 0) Report::WarningMsg(msg);
         else Report::LogMsg(msg);
      } // end of block to write out wr_pous.csv
   } // end of if (m_pEnvContext->coldStartFlag)

	return podRecords;
} // end of LoadWRDatabase()

 
bool AltWaterMaster::OpenFileInDocumentsFolder(CString fileName, FILE ** pOfile)
{
   bool rtnFlag = true;

   // Get the path to the current user's Documents folder.
   PWSTR userPath;
   SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &userPath);
   char szBuffer[255];
   WideCharToMultiByte(CP_ACP, 0, userPath, -1, szBuffer, sizeof(szBuffer), NULL, NULL);

   // Set the string "path_and_filename" equal to Users\<username>\Documents\<fileName>
   CString path_and_filename;
   path_and_filename.Format("%s\\%s", szBuffer, fileName);
   const char * filename_const = path_and_filename;

   *pOfile = NULL;
   int errNo = fopen_s(pOfile, filename_const, "w");
   if (errNo != 0)
   {
      CString msg;
      msg.Format("OpenFileInDocumentsFolder() failed with errNo = %d", errNo);
      Report::WarningMsg(msg);
      rtnFlag = false;
   }

   return(rtnFlag);
} // end of OpenFileInDocumentsFolder()

   void AltWaterMaster::CalculateWRattributes(FlowContext *pFlowContext, MapLayer *pLayer, MapLayer *pStreamLayer, int pouRecords)
   {  // Calculate WREXISTS, WR_MUNI, WR_INSTRM, WR_IRRIG_S, WR_IRRIG_G, STRM_ORDER, WR_PCT_POU, WR_PCT_IDU, and COMID_POD.  Initializes WR_PROB to zero.
   CString msg;
   msg.Format("AltWM::CalculateWRattributes() starting now. pouRecords = %d", pouRecords);
   Report::LogMsg(msg);

   bool readOnlyFlag = pLayer->m_readOnly;
   pLayer->m_readOnly = false;
   pLayer->SetColData(m_colWREXISTS, VData(0), true);
   pLayer->SetColData(m_colWR_MUNI, VData(0), true);
   pLayer->SetColData(m_colWR_INSTRM, VData(0), true);
   pLayer->SetColData(m_colWR_IRRIG_S, VData(0), true);
   pLayer->SetColData(m_colWR_IRRIG_G, VData(0), true);
   pLayer->SetColData(m_colSTRM_ORDER, VData(0), true);
   pLayer->SetColData(m_colWATERRIGHT, VData(0), true);
   // pLayer->SetColData(m_colWR_PROB, VData(0), true);
   pLayer->SetColData(m_colWR_PCT_POU, VData(0), true);
   pLayer->SetColData(m_colWR_PCT_IDU, VData(0), true);
   pLayer->SetColData(m_colCOMID_POD, VData(0), true);

   for (int i = 0; i < pouRecords; i++)
      {
      // If the IDU overlap fraction is less than the threshold value, ignore this record.
      float percent_idu = m_pouDb.GetAsFloat(m_colPouPERCENT_IDU, i);
      if (percent_idu < m_pctIDUPOUIntersection) continue;

      int idu_id = -1; m_pouDb.Get(m_colPouIDU_ID, i, idu_id);

      m_iduIndexLookupKey.idu_id = idu_id;
      vector<int> *iduNdxVec = 0;
      iduNdxVec = &m_IDUIndexMap[m_iduIndexLookupKey]; // Should return a vector consisting of a single local idu index.
      if (iduNdxVec->size() == 0) continue; // If iduNdxVec has length 0, then the IDU referenced in this record is not in in the current layer.
      if (iduNdxVec->size() > 1)
      {
         CString msg;
         msg.Format("*** CalculateWRattributes(): i = %d, m_iduIndexLookupKey.idu_id = %d, iduNdxVec->size() = %d", i, m_iduIndexLookupKey.idu_id, iduNdxVec->size());
         Report::WarningMsg(msg);
      }
      
      int idu = iduNdxVec->at(0); // idu is the local index of the idu of interest in the idu database, whether the database is for the whole study area or a subbasin.

      int use_code = 0;
      m_pouDb.Get(m_colPouUSECODE, i, use_code);
      int permit_code = 0;
      m_pouDb.Get(m_colPouPERMITCODE, i, permit_code);

      int wr_exists = 0;
      pLayer->GetData(idu, m_colWREXISTS, wr_exists);

      wr_exists |= permit_code;
      wr_exists |= (use_code * 256);

      bool readOnlyFlag = pLayer->m_readOnly;
      pLayer->m_readOnly = false;

      pLayer->SetData(idu, m_colWREXISTS, wr_exists);

      CString msg;
      float wr_pct_pou = 0.f;
      float wr_pct_idu = 0.f;
      switch (use_code)
         {
         case (int)WRU_IRRIGATION:
            if (permit_code == (int)WRP_SURFACE) pLayer->SetData(idu, m_colWR_IRRIG_S, 1);
            else if (permit_code == (int)WRP_GROUNDWATER) pLayer->SetData(idu, m_colWR_IRRIG_G, 1);
            m_pouDb.Get(m_colPouPERCENT_POU, i, wr_pct_pou);
            m_pouDb.Get(m_colPouPERCENT_IDU, i, wr_pct_idu);
            pLayer->SetData(idu, m_colWR_PCT_POU, wr_pct_pou);
            pLayer->SetData(idu, m_colWR_PCT_IDU, wr_pct_idu);
            break;
         case (int)WRU_MUNICIPAL: pLayer->SetData(idu, m_colWR_MUNI, 1); break;
         //case (int)WRU_INSTREAM: not needed. An instream water right's "point of use" is the same as its "point of diversion".
         default:
            msg.Format("AltWaterMaster::CalculateWRattributes() USE_CODE in POU file is not WRU_IRRIGATION or WRU_MUNICIPAL. use_code = %d, idu_id = %d", 
               use_code, idu);
            Report::LogMsg(msg);
            break;
         } // end of switch(use_code)
      } // end of loop thru POUs

   // Loop thru the IDUs: for each IDU with an irrigation water right, populate the IDU's STRM_ORDER attribute.
   // Also, populate every IDU's STRM_INDEX attribute,
   // and populate the COMID_POD attribute of IDUs with surface water irrigation rights.
   // For IDUs which have UGAMUNIPOD values in the range of actual UGAs, set WR_MUNI = 1. 
   int podArrayLen = (int)m_podArray.GetCount();
   int missing_streamNdx_count = 0;
   for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
      {
      int ugamunipod = -1; pLayer->GetData(idu, m_colUGAMUNIPOD, ugamunipod);
      if (1 <= ugamunipod && ugamunipod <= MAX_UGA_NDX) pLayer->SetDataU(idu, m_colWR_MUNI, 1);
         
      int comid = 0; pLayer->GetData(idu, m_colCOMID, comid);
      int streamNdx = pStreamLayer->FindIndex(m_colStreamCOMID, comid);
      if (streamNdx < 0) missing_streamNdx_count++;
      else 
         { 
         pLayer->m_readOnly = false; pLayer->SetData(idu, m_colSTRM_INDEX, streamNdx); 
         int instrm_wr = 0;
         pStreamLayer->GetData(streamNdx, m_colStreamINSTRM_WR, instrm_wr);
         pLayer->SetData(idu, m_colWR_INSTRM, instrm_wr);
         if (instrm_wr != 0) 
            {
            int instrmwrid = 0; pStreamLayer->GetData(streamNdx, m_colStreamINSTRMWRID, instrmwrid);
            pLayer->m_readOnly = false; pLayer->SetData(idu, m_colWATERRIGHT, instrmwrid);
            }
         }

      int wr_irrig_s = 0; pLayer->GetData(idu, m_colWR_IRRIG_S, wr_irrig_s);
      int wr_irrig_g = 0; pLayer->GetData(idu, m_colWR_IRRIG_G, wr_irrig_g);
      if (wr_irrig_s != 1 && wr_irrig_g != 1) continue;

      int idu_id = -1; pLayer->GetData(idu, m_colIDU_ID, idu_id);
      bool doneFlag = false;
      for (int podNdx = 0; !doneFlag && podNdx < podArrayLen; podNdx++)
         {
         WR_USE useCode = m_podArray[podNdx]->m_useCode;
         if (!IsIrrigation(useCode)) continue;

         int pouID = m_podArray[podNdx]->m_pouID;
         PouKeyClass pouLookupKey; pouLookupKey.pouID = pouID;
         vector<int> *pouIDUs = 0; pouIDUs = &m_pouInputMap[pouLookupKey];
         if (pouIDUs->size() <= 0) continue;

         for (int pouIDUs_ndx = 0; !doneFlag && pouIDUs_ndx < pouIDUs->size(); pouIDUs_ndx++)
            {
            int pou_row = pouIDUs->at(pouIDUs_ndx);
            if (pou_row < 0 || pou_row >= m_pouDb.GetRowCount()) continue;
            float percent_idu = m_pouDb.GetAsFloat(m_colPouPERCENT_IDU, pou_row);
            if (percent_idu < m_pctIDUPOUIntersection) continue;

            int tempIDU_ID = m_pouDb.GetAsInt(m_colPouIDU_ID, pou_row);
            if (idu_id != tempIDU_ID) continue;

            // The IDU is associated with this POD.

            // Populate the IDU's STRM_ORDER attribute.
            // N.B. STRM_ORDER is for COMID_POD, not for the COMID which drains the IDU!
            int reachcomid = m_podArray[podNdx]->m_reachComid;
            int reachPolyNdx = pStreamLayer->FindIndex(m_colStreamCOMID, m_podArray[podNdx]->m_reachComid, 0);
            if (reachPolyNdx < 0) continue; // This might happen if we're simulating a subbasin instead of the whole study area.
            Reach * pReach = pFlowContext->pFlowModel->FindReachFromPolyIndex(reachPolyNdx);
            pLayer->m_readOnly = false; pLayer->SetData(idu, m_colSTRM_ORDER, pReach->m_streamOrder);

            if (wr_irrig_s == 1)
               { // Populate the COMID_POD attribute for the IDU.
               int comid_pod = m_podArray[podNdx]->m_reachComid;
               pLayer->m_readOnly = false; pLayer->SetData(idu, m_colCOMID_POD, comid_pod);
               }

            // Populate the IDU's WATERRIGHT attribute.
            pLayer->m_readOnly = false; pLayer->SetData(idu, m_colWATERRIGHT, m_podArray[podNdx]->m_wrID);

            doneFlag = true;
            } // end of loop thru POUs associated with this POD
         } // end of loop thru PODs

      } // end of loop thru IDUs

   if (missing_streamNdx_count > 0)
         {
         CString msg; msg.Format("*** AltWaterMaster::Init() There are %d IDUs with bad STRM_INDEX attribute values.", missing_streamNdx_count); 
         Report::WarningMsg(msg);
         }

   pLayer->m_readOnly = readOnlyFlag;

   msg.Format("AltWM::CalculateWRattributes() is completing now.");
   Report::LogMsg(msg);

   } // end CalculateWRattributes()


int AltWaterMaster::LoadDynamicWRDatabase(FlowContext *pFlowContext)
	{
	MapLayer *pStreamLayer = (MapLayer*)pFlowContext->pFlowModel->m_pStreamLayer;

	MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;	
			
	CString colLabel;

	int dynamWRRecords = m_dynamicWRDb.ReadAscii( m_dynamicWRTablePath ); 

	if (dynamWRRecords == 0)
		{
		CString msg;
		msg.Format("AltWM::LoadDynamicWRDatabase could not load dynamic WR .csv file (specified in .xml file) \n");
		Report::InfoMsg(msg);
		return false;
		}

	int colCount = m_dynamicWRDb.GetColCount();

	int nWRZones = 0;

	for ( int c = 0; c < colCount; c++ )
		{
		 colLabel = m_dynamicWRDb.GetLabel(c);

		 int foundZone = colLabel.Find( "WRZONE", 0 );

		 if ( foundZone >= 0 )  
			{
			nWRZones++;
			m_zoneLabels.Add( colLabel );
			}
		}
	
	if ( nWRZones == 0 )
		{
		CString msg;
		msg.Format("AltWM::LoadDynamicWRDatabase: No Water right zones (WRZONE1..WRZONEn) specified in DynamicWaterRights .csv file (should match IDU Layer) \n");
		Report::ErrorMsg(msg);
		return false;
		}

	// these columns must exist, similar to other input files etc...
	m_colDynamTimeStep	= m_dynamicWRDb.GetCol("TIMESTEP");
	m_colDynamPermitCode = m_dynamicWRDb.GetCol("PERMITCODE");
	m_colDynamUseCode	   = m_dynamicWRDb.GetCol("USECODE");
	m_colDynamIsLease	   = m_dynamicWRDb.GetCol("ISLEASE");
	
     int flag = (m_colDynamTimeStep < 0 ? 1 : 0 );
     flag += ( m_colDynamPermitCode < 0 ? 2 : 0 );
     flag += ( m_colDynamUseCode    < 0 ? 4 : 0 );
     flag += ( m_colDynamIsLease    < 0 ? 8 : 0 );

	if ( flag != 0  )
        {
		CString msg;
		msg.Format("AltWM::LoadDynamicWRDatabase: One or more column headings are incorrect in DynamicWaterRights .csv file.  Error code:%i\n", flag );
		Report::ErrorMsg(msg);
		return false;
		}
	
	pLayer->CheckCol( m_colWRZone, "WRZONE" , TYPE_LONG, CC_MUST_EXIST );

	  if ( m_colWRZone < 0  )
		{
		CString msg;
		msg.Format("AltWM::LoadDynamicWRDatabase: WRZONE(s) found in DynamicWaterRights .csv file, WRZONE must exist in IDU layer\n");
		Report::ErrorMsg(msg);
		return false;
		}

	if ( nWRZones > 0 && m_colWRZone != -1 ) m_DynamicWRs = true;
		
	return dynamWRRecords;
	}

int AltWaterMaster::IDUIndexLookupMap(FlowContext *pFlowContext)
   {
	int nIDUs = -1;

	MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

	for (MapLayer::Iterator idu = pLayer->Begin(); idu != pLayer->End(); idu++)
	   {
		// Build the key for the map lookup
      int idu_id;  m_pIDUlayer->GetData(idu, m_colIDU_ID, idu_id);
      m_iduIndexInsertKey.idu_id = idu_id;

		// Build the map
		m_IDUIndexMap[m_iduIndexInsertKey].push_back(idu);

		nIDUs = idu;
	   }

	return nIDUs;
   }

int AltWaterMaster::SortWrData(FlowContext *pFlowContext, PtrArray<WaterRight> arrayIn)
	{
	return 1;
	}


/* commented out because testing kept turning up problems.  If you put this back in service, test it thoroughly.  -- Dave Conklin 6/11/15 
int AltWaterMaster::GetNearestReachToPoint(FlowContext *pFlowContext, float easting, float northing)
   {
   MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;
   MapLayer* pStreamLayer = (MapLayer*)pFlowContext->pFlowModel->m_pStreamLayer;
   int reachesCount = pFlowContext->pFlowModel->GetReachCount();   
   float currentDistanceBetween = 100000000; // large number in meters
   int returnedComid = -1;
   Vertex tgtVertex(easting, northing);
   for (int reachNdx = 0; reachNdx < reachesCount; reachNdx++)
      {
      int reachX = 0; pStreamLayer->GetData(reachNdx, m_colStreamSTRMCENTX, reachX);
      int reachY = 0; pStreamLayer->GetData(reachNdx, m_colStreamSTRMCENTY, reachY);
      float dx = reachX - easting;
      float dy = reachY - northing;
      float distanceBetween = sqrt(dx*dx + dy*dy);
      // Poly *pReachPoly = pLayer->GetPolygon(reachNdx);
      // float distanceBetween = pReachPoly->NearestDistanceToVertex(&tgtVertex);
      if (distanceBetween < currentDistanceBetween)
         {
         currentDistanceBetween = distanceBetween;
         pStreamLayer->GetData(reachNdx, m_colStreamCOMID, returnedComid);
         }
      } // end of loop thru reaches

   return returnedComid;
   } // end of GetNearestReachToPoint()
*/


bool AltWaterMaster::AddWaterRight(FlowContext *pFlowContext, int idu, int streamLayerComid)
   {
   //if (streamLayerComid < 0) return(false);

      {
      CString msg; msg.Format("*** AddWaterRight() idu = %d, streamLayerComid = %d", idu, streamLayerComid); Report::LogMsg(msg);
      }

   MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

   MapLayer* pStreamLayer = (MapLayer*)pFlowContext->pFlowModel->m_pStreamLayer;

   GDALWrapper gdal;

   GeoSpatialDataObj geoSpatialObj;

   int errorNum = 0;

   float iduAreaM2 = 0.0;
   int   idu_id = -1;
   int   wrexist = 0;

   pLayer->GetData(idu, m_colAREA, iduAreaM2);
   pLayer->GetData(idu, m_colIDU_ID, idu_id);
   pLayer->GetData(idu, m_colWREXISTS, wrexist);
   float iduAreaAc = iduAreaM2 * ACRE_PER_M2; // acres

   __int32 newWRexists = 0;
   //SetUse( newWRexists, (unsigned __int16) 16 ); // irrigation 
   //SetPermit( newWRexists, (unsigned __int16) 2 ); // surface water right
   newWRexists = wrexist | 16 << 8;
   newWRexists = newWRexists | 2;

   pLayer->m_readOnly = false;
   pLayer->SetData(idu, m_colWREXISTS, newWRexists); // set bitwise WREXISTS attribute in IDU layer
   pLayer->SetData(idu, m_colWR_IRRIG_S, 1); 
   pLayer->SetData(idu, m_colWR_PCT_POU, 100.f);
   pLayer->SetData(idu, m_colWR_PCT_IDU, 100.f);
   pLayer->m_readOnly = true;

   WaterRight* newWaterright = new WaterRight;
   Vertex vStream, vIDU;
   int reachPolyIndex = -99;
   float distanceBetween = -99.f;
   float appropriatedPODRate = m_maxRate_cfs_per_ac * iduAreaAc / FT3_PER_M3; // cfs/acre to m3/sec
   if (streamLayerComid>0) reachPolyIndex = pStreamLayer->FindIndex(m_colStreamCOMID, streamLayerComid, 0);
   if (reachPolyIndex >= 0)
      {
      Poly *pStreamLayerPoly = pStreamLayer->GetPolygon(reachPolyIndex);
      Poly *pIDUPoly = pLayer->GetPolygon(idu);

      vStream = pStreamLayerPoly->GetCentroid();

      vIDU = pIDUPoly->GetCentroid();

      CString projectionWKT = pLayer->m_projection;

      int utmZone = geoSpatialObj.GetUTMZoneFromPrjWKT(projectionWKT);

      double reachLat, reachLon, iduLat, iduLon;

      // 22 = WGS 84 EquatorialRadius, and 1/flattening.
      gdal.UTMtoLL(22, vStream.y, vStream.x, utmZone, reachLat, reachLon);
      gdal.UTMtoLL(22, vIDU.y, vIDU.x, utmZone, iduLat, iduLon);

      // in meters
      char units = 'm';

      float distanceBetween = (float)gdal.DistanceBetween(reachLat, reachLon, iduLat, iduLon, units);
      }
   else
      {
      vStream.x = 0.f;
      vStream.y = 0.f;
      }
   // define water right Point of Diversion (POD) attributes and add to POD data object
   newWaterright->m_wrID = idu;
   newWaterright->m_xCoord = vStream.x;
   newWaterright->m_yCoord = vStream.y;
   newWaterright->m_podID = -reachPolyIndex;
   newWaterright->m_pouID = -idu;
   newWaterright->m_appCode = 1;
   newWaterright->m_permitCode = WRP_SURFACE;
   newWaterright->m_podRate_cfs = appropriatedPODRate*FT3_PER_M3;
   newWaterright->m_useCode = WRU_IRRIGATION;
   newWaterright->m_supp = 0; //primary
   newWaterright->m_priorDoy = 1;
   newWaterright->m_priorYr = m_dynamicWRAppropriationDate == -1 ? pFlowContext->pEnvContext->currentYear : m_dynamicWRAppropriationDate; 
   newWaterright->m_beginDoy = 121; // may 1st
   newWaterright->m_endDoy = 273; // september 30th
   newWaterright->m_podStatus = WRPS_NONCANCELED;
   newWaterright->m_reachComid = streamLayerComid;

   // if (index < 0) return(false); // If the reach isn't in the current stream layer, don't add the water right.

   if (reachPolyIndex >= 0) newWaterright->m_pReach = pFlowContext->pFlowModel->FindReachFromPolyIndex(reachPolyIndex);
   else newWaterright->m_pReach = NULL;

   int s = -1;
   for (s = 0; s < m_podArray.GetSize(); s++)
      {
      WaterRight * existingWR = m_podArray[s];
      if (newWaterright->m_priorYr < existingWR->m_priorYr || (newWaterright->m_priorYr==existingWR->m_priorYr && newWaterright->m_priorDoy<existingWR->m_priorDoy))
         {
         m_podArray.InsertAt(s, newWaterright);
         break;
         }
      }
   if (s >= m_podArray.GetSize()) m_podArray.Add(newWaterright);
   
   // define water right Place of Use (POU) attributes and add to POU data object

   //Build the Key for the map lookup
   m_pouInsertKey.pouID = -idu;

   int lastNdxPos = m_pouDb.GetRowCount() - 1;

   int pouDBNdx = 0;

   //Result from the Map lookup
   m_pouDb.Get(m_colPouDBNdx, lastNdxPos, pouDBNdx);

   pouDBNdx++;

   //Build the Map
   m_pouInputMap[m_pouInsertKey].push_back(pouDBNdx);

   CArray<VData, VData> newPouRow;
   // from the end of LoadWRDatabase()
   // fprintf(oFile, "POU_INDEX, POU_ID, IDU_ID, IDU_WW2100, WRIS_ID, AREA_POU, PERCENT_POU, AREA_IDU, PERCENT_IDU, USECODE, PERMITCODE, XCOORD, YCOORD, CERTIFICATE\n");
   newPouRow.SetSize(14, 1);
   newPouRow[0] = pouDBNdx; // POU_INDEX
   newPouRow[1] = -idu; // POU_ID
   newPouRow[2] = idu_id; // IDU_ID
   newPouRow[3] = -99; // IDU_WW2100
   newPouRow[4] = -99; // WRIS_ID
   newPouRow[5] = iduAreaM2; // AREA_POU
   newPouRow[6] = 100; // PERCENT_POU (ratio of IDU area/ POU area) in %
   newPouRow[7] = iduAreaM2; // AREA_IDU
   newPouRow[8] = 100; // PERCENT_IDU 
   newPouRow[9] = newWaterright->m_useCode; // USECODE
   newPouRow[10] = newWaterright->m_permitCode; // PERMITCODE
   newPouRow[11] = -99; // XCOORD
   newPouRow[12] = -99; // YCOORD
   newPouRow[13] = -99; // CERTIFICATE
   m_pouDb.AppendRow(newPouRow);

   return true;
   } // end of AddWaterRight()


bool AltWaterMaster::OutputUpdatedWRLists()
   {
   ofstream outputFile;

   outputFile.open("\\Envision\\testPOD.csv");

   outputFile << "m_wrID ,m_xCoord ,m_yCoord,m_podID,m_pouID,m_appCode ,m_permitCode ,m_podRate ,m_useCode ,m_supp ,m_priorDoy,m_priorYr,m_beginDoy,m_endDoy,m_podStatus,m_reachComid" << endl;

   for (int i = 0; i < (int)m_podArray.GetSize(); i++)
      {
      WaterRight *wrRight = m_podArray[i];
      outputFile << wrRight->m_wrID << "," << wrRight->m_xCoord << "," << wrRight->m_yCoord << "," << wrRight->m_podID << "," << wrRight->m_pouID << "," << wrRight->m_appCode << "," << wrRight->m_permitCode << "," << wrRight->m_podRate_cfs << "," << wrRight->m_useCode << "," << wrRight->m_supp << "," << wrRight->m_priorDoy << "," << wrRight->m_priorYr << "," << wrRight->m_beginDoy << "," << wrRight->m_endDoy << "," << wrRight->m_podStatus << "," << wrRight->m_reachComid << endl;
      }

   outputFile.close();

   ofstream outputFile2;

   outputFile2.open("\\Envision\\testPOU.csv");

   outputFile2 << "POU_INDEX ,POUID, IDU_INDEX, PERCENT_POU" << endl;

   for (int i = 0; i < (int)m_pouDb.GetRowCount(); i++)
      {
      int pouIndex = 0;
      int pouID = 0;
      int iduIndex = 0;
      float percent_pou = 0.0;

      m_pouDb.Get(m_colPouDBNdx, i, pouIndex);
      m_pouDb.Get(m_colPouPOU_ID, i, pouID);
      m_pouDb.Get(m_colPouIDU_ID, i, iduIndex);
      m_pouDb.Get(m_colPouPERCENT_POU, i, percent_pou);

      outputFile2 << pouIndex << "," << pouID << "," << iduIndex << "," << "," << percent_pou << endl;
      }

   outputFile2.close();

   return true;
   }

bool AltWaterMaster::ExportDistPodComid(FlowContext *pFlowContext, char unit)
	{
	// If the stream layer "resolution" is less than the POD layer, then the user might want to
	// quantify the distance between a POD and the closest vertex of the "polyline" representing
	// a reach.  the output is the same size and indexed the same as the input data file as read
	// in by the method, LoadWRDatabase

	GDALWrapper gdal;

	GeoSpatialDataObj geoSpatialObj;

	MapLayer *pStreamLayer = (MapLayer*)pFlowContext->pFlowModel->m_pStreamLayer;

	MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

	Poly *reachPoly;

	ofstream outputFile;

	outputFile.open("\\Envision\\distanceBetweenPodComid.csv");

	if ( !outputFile )
		{
		CString msg;
	   msg.Format( "AltWM::ExportDistPodComid: cannot open output file" );
	   Report::ErrorMsg( msg );
		return false;
		}

	outputFile << "COMID,PODX,PODY,Distance" << endl;

	for (int i = 0; i < (int)m_podArray.GetSize(); i++)
		{
		// Get current WR
		WaterRight *pRight = m_podArray[i];

		int comid = pRight->m_reachComid;

		double podX = pRight->m_xCoord;

		double podY = pRight->m_yCoord;

		int comidIndex = pStreamLayer->FindIndex( m_colStreamCOMID, comid, 0 );
		
		if ( comidIndex < 0 ) 
			{
			outputFile << comid << "," << podX << "," << podY << ",-99"<< endl;
			continue; // for -99 comids (outside basin) and subbasin issues
			}

		reachPoly = pStreamLayer->GetPolygon( comidIndex );

		int vertexCount = reachPoly->GetVertexCount();
		
		float closestDistance = 10000000;  // a large number

		for (int v = 0; v < vertexCount; v++)
			{           
			if ( reachPoly->GetVertexCount() > 0 )
				{
			   double reachX = reachPoly->GetVertex( v ).x;  
			   double reachY = reachPoly->GetVertex( v ).y;
				
				double reachLat = 0.0;
				double reachLon = 0.0;
				double podLat   = 0.0;
				double podLon   = 0.0;
			   
				CString projectionWKT = pLayer->m_projection; 
				
				int utmZone = geoSpatialObj.GetUTMZoneFromPrjWKT( projectionWKT );

				// 22 = WGS 84 EquatorialRadius, and 1/flattening.  UTM zone 10.
				gdal.UTMtoLL( 22,reachY,reachX,utmZone,reachLat,reachLon );
				gdal.UTMtoLL( 22,podY,podX,utmZone,podLat,podLon );
				gdal.UTMtoLL( 22,podY,podX,10,podLat,podLon );

				double distanceBetween = gdal.DistanceBetween( reachLat,reachLon,podLat,podLon,unit );

				if ( distanceBetween < closestDistance ) 
					closestDistance = (float) distanceBetween;
			   }
			else
			   {
			   CString msg;
			   msg.Format( "AltWM::ExportDistPodComid Bad stream poly (id=%i) - no vertices!!!", reachPoly->m_id );
			   Report::ErrorMsg( msg );
			   }
			}	// endfor vertices
		
		outputFile << comid << "," << podX << "," << podY << ","<<closestDistance<< endl;
			
		} // endfor POD lookup table
	
	outputFile.close();

	return true;
	}

int AltWaterMaster::ApplyRegulation(FlowContext *pFlowContext, int regulationType, WaterRight *pWaterRight, int depth, float deficit)
	{

	int nPODsAffected = 0;

	if (depth>=0) switch ( regulationType )
      {

      case 1:
		   
         nPODsAffected = RegulatoryShutOff(pFlowContext, pWaterRight, m_recursiveDepth, deficit);

         break;

      case 2:
		   
         nPODsAffected = NoteJuniorWaterRights(pFlowContext, pWaterRight, m_recursiveDepth, deficit);

         break;

      default:
         
			//no regulation
         break;

      }

	return nPODsAffected;
	}


int AltWaterMaster::RegulatoryShutOff(FlowContext *pFlowContext, WaterRight *pWaterRight, int depth, float deficit) // Returns number of PODs shut off
   {
   Reach *pReach = pWaterRight->m_pReach;
   int num_IDU_shutoffs = RegulatoryShutOff(pFlowContext, pWaterRight, depth, deficit, pReach);
   return num_IDU_shutoffs;
   } // end of RegulatoryShutOff(4 arguments)


int AltWaterMaster::RegulatoryShutOff(FlowContext *pFlowContext, WaterRight *pWaterRight, int depth, float deficit, Reach *pReach) // Returns number of PODs shut off
   {
   MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;
   int doy1 = pFlowContext->dayOfYear + 1; // day of year, with Jan 1 = 1
   int index = 0;
		
	CArray<WaterRight*, WaterRight*> *wrArray = new CArray<WaterRight*, WaterRight*>;   // create an initial, empty array for holding any found junior water rights in this reach and upstream reaches

   int count = GetJuniorWaterRights(pFlowContext, pReach, pWaterRight, depth, wrArray);  // get list of all water rights more junior to the target WR that are in this reach or upstream reaches
   int num_POD_shutoffs = 0;
   int num_IDU_shutoffs = 0;
   while (deficit > 0 && index < count)  // iterate through junior rights until deficit is covered.
		{    
		// this array was sorted from junior to senior as it was built in GetJuniorWaterRights
		WaterRight *pNext = wrArray->GetAt(index);
      index++;

		// if surface water right for irrigation, has not been seen this step, is "in-season", has not been shut off yet this season, and m_allocatedYesterday_cms > 0
      // then simulate regulatory shut off
      if (pNext->m_permitCode == WRP_SURFACE && pNext->m_useCode == WRU_IRRIGATION &&
         pNext->m_stepShortageFlag == false && pNext->m_allocatedYesterday_cms > 0.f && !pNext->m_suspendForYear &&
         (pFlowContext->dayOfYear >= pNext->m_beginDoy && pFlowContext->dayOfYear <= pNext->m_endDoy))
         {
         int num_IDUs_shutoff_for_this_POD = ShutOffOnePOD(pFlowContext, pNext, &deficit);
         num_IDU_shutoffs += num_IDUs_shutoff_for_this_POD;
         num_POD_shutoffs = 1;
         // Log the shutoff.
            {
            CString msg;
            msg.Format("junior WR %d (%d day %d COMID %d) shutoff caused by senior WR %d at COMID %d: year = %d, doy1 = %d, junior POD_ID = %d, senior POD_ID = %d, "
               "remaining deficit = %f m3/s, junior POD m_allocatedYesterday_cms = %f",
               pNext->m_wrID, pNext->m_priorYr, pNext->m_priorDoy, pNext->m_reachComid, pWaterRight->m_wrID, pReach->m_reachID,
               pFlowContext->pEnvContext->currentYear, doy1, pNext->m_podID, pWaterRight->m_podID, deficit, pNext->m_allocatedYesterday_cms);
            Report::LogMsg(msg);
            }
         int waterRightID = pNext->m_wrID;
         // Now make sure that all the other PODs with the same water right ID are shut off.
         for (int i = 0; i < (int)m_podArray.GetSize(); i++) 
            if ((m_podArray[i]->m_wrID == waterRightID) && (!m_podArray[i]->m_suspendForYear))
               {
               num_IDU_shutoffs += ShutOffOnePOD(pFlowContext, m_podArray[i], &deficit);
               num_POD_shutoffs++;
               // Log the shutoff.
                  {
                  CString msg;
                  msg.Format("junior WR %d (%d day %d COMID %d) shutoff caused by senior WR %d at COMID %d: year = %d, doy1 = %d, junior POD_ID = %d, senior POD_ID = %d, "
                     "remaining deficit = %f m3/s, junior POD m_allocatedYesterday_cms = %f",
                     m_podArray[i]->m_wrID, m_podArray[i]->m_priorYr, m_podArray[i]->m_priorDoy, m_podArray[i]->m_reachComid, pWaterRight->m_wrID, pReach->m_reachID,
                     pFlowContext->pEnvContext->currentYear, doy1, m_podArray[i]->m_podID, pWaterRight->m_podID, deficit,
                     m_podArray[i]->m_allocatedYesterday_cms);
                  Report::LogMsg(msg);
                  }
               }
         }
		} // end of while ( deficit > 0 && index < count )
   return num_POD_shutoffs;  
   } // end of RegulatoryShutOff(5 arguments)


int AltWaterMaster::ShutOffOnePOD(FlowContext *pFlowContext, WaterRight * pPOD, float * pDeficit) // Returns the number of affected IDUs
   {
   MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

   pPOD->m_suspendForYear = true;
 
   // Assumes today's step request will be the same as yesterday's step request
   // *pDeficit -= pPOD->m_stepRequest; // m3/sec reduce the deficit appropriately
   *pDeficit -= pPOD->m_allocatedYesterday_cms; // m3/sec reduce the deficit appropriately
   pPOD->m_stepShortageFlag = true;

   if (pPOD->m_useCode != WRU_IRRIGATION) return(0);

   vector<int> *pouIDUs = 0; // object to hold idus for this WR   				     
   vector<int> *iduNdxVec = 0; // object for getting IDU_INDEX in sub area   
   int pouID = pPOD->m_pouID; // Get POUID (aka Place of Use) for current Permit   
   m_pouLookupKey.pouID = pouID; // Build lookup key into point of use (POU) map/lookup table   
   pouIDUs = &m_pouInputMap[m_pouLookupKey]; // Returns vector of indexs into the POU input data file. Used for relating to current water right POU to polygons in IDU layer
   if (pouIDUs->size() <= 0)  return(0);

   // Set Begin looping through IDUs Associated with WRID and PLace of Use (POU)
   int num_IDU_shutoffs = 0;
   for (int j = 0; j < (int)pouIDUs->size(); j++)
      {
      int pou_row = pouIDUs->at(j);
      if (pou_row < 0 || pou_row >= m_pouDb.GetRowCount()) continue;
      float percent_idu = m_pouDb.GetAsFloat(m_colPouPERCENT_IDU, pou_row);
      if (percent_idu < m_pctIDUPOUIntersection) continue;
      int tempIDU_ID = m_pouDb.GetAsInt(m_colPouIDU_ID, pou_row);

      //build key into idu index map
      ASSERT(tempIDU_ID >= 0);
      m_iduIndexLookupKey.idu_id = tempIDU_ID;

      //returns vector with the idu index for idu layer
      iduNdxVec = &m_IDUIndexMap[m_iduIndexLookupKey];

      int iduNdx = 0;

      //if no index is return, then the idu that the POU is associated with is not in current layer
      if (iduNdxVec->size() == 0) continue;

      iduNdx = iduNdxVec->at(0);
      pLayer->m_readOnly = false;
      pLayer->SetData(iduNdx, m_colWRShutOff, 1);
      pLayer->SetData(iduNdx, m_colIRR_STATE, IRR_SHUT_OFF_BY_REG_ACTION);
      pLayer->SetData(iduNdx, m_colWR_SH_DAY, pFlowContext->dayOfYear); 
      pLayer->m_readOnly = true;
      num_IDU_shutoffs++;
      } // end of for (int j = 0; j < (int)pouIDUs->size(); j++)

   return num_IDU_shutoffs;
   } // end of ShutOffOnePOD()


int AltWaterMaster::NoteJuniorWaterRights( FlowContext *pFlowContext, WaterRight *pWaterRight, int depth, float deficit )
   {	
	MapLayer *pLayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;
   int doy1 = pFlowContext->dayOfYear + 1; // day of year, with Jan 1 = 1
	int index = 0;
	
	// set .xml file.  only proceed if there is a threshold for suspending a WR (0-365)
	if ( m_maxDaysShortage >= 0 )
		{
		
		Reach *pReach = pWaterRight->m_pReach;
		
		CArray<WaterRight*, WaterRight*> *wrArray = new CArray<WaterRight*, WaterRight*>;   // create an initial, empty array for holding any found junior water rights in this reach and upstream reaches

      int count = GetJuniorWaterRights(pFlowContext, pReach, pWaterRight, depth, wrArray);  // get list of all water rights more junior to the target WR that are in this reach or upstream reaches
 
		while ( deficit > 0 && index < count )  // iterate through junior rights until deficit is covered.
		   {    
			// this array was sorted from junior to senior as it was built in GetJuniorWaterRights
			WaterRight *pNext = wrArray->GetAt(index);

         // if surface water right for irrigation, has not been seen this step, and is "in-season", and m_stepRequest > 0
         if (pNext->m_permitCode == WRP_SURFACE && pNext->m_useCode == WRU_IRRIGATION &&
            pNext->m_stepShortageFlag == false && pNext->m_stepRequest > 0.f &&
				  (  doy1 >= pNext->m_beginDoy && doy1 <= pNext->m_endDoy  ) )
				{
				// object to hold idus for this WR
				vector<int> *pouIDUs = 0;	

				// object for getting IDU_INDEX in sub area				     
				vector<int> *iduNdxVec = 0;

				// figure out flow increase when we cut this one off;
				// Assumes next step request is the same as this step request
				float flowFromNextRight = pNext->m_stepRequest;

				deficit -= flowFromNextRight; // m3/sec reduce the deficit appropriately

				pNext->m_stepShortageFlag = true;

				// Get POUID (aka Place of Use) for current Permit
		      int pouID = pNext->m_pouID;

				// Build lookup key into point of use (POU) map/lookup table
				m_pouLookupKey.pouID = pouID;

				// Returns vector of indexs into the POU input data file. Used for relating to current water right POU to polygons in IDU layer
				pouIDUs = &m_pouInputMap[m_pouLookupKey];

				// if a PODID does not have a POUID, consider next WR
            if (pouIDUs->size() == 0) continue;

				// Set action level. Begin looping through IDUs Associated with WRID and PLace of Use (POU)
				for (int j = 0; j < (int) pouIDUs->size(); j++)
					{
               int pou_row = pouIDUs->at(j);
               if (pou_row < 0 || pou_row >= m_pouDb.GetRowCount()) continue;
               int tempIDU_ID = m_pouDb.GetAsInt(m_colPouIDU_ID, pou_row);

					//build key into idu index map
					m_iduIndexLookupKey.idu_id = tempIDU_ID;

					//returns vector with the idu index for idu layer
					iduNdxVec = &m_IDUIndexMap[m_iduIndexLookupKey];

					int iduNdx = 0;

					//if no index is return, then idu the POU is associated with is not in current layer
               if (iduNdxVec->size() == 0) continue;

					iduNdx = iduNdxVec->at(0);

					pLayer->m_readOnly = false;
					pLayer->m_readOnly = true;
/*
					int consecCheck =  pFlowContext->dayOfYear - pNext->m_lastDOYShortage; 

					( consecCheck < 2 ) ? pNext->m_stepsSuspended++ : pNext->m_stepsSuspended = 0;
				
					if ( pNext->m_stepsSuspended >= m_maxDaysShortage )
						{					
						pNext->m_suspendForYear = false;

						pLayer->m_readOnly = false;
						pLayer->SetData( iduNdx, m_colWRJuniorAction, 2 ); // level 2
						pLayer->m_readOnly = true;
						}
*/
					}

				pNext->m_lastDOYShortage = pFlowContext->dayOfYear; 

				}
		   index++;
		   }
      wrArray->RemoveAll();
		}
   return index;  // number of junior rights cut off
   }

int AltWaterMaster::GetJuniorWaterRights( FlowContext *pFlowContext,  Reach *pReach, WaterRight *pWaterRight, int depth, CArray<WaterRight*, WaterRight*> *wrArray )
   {
	// note: calling this initially with depth=0 will look only at the reach; values greater than 0 
	// cause it to look increasngly further upstream.
   /*
      {
      CString msg; msg.Format("Enter GetJuniorWaterRights() with pReach->m_polyIndex = %d, pWaterRight->m_wrID = %d, pWaterRight->m_podID = %d, "
      "pWaterRight->m_pouID = %d, depth = %d",
      pReach->m_polyIndex, pWaterRight->m_wrID, pWaterRight->m_podID, pWaterRight->m_pouID, depth); Report::LogMsg(msg);
      }
   */
	// terminate recursion?
   if ( depth < 0 )
      return (int) wrArray->GetSize();

	MapLayer* pStreamLayer = (MapLayer*) pFlowContext->pFlowModel->m_pStreamLayer;

   // get the water rights associated with this reach
	CArray<WaterRight*, WaterRight*> *thisWRArray = new CArray<WaterRight*, WaterRight*>;
   CArray< Reach*, Reach* > *thisReachArray = new CArray< Reach*, Reach* >;

	thisReachArray->Add( pReach );

 	int nWR = GetWRsInReach( thisReachArray, thisWRArray );

   // this would need to be created when the wr database is loaded.
   // if there are any juniors, put them into the cumulative array
   if (nWR>0) for ( int i=0; i < nWR; i++ )
      {
      WaterRight * pWaterRight_local = thisWRArray->GetAt( i );
/*       {
         CString msg; msg.Format("water right in reach: pWaterRight->m_wrID = %d, pWaterRight->m_podID, pWaterRight->m_pouID = %d",
            pWaterRight_local->m_wrID, pWaterRight_local->m_podID, pWaterRight_local->m_pouID); Report::LogMsg(msg);
         }
*/
		// do not consider the seed water right
		if ( pWaterRight_local->m_wrID == pWaterRight->m_wrID /* && _pWaterRight->m_podID == pWaterRight->m_podID */ )
			continue;

		bool isJunior = IsJunior( pWaterRight, pWaterRight_local);
		
      // If the candidate WR is in the same reach as the original WR, consider it to be downstream.
      bool isUpStream = pWaterRight->m_reachComid != pWaterRight_local->m_reachComid;

      // compare to the initial water right experiencing a shortage
		if ( isJunior && isUpStream ) 	
			{
/*          {
            CString msg; msg.Format("water right is junior and is upstream: pWaterRight->m_wrID = %d, pWaterRight->m_podID = %d, pWaterRight->m_pouID = %d",
            pWaterRight_local->m_wrID, pWaterRight_local->m_podID, pWaterRight_local->m_pouID); Report::LogMsg(msg);
            }
*/
			if ( wrArray->GetSize() == 0 )
				{
				wrArray->Add( pWaterRight_local );
				}
			else // sort as you go from junior to senior
				{				 
				for ( int s = 0; s < wrArray->GetSize(); s++ )
					{
					
					WaterRight * existingWR = wrArray->GetAt( s );
					
				   bool isJunior = IsJunior( existingWR, pWaterRight_local );

					if ( isJunior  )
					   {			
						wrArray->InsertAt( s, pWaterRight_local );
						break;
						}
					else if ( s == (wrArray->GetSize() - 1) && !isJunior )
						{
						wrArray->Add( pWaterRight_local );
						}
					} 
				}
			}
      }
     
	// recurse upstream if needed
	Reach *pUpstreamReachLeft = m_pFlowModel->GetReachFromNode(pReach->m_pLeft);
	Reach *pUpstreamReachRight = m_pFlowModel->GetReachFromNode(pReach->m_pRight);
/*      {
      CString msg; msg.Format("%s %s %s %d %d %d",
         pReach->m_pDown == NULL ? "pDown is NULL" : "",
         pReach->m_pLeft == NULL ? "pLeft is NULL" : "",
         pReach->m_pRight == NULL ? "pRight is NULL" : "",
         nullDown, nullLeft, nullRight); 
      Report::LogMsg(msg);
      }

   if (pReach->m_pDown != NULL)
      {         { CString msg; msg.Format("pDown->m_reachIndex = %d", ((Reach *)(pReach->m_pDown))->m_reachIndex);
         Report::LogMsg(msg);
         }
      }

   if (!nullLeft)
      {
         { CString msg; msg.Format("pLeft->m_reachIndex = %d", ((Reach *)(pReach->m_pLeft))->m_reachIndex);
         Report::LogMsg(msg);
         }

      int index = ((Reach *)(pReach->m_pLeft))->m_reachIndex;
      pUpstreamReachLeft = pFlowContext->pFlowModel->FindReachFromIndex(index);
      }

   if (!nullRight)
		{
         { CString msg; msg.Format("pRight->m_reachIndex = %d", ((Reach *)(pReach->m_pRight))->m_reachIndex);
         Report::LogMsg(msg);
         }

      int index = pReach->m_pRight->m_reachIndex;
		pUpstreamReachRight = pFlowContext->pFlowModel->FindReachFromIndex(index);		
		}
*/		
	if (pUpstreamReachLeft != NULL) 
		{
		GetJuniorWaterRights( pFlowContext, pUpstreamReachLeft, pWaterRight, depth-1, wrArray );
		}

	if (pUpstreamReachRight != NULL)
		{
		GetJuniorWaterRights( pFlowContext, pUpstreamReachRight,  pWaterRight, depth-1, wrArray );
		}

   thisWRArray->RemoveAll();
   thisReachArray->RemoveAll();

   return (int) wrArray->GetSize();
   }

bool AltWaterMaster::IsJunior(WaterRight *incumbentWR, WaterRight *canidateWR)
	{
	bool isJunior = false;		
	
	if ( canidateWR->m_priorYr >= incumbentWR->m_priorYr )
		{
		isJunior = true;
	
//      if (canidateWR->m_priorYr == incumbentWR->m_priorYr && canidateWR->m_priorDoy <= incumbentWR->m_priorDoy) // This seems to cause an endless loop
      if (canidateWR->m_priorYr == incumbentWR->m_priorYr && canidateWR->m_priorDoy < incumbentWR->m_priorDoy)
         {
			isJunior = false;
			}
		}
	return isJunior;
	}

int  AltWaterMaster::GetWRsInReach( CArray<Reach*, Reach*> *reachArray, CArray<WaterRight*, WaterRight*> *wrArray )
	{ 
	
	for ( int i = 0; i < (int) reachArray->GetSize(); i++ )
		{
		
		Reach *pReach = reachArray->GetAt( i );

		for ( int j = 0; j < (int) m_podArray.GetSize(); j++ )
			{
         /*
			WaterRight *tmpRight = m_podArray[j];

			int reachID = tmpRight->m_reachComid;

			if ( pReach->m_reachID == tmpRight->m_reachComid )
				wrArray->Add( tmpRight );
         */
         if ((pReach == m_podArray[j]->m_pReach) && (m_podArray[j]->m_podStatus != WRPS_CANCELED)) wrArray->Add(m_podArray[j]);
			}
		}

	return (int) wrArray->GetSize();
	}


bool AltWaterMaster::LoadXml(WaterAllocation *pMethod, TiXmlElement *pXmlElement, LPCTSTR filename )
   {
   LPTSTR query    = NULL;   // ignored for now
   LPTSTR method   = NULL;
   LPTSTR podTable = NULL;
   LPTSTR pouTable = NULL;
	LPTSTR dynamicWRTable = NULL;
   LPTSTR value    = NULL;
   LPTSTR aggCols  = NULL;
   int irrigatedHruLayer = 0;
	int nonIrrigatedHruLayer = 0;
   int nYearsWRNotUsedLimit = 0;
   int nDaysWRConflict1 = 0;
   int nDaysWRConflict2 = 0;
	int   exportDisPodComid = 0;
	float dynamicWRRadius = 0.0;
	int maxDaysShortage = 366;
	int recursiveDepth = 1;
	float pctIDUPOUIntersection = 60.0;
	int dynamAppropriationDate = -1;

	bool  wmDebug = false;
	int   maxDutyHalt = 0;
   LPTSTR WRExistColName = NULL;
   LPTSTR irrigateColName = NULL;
   int irrBeginDoy = 366; // if initial irr default begin and end DOY are 366, no diversions will happen 
	int irrEndDoy = 366;
   bool use = true;

    XML_ATTR attrs[] = {
		// attr                                   type				address						isReq  checkCol
      { "name",											TYPE_CSTRING,  &(pMethod->m_name),		false,   0 },
      { "method",											TYPE_STRING,   &method,						true,    0 },
      { "query",											TYPE_STRING,   &query,						false,   0 },
      { "pod_table",										TYPE_STRING,   &podTable,					true,    0 },
      { "pou_table",										TYPE_STRING,   &pouTable,					true,    0 },
		{ "dynamic_WR_table",							TYPE_STRING,   &dynamicWRTable,			true,    0 },
      { "n_years_WR_not_used_limit",				TYPE_INT,      &nYearsWRNotUsedLimit,	true,    0 },
      { "n_days_WR_shortage_1",						TYPE_INT,      &nDaysWRConflict1,		true,    0 },
      { "n_days_WR_shortage_2",						TYPE_INT,      &nDaysWRConflict2,		true,    0 },
		{ "export_distance_POD_COMID",				TYPE_INT,      &exportDisPodComid,		true,    0 },
		{ "percent_IDU_POU_intersection",         TYPE_FLOAT,    &pctIDUPOUIntersection, true,    0 },
		{ "max_duty_halt",								TYPE_INT,      &maxDutyHalt,		      true,    0 },
		{ "dynamicWRRadius",                      TYPE_FLOAT,    &dynamicWRRadius,       false,   0 },
		{ "max_days_in_shortage",						TYPE_INT,      &maxDaysShortage,		   true,    0 },
		{ "reach_recursive_depth_shortage",			TYPE_INT,      &recursiveDepth,		   true,    0 },
      { "Water_RightExist_BitWise_col_name",		TYPE_STRING,   &WRExistColName,			false,   0 },
      { "irrigate_or_not_col_name",					TYPE_STRING,	&irrigateColName,			false,   0 },
		{ "default_irrigation_begin_dayofyear",	TYPE_INT,		&irrBeginDoy,				false,	0 },
	   { "default_irrigation_end_dayofyear",		TYPE_INT,      &irrEndDoy,					false,   0 },
      { "value",											TYPE_STRING,   value,						false,   0 },
		{ "DynamicWR_Appropriation_Date",         TYPE_INT,      &dynamAppropriationDate,false,   0 },
      { "agg_cols",										TYPE_STRING,   &aggCols,					false,   0 },
      { "use",												TYPE_BOOL,     &use,							false,   0 },
		{ "debug",											TYPE_BOOL,     &wmDebug,					false,   0 },
      { NULL,												TYPE_NULL,     NULL,							false,   0 } };


   bool ok = TiXmlGetAttributes( pXmlElement, attrs, filename );
   if ( ! ok )
      {
      CString msg; 
      msg.Format( _T("Flow: Misformed element reading <allocation> attributes for AltWaterMaster method in input file %s"), filename );
      Report::ErrorMsg( msg );
      delete pMethod;
      return false;
      }

   m_podTablePath = podTable;
   m_pouTablePath = pouTable;
   m_dynamicWRTablePath = dynamicWRTable;
   m_irrigatedHruLayer = BOX_IRRIG_SOIL;
   m_nonIrrigatedHruLayer = BOX_NAT_SOIL;
   m_shallowSubsoilHRUlayer = BOX_FAST_GW;
   m_nDaysWRConflict1 = nDaysWRConflict1;
   m_nDaysWRConflict2 = nDaysWRConflict2;
   m_maxDutyHalt = maxDutyHalt;
   m_exportDistPodComid = exportDisPodComid;
   m_wrExistsColName = WRExistColName;
   m_irrigateColName = irrigateColName;
   m_nYearsWRNotUsedLimit = nYearsWRNotUsedLimit;
   m_irrDefaultBeginDoy = irrBeginDoy;
   m_irrDefaultEndDoy = irrEndDoy;
   m_debug = wmDebug;
   m_dynamicWRRadius = dynamicWRRadius;
   m_maxDaysShortage = maxDaysShortage;
   m_recursiveDepth = recursiveDepth;
   m_dynamicWRAppropriationDate = dynamAppropriationDate;
   m_pctIDUPOUIntersection = pctIDUPOUIntersection;
   pMethod->m_pAltWM = this;   

   return true;
   }


bool AltWaterMaster::WriteWRreport()
{
   // Get the path to the current user's Documents folder.
   PWSTR userPath;
   SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &userPath);
   char szBuffer[255];
   WideCharToMultiByte(CP_ACP, 0, userPath, -1, szBuffer, sizeof(szBuffer), NULL, NULL);

   // Set the string "filename" equal to Users\<username>\Documents\InstreamWaterRightsReport.csv
   CString filename;
   filename.Format("%s\\InstreamWaterRightsReportByReach.csv", szBuffer);
   const char * filename_const = filename;

   FILE *oFile = NULL;
   int errNo = fopen_s(&oFile, filename_const, "w");
   if (errNo != 0) return(false);

   fprintf(oFile, "COMID, INSTRMWRID, INSTRM_WR, STRM_ORDER\n");

   for (MapLayer::Iterator reach = m_pReachLayer->Begin(); reach < m_pReachLayer->End(); reach++)
   {
      int instrmwrid; m_pReachLayer->GetData(reach, m_colStreamINSTRMWRID, instrmwrid);
      if (instrmwrid == 0) continue;
      int comid; m_pReachLayer->GetData(reach, m_colStreamCOMID, comid);
      int instrm_wr; m_pReachLayer->GetData(reach, m_colStreamINSTRM_WR, instrm_wr);
      int strm_order; m_pReachLayer->GetData(reach, m_colStreamSTRM_ORDER, strm_order);
      fprintf(oFile, "%d, %d, %d, %d\n", comid, instrmwrid, instrm_wr, strm_order);
   }

   fclose(oFile); oFile = NULL;

   // Now set the string "filename" equal to Users\<username>\Documents\IrrigationWaterRightsReportByIDU.csv
   filename.Format("%s\\IrrigationWaterRightsReportByIDU.csv", szBuffer);
   const char * filename_const2 = filename;

   errNo = fopen_s(&oFile, filename_const2, "w");
   if (errNo != 0) return(false);

   fprintf(oFile, "IDU_ID, WR_IRRIG_S, WR_IRRIG_G, WATERRIGHT, WRPRIORITY, WR_PCT_IDU, WR_PCT_POU, SUB_AREA, HBVCALIB, COUNTYID, COMID_POD\n");

   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu < m_pIDUlayer->End(); idu++)
   {
      int wr_irrig_s; m_pIDUlayer->GetData(idu, m_colWR_IRRIG_S, wr_irrig_s);
      int wr_irrig_g; m_pIDUlayer->GetData(idu, m_colWR_IRRIG_G, wr_irrig_g);
      if (wr_irrig_s == 0 && wr_irrig_g == 0) continue;

      int idu_id; m_pIDUlayer->GetData(idu, m_colIDU_ID, idu_id);
      int waterright; m_pIDUlayer->GetData(idu, m_colWATERRIGHT, waterright);
      int wrpriority; m_pIDUlayer->GetData(idu, m_colWRPRIORITY, wrpriority);
      float wr_pct_idu; m_pIDUlayer->GetData(idu, m_colWR_PCT_IDU, wr_pct_idu);
      float wr_pct_pou; m_pIDUlayer->GetData(idu, m_colWR_PCT_POU, wr_pct_pou);
      int sub_area; m_pIDUlayer->GetData(idu, m_colSUB_AREA, sub_area);
      int hbvcalib; m_pIDUlayer->GetData(idu, m_colHBVCALIB, hbvcalib);
      int countyid; m_pIDUlayer->GetData(idu, m_colCOUNTYID, countyid);
      int comid_pod; m_pIDUlayer->GetData(idu, m_colCOMID_POD, comid_pod);

      fprintf(oFile, "%d, %d, %d, %d, %d, %f, %f, %d, %d, %d, %d\n", 
         idu_id, wr_irrig_s, wr_irrig_g, waterright, wrpriority, wr_pct_idu, wr_pct_pou, sub_area, hbvcalib, countyid, comid_pod);
   }

   fclose(oFile); oFile = NULL;

   // Now set the string "filename" equal to Users\<username>\Documents\InstreamWaterRightsReportByPOD.csv
   filename.Format("%s\\InstreamWaterRightsReportByPOD.csv", szBuffer);
   const char * filename_const3 = filename;

   errNo = fopen_s(&oFile, filename_const3, "w");
   if (errNo != 0) return(false);

   fprintf(oFile, "POD_INDEX, REACHCOMID, REACH_ID, STREAMINDEX, XCOORD, YCOORD, BEGIN_DOY, END_DOY, PODRATE_CFS, WATERRIGHTID, PODID, PRIORYR, PRIORDOY, PERMITCODE, USECODE, PODSTATUS\n");

   // loop thru the m_podArray entries: for each entry with use=WRU_INSTREAM, find the associated reach
   int podArrayLen = (int)m_podArray.GetCount();
   for (int podNdx = 0; podNdx < podArrayLen; podNdx++)
   {
      Reach *pReach = m_podArray[podNdx]->m_pReach;
      if (m_podArray[podNdx]->m_useCode == WRU_INSTREAM)
         fprintf(oFile, "%d, %d, %d, %d, %f, %f, %d, %d, %f, %d, %d, %d, %d, %d, %d, %d\n",
            podNdx,
            m_podArray[podNdx]->m_reachComid,
            pReach != NULL ? pReach->m_reachID : 0,
            m_podArray[podNdx]->m_streamIndex,
            m_podArray[podNdx]->m_xCoord,
            m_podArray[podNdx]->m_yCoord,
            m_podArray[podNdx]->m_beginDoy,
            m_podArray[podNdx]->m_endDoy,
            m_podArray[podNdx]->m_podRate_cfs,
            m_podArray[podNdx]->m_wrID,
            m_podArray[podNdx]->m_podID,
            m_podArray[podNdx]->m_priorYr,
            m_podArray[podNdx]->m_priorDoy,
            m_podArray[podNdx]->m_permitCode,
            m_podArray[podNdx]->m_useCode,
            m_podArray[podNdx]->m_podStatus);
   }

   fclose(oFile); oFile = NULL;

   // Now set the string "filename" equal to Users\<username>\Documents\SurfaceIrrigationWaterRightsReportByPOD.csv
   filename.Format("%s\\SurfaceIrrigationWaterRightsReportByPOD.csv", szBuffer);
   const char * filename_const4 = filename;

   errNo = fopen_s(&oFile, filename_const4, "w");
   if (errNo != 0) return(false);

   fprintf(oFile, "POD_INDEX, REACHCOMID, REACH_ID, STREAMINDEX, XCOORD, YCOORD, BEGIN_DOY, END_DOY, PODRATE_CFS, WATERRIGHTID, PODID, PRIORYR, PRIORDOY, PERMITCODE, USECODE, PODSTATUS, "
      "POUID, POURATE, INUSE, NPODSPERWR, PODUSERATE_CFS, SPECIALCODE\n");

   // loop thru the m_podArray entries: for each entry with use=WRU_INSTREAM, find the associated reach
   for (int podNdx = 0; podNdx < podArrayLen; podNdx++)
   {
      Reach *pReach = m_podArray[podNdx]->m_pReach;
      if (m_podArray[podNdx]->m_useCode == WRU_IRRIGATION && m_podArray[podNdx]->m_permitCode == WRP_SURFACE)
         fprintf(oFile, "%d, %d, %d, %d, %f, %f, %d, %d, %f, %d, %d, %d, %d, %d, %d, %d, %d, %f, %d, %d, %f, %d\n",
            podNdx,
            m_podArray[podNdx]->m_reachComid,
            pReach != NULL ? pReach->m_reachID : 0,
            m_podArray[podNdx]->m_streamIndex,
            m_podArray[podNdx]->m_xCoord,
            m_podArray[podNdx]->m_yCoord,
            m_podArray[podNdx]->m_beginDoy,
            m_podArray[podNdx]->m_endDoy,
            m_podArray[podNdx]->m_podRate_cfs,
            m_podArray[podNdx]->m_wrID,
            m_podArray[podNdx]->m_podID,
            m_podArray[podNdx]->m_priorYr,
            m_podArray[podNdx]->m_priorDoy,
            m_podArray[podNdx]->m_permitCode,
            m_podArray[podNdx]->m_useCode,
            m_podArray[podNdx]->m_podStatus,         
            m_podArray[podNdx]->m_pouID, 
            m_podArray[podNdx]->m_pouRate,
            m_podArray[podNdx]->m_inUse,
            m_podArray[podNdx]->m_nPODSperWR,
            m_podArray[podNdx]->m_podUseRate_cfs,
            m_podArray[podNdx]->m_specialCode);
   }

   fclose(oFile); oFile = NULL;

   return(true);
} // end of WriteWRreport()