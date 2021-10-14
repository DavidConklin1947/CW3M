#pragma once

#include "stdafx.h"
#include <GeoSpatialDataObj.h>
#include <EnvEngine\EnvConstants.h>
#include <Flow\FlowInterface.h>
#include <Vdataobj.h>
#include <EnvExtension.h>

#include <PtrArray.h>
#include <FDATAOBJ.H>
#include <randgen\Randunif.hpp>
#include <QueryEngine.h>
#include <misc.h>
#include <tixml.h>


using namespace std;

#define ID_COLDSTART 0 /* id of cold start AP in .envx file */
#define ID_GETWEATHER 20 /* id of GetWeather AP in .envx file */
#define STATIONARY_CLIM_SCENARIO_INDEX 3

#define MAX_FISH_GROUPS 2

#define NUM_GRIDCELLS 3933 /* Grid_US4km -r 92,160 -c 27,83 69 rows x 57 cols */
#define FIRST_YEAR_OF_CLIMATE_DATA 1895
#define MI_PER_M 0.000621371f
#define GAL_PER_FT3 7.48052

#define LULCA_DEVELOPED 1 /* LULC_A {Developed} */
#define LULCA_AGRICULTURE 2 /* LULC_A {Agriculture} */
// #define LULCA_OTHER_VEG 3 /* LULC_A {Other vegetation} */
#define LULCA_FOREST 4 /* LULC_A {Forest} */
#define LULC_FINE_DEVELOPED_UNDIFFERENTIATED 125 /* LULC_FINE (aka VEGCLASS) {Developed/Undifferentiated} */
#define LULC_MED_ORCHARDS_VINEYARDS_AND_TREE_FARMS 21 /* LULC_MED for Orchands, vineyards, and tree farms */
#define VEGCLASS_CROP_NOT_YET_SELECTED 299
#define VEGCLASS_FALLOW 61 /* VEGCLASS Fallow/Idle Cropland {fallow} */
#define VEGCLASS_DF_GFp 4002100 /* VEGCLASS {DF:GFp} */
#define PVT_FWI 6 /* PVT {Western hemlock intermediate (fwi)} */
#define OWNER_PRIVATE_INDUSTRIAL 1 /* OWNER {Private industrial forest} */
#define OWNER_PNI 0 /* OWNER {Private non-industrial} */
#define ZONE_AG 2 /* code for ZONED_USE {Ag} */
// #define ZONE_EFU 21 /* code for ZONED_USE {Exclusive Farm Use} */
#define ZONE_RR 13 /* code for ZONED_USE {rural residential} */
#define MAX_RR_DH_ID 1000 /* values in the RR_DH_ID column of the population unit file should be less than this */
#define MAX_UGB 100 /* UGB codes (after translating from UGB_DH_ID codes) should be less than this */
#define MAX_UGA_NDX 72
#define POP_DATA_INTERVAL 5 /* population projections are specified at this interval of years */
#define YR_OF_FIRST_POP_PROJECTION 2010 /* first population projection is for this year */
#define PEOPLE_PER_HOUSEHOLD 2.4f

#define TOKEN_FOR_NO_CHANGE -1 /* used in RunPrescribedLULCs() */

// UGB codes for the 8 UGBs with populations > 20,000 in 2010
#define UGB_METRO 40
#define UGB_EUGENESPRINGFIELD 22
#define UGB_SALEM 51
#define UGB_CORVALLIS 12
#define UGB_ALBANY 2
#define UGB_MCMINNVILLE 39
#define UGB_NEWBERG 47
#define UGB_WOODBURN 71

#define BENTON_GROUP 1 /* Benton "group" of counties: Benton alone */
#define LANE_GROUP 2 /* Lane "group" of counties: Lane alone */
#define WASHINGTON_GROUP 3 /* Washington group of counties: 
      Clackamas, Columbia, Multnomah, Washington, Yamhill */
#define MARION_GROUP 4 /* Marion group of counties: Linn, Marion, Polk */

/* from IDU.xml 
<field col = "COUNTYID"
   label = "County"
   ...
   <description>
   Unique identifier for eac county
   < / description>
   ...
   <attributes>
   <attr value = "0" color = "(227,227,227)" label = "Unknown" / >
   <attr value = "1" color = "(207,111,79)" label = "Benton" / >
   <attr value = "2" color = "(159,223,159)" label = "Clackamas" / >
   <attr value = "3" color = "(15,47,143)" label = "Douglas" / >
   <attr value = "4" color = "(223,159,223)" label = "Lane" / >
   <attr value = "5" color = "(175,15,47)" label = "Lincoln" / >
   <attr value = "6" color = "(127,127,127)" label = "Linn" / >
   <attr value = "7" color = "(79,239,207)" label = "Marion" / >
   <attr value = "8" color = "(31,95,31)" label = "Multnomah" / >
   <attr value = "9" color = "(239,207,111)" label = "Polk" / >
   <attr value = "10" color = "(191,63,191)" label = "Tillamook" / >
   <attr value = "11" color = "(95,31,95)" label = "Washington" / >
   <attr value = "12" color = "(47,143,175)" label = "Yamhill" / >
   <attr value = "-3" color = "(111,79,239)" label = "Columbia" / >
   <attr value = "-4" color = "(63,191,63)" label = "Deschutes" / >
   <attr value = "-13" color = "(127,127,127)" label = "Wasco" / >
   < / attributes>
   < / field>
*/
enum COUNTYid
   {
   BentonCoID=1,
   ClackamasCoID=2,
   DouglasCoID=3,
   LaneCoID=4,
   LincolnCoID=5,
   LinnCoID=6,
   MarionCoID=7,
   MultnomahCoID=8,
   PolkCoID=9,
   TillamookCoID=10,
   WashingtonCoID=11,
   YamhillCoID=12,
   ColumbiaCoID=-3,
   DeschutesCoID=-4,
   WascoCoID=-13
   };
enum COUNTYcolumn
   {
   NoCol=0,
   BentonCol=1,
   ClackamasCol=2,
   ColumbiaCol=3,
   LaneCol=4,
   LinnCol=5,
   MarionCol=6,
   MultnomahCol=7,
   PolkCol=8,
   WashingtonCol=9,
   YamhillCol=10,
   MetroCol=11
   };
   
enum UGAcode
   {
   UGA_AdairVillage=1,
   UGA_Albany=2,
   UGA_Amity=3,
   UGA_Aumsville=4,
   UGA_Aurora=5,
   UGA_Banks=6,
   UGA_Barlow=7,
   UGA_Brownsville=8,
   UGA_Canby=9,
   UGA_Carlton=10,
   UGA_Coburg=11,
   UGA_Corvallis=12,
   UGA_CottageGrove=13,
   UGA_Creswell=14,
   UGA_Dallas=15,
   UGA_Dayton=16,
   UGA_Detroit=17,
   UGA_Donald=18,
   UGA_Dundee=19,
   UGA_Estacada=20,
   UGA_UnknownLaneCo=21,
   UGA_EugeneSpringfield=22,
   UGA_FallsCity=23,
   UGA_ForestGrove=24,
   UGA_Gaston=25,
   UGA_Gates=26,
   UGA_Gervais=27,
   UGA_Halsey=28,
   UGA_Harrisburg=29,
   UGA_Hubbard=30,
   UGA_Idanha=31,
   UGA_Independence=32,
   UGA_Jefferson=33,
   UGA_JunctionCity=34,
   UGA_Lafayette=35,
   UGA_Lebanon=36,
   UGA_Lowell=37,
   UGA_Lyons=38,
   UGA_McMinnville=39,
   UGA_Metro=40,
   UGA_MillCity=41,
   UGA_Millersburg=42,
   UGA_Molalla=43,
   UGA_Monmouth=44,
   UGA_Monroe=45,
   UGA_MtAngel=46,
   UGA_Newberg=47,
   UGA_NorthPlains=48,
   UGA_Oakridge=49,
//   UGA_Philomath=50,  Philomath is combined with Corvallis
   UGA_SalemKeizer=51,
   UGA_Sandy=52,
//   UGA_=53,
//   UGA_=54
   UGA_Scio=55,
   UGA_ScottsMills=56,
   UGA_Sheridan=57,
   UGA_Silverton=58,
   UGA_Sodaville=59,
//   UGA_=60,
   UGA_StPaul=61,
   UGA_Stayton=62,
//   UGA_Sublimity=63,  Sublimity is combined with Stayton
   UGA_SweetHome=64,
   UGA_Tangent=65,
   UGA_Turner=66,
   UGA_Veneta=67,
   UGA_Waterloo=68,
   UGA_Westfir=69,
   UGA_Willamina=70,
   UGA_Woodburn=71,
   UGA_Yamhill=72
   };
   

enum CROPS {CROP_GRASS_SEED=0, CROP_PASTURE, CROP_WHEAT, CROP_FALLOW, CROP_CORN, CROP_CLOVER, CROP_HAY, CROP_OTHER, NUM_CROPS};

enum TARGET_METHOD // duplicates enum in Target.h
   {
   TM_UNDEFINED   = -1,
   TM_RATE_LINEAR =  0,
   TM_RATE_EXP    =  1,
   TM_TIMESERIES_CONSTANT_RATE  =  2,   
   TM_TIMESERIES_LINEAR  =  3    // not currently implemented
   };

enum TD_DATA_TYPE {TD_TYPE_FILE, TD_TYPE_TIMESERIES};
enum TD_DATA_METHOD {TD_METHOD_LINEAR, TD_METHOD_CONSTANT_RATE};

enum WW2100stratum { unk=-99, fdw=1, fmh=2, fsi=3, fto=4, fvg=5, fwi=6, fuc=7, ftm=8, fdd=9};
struct PVTencoding { CString abbrev; CString region; CString name; WW2100stratum stratum; };
struct CTSSencoding { CString abbrev; CString name; int id; };

class VegState
{
public:
   VegState(CString PVTtext, CString CTSStext);
   ~VegState() {}
public:
   bool Forest() { return m_valid_forest_state; }
   int Stratum() { return m_stratum_id; }
   int CTSS() { return m_ctss_id; }
   CString CoverType() { return m_cover_type; }
   CString StructuralStage() { return m_structural_stage; }

   bool m_foundPVTflag, m_foundCTflag, m_foundSSflag;
private:
   bool m_valid_forest_state;

   CString m_pvt_text;

   CString m_ctss_text;
   int m_ctss_id;

   CString m_cover_type;
   int m_cover_type_id;

   CString m_structural_stage;
   int m_structural_stage_id;

   CString m_stratum_text;
   int m_stratum_id;

}; // end of class VegState


//------------------------------------------------------------------------------
// fish model supporting classes
//------------------------------------------------------------------------------
class SpeciesGroup;

class FishSpecie
{
public:
   CString m_name;
   CString m_code;
   int     m_group;     // 0=native, 1=exotic
   int     m_use;       // 0=ignore, 1=use in mainstem, 2=use in slough, 3=use in both

   float m_intercept;
   float m_tMax;

   SpeciesGroup *m_pGroup;

   FishSpecie( void ) : m_group( 0 ) { }
};


class SpeciesGroup
{
public:
   CString m_name;
   int     m_id;
   int     m_colScore;

   PtrArray< FishSpecie > m_sppArray;

   // temporary variables for scoring
   float m_presentCountReach;
   float m_presentCountTotal;
};



//------------------------------------------------------------------------------
// Urban water model supporting classes
//------------------------------------------------------------------------------

struct UWI
	{
	// Fields which are not time-dependent
	CString ugaName;
	int   ugbCode;       // UGB code from IDU coveragem_uwiArray
	bool bigFlag; // true => one of the "big" cities
	float aveCost_t0;
	float priceRes_t0;      // residential price ($/100 ft3)
	float priceNonRes_t0;   // comm/ind price ($/100 ft3)
	int   ibr;           // 1=Increasing Block Rate, 0=not
   float population_divisor; // fudge factor to allow for multiple water providers in big cities

	// Fields which contain exogenous time-dependent input data
	float incomeRes;     // median household income ($/household)
	float incomeComm_Mdollars;    // total income from commercial for area ($millions)
	float incomeInd_Mdollars;     // total income from Industrial for area ($millions)
	float coolingCost;   // total cooling cost for the UGA, $/(100,000 cubic feet))

	// Fields which are created in each annual timestep by summing
	// over the IDUs in the UGA.
	float population; // total population
   float urb_H2O_prc_multiplier; // average XURBH2OPRC
	float ugb_area_m2; // area of UGB, m2
//	float muniArea; // combined area of water districts in the UGB, acres
//	float muniFrac; // = muniArea/ugbArea

	// Fields which are calculated in each annual timestep.
	// Daily demand is average daily demand for the year.
	// Seasonality adjustment is made in the Flow model daily loop.
	// float qTotal;       // total daily water use, ccf/day
	double priceRes_lastyr; // $/ccf
	double priceNonRes_lastyr; // $/ccf
	double pricesmallcity_lastyr; // $/ccf
	double ac_lastyr; // $/ccf

	float qTotalDens;   // total daily water use/area of city, ccf/day/acre
	float qRes;         // big city residential daily water use, ccf/day
	float qNonRes;      // big city non-residential daily water use, ccf/day
	float aveCost;      // $/ccf
	float longRunAveCost; // $/ccf including capital costs

   // Fields calculated in the first timestep only.
   float origQperCapita_galperday; // per capita daily demand in gal/day as calculated in the first timestep
   float origResFrac; // fraction of Qtot which is Qresidential in big cities, as calculated in the first timestep

	UWI() :ugbCode(-1), bigFlag(false), aveCost_t0(0.0f), priceRes_t0(0.0f), priceNonRes_t0(0.0f), 
		ibr(0), population_divisor(1.f), incomeRes(0.0f), incomeComm_Mdollars(0.0f), incomeInd_Mdollars(0.0f),coolingCost(0.0f), population(0.0f), 
		qTotalDens(0.0f), qRes(0.0f), qNonRes(0.0f), 
		aveCost(0.0f), longRunAveCost(0.0f), origQperCapita_galperday(0.0f), origResFrac(0.0f) {}
	};
//------------------------------------------------------------------------------
// Land-use transition model supporting classes
//------------------------------------------------------------------------------

struct LTconstraintParams
   {
   float maxVal;
   float scaleFactor;
	LTconstraintParams() :maxVal(0.0f), scaleFactor(0.0f) {}
   }; // end of struct LTconstraintParams

struct LTparamSet
   {
   LTconstraintParams toAg, toForest, toDeveloped;
   }; // end of struct LTparamSet


struct DGVMvegtypeDataFILE
   {   
   CString DGVMvegtype_filename;
   };

struct PriceCCF
   {
   CString m_area;
   float m_residential_price; // $/ccf H2O
   float m_non_residential_price; // $/ccf H2O
   bool m_IBRflag; // true => increasing block rate (IBR) pricing
   float m_population_divisor; // divide population by this before using it in the expressions for AC and LRAC
	PriceCCF() : m_residential_price(0.0f), m_non_residential_price(0.0f), m_IBRflag(false), m_population_divisor(0.f) {}
   }; // end of struct PriceCCF

struct ModelScalar
   {
   CString m_symbol;
   double m_value;
   double m_offset_value;
   CString m_description;
	ModelScalar() :m_value(0.0), m_offset_value(0.0) {}
   }; // end of struct ModelScalar

struct ModelCoefficient
   {
   CString m_symbol;
   double m_coef_value;
	ModelCoefficient() :m_coef_value(0.0) {}
   };

class ModelTimeDependentCoefficient
   {
   public:
   ModelTimeDependentCoefficient() : m_pTDdata( NULL ) {} // default constructor
   ~ModelTimeDependentCoefficient() { if ( m_pTDdata ) delete m_pTDdata; } // default destructor
   float Get(float time);

   CString m_symbol;
   TARGET_METHOD m_method;
   CString m_description;

   // for method="linear" and method="exponential"
   float m_rate, m_start_value; 

   // for method="timesequence_linear" and method="timesequence_constant_rate"
   CString m_targetValues;
   FDataObj *m_pTDdata;   
   };

class TDdataset // time dependent dataset
   {
   public:
   TDdataset() : m_pTDdata( NULL ) {} // default constructor
   ~TDdataset() { if ( m_pTDdata != NULL ) delete m_pTDdata; } // default destructor
   float Get(float time);
   bool InterpretValueStr(CString scenarioName = "");

   CString m_symbol;
   CString m_description;
   TD_DATA_TYPE m_type;
   TD_DATA_METHOD m_method;
   CString m_value_str;
   FDataObj *m_pTDdata;

   }; // end of class TDdataset

struct UGAinfo
   {
   int id;
   CString name;
   Vertex city_center;
   float threshold_pct;
   int display_flag;
	UGAinfo() : id(-1), threshold_pct(0.0f), display_flag(-1) {}
   }; // end of struct UGAinfo

//////////////////////////////////////////////////////////////////////////

class APs : public EnvAutoProcess
{
public:
   APs(void);
   ~APs( void );

   BOOL Init   ( EnvContext *pContext, LPCTSTR initStr );
   BOOL InitRun( EnvContext *pContext, bool useInitSeed);
   BOOL Run    ( EnvContext *pContext );
   BOOL EndRun ( EnvContext *pContext );
   int InputVar( int id, MODEL_VAR** modelVar );
   int OutputVar( int id, MODEL_VAR** modelVar );
 
protected:
   bool LoadXml( EnvContext*, LPCTSTR filename );
   void RequiredTagIsMissing(CString tag, CString filename, CString whenClause);
   void RequiredTagIsMissing(CString tag, CString filename);

   //--------------------------------------------
   //---------- Cold Start --------------------
   //--methods
   bool InitColdStart( EnvContext *pContext ); // This is the Init() routine.
   int lookup(VDataObj * tableP, int nRec, int colIn, CString inString, int colOut);
   void lookup(VDataObj * tableP, int nRec, int colIn, CString inString, int colOut, CString * outStringP);
   int find(VDataObj * tableP, int nRec, int col, CString tgt);
   int VegclassNum(CString vegclassAbbrev);
   float LeafAreaIndex(CString vegclassAbbrev);
   float CarbonDensity(CString vegclassAbbrev);
   int UGBlookup(int ugbID);

//x   int m_pIDUlayer;

   int m_useColdStart;

   bool m_useColdStart_vegclass_initialization;
   bool m_useColdStart_standage_reconciliation;
   CString m_A2S_file; // age2SS.csv
   VDataObj m_A2S_table;
   int m_records_A2S;
   int m_colA2S_START_AGE, m_colA2S_fmh, m_colA2S_fwi, m_colA2S_fdw, m_colA2S_fdd;

   double m_initial_irrigation_fraction;

   bool m_useColdStart_SWHCfile;
   CString m_SWHCfile;
   GeoSpatialDataObj *m_pDataObj_SWHCtop;
   GeoSpatialDataObj *m_pDataObj_SWHCmid;
   GeoSpatialDataObj *m_pDataObj_SWHCdeep;
   GeoSpatialDataObj *m_pDataObj_SoilDepth;

   bool m_useColdStart_PopUnitsfile;
   CString m_PopUnits_file;
   VDataObj m_PopUnits_table; 
   int m_records_PU; 
   int m_colPU_IDU_ID, m_colPU_UGB_name, m_colPU_UGB, m_colPU_UGB_DH_ID, m_colPU_RR, m_colPU_RR_DH_ID;

   bool m_useColdStart_ForestSTMfiles;
   CString m_CoverTypes_file; VDataObj m_CoverTypes_table; int m_records_CT; int m_colCT_Abbrev, m_colCT_CTID;
   CString m_StructuralStages_file; VDataObj m_StructuralStages_table; int m_records_SS; int m_colSS_Abbrev, m_colSS_SSID;
   CString m_TransitionTypes_file; VDataObj m_TransitionTypes_table;
   CString m_Stratum2PVT_file; VDataObj m_Stratum2PVT_table;
   CString m_LAIandC_file; VDataObj m_LAIandC_table; int m_records_LAIandC; int m_colLAIandC_Abbrev, m_colLAIandC_LAI, m_colLAIandC_CARBON;
   CString m_DeterministicTransitionInput_file; VDataObj m_DeterministicTransitionInput_table;
   CString m_ProbabilisticTransitionInput_file; VDataObj m_ProbabilisticTransitionInput_table;
   // CString m_TransitionProbabilityMultiplierInput_file; VDataObj m_TransitionProbabilityMultiplierInput_table;
   CString m_DeterministicTransitionOutput_file;
   CString m_ProbabilisticTransitionOutput_file;
   // CString m_TransitionProbabilityMultiplierOutputFile;


   //--------------------------------------------
   //---------- GetWeather --------------------
   //--------------------------------------------
   bool InitRunGetWeather(EnvContext* pContext);
   bool RunGetWeather(EnvContext* pContext);

   RandUniform m_WeatherRandomDraw;
   int m_GetWeatherInVarIndex, m_GetWeatherInVarCount;
   bool m_useStationaryClim;
   int m_stationaryClimFirstYear;
   int m_stationaryClimLastYear;
   float m_stationaryClimSeed;


   //--------------------------------------------
   //---------- carbon model --------------------
   //--------------------------------------------
   bool InitCarbon( EnvContext *pContext );
   bool RunCarbon( EnvContext *pContext );
  
   int m_useCarbonModel;

   // carbon helper methods
   bool SetCarbonStocks( EnvContext *pEnvContext,int idu);
   

   //--------------------------------------------
   //---------- fish model ----------------------
   //--------------------------------------------
   bool InitFish( EnvContext *pContext );
   bool RunFish( EnvContext *pContext );

   int m_useFishModel, m_FishOutVarIndex, m_FishOutVarCount;

   // fish helper data/methods
   float      m_paThreshold;   // presence/absence threshold, decimal percent
   CUIntArray m_reachIndexArray;   
   SpeciesGroup *FindSppGroupFromID( int id );
   int GetSppGroupCount( void ) { return (int) m_sppGroupArray.GetSize(); }

   PtrArray< SpeciesGroup > m_sppGroupArray;
   float m_FishGroupPresenceProbability[MAX_FISH_GROUPS]; // presence/absence probability for each species group


   //--------------------------------------------
   //---------- Prescribed LULCs ---------------
   //--------------------------------------------
   bool InitPrescribedLULCs(EnvContext* pContext);
   bool InitRunPrescribedLULCs(EnvContext* pContext);
   bool RunPrescribedLULCs(EnvContext* pContext);

   bool AddIDUtoUGA(EnvContext* pContext, int idu_to_add, int ugb); // idu_to_add is an idu_index, not an idu_id

   int m_idPrescribedLULCs;
   int m_PLtestMode;
   CString m_LULCsFile;
   VDataObj m_PLtable;
   int m_PLcurrentRecord;
   int m_PLrecords;
   int m_colPLyear;
   int m_colPLidu_id;
   int m_colPLlulc;
   int m_colPLugb;



   //--------------------------------------------
   //---------- land use transitions model ---------------
   //--------------------------------------------
   bool InitLandTrans( EnvContext *pContext );
   BOOL InitRunLandTrans(EnvContext *pContext);
   bool RunLandTrans( EnvContext *pContext );
   bool LoadXmlTransition(LPCTSTR file_name, TiXmlElement *pXmlTransition, 
      LPCTSTR tgtFromUse,  LTparamSet *pConstraintSet);
   bool LoadXmlTransitionConstraint(LPCTSTR filename, TiXmlElement *pXmlTransition, LPCTSTR tgtToUse,   
      LTconstraintParams *pConstraint);
   double LTdevUsePredVal(int ugb, float acres, float pop_den, float HH_inc_Kdollars, float citydist, int county_group);
   double LTagUsePredVal(float acres, float slope, float citydist, int county_group, float farm_rent);
   double LTforUsePredVal(float acres, float slope, float elev, float riv_feet, bool PNI, float ugbdist, float citydist, int county_group);
   void CalculateLTOutputVariables(EnvContext *pContext);

   int m_idLandTrans, m_LTtestMode, m_LTinVarIndex, m_LTinVarCount, m_LToutVarIndex, m_LToutVarCount;
   PtrArray< ModelScalar > m_LTcoefArray;
   double m_LTdevBeta[12];
   ModelScalar m_LTdevCorrFactor;
   ModelScalar m_LTdevImp;
   double m_LTagBeta[11];
   ModelScalar m_LTagCorrFactor;
   ModelScalar m_LTagImp;
   ModelScalar m_LTagMeanImp;
   double m_LTforestBeta[14];
   ModelScalar m_LTforestCorrFactor;
   ModelScalar m_LTforestImp;
   ModelScalar m_LTforestMeanImp;
   ModelScalar m_LTself;
   ModelScalar m_LTcrossR;
   ModelScalar m_LT_DR;
   LTparamSet m_LTfromAg;
   LTparamSet m_LTfromForest;   
   RandUniform m_LTrandomDraw;
   FDataObj m_LToutVars;
   int m_LTag2devCount, m_LTag2forestCount, m_LTforest2devCount, m_LTforest2agCount;
   int m_LTagCount, m_LTforestCount;

   // for test mode
   double m_LT_X[14]; 
   float m_LTbetaX;
   int m_colDEV_VAL;
   int m_colAG_VAL;
   int m_colFOR_VAL;

   //--------------------------------------------
   //---------- urban water model ---------------
   //--------------------------------------------
   bool InitUrbanWater( EnvContext *pContext );
   bool InitRunUrbanWater(EnvContext *pContext);
   bool RunUrbanWater( EnvContext *pContext );
   float Get4UGB(FDataObj *pDataObj, int ugb, int row);
   bool ConfirmCols(FDataObj * pDataObj);
   bool CheckFCUpriceData();

   // output variable stuff
   int m_urbanWaterDemandIndex;
   int	m_urbanWaterDemandVarCount;
   FDataObj m_annualUrbanWaterDemand;
   FDataObj m_ECONincomeOutVar;

	float m_UGBUrbanWaterDemand[MAX_UGB]; // adjusted densities = pop in whole UGA / area of only private, developed part, persons/m2
	float m_metroDemandNonResidential;
	float	m_eugSprDemandNonResidential;
	float	m_salemKeiserDemandNonResidential;
	float	m_corvallisDemandNonResidential;
	float	m_albanyDemandNonResidential;
	float	m_mcMinnDemandNonResidential;
	float	m_newbergDemandNonResidential;
	float	m_woodburnDemandNonResidential;
	float m_metroDemandResidential;
	float	m_eugSprDemandResidential;
	float	m_salemKeiserDemandResidential;
	float	m_corvallisDemandResidential;
	float	m_albanyDemandResidential;
	float	m_mcMinnDemandResidential;
	float	m_newbergDemandResidential;
	float	m_woodburnDemandResidential;

   // getPermit(), getUse(), and getMuni() are in FlowInterface.h
   bool IsIrrigable(__int32 wrexists_arg)
      {
      return (IsIrrigation(GetUse(wrexists_arg)) &&
            (IsSurface(GetPermit(wrexists_arg)) ||
            IsGroundwater(GetPermit(wrexists_arg)))); 
      }
   bool IsIrrigableFromGroundwater(__int32 wrexists_arg)
      {
      return (IsIrrigation(GetUse(wrexists_arg)) && IsGroundwater(GetPermit(wrexists_arg)));
      }
   bool IsSurface(unsigned __int8 flags)      { return ((flags & WRP_SURFACE) > 0) ? true : false; }
   bool IsGroundwater(unsigned __int8 flags)  { return ((flags & WRP_GROUNDWATER) > 0) ? true : false; }
   bool IsIrrigation(unsigned __int16 flags)   { return ((flags & WRU_IRRIGATION) > 0) ? true : false; }

   int m_idUW, m_UWtestMode, m_UWinVarIndex, m_UWinVarCount;
   int m_currentUWScenarioIndex;  // 0=Ref, 1=FullCostUrb (aka FCU), 2=Extreme, 3=Managed, 4=Short Run Demand Model (aka SRD)

   CString m_UWmfgInc_file; 
   FDataObj * m_pUWmfgInc; 
   int m_UWrecords_mfgInc, m_UWmfgInc_yr0;
   CString m_UWcommInc_file; 
   FDataObj * m_pUWcommInc; 
   int m_UWrecords_commInc, m_UWcommInc_yr0;
   CString m_UWcoolingCost_file; 
   FDataObj * m_pUWcoolingCost; 
   int m_UWrecords_coolingCost, m_UWcoolingCost_yr0;
   CString m_UW_FCUresPrice_file;
   FDataObj * m_pUW_FCUresPrices;
   int m_UWrecords_FCUresPrices;
   CString m_UW_FCUindPrice_file;
   FDataObj * m_pUW_FCUindPrices;
   int m_UWrecords_FCUindPrices;

   PtrArray< PriceCCF > m_UWpriceArray;

   UWI m_uwiArray[ MAX_UGA_NDX+1 ];
   int m_colUW_AC;
   int m_colUW_LRAC;
   int m_colH2OPRICRES;
   int m_colH2OPRICIND;
   double m_UW_pumpingCost; // pumping cost for rural residential demand equation, $/ccf
   double m_UWruralDensity; // people per sq. mile

   //--------------------------------------------
   //---------- DGVM vegtype data process -------
   //--------------------------------------------
   bool InitRunDGVMvegtypeData( EnvContext *pContext );
   bool RunDGVMvegtypeData( EnvContext *pContext );
   bool GetDGVMvegtypeData( EnvContext *pContext, int yearNdx );
   int CrosswalkVtypeToKuchler(int vtype);

   int m_useDGVMvegtypeData;
   int m_DGVMvegtypeDataInVarIndex, m_DGVMvegtypeDataInVarCount;
   DGVMvegtypeDataFILE m_DGVMvegtypefile;
   GeoSpatialDataObj *m_pDataObj_DGVMvegtype;
   CString m_projectionWKT;   
 
   
   //--------------------------------------------
   //---------- Irrigation Choice process -------
   //--------------------------------------------
   bool InitIrrChoice( EnvContext *pContext );
   bool InitRunIrrChoice( EnvContext *pContext );
   bool RunIrrChoice( EnvContext *pContext );
  
   int m_idIrr, m_IrrTestMode;
   int m_IrrInVarIndex, m_IrrInVarCount;
   double m_IrrBeta[17];
   double m_IrrX[17];
   PtrArray< ModelCoefficient > m_IrrCoefficientArray;
// ModelTimeDependentCoefficient m_IrrTDcoefficient; // energy cost index = cost of energy in current year / cost of energy in first year
   CString m_IrrPrecip_filename;
   GeoSpatialDataObj *m_pDataObj_IrrPrecip;
   CString m_IrrProjectionWKT; 
   RandUniform m_IrrRandomDraw;

   int m_currentIrrScenarioIndex; // 0=Ref (~2/3), 1=More (~5/6) 2=None (0)


   //--------------------------------------------
   //---------- Population Growth process -------
   //--------------------------------------------
   BOOL InitRunPG(EnvContext *pContext);
   // bool InitPopGrowth( EnvContext *pContext );
   bool RunPopGrowth( EnvContext *pContext );
   void CalculatePopGrowthOutputVariables(EnvContext *pContext);
 
   int m_idPG, m_PGtestMode, m_PGoutVarIndex, m_PGoutVarCount;
   int m_PGinVarIndex, m_PGinVarCount;
   CString m_popGrowth_RPAcolName; int m_popGrowth_RPAcol;
   CString m_UGApopProj_file; VDataObj m_UGApopProj_table; int m_records_UPP; 
         int m_colUPP_UGB_DH_ID, m_colUPP_UGB_name, m_colUPP_pop2010;
   CString m_RPApopProj_file; VDataObj m_RPApopProj_table; int m_records_RPP; 
         int m_colRPP_RR_DH_ID, m_colRPP_pop2010;

   float m_UGBadjPopDensities[MAX_UGB]; // adjusted densities = pop in whole UGA / area of only private, developed part, persons/m2
   float m_MetroAdjDens, m_EugeneSpringfieldAdjDens, m_SalemAdjDens, m_CorvallisAdjDens, m_AlbanyAdjDens, m_McMinnvilleAdjDens,
         m_NewbergAdjDens, m_WoodburnAdjDens; // output variables, persons per acre
   float m_RPAareas[MAX_RR_DH_ID]; 
   float m_RPAtargetPops[MAX_RR_DH_ID];
   float m_RPAdensities[MAX_RR_DH_ID];
   float m_UGBadjAreas[MAX_UGB]; // for UGBs, this is only the area of developed, privately-owned IDUs in the UGB
   float m_UGBtargetPops[MAX_UGB];
   float m_UGBlegacyPops[MAX_UGB];
   float m_UGBdensities[MAX_UGB]; // = (total pop - legacy pop)/area of private, developed land
   FDataObj m_PGoutVars;

   //--------------------------------------------
   //------------- Farmland Rent subprocess -----
   //--------------------------------------------
   bool InitFR( EnvContext *pContext );
   bool InitRunFR( EnvContext *pContext );
   bool RunFR( EnvContext *pContext );
   bool InterpretXmlCoefficients(char * symbolPrefix, double coeff[], int numCoeff, PtrArray< ModelScalar > coeffsFromXml);

   int m_useFR, m_FRtestMode;
   int m_FRinVarIndex, m_FRinVarCount;
   PtrArray< ModelScalar > m_FRcoefficientArray;
   double m_FR_a[19];
   float m_FR_Eyr0;
   TDdataset m_FR_Cw_ground;    // cost of irrigation water from groundwater, $/acre
   TDdataset m_FR_Cw_surface;   // cost of irrigation water from surface water, $/acre

   FDataObj m_FRoutVars;
   int m_FRoutVarIndex;
   int m_FRoutVarCount;
   
//--------------------------------------------
   //------------- Crop Choice subprocess -----
   //--------------------------------------------
   bool InitCrop( EnvContext *pContext );
   bool InitRunCrop( EnvContext *pContext );
   bool RunCrop( EnvContext *pContext );
   bool LoadXmlTDdatasets(LPCTSTR file_name, TiXmlElement *pXmlSubProc, PtrArray< TDdataset > *pTDdatasetArray);
   bool LoadXmlTDdata(const char * symbolStr, LPCTSTR fileName, TiXmlElement *pXmlTDdata, TDdataset * pTDdata);

   int m_useCrop, m_CropTestMode;
   int m_CropInVarIndex, m_CropInVarCount;
   int m_currentCropScenarioIndex;

   CString m_cropfile; VDataObj m_crop_table; int m_records_crop; 
         int m_colCrop_beta, m_colCrop_grass_seed, m_colCrop_pasture, m_colCrop_wheat, m_colCrop_fallow, m_colCrop_corn,
         m_colCrop_clover, m_colCrop_hay;
   int m_cropCols[NUM_CROPS];
   float m_cropBeta[15][8]; // first index is beta subscript, second index is crop

   PtrArray< TDdataset > m_TDdatasetArray;
   TDdataset *m_pCropPG;
   TDdataset *m_pCropPW;
   TDdataset *m_pCropSnowpack;

   // double m_CropX[15];
   float m_CropMeanSnowpack;
   RandUniform m_CropRandomDraw;


    //--------------------------------------------
   //---------- UGB expansion model -------------
   //--------------------------------------------
   bool InitUX( EnvContext *pContext );
   bool InitRunUX(EnvContext *pContext);
   bool RunUX(EnvContext *pContext);
   void UXcalcDevelopedFracs(EnvContext *pContext);
   void UXsaveOutVars(EnvContext *pContext);
   bool UXaddIDUsToUGA(EnvContext * pContext, int ugb, Query *pIdu_query, int *pIdus_added, float *pArea_added);

   int m_idUX, m_UXtestMode, m_UXoutVarIndex, m_UXoutVarCount;
   int m_UXinVarIndex, m_UXinVarCount;
   FDataObj m_UXoutVars;
   float m_UXthresholdPct[MAX_UGA_NDX+1];
   double m_UXdevelopedArea[MAX_UGA_NDX+1];
   double m_UXdevelopableArea[MAX_UGA_NDX+1];
   // Vertex m_UXugaCityCenter[MAX_UGA_NDX+1];   
   UGAinfo m_UXugaArray[MAX_UGA_NDX+1];
   float m_UXdefaultThresholdPct;
   int m_currentUGAScenarioIndex; // 0=MIROC, 1=altUGA_threshold
   CString m_UGAthreshold_file; VDataObj m_UGAthreshold_table; int m_records_UGAthreshold;
		int m_colUGAthreshold_UGB, m_colUGAthreshold_UGB_NAME, m_colUGAthreshold_ref_thresh, m_colUGAthreshold_alt_thresh1;
   bool RunUgaExpansion( EnvContext *pContext );
   bool InitRunUgaExpansion( EnvContext *pContext );
   bool EndRunUgaExpansion( EnvContext *pContext );

   int m_colPopCap;
   int m_colPopAvail;
   int m_colUgaPriority;
   int m_colUgaExpEvent;
   int m_colAllowedDens;
   int m_colNdu;
   int m_colNdu00;
   int m_colNewDU;
   int m_colConserve;
   int m_colIDU_ID;
   int m_colZONED_USE;
   int m_colUGB_NAME;
   int m_colUGB;

   //--------------------------------------------
   //---- Common to 2 or more subprocesses ------
   //--------------------------------------------
   bool LoadXmlTDcoefficient(TiXmlElement *pXml0, LPCTSTR fileName);
   bool LoadXmlScalar(LPCTSTR file_name, TiXmlElement *pXmlScalar, LPCTSTR tgtSymbol, ModelScalar *pScalar);
   bool LoadXmlParameter(LPCTSTR file_name, TiXmlElement *pXmlParameter, LPCTSTR tgtSymbol, ModelScalar *pParameter);
   bool LoadXmlCoefficient(LPCTSTR file_name, TiXmlElement *pXmlCoefficient, LPCSTR tgtSymbol, ModelScalar *pCoefficient);
   bool LoadXmlUGA(LPCTSTR file_name, TiXmlElement *pXmlUGA);
   bool LoadXmlCoefficients(LPCTSTR file_name, TiXmlElement *pXmlSubProc, PtrArray< ModelScalar> *pCoefficientArray);
   bool LoadXmlUGAs(LPCTSTR file_name, TiXmlElement *pXmlSubProc);
   bool LoadXmlCoefficients(LPCTSTR file_name, TiXmlElement *pXmlSubProc, PtrArray< ModelScalar> *pCoefficientArray, LPCTSTR prefix);
   bool UpdateAgBasinClimateMean(EnvContext *pContext);
   bool InitializeGrdCellNdx() { ASSERT(0); return(true); } // unfinished
   bool StartXMLforSubprocess(EnvContext *pContext, LPCTSTR file_name, TiXmlElement * pXml_root, 
      CString subprocess_tag, int * p_id, TiXmlElement **ppXmlSubProc);
   bool StartXMLforSubprocess(EnvContext *pContext, LPCTSTR file_name, TiXmlElement * pXml_root, 
      CString subprocess_tag, int * p_id, TiXmlElement **ppXmlSubProc, int *pTestMode);
   bool CheckCols(MapLayer *pLayer, int lastNdx, LPCTSTR prefix, int * pCol0);
   bool InitializeTestMode(MapLayer *pLayer, int lastXndx, int lastPndx);
   BOOL GetUrbanPopData();
   void InitPopDensCalcs( EnvContext *pContext );
   bool attCheck(int attVal, int minVal, int maxVal, LPCTSTR attName);
   bool attCheck(float attVal, float minVal, float maxVal, LPCTSTR attName);
   void testMessage(EnvContext *pContext, LPCTSTR modelName);
   int CountyIDtoUse(int countyIDin);
   COUNTYcolumn CountyColFromCountyID(int countyID);

   EnvContext * m_pEnvContext;
   CString m_simulationScenario;
   int m_currentClimateScenarioIndex; // 0=Ref, 1=LowClim, 2=HiClim, 3=StationaryClim, 4=Historical, 5=Baseline
   char * m_climateScenarioName;
   int m_currentPopScenarioIndex;  // 0=Ref, 1=HiPop, 2=NoGrow, 3=Extreme, 4=NoPopGrowth, 5=NoIncGrowth, 6=Baseline
   char * m_popScenarioName;
   int m_dynamicWRType; // 0=none, 1=unused, 2=unused, 3=NewIrrig Scenario, 4=Extreme Scenario
   int m_testMode;
   bool m_commonInitComplete;
   bool m_commonInitRunComplete;
   bool m_commonEndRunComplete;
   bool m_climateFilesAreOpen;
   bool m_climateDataHasBeenInitialized;
   int m_yrOfMeanValues; // calendar year for which long term mean values were last updated
   float m_tauClimate, m_tauWR;
   ModelTimeDependentCoefficient m_TDcoefficient; // energy cost index = cost of energy in current year / cost of energy in first year
   CString m_Precip_filename;
   CString m_Tmin_filename;
   GeoSpatialDataObj *m_pDataObj_Precip;
   GeoSpatialDataObj *m_pDataObj_Tmin;
   CString m_ProjectionWKT;  
	bool  m_isBasin; // is grid cell within WW2100 study area boundary
   float m_BasinMeanTminGrowingSeason; // The basin area-weighted average for ag IDUs of TMINGROAVG, degC
   float m_BasinMeanPrcpGrowingSeason; // The basin area-weighted average for ag grid cells of PRCPGROAVG, inH2O
   CString m_HholdInc_file; 
   FDataObj * m_pHholdInc; 
   int m_recordsHholdInc, m_HholdInc_yr0;
   float m_UGAdevelopedFrac[MAX_UGA_NDX+1];
   int m_LandUseInterval; // number of years between updating predicted values for different land uses and
         // doing UGA expansions
  
   // for test mode
   int m_colX0; 
   int m_colBETAX;
   int m_colPROB0;


   ////// other stuff //////////
   // reference db columns
   //int m_colOrder;
   int m_colTMax;
   int m_colInSlices;

   int m_colCarbonStock;
   int m_colAGECLASS;
   int m_colDisturb;
   int m_colStandOrig;
   int m_colLulcA;
   int m_colLulcMed;

   int m_colH2ORESIDNT;
   int m_colH2OINDCOMM;
   int m_colAREA;

   int m_colPopDens;

   int m_colLCC;        // Land capability class
   int m_colCOUNTYID;

   int m_colRentCrops;
   int m_colRentPasture;
   int m_colRentForest;
   int m_colRentUrban; 
   int m_colRentBest;  

   int m_colNearestUgb;

   int m_ageClass;
   int m_lulcMed;
   int m_disturb;

   int m_colSWHC_TOP;
   int m_colSWHC_MID;
   int m_colSWHC_DEEP;
   int m_colSOIL_DEPTH;
   int m_colDGVMvegtyp;
   int m_colGrdCellNdx;
   int m_colRPA;
   int m_colOWNER;
   int m_colPOP;

   int m_colELEV;
   int m_colWREXISTS;
   int m_colDRAINAGE;
   int m_colAWS;
   int m_colD_WRIVER;
   int m_colD_CITY_20K;
   int m_colD_CITY_50K;
   int m_colD_GWATER;
   int m_colIRRIGATION;

   int m_colFRMRNT_LR;
   int m_colFRMRNT_YR;
   int m_colWR_SHUTOFF;
   int m_colWR_SH_NDX;
   int m_colIRRH2OCOST;

   int m_colSLOPE_DEG;
   int m_colVEGCLASS;

   int m_colSTART_AGE;
   int m_colEND_AGE;
   int m_colMEDIAN_AGE;

   int m_colPROB_DEV;
   int m_colPROBUSECHG;
   int m_colPVT;
   int m_colNRUGBndx;
   int m_colSTREAM_LEN;
   int m_colD_UGB_EE;

   int m_colMETRO_RSRV;
   int m_colD_HWY;
   int m_colCENTROIDX;
   int m_colCENTROIDY;

   int m_colLIFT;
   int m_colDISTANCE;
   int m_colECC0; // extra conveyance cost applicable when a new surface water irrigation right is established, $/ac
   int m_colECC; // extra conveyance cost associated with an irrigation water right, $/ac
   int m_colIRP0; // irrigation rent premium before subtracting the cost of irrigating, $/ac
   int m_colIRP; // current irrigation rent premium, $/ac
   int m_colWR_PROB; // probability of adding new stored water irrigation water right
   int m_colWR_IRRIG_S; // surface water irrigation water right, 1=>yes, 0=>no
   int m_colWRPRIORITY; // 1...n for most senior to most junior surface water irrigation water right

   int m_colKUCHLER; // Kuchler vegetation classification

   int m_colFLOODPLAIN;
   
   int m_colStateAbbre;
   int m_colPVTCode;
   int m_colCT_Text;
   int m_colSS_Text;

   int m_colLAI;

   int m_colXURBH2OPRC;
   int m_colXIRP_MIN;

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
};


inline
SpeciesGroup *APs::FindSppGroupFromID( int id )
   {
   for ( int i=0; i < (int) m_sppGroupArray.GetSize(); i++ )
      {
      if ( m_sppGroupArray[ i ]->m_id == id )
         return m_sppGroupArray[ i ];
      }

   return NULL;
   }

CString ReplaceSubstring(CString origStr,  CString strToBeReplaced,  CString replacementStr);
