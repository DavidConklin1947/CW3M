#pragma once


#include "FlowContext.h"
#include "GlobalMethods.h"

#include <EnvExtension.h>
#include <MapLayer.h>
#include <reachtree.h>
#include <FDataObj.h>
#include <PtrArray.h>
#include <QueryEngine.h>
#include <Vdataobj.h>
#include <IntegrationBlock.h>
#include <DataAggregator.h>
#include <MapExprEngine.h>
#include "GDALWrapper.h"
#include "GeoSpatialDataObj.h"
#include <randgen\Randunif.hpp>
#include <randgen\Randnorm.hpp>
#include "..\..\GDAL\include\netcdf.h"
#include <UNITCONV.H>
#include <WaterParcel.h>
//x #include <ScienceFcns.h>

#include <exception>
using namespace std;


#ifdef BUILD_FLOW
#define FLOWAPI __declspec( dllexport )
#else 
#define FLOWAPI __declspec( dllimport )
#endif

/* reservoir ID numbers and USACE three letter codes
1 HCR Hills Creek Reservoir
2 LOP Lookout Point Reservoir
3 DEX Dexter Reservoir
4 FAL Fall Creek Reservoir
5 DOR Dorena Reservoir
6 COT Cottage Grove Reservoir
7 FRN Fern Ridge Reservoir
8 CGR Cougar Reservoir
9 BLU Blue River Reservoir
10 GPR Green Peter Reservoir
11 FOS Foster Reservoir
12 DET Detroit Reservoir
13 BCL Big Cliff Reservoir
*/
#define HCR 1
#define LOP 2
#define DEX 3
#define FAL 4
#define DOR 5
#define COT 6
#define FRN 7
#define CGR 8
#define BLU 9
#define GPR 10
#define FOS 11
#define DET 12
#define BCL 13

#define BOX_SNOWPACK 0
#define BOX_SNOW 0
#define BOX_MELT 1
#define BOX_STANDING_H2O 1
#define BOX_SURFACE_H2O 1
#define BOX_NAT_SOIL 2
#define BOX_IRRIG_SOIL 3
#define BOX_FAST_GW 4
#define BOX_SLOW_GW 5

#define NON_WETLAND_WETNESS_TOKEN -1000.
#define NON_WETLAND_TOKEN 0
#define NOMINAL_LOW_FLOW_CMS 0.010 /* 10 liters of water per sec */
#define NOMINAL_LOW_WATER_LITERS_PER_METER 1 /* one liter of water per meter of stream length*/
#define NOMINAL_MINIMUM_SOIL_WATER_CONTENT 0.001 /* 1 mm of water content per meter of soil depth*/
#define LITERS_PER_M3 1000
//x #define SPECIFIC_HEAT_H2O 4.187 /* kJ/(kg degC) */
//x #define DENSITY_H2O 998.2 /* kg/m^3 */
#define DEFAULT_REACH_H2O_TEMP_DEGC 8.f
#define DEFAULT_SOIL_H2O_TEMP_DEGC 5.f
#define DEFAULT_MIN_SKIN_TEMP_DEGC 1.f /* minimum temperature of liquid stream water in contact with the air (the air temp itself may be much colder) */

// Default value of Manning roughness coefficient
// from Table 9-6 in S.L. Dingman Physical Hydrology 2nd ed. (2002) (p.428)
// originally from V.T. Chow Open Channel Hydraulics (1959)
// 0.030 is the normal value in s/ft^(1/3) for 
// "Minor streams (top width at flood stage < 100 ft)
// Streams on plain
// 1. Clean, straight, full stage, no riffles or deep pools"
// Our units are s/m^(1/3) instead of s/ft^(1/3)
// 1 ft^(1/3) = 0.673 m^(1/3)
// 0.030 s/ft^(1/3) = 0.030 / 0.673 = 0.045 s/m^(1/3)
#define DEFAULT_MANNING_N 0.045f /* s/m^(1/3), = 0.030 s/ft^(1/3) */

#define BEERS_LAW_K 0.5

#define HruCOMID gpFlowModel->m_colHruCOMID
#define HruFIELD_CAP gpFlowModel->m_colHruFIELD_CAP
#define HruGW_FASTBOX gpFlowModel->m_colHruGW_FASTBOX
#define HruGW_SLOWBOX gpFlowModel->m_colHruGW_SLOWBOX
#define HruHBVCALIB gpFlowModel->m_colHruHBVCALIB
#define HruHRU_ID gpFlowModel->m_colHruHRU_ID
#define HruIRRIG_SOIL gpFlowModel->m_colHruIRRIG_SOIL
#define HruBOXSURF_M3 gpFlowModel->m_colHruBOXSURF_M3
#define HruH2OMELT_M3 gpFlowModel->m_colHruH2OMELT_M3
#define HruH2OSTNDGM3 gpFlowModel->m_colHruH2OSTNDGM3
#define HruNAT_SOIL gpFlowModel->m_colHruNAT_SOIL
#define HruSNOW_BOX gpFlowModel->m_colHruSNOW_BOX

#define ReachDEPTH_MIN gpFlowModel->m_colReachDEPTH_MIN
#define ReachDIRECTION gpFlowModel->m_colReachDIRECTION
#define ReachHBVCALIB gpFlowModel->m_colReachHBVCALIB
#define ReachQ_CAP gpFlowModel->m_colReachQ_CAP
#define ReachQSPILL_FRC gpFlowModel->m_colReachQSPILL_FRC
#define ReachQ2WETL gpFlowModel->m_colReachQ2WETL
#define ReachRADSWGIVEN gpFlowModel->m_colReachRADSWGIVEN
#define ReachREACH_H2O gpFlowModel->m_colReachREACH_H2O
#define ReachRES_H2O gpFlowModel->m_colReachRES_H2O
#define ReachRES_TEMP gpFlowModel->m_colReachRES_TEMP
#define ReachTEMP_H2O gpFlowModel->m_colReachTEMP_H2O
#define ReachTOPOELEV_E gpFlowModel->m_colReachTOPOELEV_E
#define ReachTOPOELEV_S gpFlowModel->m_colReachTOPOELEV_S
#define ReachTOPOELEV_W gpFlowModel->m_colReachTOPOELEV_W
#define ReachVEG_HT_L gpFlowModel->m_colReachVEG_HT_L
#define ReachVEG_HT_R gpFlowModel->m_colReachVEG_HT_R
#define ReachVGDNSGIVEN gpFlowModel->m_colReachVGDNSGIVEN
#define ReachVEGHTGIVEN gpFlowModel->m_colReachVEGHTGIVEN
#define ReachVEGHTREACH gpFlowModel->m_colReachVEGHTREACH
#define ReachWIDTH_CALC gpFlowModel->m_colReachWIDTH_CALC
#define ReachWIDTH_MIN gpFlowModel->m_colReachWIDTH_MIN
#define ReachWIDTHGIVEN gpFlowModel->m_colReachWIDTHGIVEN
#define ReachWIDTHREACH gpFlowModel->m_colReachWIDTHREACH

/*! \mainpage A brief introduction to Flow:  A framework for the development of continuous-time simulation models within Envision
 *
 * \section intro_sec Introduction
 *
 * Flow is a modeling framework that allows a user of Envision to develop spatially and temporally distributed simulation models that can take advantage...
 * The user interacts with the framework through an xml-based format that allows for the definition of state variables, fluxes and solution procedures.

 * Basic idea - provides a plug-in framework for hydrologic modeling.  The framework assumes that
 * that the baisc construct for hydrologic modeling consists of catchments that intercept water and route that
 * water to a stream network.  Catchments can have any number of fluxes that contribute or remove
 * water from the catchment
 *
 * \section install_sec The Flow framework provides the following:
 *
 * \par 1. Two spatial representations relevant to hydrological modeling
 *
 * -# an instream linear network consisting of a multi-rooted tree
 * -# an upslope catchment coverage, consisting of polygons defined by a shape file
 *  
 * \par 2: a set of interface methods for solving the system of ODEs that results
 *
 *  etc.
 */



// Basic idea - provides a plug-in framework for hydrologic modeling.  The framework assumes that
// that the baisc construct for hydrologic modeling consists of catchments that intercept water and route that
// water to a stream network.  Catchments can have any number of fluxes that contribute or remove
// water from the catchment

// The Flow framework provides the following:
//
// 1) Two spatial representations relevant to hydrol modeling 
//    - a instream linear network consisting of a multi-rooted tree
//    - an upslope catchment coverage, consisting of polygons defined be a shape file
//
// 2) a set of interface methods for 





//--------------------------------------------------------------------------------------------------
// Network simplification process:
//
// basic idea: iterate through the the stream network starting with the leaves and
// working towards the roots.  For each reach, if it doesn't satisfy the stream query
// (meaning it is to be pruned), move its corresponding HRUs to the first downstream 
// that satisfies the query.
//
// The basic idea of simplification is to aggregate Catchments using two distinct methods: 
//   1) First, aggregate upstream catchments for any reach that does not satisify a
//      user-defined query.  This is accomplished in SimplifyTree(), a recursive function 
//      that iterates through a reach network, looking for catchments to aggregate based 
//      on a reach-based query.  For any reach that DOES NOT satisfy the query, all catchments 
//      associated with all upstream reaches are aggregated into the catchment associated with
//      the reach with the failed query.  
//
//      SimplifyTree() combines upstream catchments (deleting pruned Catchments) and prunes 
//           all upstream Reaches (deleted) and ReachNodes (converted to phantom nodes).
//
//   2) Second, minimum and/or maximum area constraints area applied to the catchments.  This is 
//      an iterative process that combines Catchments that don't satisfy the constraints.
//      The basic rule is:
//         if the catchment for this reach is too small, combine it with another
//         reach using the following rules:
//           1) if one of the upstream catchment is below the minimum size, add this reaches
//              catchment to the upstream reach.  (Note that this works best with a root to leaf search)
//           2) if there is no upstream capacity, combine this reach with the downstream reach (note
//              that this will work best with a leaf to root, breadth first search.)
//
//       ApplyCatchmentConstraints() applies these rules.  For any catchment that is combined,
// 
//--------------------------------------------------------------------------------------------------

class WaterParcel;
class Wetland;
class Shade_a_latorData;
class Reach;
class Reservoir;
class HRULayer;
class Flux;
class FluxInfo;
class FlowModel;
class HRU;
class Catchment; 
class StateVar;
class VideoRecorder;
class ResConstraint;
class TopoSetting;

#include <MovingAvg.h>


// global functions
float GetManningDepthFromQ( Reach *pReach, double Q, float wdRatio );  // ASSUMES A SPECIFIC CHANNEL GEOMETRY
float GetManningDepthFromQ(double Q, float wdRatio, float n, float slope);  // ASSUMES A SPECIFIC CHANNEL GEOMETRY
FLUXSOURCE ParseSource( LPCTSTR sourceStr, CString &path, CString &source, HINSTANCE &hDLL, FLUXFN &fn );
float GetVerticalDrainage( float wc );
float GetBaseflow(float wc);
float CalcRelHumidity(float specHumid, float tMean, float elevation);

// exception handling

//struct FlowException : public std::exception
//   {
//   TCHAR msg[ 256 ];
//   const TCHAR *what () const throw ()
//   {
//   return "Flow Exception";
//   }
//};
 
//int main()
//{
//  try
//  {
//    throw MyException();
//  }
//  catch(MyException& e)
//  {
//    std::cout << "MyException caught" << std::endl;
//    std::cout << e.what() << std::endl;
//  }



//-----------------------------------------------------------------------------
//------------------------ F L U X   D E S C R I P T O R S  -------------------
//-----------------------------------------------------------------------------

class FluxInfo
{
friend class FlowProcess;

public:
   FluxInfo();
   ~FluxInfo();

public:
   CString   m_name;
   CString   m_description;
   CString   m_path;            ///< database path or DLL path
   CString   m_initInfo;
   bool      m_use;

   FLUXTYPE   m_type;           ///< FT_UNDEFINED, FT_PRECIP
   FLUXSOURCE m_dataSource;     ///< FS_UNDEFINED=0, FS_FUNCTION=1, FS_DATABASE=2

   // connections
   CString   m_sourceQuery;     ///< source of straw - empty for a one-way straw adding water
   CString   m_sinkQuery;       ///< sink of straw - empty for a one-way straw removing water
   
   FLUXLOCATION  m_sourceLocation;     ///< FL_NDEFINED=0, FL_REACH=1, FL_HRULAYER=2, FL_RESERVOIR=3, FL_IDU;
   FLUXLOCATION  m_sinkLocation;       ///< these are set in LoadXML() 

   Query    *m_pSourceQuery;     ///< associated queries (memory managed by QueryEngine)
   Query    *m_pSinkQuery;       ///< these are set in ???? (memory managed by QueryEngine)

   int       m_sourceLayer;   ///< only defined for FL_HRULAYER types - zero-based HRU Layer this flux is associated with
   int       m_sinkLayer;     ///< only defined for FL_HRULAYER types - zero-based HRU Layer this flux is associated with
   
   FLUXFN    m_pFluxFn;       ///< for FS_FUNCTION type
   CString   m_fnName;
   HINSTANCE m_hDLL;

   FDataObj *m_pFluxData;         ///< for FS_DATABASE type (memory managed by this object)
   CString   m_fieldName;
   int       m_col;

   // associated state variables.  the first is always water.  (memory managed elsewhere)
   CArray< StateVar*, StateVar* > m_stateVars;  ///< note: water represented with a NULL????? in first array element 
   CMap< int, int, StateVar*, StateVar* > m_stateVarsMap;  ///< key = uniqueID
   
   bool IsSource( void ) { return m_sourceLocation != FL_UNDEFINED; }
   bool IsSink  ( void ) { return m_sinkLocation != FL_UNDEFINED; }

   CString m_initFn;
   CString m_initRunFn;
   CString m_endRunFn;

   FLUXINITFN    m_pInitFn;
   FLUXINITRUNFN m_pInitRunFn;
   FLUXENDRUNFN  m_pEndRunFn;
   
   // cumulative flux tracking
   float  m_totalFluxRate; // instantaneous, reflects total for the given time step over all locations
   SVTYPE m_annualFlux;    // over the current year, integrated from total flux rate
   SVTYPE m_cumFlux;       // over the run
};


class FLOWAPI Flux
{
public:   
   Flux( FluxInfo *pFluxInfo, StateVar *pStateVar ) 
      : m_pFluxInfo( pFluxInfo ), m_pStateVar( pStateVar ), m_value( 0 )/*, m_valid( false ) */ { }

   //----------------------------------------------------------
   /// Notes on flux evaluation
   //
   /// Flux Type        Value interpretation
   // -------------    -----------------------------------------
   /// Source only      positive values mean an addition to the state variable, negative values a subtraction>
   /// Sink only        positive values mean a subtraction from the state variable, negative values an addition>
   /// Source AND Sink  interpretation consistent with above, depending on where the evaluation is occurring>
   //-----------------------------------------------------------
   float Evaluate( FlowContext* );

   bool IsSource( void ) { ASSERT( m_pFluxInfo != NULL );  return m_pFluxInfo->IsSource(); }
   bool IsSink  ( void ) { ASSERT( m_pFluxInfo != NULL );  return m_pFluxInfo->IsSink(); }

   FLUXLOCATION GetSourceType( void ) { ASSERT( m_pFluxInfo != NULL ); return m_pFluxInfo->m_sourceLocation; }
   FLUXLOCATION GetSinkType( void )   { ASSERT( m_pFluxInfo != NULL ); return m_pFluxInfo->m_sinkLocation; }
   
public:
   FluxInfo *m_pFluxInfo;     // FluxInfo this flux is associated with (memory managed by FlowModel::m_fluxInfoArray)
   StateVar *m_pStateVar;     // state variable associated with this flux. (memory managed by ???

protected:
   float m_value;    // current value of this flux
//   bool  m_valid;    // is current evaluation good?
};

/*x
class FLOWAPI WaterParcel
{
public:
   WaterParcel();
   WaterParcel(double volume_m3, double temperature_degC);
   ~WaterParcel() {}

   void Discharge(WaterParcel outflowWP);
   WaterParcel Discharge(double volume_m3);
   bool Evaporate(double evap_volume_m3, double evap_energy_kJ);
   void MixIn(WaterParcel inflow);
   double WaterTemperature();
   double WaterTemperature(double thermalEnergy_kJ);
   static double WaterTemperature(double volume_m3, double thermalEnergy_kJ);
   double ThermalEnergy();
   double ThermalEnergy(double temperature_degC);
   static double ThermalEnergy(double volume_m3, double temperature_degC);
   static double SatVP_mbar(double tempAir_degC);

//private:
   double m_volume_m3;
   double m_temp_degC;
}; // end of class WaterParcel
x*/

// a FluxContainer is a object (e.g. HRU, Reach) that can contain fluxes. 
class FluxContainer
   // Somewhat counterintuitively, negative fluxes represent quantities going into the flux container,
   // and positive fluxes represent fluxes coming out of the flux container.
{
   friend class FlowModel;

protected:
   FluxContainer() : m_globalHandlerFluxValue(0.0f), m_nanOccurred(false) { }
   ~FluxContainer() { }

protected:
   float m_globalHandlerFluxValue;        // current value of the global flux  - m3/day

public:
   bool m_nanOccurred; // true when a not-a-number has occurred

   float GetFluxValue( ) { return m_globalHandlerFluxValue; } 
   void  ResetFluxValue( ) 
   { 
      m_globalHandlerFluxValue=0.0f;
   } 

   // array of attached fluxes
   PtrArray< Flux > m_fluxArray;    // memory managed by this object)

   int AddFlux( FluxInfo *pFluxInfo, StateVar *pStateVar ); // m3/day

   int AddFlux( Flux *pFlux ) { return (int) m_fluxArray.Add( pFlux ); }   // assumes flux already allocated somewhere else

   int GetFluxCount( void ) { return (int) m_fluxArray.GetSize(); }
   Flux *GetFlux( int i ) { return m_fluxArray[ i ]; }  // m3/day

   int FindFlux( Flux *pFlux )   { for ( int i=0; i < (int) m_fluxArray.GetSize(); i++ ) if ( m_fluxArray[ i ] == pFlux ) return i; return -1; }
   int RemoveFlux( Flux *pFlux, bool doDelete );
};


class FLOWAPI ParamTable
   {
   friend class FlowModel;
   friend class FlowProcess;

   protected:
      ParamTable(void) : m_pTable(NULL), m_pInitialValuesTable(NULL), m_iduCol(-1), m_type(DOT_NULL) { }

   public:
      ~ParamTable(void) { if (m_pTable) delete m_pTable; /* if ( m_pInitialValuesTable ) delete m_pInitialValuesTable; */ }

   protected:
      DataObj *m_pTable;
      DataObj *m_pInitialValuesTable;

   protected:
      CMap< VData, VData&, int, int > m_lookupMap;
   public:
      int CreateMap();

      CString m_description;
      CString m_fieldName;
      int     m_iduCol;
      DO_TYPE m_type;  //  { DOT_VDATA, DOT_FLOAT, DOT_INT, DOT_NULL, DOT_OTHER };

      // external methods 
      DataObj *GetDataObj(void) { return m_pTable; }
      DataObj *GetInitialDataObj(void) { return m_pInitialValuesTable; }

      int GetFieldCol(LPCTSTR field) { if (m_pTable) return m_pTable->GetCol(field); else return -1; }

      bool Lookup(int key, int col, int   &value) { return Lookup(VData(key), col, value); }
      bool Lookup(int key, int col, float &value) { return Lookup(VData(key), col, value); }

      bool Lookup(VData key, int col, int   &value);
      bool Lookup(VData key, int col, float &value);
   };

//class ExternalMethod
//{
//public:
//   CString m_name;
//   CString m_path;
//   //CString m_entryPt;
//
//   CString m_entryPtInit;
//   CString m_entryPtInitRun;
//   CString m_entryPtRun;
//   CString m_entryPtEndRun;
//   CString m_initInfo;
//
//   int     m_timing; 
//   bool    m_use;
//
//   HMODULE       m_hDLL;
//   FLOWINITFN    m_initFn;
//   FLOWINITRUNFN m_initRunFn;
//   FLOWRUNFN     m_runFn;
//   FLOWENDRUNFN  m_endRunFn;
//
//   ExternalMethod( void ) : m_timing( 2 ), m_use( true ), m_hDLL( 0 ), m_initFn( NULL ), 
//      m_initRunFn( NULL ), m_runFn( NULL ), m_endRunFn( NULL ) { }
//
//};
//

class StateVar
{
public: 
   CString m_name;
   int     m_uniqueID;
   int     m_index;

   bool    m_isIntegrated;
   float   m_scalar;
   int     m_cropCol;
   int     m_soilCol;
   VDataObj *m_paramTable; //this table stores parameters.  
   PtrArray <ParamTable> m_tableArray;//the array stores tables for each application strategy for the pesticide
   //it maps back to the idu (crop) and so is a ParamTable, rather than a dataobj

   StateVar(void) : m_uniqueID(-1), m_isIntegrated(true), m_scalar(1.0f), m_cropCol(-1), m_soilCol(-1){}
   };


//-- StateVarCountainer ----------------------------------------
//
// stores arrays of state variables, manages state var memory
// First state variable is always quantity of water, units=m3
//--------------------------------------------------------------

class StateVarContainer
{
friend class FlowModel;
friend class ReachRouting;

protected:
   StateVarContainer() : m_svArray( NULL ), m_svArrayTrans( NULL ), m_svIndex( -1 ) { }
   ~StateVarContainer() { if ( m_svArray != NULL ) delete [] m_svArray; if ( m_svArrayTrans != NULL ) delete [] m_svArrayTrans; }

protected:
   SVTYPE *m_svArray;       // contains state variables (memory managed here)
   int     m_svIndex;       // index in the associated integration block for the first statevar contained by this container
   
public:
   SVTYPE  GetStateVar( int i ) { return m_svArray[ i ]; }
   void    SetStateVar( int i, SVTYPE value ) { m_svArray[ i ] = value; }

   int   AllocateStateVars( int count ) { ASSERT( count > 0 ); if ( m_svArray != NULL ) delete [] m_svArray; if ( m_svArrayTrans != NULL ) delete [] m_svArrayTrans; m_svArray = new SVTYPE[ count ]; m_svArrayTrans = new SVTYPE[ count ]; return count; }
   SVTYPE *m_svArrayTrans;   // ???? (memory managed here)
   void  AddExtraSvFluxFromGlobalHandler(int svOffset, float value ) { m_svArrayTrans[svOffset] += value; } 

  //  void  AddFluxFromGlobalHandler( float value ) { m_globalHandlerFluxValue += value; } 

   double GetExtraSvFluxValue(int svOffset ) { return m_svArrayTrans[svOffset]; } 
   void  ResetExtraSvFluxValue(int svOffset ) { m_svArrayTrans[svOffset]=0.0f; } 
   };


// groundwater
class FLOWAPI Groundwater : public FluxContainer
{
public:
   SVTYPE m_volumeGW;    // m3
   
   Groundwater( void ) : m_volumeGW(0.0f) { } 
};


class ResConstraint
{
public:
   ResConstraint( void ) : m_pRCData( NULL ) /*, m_pControlPoint( NULL )*/ { }
   //ResConstraint( const ResConstraint &rc ) { *this = rc; }

   ~ResConstraint( void ) { if ( m_pRCData != NULL ) delete m_pRCData; }

   //ResConstraint &operator = ( const ResConstraint &rc )
   //   { m_constraintFileName = rc.m_constraintFileName; m_pRCData = NULL;  if( rc.m_pRCData ) m_pRCData = new FDataObj( *m_pRCData );
   //     m_type = rc.m_type; m_comID = rc.m_comID;  return *this; }


   CString   m_constraintFileName;
   FDataObj *m_pRCData;          // (memory managed here)
   //Reach    *m_pControlPoint;
   RCTYPE     m_type;   //Type = min (1) , max (2) , IncreasingRate (3), DecreasingRate (4)
   int       m_comID;
  
};


class ZoneInfo
{
public:
   ZONE m_zone;
   //CArray< ResConstraint*, ResConstraint* > m_resConstraintArray;  // (memory managed elsewhere)
   PtrArray< ResConstraint > m_resConstraintArray;    ///???

   ZoneInfo( void ) : m_zone( ZONE_UNDEFINED ) { m_resConstraintArray.m_deleteOnDelete = false; }
};


//-----------------------------------------------------------------------------
//---------------------------- H R U L A Y E R   ------------------------------
/// Represents the fundamental unit of calculation for upslope regions.  An HRU
/// is comprised of n HRULayers representing project specific state variables.
/// These may include water on vegetation, snow, water in a particular soil layer,
/// etc.  They are specified based upon project-specific spatial queries
//-----------------------------------------------------------------------------

/// The fundamental unit describing upslope state variables

class FLOWAPI HRULayer : public FluxContainer, public StateVarContainer
{
public:
    HRULayer();
    ~HRULayer( void ) { }
   // State Variables
   SVTYPE m_volumeWater;                // m3
   float m_soilThickness_m ;  // thickness of soil layer (m)

   // Next variable is usually 1.  However, for topsoil, the irrigated part of the HRU is in one layer, and the non-irrigated part is in another layer.
   // That allows for keeping track of soil moisture separately in the irrigated part vs. the non-irrigated part.
   float m_HRUareaFraction; // fraction of HRU's total area in which this HRULayer exists

   CArray< float, float > m_waterFluxArray;  // for gridded state variable representation

   // additional variables
   float m_contributionToReach;
   float m_verticalDrainage;
   float m_horizontalExchange;//only used for grids.  Populated by Flow PlugIn
   
   // calculated stuff
   float m_addedVolume_m3;
   float m_wc;                         // water content (mm/mm)
   float m_wDepth;                     // water content (mm)

   // summary fluxes
   float m_volumeLateralInflow;        // m3/day
   float m_volumeLateralOutflow;
   float m_volumeVerticalInflow;
   float m_volumeVerticalOuflow;

   // general info
   HRU *m_pHRU;            // containing HRU    (memory managed in FlowModel::m_hruArray)
   int  m_layer;           // zero-based layer number - 0 = first layer
   HRULAYER_TYPE m_type;   // soil, snow or veg
   
   // methods for computed variables
   float GetSaturatedDepth();       // m
   float GetSWC();                  // m3/day                     // soil water content

   static MTDOUBLE m_mvWC;
   static MTDOUBLE m_mvWDepth;
   static MTDOUBLE m_mvESV;

public:
   HRULayer( HRU *pHRU );
   HRULayer( HRULayer &l ) { *this=l; }
   
   HRULayer & operator = ( HRULayer &l );

   bool Update( void );

   Reach *GetReach( void );

   bool AddFluxFromGlobalHandler( float value, WFAINDEX index  );  // m3/day

   bool CheckForNaNs(CString callerName, bool OKflag)
      {
      if (OKflag) return(false);
      CString msg; msg.Format("*** CheckforNaNs() found evidence of NaNs in call from %s", callerName); Report::LogMsg(msg);
      return(true);
      } // end of CheckForNaNs()

   float GetExtraStateVarValue( int k ) {return (float)m_svArray[k];}
   };
///-----------------------------------------------------------------------------
//----------------------------------- H R U -----------------------------------
//-----------------------------------------------------------------------------
/// HRU - fundamental unit of spatial representation.  This corresponds to a 
/// collection of similar polygons in the Maplayer. It contains 1 or more 
/// HRULayers that contain the relevant state variable information for the model
//-----------------------------------------------------------------------------
/// A collection of HRU Layers.

class FLOWAPI HRU
{
public:
   HRU();
   ~HRU();  

   int m_id;            // unique identifier for this HRU
   int m_hruNdx; // index of this HRU in the HRUarray[], also row number in the HRU.shp file

   bool m_standingH2Oflag; // true => there exists an IDU in this HRU which has standing water (can only happen for wetland IDUs)
   bool m_snowpackFlag; // true => there exists an IDU in this HRU which has snow or meltwater on the ground
   double m_wetlandArea_m2; // Update each year in FlowModel::StartYear()
   double m_snowpackArea_m2; // = m_HRUtotArea_m2 - m_wetlandArea_m2 i.e. the part of the HRU that may have snow and meltwater on it, always > 0
   double m_infiltrationFromStandingH2O_m3; // Calculate in HBV daily process. Use to reduce WETNESS in Wetland daily process.

   float m_HRUtotArea_m2;  // total area of the HRU
   double m_frc_naturl; // FRC_NATURL attribute in HRU layer
   float m_HRUeffArea_m2;  // effective area of the HRU ( = total area X natural fraction )

   // climate variables
   float m_precip_wateryr;    // mm/year
   float m_precip_yr;         // mm/year, accumulates throughout the year
   MovingAvg m_precip_10yr;   // mm/year (avg) 20 yr moving avg
   float m_precipCumApr1;     // mm accumulated from Jan 1 through April 1

   float m_gwRecharge_yr;       // mm
   float m_gwFlowOut_yr;

   float m_rainfall_yr;       // mm/year, accumulate throughout the year
   float m_snowfall_yr;       // mm/year, accumulate throughout the year
   double m_areaIrrigated;
   double m_fracIrrigated;

   float m_temp_yr;           // C, average over year
   MovingAvg m_temp_10yr;     // C, 20 year mov avg

   float m_soilTemp;
   float m_biomass;

   // SWE-related variables
   float     m_depthMelt;          // depth of water in snow (m)
   float     m_depthSWE;           // depth of ice in snow (m)
   float     m_depthApr1SWE_yr;    // (m)
   MovingAvg m_apr1SWE_10yr;       // 20 year moving avg (m)
   float m_addedVolume_m3;
   bool m_nanOccurred;
   
   Vertex m_centroid;   //centroid of the HRU
   float m_elevation;
   float m_currentPrecip;     // mmH2O/day = rain thrufall + rain evap + snow thrufall + snow evap
   float m_rainThrufall_mm;   // rain thrufall, mmH2O/day
   float m_rainEvap_mm;   // liquid precip which evaporates off the canopy before reaching the ground, mmH2O/day
   float m_snowThrufall_mm; // snow thrufall, mmH2O SWE per day
   float m_snowEvap_mm; // snow which sublimates off the canopy before reaching the snowpack, mmH2O SWE per day
   float m_melt_mm;
   float m_refreezing_mm;
   float m_infiltration_mm;
   float m_rechargeToIrrigatedSoil_mm;
   float m_rechargeToNonIrrigatedSoil_mm;
   float m_rechargeTopSoil_mm;
   float m_rechargeToUpperGW_mm;
   float m_Q0_mm; // mm H2O across entire area of HRU, not just across natural area
   float m_Q2_mm; // mm H2O across entire area of HRU, not just across natural area
   float m_aquifer_recharge_mm; // daily recharge to the aquifer, mm
   float m_aquifer_recharge_yr_mm;
   float m_percolation_mm; // mm H2O across entire area of HRU, not just across natural area
   float m_currentAirTemp;
	float m_currentMinTemp;
	float m_TMAX;
	float m_SOLARRAD;
	float m_SPHUMIDITY;
	float m_WINDSPEED;
	float m_RELHUMIDITY;
   float m_currentET;         // mm/day
   float m_currentMaxET;      // mm/day
   float m_currentRunoff;     // mm/day
   float m_currentGWRecharge; // mm/day
   float m_currentGWFlowOut;  // mm/day
   float m_currentSediment;
   float m_maxET_yr;          // mm/year
   float m_elws;                //elevation (map units) of the water surface
   float m_et_yr;             // mm/year      
   float m_runoff_yr;         // mm/year
   float m_meanLAI;           // dimensionless

   int m_climateIndex;  //the index of the climate grid cell representing the HRU
   int m_climateRow;    // row, column for this HRU in the climate data grid
   int m_climateCol;

   int m_demRow;  //for gridded state variable representation
   int m_demCol;  //for gridded state variable representation
//   CArray< float, float> m_waterFluxArray;  //for gridded state variable representation
   bool m_lowestElevationHRU;

   Catchment *m_pCatchment;        // (memory managed in FlowModel::m_catchmentArray)

   PtrArray< HRULayer > m_layerArray;     // (memory managed locally)
   CArray< HRU*, HRU* > m_neighborArray;        // (memory managed by FlowModel::m_hruArray)

   CUIntArray m_polyIndexArray; // Holds the indices into the IDU shapefile for the IDUs in the HRU.
   CUIntArray m_reachNdxArray; // Holds the indices into the Reach shapefile for the reaches associated with this HRU.

   // the following static members hold value for model variables
   static MTDOUBLE m_mvDepthMelt;  // volume of water in snow
   static MTDOUBLE m_mvDepthSWE_mm;   // volume of ice in snow
   static MTDOUBLE m_mvTopSoilH2O_mm; // hruVolTopSoilH2O
   static MTDOUBLE m_mvShallowGW_mm; // hruVolSGW
   static MTDOUBLE m_mvDeepGW_mm; // hruVolDGW
   static MTDOUBLE m_mvCurrentPrecip;
   static MTDOUBLE m_mvRainThrufall_mm;
   static MTDOUBLE m_mvRainEvap_mm;
   static MTDOUBLE m_mvSnowThrufall_mmSWE;
   static MTDOUBLE m_mvSnowEvap_mmSWE;
   static MTDOUBLE m_mvMelt_mm; // mm of snowmelt in the current timestep
   static MTDOUBLE m_mvRefreezing_mm; 
   static MTDOUBLE m_mvInfiltration_mm;
   static MTDOUBLE m_mvRechargeToIrrigatedSoil_mm;
   static MTDOUBLE m_mvRechargeToNonIrrigatedSoil_mm;
   static MTDOUBLE m_mvRechargeTopSoil_mm;
   static MTDOUBLE m_mvRechargeToUpperGW_mm;
   static MTDOUBLE m_mvQ0_mm;
   static MTDOUBLE m_mvQ2_mm;
   static MTDOUBLE m_mvPercolation_mm;
   static MTDOUBLE m_mvCurrentRecharge;
   static MTDOUBLE m_mvCurrentGwFlowOut;
   static MTDOUBLE m_mvHRU_TO_AQ_mm;

   static MTDOUBLE m_mvCurrentAirTemp;
	static MTDOUBLE m_mvCurrentMinTemp;
   static MTDOUBLE m_mvCurrentET;
   static MTDOUBLE m_mvCurrentMaxET;
   static MTDOUBLE m_mvCumET; 
   static MTDOUBLE m_mvCumRunoff;
   static MTDOUBLE m_mvCumMaxET;   
   static MTDOUBLE m_mvCumP; 
   static MTDOUBLE m_mvElws; 
   static MTDOUBLE m_mvCurrentSediment;
   static MTDOUBLE m_mvCumRecharge;
   static MTDOUBLE m_mvCumGwFlowOut;


public:
   int GetLayerCount( void ) { return (int) m_layerArray.GetSize(); }
   HRULayer *GetLayer( int i ) { return m_layerArray[ i ]; }
   int AddLayers( int soilLayerCount, int snowLayerCount, int vegLayerCount, float initWaterContent, float initTemperature, bool grid );
   bool DistributeToReaches(float amount);
   double Att(int hruCol);
   int AttInt(int hruCol);
   float AttFloat(int hruCol);
   void SetAtt(int col, double attValue);
   void SetAttInt(int col, int attValue);
   void SetAttFloat(int col, float attValue);
   bool WetlSurfH2Ofluxes(double precip_mm, double fc, double Beta, double* pPrecip2WetlSurfH2O_m3, double* pWetl2TopSoil_m3, double* pWetl2SubSoil_m3, double* pWetl2Reach_m3);
 }; // end of class HRU



//-----------------------------------------------------------------------------
//---------------------------- C A T C H M E N T ------------------------------
//-----------------------------------------------------------------------------
/// A catchment consists of one or more HRUs, representing areal 
/// subdivision of the catchment.  It is a distinct watersheds.  It may be 
/// connected to a groundwater system.
/// They may also be connected to zero or one reaches.
//-----------------------------------------------------------------------------

/// A collection of model units that are connected to a reach.

class FLOWAPI Catchment
{
public:
   // data - static
   int   m_id;
   float m_area;

   // data - dynamic
   float m_currentAirTemp;
   float m_currentPrecip;
   float m_currentThroughFall;
   float m_currentSnow;
   float m_currentET;
   float m_currentGroundLoss;
   float m_meltRate;
   float m_contributionToReach;    // m3/day
   CArray< HRU*, HRU* > m_catchmentHruArray;     // (memory managed in FlowModel::m_hruArray)


protected:
   // layers
   // topology - catchments (not currently used)
   CArray< Catchment*, Catchment* > m_upslopeCatchmentArray;      // (memory managed in FlowModel::m_catchmentArray)
   Catchment *m_pDownslopeCatchment;

   // topology - reach
public:
   Reach *m_pReach;     // ptr to associated Reach.  NULL if not connected to a reach  (memory managed by FlowModel::m_reachArray)
                        
   // topology - groundwater
   Groundwater *m_pGW;  // not used????  managed by???

public:
   // Constructor/destructor
   Catchment( void );

   int  GetHRUCountInCatchment( void )   { return (int) m_catchmentHruArray.GetSize(); }
   //int  AddHRU( HRU *pHRU )   { m_hruArray.Add( pHRU ); pHRU->m_pCatchment = this; return (int) m_hruArray.GetSize(); }
   HRU *GetHRUfromCatchment( int i )       { return m_catchmentHruArray[ i ]; }
   //void RemoveHRU( int i )    { return m_hruArray.RemoveAt( i ); }
   //void RemoveAllHRUs( void ) { m_area = 0; m_hruArray.RemoveAll(); }

   void ComputeCentroid( void );
};


class ReachSubnode : public SubNode, public StateVarContainer
{
public:
   void SetSubreachGeometry(double volume_m3, double wdRatio);
   void SetSubreachGeometry(double volume_m3, double dummy, double widthGiven_m);

   // Values which remain the same over time
   double m_subreach_length_m;
   double m_min_width_m;
   double m_min_depth_m;
   double m_wd_ratio;
   double m_min_surf_area_m2;
   double m_min_volume_m3;
   double m_midpt_elev_mASL;
   double m_aspect_deg;
   int m_aspect_cat;
   double m_topo_shade;

   // Values which change from day to day
   double m_veg_shade;
   double m_bank_shade;
   SVTYPE m_previousVolume;
   double  m_lateralInflow;       // m3/day
   WaterParcel m_dischargeWP; // m_dischargeWP.m_volume_m3 should be maintained equal to m_discharge * SEC_PER_DAY
   double  m_discharge;           // m3/sec; should be maintained equal to m_dischargeWP.m_volume_m3/SEC_PER_DAY 
   int m_dischargeDOY; // day index of day to which this discharge applies (Jan 1 = 0)
   double  m_previousDischarge;   // m3/sec;
   WaterParcel m_waterParcel;
   WaterParcel m_previousWP;
   WaterParcel m_runoffWP; // lateral flow into the subreach
   double m_withdrawal_m3; // lateral flow out of the subreach, at the temperature of the water in the subreach
   double m_subreach_width_m;
   double m_subreach_depth_m;
   double m_manning_depth_m;
   double m_subreach_surf_area_m2; // water surface area used for converting W/m2 to kJ
   double m_sw_kJ; // today's incoming shortwave energy, net of shading and cloudiness effects
   double m_lw_kJ; // today's net longwave energy, positive for outgoing, negative for incoming

   // These two can't be expressed as a WaterParcel because the energy includes the latent heat of vaporization.
   double m_evap_m3;
   double m_evap_kJ;

   WaterParcel m_addedVolTodayWP; // amount added today to keep compartment from going negative
   WaterParcel m_addedVolYTD_WP; // amount added so far this year to keep compartment from going negative
   double m_addedDischarge_cms; // amount added to ensure discharge is always > 0
   bool m_nanOccurred;
 
   static SubNode *CreateInstance( void ) { return (SubNode*) new ReachSubnode; }

      ReachSubnode(void) : SubNode(), StateVarContainer(), m_discharge(0.11), m_dischargeWP(0.11 * SEC_PER_DAY, DEFAULT_REACH_H2O_TEMP_DEGC),
         m_previousDischarge(0.11), m_previousVolume(0.0),
         m_waterParcel(0, 0),
      m_addedVolTodayWP(0,0), m_addedVolYTD_WP(0,0), m_addedDischarge_cms(0), m_nanOccurred(false) { }
   virtual ~ReachSubnode( void ) { }

};


class TopoSetting // Topographical Setting class
{
public:
   TopoSetting() {};
   TopoSetting(double elev_m, double topoElev_E_deg, double topoElev_S_deg, double topoElev_W_deg, double lat_deg, double long_deg);
   ~TopoSetting() {};

   double ShadeFrac(int jday0, double* pRadSWestimate_W_m2, double direction_deg, double reachWidth_m, double vegHt_m);
   double VegShade_pct(double elev_deg, double azimuth_deg, double direction_deg, double reachWidth_m, double vegHt_m);
   bool IsTopoShaded(double solarElev_deg, double solarAzimuth_deg);
   bool IsVegShaded(double solarElev_deg, double solarAzimuth_deg);
   static double SolarDeclination_deg(int jday0);
   double SolarElev_deg(int jday0, double time_hr);
   static double SolarElev_deg(double lat_deg, int jday0, double time_hr);
   double SolarAzimuth_deg(int jday0, double time_hr);
   double OpticalAirMassThickness(double solarElev_deg);
   double DiffuseFrac(double C_I, int jday0);
   double VegElevEW_deg(double direction_deg, double reach_width_m, double veg_ht_m);
   double VegElevNS_deg(double direction_deg, double reach_width_m, double veg_ht_m);

public:
   double m_elev_m; // elevation above sea level
   double m_topoElevE_deg, m_topoElevS_deg, m_topoElevW_deg;
   double m_vegElevEW_deg, m_vegElevNS_deg;
   double m_topoViewToSky;
   double m_lat_deg, m_long_deg;
   double m_vegDensity; // in the sense of Shade-a-lator = the fraction of direct shortwave which is intercepted by the vegetation before it gets to the stream
}; // end of class TopoSetting


class FLOWAPI Reach : public ReachNode, public FluxContainer          // extends the ReachNode class
{
public:  
   Reach();
   virtual ~Reach();

   ReachSubnode *GetReachSubnode( int i ) { return (ReachSubnode*) m_subnodeArray[ i ]; }
   static ReachNode *CreateInstance( void ) { return (ReachNode*) new Reach; }

   Reservoir *GetReservoir( void ) { return (Reservoir*) m_pReservoir; }
   float GetManningDepthFromQ( double Q, float wdRatio );  // ASSUMES A SPECIFIC CHANNEL GEOMETRY
   WaterParcel GetSubreachDischargeWP(int subnode);
   WaterParcel GetReachDischargeWP(); // Calculates and returns Q_DISCHARG
   double Evap_m_s(int subreachNdx, double rad_sw_net_W_m2, double net_lw_out_W_m2, double temp_air_degC, double ws_m_sec, double sphumidity);
   double GetUpstreamInflow();
   bool GetUpstreamInflow(double &QLeft, double &QRight);
   double SubreachNetRad_kJ(int subreachIndex); // Totals up the incoming shortwave and outgoing longwave from this subreach.
   bool CalcReachVegParamsIfNecessary();
   double GetSubreachViewToSky_frac(int subreachNdx);
   double GetSubreachArea_m2(int subreachNdx);
   float GetCatchmentArea( void );
   double NominalLowFlow_cms();
   double NominalMinWidth_m();

   static double LatentHeatOfVaporization_MJ_kg(double temp_H2O_degC);
   static double Evap_m_s(double tempH2O_degC, double swIn_W_m2, double lwOut_W_m2, double tempAir_degC, double ws_m_sec, double sphumidity);

   bool AccumAdditions(WaterParcel incomingWP);
   bool AccumWithdrawals(double withdrawal_volume_m3);
   bool  AddFluxFromGlobalHandler(float value);

   bool CheckForNaNs(CString callerName, bool OKflag)
      {
      if (OKflag) return(false);
      CString msg; msg.Format("*** CheckforNaNs() found evidence of NaNs in call from %s", callerName); Report::LogMsg(msg);
      return(true);
      } // end of CheckForNaNs()

   public:
   WaterParcel m_additionsWP;
//   double m_withdrawals_m3;

public:
   // reach-level parameters
   double Att(int col); 
   int AttInt(int col);
   void SetAtt(int col, double attValue);
   void SetAttInt(int col, int attValue);

   int m_reachArrayNdx; // index into gpFlowModel->m_reachArray
   int m_wetlNdx; // index into gpFlowModel->m_wetlArray[]; not the same as WETL_ID; -1 if the reach isn't associated with a wetland
   double m_q2wetl_cms;

   TopoSetting m_topo;
   double m_reach_volume_m3;
   float m_wdRatio;
   float m_cumUpstreamArea;   // cumulative upslope area, in map units, above this reach in the network
   float m_cumUpstreamLength;
   float m_instreamWaterRightUse;  //used to keep track of instream water right use for this reach
   double m_availableDischarge;//keep track of the amount of water diverted from the reach during each timestep
   double m_rad_lw_kJ; // today's net longwave energy out (>0) or in (<0) to the reach
   // These next two can't be expressed as a WaterParcel because the energy includes the latent heat of vaporization.
   double m_reach_evap_m3;
   double m_reach_evap_kJ;

   int m_climateIndex;
   int m_IDUndxForReach; // index to an IDU in the IDU layer which contains a vertex of this reach

   double m_addedDischarge_cms;
   WaterParcel m_reachAddedVolumeWP; // amount added so far this year to keep compartment from going negative

   // kinematic wave parameters
   float m_alpha;
   float m_beta;
   float m_n;              // Mannings "N"

   // summary variables
   //float m_lateralInflow;         // flow rate into this reach
   double m_meanYearlyDischarge;     // ft3/sec (cfs)
   double m_maxYearlyDischarge;    
   double m_sumYearlyDischarge;

   bool m_isMeasured;

   CArray< Catchment*, Catchment* > m_catchmentArray;

   Reservoir *m_pReservoir;            // (memory managed in FlowModel::m_reservoirArray)

   FDataObj *m_pStageDischargeTable;   // (memory managed here)

   // the following static members hold value for model variables
   static MTDOUBLE m_mvCurrentStreamFlow;
	static MTDOUBLE m_mvCurrentStreamTemp;
   static MTDOUBLE m_mvCurrentTracer;
   static MTDOUBLE m_mvInstreamWaterRightUse;  // Used to keep track of instream water right use for this reach
   static MTDOUBLE m_mvQ_DIV_WRQ;  // Flow divided by regulatory demand
   static MTDOUBLE m_mvINSTRM_REQ;  // regulatory required flow, cms
   static MTDOUBLE m_mvRESVR_H2O; // volume of water in reservoir on this reach, m3 H2O

   static MapLayer* pLayer;
  };


class FLOWAPI LinkMV // Link model variables
   {
   public:
   static MTDOUBLE m_mvMAX_FLOW;
   static MTDOUBLE m_mvLINK_ID;
   }; // end of class LinkMV


enum ResType
{
	ResType_FloodControl = 1,
	ResType_RiverRun = 2,
	ResType_CtrlPointControl = 4
};

class FLOWAPI Reservoir : public FluxContainer, public StateVarContainer
{
friend class FlowModel;

public:
   Reservoir(void);
   ~Reservoir(void);	

   // XML variables
   CString m_name;
   int     m_id;
   bool m_in_use;
   CString m_dir;
   CString m_avdir;
   CString m_rcdir;
   CString m_redir;
   CString m_rpdir;
   CString m_cpdir;
   CString m_streamTempDir;

   int     m_col;
   float   m_dam_top_elev;       // Top of dam elevation
   float   m_inactive_elev;      // Inactive elevation. Below this point, no water can be released
   float   m_fc1_elev;           // Top of primary flood control zone
   float   m_fc2_elev;           // Seconary flood control zone (if applicable)
   float   m_fc3_elev;           // Tertiary flood control zone (if applicable)
   float   m_buffer_elevation;   //Below this elevation, rules for buffer zone apply
   float   m_tailwater_elevation;
   float   m_turbine_efficiency;
   float   m_minOutflow;
   float   m_maxVolume;
	int	  m_releaseFreq;        // how often the reservoir will release averages over this number of days
   
   float   m_gateMaxPowerFlow;   //Physical limitation of gate
   float   m_maxPowerFlow;       //Operations limit, depends on rules applied
   float   m_minPowerFlow;       //Operations limit, depends on rules applied
   float   m_powerFlow;          //Flow through Powerhouse in current timestep
   
   float   m_gateMaxRO_Flow;     //Physical limitation of gate
   float   m_maxRO_Flow;         //Operations limit, depends on rules applied
   float   m_minRO_Flow;         //Operations limit, depends on rules applied
   float   m_RO_flow;            //Flow through Regulating Outlet in current timestep
   
   float   m_gateMaxSpillwayFlow;//Physical limitation of gate
   float   m_maxSpillwayFlow;    //Operations limit, depends on rules applied
   float   m_minSpillwayFlow;    //Operations limit, depends on rules applied
   float   m_spillwayFlow;       //Flow through Spillway in current timestep

	ResType m_reservoirType;      //  enumerated type which indicates Reservoir Type 
   LPCTSTR m_activeRule;           //Rule that is controlling release (last implemented).
   float m_constraintValue; // Constraint value associated with active rule.
   int m_zone;
   int m_daysInZoneBuffer;

   double   m_inflow; // m3 
   WaterParcel m_inflowWP;
   double   m_outflow; // m3 
   WaterParcel m_outflowWP;
   float   m_elevation;          //current pool elevation
   float   m_power; // Current hydropower output, MW

   int     m_filled;  // -1 means never reached, updated annually

   float m_probMaintenance;
   float m_resSWmult; // tuning knob for average topographic shading

   /////////////////////Read in from ResSIM output files for model comparison.  mmc 5/22/2013///////////
   float m_resSimInflow;
   float m_resSimOutflow;
   float m_resSimVolume;
   CString resSimActiveRule;
   ///////////////////////////////////////////////

   SVTYPE m_volume;   // this is the same as m_svArray[ 0 ].
   WaterParcel m_resWP;

   CString m_areaVolCurveFilename;
   CString m_ruleCurveFilename;
   CString m_bufferZoneFilename;
   CString m_releaseCapacityFilename;
   CString m_RO_CapacityFilename;
   CString m_spillwayCapacityFilename;
	CString m_rulePriorityFilename; 
   CString m_ressimFlowOutputFilename;     //Inflow, outflows and pool elevations
   CString m_ressimRuleOutputFilename;     //Active rules
   

   bool ProcessRulePriorityTable( void );
	void InitializeReservoirVolume(float volume) 
   { 
      m_volume = volume; 
      m_resWP = WaterParcel(volume, DEFAULT_REACH_H2O_TEMP_DEGC);
   } // end of InitializeReservoirVolume()
   
   PtrArray< ZoneInfo > m_zoneInfoArray;              // (memory managed here)
   PtrArray< ResConstraint > m_resConstraintArray;    // (memory managed here)

   FDataObj *m_pStreamTempProfile;                    // (memory managed here)

protected:
   Reach *m_pReach;   // associated downstream reach (NULL if Reservoir not connected to Reaches) (memory managed in FlowModel::m_reachArray)
   Reach *m_pFlowfromUpstreamReservoir;   // For testing with ResSIM.  2s reservoir (Foster and Lookout) (memory managed in FlowModel::m_reachArray)
                                          // have upstream inputs from other reservoirs.  This points to the stream
                                          // reaches containing those inflow (just upstream).
   void InitDataCollection( void );
   void CollectData();
   
   float GetPoolElevationFromVolume();
   float GetPoolSurfaceAreaFromVolume_ha();
   float GetPoolVolumeFromElevation(float elevation);
   float GetTargetElevationFromRuleCurve( int dayOfYear );
   float GetBufferZoneElevation( int dayOfYear );
   WaterParcel GetResOutflowWP(Reservoir* pRes, int dayOfYear);
   void  AssignReservoirOutletFlows( Reservoir *pRes, float outflow );
   void  UpdateMaxGateOutflows( Reservoir *pRes, float currentPoolElevation );
   float CalculateHydropowerOutput( Reservoir *pRes );

   ResConstraint *FindResConstraint( LPCTSTR name );
   
protected:
   FDataObj *m_pAreaVolCurveTable;                    // (memory managed here)
   FDataObj *m_pRuleCurveTable;                       // (memory managed here)
   FDataObj *m_pBufferZoneTable;                      // (memory managed here)
   FDataObj *m_pCompositeReleaseCapacityTable;        // (memory managed here)
   FDataObj *m_pRO_releaseCapacityTable;              // (memory managed here)
   FDataObj *m_pSpillwayReleaseCapacityTable;         // (memory managed here)
	FDataObj *m_pDemandTable;									// (memory managed here)
   VDataObj *m_pRulePriorityTable;                    // (memory managed here)

   // data collection
   FDataObj *m_pResData;                              // (memory managed here)
   //FDataObj *m_pResMetData;
   
   //Comparison with ResSIM
   FDataObj *m_pResSimFlowOutput;                     //Stores output from the ResSim model (from text file)
   VDataObj *m_pResSimRuleOutput;
   FDataObj *m_pResSimFlowCompare;
   VDataObj *m_pResSimRuleCompare;                    // Enables side by side comparison of ResSim and Envision outputs

};



class ControlPoint
{
public:
   CString m_name;
   CString m_dir;
   CString m_controlPointFileName;
   CString m_reservoirsInfluenced;
   int m_id;
   int m_location;
   RCTYPE m_type;
   float m_localFlow;   ///Used in comparison to ResSim
   int localFlowCol;   //  Used in comparison to ResSim

   FDataObj *m_pResAllocation;                  // stores allocations for each attached reservoir
   FDataObj *m_pControlPointConstraintsTable;

   CArray< Reservoir*, Reservoir* > m_influencedReservoirsArray;
   Reach *m_pReach;   // associated reach  
  
   // methods
   ControlPoint(void) : m_id( -1 ), m_location( -1 ), m_type( RCT_UNDEFINED ), 
      m_pResAllocation( NULL ), m_pControlPointConstraintsTable( NULL ), m_pReach( NULL ) { }

   ~ControlPoint( void );

   bool InUse( void ) { return m_pReach != NULL; }
};





class ReachSampleLocation
{
public:
   ReachSampleLocation( void ) 
      : m_name(_T("")), m_fieldName(_T("")), m_id(-1), m_iduCol(-1), m_pReach(NULL), m_pHRU( NULL ),m_pMeasuredData(NULL), row(-1),col(-1) { }

   ~ReachSampleLocation( void ) {}

   CString m_name;
   CString m_fieldName;
   int m_iduCol;
   int m_id;
   Reach* m_pReach;
   HRU* m_pHRU;
   FDataObj *m_pMeasuredData;
   int row;
   int col;

};


// ModelObject stores 
class ModelOutput
{
public:
   ModelOutput() : m_pQuery( NULL ), m_pMapExpr( NULL ), m_pDataObj( NULL ), m_pDataObjObs( NULL), m_modelType( MOT_UNDEFINED ), m_modelDomain( MOD_UNDEFINED ), m_inUse( true ), 
      m_value( 0 ), m_totalQueryArea( 0 ), m_number(0), m_esvNumber(0), m_pLinkLayer(NULL){ }
   ~ModelOutput() { if ( m_pDataObj != NULL ) delete m_pDataObj; if ( m_pDataObjObs != NULL ) delete m_pDataObjObs; }   // note that m_pQuery is managed by the queryengine

   ModelOutput( ModelOutput &mo ) { *this = mo; }

   ModelOutput& operator = (ModelOutput& mo) {
      m_name = mo.m_name; m_queryStr = mo.m_queryStr;
      m_exprStr = mo.m_exprStr; m_pQuery = NULL, m_pMapExpr = NULL; m_modelType = mo.m_modelType;
      m_modelDomain = mo.m_modelDomain; m_inUse = mo.m_inUse; m_pDataObj = NULL; m_pDataObjObs = NULL;
      m_value = 0; m_totalQueryArea = 0; m_number = 0; m_esvNumber = 0;
      m_dayNumberOfObsFor1900Jan1 = mo.m_dayNumberOfObsFor1900Jan1;  m_pLinkLayer = mo.m_pLinkLayer; return *this;
   }
   
   CString m_name;
   CString m_nameObs;
   CString m_queryStr;
   CString m_exprStr;

   Query    *m_pQuery;           // memory managed by QueryEngine
   MapExpr  *m_pMapExpr;         // memory managed by MapExprEngine

   MOTYPE    m_modelType;
   MODOMAIN  m_modelDomain;
   bool      m_inUse;
   FDataObj *m_pDataObj;
   FDataObj *m_pDataObjObs;
   int m_dayNumberOfObsFor1900Jan1; // day number of observation for 1/1/1900
   MapLayer * m_pLinkLayer;

   float m_value;
   float m_totalQueryArea;    // total area satisfied by the query, 

   bool InitModelOutput(MapLayer * pIDULayer);
   bool InitIDUdomain(MapLayer* pIDUlayer);
   bool InitReachDomain(MapLayer* pReachLayer);
   bool InitHRUdomain(MapLayer* pHRUlayer);
   bool InitXHRUdomain(MapLayer* pHRUlayer);
   bool InitLinkDomain(MapLayer* pLinkLayer);
   bool InitNodeDomain(MapLayer * pNodeLayer);
   bool InitSubcatchDomain(MapLayer * pSubcatchLayer);

   int m_number;
   int m_esvNumber;

};


class ModelOutputGroup : public PtrArray< ModelOutput >
{
public:
   ModelOutputGroup( void ) : PtrArray< ModelOutput >(), m_pDataObj( NULL ), 
         m_moCountIdu( 0 ), m_moCountHru( 0 ), m_moCountHruLayer( 0 ), m_moCountReach( 0 ), m_moInterval(MOI_UNDEFINED) { }

   ~ModelOutputGroup( void ) { if ( m_pDataObj ) delete m_pDataObj; m_pDataObj = NULL; }

   ModelOutputGroup( ModelOutputGroup &mog ) { *this = mog; }

   ModelOutputGroup &operator = ( ModelOutputGroup &mog );
   
   CString   m_name;
   FDataObj *m_pDataObj;

   int m_moCountIdu;
   int m_moCountHru;
   int m_moCountHruLayer;
   int m_moCountReach;
   MOINTERVAL m_moInterval;
};


class ModelVar
{
public:
   CString  m_label;
   MapVar  *m_pMapVar;
   MTDOUBLE m_value;

   ModelVar( LPCTSTR label ) : m_label( label ), m_pMapVar( NULL ), m_value( 0 ) { }
};


class ParameterValue
{
public:
   ParameterValue( void ) 
      : m_table(_T("")), m_name(_T("")), m_value(0.5f), m_minValue(0.0f), m_maxValue(1.0f), m_distribute(true) { }
   ~ParameterValue( void ) {}
   CString m_table;
   CString m_name;
   float m_value;
   float m_minValue;
   float m_maxValue;
   bool m_distribute;
};


class ClimateDataInfo
{
public:
   int GetTimeIndex(SYSDATE tgtDate, int maxDaysInYear);
   void ScaleTheValues(float * fieldVals, size_t numVals);
   CDTYPE m_type;    // see enum
   GeoSpatialDataObj *m_pDataObj;
   PtrArray<GeoSpatialDataObj> m_pDataObjArray;

   CString m_path;
   CString m_varName;

   bool m_IDUbased;
   bool m_row0inSouth; // applies only to gridded data, not IDU-based data. true => the first row of data is at the south edge of the grid.
   int m_firstYear;
   int m_lastYear;
   int m_mostRecentIndexYear; // A calendar year will go here
   int m_recentYearIndex; // Offset from the beginning of the file to most recent index year
   int m_ncid, m_varid_idu_id, m_varid_data, m_dimid_idu, m_dimid_time;
   nc_type m_nctype;
   size_t m_size_idu, m_size_time;

   // for converting from the form of the raw data to floats
//x   // e.g. float = (int16 + m_addOffsetAttribute) * m_scaleFactorAttribute
   // e.g. float = int16 * m_scaleFactorAttribute + m_addOffsetAttribute
   float m_scaleFactorAttribute; // This value is from the "scale_factor" attribute in the NetCDF file, used for converting INT16 values back to floats.
   float m_addOffsetAttribute; // This value is from the "add_offset" attribute in the NetCDF file, used for converting INT16 values back to floats.

   // for converting from the units of the input file to the units used by the model aka the interpreted value
   // interpreted value = raw_value_as_float * m_scale_factor + m_offset
   // e.g. deg C = deg K * 1.0 - 273.15
   CString m_unitsAttribute;;
   float m_scaleFactor; // This value is the final scale factor to be used in calculating the interpreted value.
   float m_offset; // This is the final offset to be used in calculating the interpreted value (e.g. for converting deg K to deg C).

   int m_maxDaysInClimateYear;

   bool  m_useDelta;
   float m_delta;

   ClimateDataInfo(void) :
      m_type(CDT_UNKNOWN), m_pDataObj(NULL), m_pDataObjArray(NULL),
      m_IDUbased(false), m_row0inSouth(false), m_firstYear(-1), m_lastYear(-1), m_mostRecentIndexYear(-1), m_recentYearIndex(-1),
      m_ncid(-1), m_varid_idu_id(-1), m_varid_data(-1), m_nctype(NC_NAT), m_size_idu(-1), m_size_time(-1), m_dimid_idu(-1), m_dimid_time(-1),
      m_scaleFactor(1.f), m_scaleFactorAttribute(1.f), m_offset(0.f), m_addOffsetAttribute(0.f), m_unitsAttribute(""), m_maxDaysInClimateYear(366), m_useDelta(false), m_delta(0) { }
   ~ClimateDataInfo(void)
   { 
      if (m_pDataObj) delete m_pDataObj; 
   }
};


class ClimateScenario
{
public:
   CString m_name;
   int     m_id;
   int m_maxDaysInClimateYear;
   int m_firstYear;
   int m_lastYear;
   bool m_row0inSouth; // applies only to gridded data, not IDU-based data. true => the first row of data is at the south edge of the grid.

   PtrArray< ClimateDataInfo > m_climateInfoArray;  
   
   //CArray< ClimateDataInfo*, ClimateDataInfo* > m_climateInfoArray;  // memory managed by FlowModel
   CMap< int, int, ClimateDataInfo*, ClimateDataInfo* > m_climateDataMap;

};

struct SCENARIO_MACRO
{
   //int envScenarioIndex;      // Envision scenarion index associated with this macro
   CString envScenarioName;
   CString value;             // substitution text

   //SCENARIO_MACRO( void ) : envScenarioIndex( -1 ) { }
};


class ScenarioMacro
{
public:
   CString m_name; 
   CString m_defaultValue;

   PtrArray< SCENARIO_MACRO > m_macroArray;
};


class Wetland
{
   friend class FlowModel;

public:
   Wetland(int wetlID);
   ~Wetland() {  };
   double ReachH2OtoWetland(int reachComid, double H2OtoWetl_m3); // Returns volume of water remaining when wetland is at its capacity.

public:
   int m_wetlID;
   int m_wetlNdx;
   double m_wetlArea_m2;

   // m_wetlIDUndxArray is an ordered list of the IDU polygon indices of the IDUs in the wetland.
   // Note these are IDU index values, not IDU_ID values.
   // The IDUs are in the order in which they would fill when water overflows the banks of the associated reaches,
   // i.e. from lowest elevation to highest elevation.
   CArray <int, int> m_wetlIDUndxArray;

   // m_wetlHRUndxArray is a list of the HRU polygon indices of the HRUs in the wetland.
   // Note that HRU index values are the same for the HRU shapefile and m_hruArray.
   // This list has a 1-to-1 correspondence to m_wetlIDUndxArray, so a given HRU index
   // may appear more than once in the list.
   CArray <int, int> m_wetlHRUndxArray;

   // m_wetlReachNdxArray is a list of indices into m_reachArray of the reaches associated with the IDUs in the wetland.
   // Note that indices into m_reachArray are NOT the same as the indices into the Reach shapefile.
   // This list has a 1-to-1 correspondence to m_wetlIDUndxArray, so a given reach index
   // may appear more than once in the list.
   CArray <int, int> m_wetlReachNdxArray;
}; // end of class Wetland


class FLOWAPI FlowModel
{
friend class FlowProcess;
friend class HRU;
friend class Catchment;
friend class Reservoir;
friend class ModelOutput;
friend class ReachRouting;
friend class SWMM;
friend class Spring;

public:
   FlowModel();
   ~FlowModel();

protected:
   int m_id;

public:
   FlowContext m_flowContext;

public:
   bool Init( EnvContext* );
   bool InitRun( EnvContext* );
   bool Run( EnvContext*  );
   bool EndRun( EnvContext* );
      
   bool StartYear( FlowContext* ); // start of year initializations go here
   bool StartStep( FlowContext* );
   bool EndStep  ( FlowContext* );
   bool EndYear  ( FlowContext* );
   // manage global methods
   //void RunGlobalMethods( void );          

   bool ApplyQ2WETL(); // Move water spilling over the stream banks into the wetlands.
   inline double Att(int IDUindex, int col);
   inline int AttInt(int IDUindex, int col);
   inline float AttFloat(int IDUindex, int col);
   void SetAtt(int IDUindex, int col, double attValue);
   void SetAttInt(int IDUindex, int col, int attValue);
   void SetAttFloat(int IDUindex, int col, float attValue);
   static double VegDensity(double lai);
   Reach* GetReachFromCOMID(int comid);
   Reach *GetReachFromStreamIndex( int index ) { return (Reach*) m_reachTree.GetReachNodeFromPolyIndex( index ); }
   Reach *GetReach( int i )  // from internal array
      {
      if (i >= 0 && i < m_reachArray.GetSize()) return(m_reachArray[i]);
      CString msg; msg.Format("GetReach() i = %d, m_reachArray.GetSize() = %d", i, m_reachArray.GetSize());
      Report::ErrorMsg(msg);
      return(NULL);
      } // end of GetReach()

   int    GetReachCount( void ) { return (int) m_reachArray.GetSize(); }

   float  GetTotalReachLength( void );

   int AddHRU( HRU *pHRU, Catchment *pCatchment ) { m_hruArray.Add( pHRU ); pCatchment->m_catchmentHruArray.Add( pHRU ); pHRU->m_pCatchment = pCatchment; return (int) m_hruArray.GetSize(); }
   int GetHRUCount( void ) { return (int)m_hruArray.GetSize(); }

   int GetHRULayerCount( void ) { return m_soilLayerCount + m_vegLayerCount + m_snowLayerCount; }

   HRU *GetHRU(int i) 
      {
      if (i >= 0 && i < m_hruArray.GetSize()) return(m_hruArray[i]);
      CString msg; msg.Format("GetHRU() i = %d, m_hruArray.GetSize() = %d", i, m_hruArray.GetSize());
      Report::ErrorMsg(msg);
      return(NULL);
      } // end of GetHRU()
   
   int GetCatchmentCount( void ) { return (int)m_catchmentArray.GetSize(); }
   Catchment *GetCatchment( int i ) { return m_catchmentArray[ i ]; }

   Catchment *AddCatchment( int id, Reach *pReach ) 
   { 
      Catchment *pCatchment = new Catchment; 
      pCatchment->m_id = id; 
      pCatchment->m_pReach = pReach; 
      pReach->m_catchmentArray.Add( pCatchment ); 
      m_catchmentArray.Add( pCatchment ); 
      return pCatchment; 
   }

   Reach * FindReachFromPolyIndex( int polyIndex ) 
   {  
      for ( int i=0; i < m_reachArray.GetCount(); i++ ) if ( m_reachArray[i]->m_polyIndex == polyIndex ) return m_reachArray[ i ]; 
      return NULL; 
   } // end of FindReachFromPolyIndex()

   Reach* FindReachFromID(int id) { for (int i = 0; i < m_reachArray.GetCount(); i++) if (m_reachArray[i]->m_reachID == id) return m_reachArray[i]; return NULL; }

   StateVar *GetStateVar(int i) { return m_stateVarArray.GetAt(i); }
   StateVar *FindStateVar(LPCTSTR varName) { for (int i = 0; i < m_stateVarArray.GetCount(); i++) if (m_stateVarArray[i]->m_name.CompareNoCase(varName) == 0) return m_stateVarArray[i]; return NULL; }

   bool DailyUpdateToSingleYearWeatherAverages(int doy, int daysInYear, int year);
   bool UpdateAgBasinClimateTemporalAverages(EnvContext * pContext);
   bool UpdateIDUclimateTemporalAverages(int tau, EnvContext * pContext);
   double MagicReachWaterReport_m3(bool msgFlag = false); // Report on NaNs and added amounts in reaches.
   double MagicHRUwaterReport_m3(bool msgFlag = false); // Report on NaNs and added amounts in HRUs.
   double GetReachShade_a_lator_W_m2(Reach* pReach, double SW_unshaded_W_m2);
   bool DumpReachInsolationData(Shade_a_latorData* pSAL);
   bool ProcessShade_a_latorInputData(Shade_a_latorData* pSAL, FDataObj* pData, int dataRows, int reachCt);

   FDataObj m_SALtable; // Shade-a-lator table 366 rows (jday0) x 61 columns (jday0 + 4 directions x 15 veg ht / stream width ratios)
   // The value in the SALtable is the %shade on jday given a veg ht:stream width ratio and a stream axis direction from NE-SW, E-W, SE-NW, S-N
   // assuming the veg is on the southerly bank and has a vegetation density of 100%.


protected:
   bool InitClimateMeanValues(EnvContext *pContext);
   bool InitReaches( void  );
   int InitWetlands(void); // Returns the number of wetlands.
   bool InitCatchments( void );
   bool InitHRULayers(EnvContext*);
   bool CheckHRUwaterBalance(HRU* pHRU);
   bool FixHRUwaterBalance(HRU* pHRU);
   void SetAttributesFromHRUmemberData(HRU* pHRU);
   bool InitReservoirs(void);
   bool InitReservoirControlPoints( void );
   bool InitRunReservoirs( EnvContext* );
   void BackDownTheTree(int * pReachComid, int * pReachNdx, bool * pRightBranchFlag, int downstreamLimit);
   bool UpdateReservoirControlPoints( int doy );
   void UpdateReservoirWaterYear( int doy );
   int  SimplifyNetwork( void );
   int  SimplifyTree( ReachNode *pNode );
   int  ApplyCatchmentConstraints( ReachNode *pNode );
   int  ApplyAreaConstraints( ReachNode *pNode );
   int  SaveState(int calendar_year);
   bool ReadState();
   bool InitializeSpinup(void);
   bool IsICfileAvailable();

   int  OpenDetailedOutputFiles();
   int  SaveDetailedOutputIDU( CArray< FILE*, FILE* > & );
   int  SaveDetailedOutputReach( CArray< FILE*, FILE* > & );
   void  CloseDetailedOutputFiles(); 

   // helper functions
   int  OpenDetailedOutputFilesIDU  ( CArray< FILE*, FILE* > & );
   int  OpenDetailedOutputFilesGrid(CArray< FILE*, FILE* > &);
   int  OpenDetailedOutputFilesReach( CArray< FILE*, FILE* > & );
 
   int  m_detailedOutputFlags;  //1=dailyIDU, 2=annualIDU, 4=daily reach, 8=annual reach
   CArray< FILE*, FILE* > m_iduAnnualFilePtrArray;   // list of file pointers
   CArray< FILE*, FILE* > m_iduDailyFilePtrArray;   // list of file pointers
   CArray< FILE*, FILE* > m_reachAnnualFilePtrArray;   // list of file pointers
   CArray< FILE*, FILE* > m_reachDailyFilePtrArray;   // list of file pointers
   
   bool m_ICincludesWP;
      
   int  MoveCatchments( Reach *pToReach, ReachNode *pStartNode, bool recurse );  // recursive
   int  CombineCatchments( Catchment *pTargetCatchment, Catchment *pSourceCatchment, bool deleteSource ); // copy the HRUs from one catchment to another, optionally deleting the source Catchment
   
   void  PopulateTreeID( ReachNode *pNode, int treeID ); // recursive
   float PopulateCatchmentCumulativeAreas( void );
   float PopulateCatchmentCumulativeAreas( ReachNode *pNode );
   void PopulateRIVER_KM(ReachNode* pRoot);

   int  RemoveReaches( ReachNode *pStartNode );       // recursive
   bool RemoveReach( ReachNode *pNode );              // a single reach
   bool RemoveCatchment( ReachNode *pNode );          //single catchment - 
   int  GetUpstreamCatchmentCount( ReachNode *pStartNode );
   bool BuildCatchmentsFromQueries( void );
   bool BuildCatchmentsFromGrids( void );
   bool BuildCatchmentsFromIDUlayer(void);
   bool BuildCatchmentsFromHRUlayer(void);
   int  RemoveCatchmentHRUs( Catchment *pCatchment );

   //void PopulateHRUArray(void);

   int  BuildCatchmentFromPolygons( Catchment*, int iduArray[], int count );

   bool AggregateModel( void );
   bool CalculateUpstreamGeometry( void );

   Query *BuildFluxQuery( LPCTSTR queryStr, FLUXLOCATION type );
   int  AllocateFluxes( FluxInfo *pFluxInfo, Query *pQuery, FLUXLOCATION type, int layer );
   bool ParseFluxLocation( LPCTSTR typeStr, FLUXLOCATION &type, int &layer );
   bool SetAllCatchmentAttributes(void);
   bool SetHRUAttributes(Catchment *);
   int  GenerateAggregateQueries( MapLayer *pLayer /*in*/, CUIntArray &colsAgg /*in*/, CArray< Query*, Query* > &queryArray /*out*/ );
   int  ParseCols( MapLayer *pLayer /*in*/, CString &aggCols /*in*/, CUIntArray &colsAgg /*out*/ );
   int  ParseHRULayerDetails(CString &aggCols, int detail);
   bool ConnectCatchmentsToReaches( void );
   bool AssignReachesToHRUs();
   bool AssignIDUsToStreamBanks();
   //void UpdateCatchmentFluxes( float time, float timeStep, FlowContext *pFlowContext );

   bool InitFluxes( void );
   bool InitRunFluxes( void );
   bool InitIntegrationBlocks( void );
   // void AllocateOutputVars();
   int  AddStateVar( StateVar *pSV ) { return ( pSV->m_index = (int) m_stateVarArray.Add( pSV ) ); }
   int  GetStateVarCount( void ) { return (int) m_stateVarArray.GetSize(); }

   Reservoir *FindReservoirFromID( int id ) {  for ( int i=0; i < m_reservoirArray.GetCount(); i++ ) if ( m_reservoirArray[i]->m_id == id ) return m_reservoirArray[ i ]; return NULL; }

   void UpdateSummaries() { }

   void SummarizeIDULULC(void);
   void AddReservoirLayer( void );
   bool WriteDataToMap( EnvContext* );
   void UpdateAprilDeltas( EnvContext*  );
   void GetMaxSnowPack(EnvContext*);
   void UpdateYearlyDeltas( EnvContext*  );
   bool RedrawMap( EnvContext* );
   void ResetHRUfluxes();
   void ResetReachFluxes();
   bool ResetCumulativeYearlyValues();
   bool ResetCumulativeWaterYearValues();

   bool  SetGlobalExtraSVRxn( void );
   
   bool  SetGlobalReservoirFluxes( void );  // high-level method
   bool  SetGlobalReservoirFluxesResSimLite( void );
   bool  SetGlobalClimate( int dayOfYear );
   bool  SetGlobalFlows( void );

   bool  SolveGWDirect( void );     // high level method
   bool  SetGlobalGroundWater( void );

protected:
   // various collections
   IDataObj              *m_pHruGrid;               // a gridded form of the HRU array (useful for gridded models only)
   PtrArray< Reservoir > m_reservoirArray;         // list of reservoirs included in this model
   PtrArray< ControlPoint > m_controlPointArray;   // list of control points included in this model
     
public:
   CArray< Reach*, Reach* > m_reachArray;       // N.B. the index to m_ReachArray is not necessarily the same as the index to the corresponding reach in the Reach shapefile
												// pReach->m_polyIndex is the index to the Reach shapefile
   CArray< Wetland *, Wetland *> m_wetlArray; // The index to m_wetlArray is not the same as WETL_ID.
   PtrArray< Catchment > m_catchmentArray;         // list of catchments included in this model
   PtrArray< HRU >       m_hruArray;               // list of HRUs included in this model, ordered as in the HRU shapefile
   CString m_path;

   // flux information
protected:
   PtrArray< FluxInfo > m_fluxInfoArray;        // list of fluxes included in this model      

   // state var information
   PtrArray< StateVar > m_stateVarArray;
   PtrArray< Vertex > m_vertexArray;
   
protected:
   // These arrays are used for idu-based climate data files.  
   // They are indexed by IDU_index, and contain the location in the polygon-based climate files of the value for the IDU.
   CUIntArray m_precipNdx;
   CUIntArray m_tmeanNdx;
   CUIntArray m_tminNdx;
   CUIntArray m_tmaxNdx;
   CUIntArray m_solarradNdx;
   CUIntArray m_relhumidityNdx;
   CUIntArray m_sphumidityNdx;
   CUIntArray m_rhNdx;
   CUIntArray m_windspeedNdx;

   CString m_filename;
   CString m_name;
   CString m_streamLayer;
   CString m_catchmentLayer;
   CString m_climateFilePath;
   CString m_grid;
   MapLayer *m_pGrid;
   float m_climateStationElev;

   CString m_streamQuery;
   CString m_catchmentQuery;
   CString m_projectionWKT;

   CString m_elevCol;
   
   CString m_catchIDCol;
   CString m_streamJoinCol;
   CString m_hruIDCol;
   CString m_catchmentJoinCol;
   CString m_treeIDCol;

   CString m_initConditionsFileName;

   int     m_minStreamOrder;
   float   m_subnodeLength;
   int     m_buildCatchmentsMethod;
   int     m_soilLayerCount;
   int     m_snowLayerCount;
   int     m_vegLayerCount;
 
   float   m_initTemperature;
   float   m_wdRatio;     // global width/depth ratio ???? where does this come from???
   float   m_waterYearType;   //Variable used in reservoir operations model

   float   m_minCatchmentArea;
   float   m_maxCatchmentArea;
   int     m_numReachesNoCatchments;
   int     m_numHRUs;

public:
   int m_hruSvCount;
   int m_reachSvCount;
   int m_reservoirSvCount;
   float m_minElevation;

// catchment layer required columns
   int m_colCatchmentArea;
   int m_colArea;
   int m_colElev;
   int m_colLai;
   int m_colLulcB;
   int m_colLulcA;
   int m_colPVT;
   int m_colVEGCLASS;
   int m_colAGECLASS;
   int m_colLAI;
   int m_colTREE_HT;

   int m_colCatchmentReachIndex;
   int m_colCatchmentCatchID;
   int m_colCatchmentHruID;
   int m_colCatchmentReachId;
   int m_colCatchmentJoin;  
   
   int m_colPRECIP_YR;
   int m_colPRCP_Y_AVG;
   int m_colET_DAY;
   int m_colET_YR;
   int m_colAET_Y_AVG;
   int m_colMAX_ET_YR;
   int m_colSNOWEVAP_Y;
   int m_colSM2ATM;
   int m_colSM2ATM_YR;
   int m_colSNOW_SWE;
   int m_colH2O_MELT;
   int m_colFIELD_CAP;
   int m_colSNOWCANOPY;
   int m_colP_MINUS_ET;

   int m_colIDU_ID;
   int m_colHBVCALIB;
   int m_colECOREGION;
   int m_colAREA;
   int m_colELEV_MEAN;
   int m_colHruAREA;

   int m_colGRID_INDEX;
   int m_colTEMP;
   int m_colTMAX;
   int m_colTMIN;
   int m_colPRECIP;
   int m_colRAD_SW;
   int m_colRAD_SW_YR;
   int m_colSPHUMIDITY;
   int m_colRH;
   int m_colWINDSPEED;
   int m_colF_THETA;
   int m_colVPD_SCALAR;

   int m_colTMIN_GROW;
   int m_colPRCP_GROW;
   int m_colPRCPSPRING;
   int m_colPRCPWINTER;
   int m_colPRCP_JUN;
   int m_colPRCP_JUL;
   int m_colPRCP_AUG;

   int m_colTMINGROAVG;
   int m_colPRCPGROAVG;
   int m_colPRCPJUNAVG;
   int m_colPRCPJULAVG;
   int m_colPRCPAUGAVG;

   int m_colCENTROIDX;
   int m_colCENTROIDY;

   int m_colWETNESS;
   int m_colWETL_CAP;
   int m_colWETL2Q;
   int m_colWETL_ID;
   int m_colCOMID;

   int m_colHruHRU_ID;
   int m_colHruCOMID;
   int m_colHruHBVCALIB;
   int m_colHruFIELD_CAP;
   int m_colHruTEMP;
   int m_colHruTMAX;
   int m_colHruTMIN;
   int m_colHruPRECIP;
   int m_colHruRAD_SW;
   int m_colHruSPHUMIDITY;
   int m_colHruRH;
   int m_colHruWINDSPEED;

   int m_colHruSNOW_BOX;
   int m_colHruBOXSURF_M3;
   int m_colHruH2OMELT_M3;
   int m_colHruH2OSTNDGM3;
   int m_colHruNAT_SOIL;
   int m_colHruIRRIG_SOIL;
   int m_colHruGW_FASTBOX;
   int m_colHruGW_SLOWBOX;

   int m_colHruSM2ATM;
   int m_colHruET_DAY;
   int m_colHruAREA_M2;
   int m_colHruAREA_IRRIG;

    int m_colhruCATCH_NDX;
   int m_colCATCHID;
   int m_colhruCATCH_ID;
   int m_colHRU_ID;
   int m_colhruHRU_ID;
   int m_colHRU_NDX;
   int m_colhruCOMID;

// stream layer required columns
   int m_colStreamOBJECTID;
   int m_colStreamFrom;    // Used by BuildTree() to establish stream reach topology
   int m_colStreamTo;      // Used by BuildTree() to establish stream reach topology
   int m_colStreamDOBJECTID;
   int m_colStreamLOBJECTID;
   int m_colStreamROBJECTID;
   int m_colReachSTRM_ORDER;
   int m_colReachRIVER_KM;
   int m_colReachWIDTH_MIN;
   int m_colReachDEPTH_MIN;
   int m_colReachZ_MAX;
   int m_colReachZ_MIN;
   int m_colStreamJoin;   
   int m_colStreamCOMID;
   int m_colStreamCOMID_DOWN;
   int m_colStreamCOMID_LEFT;
   int m_colStreamCOMID_RT;
   int m_colReachREACH_NDX;
   int m_colReachREACH_H2O;
   int m_colReachHRU_ID;
   int m_colReachHRU_FRAC;
   int m_colStreamRES_ID;
   int m_colReachRESAREA_HA;
   int m_colReachRES_EVAPMM;
   int m_colReachRES_H2O;
   int m_colReachRES_LW_OUT;
   int m_colReachRES_SW_IN;
   int m_colReachRES_TEMP;
   int m_colStreamRESOUTFALL;
   int m_colStreamSTRMVERT0X;
   int m_colStreamSTRMVERT0Y;
   int m_colStreamQ_DIV_WRQ;
   int m_colStreamINSTRM_REQ; // regulatory flow requirement for this reach, cms (requirement of most junior instream WR, if more than one applies)
   int m_colStreamREACH_H2O; // volume of water in reach, m3
   int m_colReachADDED_VOL; // m3 H2O
   int m_colReachADDEDVOL_C; // deg C
   int m_colReachADDED_Q; // cms
   int m_colReachEVAP_MM; // daily evaporation from reach, m3H2O
   int m_colStreamHYDRO_MW;
   int m_colStreamTEMP_AIR;
   int m_colStreamTMAX_AIR;
   int m_colStreamTMIN_AIR;
   int m_colStreamPRECIP;
   int m_colStreamSPHUMIDITY;
   int m_colStreamRH;
   int m_colStreamRAD_SW;
   int m_colStreamWINDSPEED;
   int m_colStreamTMAX_H2O_Y;
   int m_colStreamIDU_ID;

   int m_colHruSWC;
   int m_colHruPrecip10Yr;
   int m_colHruTempYr;
   int m_colHruTemp10Yr;
   int m_colMAXSNOW;
   int m_colHruApr1SWE;
   int m_colHruApr1SWE10Yr;
   int m_colCLIMATENDX;

   int m_colHruDecadalSnow;
   int m_colReachQ_DISCHARG;
   int m_colReachLOG_Q;
   int m_colReachQ_CAP;
   int m_colReachQSPILL_FRC;
   int m_colReachQ2WETL;
   int m_colReachTEMP_H2O;
   int m_colReachArea;
   int m_colReachZ_MEAN;
   int m_colReachRAD_LW_OUT; // W/m2
   int m_colReachRAD_SW_IN; // W/m2
   int m_colReachAREA_H2O;
   int m_colReachWIDTH_CALC;
   int m_colReachDEPTH;
   int m_colReachDEPTHMANNG;
   int m_colReachTURNOVER;
   int m_colReachXFLUX_D;
   int m_colReachXFLUX_Y;
   int m_colReachSPRING_CMS; // flow rate of spring discharge, cms
   int m_colReachSPRINGTEMP; // temperature of spring discharge, deg C
   int m_colReachIN_RUNOFF;
   int m_colReachIN_RUNOF_C; // temperature of water running off the land into the reach, deg C
   int m_colReachQ_UPSTREAM;
   int m_colReachQ_MIN;
   int m_colReachHBVCALIB;
   int m_colReachDIRECTION;
   int m_colReachSLength;
   int m_colReachSHADECOEFF;
   int m_colReachBANK_L_IDU; // IDU index of IDU representative of the left bank of the reach
   int m_colReachBANK_R_IDU; // IDU index of IDU representative of the right bank of the reach
   int m_colReachRAD_SW_EST;
   int m_colReachSHADE_TOPO;
   int m_colReachVGDNS_CALC;
   int m_colReachVGDNSGIVEN;
   int m_colReachVGDNSREACH;
   int m_colReachSHADE_VEG;
   int m_colReachLAI_REACH;
   int m_colReachVEGHTREACH;
   int m_colReachVEGHTGIVEN;
   int m_colReachWIDTHREACH;
   int m_colReachWIDTHGIVEN;
   int m_colReachRADSWGIVEN;
   int m_colReachSAL_REACH;
   int m_colReachKCAL_GIVEN;
   int m_colReachKCAL_REACH;
   int m_colReachTOPOELEV_E;
   int m_colReachTOPOELEV_S;
   int m_colReachTOPOELEV_W;
   int m_colReachVEG_HT_R;
   int m_colReachVEG_HT_L;

   int m_colHbvW2A_SLP;
   int m_colHbvW2A_INT;

   int m_colARIDITYNDX;
   int m_colHRUPercentIrrigated;
   int m_colHRUMeanLAI;
   int m_colMaxET_yr;
   int m_colIrrigation_yr;
   int m_colIrrigation;
   int m_colRunoff_yr;
   int m_colAgeClass;

   int m_colIRRIGATION;
   int m_colSM_DAY;

   // optional cols
   int m_colStreamCumArea;
   int m_colCatchmentCumArea;
   int m_colTreeID;

// reservoir column (in stream coverage)
   int m_colResID;

// layer structures
   Map *m_pMap;
   MapLayer *m_pCatchmentLayer;
   MapLayer *m_pStreamLayer;
   IDUlayer * m_pIDUlayer;
   MapLayer * m_pReachLayer;
   MapLayer * m_pHRUlayer;
   MapLayer * m_pLinkLayer;
   MapLayer * m_pNodeLayer;
   MapLayer * m_pSubcatchLayer;
   MapLayer *m_pResLayer;  // visual display only
   ReachTree m_reachTree;

// Catchment information
   CString m_catchmentAggCols;   // columns for aggregating catchment
   CUIntArray m_colsCatchmentAgg;

// HRU information
   CString m_hruAggCols;         // columns for aggregating HRU's
   CUIntArray m_colsHruAgg;
 
   bool m_useStationData;

   INT_METHOD m_integrator;
   float m_minTimeStep;
   float m_initTimeStep;
   float m_maxTimeStep;
   float m_errorTolerance;

   const MapLayer* GetGrid() { return m_pGrid; }

public:
   // FluxInfo access
   int AddFluxInfo( FluxInfo *p ) { return (int) m_fluxInfoArray.Add( p ); }
   int GetFluxInfoCount( void ) { return (int) m_fluxInfoArray.GetCount(); }
   FluxInfo *GetFluxInfo( int i ) { return m_fluxInfoArray[ i ]; }

   // Storage for model results
   //FDataObj *m_pReachDischargeData;
   //FDataObj *m_pHRUPrecipData;     
   //FDataObj *m_pHRUETData;  // contains HRULayer precip info for each sample location
   FDataObj *m_pTotalFluxData;  // for mass balance, acre-ft/yr
   FDataObj m_FlowWaterBalanceReport;
   VDataObj m_ResActiveRuleReport;
//   PtrArray <char> m_DayStrings;

   double m_StartYear_totH2O_m3;
   double m_StartYear_totReaches_m3;
   double m_StartYear_totReservoirs_m3;
   double m_StartYear_totHRUs_m3;
   float m_totArea;

   double CalcTotDailyEvapFromReaches(); // Returns m3 H2O
   double m_totEvapFromReachesYr_m3;
   double m_totEvapFromReservoirsYr_m3;

   double CalcTotH2OinReaches(); // Returns m3 H2O
   double CalcTotH2OinReservoirs(); // Returns m3 H2O
   double CalcTotH2OinHRUs(); // Returns m3 H2O
   double CalcTotH2O(); // Returns m3 H2O
   
   //FDataObj *m_pReservoirData;
   PtrArray < FDataObj > m_reservoirDataArray;
   PtrArray < FDataObj > m_reachHruLayerDataArray;   // each element is an FDataObj that stores modeled hrulayer water content (m_wc) for a reach sample point
   PtrArray < FDataObj > m_hruLayerExtraSVDataArray;    
   PtrArray < FDataObj > m_reachMeasuredDataArray;
   CStringArray m_hruLayerNames;
   CStringArray m_hruLayerDepths;
   CStringArray m_initWaterContent;

   // exposed outputs
   float m_volumeMaxSWE;
   int m_dateMaxSWE;

   float m_annualTotalET;           // acre-ft
   float m_annualTotalPrecip;       // acre-ft
   double m_annualTotalDischarge;    //acre-ft
   float m_annualTotalRainfall;     // acre-ft
   float m_annualTotalSnowfall;     // acre-ft

   int IdentifyMeasuredReaches( void );
   //bool InitializeReachSampleArray(void);
   //bool InitializeHRULayerSampleArray(void);
   //bool InitializeReservoirSampleArray(void);
   void CollectData( int dayOfYear );
   bool RestoreStateVariables(bool spinupFlag);
   void ResetDataStorageArrays( EnvContext *pEnvContext );
   //void CollectHRULayerData( void );
   //void CollectHRULayerExtraSVData( void );
   //void CollectHRULayerDataSingleObject( void );
   //void CollectDischargeMeasModelData( void );
   bool CollectModelOutput( void );
   void UpdateHRULevelVariables(EnvContext *pEnvContext);

   void CollectReservoirData();
 
   int m_mapUpdate;  // Update the map with Results from the Hydrology model?  0: no update, 1: update each year, 2: update each flow timestep" ); 

   int m_gridClassification;

   // simulation 
   double  m_currentTime;   // days since 0000
   double m_yearStartTime;
protected:
   double  m_stopTime;      // note: All time units are days
   double  m_timeInRun;     // days in run (0-based, starting in startYear)
   double  m_timeStep;      // for internal synchronization/scheduling
   int     m_timeOffsett;
   float   m_collectionInterval;
   bool    m_useVariableStep;
   double  m_rkvTolerance;
   double  m_rkvMaxTimeStep; 
   double  m_rkvMinTimeStep;
   double  m_rkvInitTimeStep;
   double  m_rkvTimeStep;

   // ptrs to required global methods - (note: memore is managed elsewhere
   ReachRouting    *m_pReachRouting;   // can this be moved to externasl (really, global) methods array?
   LateralExchange *m_pLateralExchange;
   HruVertExchange *m_pHruVertExchange;


   // hruLayer extra sv Reaction methods
   EXSVMETHOD  m_extraSvRxnMethod;
   CString m_extraSvRxnExtSource;
   CString m_extraSvRxnExtPath;
   CString m_hextraSvRxnExtFnName;
   HINSTANCE m_extraSvRxnExtFnDLL;
   DIRECTFN m_extraSvRxnExtFn;

   // reservoir flux methods
   RESMETHOD  m_reservoirFluxMethod;
   CString m_reservoirFluxExtSource;
   CString m_reservoirFluxExtPath;
   CString m_reservoirFluxExtFnName;
   HINSTANCE m_reservoirFluxExtFnDLL;
   DIRECTFN m_reservoirFluxExtFn;
   FDataObj *m_pResInflows;  //Use to store inflow from ResSIM for model testing.  mmc 01/04/2013
   FDataObj *m_pCpInflows;  //Use to store inflow at control points from ResSIM for model testing.  mmc 06/04/2013

   // groundwater methods
   //GWMETHOD m_gwMethod;
   //CString m_gwExtSource;
   //CString m_gwExtPath;
   //CString m_gwExtFnName;
   //HINSTANCE m_gwExtFnDLL;
   //DIRECTFN m_gwExtFn;

   // simulation control
   IntegrationBlock m_hruBlock;
   IntegrationBlock m_reachBlock;
   IntegrationBlock m_reservoirBlock;
   IntegrationBlock m_totalBlock;         // for overall mass balance

   // cumulative variables for mass balance error check
   float m_totalWaterInputRate;
   float m_totalWaterOutputRate;
   SVTYPE m_totalWaterInput;
   SVTYPE m_totalWaterOutput;

   FDataObj *m_pGlobalFlowData;
   
   static void GetCatchmentDerivatives( double time, double timestep, int svCount, double *derivatives, void *extra );
   static void GetReservoirDerivatives( double time, double timestep, int svCount, double *derivatives, void *extra ) ;
   //static void GetReachDerivatives( float time, float timestep, int svCount, double *derivatives, void *extra );
   static void GetTotalDerivatives( double time, double timestep, int svCount, double *derivatives, void *extra ) ;

   bool CheckSurfaceH2O(HRU* pHRU, double boxSurfaceH2Oadjustment_m3);

   
protected:
// parameter tables
   PtrArray <ParamTable> m_tableArray;
   PtrArray <ReachSampleLocation> m_reachSampleLocationArray;

   PtrArray <ParameterValue> m_parameterArray;
   int m_numQMeasurements;

   PtrArray< ModelOutputGroup > m_modelOutputGroupArray;
   PtrArray< ModelVar > m_modelVarArray;     // for any flow-defined variables registered with the MapExprEngine
 
public:
   int GetModelOutputGroupCount( void ) { return (int) m_modelOutputGroupArray.GetSize(); }
   ModelOutputGroup *GetModelOutputGroup( int i ) { return m_modelOutputGroupArray[ i ]; }

public:
   ParamTable *GetTable( LPCTSTR name );     // get a table
   bool        GetTableValue( LPCTSTR name, int col, int row, float &value );
   // note: for more efficient ParamTable data access, get (and store) a ParamTable ptr
   // and use ParamTable::Lookup() methods
   int AddModelVar(LPCTSTR label, LPCTSTR varName, MTDOUBLE *value);
   int AddModelVar(MapExprEngine * pME, LPCTSTR label, LPCTSTR varName, MTDOUBLE *value);

   // various GetData()'s
   bool GetHRUData( HRU *pHRU, int col, VData &value, DAMETHOD method ); 
   bool GetHRUData( HRU *pHRU, int col, bool  &value, DAMETHOD method ); 
   bool GetHRUData( HRU *pHRU, int col, int   &value, DAMETHOD method ); 
   bool GetHRUData( HRU *pHRU, int col, float &value, DAMETHOD method ); 

   bool GetCatchmentData( Catchment *pCatchment, int col, VData &value, DAMETHOD method ); 
   bool GetCatchmentData( Catchment *pCatchment, int col, bool  &value, DAMETHOD method ); 
   bool GetCatchmentData( Catchment *pCatchment, int col, int   &value, DAMETHOD method ); 
   bool GetCatchmentData( Catchment *pCatchment, int col, float &value, DAMETHOD method ); 

   bool SetCatchmentData( Catchment *pCatchment, int col, VData value ); 
   bool SetCatchmentData( Catchment *pCatchment, int col, bool  value ); 
   bool SetCatchmentData( Catchment *pCatchment, int col, int   value ); 
   bool SetCatchmentData( Catchment *pCatchment, int col, float value ); 

   bool GetReachData( Reach *pReach, int col, VData &value );
   bool GetReachData( Reach *pReach, int col, bool  &value ); 
   bool GetReachData( Reach *pReach, int col, int   &value ); 
   bool GetReachData( Reach *pReach, int col, float &value );

   const char* GetPath() {return m_path.GetString();}

   // misc utility functions
   bool MoveFlux( Flux *pFlux, FluxContainer *pStartContainer, FluxContainer *pEndContainer );
   int  GetCurrentYear( void ) { return this->m_flowContext.pEnvContext->currentYear; }
   int  GetCurrentYearOfRun( void ) { return this->m_flowContext.pEnvContext->yearOfRun; }
  CString GetScenarioName(void) { return ((this->m_scenarioArray[m_currentFlowScenarioIndex] != NULL) ? this->m_scenarioArray[m_currentFlowScenarioIndex]->m_name : ""); }


protected:
	int m_currentEnvisionScenarioIndex;

	// scenario macros 
	PtrArray< ScenarioMacro > m_macroArray;
	//  void ApplyMacros( CString &str );

public:
	PtrArray< ClimateScenario > m_scenarioArray;
	int m_currentFlowScenarioIndex;
   int m_waterRightsReportInputVar;
	void ApplyMacros(CString &str);

   float m_parameter1, m_parameter2; // For passing parameter values from external function xml blocks in Flow.xml
   float m_shortwave_interception_fraction; // fraction of shortwave radiation which is intercepted by the PV panels, passed from SolarPanels to EvapTrans and FoodAndAg
   float m_pv_lw_coeff; // multiplier on netLongwaveRadiation in reference ET calculation, representing the effect of PV panels, passed from SolarPanels to EvapTrans

// climate data (assumes daily values, one years worth of data)
protected:
   //PtrArray< ClimateDataInfo > m_climateDataInfoArray;
   //CMap< int, int, ClimateDataInfo*, ClimateDataInfo* > m_climateDataMap;

   //void InitRunClimate( void );
   bool OpenClimateDataFiles(int tgtYear);
   void CloseClimateData( void );

   // doy of weather data last read from netCDF files; -1 if none
   int m_doyTMAX;
   int m_doyTMIN; 
   int m_doyTEMP;
   int m_doyPRECIP;
   int m_doyRAD_SW;
   int m_doySPHUMIDITY; 
   int m_doyRH;
   int m_doyWINDSPEED; 

public:
   bool InitGridIndex();
   int GetIDUndxForReach(Reach * pReach);
   bool GetHRUClimate(CDTYPE type, HRU *pHRU, int dayOfYear, float &value);
   bool GetTodaysHRUclimate(CDTYPE type, HRU * pHRU, float &value);
   bool GetDailyWeatherField(CDTYPE fieldID, int tgtDoy0, int tgtYear);
   bool GetCurrentYearDailyWeatherField(CDTYPE fieldID, int tgtDoy0);
   bool GetTodaysWeatherField(CDTYPE type);
   bool GetWeatherField(CDTYPE type, int tgtDoy0, int tgtYear, float field_vals[]);
   bool GetHRUweatherFieldFromIDUs(CDTYPE type, int colHRU, int colIDU);
   bool GetReachWeatherFieldFromIDUs(CDTYPE type, int colReach, int colIDU);

   float GetTodaysReachTEMP_AIR(Reach * pReach);
   float GetTodaysReachTMAX_AIR(Reach * pReach);
   float GetTodaysReachTMIN_AIR(Reach * pReach);
   float GetTodaysReachPRECIP(Reach * pReach);
   float GetTodaysReachSPHUMIDITY(Reach * pReach);
   float GetTodaysReachRAD_SW(Reach * pReach);
   float GetTodaysReachWINDSPEED(Reach * pReach);

   ClimateDataInfo   *GetClimateDataInfo( CDTYPE type ) 
   { 
      ClimateScenario *pScenario = m_scenarioArray.GetAt( m_currentFlowScenarioIndex ); 
      ClimateDataInfo *pInfo = NULL;  
      if ( pScenario && pScenario->m_climateDataMap.Lookup( type, pInfo ) ) return pInfo; 
      return NULL; 
   }
   GeoSpatialDataObj *GetClimateDataObj ( CDTYPE type ) { ClimateScenario *pScenario = m_scenarioArray.GetAt( m_currentFlowScenarioIndex ); ClimateDataInfo *pInfo = NULL;  if ( pScenario && pScenario->m_climateDataMap.Lookup( type, pInfo ) ) return pInfo->m_pDataObj; return NULL; }

   QueryEngine * m_pQE_IDU;
   QueryEngine * m_pQE_Reach;
   QueryEngine * m_pQE_HRU;
   QueryEngine * m_pQE_XHRU;
   QueryEngine * m_pQE_Link;
   QueryEngine * m_pQE_Node;
   QueryEngine * m_pQE_Subcatch;

   MapExprEngine * m_pME_IDU;
   MapExprEngine * m_pME_Reach;
   MapExprEngine * m_pME_HRU;
   MapExprEngine * m_pME_XHRU;
   MapExprEngine * m_pME_Link;
   MapExprEngine * m_pME_Node;
   MapExprEngine * m_pME_Subcatch;

   QueryEngine   *m_pStreamQE;
   QueryEngine   *m_pCatchmentQE;
   Query *m_pStreamQuery;
   Query *m_pCatchmentQuery;
   
   // plugin interfaces this only apply to DLLs with global methods
   bool InitPlugins   ( void );
   bool InitRunPlugins( void );
   bool EndRunPlugins ( void );
   
  //Parameter Estimation
public:
   void GetNashSutcliffe(float *ns);
   float GetObjectiveFunction(FDataObj *pData, float &ns, float &nsLN, float &ve);

   int LoadTable( LPCTSTR filename, DataObj** pDataObj, int type );  // type=0->FDataObj, type=1->VDataObj;

   PtrArray< FDataObj > m_mcOutputTables;
   FDataObj *m_pErrorStatData;
   FDataObj *m_pParameterData;
   FDataObj *m_pDischargeData;
   FDataObj *m_pOtherTimeSeriesData;
   FDataObj *m_pOtherObjectiveFunctions;
   RandUniform m_randUnif1;
   RandNormal m_randNorm1;
   long m_rnSeed;
   bool InitializeParameterEstimationSampleArray(void);
   bool m_estimateParameters;
   int m_numberOfRuns;
   int m_numberOfYears;
   int m_saveResultsEvery;
   float m_nsThreshold;
   //CString m_paramEstOutputPath;   // defaults to 'm_path'\outputs\

   void UpdateMonteCarloOutput(EnvContext *pEnvContext, int runNumber);
   void UpdateMonteCarloInput(EnvContext *pEnvContext, int runNumber);
   
   FDataObj *m_pClimateStationData;
   int m_timeOffset;
   void GetTotalStorage(float &channel, float &terrestrial);

   // various timers
   float m_loadClimateRunTime;   
   float m_globalFlowsRunTime;   
   float m_externalMethodsRunTime;   
   float m_gwRunTime;   
   float m_hruIntegrationRunTime;   
   float m_reservoirIntegrationRunTime;   
   float m_reachIntegrationRunTime;   
   float m_massBalanceIntegrationRunTime; 
   float m_totalFluxFnRunTime;                 // time in Flux::Evaluate()
   float m_reachFluxFnRunTime;
   float m_hruFluxFnRunTime;                   // time in GetCatchmentDerivatives()
   float m_collectDataRunTime;
   float m_outputDataRunTime;

   int m_checkMassBalance;    // do mass balance check?

   void ReportTimings( LPCTSTR format, float timing ) { CString msg; msg.Format( format, timing ); Report::LogMsg( msg ); }   
   void ReportTimings( LPCTSTR format, float timing, float timing2 ) { CString msg; msg.Format( format, timing, timing2 ); Report::LogMsg( msg ); }   
   
// video recording
//protected:
   bool  m_useRecorder;
   CArray< int > m_vrIDArray;
   //int   m_frameRate;
};



class FlowProcess : public EnvAutoProcess
{
public:
   FlowProcess();
   ~FlowProcess();

   BOOL Init   ( EnvContext *pEnvContext, LPCTSTR initStr );
   BOOL InitRun( EnvContext *pEnvContext );
   BOOL Run    ( EnvContext *pEnvContext );
   BOOL EndRun ( EnvContext *pEnvContext );
   BOOL Setup  ( EnvContext*, HWND );

protected:
   PtrArray< FlowModel > m_flowModelArray;

public:
   int AddModel( FlowModel *pModel ) { return (int) m_flowModelArray.Add( pModel ); }

protected:
   FlowModel *GetFlowModelFromID( int id );
   bool LoadXml( LPCTSTR filename, EnvContext *pEnvContext);

public:
   int m_maxProcessors;
   int m_processorsUsed;
   MapLayer * m_pIDUlayer;
   MapLayer * m_pReachLayer;
   MapLayer * m_pHRUlayer;
   MapLayer * m_pLinkLayer;
   MapLayer * m_pNodeLayer;
   MapLayer * m_pSubcatchLayer;
};


inline
bool HRULayer::AddFluxFromGlobalHandler(float value, WFAINDEX wfaIndex)
   {
   switch (wfaIndex)
      {
      case FL_TOP_SINK:
      case FL_BOTTOM_SINK:
      case FL_STREAM_SINK:
      case FL_SINK:  //negative values are gains
         value = -value;
      break;
   default:
      break;
      }
   if (isnan(value))
      {
      m_nanOccurred = true;
      return(false);
      }
   float orig_stored_value1 = m_waterFluxArray[wfaIndex];
   float orig_stored_value2 = m_globalHandlerFluxValue;
   m_waterFluxArray[wfaIndex] += value;
   m_globalHandlerFluxValue += value;
   if ((m_waterFluxArray[wfaIndex] == m_waterFluxArray[wfaIndex]) && (m_globalHandlerFluxValue == m_globalHandlerFluxValue)) return(true);
   m_waterFluxArray[wfaIndex] = orig_stored_value1;
   m_globalHandlerFluxValue = orig_stored_value2;
   m_nanOccurred = true;
   return(false);
   } // end of HRULayer::AddFluxFromGlobalHandler()

inline
float Reach::GetCatchmentArea( void )
   {
   float area = 0;
   for ( int i=0; i < (int) m_catchmentArray.GetSize(); i++ )
      area += m_catchmentArray[ i ]->m_area;
   return area;
   }


inline
ResConstraint *Reservoir::FindResConstraint( LPCTSTR name )
   {
   for ( INT_PTR i=0; i < m_resConstraintArray.GetSize(); i++ ) 
      {
      ResConstraint *pConstraint = m_resConstraintArray[ i ];
      
      if ( pConstraint->m_constraintFileName.CompareNoCase( name ) == 0 ) 
         return m_resConstraintArray[ i ];  
      }

   return NULL;
   }

inline
bool FlowModel::SetCatchmentData( Catchment *pCatchment, int col, bool  value )
   {
   VData _value( value );
   return SetCatchmentData( pCatchment, col, _value );
   }

inline
bool FlowModel::SetCatchmentData( Catchment *pCatchment, int col, int  value )
   {
   VData _value( value );
   return SetCatchmentData( pCatchment, col, _value );
   }

inline
bool FlowModel::SetCatchmentData( Catchment *pCatchment, int col, float value )
   {
   VData _value( value );
   return SetCatchmentData( pCatchment, col, _value );
   }


inline 
Reach* GetReachFromNode(ReachNode* pNode)
{
   if ( pNode == NULL ) 
      return NULL;
   
   if ( pNode->m_reachIndex < 0 )
      return NULL;

   if ( pNode->IsPhantomNode() )
      return NULL;
   
   return static_cast<Reach*>( pNode );
   } 
