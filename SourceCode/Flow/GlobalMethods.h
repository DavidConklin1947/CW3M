#pragma once

#include <afxtempl.h>

#include "FlowContext.h"

#include "LayerDistributions.h"
#include "ETEquation.h"

#include <tixml.h>
#include <PtrArray.h>
#include <QueryEngine.h>
#include <Vdataobj.h>
#include <FDATAOBJ.H>
#include <IDATAOBJ.H>
#include <MapExprEngine.h>
#include <map>
#include <DATE.HPP>

using namespace std;

#define LULCA_AG 2 /* LULC_A {Agriculture} */
#define LULCA_FOREST 4 /* LULC_A {Forest} */
#define LULCA_WETLAND 62 /* Palustrine non-isolated wetland */ 

class WaterMaster;
class AltWaterMaster;
class WaterParcel;
class ParamTable;

enum IrrigationState
   {
   IRR_NOT_STARTED_OR_OFF_SEASON, IRR_FULL, IRR_PARTIAL, IRR_SUSPENDED, IRR_SHUT_OFF_BY_REG_ACTION
   };

//--------------------------------------------------------------------------------
// GlobalMethods
//

enum GM_DOMAIN { GMD_HRU=1, GMD_REACH=2 };

enum GM_METHOD
   { 
   GM_NONE=1,                                            // All
   GM_EULER, GM_RK4, GM_RKF, GM_KINEMATIC, GM_2KW,        // ReachRouting
   GM_LINEAR_RESERVOIR,                                  // LateralExchange
   GM_BROOKS_COREY,                                      // HruVerticalExchange
   GM_MODFLOW,                                           // GroundWater
   GM_ASCE, GM_FAO56, GM_PENMAN_MONTEITH, GM_HARGREAVES, GM_WETLAND_ET, // GM_KIMBERLY_PENNMAN,        // EvapTrans
	GM_URBAN_DEMAND_DY,							   			   // Daily Urban demand
   GM_WATER_RIGHTS, GM_EXPR_ALLOCATOR, GM_ALTWM,         // WaterAllocation
   GM_FLUX_EXPR,                                         // FluxExpr
   GM_SPRING, // special case of flux expressions, for springs which flow into reaches
   GM_DEMAND_EXPR,                                       // DemandExpr
   GM_EXTERNAL
   };


enum VALUE_TYPE { VT_EXPR, VT_TIMESERIES, VT_FILE };

enum FLUX_DOMAIN
   {
   FD_UNDEFINED = 0,
   FD_CATCHMENT=1,
   FD_REACH = 2,
   FD_RESERVOIR = 3
   };
   
//-----------------------------------------------------------------------------------------
// Notes on EvapTrans
//
//  1) FAO56 - similar to kimberly, but not the same - requires crop coefficients that vary
//             over the growing season. Crop coefficients are based on accumulating
//             heat units to define start of growing season, length of growing season.
//  2) PENMAN_MONTEITH - doesn't use crop coefficient, used in forested landscape
//             (simulates resistences based on veg characteristics, not crop coeffiencts)
//             (has coefficients related to aerodynamic resistance, surface resistance)
//  3) HARGREAVES - 
//-----------------------------------------------------------------------------------------




struct LayerDistr
{
int   layerIndex;
float fraction;      // decimal percent
};


class GlobalMethod
{
public:
   CString m_name;
   int m_use;
   int m_timing; 
   int m_id;  // unique identifier, 1-based
   float m_runTime;     // seconds/run

   static int m_nextID;  // starts at 1, used to assign unique ids as GMs are added

protected:
   GM_METHOD m_method;           // definition is specific to the instance... See table above 

public:
   Query *m_pQuery;      // NULL if no query specified - uses Envision's QueryEngine
 
   CString m_query; 

   virtual bool SetMethod( GM_METHOD method ) = 0;

   GM_METHOD GetMethod( void ) { return m_method; }

   // run looks for externally-defined methods and runs them if they exist.
   // Otherwise, it returns false, signalling that the children methods should handle
   virtual bool Init     ( FlowContext* );                          // start of Envision session
   virtual bool InitRun  ( FlowContext* ) { return true; }          // start of an Envision Run
   virtual bool StartYear( FlowContext* ) { return true; }          // start of Flow::Run() invocation (was PreRun)
   virtual bool StartStep( FlowContext* ) { return true; }          // start of a Flow timestep
   virtual bool Step     ( FlowContext* );                          // during Flow::Run() (may be called multiple times, depending on m_time value)
   virtual bool EndStep  ( FlowContext* ) { return true; }          // end of a Flow timestep
   virtual bool EndYear  ( FlowContext* ) { return true; }          // end of a Flow::Run() invocation
   virtual bool EndRun   ( FlowContext* ) { return true; }          // end of an Envision run

   virtual bool SetTimeStep( float stepSize ) { return true; }

   GlobalMethod( LPCTSTR name, GM_METHOD method ) : m_name( name ), m_method( method ), m_pQuery( NULL ),
      m_use( true ), m_timing( 0 ), m_id( m_nextID ), m_runTime( 0 ), m_extFnDLL( 0 ), m_extFn( NULL ) { m_nextID++; }

   virtual ~GlobalMethod( void ) { if ( m_extFnDLL > 0 ) AfxFreeLibrary( m_extFnDLL );  }

protected:
   CString   m_extSource;
   CString   m_extPath;
   CString   m_extFnName;
   HINSTANCE m_extFnDLL;
   DIRECTFN  m_extFn;

   bool CompileQuery( FlowContext*);

// shared data
public:
   bool DoesHRUPassQuery( int hruIndex ) { int mask = 1 << m_id; return m_hruQueryStatusArray[ hruIndex ] & mask ? true : false; }
   bool DoesIDUPassQuery( int iduIndex ) { int mask = 1 << m_id; return m_iduQueryStatusArray[ iduIndex ] & mask ? true : false; }

   // shared arrays ------------------------------->>    //  Definition                                 | Written by    | Read by   
                                                         //-----------------------------------------------------------------------------      
   static CArray< float, float > m_iduMuniDemandArray;   // daily values for each IDU (m3/day)          |               | WaterMaster
   static CArray< float, float > m_iduIrrRequestArray;   // daily values for each IDU (mm/day)          | EvapTrans     | EvapTrans, WaterMaster
   static CArray< float, float > m_iduVPDarray;          // daily values of VPD for each forested IDU,  | ETEquation    | EvapTrans
                                                         // from Penman-Monteith equation (kPa)   
	static CArray< float, float > m_iduAnnAvgMaxETArray;  // annual average values of MaxET mm /year     | EvapTrans     |
	static CArray< float, float > m_iduSeasonalAvgETArray;     // annual average values - mm/year        | EvapTrans     |
	static CArray< float, float > m_iduSeasonalAvgMaxETArray;  // annual average values of MaxET mm /year| EvapTrans     |
	static CArray< float, float > m_iduYrMuniArray;       // annual municipal m3/day					        | Watermaster
 
   static CArray< float, float > m_iduAllocationArray;   // m3/sec                                      | WaterMaster   | WaterMaster
	static CArray< float, float > m_iduIrrAllocationArray;   // m3/sec                                      | WaterMaster   | WaterMaster
	static CArray< float, float > m_iduNonIrrAllocationArray;   // m3/sec                                      | WaterMaster   | WaterMaster
   // Notes on shared arrays
   // 1) Memory and initialization of these arrays are managed by the GlobalMethodsManager - GlobalMethods should never initialize these arrays
   // 2) If a global method-derived class populates these arrays, they should always increment exisintg values, not overwrite existing values,
   //    to avoid overwriting other GlboalMethods contributions to these arrays

      // arrays for tracking which methods apply to which HRUs/IDUs
   static CArray< int,   int   > m_iduQueryStatusArray;  // or'd IDs of GlobalMethods instances for whose queries pass. 
   static CArray< int,   int   > m_hruQueryStatusArray;  // or'd IDs of GlobalMethods instances for whose queries pass. 
   static CArray< HRU*,  HRU*  > m_iduHruPtrArray;        // stores ptr to corresponding HRU for every IDU 

private:
   GlobalMethod( void ) { }   // don't let anyone use this constructor

};


////////////////////////////////////////////////////
// Standard global methods
////////////////////////////////////////////////////

class ReachRouting : public GlobalMethod
{
// valid methods include: GM_EULER, GM_RK4, GM_RKF, GM_KINEMATIC
public:
    ReachRouting( LPCTSTR name ) : GlobalMethod( name, GM_KINEMATIC ), m_reachTimeStep( 1.0f ) {  m_timing = GMT_REACH; }

    virtual bool Init(FlowContext* pFlowContext);
    virtual bool Step( FlowContext *pFlowContext );

   virtual bool SetTimeStep( float timestep ) { m_reachTimeStep = timestep;  return true; }


   virtual bool SetMethod( GM_METHOD method ) { if ( method != GM_EULER && method != GM_RK4 && method != GM_RKF && method != GM_KINEMATIC && method != GM_EXTERNAL ) return false;  m_method = method; return true; }

   static ReachRouting* LoadXml( TiXmlElement *pXmlReachRouting, LPCTSTR filename );
   static double Cloudiness(double SWunshaded_W_m2, int dayOfYear);
   static WaterParcel ApplyEnergyFluxes(WaterParcel origWP, double H2Oarea_m2, double netSW_W_m2,
      double H2Otemp_degC, double airTemp_degC, double VTSfrac, double cloudinessFrac, double windspeed_m_sec, double spHumidity, double RHpct,
      double& rEvap_m3, double& rEvap_kJ, double& rSW_kJ, double& rLW_kj);

public:
   float m_reachTimeStep;

protected:
   ParamTable* m_pHBVtable;

   void MoveWP(double volume_m3, WaterParcel* pFromWP, WaterParcel* pToWP);
   bool  SolveReachKinematicWave( FlowContext* );

   double GetLateralInflow( Reach *pReach );

   void PutLateralWP(Reach* pReach, int subreachNdx, double daily_net_subreach_lateral_flow_m3); // Allocates runoff and withdrawals to a subreach
   void PutLateralWP(Reach* pReach, int subreachNdx, WaterParcel runoffWP, double withdrawal_m3); 

   double GetReachFluxes( FlowContext*, Reach *pReach );

   double GetReachInflow( Reach *pReach, int subNode );
   WaterParcel GetReachInflowWP(Reach * pReach, int subnode);
   WaterParcel GetReachQdischargeWP(ReachNode* pReachNode);

   WaterParcel ApplyReachOutflowWP(Reach* pReach, int subnode, double timeStep);
   static double KinematicWave(double oldQ_cms, double upstreamInflow_cms, double lateralInflow_cms, double manningDepth_m, double wdRatio, double manningN, double slope, double length_m);

   double GetLateralSVInflow( Reach *pReach, int sv );
   double GetReachSVOutflow( ReachNode *pReachNode, int sv );   // recursive!!! for pahntom nodes
   static void GetReachDerivatives( double time, double timestep, int svCount, double *derivatives, void *extra );

   static double NetLWout_W_m2(double tempAir_degC, double cL, double tempH2O_degC, double RH_pct, double theta_vts); // net longwave radiation out of the reach
}; // end of class ReachRouting


class LateralExchange : public GlobalMethod
{
// valid types include: GM_NONE, GM_LINEAR_RESERVOIR
public:
   LateralExchange( LPCTSTR name ) : GlobalMethod( name, GM_NONE ) { m_timing=GMT_CATCHMENT; }
   
   virtual bool Step( FlowContext* );

   virtual bool SetMethod( GM_METHOD method ) { if ( method != GM_NONE && method != GM_LINEAR_RESERVOIR && method != GM_EXTERNAL ) return false;  m_method = method; return true; }
   
   static LateralExchange* LoadXml( TiXmlElement *pXmlLatExch, LPCTSTR path );
   
protected:
   bool SetGlobalHruToReachExchangesLinearRes( void );
};


class HruVertExchange : public GlobalMethod
{
// valid types include: GM_NONE, GM_LINEAR_RESERVOIR
public:
   HruVertExchange( LPCTSTR name ) : GlobalMethod( name, GM_NONE ) { m_timing = GMT_CATCHMENT; }

   virtual bool Step( FlowContext* );

   virtual bool SetMethod( GM_METHOD method ) { if ( method != GM_NONE && method != GM_BROOKS_COREY && method != GM_EXTERNAL ) return false;  m_method = method; return true; }
   
   static HruVertExchange* LoadXml( TiXmlElement *pXmlVertExch, LPCTSTR path );
   
protected:
   bool SetGlobalHruVertFluxesBrooksCorey( void );
};


class GroundWater : public GlobalMethod
{
// valid types include: GM_NONE, GM_MODFLOW
public:
    GroundWater( LPCTSTR name ) : GlobalMethod( name, GM_NONE ) { m_timing = GMT_CATCHMENT; }

   virtual bool Step( FlowContext* );

   virtual bool SetMethod( GM_METHOD method ) { if ( method != GM_NONE && method != GM_MODFLOW && method != GM_EXTERNAL ) return false;  m_method = method; return true; }
   
   static GroundWater* LoadXml( TiXmlElement *pXmlGW, LPCTSTR path );
   
protected:
   //bool SetGlobalHruVertFluxesBrooksCorey( void );
};


class EvapTrans : public GlobalMethod
{
public:
   EvapTrans( LPCTSTR name ); // : GlobalMethod( name, GM_PENMAN_MONTEITH ) { }
 //  m_pEvapTrans(NULL) { }
   ~EvapTrans( void );

   // Run() sets MaxET, AET values in the IDUs
   virtual bool Init     ( FlowContext*);
   virtual bool InitRun  ( FlowContext* );
   virtual bool StartYear( FlowContext* );          // start of Flow::StartYear() invocation 
   virtual bool StartStep( FlowContext* );          // start of Flow::StartStep() invocation 
   virtual bool Step      ( FlowContext* );
   virtual bool EndStep  ( FlowContext* );          // end of Flow timestep
   virtual bool EndYear  ( FlowContext* );          // end of a Flow::Run() invocation

   // valid methods include: GM_PENMAN_MONTEITH, GM_HARGREAVES, GM_FAO56
   virtual bool SetMethod( GM_METHOD method );

   // xml variables
   CString m_cropTablePath;
   CString m_soilTablePath;
	CString m_stationTablePath;

   float m_bulkStomatalResistance; // minimum stomatal resistance across all forest cover types.  s/m

   float m_latitude;
   float m_longitude;

   float m_irrigLossFactor;  // decimal percent of losses associated with crop demand

   static EvapTrans *LoadXml( TiXmlElement *pXmlEvapTrans, MapLayer*, LPCTSTR path );
   

public:
   FlowContext *m_flowContext;
   MapLayer * m_pIDUlayer;
   MapLayer * m_pHRUlayer;

protected:
   VDataObj *m_pCropTable;
   FDataObj *m_pSoilTable;
	FDataObj *m_pStationTable;

   // layer distributions?
	LayerDistributions *m_pLayerDists;

 // lookup tables for Crops
   IDataObj *m_phruCropStartGrowingSeasonArray;
   IDataObj *m_phruCropEndGrowingSeasonArray;
   FDataObj *m_phruCurrCGDDArray;
  
   FDataObj *m_pLandCoverCoefficientLUT;

	//optional at times
//	FDataObj  *m_pDailyReferenceET;   // for plotting at the end of run.

   
   // crop utility functions
   float UpdateCGDD(float cgddIn, bool isCorn, float tMax, float tMin, float tMean, float baseTemp);
   void CalculateCGDD(FlowContext *pFlowContext, HRU *pHRU, float baseTemp, int doy, bool isCorn, float &cGDD);
   float CO2byYear(int calendarYear, CString scenarioName);
   bool FillLookupTables(FlowContext* pFlowContext);

   void LookupLandCoverCoefficient(int row, float cGDD, float denominator, float &landCover_coefficient);

   ETEquation m_ETEq;

   // IDU columns
   int m_colLulc;
   int m_colIduSoilID;        // "SoilID"
   int m_colArea;             // "Area"

   int m_colAnnAvgMaxET;			// "PET_YR (mm/year)
   int m_colDailyET;					// "ET_DAY", daily ET rate (mm/day)
   int m_colDailyMaxET;				// "PET_DAY" (mm/day)
   int m_colSM_DAY;   // soil moisture (mm)

   int m_colDailySOILH2OEST;
   int m_colDailyIRR_STATE;
   int m_colDailyIRRRQST_D; // unsmoothed irrigation request (mm/day)
   int m_colDailyIRRACRQ_D; // smoothed irrigation request aka actual irrigation request (mm/day)
   int m_colDailyPRECIP;

   int m_colTMIN;
   int m_colTMIN_GROW;
   int m_colTMINGROAVG;
   int m_colPRCP_GROW;
   int m_colPRCPSPRING;
   int m_colPRCPWINTER;
   int m_colPRCP_JUN;
   int m_colPRCP_JUL;
   int m_colPRCP_AUG;

   int m_colIrrigation;				// IRRIGATION binary from Iffiragation decision
   int m_colIDUIndex;
	int m_colYieldReduction;		// Yield Reduction Fraction
   int m_colF_THETA;
   int m_colVPD;
   int m_colVPD_SCALAR;
   int m_colRAD_SW;              // Shortwave radiation, W m-2

   int m_colHruPV_HERE;

   bool m_calcFracCGDD;          // Daily fraction of CGDD / Potential CGDD
   float m_CO2_scalar;           // dimensionless, on the unit interval
   int m_TurnerScenario; 
   float m_atmCO2conc;           // [CO2] ppm in the atmosphere
   FDataObj m_CO2effectMetrics;  // an output var for CO2, CO2_scalar, and effective bulk stomatal resistance

public:
   int m_colLAI;
   int m_colLULC_A;
   int m_colAgeClass;
   int m_colPVT;
	int m_colAgrimetStationID;  // Agrimet Station ID

   float m_effBulkStomatalResistance;

protected:
   // soil table columns
   int m_colSoilTableSoilID;  // "SoilID"  
   int m_colWP;      // "WP"
	int m_colFC;      // "FC"  
   int m_colHbvET_MULT;

   // crop table columns
   int m_cropCount;
   int m_colCropTableLulc;   // "LULC"
   int m_plantingMethodCol;
   int m_plantingThresholdCol;
   int m_t30BaselineCol;
   int m_earliestPlantingDateCol;
   int m_gddBaseTempCol;
   int m_termMethodCol;
   int m_gddEquationTypeCol;
   int m_termCGDDCol;
   int m_termTempCol;
   int m_minGrowSeasonCol;
   int m_binToEFCCol;
   int m_cGDDtoEFCCol;
   int m_useGDDCol;
   int m_binEFCtoTermCol;
   int m_binMethodCol;
   int m_constCropCoeffEFCtoTCol;
	int m_depletionFractionCol;
	int m_yieldResponseFactorCol;
   int m_precipThresholdCol;
   int m_colPLANTDATE;           // doy (=day of year) for start of growing season
   int m_colEndGrowSeason;             // doy (=day of year) for end of growing season

	// station coefficients table columns

	int m_colStationTableID;
	int m_colStationCoeffOne;
	int m_colStationCoeffTwo;
	int m_colStationCoeffThree;
	int m_colStationCoeffFour;
	int m_colStationCoeffFive;

	float m_stationLongitude;
	float m_stationLatitude;
	int m_stationCount;

	// other variables
   float m_currReferenceET;
   int m_currMonth;

   map< int, float > m_hruCurrReferenceETMap;

   int m_runCount;      // used for averaging annual ET values accumulated during successive calls to run

   void CalculateTodaysReferenceET(FlowContext *pFlowContext, HRU *pHRU, unsigned short etMethod, float &referenceET, float sw_coef = 1.f, float lw_coef = 1.f);
	void GetHruET(FlowContext *pFlowContext, HRU *pHRU, int hruIndex);
   //float CalcRelHumidity(float specHumid, float tMean, float elevation);

   static SYSDATE m_dateEvapTransLastExecuted;
};

class DailyUrbanWaterDemand : public GlobalMethod
{
public:
	DailyUrbanWaterDemand(LPCTSTR name); // : GlobalMethod( name, GM_PENMAN_MONTEITH ) { }
	~DailyUrbanWaterDemand(void);

	// Run() sets MaxET, AET values in the IDUs
	virtual bool Init(FlowContext*);
	//virtual bool InitRun(FlowContext*);
	virtual bool StartYear(FlowContext*);    // start of Flow::StartYear() invocation 
	virtual bool StartStep(FlowContext*);    // start of Flow::StartStep() invocation 
	//virtual bool Step(FlowContext*);
	virtual bool EndStep(FlowContext*);      // end of Flow timestep
	//virtual bool EndYear(FlowContext*);      // end of a Flow::Run() invocation

	virtual bool SetMethod(GM_METHOD method) { m_method = GM_URBAN_DEMAND_DY; return true; }

	bool CalcDailyUrbanWaterDemand(FlowContext *pFlowContext);
	static DailyUrbanWaterDemand *LoadXml(TiXmlElement *pXmlDailyUrbWaterDmd, MapLayer *pIDULayer, LPCTSTR filename);
   static float UWindoorFraction(int doy);
   static float UWseasonalVolumeAdjustment(int doy);
	//-------------------------------------------------------------------  
	//------- urban water demand ----------------------------------------
	//-------------------------------------------------------------------  

protected:
	int   m_colDailyUrbanDemand;	// Calculated Daily Urband Water Demand m3/day
	int   m_colH2OResidnt;			// Annual Residential Demand ccf/day/acre
	int 	m_colH2OIndComm;			// Annual Residential Demand ccf/day/acre
	int   m_colIDUArea;				// IDU area from IDU layer
	int   m_colUGB;					// IDU UGB from IDU layer
	int   m_colWrexist;           // IDU attribute from IDU layer with WR information
   int   m_colPop;               // IDU POP from IDU layer
   // Big 8 Municipalities
	float m_iduMetroWaterDmdDy;   // UGB 40 Metro daily urban water demand ccf/day
	float m_iduEugSpWaterDmdDy;   // UGB 22 EugSp daily urban water demand ccf/day
	float m_iduSalKiWaterDmdDy;   // UGB 51 Salki daily urban water demand ccf/day
	float m_iduCorvlWaterDmdDy;   // UGB 12 Corvl daily urban water demand ccf/da
	float m_iduAlbnyWaterDmdDy;   // UGB 02 Albny daily urban water demand ccf/day
	float m_iduMcminWaterDmdDy;   // UGB 39 Mcmin daily urban water demand ccf/day
	float m_iduNewbrWaterDmdDy;   // UGB 47 Newbr daily urban water demand ccf/day
	float m_iduWoodbWaterDmdDy;   // UGB 71 Woodb daily urban water demand ccf/day
	
	FDataObj m_timeSeriesMunDemandSummaries; // for graph daily urban water demand (ccf/day)

};


class WaterAllocation : public GlobalMethod
{
public:
   // water rights related
   WaterMaster    *m_pWaterMaster;
   AltWaterMaster *m_pAltWM;

   //int m_surfaceHruLayer;    // index of layer to which watter is added
   //CString m_podTablePath;
   //CString m_pouTablePath;

public:
   WaterAllocation( LPCTSTR name, GM_METHOD method ) : GlobalMethod( name, method ),
      m_pWaterMaster( NULL ), m_pAltWM( NULL ) { m_timing = GMT_CATCHMENT; }

   ~WaterAllocation( void );

   virtual bool Init   ( FlowContext* );
   virtual bool InitRun( FlowContext* );
   virtual bool StartYear( FlowContext* );
   virtual bool StartStep(FlowContext*);
   virtual bool Step(FlowContext*);
   virtual bool EndStep( FlowContext* );   
   virtual bool EndYear( FlowContext* );

   virtual bool EndRun ( FlowContext* ) { return true; }

   virtual bool SetMethod( GM_METHOD method ) { if ( method != GM_EXTERNAL && method != GM_NONE && method != GM_WATER_RIGHTS && method != GM_EXPR_ALLOCATOR && method != GM_ALTWM ) { m_method=GM_NONE; return false; } m_method=method; return true; }

   static WaterAllocation *LoadXml( TiXmlElement *pXmlMethod, LPCTSTR path );  

protected:
   bool RunWaterRights( FlowContext* );
   bool RunAltWM( FlowContext* );
   bool RunExprAllocator( FlowContext* );
};


// a FluxExpr is a general purpose method for defining a flux.  In the current
// implementation, the "source" must be a stream reach, the sink
// must be an HRU or IDU.

enum FLUX_TYPE { FT_NONE=0, FT_SOURCE=1, FT_SINK=2, FT_SOURCESINK=3 };

class FluxSourceSink
{
public:
   int m_sourceIndex;      // record offset in the source layer
   CArray< int, int > m_sinkIndexArray;   // record offsets in the sink layer
   
   FluxSourceSink( void ) : m_sourceIndex( -1 ), m_sinkIndexArray() { }
};


class FluxExpr : public GlobalMethod
{
public:
   FluxExpr(LPCTSTR name);

   ~FluxExpr(void);

   virtual bool Init(FlowContext*);
   virtual bool InitRun(FlowContext*);
   virtual bool StartYear(FlowContext*);
   virtual bool StartStep(FlowContext*);
   virtual bool Step(FlowContext*);
   virtual bool EndStep(FlowContext*);
   virtual bool EndRun(FlowContext*) { return true; }
   virtual bool EndYear(FlowContext*);

   virtual bool SetMethod(GM_METHOD method) { if (method != GM_FLUX_EXPR) return false; m_method = method; return true; }

   static FluxExpr* LoadXml(TiXmlElement* pXmlEvapTrans, FlowModel*, MapLayer*, LPCTSTR path);

protected:
   PtrArray< FluxSourceSink > m_ssArray;

   FLUX_TYPE m_fluxType;

   bool m_isDynamic;
   int BuildSourceSinks(void);

   CString m_joinField;
   CString m_limitField;
   CString m_startDateField;
   CString m_endDateField;
   CString m_minInstreamFlowField;
   CString m_dailyOutputField;
   CString m_annualOutputField;
   CString m_unsatisfiedOutputField;
   CString m_expr;               // one-way straw=m3/sec, two way straw mm/day

   FLUX_DOMAIN m_sourceDomain; // 0=undefined, 1=catchment, 2=reach, 3=reservoir
   FLUX_DOMAIN m_sinkDomain;
   FLUX_DOMAIN m_valueDomain;
   VALUE_TYPE m_valueType;

   int m_colJoinSrc;
   int m_colJoinSink;
   int m_colStartDate;
   int m_colEndDate;
   int m_colLimit;   // maximum flux value (NOT CURRENTLY USED)
   int m_colMinInstreamFlow;
   int m_colDailyOutput;
   int m_colAnnualOutput;
   int m_colReachXFLUX_D;   // m3/sec
   int m_colReachXFLUX_Y;     // m3/sec, averaged over year
   int m_colReachSPRING_CMS;
   int m_colSourceUnsatisfiedDemand;     // m3/sec of unsatisified demand, populated daily
   int m_startDate;     // used if column isn't specified
   int m_endDate;

   float m_withdrawalCutoffCoeff;
   float m_lossCoeff;
   float m_temp_C;
   bool  m_collectOutput;

   MapLayer* m_pSourceLayer;
   MapLayer* m_pSinkLayer;
   MapLayer* m_pValueLayer;

   MapExprEngine* m_pMapExprEngine;  // check memory management for expr types
   MapExpr* m_pMapExpr;

   FDataObj* m_pValueData;    // for time series, expression types

   QueryEngine* m_pSourceQE;
   QueryEngine* m_pSinkQE;

   CString m_sourceQuery;
   CString m_sinkQuery;

   Query* m_pSourceQuery;
   Query* m_pSinkQuery;

   // layer distributions?
   LayerDistributions* m_pLayerDists;



   // outputs
   float m_dailySatisfiedDemand;     // m3/day
   float m_dailySinkDemand;          // m3/day
   float m_dailyUnsatisfiedDemand;   // m3/day
   float m_cumSatisfiedDemand;       // m3/day
   float m_cumSinkDemand;            // m3/day
   float m_cumUnsatisfiedDemand;     // m3/day
   float m_totalSinkArea;            // m2

   float m_annDailyAvgFlux;         // m3/day -collected once per year, annual average

   //FDataObj m_annDailyAvgFluxData;  // m3/day
   FDataObj* m_pDailyFluxData;
   FDataObj* m_pCumFluxData;

protected:
   int m_stepCount;

}; // end of class FluxExpr


class Spring : public GlobalMethod // a special case of flux expressions, tailored to representation of springs flowing to reaches
{
public:
   Spring(LPCTSTR name, int comid, double flow_cms, double temp_C);

   ~Spring(void);

   virtual bool Init(FlowContext*);
   virtual bool InitRun(FlowContext*);
   virtual bool StartYear(FlowContext*);
   virtual bool StartStep(FlowContext*);
   virtual bool Step(FlowContext*);
   virtual bool EndStep(FlowContext*);
   virtual bool EndRun(FlowContext*) { return true; }
   virtual bool EndYear(FlowContext*);

   virtual bool SetMethod(GM_METHOD method) { if (method != GM_SPRING) return false; m_method = method; return true; }

   static Spring* LoadXml(TiXmlElement* pXmlEvapTrans, FlowModel*, MapLayer*, LPCTSTR path);

protected:
   CString m_springName;
   int m_springCOMID;
   double m_springFlow_cms;
   double m_temp_C;

   Reach* m_pReach;
   int m_colReachSPRING_CMS;   // m3/sec
   int m_colReachSPRINGTEMP; // temperature of spring water, deg C
   int m_colReachIN_RUNOFF; // lateral inflow to the reach from runoff, cms
}; // end of class Spring


class ExternalMethod : public GlobalMethod
{
public:
   ExternalMethod( LPCTSTR name ) : GlobalMethod( name, GM_EXTERNAL ) { }

   virtual bool Init     ( FlowContext* );      // start of Envision session
   virtual bool InitRun  ( FlowContext* );      // start of an Envision Run
   virtual bool StartYear( FlowContext* );      // start of Flow::Run() invocation
   virtual bool StartStep( FlowContext* );      // start of Flow::Run() invocation
   virtual bool Step   ( FlowContext* );      // during Flow::Run()
   virtual bool EndStep  ( FlowContext* );      // end of a Flow::Run() invocation
   virtual bool EndYear  ( FlowContext* );      // start of Flow::Run() invocation
   virtual bool EndRun   ( FlowContext* );      // end of an Envision run

   CString m_initInfo;  
   float m_parameter1;
   float m_parameter2;
   FlowModel * m_pFlowModel;

   virtual bool SetMethod( GM_METHOD method ) { if ( method == GM_EXTERNAL || method == GM_NONE ) { m_method=method; return true; } return false; }

   static ExternalMethod *LoadXml( TiXmlElement *pXmlMethod, LPCTSTR path );

};



class GlobalMethodManager
{
protected:
   static PtrArray< GlobalMethod > m_gmArray;

public:
   static int AddGlobalMethod( GlobalMethod *pMethod ) { return (int) m_gmArray.Add( pMethod ); }

   static bool Init     ( FlowContext* );    // start of Envision session
   static bool InitRun  ( FlowContext* );    // start of an Envision Run
   static bool StartYear( FlowContext* );    // start of Flow::Run() invocation (was PreRun)
   static bool StartStep( FlowContext* );    // start of a Flow timestep
   static bool Step     ( FlowContext* );    // during Flow::Run() (may be called multiple times, depending on m_time value)
   static bool EndStep  ( FlowContext* );    // end of a Flow timestep
   static bool EndYear  ( FlowContext* );    // end of a Flow::Run() invocation
   static bool EndRun   ( FlowContext* );    // end of an Envision run

   static bool SetTimeStep( float timestep );


   static float m_runTime;

};