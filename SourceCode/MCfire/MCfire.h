/*
 *  MCfire.h
 *  Whole Watershed Models
 */

#include <FDataObj.h>
#include <Vdataobj.h>
#include <MapLayer.h>
#include "..\Libs\ScienceFcns.h"
//#include <EnvEngine\EnvContext.h>

#include "..\Flow\FlowContext.h"

#define MAX_VTYPE 60

#define TOKEN_FOR_NO_LONGER_FOREST 1000000
#define TOKEN_FOR_FUELLOAD_FAILED 2000000
#define TOKEN_FOR_FIREBEHAV_FAILED 3000000

/*
void MC_FireModel::initializeFireModelConstantsAndOutputs()
{
   c_stl = c_std = 0.055;
   // mineral damping coefficent of live and dead fuels    
   float sd, sl;
   sd = sl = 0.01;
   c_etasd = 0.174 * pow(sd, -0.19);
   c_etasl = 0.174 * pow(sl, -0.19);

   fireParams.snw0 = 3.0f;
   fireParams.snw1 = 0.0f;
   fireParams.no_melt = 1.0f;
   fireParams.melt_b = 1.5f;

   fireParams.mc_tree_min = 80.f;
   fireParams.mc_tree_max = 130.f;
   fireParams.mc_grass_min = 30.f;
   fireParams.mc_grass_max = 120.f;

   fireParams.slp = 0.f;
   fireParams.prob_thres = 45.f;

   // These are output variables.
   m_d1hr = m_d10hr = m_d100hr = m_d1000hr = NC_FILL_FLOAT;
} // end of initializeFireModelConstantsAndOutputs()
*/

typedef struct
{
   float tree_ht_m = 0.;
   float crown_len_m = 0.;
   float bark_thick = 0.;
   float fine_fuel_frac = 0.;
   float d1hr = 0.; // g dry matter per m2
   float d10hr = 0.; // g dry matter per m2 
   float d100hr = 0.; // g dry matter per m2
   float d1000hr = 0.; // g dry matter per m2
   float lgras = 0.f; // live grass, g of biomass per m2
   float lwod1 = 0.;
   float lwod100 = 0.;
   float fuel_depth = 0.;
} FuelSupply;


typedef struct
{
   int idu_ndx; // index of IDU to which this state applies
   FuelSupply fuel; // Calculated in StartYear()

   float ave_ann_ppt_mm; // ???
   //float   sum_ann_ppt;
   //float		Pprev;
   float		Kprev = 0.f;
   float		Ksum = 0.f;
   float Kmax = 0.f;
   float Kcurr = 0.f; // current KBDI
   int		F1 = 0;

   // moisture content of dead fuel classes, %
   float	mc_litter;
   float	mc_1hr;
   float	mc_10hr;
   float mc_100hr = 35.f;;
   float mc_1000hr[7] = { 35.f, 35.f, 35.f, 35.f, 35.f, 35.f, 35.f }; // oldest to newest, daily 1000-hr fuel moisture content for 7 days
   float bnd_1000[7]; // oldest to newest, daily 1000-hr boundary conditions for 7 days

   // moisture content of live fuel classes
   float	mc_grass = 30.f;
   float	mc_tree = 80.f;

   float ffmc = 85.f; // Canadian Forest Fire Weather Index Fine Fuel Moisture Code
  float dmc = 6.f; // Canadian Forest Fire Weather Index Duff Moisture Code
  float dc = 15.f; // Canadian Forest Fire Weather Index Drought Code

   float tau; // residence time of flaming front in minutes
   float ire; // reaction intensity
} FireState;


typedef struct
{
   //float snw0, snw1, no_melt, melt_b; // parameters for daily snow model

   // parameters for live fuel moisture content
   float	mc_tree_min = 80.f;
   float mc_tree_max = 130.f;
   float mc_grass_min = 30.f;
   float mc_grass_max = 120.f;

   //float	slp; // parameter for slope of terrain
   //float prob_thres; // probability threshold for complete mortality
}  FireParams;


typedef struct
{
   float precip_mm;
   float temp_degC;
   float tmax_degC;
   float tmin_degC;
   float windspeed_m_per_sec;
   float vpd_Pa;
   float sphumidity;
   bool snow_flag;
   float pet;
} Weather;


class MCfire
{
   public:
      MCfire(void) {}
      ~MCfire(void) {}

      bool FuelLoad(int pvt, int vegclass, int ageclass, int standStartYear, int stm_index);
      float updateKBDI();
      bool fwi(float * pFFMC, float *pBUI, int monthNdx); // calculate Canadian FWI indices
      bool fuel_mc(float latitude_deg, int jday0); // Jan 1 = 0 for jday0; calculate dead fuel moisture
      bool FireBehav(int vegclass, float slope_deg);
      float liveFuelMoistureContent(float stress, float min_mc, float max_mc);
      int month0(int day0); // Returns 0 for Jan, 1 for Feb, ...

      FireState m_fire;
      float m_kbdi;
      Weather m_wthr;
      float	m_mc_1hr_frac; // fractional moisture content of 1-hour dead fuels
      float	m_mc_10hr_frac; // fractional moisture content of 10-hour dead fuels
      float m_mc_100hr_frac; // fractional moisture content of 100-hour dead fuels
      float m_mc_1000hr_frac; // fractional moisture content of 1000-hour dead fuels
      float m_mc_grass_frac; // fractional moisture content of live grass
      float m_mc_tree_frac; // fractional moisture content of live trees
      FireParams m_fireParams;

      float m_fli; // fire line intensity (BTU per foot per sec)
      float m_erc; // energy release component
      float m_ros; // rate of spread
      double m_zeta; // no-wind propagating flux ratio
      double m_sgbrt; // characteristic surface-to-volume (m2/m3) ratio for total fuel, weighted by volume
      double m_dedrt; // for dead fuels, the fraction of moisture of extinction represented by the actual moisture content
      double m_wtmcd; // weighted moisture content of dead fuels, as a fraction
      double m_rh_corr_pct; // corrected daytime relative humidity (%)

      VDataObj m_forestStatesTable; // deterministic transition table
      int m_colDetVEGCLASS;
      int m_colDetABBREV;
      int m_colDetPVT;
      int m_colDetLAI;
      int m_colDetGAP_FRAC;
      int m_colDetCARBON;
      int m_colDetDEAD_FUEL;
      int m_colDetLIVE_FUEL;
      int m_colDetHEIGHT_M;
      int m_colDetDBH;
      int m_colDetFUEL_DEPTH;
      int m_colDetFINE_FUEL_FRAC;
      int m_colDetFRAC_1HR_LIVE;
      int m_colDetFRAC_100HR_LIVE;

   protected:
      bool DeadFuelLoad(int pvt, int standAge, int standStartYear, const double * vveg2loadP);
      bool LiveFuelLoad(int pvt, int standAge, int standStartYear, int stmIndex, const double * vveg2loadP);

      void fuel_characteristics(int mc_vtype);
      bool min_mc_day(int idu_index); // find day with minimum 1000-hr mc
      void rateOfSpread(float slope_deg); // Calculates m_ros and m_sgbrt.
      float energyReleaseComponent();
      int days_in_month0(int month0);
      float VPDtoRH(float vpd_Pa, float temp_degC); // Returns relative humidity in percent.
      int MCvtypeOfForestVEGCLASS(int vegclass);
      float GetLitterFromSLCB(int pvt, int standAge, int standStartYear); // Returns litter in g dry matter per m2
      float GetTotDeadFuelFromSLCB(int pvt, int standAge, int standStartYear); // Returns total dead fuel in g dry matter per m2
      float GetTotLiveFuelFromSLCB(int pvt, int standAge, int standStartYear); // Returns total live fuel (aboveground live biomass) in g dry matter per m2
      float GetLiveStemDMfromSLCB(int pvt, int standAge, int standStartYear); // Returns live stem biomass in g dry matter per m2
      float GetLiveLeafDMfromSLCB(int pvt, int standAge, int standStartYear); // Returns live leaf biomass in g dry matter per m2

       // float m_bnd_1000; // 1000-hr boundary condition for the current day
      float m_ire; // reaction intensity
      float m_fd; // depth of flaming zone (ft)
      float m_tau; // residence time of flaming front (minutes)
      float m_bui; // build-up index
      float m_ffmc; // fine fuel moisture code
 
      // current fuel characteristics
      float m_w1p, m_w10, m_w100, m_w1000; // dead fuel classes in lbs per square foot
      float m_wherbp; // live herbaceous fuel (lbs per square foot)
      float m_wwood; // live wood (lbs per ft2)
      double m_sg1, m_sg10, m_sg100, m_sg1000, m_sgherb, m_sgwood, m_hl, m_hd;
      double m_mxd_frac, m_wndfac, m_mxl_frac;
      double m_livrt; // for live fuels, the fraction of moisture of extinction represented by the actual moisture content
      double m_sgbrte; // characteristic surface-to-volume (m2/m3) ratio for total fuel, weighted by mass
}; // end of class MCfire

 
class CW3Mfire
{
public:
   CW3Mfire(void);
   ~CW3Mfire(void) {}

   bool CW3Mfire_DailyProcess(FlowContext * pFlowContext);

protected:
   bool CW3Mfire_Init(void);
   bool CW3Mfire_InitRun(void);
   bool CW3Mfire_StartYear(void);
   // bool CW3Mfire_StartStep(void);
   bool CW3Mfire_Step(void);
   // bool CW3Mfire_EndStep();
   // bool CW3Mfire_EndYear();
   // bool CW3Mfire_EndRun();

   CArray< FireState, FireState > m_fire;
   MCfire m_Fm;

   FlowContext * m_pFlowContext;
   EnvContext * m_pEnvContext;
   FlowModel * m_pFlowModel;
   MapLayer * m_pIDUlayer;
   int m_numIDUs;

   int m_colFIRE_BUI;
   int m_colFIRE_FFMC;
   int m_colFIRE_ROS;
   int m_colFIRE_FLI;
   int m_colFIRE_ERC;
   int m_colVEGCLASS;
   int m_colPRECIP;
   int m_colVPD;
   int m_colTEMP;
   int m_colTMAX;
   int m_colTMIN;
   int m_colWINDSPEED;
   int m_colSPHUMIDITY;
   int m_colELEV_MEAN;
   int m_colLULC_A;
   int m_colSTM_INDEX;
   int m_colSNOWTHRU_D;
   int m_colSLOPE_DEG;
   int m_colFIRE_INDEX;
   int m_colCENTROIDY;
   int m_colPVT;
   int m_colAGECLASS;
   int m_colMAX_ET_DAY;
   //int m_colDEDRT;
   //int m_colZETA;
   //int m_colSGBRT;
   //int m_colWTMCD;
   //int m_colRHCORR_PCT;
   int m_colROS_MAX;
   int m_colROSMAX_DOY;
   int m_colFFMC_MAX;
   int m_colFFMCMAXDOY;
   int m_colBUI_MAX;
   int m_colBUIMAX_DOY;
   int m_colDC;
   int m_colDMC;
   int m_colTREE_HT;
   int m_colGAP_FRAC;
}; // end of class CW3Mfire


