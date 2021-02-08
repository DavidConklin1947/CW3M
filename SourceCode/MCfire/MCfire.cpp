// MCfire.cpp for Whole Watershed Models

#include "stdafx.h"
#include <stdio.h>

#include <Maplayer.h>
#include <Map.h>
#include <math.h>
#include <UNITCONV.H>
#include <path.h>
#include <EnvEngine\EnvModel.h>
#include <EnvInterface.h>
#include <direct.h>
#include "AlgLib\ap.h"
#include <Flow\Flow.h>
#include <PathManager.h>
#include "assert.h"

#include "MCFire.h"


CW3Mfire::CW3Mfire() :
   m_colFIRE_BUI(-1)
   , m_colFIRE_FFMC(-1)
   , m_colFIRE_ROS(-1)
   , m_colFIRE_FLI(-1)
   , m_colFIRE_ERC(-1)
   , m_colVEGCLASS(-1)
   , m_colAGECLASS(-1)
   , m_colPVT(-1)
   , m_colMAX_ET_DAY(-1)
//   , m_colDEDRT(-1)
//   , m_colZETA(-1)
//   , m_colSGBRT(-1)
//   , m_colWTMCD(-1)
//   , m_colRHCORR_PCT(-1)
   , m_colROS_MAX(-1)
   , m_colROSMAX_DOY(-1)
   , m_colFFMC_MAX(-1)
   , m_colFFMCMAXDOY(-1)
   , m_colBUI_MAX(-1)
   , m_colBUIMAX_DOY(-1)
   , m_colDC(-1)
   , m_colDMC(-1)
   , m_colGAP_FRAC(-1)
   , m_colTREE_HT(-1)
{}


bool CW3Mfire::CW3Mfire_DailyProcess(FlowContext * pFlowContext)
{
   m_pFlowContext = pFlowContext;
   m_pEnvContext = m_pFlowContext->pEnvContext;
   m_pFlowModel = m_pFlowContext->pFlowModel;
   m_pIDUlayer = (MapLayer *)m_pEnvContext->pMapLayer;

   int timing = pFlowContext->timing;

   if (timing & GMT_INIT) return(CW3Mfire_Init());
   if (timing & GMT_INITRUN) return(CW3Mfire_InitRun());
   if (timing & GMT_START_YEAR) return(CW3Mfire_StartYear());
   if (timing & GMT_CATCHMENT) return(CW3Mfire_Step());

   return(false);
} // end of CW3Mfire_DailyProcess()


bool CW3Mfire::CW3Mfire_Init()
{
   if (m_pFlowContext->m_extFnInitInfo.GetLength() <= 0)
   {
      CString msg; msg.Format("CW3Mfire_Init() m_pFlowContext->m_extFnInitInfo.GetLength() = %d", m_pFlowContext->m_extFnInitInfo.GetLength());
      Report::ErrorMsg(msg);
      return(false);
   }

   m_pIDUlayer->CheckCol(m_colFIRE_BUI, "FIRE_BUI", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colFIRE_FFMC, "FIRE_FFMC", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colFIRE_ROS, "FIRE_ROS", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colFIRE_FLI, "FIRE_FLI", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colFIRE_ERC, "FIRE_ERC", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colVEGCLASS, "VEGCLASS", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colPRECIP, "PRECIP", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colVPD, "VPD", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colTEMP, "TEMP", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colTMAX, "TMAX", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colTMIN, "TMIN", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colWINDSPEED, "WINDSPEED", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colSPHUMIDITY, "SPHUMIDITY", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colELEV_MEAN, "ELEV_MEAN", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colLULC_A, "LULC_A", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colSTM_INDEX, "STM_INDEX", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colSNOWTHRU_D, "SNOWTHRU_D", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colSLOPE_DEG, "SLOPE_DEG", TYPE_FLOAT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colFIRE_INDEX, "FIRE_INDEX", TYPE_INT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colCENTROIDY, "CENTROIDY", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colPVT, "PVT", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colAGECLASS, "AGECLASS", TYPE_INT, CC_MUST_EXIST);
   m_pIDUlayer->CheckCol(m_colMAX_ET_DAY, "MAX_ET_DAY", TYPE_FLOAT, CC_MUST_EXIST);
   //m_pIDUlayer->CheckCol(m_colDEDRT, "DEDRT", TYPE_FLOAT, CC_AUTOADD);
   //m_pIDUlayer->CheckCol(m_colZETA, "ZETA", TYPE_FLOAT, CC_AUTOADD);
   //m_pIDUlayer->CheckCol(m_colSGBRT, "SGBRT", TYPE_FLOAT, CC_AUTOADD);
   //m_pIDUlayer->CheckCol(m_colWTMCD, "WTMCD", TYPE_FLOAT, CC_AUTOADD);
   //m_pIDUlayer->CheckCol(m_colRHCORR_PCT, "RHCORR_PCT", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colROS_MAX, "ROS_MAX", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colROSMAX_DOY, "ROSMAX_DOY", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colFFMC_MAX, "FFMC_MAX", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colFFMCMAXDOY, "FFMCMAXDOY", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colBUI_MAX, "BUI_MAX", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colBUIMAX_DOY, "BUIMAX_DOY", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colDC, "DC", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colDMC, "DMC", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colGAP_FRAC, "GAP_FRAC", TYPE_FLOAT, CC_AUTOADD);
   m_pIDUlayer->CheckCol(m_colTREE_HT, "TREE_HT", TYPE_FLOAT, CC_AUTOADD);

   CString deterministic_filename = PathManager::MakeAbsolutePath(m_pFlowContext->m_extFnInitInfo, PM_IDU_DIR);
   CString msg; msg.Format("CW3Mfire_Init() deterministic_filename = %s", deterministic_filename); Report::LogMsg(msg);
   int records = m_Fm.m_deterministic_inputtable.ReadAscii(deterministic_filename, ',', TRUE);
   if (records <= 0)
   {
      msg.Format("CW3Mfire_Init() could not load deterministic transition .csv file. records = %d", records);
      Report::ErrorMsg(msg);
      return(false);
   }
   //int region_col = m_deterministic_inputtable.GetCol("REGION");
   m_Fm.m_colDetVEGCLASSfrom = m_Fm.m_deterministic_inputtable.GetCol("VEGCLASS");
   //int vto_col = m_deterministic_inputtable.GetCol("VEGCLASSto");
   m_Fm.m_colDetABBREVfrom = m_Fm.m_deterministic_inputtable.GetCol("ABBREV");
   //int abbvto_col = m_Fm.m_deterministic_inputtable.GetCol("ABBREVto");
   m_Fm.m_colDetPVT = m_Fm.m_deterministic_inputtable.GetCol("PVT");
   //int pvtto_col = m_Fm.m_deterministic_inputtable.GetCol("PVTto");
   //int startage_col = m_Fm.m_deterministic_inputtable.GetCol("STARTAGE");
   //int endage_col = m_Fm.m_deterministic_inputtable.GetCol("ENDAGE");
   //int rndage_col = m_Fm.m_deterministic_inputtable.GetCol("RNDAGE");
   m_Fm.m_colDetLAI = m_Fm.m_deterministic_inputtable.GetCol("LAI");
   m_Fm.m_colDetGAP_FRAC = m_Fm.m_deterministic_inputtable.GetCol("GAP_FRAC");
   m_Fm.m_colDetCARBON = m_Fm.m_deterministic_inputtable.GetCol("CARBON");
   m_Fm.m_colDetDEAD_FUEL = m_Fm.m_deterministic_inputtable.GetCol("DEAD_FUEL");
   m_Fm.m_colDetLIVE_FUEL = m_Fm.m_deterministic_inputtable.GetCol("LIVE_FUEL");
   m_Fm.m_colDetHEIGHT_M = m_Fm.m_deterministic_inputtable.GetCol("HEIGHT_M");
   m_Fm.m_colDetDBH = m_Fm.m_deterministic_inputtable.GetCol("DBH");
   m_Fm.m_colDetFUEL_DEPTH = m_Fm.m_deterministic_inputtable.GetCol("FUEL_DEPTH");
   m_Fm.m_colDetFINE_FUEL_FRAC = m_Fm.m_deterministic_inputtable.GetCol("FINE_FUEL_FRAC");
   m_Fm.m_colDetFRAC_1HR_LIVE = m_Fm.m_deterministic_inputtable.GetCol("FRAC_1HR_LIVE");
   m_Fm.m_colDetFRAC_100HR_LIVE = m_Fm.m_deterministic_inputtable.GetCol("FRAC_100HR_LIVE");

   if (m_Fm.m_colDetVEGCLASSfrom < 0 || m_Fm.m_colDetABBREVfrom < 0 || m_Fm.m_colDetPVT < 0 || m_Fm.m_colDetLAI < 0 
      || m_Fm.m_colDetGAP_FRAC < 0 || m_Fm.m_colDetCARBON < 0
      || m_Fm.m_colDetDEAD_FUEL < 0 || m_Fm.m_colDetLIVE_FUEL < 0 || m_Fm.m_colDetHEIGHT_M < 0 || m_Fm.m_colDetDBH < 0
      || m_Fm.m_colDetFUEL_DEPTH < 0 || m_Fm.m_colDetFINE_FUEL_FRAC < 0
      || m_Fm.m_colDetFRAC_1HR_LIVE < 0 || m_Fm.m_colDetFRAC_100HR_LIVE < 0)
   {
      msg.Format("CW3Mfire_Init() One or more column headings are incorrect or missing in the deterministic lookup file");
      Report::ErrorMsg(msg);
      return(false);
   }

   if (m_pEnvContext->coldStartFlag)
   {
      CW3Mfire_InitRun();
      CW3Mfire_StartYear();
   } // end of coldstart logic

   return(true);
} // end of CW3Mfire_Init()


bool CW3Mfire::CW3Mfire_InitRun()
{
   // Initialize the m_fire array.
   m_fire.RemoveAll();
   m_pIDUlayer->SetColDataU(m_colFIRE_INDEX, -1);
   int fire_index = 0;
   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      int lulc_a; m_pIDUlayer->GetData(idu, m_colLULC_A, lulc_a);
      if (lulc_a != LULCA_FOREST) continue;
      int pvt; m_pIDUlayer->GetData(idu, m_colPVT, pvt);
      if (pvt < 1 || pvt > 9) continue;

      m_pIDUlayer->SetDataU(idu, m_colFIRE_INDEX, fire_index);
      fire_index++;
   }
   m_fire.SetSize(fire_index);

   for (MapLayer::Iterator idu = m_pIDUlayer->Begin(); idu != m_pIDUlayer->End(); idu++)
   {
      int fire_index; m_pIDUlayer->GetData(idu, m_colFIRE_INDEX, fire_index);
      if (fire_index < 0) continue;

      m_fire[fire_index].idu_ndx = (int)idu;
      m_fire[fire_index].ave_ann_ppt_mm = 2000.f; // ???

      m_fire[fire_index].mc_grass = 30.f;
      m_fire[fire_index].mc_tree = 80.f;

      //m_fire[fire_index].mc_l1hr = 0;
      //m_fire[fire_index].mc_dstand = 0;
      m_fire[fire_index].mc_litter = 0;
      m_fire[fire_index].mc_1hr = 0;
      m_fire[fire_index].mc_10hr = 0;
      m_fire[fire_index].mc_100hr = 35.f;
      for (int i = 0; i < 7; i++)
      {
         m_fire[fire_index].mc_1000hr[i] = 35.f;
         m_fire[fire_index].bnd_1000[i] = 0.f;
      }
      m_fire[fire_index].Kcurr = m_fire[fire_index].Kprev = 0.f;
   }

   return(true);
} // end of CW3Mfire_InitRun()


bool CW3Mfire::CW3Mfire_StartYear()
{
   bool retval = true;

   m_pIDUlayer->SetColDataU(m_colFIRE_BUI, 0);
   m_pIDUlayer->SetColDataU(m_colBUI_MAX, 0);
   m_pIDUlayer->SetColDataU(m_colBUIMAX_DOY, -1);

   m_pIDUlayer->SetColDataU(m_colFIRE_FFMC, 0);
   m_pIDUlayer->SetColDataU(m_colFFMC_MAX, 0);
   m_pIDUlayer->SetColDataU(m_colFFMCMAXDOY, -1);

   m_pIDUlayer->SetColDataU(m_colFIRE_ROS, 0);
   m_pIDUlayer->SetColDataU(m_colROS_MAX, 0);
   m_pIDUlayer->SetColDataU(m_colROSMAX_DOY, -1);

   m_pIDUlayer->SetColDataU(m_colFIRE_FLI, 0);
   m_pIDUlayer->SetColDataU(m_colFIRE_ERC, 0);
   m_pIDUlayer->SetColDataU(m_colDC, 0);
   m_pIDUlayer->SetColDataU(m_colDMC, 0);

   m_pIDUlayer->SetColDataU(m_colTREE_HT, 0);

   for (int fire_ndx = 0; fire_ndx < m_fire.GetSize(); fire_ndx++)
   {
      FireState * pFS = &(m_fire[fire_ndx]);
      int idu_ndx = pFS->idu_ndx;
      if (idu_ndx < 0) continue;

      int lulc_a; m_pIDUlayer->GetData(idu_ndx, m_colLULC_A, lulc_a);
      if (lulc_a != LULCA_FOREST)
      { // Land use change may have moved an IDU from forest to ag or developed.
        // IDUs newly moved from ag to forest are being ignored; there are very few.
         CString msg;
         msg.Format("CW3Mfire_StartYear() Turning off CW3Mfire for idu_ndx = %d due to lulc_a = %d", idu_ndx, lulc_a);
         Report::LogMsg(msg);
         pFS->idu_ndx = -idu_ndx - TOKEN_FOR_NO_LONGER_FOREST;
         continue;
      }

      int pvt; m_pIDUlayer->GetData(idu_ndx, m_colPVT, pvt);
      int vegclass; m_pIDUlayer->GetData(idu_ndx, m_colVEGCLASS, vegclass);
      int ageclass; m_pIDUlayer->GetData(idu_ndx, m_colAGECLASS, ageclass);
      int standStartYear = m_pEnvContext->currentYear - ageclass;
      int stm_index; m_pIDUlayer->GetData(idu_ndx, m_colSTM_INDEX, stm_index);

      m_Fm.m_fire = m_fire[fire_ndx];
      if (!m_Fm.FuelLoad(pvt, vegclass, ageclass, standStartYear, stm_index))
      {
         CString msg;
         msg.Format("CW3Mfire_StartYear() Turning off CW3Mfire for idu_ndx = %d due to FuelLoad() failure", idu_ndx);
         Report::WarningMsg(msg);
         msg.Format("pvt = %d, vegclass = %d, ageclass = %d, standStartYear = %d, stm_index = %d", pvt, vegclass, standStartYear, stm_index);
         Report::LogMsg(msg);
         retval = false;
         pFS->idu_ndx = -idu_ndx - TOKEN_FOR_FUELLOAD_FAILED; 
         continue;
      }

      m_fire[fire_ndx] = m_Fm.m_fire;
      m_pIDUlayer->SetDataU(idu_ndx, m_colTREE_HT, m_Fm.m_fire.fuel.tree_ht_m);

      float gap_frac = m_Fm.m_deterministic_inputtable.GetAsFloat(m_Fm.m_colDetGAP_FRAC, stm_index);
      m_pIDUlayer->SetDataU(idu_ndx, m_colGAP_FRAC, gap_frac);
   } // end of loop thru array of fire states

   return(retval);
} // end of CW3Mfire_StartYear()


bool CW3Mfire::CW3Mfire_Step()
{
   bool retval = true;

   for (int fire_ndx = 0; fire_ndx < m_fire.GetSize(); fire_ndx++)
   {
      m_Fm.m_fire = m_fire[fire_ndx];
      int idu_ndx = m_fire[fire_ndx].idu_ndx;
      if (idu_ndx < 0) continue;

      //m_pIDUlayer->GetData(idu_ndx, m_colELEV_MEAN, m_elev_mean);
      float slope_deg; m_pIDUlayer->GetData(idu_ndx, m_colSLOPE_DEG, slope_deg);
      // ??? float centroidy; m_pIDUlayer->GetData(idu_ndx, m_colCENTROIDY, centroidy);
      float latitude_deg = 45.f; // ??? ScienceFcns::Northing_to_DegN(centroidy, UTM_ZONE);
      
      Weather wthr;
      m_pIDUlayer->GetData(idu_ndx, m_colPRECIP, wthr.precip_mm);
      m_pIDUlayer->GetData(idu_ndx, m_colTEMP, wthr.temp_degC);
      m_pIDUlayer->GetData(idu_ndx, m_colTMAX, wthr.tmax_degC);
      m_pIDUlayer->GetData(idu_ndx, m_colTMIN, wthr.tmin_degC);
      m_pIDUlayer->GetData(idu_ndx, m_colWINDSPEED, wthr.windspeed_m_per_sec);
      float vpd_kPa;  m_pIDUlayer->GetData(idu_ndx, m_colVPD, vpd_kPa);
      wthr.vpd_Pa = 1000.f * vpd_kPa;
      m_pIDUlayer->GetData(idu_ndx, m_colSPHUMIDITY, wthr.sphumidity);
      float snowthru_d; m_pIDUlayer->GetData(idu_ndx, m_colSNOWTHRU_D, snowthru_d);
      wthr.snow_flag = snowthru_d > 0.f;
      m_pIDUlayer->GetData(idu_ndx, m_colMAX_ET_DAY, wthr.pet);
      m_Fm.m_wthr = wthr;
      m_Fm.m_kbdi = m_Fm.updateKBDI();

      float ffmc, bui;
      int monthNdx = m_Fm.month0(m_pFlowContext->dayOfYear);
      m_Fm.fwi(&ffmc, &bui, monthNdx); // calculate Canadian FWI indices       
      m_Fm.fuel_mc(latitude_deg, m_pFlowContext->dayOfYear); // calculate dead fuel moisture       
                                        // convert moisture contents from % to fractions
      m_Fm.m_mc_1hr_frac = m_Fm.m_fire.mc_1hr * .01f;
      m_Fm.m_mc_10hr_frac = m_Fm.m_fire.mc_10hr * .01f;
      m_Fm.m_mc_100hr_frac = m_Fm.m_fire.mc_100hr * 0.01f;
      m_Fm.m_mc_1000hr_frac = m_Fm.m_fire.mc_1000hr[6] * 0.01f;
      m_Fm.m_mc_grass_frac = m_Fm.m_fire.mc_grass * 0.01f;
      m_Fm.m_mc_tree_frac = m_Fm.m_fire.mc_tree * 0.01f;

      float mc_duff = m_Fm.m_fire.mc_litter;
      float grass_stress = 1.f - mc_duff / 100.f;
      float mc_grass = m_Fm.liveFuelMoistureContent(grass_stress, m_Fm.m_fireParams.mc_grass_min, m_Fm.m_fireParams.mc_grass_max);

      float tree_stress = 1.f - m_Fm.m_fire.mc_tree / 100.f;
      m_Fm.m_fire.mc_tree = m_Fm.liveFuelMoistureContent(tree_stress, m_Fm.m_fireParams.mc_tree_min, m_Fm.m_fireParams.mc_tree_max);

      int vegclass; m_pIDUlayer->GetData(idu_ndx, m_colVEGCLASS, vegclass);
      if (!m_Fm.FireBehav(vegclass, slope_deg))
      {
         CString msg;
         msg.Format("CW3Mfire_Step() Turning off CW3Mfire for idu_ndx = %d due to FireBehav() failure", idu_ndx);
         Report::WarningMsg(msg);
         msg.Format("vegclass = %d, slope_deg = %f", vegclass, slope_deg);
         Report::LogMsg(msg);
         m_Fm.m_fire.idu_ndx = -idu_ndx - TOKEN_FOR_FIREBEHAV_FAILED;
         retval = false;
         continue;
      }    

      m_pIDUlayer->SetDataU(idu_ndx, m_colFIRE_BUI, bui);
      m_pIDUlayer->SetDataU(idu_ndx, m_colFIRE_FFMC, ffmc);
      m_pIDUlayer->SetDataU(idu_ndx, m_colFIRE_ROS, m_Fm.m_ros);
      m_pIDUlayer->SetDataU(idu_ndx, m_colFIRE_FLI, m_Fm.m_fli);
      m_pIDUlayer->SetDataU(idu_ndx, m_colFIRE_ERC, m_Fm.m_erc);

      //m_pIDUlayer->SetDataU(idu_ndx, m_colDEDRT, m_Fm.m_dedrt);
      //m_pIDUlayer->SetDataU(idu_ndx, m_colZETA, m_Fm.m_zeta);
      //m_pIDUlayer->SetDataU(idu_ndx, m_colSGBRT, m_Fm.m_sgbrt);
      //m_pIDUlayer->SetDataU(idu_ndx, m_colWTMCD, m_Fm.m_wtmcd);
      //m_pIDUlayer->SetDataU(idu_ndx, m_colRHCORR_PCT, m_Fm.m_rh_corr_pct);
      m_pIDUlayer->SetDataU(idu_ndx, m_colDC, m_Fm.m_fire.dc);
      m_pIDUlayer->SetDataU(idu_ndx, m_colDMC, m_Fm.m_fire.dmc);

      float ros_max; m_pIDUlayer->GetData(idu_ndx, m_colROS_MAX, ros_max);
      if (m_Fm.m_ros > ros_max)
      {
         m_pIDUlayer->SetDataU(idu_ndx, m_colROS_MAX, m_Fm.m_ros);
         m_pIDUlayer->SetDataU(idu_ndx, m_colROSMAX_DOY, m_pFlowContext->dayOfYear);
      }

      float ffmc_max; m_pIDUlayer->GetData(idu_ndx, m_colFFMC_MAX, ffmc_max);
      if (ffmc > ffmc_max)
      {
         m_pIDUlayer->SetDataU(idu_ndx, m_colFFMC_MAX, ffmc);
         m_pIDUlayer->SetDataU(idu_ndx, m_colFFMCMAXDOY, m_pFlowContext->dayOfYear);
      }

      float bui_max; m_pIDUlayer->GetData(idu_ndx, m_colBUI_MAX, bui_max);
      if (bui > bui_max)
      {
         m_pIDUlayer->SetDataU(idu_ndx, m_colBUI_MAX, bui);
         m_pIDUlayer->SetDataU(idu_ndx, m_colBUIMAX_DOY, m_pFlowContext->dayOfYear);
      }

      m_fire[fire_ndx] = m_Fm.m_fire;

   } // end of loop thru IDUs

   return(retval);
} // end of MCfire_Step()


int MCfire::month0(int day0) // Returns 0 for Jan, 1 for Feb, ...; defaults to current date
{
   int month_length[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 20, 31, 30, 31 };

   int days_so_far = 0;
   int month_ndx = 0;
   do
   {
      days_so_far += month_length[month_ndx];
      if (day0 < days_so_far) return(month_ndx);
      month_ndx++;
   } while (month_ndx < 12);
   return(11);
} // end of month0()


int MCfire::days_in_month0(int month0)
{
   int month_length[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 20, 31, 30, 31 };
   //if (month0 ==1 && isLeapYear(m_pEnvContext->calendar_year)) return(29);
   return(month_length[month0]);
}


bool MCfire::fwi(float * pFFMC, float * pBUI, int monthNdx)
// Calculate Canadian FWI indices.
// FWI is Canadian Forest Fire Weather Index System.  See Stocks et al. 1989.
// FFMC is fine fuel moisture code, one of 3 numerical ratings of the fuel moisture content.
// BUI is buildup index, one of 3 fire behavior indices.
// Uses metric units.
{
   *pFFMC = 0.;
   *pBUI = 0.;
   float ffmc_old = m_fire.ffmc;
   float dmc_old = m_fire.dmc;
   float dc_old = m_fire.dc;
   float el[] = { 6.5f, 7.5f, 9.0f, 12.8f, 13.9f, 13.9f, 12.4f, 10.9f, 9.4f, 8.0f, 7.0f, 6.0f };
   float fl[] = { -1.6f, -1.6f, -1.6f, 0.9f, 3.8f, 5.8f, 6.4f, 5.0f, 2.4f, 0.4f, -1.6f, -1.6f };

   float dc = -1.;
   float dmc = -1.;
   float ffmc = -1.;
   float bui, t, ra, wmo, ed, ew, z, x, wm, rk, pr, rw, wmi, b, wmr, pe;
   float smi, dr, p, cc; // ,bb,wnd;

///// CALC FFMC          
   wmo = 147.2f * (101.f - ffmc_old) / (59.5f + ffmc_old);
   float precip = m_wthr.precip_mm; // mm here, not inches
   if (precip>0.5)
   {
      ra = precip - 0.5F;
      if (wmo > 150.f) wmo += (float)
         (42.5*ra*exp(-100.0 / (251 - wmo))*(1.0 - exp(-6.93 / ra)) + 0.0015*(wmo - 150)*(wmo - 150)*sqrt(ra));
      else wmo += (float)(42.5*ra*exp(-100.0 / (251 - wmo))*(1.0 - exp(-6.93 / ra)));
   }
   if (wmo>250.f) wmo = 250.f;

   float temp_degC = m_wthr.temp_degC;
   float rh = VPDtoRH(m_wthr.vpd_Pa, temp_degC);
   ed = (float)(0.942*pow(rh, 0.679) + (11.0*exp((rh - 100.0) / 10.0)) +
      0.18*(21.1 - temp_degC)*(1.0 - 1.0 / exp(rh * 0.115)));
   ew = (float)(0.618*pow(rh, 0.753) + (10.0*exp((rh - 100.0) / 10.0)) +
      0.18*(21.1 - temp_degC)*(1.0 - 1.0 / exp(rh * 0.115)));

   float windspeed = m_wthr.windspeed_m_per_sec; // m per sec here, not mph
   if (wmo<ed && wmo<ew)
   {
      z = (float)(0.424*(1.0 - pow(((100.0 - rh) / 100.0), 1.7)) +
         0.0694*sqrt(windspeed)*(1.0 - pow((100.0 - rh) / 100.0, 8.0)));
      x = (float)(z * 0.581*exp(0.0365*temp_degC));
      wm = (float)(ew - (ew - wmo) / pow(10.0, x));
   }
   else if (wmo > ed)
   {
      z = (float)(0.424*(1.0 - pow((rh / 100.), 1.7)) + 0.0694*sqrt(windspeed)*(1.0 - pow(rh / 100, 8.0)));
      x = (float)(z * 0.581*exp(0.0365*temp_degC));
      wm = (float)(ed + (wmo - ed) / pow(10.0, x));
   }
   else wm = wmo;

   ffmc = (float)(59.5*(250.0 - wm) / (147.2 + wm));
   assert(!isnan(ffmc));
   if (ffmc>101.0f) ffmc = 101.0f;
   if (ffmc<0.0) ffmc = 0.f;
   *pFFMC = ffmc;
   m_fire.ffmc = ffmc;

///// CALC Duff Moisture Code DMC 
   t = temp_degC;
   if (t<-1.1f) t = -1.1f;

   rk = (float)(1.894*(t + 1.1)*(100.0 - rh)*el[monthNdx] * 0.0001);

   if (precip <= 1.5) pr = dmc_old;
   else
   {
      ra = precip; // ra is actual rain, mm
      rw = (float)(0.92*ra - 1.27); // rw is effective rain, mm
      wmi = (float)(20.0 + 280.0 / exp(0.023*dmc_old)); // wmi is the initial moisture content, %

      if (dmc_old <= 33) b = (float)(100.0 / (0.5 + 0.3*dmc_old));
      else if (dmc_old <= 65) b = (float)(14.0 - 1.3*log(dmc_old));
      else b = (float)(6.2*log(dmc_old) - 17.2);

      wmr = (float)(wmi + 1000.0*rw / (48.77 + b * rw)); // wmr is the moisture content after rain, %

      pr = (float)(43.43*(5.6348 - log(wmr - 20)));
   }

   if (pr<0) pr = 0;

   dmc = pr + rk;
   if (dmc<0) dmc = 0;
   if (dmc > 250.) dmc = 250.f;
   m_fire.dmc = dmc;

///// CALC Drought Code DC 
   if (temp_degC<-2.8) t = -2.8f;

   pe = (float)((.36*(t + 2.8) + fl[monthNdx]) / 2.0); // dry-day increment to DC
   if (pe<0.0) pe = 0.0;

   if (precip <= 2.8) dr = dc_old;
   else
   {
      ra = precip; // ra is actual rain, mm
      rw = (float)(0.83*ra - 1.27); // rw is effective rain, mm
      smi = (float)(800 * exp(-dc_old / 400)); // Stored Moisture Index equivalent of yesterday's Drought Code
      // Today's SMI is yesterday's SMI + 3.937 * effective rain
      // DC(today's SMI) = 400 * ln(800 / today's SMI)
      dr = (float)(dc_old - 400.0*log(1.0 + 3.937*rw / smi));
      if (dr < 0) dr = 0;
   }

   dc = dr + pe;
   if (dc<0) dc = 0;
   if (dc>2000.) dc = 2000.f;
   m_fire.dc = dc;

///// CALC BUI 
   if (dmc == 0 && dc == 0) bui = 0;
   else bui = 0.8f*dc*dmc / (dmc + 0.4f*dc);
   if (bui< dmc)
   {
      p = (dmc - bui) / dmc;
      cc = (float)(0.92 + pow((0.0114*dmc), 1.7));
      bui = dmc - cc * p;
      if (bui<0) bui = 0;
   }
   *pBUI = bui;

   return(true);
} // end of fwi()


bool MCfire::fuel_mc(float latitude_deg, int jday0)
// Calculate daily moisture values of 1-hr, 10-hr, 100-hr, and 1000-hr fuels.   
{
   float pet_today = m_wthr.pet;
   float ymc_100 = m_fire.mc_100hr;
   //float ymc_1000 = m_fire.mc_1000hr; 
   float tmax_degF = ScienceFcns::CtoF(m_wthr.tmax_degC);
   float tmin_degF = ScienceFcns::CtoF(m_wthr.tmin_degC);
   float temp_corr_F = ScienceFcns::CtoF(m_wthr.tmax_degC) + 15.f;
   float rhmin_pct = VPDtoRH(m_wthr.vpd_Pa, m_wthr.tmax_degC);
   m_rh_corr_pct = rhmin_pct * 0.87f; // correct daytime relative humidity 
   float rhmax_pct = VPDtoRH(m_wthr.vpd_Pa, m_wthr.tmin_degC);

   // Estimate precipitation duration, constrain to 8 hr 
   float ppt_in = ScienceFcns::mm_to_in(m_wthr.precip_mm);
   // estimate rainfall intensity using frontal vs. convectional pet threshold 
   int monthNdx = month0(jday0);
   float ppt_rat_in_per_hr = (pet_today * days_in_month0(monthNdx) > 100.f) ? 0.25f : 0.05f; 
   float ppt_dur = floor((ppt_in / ppt_rat_in_per_hr) + 0.49f);
   if (ppt_dur>8.0f) ppt_dur = 8.0f;

   if (m_wthr.tmin_degC < 0.f)
   { // set ppt_dur and rh_corr for frozen fuels 
      ppt_dur = 0.f;
      m_rh_corr_pct = 100.f;
   }

   // calculate corrected minimum equilibrium moisture content using corrected tmp and rh 
   float emc_min; // corrected minimum equilibrium moisture content
   if (m_rh_corr_pct<10.f) emc_min = (float)(0.03229 + 0.281073 * m_rh_corr_pct - 0.000578 * temp_corr_F * m_rh_corr_pct);
   else if (m_rh_corr_pct<50.f) emc_min = (float)(2.22749 + 0.160107 * m_rh_corr_pct - 0.014784 * temp_corr_F);
   else emc_min = (float)(21.06060 + 0.005565*pow(m_rh_corr_pct, 2.) - 0.00035*m_rh_corr_pct*temp_corr_F - 0.483199*m_rh_corr_pct);

   // calculate uncorrected minimum equilibrium moisture content using uncorrected tmp and rh 
   float emc_min_uc; // uncorrected minimum equilibrium moisture content
   if (rhmin_pct<10.f) emc_min_uc = (float)(0.03229 + 0.281073*rhmin_pct - 0.000578*tmax_degF * rhmin_pct);
   else if (rhmin_pct<50.f) emc_min_uc = (float)(2.22749 + 0.160107*rhmin_pct - 0.014784*tmax_degF);
   else emc_min_uc = (float)(21.06060 + 0.005565*pow(rhmin_pct, 2.) - 0.00035*rhmin_pct * tmax_degF - 0.483199*rhmin_pct);

   // calculate maximum equilibrium moisture content 
   float emc_max; // maximum equilibrium moisture content
   if (rhmax_pct < 10.f) emc_max = (float)(0.03229 + 0.281073*rhmax_pct - 0.000578*tmin_degF * rhmax_pct);
   else if (rhmax_pct<50.f) emc_max = (float)(2.22749 + 0.160107*rhmax_pct - 0.014784*tmin_degF);
   else emc_max = (float)(21.06060 + 0.005565*pow(rhmax_pct, 2.) - 0.00035*rhmax_pct * tmin_degF - 0.483199*rhmax_pct);

   // 1-HOUR AND 10-HOUR FUEL MOISTURE CONTENT 
   if ((ppt_in>0.f) || (tmin_degF<32.f)) m_fire.mc_1hr = m_fire.mc_10hr = 35.; // wet or frozen fuels 
   else
   {
      m_fire.mc_1hr = 1.0329f * emc_min;
      m_fire.mc_10hr = 1.2815f * emc_min;
   }

   // calculate daylength 
   float phi = latitude_deg * 0.01745f; // inputDataP->lat * 0.01745;
   int jdate = jday0 + 1;
   float decl = (float)(0.41008 * sin((jdate - 82.) * 0.01745));
   float arg = tan(phi) * tan(decl);
   if (arg < -1.f) arg = -1.f;
   else if (arg > 1.f) arg = 1.f;
   float daylit = (float)(24. * (1. - acos(arg) / PI));
 
   float emc_bar = // calculate weighted 24-hour average emc
      (float)((daylit * emc_min_uc + (24. - daylit) * emc_max) / 24.);
   assert(emc_bar > 0.);
   
   // 100-HR MC     
   float bnd_100 = // calculate 100-hr boundary condition for current day
      (float)(((24. - ppt_dur) * emc_bar + ppt_dur * (0.5 * ppt_dur + 41.)) / 24.);

   m_fire.mc_100hr = (float)(ymc_100 + (bnd_100 - ymc_100) * (1. - 0.87 * exp(-0.24)));
   if (m_fire.mc_100hr > 35.) m_fire.mc_100hr = 35.f;

   // 1000-HR MC 
   float bnd_1000_today = // calculate 1000-hr boundary condition for current day
      (float)(((24. - ppt_dur) * emc_bar + ppt_dur * (2.7 * ppt_dur + 76.)) / 24.);
   int days_of_data = 1;
   float data_sum = bnd_1000_today;
   for (int i = 6; (i > 0) && (m_fire.bnd_1000[i] != 0); i--) 
   { 
      data_sum += m_fire.bnd_1000[i];
      days_of_data++;
   }
   float bnd_bar = data_sum / days_of_data;
   float mc_1000hr_today = (float)(m_fire.mc_1000hr[0] +
      (bnd_bar - m_fire.mc_1000hr[0])*(1. - 0.82 * exp(-0.168)));
   if (mc_1000hr_today > 35.) mc_1000hr_today = 35.f;

   for (int i = 6; i > 0; i--)
   {
      m_fire.mc_1000hr[i - 1] = m_fire.mc_1000hr[i];
      m_fire.bnd_1000[i - 1] = m_fire.bnd_1000[i];
   }
   m_fire.mc_1000hr[6] = mc_1000hr_today;
   m_fire.bnd_1000[6] = bnd_1000_today;

   return(true);
} // end of MC_FireModel::fuel_mc()


float MCfire::liveFuelMoistureContent(float stress, float mc_min, float mc_max)
// Estimates percent moisture content of live herbaceous and tree fuel classes
{
   float x, y, mc;

   x = stress * 100.f;
   y = (float)(16.99365 + 84.59560 / (1. + exp(-(x - 39.88160) / -5.19634)));
   if (y>100.) y = 100.;
   else if (y<0.) y = 0.;

   mc = ((mc_max - mc_min) * (y / 100.f)) + mc_min;
   assert(mc >= mc_min && mc <= mc_max);

   return(mc);
} // end of MC_FireModel::liveFuelMoistureContent()


const double vveg2loadGlobal[][7] = {
// 0 frac_1hr 
// 1 frac_10hr 
// 2 frac_100hr 
// 3 frac_1000hr 
// 4 depth_ratio = bed_depth in feet of fuel bed per (ton per acre of fuel)
// 5 thick_ratio 
// 6 cl_ratio 
{ .00, .00, .00, .00, .0, .000, .0 }, // 0 unused
{ .00, .00, .00, .00, .0, .000, .0 }, // 1 ice aka barren
{ .25, .25, .25, .25, .4, .022, .8 }, // 2 tundra aka alpine
{ .27, .20, .24, .29, .042, .022, .8 }, // 3 taiga-tundra
{ .27, .20, .24, .29, .042, .022, .8 }, // 4 boreal needleleaf forest
{ .37, .26, .27, .10, .042, .043, .5 }, // 5 boreal mixed woodland
{ .27, .20, .24, .29, .042, .022, .8 }, // 6 subalpine forest
{ .20, .17, .20, .43, .042, .062, .7 }, // 7 maritime needleleaf forest
{ .39, .28, .14, .19, .042, .043, .5 }, // 8 temperate needleleaf forest
{ .62, .20, .18, .00, .042, .033, .4 }, // 9 temperate deciduous broadleaf forest
{ .37, .26, .27, .10, .042, .043, .5 }, // 10 temperate cool mixed forest
{ .45, .37, .16, .02, .042, .043, .5 }, // 11 temperate warm mixed forest
{ .61, .31, .06, .02, .042, .062, .4 }, // 12 temperate needleleaf woodland
{ .83, .07, .05, .05, .4, .033, .4 }, // 13 temperate deciduous broadleaf woodland
{ .83, .07, .05, .05, .4, .033, .4 }, // 14 temperate cool mixed woodland
{ .70, .18, .12, .00, .4, .043, .5 }, // 15 temperate warm mixed woodland
{ .72, .24, .02, .02, .4, .043, .7 }, // 16 C3 shrubland and temperate shrubland
{ .93, .05, .01, .01, 1.0, .022, .8 }, // 17 C3 grassland
{ .75, .24, .01, .00, .042, .043, .7 }, // 18 temperate desert
{ .39, .28, .14, .19, .042, .043, .5 }, // 19 subtropical needleleaf forest
{ .62, .20, .18, .00, .042, .033, .4 }, // 20 subtropical deciduous broadleaf forest 
{ .45, .37, .16, .02, .042, .043, .5 }, // 21 subtropical evergreen broadleaf forest
{ .45, .37, .16, .02, .042, .043, .5 }, // 22 subtropical mixed forest
{ .78, .19, .01, .02, .4, .062, .4 }, // 23 subtropical needleleaf woodland
{ .83, .07, .05, .05, .4, .033, .4 }, // 24 subtropical deciduous broadleaf woodland
{ .70, .18, .12, .00, .4, .043, .5 }, // 25 subtropical evergreen broadleaf woodland
{ .70, .18, .12, .00, .4, .043, .5 }, // 26 subtropical mixed woodland
{ .72, .24, .02, .02, .4, .043, .7 }, // 27 dry shrub-steppe
{ .92, .07, .01, .00, 1.0, .022, .8 }, // 28 C4 grassland
{ .75, .24, .01, .00, .042, .043, .7 }, // 29 subtropical desert
{ .62, .20, .18, .00, .042, .033, .4 }, // 30 tropical evergreen broadleaf forest 
{ .83, .07, .05, .05, .4, .033, .4 }, // 31 tropical deciduous woodland
{ .83, .07, .05, .05, .4, .033, .4 }, // 32 tropical savanna
{ .72, .24, .02, .02, .4, .043, .7 }, // 33 tropical shrubland
{ .92, .07, .01, .00, 1.0, .022, .8 }, // 34 tropical grassland
{ .75, .24, .01, .00, .042, .043, .7 }, // 35 tropical desert
{ .39, .28, .14, .19, .042, .043, .5 }, // 36 cool moist needleleaf forest
{ .72, .24, .02, .02, .4, .043, .7 }, // 37 unused or Lynx_AgricultureGrazing 
{ .93, .05, .01, .01, 1.0, .022, .8 }, // 38 subalpine meadow
{ .00, .00, .00, .00, .0, .000, .0 }, // 39 water and wetlands
{ .00, .00, .00, .00, .0, .000, .0 }, // 40 natural barren 
{ .00, .00, .00, .00, .0, .000, .0 }, // 41 developed
{ .27, .20, .24, .29, .042, .022, .8 }, // 42 larch forest 
{ .20, .17, .20, .43, .042, .062, .7 }, // 43 Sitka spruce zone, from 7 maritime needleleaf forest
{ .20, .17, .20, .43, .042, .062, .7 }, // 44 western hemlock zone, from 7 maritime needleleaf forest
{ .20, .17, .20, .43, .042, .062, .7 }, // 45 Pacific silver fir zone, 7 maritime needleleaf forest
{ .27, .20, .24, .29, .042, .022, .8 }, // 46 mountain hemlock zone, from 6 subalpine forest
{ .27, .20, .24, .29, .042, .022, .8 }, // 47 subalpine fir zone, from 6 subalpine forest
{ .37, .26, .27, .10, .042, .043, .5 }, // 48 subalpine parkland zone, from 5 boreal mixed woodland
{ .39, .28, .14, .19, .042, .043, .5 }, // 49 cool dry needleleaf forest
{ .72, .24, .02, .02, .4, .043, .7 }, // 50 boreal shrubland (from 16 shrub-steppe)
{ .72, .24, .02, .02, .4, .043, .7 }, // 51 semidesert shrubland (from 27 dry shrub-steppe)
{ .39, .28, .14, .19, .042, .043, .5 }, // 52 LPPZveg Lodgepole pine zone
{ .39, .28, .14, .19, .042, .043, .5 }, // 53 JPZveg Jeffrey pine zone
{ .39, .28, .14, .19, .042, .043, .5 }, // 54 WWPZveg Western white pine zone
{ .39, .28, .14, .19, .042, .043, .5 }, // 55 DFZ2veg Douglas-fir zone 2
{ .39, .28, .14, .19, .042, .043, .5 }, // 56 POCZveg Port Orford-cedar zone
{ .39, .28, .14, .19, .042, .043, .5 }, // 57 GFZveg Grand fir zone
{ .39, .28, .14, .19, .042, .043, .5 }, // 58 WFZveg White fir zone
{ .39, .28, .14, .19, .042, .043, .5 }, // 59 SRFZveg Shasta red fir zone
{ .39, .28, .14, .19, .042, .043, .5 } // 60 PPZveg Ponderosa pine zone
}; // end of vveg2loadGlobal[][7]


bool MCfire::FuelLoad(int pvt, int vegclass, int standage, int standStartYear, int stm_index)
{
   int num_records = sizeof(vveg2loadGlobal) / (7 * sizeof(double));
   assert(num_records >= (MAX_VTYPE + 1));
   int mc_vtype = MCvtypeOfForestVEGCLASS(vegclass);
   if (mc_vtype <= 0)
   {
      CString msg;
      msg.Format("FuelLoad() vegclass = %d, mc_vtype = %d", vegclass, mc_vtype);
      Report::WarningMsg(msg);
      return(false);
   }

   const double * vveg2loadP = &vveg2loadGlobal[mc_vtype][0];

   DeadFuelLoad(pvt, standage, standStartYear, vveg2loadP);
   LiveFuelLoad(pvt, standage, standStartYear, stm_index, vveg2loadP);

   // m_fire.fuel.fuel_depth = m_deterministic_inputtable.GetAsFloat(m_colDetFUEL_DEPTH, stm_index);
   float depth_ratio = (float)*(vveg2loadP + 4); 
   float littr_gDM_per_m2 = GetLitterFromSLCB(pvt, standage, standStartYear);
   float dstnd = littr_gDM_per_m2 + m_fire.fuel.d1hr;
   float dwod1 = m_fire.fuel.d10hr + m_fire.fuel.d100hr;
   float dwod100 = m_fire.fuel.d1000hr;
   float lgras = m_fire.fuel.lgras = 0.f;

   float tot_fuel_bed_bio = lgras + dstnd + dwod1 + dwod100;
   m_fire.fuel.fuel_depth = ScienceFcns::bed_depth(tot_fuel_bed_bio, depth_ratio);

   // m_fire.fuel.fine_fuel_frac = m_deterministic_inputtable.GetAsFloat(m_colDetFINE_FUEL_FRAC, stm_index);
   m_fire.fuel.fine_fuel_frac = (lgras + dstnd) / (lgras + dstnd + dwod1);

   return(true);
} // end of FuelLoad()


bool MCfire::DeadFuelLoad(int pvt, int standAge, int standStartYear, const double * vveg2loadP)
// Outputs: m_fire.fuel.d1hr, d10hr, d100hr, d1000hr
{
   float frac_1hr = (float)*(vveg2loadP + 0);
   float frac_10hr = (float)*(vveg2loadP + 1);
   float frac_100hr = (float)*(vveg2loadP + 2);
   float frac_1000hr = (float)*(vveg2loadP + 3);
   assert(sciFn.close_enough(frac_1hr + frac_10hr + frac_100hr + frac_1000hr, 1.0, .00001) || frac_1hr == 0.);
                                                                 //   m_fire.fuel.fine_fuel_frac = 0.1f;
   //float dead_fuel = m_deterministic_inputtable.GetAsFloat(m_colDetDEAD_FUEL, stm_index);
   float dead_fuel = GetTotDeadFuelFromSLCB(pvt, standAge, standStartYear);
   m_fire.fuel.d1hr = frac_1hr * dead_fuel;
   m_fire.fuel.d10hr = frac_10hr * dead_fuel;
   m_fire.fuel.d100hr = frac_100hr * dead_fuel;
   m_fire.fuel.d1000hr = frac_1000hr * dead_fuel;

   return(true);
} // end of DeadFuelLoad()


bool MCfire::LiveFuelLoad(int pvt, int standAge, int standStartYear, int stm_index, const double * vveg2loadP)
// Output: m_fire.fuel.tree_ht_m, 
//   m_fire.fuel.bark_thick
//   m_fire.fuel.crown_len_m
//   m_fire.fuel.lwod1, lwod100, lgras
{
/* ??? 
      <attr value = "1" color = "(63,127,0)" label = "temperate warm mixed forest (fdw)" / >
      <attr value = "2" color = "(0,197,255)" label = "subalpine forest (fmh)" / >
      <attr value = "3" color = "(0,168,132)" label = "moist temperate needleleaf forest (fsi)" / >
      <attr value = "4" color = "(255,127,127)" label = "C3 shrubland (fto)" / >
      <attr value = "5" color = "(0,98,0)" label = "cool mixed forest (fvg)" / >
      <attr value = "6" color = "(0,115,76)" label = "maritime needleleaf forest (fwi)" / >
      <attr value = "7" color = "(255,170,0)" label = "temperate needleleaf woodland (fuc)" / >
      <attr value = "8" color = "(127,255,0)" label = "subtropical mixed forest (ftm)" / >
      <attr value = "9" color = "(76,230,0)" label = "temperate needleleaf forest (fdd)" / >
*/
   ASSERT(1 <= pvt && pvt <= 9);
   bool is_pvt_deciduous_broadleaf[9] = { false, false, false, true, false, false, false, false, false };

   //float live_fuel = m_deterministic_inputtable.GetAsFloat(m_colDetLIVE_FUEL, stm_index);
   //float frac_1hr_live = m_deterministic_inputtable.GetAsFloat(m_colDetFRAC_1HR_LIVE, stm_index);
   //float frac_100hr_live = m_deterministic_inputtable.GetAsFloat(m_colDetFRAC_100HR_LIVE, stm_index);
   float frac_1hr_live = 0.0f; // no estimate of the proportion of live herbaceous fuel to live woody fuel for ground fire is available
   float frac_100hr_live = 0.0f; // no estimate of the proportion of live herbaceous fuel to live woody fuel for ground fire is available
   float live_wood_fuel = 0.; // no estimate of live woody fuel for ground fire is available 
   float lgras = 0.f; // no estimate of live herbaceous vegetation is available
   float live_fuel = live_wood_fuel + lgras;
   m_fire.fuel.lwod1 = frac_1hr_live * live_fuel; 
   m_fire.fuel.lwod100 = frac_100hr_live * live_fuel; 
   m_fire.fuel.lgras = lgras;

   //m_fire.fuel.tree_ht_m = m_deterministic_inputtable.GetAsFloat(m_colDetHEIGHT_M, stm_index);
   //float dbh = m_deterministic_inputtable.GetAsFloat(m_colDetDBH, stm_index);
   float dbh;
   float live_leaf_DM = GetLiveLeafDMfromSLCB(pvt, standAge, standStartYear);
   float live_stem_DM = GetLiveStemDMfromSLCB(pvt, standAge, standStartYear);
   float ltree = live_leaf_DM + live_stem_DM; // ??? maybe should include roots?
   float lai_during_fire_season = m_deterministic_inputtable.GetAsFloat(m_colDetLAI, stm_index);
   bool db_flag = is_pvt_deciduous_broadleaf[pvt];
   ScienceFcns::tree_dim(lai_during_fire_season, ltree, db_flag, &(m_fire.fuel.tree_ht_m), &dbh);

   float thick_ratio = (float)*(vveg2loadP + 5);
   m_fire.fuel.bark_thick = dbh * thick_ratio; // estimate bark thickness using vveg thickness to dbh ratio

   float cl_ratio = (float)*(vveg2loadP + 6);                                                              // dead fuels 
   m_fire.fuel.crown_len_m = m_fire.fuel.tree_ht_m * cl_ratio; // estimate crown length using vveg length to height ratio
                                                               // dead_wood(data_point, mo, yr);
   return(true);
} // end of LiveFuelLoad()


bool MCfire::FireBehav(int vegclass, float slope_deg)
//bool MCfire::FireOccur(int iduNdx, int mo, int yrs_since_fire, int vtype, int * fire_domP, int * fire_doyP)
// Returns dom and doy of fire.  If no fire, returns NO_FIRE for both.
{
   m_ros = m_fli = m_erc = 0;
   int mc_vtype = MCvtypeOfForestVEGCLASS(vegclass);
   if (mc_vtype <= 0)
   {
      CString msg;
      msg.Format("FireBehav() vegclass = %d, mc_vtype = %d", vegclass, mc_vtype);
      Report::WarningMsg(msg);
      return(false);
   }

   fuel_characteristics(mc_vtype);

   // Express fuel classes in lbs DM per square foot (converting from g DM per square meter)
   float factor = ScienceFcns::g_to_lbs(1) / (ScienceFcns::m_to_ft(1) * ScienceFcns::m_to_ft(1));
   m_w1p = m_fire.fuel.d1hr * factor;
   m_w10 = m_fire.fuel.d10hr * factor;
   m_w100 = m_fire.fuel.d100hr * factor;
   m_w1000 = m_fire.fuel.d1000hr * factor;
   m_wherbp = m_fire.fuel.lgras * factor;
   m_wwood = (m_fire.fuel.lwod1 + m_fire.fuel.lwod100) * factor;  

   rateOfSpread(slope_deg);
   energyReleaseComponent(); 

   m_fd = m_ros * m_tau; // depth of flaming zone in ft
   m_fli = m_ire * (m_fd / 60.f); // BTU per ft per sec

   return(true);
} // end of FireBehav()


void MCfire::rateOfSpread(float slope_deg) // Calculates m_ros and m_sgbrt.
{
   m_ros = 0;
   m_sgbrt = m_dedrt = m_livrt = m_zeta = 0;

   // total dead, total live, and total fuel load, excluding 1000hr dead fuel 
   float wtotd, wtotl, wtot; // lbs ft-2
   wtotd = m_w1p + m_w10 + m_w100; 
   wtotl = m_wherbp + m_wwood;
   wtot = wtotd + wtotl;
   if (wtot < 0.1) return;

   if (m_kbdi > 100.)
   { // duff drying with drought
      float w1f, w10f, w100f, unit_incr, tot_incr;
      w1f = m_w1p / wtotd;
      w10f = m_w10 / wtotd;
      w100f = m_w100 / wtotd;

      unit_incr = m_w1p / 700.f;
      tot_incr = (m_kbdi - 100.f) * unit_incr;

      m_w1p += (w1f    * tot_incr);
      m_w10 += (w10f   * tot_incr);
      m_w100 += (w100f  * tot_incr);

      wtotd = m_w1p + m_w10 + m_w100; 
      wtot = wtotd + wtotl;
   }

   // Calculate surface area of each class.
   // fuel weights (m_w1p, m_w10, ...) are in lbs per square foot
   // surface area to volume ratios (m_sg1, m_sg10, ...) are in m2/m3
   float sa1_m2, sa10_m2, sa100_m2, saherb_m2, sawood_m2, sadead_m2, salive_m2;
   float rhol = 32.f; // weight per cubic foot of live fuel
   float rhod = 32.f; // weight per cubic foot of dead fuel
   sa1_m2 = ScienceFcns::ft3_to_m3(m_w1p / rhod) * (float)m_sg1;
   sa10_m2 = ScienceFcns::ft3_to_m3(m_w10 / rhod) * (float)m_sg10;
   sa100_m2 = ScienceFcns::ft3_to_m3(m_w100 / rhod) * (float)m_sg100;
   saherb_m2 = ScienceFcns::ft3_to_m3(m_wherbp / rhol) * (float)m_sgherb;
   sawood_m2 = ScienceFcns::ft3_to_m3(m_wwood / rhol) * (float)m_sgwood;
   sadead_m2 = sa1_m2 + sa10_m2 + sa100_m2;
   salive_m2 = saherb_m2 + sawood_m2;

   // Calculate weighting factors of each fuel class.
   float f1, f10, f100, fherb, fwood, fdead, flive;
   if (sadead_m2 <= 0.) f1 = f10 = f100 = 0.;
   else
   {
      f1 = sa1_m2 / sadead_m2;
      f10 = sa10_m2 / sadead_m2;
      f100 = sa100_m2 / sadead_m2;
   }

   if (salive_m2 <= 0.) fherb = fwood = 0.;
   else
   {
      fherb = saherb_m2 / salive_m2;
      fwood = sawood_m2 / salive_m2;
   }

   if ((sadead_m2 + salive_m2) <= 0.0) fdead = flive = 0.;
   else
   {
      fdead = sadead_m2 / (sadead_m2 + salive_m2);
      flive = salive_m2 / (sadead_m2 + salive_m2);
   }

   // net fuel loading of each fuel class 
   float std, stl, w1n, w10n, w100n, wherbn, wwoodn; // lbs per sq ft
   std = stl = 0.0555f;
   w1n = m_w1p * (1.f - std);
   w10n = m_w10 * (1.f - std);
   w100n = m_w100 * (1.f - std);
   wherbn = m_wherbp * (1.f - stl);
   wwoodn = m_wwood * (1.f - stl);

   // weighted net loadings of dead and live fuels 
   float wdeadn, wliven;
   wdeadn = (f1 * w1n) + (f10 * w10n) + (f100 * w100n);
   wliven = (fwood * wwoodn) + (fherb * wherbn);

   // dead and live fuel characteristic surface area-to-volume ratios 
   // N.B. sg1, sg10, ... are in m2/m3
   // Before using them with Rothermel equations, convert to ft2/ft3.
   // A cylinder (e.g. branch or stem) has sgbrt_per_m = length_m * 2 * pi * radius_m / (length_m * pi * radius_m^2) 
   // = 2 / radius_m = 4 / diameter_m 
   // Similarly, sgbrt_per_ft = 4 / diameter_ft
   // So sgbrt_per_ft = 4 / (diameter_m * ft/m) = (4 / diameter_m) * (m/ft) = sgbrt_per_m * m/ft = ~ one-third of sgbrt_per_m
   float sgbrd_per_m, sgbrl_per_m;
   sgbrd_per_m = (float)((f1 * m_sg1) + (f10 * m_sg10) + (f100 * m_sg100));
   sgbrl_per_m = (float)((fherb * m_sgherb) + (fwood * m_sgwood));

   double sgbrt_per_m = (fdead * sgbrd_per_m) + (flive * sgbrl_per_m); // characteristic surface area-to-volume ratio, in m2/m3
   double sgbrt = sgbrt_per_m * M_PER_FT;

   double betop = 3.348f * pow(sgbrt, -0.8189f); // optimum packing ratio 

                                              // packing ratio 
   float depth_in_feet = m_fire.fuel.fuel_depth * FT_PER_M;
   float rhobed, rhobar, betbar;
   rhol = rhod = 32.f;
   rhobed = wtot / depth_in_feet;
   rhobar = ((wtotl * rhol) + (wtotd * rhod)) / wtot;
   betbar = rhobed / rhobar;

   // float propr_rat = betbar / betop; // calculate packing ratio : optimum packing ratio 
   double gmamx = (pow(sgbrt, 1.5f)) / (495.f + 0.0594f * pow(sgbrt, 1.5f)); // maximum reaction velocity

                                                                        // optimum reaction velocity
   double ad = 133.f*pow(sgbrt, -0.7913f);
   double gmaop = gmamx * pow((betbar / betop), ad) * exp(ad * (1.0f - betbar / betop));

   // no wind propagating flux ratio 
   m_zeta = exp((0.792f + 0.681f * pow(sgbrt, 0.5f)) * (betbar + 0.1f)) / (192.f + 0.2595f * sgbrt);

   // heating numbers of each fuel class 
   float hn1, hn10, hn100, hnherb, hnwood;
   hn1 = (float)(w1n * exp(-138. / m_sg1));
   hn10 = (float)(w10n * exp(-138. / m_sg10));
   hn100 = (float)(w100n * exp(-138. / m_sg100));
   hnherb = (float)(wherbn * exp(-500. / m_sgherb));
   hnwood = (float)(wwoodn * exp(-500. / m_sgwood));

   // ratio of dead-to-live fuel heating numbers 
   float wrat = ((hnherb + hnwood) <= 0.f) ? -1.f : (hn1 + hn10 + hn100) / (hnherb + hnwood);

   // weighted dead_fuel moisture content for live-fuel extinction moisture 
   float mclfe = ((m_mc_1hr_frac * hn1) + (m_mc_10hr_frac * hn10) + (m_mc_100hr_frac * hn100))
      / (hn1 + hn10 + hn100);

   // moisture extinction of live fuels 
   if (wrat <= 0.) m_mxl_frac = -1.;
   else
   {
      m_mxl_frac = 2.9 * wrat * (1. - mclfe / m_mxd_frac) - 0.226;
      if (m_mxl_frac<m_mxd_frac) m_mxl_frac = m_mxd_frac;
   }

   // weighted moisture content of dead and live fuels
   float wtmcl;
   m_wtmcd = (f1 * m_mc_1hr_frac) + (f10 * m_mc_10hr_frac) + (f100 * m_mc_100hr_frac);
   wtmcl = (fherb * m_mc_grass_frac) + (fwood * m_mc_tree_frac);

   // mineral damping coefficent of live and dead fuels    
   float sd, sl, etasd, etasl;
   sd = sl = 0.01f;
   etasd = (float)(0.174 * pow(sd, -0.19));
   etasl = (float)(0.174 * pow(sl, -0.19));

   // moisture damping coefficents of dead and live fuels 
   float etamd, etaml;
   m_dedrt = (float)(m_wtmcd / m_mxd_frac);
   m_livrt = (float)(wtmcl / m_mxl_frac);
   etamd = (float)(1. - 2.59 * m_dedrt + 5.11 * pow(m_dedrt, 2.) - 3.52 * pow(m_dedrt, 3.));
   if (etamd < 0.) etamd = 0.;
   else if (etamd > 1.) etamd = 1.;
   etaml = (float)(1. - 2.59 * m_livrt + 5.11 * pow(m_livrt, 2.) - 3.52 * pow(m_livrt, 3.));
   if (etaml < 0.) etaml = 0.;
   else if (etaml > 1.) etaml = 1.;

   // wind effect multiplier  
   float wnd_mph, b_eff, c_var, e_eff, ufact, phiwnd;
   wnd_mph = ScienceFcns::m_per_sec_to_mph(m_wthr.windspeed_m_per_sec); 
   b_eff = (float)(0.02526 * pow(sgbrt, 0.54));
   c_var = (float)(7.47 * exp(-0.133 * pow(sgbrt, 0.55)));
   e_eff = (float)(0.715 * exp(-3.59 * pow(10., -4.) * sgbrt));
   ufact = (float)(c_var * pow((betbar / betop), (e_eff * -1.)));
   phiwnd = (float)(ufact * pow((wnd_mph * 88.f * m_wndfac), b_eff));

   // slope effect multiplier coefficient 
   float slpfct, phislp;
   if (slope_deg <= 25.f) slpfct = 0.267f;
   else if (slope_deg <= 40.f) slpfct = 0.533f;
   else if (slope_deg <= 55.f) slpfct = 1.068f;
   else if (slope_deg <= 75.f) slpfct = 2.134f;
   else slpfct = 4.273f;
   phislp = slpfct * pow(betbar, -0.3f);

   // reaction intensity 
   float ir = (float)(gmaop * ((wdeadn * m_hd * etasd * etamd) + (wliven * m_hl * etasl * etaml)));

   // correction to phiwnd for high winds 
   if ((wnd_mph * 88.f * m_wndfac) > (0.9f * ir)) phiwnd = ufact * pow((0.9f * ir), b_eff);

   // heat sink 
   float htsink = (float)(rhobed *
      (fdead *
      ((f1    * exp(-138.f / m_sg1)    * (250.f + 1116.f * m_mc_1hr_frac)) +
         (f10   * exp(-138.f / m_sg10)   * (250.f + 1116.f * m_mc_10hr_frac)) +
         (f100  * exp(-138.f / m_sg100)  * (250.f + 1116.f * m_mc_100hr_frac)))) +
         (flive *
      ((fherb * exp(-138.f / m_sgherb) * (250.f + 1116.f * m_mc_grass_frac)) +
            (fwood * exp(-138.f / m_sgwood) * (250. + 1116.f * m_mc_tree_frac)))));

   // rate of spread in ft/min       
   m_ros = (float)(ir * m_zeta * (1. + phislp + phiwnd) / htsink);
   m_sgbrt = sgbrt;

} // end of rateOfSpread()


void MCfire::fuel_characteristics(int mc_vtype)
{
   double * vveg2fuelP;
   // 0-5 surface-to-volume ratios (m2/m3) for dead and live fuels
   // 0 sg1    1hr dead
   // 1 sg10   10hr dead
   // 2 sg100  100hr dead
   // 3 sg1000 1000hr dead
   // 4 sgwood live woody (branches & stems of small shrubby woody stuff close enough to the surface to support ground fire)
   // 5 sgherb live herbaceous
   // 6 hl - heat of combustion for live fuels, BTU/lb
   // 7 hd - heat of combustion for dead fuels, BTU/lb
   // 8 mxd - moisture of extinction for dead fuels, %
   // 9 wndfac - wind-reduction factor
   const double vveg2fuelGlobal[][10] = {
      { .00, .00, .00, .00, .0, .000, .0, .0, .0, .0 }, // 0 unused
   { .00, .00, .00, .00, .0, .000, .0, .0, .0, .0 }, // 1 ice aka barren
   { 1959., 109., 30., 8., 1462., 1950., 8001., 8001., 30., 0.5 }, // 2 tundra aka alpine
   { 1852., 109., 30., 8., 1470., 1984., 8039., 8039., 30., 0.4 }, // 3 taiga-tundra
   { 1852., 109., 30., 8., 1470., 1984., 8039., 8039., 30., 0.4 }, // 4 boreal needleleaf forest
   { 1634., 109., 30., 8., 1478., 1967., 8026., 8026., 30., 0.4 }, // 5 boreal mixed woodland
   { 1852., 109., 30., 8., 1470., 1984., 8039., 8039., 30., 0.4 }, // 6 subalpine forest
   { 1960., 109., 30., 8., 1488., 2045., 8068., 8068., 23., 0.4 }, // 7 maritime needleleaf forest
   { 1937., 109., 30., 8., 1489., 2120., 8053., 8053., 21., 0.4 }, // 8 temperate needleleaf forest
   { 1730., 109., 30., 8., 1499., 2003., 8006., 8006., 30., 0.5 }, // 9 temperate deciduous broadleaf forest
   { 1634., 109., 30., 8., 1478., 1967., 8026., 8026., 30., 0.4 }, // 10 temperate cool mixed forest
   { 1670., 109., 30., 8., 1499., 2012., 8210., 8210., 30., 0.4 }, // 11 temperate warm mixed forest
   { 2072., 109., 30., 8., 1451., 2059., 8293., 8293., 16., 0.6 }, // 12 temperate needleleaf woodland
   { 1906., 109., 30., 8., 1442., 1909., 8016., 8016., 17., 0.6 }, // 13 temperate deciduous broadleaf woodland
   { 1906., 109., 30., 8., 1442., 1909., 8016., 8016., 17., 0.6 }, // 14 temperate cool mixed woodland
   { 1433., 109., 30., 8., 1386., 2000., 8680., 8680., 15., 0.6 }, // 15 temperate warm mixed woodland
   { 2326., 109., 30., 8., 1497., 1978., 8020., 8020., 16., 0.6 }, // 16 shrub-steppe
   { 2020., 109., 30., 8., 1498., 2021., 8014., 8014., 16., 0.6 }, // 17 C3 grassland
   { 2425., 109., 30., 8., 1488., 1750., 8072., 8072., 15., 0.6 }, // 18 temperate desert
   { 1937., 109., 30., 8., 1489., 2120., 8053., 8053., 21., 0.5 }, // 19 subtropical needleleaf forest
   { 1730., 109., 30., 8., 1499., 2003., 8006., 8006., 30., 0.5 }, // 20 subtropical deciduous broadleaf forest 
   { 1670., 109., 30., 8., 1499., 2012., 8210., 8210., 30., 0.4 }, // 21 subtropical evergreen broadleaf forest
   { 1670., 109., 30., 8., 1499., 2012., 8210., 8210., 30., 0.4 }, // 22 subtropical mixed forest
   { 2232., 109., 30., 8., 1500., 2023., 8001., 8001., 16., 0.6 }, // 23 subtropical needleleaf woodland
   { 1906., 109., 30., 8., 1442., 1909., 8016., 8016., 17., 0.6 }, // 24 subtropical deciduous broadleaf woodland
   { 1433., 109., 30., 8., 1386., 2000., 8680., 8680., 15., 0.6 }, // 25 subtropical evergreen broadleaf woodland
   { 1433., 109., 30., 8., 1386., 2000., 8680., 8680., 15., 0.6 }, // 26 subtropical mixed woodland
   { 2326., 109., 30., 8., 1497., 1978., 8020., 8020., 16., 0.6 }, // 27 dry shrub-steppe
   { 2040., 109., 30., 8., 1495., 2003., 8028., 8028., 15., 0.6 }, // 28 C4 grassland
   { 2425., 109., 30., 8., 1488., 1750., 8072., 8072., 15., 0.6 }, // 29 subtropical desert
   { 1730., 109., 30., 8., 1499., 2003., 8006., 8006., 30., 0.5 }, // 30 tropical evergreen broadleaf forest 
   { 1906., 109., 30., 8., 1442., 1909., 8016., 8016., 17., 0.6 }, // 31 tropical deciduous woodland
   { 1906., 109., 30., 8., 1442., 1909., 8016., 8016., 17., 0.6 }, // 32 tropical savanna
   { 2326., 109., 30., 8., 1497., 1978., 8020., 8020., 16., 0.6 }, // 33 tropical shrubland
   { 2040., 109., 30., 8., 1495., 2003., 8028., 8028., 15., 0.6 }, // 34 tropical grassland
   { 2425., 109., 30., 8., 1488., 1750., 8072., 8072., 15., 0.6 }, // 35 tropical desert
   { 1937., 109., 30., 8., 1489., 2120., 8053., 8053., 21., 0.4 }, // 36 cool moist needleleaf forest
   { 2326., 109., 30., 8., 1497., 1978., 8020., 8020., 16., 0.6 }, // 37 unused or Lynx_AgricultureGrazing 
   { 2500., 109., 30., 8., 1500., 2500., 8000., 8000., 15., 0.4 }, // 38 subalpine meadow
   { .00, .00, .00, .00, .0, .000, .0, .0, .0, .0 }, // 39 water and wetlands
   { .00, .00, .00, .00, .0, .000, .0, .0, .0, .0 }, // 40 natural barren 
   { .00, .00, .00, .00, .0, .000, .0, .0, .0, .0 }, // 41 developed
   { 1852., 109., 30., 8., 1470., 1984., 8039., 8039., 30., 0.4 }, // 42 larch forest 
   { 1960., 109., 30., 8., 1488., 2045., 8068., 8068., 23., 0.4 }, // 43 Sitka spruce zone, from 7 maritime needleleaf forest
   { 1960., 109., 30., 8., 1488., 2045., 8068., 8068., 23., 0.4 }, // 44 western hemlock zone, from 7 maritime needleleaf forest
   { 1960., 109., 30., 8., 1488., 2045., 8068., 8068., 23., 0.4 }, // 45 Pacific silver fir zone, from 7 maritime needleleaf forest
   { 1852., 109., 30., 8., 1470., 1984., 8039., 8039., 30., 0.4 }, // 46 mountain hemlock zone, from 6 subalpine forest
   { 1852., 109., 30., 8., 1470., 1984., 8039., 8039., 30., 0.4 }, // 47 subalpine fir zone, from 6 subalpine forest
   { 1634., 109., 30., 8., 1478., 1967., 8026., 8026., 30., 0.4 }, // 48 subalpine parkland zone, from 5 boreal mixed woodland
   { 1937., 109., 30., 8., 1489., 2120., 8053., 8053., 21., 0.4 }, // 49 cool dry needleleaf forest
   { 2326., 109., 30., 8., 1497., 1978., 8020., 8020., 16., 0.6 }, // 50 boreal shrubland from 16 shrub-steppe
   { 2326., 109., 30., 8., 1497., 1978., 8020., 8020., 16., 0.6 }, // 51 semidesert shrubland from 27 dry shrub-steppe
   { 1937., 109., 30., 8., 1489., 2120., 8053., 8053., 21., 0.4 }, // 52 LPPZveg Lodgepole pine zone
   { 1937., 109., 30., 8., 1489., 2120., 8053., 8053., 21., 0.4 }, // 53 JPZveg Jeffrey pine zone
   { 1937., 109., 30., 8., 1489., 2120., 8053., 8053., 21., 0.4 }, // 54 WWPZveg Western white pine zone
   { 1937., 109., 30., 8., 1489., 2120., 8053., 8053., 21., 0.4 }, // 55 DFZ2veg Douglas-fir zone 2
   { 1937., 109., 30., 8., 1489., 2120., 8053., 8053., 21., 0.4 }, // 56 POCZveg Port Orford-cedar zone
   { 1937., 109., 30., 8., 1489., 2120., 8053., 8053., 21., 0.4 }, // 57 GFZveg Grand fir zone
   { 1937., 109., 30., 8., 1489., 2120., 8053., 8053., 21., 0.4 }, // 58 WFZveg White fir zone
   { 1937., 109., 30., 8., 1489., 2120., 8053., 8053., 21., 0.4 }, // 59 SRFZveg Shasta red fir zone
   { 1937., 109., 30., 8., 1489., 2120., 8053., 8053., 21., 0.4 }, // 60 PPZveg Ponderosa pine zone
   }; // end of vveg2fuelGlobal[][10]
   int num_records = sizeof(vveg2fuelGlobal) / (10 * sizeof(float));
   assert(num_records >= (MAX_VTYPE + 1));

   vveg2fuelP = (double *)&(vveg2fuelGlobal[0][0]);

   m_sg1 = *(vveg2fuelP + mc_vtype * 10 + 0);
   m_sg10 = *(vveg2fuelP + mc_vtype * 10 + 1);
   m_sg100 = *(vveg2fuelP + mc_vtype * 10 + 2);
   m_sg1000 = *(vveg2fuelP + mc_vtype * 10 + 3);
   m_sgwood = *(vveg2fuelP + mc_vtype * 10 + 4);
   m_sgherb = *(vveg2fuelP + mc_vtype * 10 + 5);
   m_hl = *(vveg2fuelP + mc_vtype * 10 + 6);
   m_hd = *(vveg2fuelP + mc_vtype * 10 + 7);
   m_mxd_frac = *(vveg2fuelP + mc_vtype * 10 + 8)*0.01;
   m_wndfac = *(vveg2fuelP + mc_vtype * 10 + 9);

} // end of MC_FireModel::fuel_characteristics()


float MCfire::energyReleaseComponent()
//bool MCfire::erc(int iduNdx, int dom, int doy, float * ercP, float * tauP, float *ireP)
{
   // weighting factors of each fuel class 
   float wtotd = m_w1p + m_w10 + m_w100 + m_w1000;
   float wtotl = m_wherbp + m_wwood;
   float f1e, f10e, f100e, f1000e, fherbe, fwoode;
   if (wtotd <= 0.) f1e = f10e = f100e = f1000e = 0.;
   else
   {
      f1e = m_w1p / wtotd;
      f10e = m_w10 / wtotd;
      f100e = m_w100 / wtotd;
      f1000e = m_w1000 / wtotd;
   }
   if (wtotl <= 0.) fherbe = fwoode = 0.;
   else
   {
      fherbe = m_wherbp / wtotl;
      fwoode = 0; // m_wwood_dom[dom] / wtotl;
   }
   
   // weighting factors of dead and live fuels 
   float wtot = wtotl + wtotd;
   float fdeade, flivee;
   if (wtot <= 0.0) fdeade = flivee = 0.;
   else
   {
      fdeade = wtotd / wtot;
      flivee = wtotl / wtot;
   }

   float c_stl, c_std;
   c_stl = c_std = 0.055f;
   // mineral damping coefficent of live and dead fuels    
   float sd, sl;
   sd = sl = 0.01f;
   float c_etasd = (float)(0.174 * pow(sd, -0.19));
   float c_etasl = (float)(0.174 * pow(sl, -0.19));

   // net loadings of dead and live fuels 
   float wdedne, wlivne;
   wdedne = wtotd * (1.0f - c_std);
   wlivne = wtotl * (1.0f - c_stl);

   // dead and live fuel characteristic surface area-to-volume ratios 
   float sgbrde, sgbrle;
   sgbrde = (float)((f1e * m_sg1) + (f10e * m_sg10) + (f100e * m_sg100) + (f1000e * m_sg1000));
   sgbrle = (float)((fherbe * m_sgherb) + (fwoode * m_sgwood));

   float sgbrte = (fdeade * sgbrde) + (flivee * sgbrle); // characteristic surface area-to-volume ratio 
   float betope = (float)(3.348 * pow(sgbrte, -0.8189)); // optimum packing ratio 

   // packing ratio 
   float depth_in_feet = ScienceFcns::m_to_ft(m_fire.fuel.fuel_depth);
   float rhol, rhod, rhobed, rhobar, betbar;
   rhol = rhod = 32.;
   rhobed = (wtot - m_w1000) / depth_in_feet;
   rhobar = ((wtotl * rhol) + (wtotd * rhod)) / wtot;
   betbar = rhobed / rhobar;

   // float propr_rate = betbar / betope; // calculate packing ratio : optimum packing ratio 
   float gmamxe = (float)(pow(sgbrte, 1.5) / (495. + 0.0594 * pow(sgbrte, 1.5))); // maximum reaction velocity 
   float ade = (float)(133. * pow(sgbrte, -0.7913)); // optimum reaction velocity 
   float gmaope = (float)(gmamxe * pow((betbar / betope), ade) * exp(ade * (1.0 - betbar / betope)));

   // weighted moisture content of dead and live fuels 
   float wtmcde = (f1e * m_mc_1hr_frac) + (f10e * m_mc_10hr_frac)
      + (f100e * m_mc_100hr_frac) + (f1000e * m_mc_1000hr_frac);
   float wtmcle = (fherbe * m_mc_grass_frac) + (fwoode * m_mc_tree_frac);

   // moisture damping coefficents of dead and live fuels 
   float dedrte = (float)(wtmcde / m_mxd_frac);
   float livrte = (float)(wtmcle / m_mxl_frac);

   float etamde = (float)(1. - 2.0 * dedrte + 1.5 * pow(dedrte, 2.) - 0.5 * pow(dedrte, 3.));
   if (etamde < 0.) etamde = 0.;
   else if (etamde > 1.) etamde = 1.;

   float etamle = (float)(1. - 2.0 * livrte + 1.5 * pow(livrte, 2.) - 0.5 * pow(livrte, 3.));
   if (etamle < 0.) etamle = 0.;
   else if (etamle > 1.) etamle = 1.;

   m_ire = (float)(gmaope * ((fdeade*wdedne*m_hd*c_etasd*etamde) + (flivee*wlivne*m_hl*c_etasl*etamle))); // reaction intensity
   m_tau = (float)(384. / m_sgbrt); // residence time of flaming front in minutes
   m_erc = 0.04f*m_ire*m_tau; // energy release component
                             
   return(true);
} // end of MC_FireModel::energyReleaseComponent()


bool MCfire::min_mc_day(int iduNdx)
{
/*   int day, month, min_day, min_mo, month_length;
   float min_mc;

   min_mc = 35.;
   min_day = 365;

   // find day for min mc_1000hr 

   for (day = 0; day < 365; day++)
   {
      if (fuel_data.mc_1000hr[day]<min_mc)
      {
         min_mc = fuel_data.mc_1000hr[day];
         min_day = day;
      }

   }

   // find month for min mc_1000hr 
   min_mo = -1;
   day = month = 0;
   while (min_mo<0 && month<12)
   {
      month_length = sciFn.days_per_mo[modelParamsP->southernHemisphereFlag ? (month + 6) % 12 : month];
      day += month_length;
      if (min_day<day) min_mo = month;
      month++;
   }
   assert(min_mo >= 0 && min_mo<12);

   m_min_mc_yr = min_mc;
   m_month_of_min_mc = min_mo;
*/
   return(true);
} // end of min_mc_day()


float MCfire::VPDtoRH(float vpd_Pa, float temp_degC)
{
   double tk = ScienceFcns::degC_to_degK(temp_degC);
   float saturated_pressure_Pa = (float)((temp_degC >= 0.f) ? ScienceFcns::satw(tk) : ScienceFcns::sati(tk));
   float rh = ((saturated_pressure_Pa - vpd_Pa) / saturated_pressure_Pa) * 100.f;
   if (rh > 100.f) rh = 100.f;
   else if (rh < 0) rh = 0.f;
   return(rh);
} // end of VPDtoRH()


int MCfire::MCvtypeOfForestVEGCLASS(int vegclass)
{
   if (vegclass < 2000000) return(0);

   int mc_vtype = 0;
   switch (vegclass)
   {
      case 2052000: mc_vtype = 17; break; //208,115,90)" label = "2052000 PK:GF" / > 17 C3 grassland
      case 2056210: mc_vtype = 6; break; //199,87,6)" label = "2056210 PK:Sm1" / > 6 subalpine forest
      case 2057210: mc_vtype = 6; break; //189,59,177)" label = "2057210 PK:Mm1" / > 6 subalpine forest
      case 2503100: mc_vtype = 9; break; //133,146,183)" label = "2505310 Al:Pc1" / > 9 temperate deciduous broadleaf forest
      case 2504100: mc_vtype = 9; break; //133,146,183)" label = "2505310 Al:Yo" / > 9 temperate deciduous broadleaf forest
      case 2505310: mc_vtype = 9; break; //133,146,183)" label = "2505310 Al:SHo" / > 9 temperate deciduous broadleaf forest
      case 2506110: mc_vtype = 9; break; //133,146,183)" label = "2506110 Al:So1" / > 9 temperate deciduous broadleaf forest
      case 2506210: mc_vtype = 9; break; //133,146,183)" label = "2506210 Al:Sm1" / > 9 temperate deciduous broadleaf forest
      case 2506310: mc_vtype = 9; break; //124,118,99)" label = "2506310 Al:Sc1" / > 9 temperate deciduous broadleaf forest
      case 3005110: mc_vtype = 9; break; //77,233,189)" label = "3005110 Oa:Po1" / > 9 temperate deciduous broadleaf forest
      case 3006110: mc_vtype = 9; break; //68,205,105)" label = "3006110 Oa:So1" / > 9 temperate deciduous broadleaf forest
      case 3007110: mc_vtype = 9; break; //59,177,21)" label = "3007110 Oa:Mo1" / > 9 temperate deciduous broadleaf forest
      case 3008110: mc_vtype = 9; break; //49,149,192)" label = "3008110 Oa:Lo1" / > 9 temperate deciduous broadleaf forest
      case 3503300: mc_vtype = 13; break; //40,121,108)" label = "3503300 DFal:SHtc" / > 13 temperate deciduous broadleaf woodland
      case 3507320: mc_vtype = 9; break; //31,93,24)" label = "3507320 DFal:Mc2" / > 9 temperate deciduous broadleaf forest
      case 3552000: mc_vtype = 17; break; //21,65,195)" label = "3552000 ??:GF" / > 17 C3 grassland
      case 3602000: mc_vtype = 17; break; //12,37,111)" label = "3602000 ??:GF" / > 17 C3 grassland
      case 4002000: mc_vtype = 17; break; //3,9,27)" label = "4002000 DF:GF" / > 17 C3 grassland
      case 4002100: mc_vtype = 17; break; //3,9,27)" label = "4002100 DF:GFp" / > 17 C3 grassland
      case 4004100: mc_vtype = 12; break; //248,236,199)" label = "4004100 DF:Yo" / > 12 temperate needleleaf woodland
      case 4004400: mc_vtype = 12; break; //248,236,199)" label = "4004400 DF:Yop" / > 12 temperate needleleaf woodland
      case 4005110: mc_vtype = 7; break; //239,208,115)" label = "4005110 DF:Po1" / > 7 maritime needleleaf forest
      case 4005210: mc_vtype = 7; break; //230,180,31)" label = "4005210 DF:Pm1 " / > 7 maritime needleleaf forest
      case 4005310: mc_vtype = 7; break; //220,152,202)" label = "4005310 DF:Pc1" / > 7 maritime needleleaf forest
      case 4006110: mc_vtype = 7; break; //211,124,118)" label = "4006110 DF:So1" / > 7 maritime needleleaf forest
      case 4006210: mc_vtype = 7; break; //202,96,34)" label = "4006210 DF:Sm1" / > 7 maritime needleleaf forest
      case 4006220: mc_vtype = 7; break; //192,68,205)" label = "4006220 DF:Sm2" / > 7 maritime needleleaf forest
      case 4006310: mc_vtype = 7; break; //183,40,121)" label = "4006310 DF:Sc1" / > 7 maritime needleleaf forest
      case 4006320: mc_vtype = 7; break; //174,12,37)" label = "4006320 DF:Sc2" / > 7 maritime needleleaf forest
      case 4007110: mc_vtype = 7; break; //164,239,208)" label = "4007110 DF:Mo1" / > 7 maritime needleleaf forest
      case 4007210: mc_vtype = 7; break; //164,239,208)" label = "4007210 DF:Mm1" / > 7 maritime needleleaf forest
      case 4007220: mc_vtype = 7; break; //155,211,124)" label = "4007220 DF:Mm2" / > 7 maritime needleleaf forest
      case 4007320: mc_vtype = 7; break; //146,183,40)" label = "4007320 DF:Mc2" / > 7 maritime needleleaf forest
      case 4007410: mc_vtype = 7; break; //146,183,40)" label = "4007410 DF:M1p" / > 7 maritime needleleaf forest
      case 4008110: mc_vtype = 7; break; //136,155,211)" label = "4008110 DF:Lo1" / > 7 maritime needleleaf forest
      case 4008210: mc_vtype = 7; break; //127,127,127)" label = "4008210 DF:Lm1" / > 7 maritime needleleaf forest
      case 4008220: mc_vtype = 7; break; //118,99,43)" label = "4008220 DF:Lm2" / > 7 maritime needleleaf forest
      case 4008320: mc_vtype = 7; break; //108,71,214)" label = "4008320 DF:Lc2" / > 7 maritime needleleaf forest
      case 4008410: mc_vtype = 7; break; //108,71,214)" label = "4008320 DF:L1p" / > 7 maritime needleleaf forest
      case 4009110: mc_vtype = 7; break; //99,43,130)" label = "4009110 DF:Go1" / > 7 maritime needleleaf forest
      case 4009210: mc_vtype = 7; break; //90,15,46)" label = "4009210 DF:Gm1" / > 7 maritime needleleaf forest
      case 4009220: mc_vtype = 7; break; //80,242,217)" label = "4009220 DF:Gm2" / > 7 maritime needleleaf forest
      case 4009320: mc_vtype = 7; break; //71,214,133)" label = "4009320 DF:Gc2" / > 7 maritime needleleaf forest
      case 4009410: mc_vtype = 7; break; //71,214,133)" label = "4009410 DF:G1p" / > 7 maritime needleleaf forest
      case 4056320: mc_vtype = 7; break; //                      4056320 DFmx:Sc2 > 7 maritime needleleaf forest
      case 4107110: mc_vtype = 7; break; //62,186,49)" label = "4107110 DFWF:Mo1" / > 7 maritime needleleaf forest
      case 4107220: mc_vtype = 7; break; //52,158,220)" label = "4107220 DFWF:Mm2" / > 7 maritime needleleaf forest
      case 4108220: mc_vtype = 7; break; //43,130,136)" label = "4108220 DFWF:Lm2" / > 7 maritime needleleaf forest
      case 4109220: mc_vtype = 7; break; //34,102,52)" label = "4109220 DFWF:Gm2" / > 7 maritime needleleaf forest
      case 4207320: mc_vtype = 7; break; //24,74,223)" label = "4207320 DG:Mc2" / > 7 maritime needleleaf forest
      case 4208320: mc_vtype = 7; break; //15,46,139)" label = "4208320 DG:Lc2" / > 7 maritime needleleaf forest
      case 4209320: mc_vtype = 7; break; //6,18,55)" label = "4209320 DG:Gc2" / > 7 maritime needleleaf forest
      case 4352000: mc_vtype = 17; break; //251,245,227)" label = "4352000 SFDF:GF" / > 17 C3 grassland
      case 4352100: mc_vtype = 17; break; //251,245,227)" label = "4352000 SFDF:GFp" / > 17 C3 grassland
      case 4354100: mc_vtype = 12; break; //242,217,143)" label = "4354100 SFDF:Yo" / > 12 temperate needleleaf woodland
      case 4354400: mc_vtype = 12; break; //242,217,143)" label = "4354400 SFDF:Yop" / > 12 temperate needleleaf woodland
      case 4355210: mc_vtype = 7; break; //233,189,59)" label = "4355210 SFDF:Pm1" / > 7 maritime needleleaf forest
      case 4356210: mc_vtype = 7; break; //223,161,230)" label = "4356210 SFDF:Sm1" / > 7 maritime needleleaf forest
      case 4356220: mc_vtype = 7; break; //223,161,230)" label = "4356220 SFDF:Sm2" / > 7 maritime needleleaf forest
      case 4356320: mc_vtype = 7; break; //214,133,146)" label = "4356320 SFDF:Sc2" / > 7 maritime needleleaf forest
      case 4356410: mc_vtype = 7; break; //214,133,146)" label = "4356410 SFDF:S1p" / > 7 maritime needleleaf forest
      case 4357220: mc_vtype = 7; break; //205,105,62)" label = "4357220 SFDF:Mm2" / > 7 maritime needleleaf forest
      case 4357320: mc_vtype = 7; break; //195,77,233)" label = "4357320 SFDF:Mc2" / > 7 maritime needleleaf forest
      case 4357410: mc_vtype = 7; break; //195,77,233)" label = "4357410 SFDF:M1p" / > 7 maritime needleleaf forest
      case 4358220: mc_vtype = 7; break; //186,49,149)" label = "4358220 SFDF:Lm2" / > 7 maritime needleleaf forest
      case 4358320: mc_vtype = 7; break; //177,21,65)" label = "4358320 SFDF:Lc2" / > 7 maritime needleleaf forest
      case 4358410: mc_vtype = 7; break; //177,21,65)" label = "4358410 SFDF:L1p" / > 7 maritime needleleaf forest
      case 4359220: mc_vtype = 7; break; //167,248,236)" label = "4359220 SFDF:Gm2" / > 7 maritime needleleaf forest
      case 4359320: mc_vtype = 7; break; //158,220,152)" label = "4359320 SFDF:Gc2" / > 7 maritime needleleaf forest
      case 4459320: mc_vtype = 7; break; //149,192,68)" label = "4459320 WH:Gc2" / > 7 maritime needleleaf forest
      case 6052000: mc_vtype = 17; break; //139,164,239)" label = "6052000 MH:GF" / > 17 C3 grassland
      case 6052100: mc_vtype = 17; break; //139,164,239)" label = "6052100 MH:GFp" / > 17 C3 grassland
      case 6054100: mc_vtype = 12; break; //130,136,155)" label = "6054100 MH:Yo" / > 12 temperate needleleaf woodland
      case 6055110: mc_vtype = 7; break; //121,108,71)" label = "6055210 MH:Po1" / > 7 maritime needleleaf forest
      case 6055210: mc_vtype = 7; break; //121,108,71)" label = "6055210 MH:Pm1" / > 7 maritime needleleaf forest
      case 6056110: mc_vtype = 7; break; //111,80,242)" label = "6056220 MH:So1" / > 7 maritime needleleaf forest
      case 6056220: mc_vtype = 7; break; //111,80,242)" label = "6056220 MH:Sm2" / > 7 maritime needleleaf forest
      case 6056320: mc_vtype = 7; break; //102,52,158)" label = "6056320 MH:Sc2" / > 7 maritime needleleaf forest
      case 6057110: mc_vtype = 7; break; //93,24,74)" label = "6057110 MH:Mo1" / > 7 maritime needleleaf forest
      case 6057320: mc_vtype = 7; break; //83,251,245)" label = "6057320 MH:Mc2" / > 7 maritime needleleaf forest
      case 6058110: mc_vtype = 7; break; //74,223,161)" label = "6058110 MH:Lo1" / > 7 maritime needleleaf forest
      case 6058320: mc_vtype = 7; break; //74,223,161)" label = "6058320 MH:Lc2" / > 7 maritime needleleaf forest
      case 6059320: mc_vtype = 7; break; //65,195,77)" label = "6059320 MH:Gc2" / > 7 maritime needleleaf forest
      case 7052000: mc_vtype = 17; break; //55,167,248)" label = "7052000 LP:GF" / > 17 C3 grassland
      case 7052100: mc_vtype = 17; break; //55,167,248)" label = "7052100 LP:GFp" / > 17 C3 grassland
      case 7054100: mc_vtype = 12; break; //55,167,248)" label = "7054100 LP:Yo" / > 12 temperate needleleaf woodland
      case 7055210: mc_vtype = 8; break; //46,139,164)" label = "7055210 LP:Pm1" / > 8 temperate needleleaf forest
      case 7056320: mc_vtype = 8; break; //37,111,80)" label = "7056320 LP:Sc2" / > 8 temperate needleleaf forest
      case 7057320: mc_vtype = 8; break; //127,127,127)" label = "7057320 LP:Mc2" / > 8 temperate needleleaf forest
      default: mc_vtype = 0; break;
   }

   if (mc_vtype <= 0)
   {
      CString msg;
      msg.Format("MCvtypeOfForestVEGCLASS() vegclass = %d, mc_vtype = %d", vegclass, mc_vtype);
      Report::ErrorMsg(msg);
   }

   return(mc_vtype);
} // end of MCvtypeOfForestVEGCLASS()
  
/*
<attr value="1" color="(0,255,255)" label="Corn" /> 28 C4 grassland
<attr value="4" color="(255,128,0)" label="Sorghum" /> C3 grassland
<attr value="5" color="(255,128,0)" label="Soybeans" /> C3 grassland
<attr value="12" color="(0,255,255)" label="Sweet Corn" /> C4 grassland
<attr value="14" color="(255,128,0)" label="Mint" /> C3 grassland
<attr value="21" color="(0,128,128)" label="Barley" /> C3 grassland
<attr value="23" color="(0,128,128)" label="Spring Wheat" /> C3 grassland
<attr value="24" color="(0,128,128)" label="Winter Wheat" /> C3 grassland
<attr value="27" color="(255,128,0)" label="Rye" /> C3 grassland
<attr value="28" color="(255,128,0)" label="Oats" /> C3 grassland
<attr value="30" color="(255,128,0)" label="Speltz" /> C3 grassland
<attr value="31" color="(255,128,0)" label="Canola" /> 17 C3 grassland 
<attr value="32" color="(255,128,0)" label="Flaxseed" /> 17 C3 grassland
<attr value="35" color="(255,128,0)" label="Mustard" /> 17 C3 grassland
<attr value="36" color="(203,254,1)" label="Alfalfa" /> 17 C3 grassland
<attr value="37" color="(203,254,1)" label="Other Hay/Non Alfalfa" /> 17 C3 grassland
<attr value="39" color="(255,128,0)" label="Buckwheat" /> 17 C3 grassland
<attr value="41" color="(255,128,0)" label="Sugarbeets" /> 17 C3 grassland
<attr value="42" color="(255,128,0)" label="Dry Beans" /> 17 C3 grassland
<attr value="43" color="(255,128,0)" label="Potatoes" /> 17 C3 grassland
<attr value="44" color="(255,128,0)" label="Other Crops" /> 17 C3 grassland
<attr value="47" color="(255,128,0)" label="Misc Vegs and Fruits" /> 17 C3 grassland
<attr value="49" color="(255,128,0)" label="Onions" /> 17 C3 grassland
<attr value="52" color="(255,128,0)" label="Lentils" /> 17 C3 grassland
<attr value="53" color="(255,128,0)" label="Peas" /> 17 C3 grassland
<attr value="55" color="(255,128,0)" label="Caneberries" /> 17 C3 grassland
<attr value="56" color="(255,128,0)" label="Hops" /> 17 C3 grassland
<attr value="57" color="(255,128,0)" label="Herbs" /> 17 C3 grassland
<attr value="58" color="(0,85,170)" label="Clover/Wildflowers" /> 17 C3 grassland
<attr value="59" color="(127,15,0)" label="Sod/Grass Seed" /> 17 C3 grassland
<attr value="61" color="(255,128,255)" label="Fallow/Idle Cropland" /> 17 C3 grassland
<attr value="66" color="(255,0,0)" label="Cherries" /> 9 temperate deciduous broadleaf forest
<attr value="68" color="(255,0,0)" label="Apples" /> 9 temperate deciduous broadleaf forest
<attr value="69" color="(255,0,0)" label="Grapes" /> 14 temperate cool mixed woodland
<attr value="70" color="(255,0,0)" label="Christmas Trees" /> 14 temperate cool mixed woodland
<attr value="71" color="(255,0,0)" label="Other Tree Crops" /> 9 temperate deciduous broadleaf forest
<attr value="76" color="(255,0,0)" label="Walnuts" /> 9 temperate deciduous broadleaf forest
<attr value="77" color="(255,0,0)" label="Pears" /> 9 temperate deciduous broadleaf forest
<attr value="111" color="(0,0,127)" label="Open Water" /> 39 water and wetlands
<attr value="112" color="(255,255,255)" label="Perennial Ice/Snow" /> 39 water and wetlands
<attr value="121" color="(221,255,40)" label="Developed Open Space" /> 41 developed
<attr value="122" color="(181,120,120)" label="Developed Low-Intensity" /> 41 developed
<attr value="123" color="(221,90,90)" label="Developed Medium-Intensity" /> 41 developed
<attr value="124" color="(221,60,60)" label="Developed High-Intensity" /> 41 developed
<attr value="125" color="(221,100,100)" label="Developed Undifferentiated" /> 41 developed
<attr value="131" color="(175,175,175)" label="Barren" /> 40 natural barren
<attr value="141" color="(175,200,0)" label="Deciduous Forest" /> 9 temperate deciduous broadleaf forest
<attr value="142" color="(21,225,21)" label="Evergreen Forest" /> 7 maritime needleleaf forest
<attr value="143" color="(101,175,50)" label="Mixed Forest" /> 10 temperate cool mixed forest
<attr value="152" color="(200,180,50)" label="Shrubland" /> 16 shrub-steppe
<attr value="171" color="(225,200,50)" label="Grassland Herbaceous" /> 17 C3 grassland
<attr value="181" color="(203,254,1)" label="Pasture/Hay" /> 17 C3 grassland
<attr value="190" color="(0,127,127)" label="Woody Wetlands" /> 39 water and wetlands
<attr value="195" color="(0,127,127)" label="Herbaceous Wetlands" /> 39 water and wetlands
<attr value="205" color="(255,128,0)" label="Triticale" /> 17 C3 grassland
<attr value="208" color="(255,128,0)" label="Garlic" /> 17 C3 grassland
<attr value="214" color="(255,128,0)" label="Broccoli" /> 17 C3 grassland
<attr value="216" color="(255,128,0)" label="Peppers" /> 17 C3 grassland
<attr value="219" color="(255,128,0)" label="Greens" /> 17 C3 grassland
<attr value="220" color="(255,0,0)" label="Plums" /> 9 temperate deciduous broadleaf forest
<attr value="221" color="(255,128,0)" label="Strawberries" /> 17 C3 grassland
<attr value="222" color="(255,128,0)" label="Squash" /> 17 C3 grassland
<attr value="224" color="(255,128,0)" label="Vetch" /> 17 C3 grassland
<attr value="229" color="(255,128,0)" label="Pumpkins" /> 17 C3 grassland
<attr value="242" color="(255,128,0)" label="Blueberries" /> 17 C3 grassland
<attr value="243" color="(255,128,0)" label="Cabbage" /> 17 C3 grassland
<attr value="244" color="(255,128,0)" label="Cauliflower" /> 17 C3 grassland
<attr value="246" color="(255,128,0)" label="Radishes" /> 17 C3 grassland
<attr value="247" color="(255,128,0)" label="Turnips" /> 17 C3 grassland
<attr value="295" color="(255,255,0)" label="wheat for food" /> 17 C3 grassland
<attr value="296" color="(255,0,255)" label="oranges for food" /> 21 subtropical evergreen broadleaf forest
<attr value="297" color="(255,0,0)" label="tomatoes for food" /> 17 C3 grassland
<attr value="298" color="(255,128,0)" label="beans for food" /> 17 C3 grassland
<attr value="299" color="(255,255,255)" label="crop not yet selected" /> 17 C3 grassland
<attr value="2052000" color="(208,115,90)" label="2052000 PK:GF" /> 17 C3 grassland
<attr value="2056210" color="(199,87,6)" label="2056210 PK:Sm1" /> 6 subalpine forest
<attr value="2057210" color="(189,59,177)" label="2057210 PK:Mm1" /> 6 subalpine forest
<attr value="2506210" color="(133,146,183)" label="2506210 Al:Sm1" /> 9 temperate deciduous broadleaf forest
<attr value="2506310" color="(124,118,99)" label="2506310 Al:Sc1" /> 9 temperate deciduous broadleaf forest
<attr value="3005110" color="(77,233,189)" label="3005110 Oa:Po1" /> 9 temperate deciduous broadleaf forest
<attr value="3006110" color="(68,205,105)" label="3006110 Oa:So1" /> 9 temperate deciduous broadleaf forest
<attr value="3007110" color="(59,177,21)" label="3007110 Oa:Mo1" /> 9 temperate deciduous broadleaf forest
<attr value="3008110" color="(49,149,192)" label="3008110 Oa:Lo1" /> 9 temperate deciduous broadleaf forest
<attr value="3503300" color="(40,121,108)" label="3503300 DFal:SHtc" /> 13 temperate deciduous broadleaf woodland
<attr value="3507320" color="(31,93,24)" label="3507320 DFal:Mc2" /> 9 temperate deciduous broadleaf forest
<attr value="3552000" color="(21,65,195)" label="3552000 ??:GF" /> 17 C3 grassland
<attr value="3602000" color="(12,37,111)" label="3602000 ??:GF" /> 17 C3 grassland
<attr value="4002000" color="(3,9,27)" label="4002000 DF:GF" /> 17 C3 grassland
<attr value="4002100" color="(3,9,27)" label="4002100 DF:GFp" /> 17 C3 grassland
<attr value="4004100" color="(248,236,199)" label="4004100 DF:Yo" /> 12 temperate needleleaf woodland
<attr value="4005110" color="(239,208,115)" label="4005110 DF:Po1" /> 7 maritime needleleaf forest
<attr value="4005210" color="(230,180,31)" label="4005210 DF:Pm1 " /> 7 maritime needleleaf forest
<attr value="4005310" color="(220,152,202)" label="4005310 DF:Pc1" /> 7 maritime needleleaf forest
<attr value="4006110" color="(211,124,118)" label="4006110 DF:So1" /> 7 maritime needleleaf forest
<attr value="4006210" color="(202,96,34)" label="4006210 DF:Sm1" /> 7 maritime needleleaf forest
<attr value="4006220" color="(192,68,205)" label="4006220 DF:Sm2" /> 7 maritime needleleaf forest
<attr value="4006310" color="(183,40,121)" label="4006310 DF:Sc1" /> 7 maritime needleleaf forest
<attr value="4006320" color="(174,12,37)" label="4006320 DF:Sc2" /> 7 maritime needleleaf forest
<attr value="4007210" color="(164,239,208)" label="4007210 DF:Mm1" /> 7 maritime needleleaf forest
<attr value="4007220" color="(155,211,124)" label="4007220 DF:Mm2" /> 7 maritime needleleaf forest
<attr value="4007320" color="(146,183,40)" label="4007320 DF:Mc2" /> 7 maritime needleleaf forest
<attr value="4008110" color="(136,155,211)" label="4008110 DF:Lo1" /> 7 maritime needleleaf forest
<attr value="4008210" color="(127,127,127)" label="4008210 DF:Lm1" /> 7 maritime needleleaf forest
<attr value="4008220" color="(118,99,43)" label="4008220 DF:Lm2" /> 7 maritime needleleaf forest
<attr value="4008320" color="(108,71,214)" label="4008320 DF:Lc2" /> 7 maritime needleleaf forest
<attr value="4009110" color="(99,43,130)" label="4009110 DF:Go1" /> 7 maritime needleleaf forest
<attr value="4009210" color="(90,15,46)" label="4009210 DF:Gm1" /> 7 maritime needleleaf forest
<attr value="4009220" color="(80,242,217)" label="4009220 DF:Gm2" /> 7 maritime needleleaf forest
<attr value="4009320" color="(71,214,133)" label="4009320 DF:Gc2" /> 7 maritime needleleaf forest
<attr value="4107110" color="(62,186,49)" label="4107110 DFWF:Mo1" /> 7 maritime needleleaf forest
<attr value="4107220" color="(52,158,220)" label="4107220 DFWF:Mm2" /> 7 maritime needleleaf forest
<attr value="4108220" color="(43,130,136)" label="4108220 DFWF:Lm2" /> 7 maritime needleleaf forest
<attr value="4109220" color="(34,102,52)" label="4109220 DFWF:Gm2" /> 7 maritime needleleaf forest
<attr value="4207320" color="(24,74,223)" label="4207320 DG:Mc2" /> 7 maritime needleleaf forest
<attr value="4208320" color="(15,46,139)" label="4208320 DG:Lc2" /> 7 maritime needleleaf forest
<attr value="4209320" color="(6,18,55)" label="4209320 DG:Gc2" /> 7 maritime needleleaf forest
<attr value="4352000" color="(251,245,227)" label="4352000 SFDF:GF" /> 17 C3 grassland
<attr value="4354100" color="(242,217,143)" label="4354100 SFDF:Yo" /> 12 temperate needleleaf woodland
<attr value="4355210" color="(233,189,59)" label="4355210 SFDF:Pm1" /> 7 maritime needleleaf forest
<attr value="4356220" color="(223,161,230)" label="4356220 SFDF:Sm2" /> 7 maritime needleleaf forest
<attr value="4356320" color="(214,133,146)" label="4356320 SFDF:Sc2" /> 7 maritime needleleaf forest
<attr value="4357220" color="(205,105,62)" label="4357220 SFDF:Mm2" /> 7 maritime needleleaf forest
<attr value="4357320" color="(195,77,233)" label="4357320 SFDF:Mc2" /> 7 maritime needleleaf forest
<attr value="4358220" color="(186,49,149)" label="4358220 SFDF:Lm2" /> 7 maritime needleleaf forest
<attr value="4358320" color="(177,21,65)" label="4358320 SFDF:Lc2" /> 7 maritime needleleaf forest
<attr value="4359220" color="(167,248,236)" label="4359220 SFDF:Gm2" /> 7 maritime needleleaf forest
<attr value="4359320" color="(158,220,152)" label="4359320 SFDF:Gc2" /> 7 maritime needleleaf forest
<attr value="4459320" color="(149,192,68)" label="4459320 WH:Gc2" /> 7 maritime needleleaf forest
<attr value="6052000" color="(139,164,239)" label="6052000 MH:GF" /> 17 C3 grassland
<attr value="6054100" color="(130,136,155)" label="6054100 MH:Yo" /> 12 temperate needleleaf woodland
<attr value="6055210" color="(121,108,71)" label="6055210 MH:Pm1" /> 7 maritime needleleaf forest
<attr value="6056220" color="(111,80,242)" label="6056220 MH:Sm2" /> 17 C3 grassland
<attr value="6056320" color="(102,52,158)" label="6056320 MH:Sc2" /> 7 maritime needleleaf forest
<attr value="6057110" color="(93,24,74)" label="6057110 MH:Mo1" /> 7 maritime needleleaf forest
<attr value="6057320" color="(83,251,245)" label="6057320 MH:Mc2" /> 7 maritime needleleaf forest
<attr value="6058320" color="(74,223,161)" label="6058320 MH:Lc2" /> 7 maritime needleleaf forest
<attr value="6059320" color="(65,195,77)" label="6059320 MH:Gc2" /> 7 maritime needleleaf forest
<attr value="7054100" color="(55,167,248)" label="7054100 LP:Yo" /> 12 temperate needleleaf woodland
<attr value="7055210" color="(46,139,164)" label="7055210 LP:Pm1" /> 8 temperate needleleaf forest
<attr value="7056320" color="(37,111,80)" label="7056320 LP:Sc2" /> 8 temperate needleleaf forest
<attr value="7057320" color="(127,127,127)" label="7057320 LP:Mc2" /> 8 temperate needleleaf forest
*/


float MCfire::updateKBDI()
{
   /* Initialization for start of year */
   /* Initialization for start of run is done by ProcessModelInit(). */
   float Kprev = m_fire.Kcurr;
   float Kmax = m_fire.Kmax;
   int F1 = m_fire.F1;
   float sum_prev = m_fire.Ksum;

   float Pann = ScienceFcns::mm_to_in(m_fire.ave_ann_ppt_mm);
   float Pcurr = ScienceFcns::mm_to_in(m_wthr.precip_mm);
   float Tcurr = ScienceFcns::CtoF(m_wthr.temp_degC); 

   if (Tcurr < 50.f) Tcurr = 50.f;

   // Calculate drought factor.
   float num = .0486f * Tcurr;
   num = exp(num);
   num = .968f * num;
   num = num - 8.30f;
   num = (800.f - Kprev) * num;

   float dem = -.0441f * Pann;
   dem = exp(dem);
   dem = 10.88f * dem;
   dem = 1 + dem;

   float factor = (num / dem) * .001f;

   // Adjust Pcurr.
   float Padj;
   if (m_wthr.snow_flag) Padj = Pcurr;
   else if (Pcurr > 0.0)
   {
      float sum_curr = sum_prev + Pcurr;

      if (sum_curr <= 0.2f) Padj = 0.;
      else if (F1 == 0)
      {
         Padj = sum_curr - .2f;
         F1 = 1;
      }
      else Padj = Pcurr;
   }
   else
   {
      Padj = 0.;
      sum_prev = 0.;
      F1 = 0;
   }

   sum_prev += Pcurr;

   /* Current day's KDBI */
   float Kcurr = (Kprev - (Padj*100.f)) + factor;
   if (Kcurr < 0.) Kcurr = 0.;

   m_fire.Kcurr = Kcurr;

   if (Kcurr > Kmax) Kmax = Kcurr;

   m_fire.Kprev = Kcurr; // prepare for next pass thru the loop
   m_fire.Kmax = Kmax;
   m_fire.F1 = F1;
   m_fire.Ksum = sum_prev;

   return(Kcurr);
} // end of updateKBDI()

