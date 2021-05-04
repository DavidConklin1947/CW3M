// HBV.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "HBV.h"
#include <Flow\Flow.h>
#include <DATE.HPP>
#include <Maplayer.h>
#include <MAP.h>
#include <UNITCONV.H>
#include <omp.h>
#include <math.h>
#include <UNITCONV.H>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

EnvModel* gpEnvModel = NULL; 
FlowModel* gpFlowModel = NULL;

float HBV::InitHBV_Global(FlowContext *pFlowContext, LPCTSTR inti)
   {
   gpFlowModel = pFlowContext->pFlowModel;
   gpEnvModel = pFlowContext->pEnvContext->pEnvModel;
   m_pIDUlayer = pFlowContext->pFlowModel->m_pIDUlayer;
   m_pReachLayer = pFlowContext->pFlowModel->m_pStreamLayer;
   m_pHRUlayer = pFlowContext->pFlowModel->m_pHRUlayer;
      
   ParamTable *pLULC_Table = pFlowContext->pFlowModel->GetTable( "HBV" );
   if (pLULC_Table != NULL)
   {
      m_col_cfmax = pLULC_Table->GetFieldCol("CFMAX");         // get the location of the parameter in the table    
      m_col_tt = pLULC_Table->GetFieldCol("TT");
      m_col_cfr = pLULC_Table->GetFieldCol("CFR");
      m_col_fc = pLULC_Table->GetFieldCol("FC");
      m_col_beta = pLULC_Table->GetFieldCol("BETA");
      m_col_kperc = pLULC_Table->GetFieldCol("PERC");
      m_col_k0 = pLULC_Table->GetFieldCol("K0");
      m_col_k1 = pLULC_Table->GetFieldCol("K1");
      m_col_uzl = pLULC_Table->GetFieldCol("UZL");
      m_col_k2 = pLULC_Table->GetFieldCol("K2");
   }
   else
   {
      CString msg;
      msg.Format("InitHBV_Global() pLULC_Table = NULL");
      Report::ErrorMsg(msg);
   }

   MapLayer *pIDULayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;
   pIDULayer->CheckCol(m_colSNOWTHRU_D, "SNOWTHRU_D", TYPE_FLOAT, CC_AUTOADD);
   pIDULayer->CheckCol(m_colSNOWTHRU_Y, "SNOWTHRU_Y", TYPE_FLOAT, CC_AUTOADD);
   pIDULayer->CheckCol(m_colSNOWEVAP_D, "SNOWEVAP_D", TYPE_FLOAT, CC_AUTOADD); pIDULayer->SetColDataU(m_colSNOWEVAP_D, 0);
   pIDULayer->CheckCol(m_colSNOWEVAP_Y, "SNOWEVAP_Y", TYPE_FLOAT, CC_AUTOADD); pIDULayer->SetColDataU(m_colSNOWEVAP_Y, 0);
   pIDULayer->CheckCol(m_colSFCF, "SFCF", TYPE_FLOAT, CC_AUTOADD);
   pIDULayer->CheckCol(m_colRAINTHRU_D, "RAINTHRU_D", TYPE_FLOAT, CC_AUTOADD);
   pIDULayer->CheckCol(m_colRAINTHRU_Y, "RAINTHRU_Y", TYPE_FLOAT, CC_AUTOADD);

   pIDULayer->CheckCol(m_colAREA, "AREA", TYPE_FLOAT, CC_MUST_EXIST);
   pIDULayer->CheckCol(m_colSNOWPACK, "SNOWPACK", TYPE_FLOAT, CC_AUTOADD);
   pIDULayer->CheckCol(m_colSNOWCANOPY, "SNOWCANOPY", TYPE_FLOAT, CC_AUTOADD);
   pIDULayer->CheckCol(m_colSM2SOIL, "SM2SOIL", TYPE_FLOAT, CC_AUTOADD);
   pIDULayer->CheckCol(m_colSM2ATM, "SM2ATM", TYPE_FLOAT, CC_AUTOADD);
   pIDULayer->CheckCol(m_colSM2ATM_YR, "SM2ATM_YR", TYPE_FLOAT, CC_AUTOADD);

   m_pHRUlayer->CheckCol(m_colHruSNOW_BOX, "SNOW_BOX", TYPE_FLOAT, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruSM2SOIL, "SM2SOIL", TYPE_FLOAT, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruSM2ATM, "SM2ATM", TYPE_FLOAT, CC_AUTOADD);

   m_pHRUlayer->CheckCol(m_colHruQ0, "Q0", TYPE_FLOAT, CC_AUTOADD);
   m_pHRUlayer->CheckCol(m_colHruQ2, "Q2", TYPE_FLOAT, CC_AUTOADD);

   const char* p = pFlowContext->pFlowModel->GetPath();
   return -1.0f;
   }


#define HC_ECOREGION 9

/*x
float HBV::HBVdailyProcess(FlowContext *pFlowContext)
{
   MapLayer *pIDULayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

   int timing = pFlowContext->timing;
   if (timing & GMT_INIT) return HBV::InitHBV_Global(pFlowContext, NULL);
   //if (timing & GMT_INITRUN) return(HBV_InitRun());
   //if (timing & GMT_START_YEAR) return(HBV_StartYear());
   //if (timing & GMT_START_STEP) return(HBV_StartStep());
   //if (timing & GMT_CATCHMENT) see below
   //if (timing & GMT_END_STEP) return(HBV_EndStep());
   //if (timing & GMT_END_YEAR) { HBV_EndYear();  return(0.f); }
   //if (timing & GMT_ENDRUN) return(HBV_EndRun());

   if (!(timing & GMT_CATCHMENT)) return(0.f);

   if (pFlowContext->dayOfYear == 0) pIDULayer->SetColDataU(m_colSNOWEVAP_Y, VData(0));

   int hru_of_interest = -1;
   int hruCount = pFlowContext->pFlowModel->GetHRUCount();
   ParamTable *pLULC_Table = pFlowContext->pFlowModel->GetTable("HBV");   // store this pointer (and check to make sure it's not NULL)

                        // iterate through hrus/hrulayers 
   bool hbvcalib_error_flag = false;

   for (int h = 0; h < hruCount; h++)
   {
      HRU *pHRU = pFlowContext->pFlowModel->GetHRU(h);

      double hru_snowpack_m3 = 0.;
      double hru_sm2soil_m3 = 0.;
      double hru_sm2atm_m3 = 0.;
      double hru_area = 0.;
      for (int i = 0; i < pHRU->m_polyIndexArray.GetSize(); i++)
      { // Aggregate SnowMod data from the IDU level to the HRU level
         int idu = pHRU->m_polyIndexArray[i];
         float idu_area; m_pIDUlayer->GetData(idu, m_colAREA, idu_area);
         float idu_snowpack; m_pIDUlayer->GetData(idu, m_colSNOWPACK, idu_snowpack);
         float idu_snowcanopy; m_pIDUlayer->GetData(idu, m_colSNOWCANOPY, idu_snowcanopy);
         float idu_sm2soil; m_pIDUlayer->GetData(idu, m_colSM2SOIL, idu_sm2soil);
         float idu_sm2atm; m_pIDUlayer->GetData(idu, m_colSM2ATM, idu_sm2atm);

         hru_area += idu_area;
         hru_snowpack_m3 += ((idu_snowpack + idu_snowcanopy)/ 1000.) * idu_area;
         hru_sm2soil_m3 += (idu_sm2soil / 1000.) * idu_area;
         hru_sm2atm_m3 += (idu_sm2atm / 1000.) * idu_area;
      }
      float hru_snowpack_mmH2O = (float)((hru_snowpack_m3 / hru_area) * 1000.);
      float hru_sm2soil_mmH2O = (float)((hru_sm2soil_m3 / hru_area) * 1000.);
      float hru_sm2atm_mmH2O = (float)((hru_sm2atm_m3 / hru_area) * 1000.);
      if (hru_snowpack_mmH2O < 0. || hru_sm2soil_mmH2O < 0. || hru_sm2atm_mmH2O < 0.)
      {
         CString msg; 
         msg.Format("HBVdailyProcess()1 h = %d, hru_snowpack_mmH2O = %g, hru_sm2soil_mmH2O = %g, hru_sm2atm_mmH2O = %g",
            h, hru_snowpack_mmH2O, hru_sm2soil_mmH2O, hru_sm2atm_mmH2O);
         Report::WarningMsg(msg);
         // if (hru_snowpack_mmH2O < 0.) hru_snowpack_mmH2O = 0.f;
         // if (hru_sm2soil_mmH2O < 0.) hru_sm2soil_mmH2O = 0.f;
         // if (hru_sm2atm_mmH2O < 0.) hru_sm2atm_mmH2O = 0.f;
      }

      m_pHRUlayer->SetDataU(h, m_colHruSNOWPACK, hru_snowpack_mmH2O);
      m_pHRUlayer->SetDataU(h, m_colHruSM2SOIL, hru_sm2soil_mmH2O);
      m_pHRUlayer->SetDataU(h, m_colHruSM2ATM, hru_sm2atm_mmH2O);
      HRULayer * pSnowLayer = pHRU->GetLayer(BOX_SNOWPACK);
      pSnowLayer->m_volumeWater = hru_snowpack_m3;
      pSnowLayer->m_wc = pSnowLayer->m_wDepth = hru_snowpack_mmH2O;
      HRULayer * pMeltLayer = pHRU->GetLayer(BOX_MELT);
      pMeltLayer->m_volumeWater = pMeltLayer->m_wc = pMeltLayer->m_wDepth = 0.f;

      int hruLayerCount = pHRU->GetLayerCount();
      double natural_area_m2 = pHRU->m_HRUeffArea_m2;
      float CFMAX = 0.f, Beta = 0.f, kPerc = 0.0f;
      float k0 = 0.f, k1 = 0.f, UZL = 0.f, k2 = 0.0f;
      float fc = 0.0f;

      VData hbvcalib;
      int ecoregion;
      int colECOREGION = pIDULayer->GetFieldCol("ECOREGION");

      // Get model parameters which may vary by HBV calibration region.
      pFlowContext->pFlowModel->GetHRUData(pHRU, pLULC_Table->m_iduCol, hbvcalib, DAM_FIRST);  // get HRU info
      pFlowContext->pFlowModel->GetHRUData(pHRU, colECOREGION, ecoregion, DAM_FIRST);  // get HRU info
      // if (ecoregion == 8 || ecoregion == 9 || ecoregion == 10) hbvcalib = 28; // Use calibrated HBV parameter values for ClackamasAboveRiverMill for snowy areas
      if (hbvcalib < 0)
      {
         hbvcalib = 0;
         hbvcalib_error_flag = true;
      }
      int hbvcalib_int; hbvcalib.GetAsInt(hbvcalib_int);
      if (pLULC_Table->m_type == DOT_FLOAT) hbvcalib.ChangeType(TYPE_FLOAT);
      bool ok = pLULC_Table->Lookup(hbvcalib, m_col_cfmax, CFMAX);                // degree-day factor (mm oC-1 day-1)
      float melt_mm_per_degC = CFMAX; // 6.5f; // snow melt rate as a function of temperature, value from Anne Nolin
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_fc, fc);                           // maximum soil moisture storage (mm)
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_beta, Beta);                       // parameter that determines the relative contribution to runoff from rain or snowmelt (-)
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_kperc, kPerc);                     // Percolation constant (mm)
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_uzl, UZL);                         // threshold parameter (mm)
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_k0, k0);                           // Recession coefficient (day-1)
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_k1, k1);                           // Recession coefficient (day-1)
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_k2, k2);                           // Recession coefficient (day-1)
      if (!ok)
      {
         CString msg; msg.Format("*** HBVdailyProcess()2 ok = %d, h = %d, hbvcalib = %f Error getting parameter value", ok, h, hbvcalib);
         Report::ErrorMsg(msg);
      }
      // Get the current Day/HRU weather data and calculate terms which depend only on weather data
      float airTemp = -999.f;  pFlowContext->pFlowModel->GetTodaysHRUclimate(CDT_TMEAN, pHRU, airTemp);  // C
      float precip = 0.f; pFlowContext->pFlowModel->GetTodaysHRUclimate(CDT_PRECIP, pHRU, precip);  // mm

      float hruIrrigatedArea = 0.0f;
      for (int idu = 0; idu < pHRU->m_polyIndexArray.GetSize(); idu++)
      {
         float area = 0.0f;
         int irrigated = 0;
         pFlowContext->pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[idu], pFlowContext->pFlowModel->m_colIrrigation, irrigated);
         if (irrigated != 0)
         {
            pFlowContext->pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[idu], pFlowContext->pFlowModel->m_colCatchmentArea, area);
            hruIrrigatedArea += area;
         }
      }
      hruIrrigatedArea = (float)(hruIrrigatedArea * pHRU->m_frc_naturl);

      float irrigatedFracOfArea = 0.0f;
      if (natural_area_m2 > 0.0f) irrigatedFracOfArea = (float)(hruIrrigatedArea / natural_area_m2);

      float nonIrrigatedSoilWater_mm = 0.0f;
      if (natural_area_m2 - hruIrrigatedArea > 0.0f) nonIrrigatedSoilWater_mm = float(pHRU->GetLayer(BOX_NAT_SOIL)->m_volumeWater / (natural_area_m2 - hruIrrigatedArea)*1000.0f); 

      float irrigatedSoilWater_mm = 0.0f;
      if (hruIrrigatedArea > 0.0f) irrigatedSoilWater_mm = float(pHRU->GetLayer(BOX_IRRIG_SOIL)->m_volumeWater / hruIrrigatedArea * 1000.0f); 

      float upperGroundWater_mm = 0.0f;
      if (natural_area_m2 > 0.0f) upperGroundWater_mm = float(pHRU->GetLayer(BOX_FAST_GW)->m_volumeWater / natural_area_m2 * 1000.0f); 


      // gwIrrigated is the proportion of rain/snowmelt that bypasses the irrigated soil bucket, and is added directly to GW
      float gwIrrigated = GroundWaterRechargeFraction(precip, irrigatedSoilWater_mm, fc, Beta);
      // gwNonIrrigated is the proportion of rain/snowmelt that bypasses the non-irrigated soil bucket, and is added directly to GW
      float gwNonIrrigated = GroundWaterRechargeFraction(precip, nonIrrigatedSoilWater_mm, fc, Beta);
      if (gwNonIrrigated > 1.f)
         {
            CString msg; msg.Format("*** HBVdailyProcess()3: gwNonIrrigated = %f, nonIrrigatedSoilWater_mm = %f, fc = %f, Beta = %f", gwNonIrrigated, nonIrrigatedSoilWater_mm, fc, Beta);
            Report::LogMsg(msg);
         }
      float potentialPercolation_mm = Percolation(upperGroundWater_mm, kPerc); // filling the deepest reservoir
      float potentialPercolation_m3 = (float)(potentialPercolation_mm * natural_area_m2 / 1000.f);
/*
      // Mass balance check
      float precip_liters = (float)(precip * natural_area_m2);
      float massBalDiscrepancyFrac = precip != 0.f ? (hruSnowThrufall_liters + hruSnowEvap_liters + hruRainThrufall_liters + hruRainEvap_liters - precip_liters)/precip_liters : 1.e-10f;
      if (h == hru_of_interest)
      if (true || abs(massBalDiscrepancyFrac) > 1.e-5f)
      {
      CString msg;
      msg.Format("HBVdailyProcess()4 h = %d, precip = %f, natural_area_m2 = %f, pHRU->m_HRUtotArea_m2 = %f, pHRU->m_frc_naturl = %f",
      h, precip, natural_area_m2, pHRU->m_HRUtotArea_m2, pHRU->m_frc_naturl);
      Report::LogMsg(msg);
      msg.Format("*** HBVdailyProcess()5 h = %d, massBalDiscrepancyFrac = %f, precip_liters = %f, hruSnowThrufall_liters = %f, hruSnowEvap_liters = %f, hruRainThrufall_liters = %f,"
      " hruRainEvap_liters = %f", h, massBalDiscrepancyFrac, precip_liters, hruSnowThrufall_liters, hruSnowEvap_liters, hruRainThrufall_liters, hruRainEvap_liters);
      Report::LogMsg(msg);
      }
/*
      float rechargeToIrrigatedSoil_m3 = 0.f;
      float rechargeToNonIrrigatedSoil_m3 = 0.f;
      float rechargeToUpperGW_m3 = 0.f;
      float hru_sm2soil = 0.f;  m_pHRUlayer->GetData(h, m_colHruSM2SOIL, hru_sm2soil);
      float rain_and_melt_to_soil_mm = hru_sm2soil;
      float rain_and_melt_to_soil_m3 = (float)(rain_and_melt_to_soil_mm * natural_area_m2 / 1000.f);
      rechargeToIrrigatedSoil_m3 = (irrigatedFracOfArea * rain_and_melt_to_soil_m3) * (1 - gwIrrigated); // Recharge into the irrigated soil bucket, from rain/melt
      rechargeToNonIrrigatedSoil_m3 = ((1.f - irrigatedFracOfArea) * rain_and_melt_to_soil_m3) * (1 - gwNonIrrigated); // Recharge into the nonirrigated soil bucket, from rain/melt
      rechargeToUpperGW_m3 = rain_and_melt_to_soil_m3 - rechargeToIrrigatedSoil_m3 - rechargeToNonIrrigatedSoil_m3;
      if (rechargeToUpperGW_m3 < 0.f) rechargeToUpperGW_m3 = 0.f;

      pHRU->m_rainThrufall_mm = 0.f; // hruRainThrufall_mm; //mm/d
      pHRU->m_rainEvap_mm = 0.f; // (float)(hruRainEvap_liters / natural_area_m2); // mm
      pHRU->m_snowThrufall_mm = 0.f; // hruSnowThrufall_mm; //mm/d
      pHRU->m_snowEvap_mm = 0.f; // (float)(hruSnowEvap_liters / natural_area_m2); // mm SWE
      pHRU->m_melt_mm = 0.f; // hruMelt_mm;
      pHRU->m_refreezing_mm = 0.f;
      pHRU->m_infiltration_mm = rain_and_melt_to_soil_mm;
      pHRU->m_rechargeToIrrigatedSoil_mm = (float)((rechargeToIrrigatedSoil_m3 / natural_area_m2) * 1000.f);
      pHRU->m_rechargeToNonIrrigatedSoil_mm = (float)((rechargeToNonIrrigatedSoil_m3 / natural_area_m2) * 1000.f);
      pHRU->m_rechargeTopSoil_mm = (float)(((rechargeToIrrigatedSoil_m3 + rechargeToNonIrrigatedSoil_m3) / natural_area_m2) * 1000.f);
      pHRU->m_rechargeToUpperGW_mm = (float)((rechargeToUpperGW_m3 / natural_area_m2) * 1000.f);

      //Calculate the source/sink term for each HRULayer
      float q0_mm = 0.0f; float q0_m3 = 0.f;
      float q2_mm = 0.0f; float q2_m3 = 0.f;
      float starting_water_m3 = 0.f;
      float starting_total_flux_m3 = 0.f;
      float ending_water_m3 = 0.f;
      float adjusted_percolation_m3 = 0.f;
      // #pragma omp parallel for 
      for (int l = 0; l < hruLayerCount; l++)
      {
         HRULayer *pHRULayer = pHRU->GetLayer(l);
         starting_water_m3 += (float)pHRULayer->m_volumeWater;
         float waterDepth = float((pHRULayer->m_volumeWater / natural_area_m2)*1000.0f);//mm
         float ss = 0.0f;
         float starting_layer_flux_m3 = pHRULayer->GetFluxValue();
         starting_total_flux_m3 += starting_layer_flux_m3;
         pHRULayer->m_wc = waterDepth;//mm water

         switch (pHRULayer->m_layer)
         {
            case BOX_SNOWPACK: break; // Snow
            case BOX_MELT: break; // Melt

            case BOX_NAT_SOIL: // UnirrigatedSoil
               pHRULayer->m_wc = nonIrrigatedSoilWater_mm; // This stmt is needed because non-irrigated area is not necessarily the same as the area of HRU.
               pHRULayer->AddFluxFromGlobalHandler(rechargeToNonIrrigatedSoil_m3, FL_TOP_SOURCE);     //m3/d
               // EvapTrans will add a transpiration flux out of this layer later in this timestep.
               break;

            case BOX_IRRIG_SOIL: // IrrigatedSoil
               pHRULayer->m_wc = irrigatedSoilWater_mm; // This stmt is needed because irrigated area is not necessarily the same as the area of HRU.
               pHRULayer->AddFluxFromGlobalHandler(rechargeToIrrigatedSoil_m3, FL_TOP_SOURCE);     //m3/d
               // EvapTrans will add a transpiration flux out of this layer later in this timestep.
               break;

            case BOX_FAST_GW: // ss = meltToSoil*(gw) - percolation - q0
                    //                              + outdoor use of urban water + rural residential water - evapotranspiration from outdoor use of municipal and rural residential water
               pHRULayer->AddFluxFromGlobalHandler(-rechargeToUpperGW_m3, FL_SINK); //m3/d   
               q0_mm = Q0(waterDepth, k0, k1, UZL);
               q0_m3 = (float)(q0_mm*natural_area_m2 / 1000.0f);
               adjusted_percolation_m3 = min(potentialPercolation_m3, (float)pHRULayer->m_volumeWater + rechargeToUpperGW_m3 - q0_m3);
               if (adjusted_percolation_m3 > 0.f) pHRULayer->AddFluxFromGlobalHandler(adjusted_percolation_m3, FL_BOTTOM_SINK);     //m3/d 
               pHRULayer->AddFluxFromGlobalHandler(q0_m3, FL_STREAM_SINK);     //m3/d
               if (h == hru_of_interest)
               {
                  CString msg; msg.Format("HBVdailyProcess()6 waterDepth = %f, k0 = %f, k1 = %f, UZL = %f, q0_mm = %f, q0_m3 = %f, adjusted_percolation_m3 = %f",
                     waterDepth, k0, k1, UZL, q0_mm, q0_m3, adjusted_percolation_m3);
                  Report::LogMsg(msg);
               }
               // AltWaterMaster::FateOfUGA_UrbanWater() and FateOfIDU_RuralResidentialWater() will add fluxes in and out of this layer later in this timestep.
               break;

            case BOX_SLOW_GW: 
               q2_mm = Q2(waterDepth, k2);
               q2_m3 = (float)(q2_mm*natural_area_m2 / 1000.f);
               //ss = percolation - q2; //filling the deepest reservoir
               pHRULayer->AddFluxFromGlobalHandler(adjusted_percolation_m3, FL_TOP_SOURCE);     //m3/d 
               pHRULayer->AddFluxFromGlobalHandler(q2_m3, FL_STREAM_SINK);     //m3/d
               if (h == hru_of_interest)
               {
                  CString msg; msg.Format("HBVdailyProcess()7 waterDepth = %f, k2 = %f, q2_mm = %f, q2_m3 = %f", waterDepth, k2, q2_mm, q2_m3);
                  Report::LogMsg(msg);
               }
               if (ecoregion == HC_ECOREGION) pHRU->m_aquifer_recharge_mm = q2_mm;
               break;

            default: break;
         } // end of switch on soil layer

         float trial_ending_layer_m3 = (float)(pHRULayer->m_volumeWater + pHRULayer->GetFluxValue());
         if (trial_ending_layer_m3 < 0.f)
         { 
            if (pHRULayer->m_layer != BOX_FAST_GW || trial_ending_layer_m3 < -1.f)
            { // Report an unexpected or non-trivial negative water volume in the layer.
               CString msg; msg.Format(
                  "*** HBVdailyProcess()8: h = %d, l = %d, pHRULayer->m_layer = %d, trial_ending_layer_m3 = %f, pHRULayer->m_volumeWater = %f, pHRULayer->GetFluxValue() = %f, "
                  "rain_and_melt_to_soil_m3 = %f",
                  h, l, pHRULayer->m_layer, trial_ending_layer_m3, pHRULayer->m_volumeWater, pHRULayer->GetFluxValue(), rain_and_melt_to_soil_m3);
               Report::LogMsg(msg);
               if (pHRULayer->m_layer == 4)
               {
                  msg.Format("HBVdailyProcess()9 rechargeToUpperGW_m3 = %f, potentialPercolation_m3 = %f, adjusted_percolation_m3 = %f, q0_m3 = %f", rechargeToUpperGW_m3, potentialPercolation_m3, adjusted_percolation_m3, q0_m3);
                  Report::LogMsg(msg);
               }
            }
         }

         float ending_layer_m3 = (float)(pHRULayer->m_volumeWater + pHRULayer->GetFluxValue());
         ending_water_m3 += ending_layer_m3;
      } // end of loop thru the soil layers

      pHRU->m_currentRunoff = q0_mm + q2_mm;
      pHRU->DistributeToReaches(q0_m3 + q2_m3);
      if (pHRU->m_HRUeffArea_m2 > 0)
      {
         pHRU->m_Q0_mm = (q0_m3 / pHRU->m_HRUeffArea_m2) * 1000.f;
         pHRU->m_Q2_mm = (q2_m3 / pHRU->m_HRUeffArea_m2) * 1000.f;
         pHRU->m_percolation_mm = (adjusted_percolation_m3 / pHRU->m_HRUeffArea_m2) * 1000.f;
      }
      else pHRU->m_Q0_mm = pHRU->m_Q2_mm = pHRU->m_percolation_mm = 0.f;
      m_pHRUlayer->SetDataU(h, m_colHruQ0, pHRU->m_Q0_mm);
      m_pHRUlayer->SetDataU(h, m_colHruQ2, pHRU->m_Q2_mm);

      // water = starting_water 
      // + snowThrufall + refreezing - hruMelt      layer 0
      //                - refreezing + hruMelt + rainThrufall - rain_and_melt_to_soil     layer 1
      //                                                                              + rechargeToNonIrrigatedSoil    layer 2
      //                                                                                                           + rechargeToIrrigatedSoil  layer 3
      //                                                                                                                                     + rain_and_melt_to_soil_mm*((gwIrrigated*irrigatedFracOfArea) + (gwNonIrrigated*(1-irrigatedFracOfArea))) - percolation - q0    layer 4
      //                                                                                                                                                                                                                                     + percolation      - q2   layer 5
      // = starting_water + snowThrufall + rainThrufall - rain_and_melt_to_soil + rechargeToNonIrrigatedSoil + rechargeToIrrigatedSoil + rain_and_melt_to_soil_mm*((gwIrrigated*irrigatedFracOfArea) + (gwNonIrrigated*(1-irrigatedFracOfArea))) - q0 - q2
      // = starting_water + snowThrufall + rainThrufall - rain_and_melt_to_soil + ((1.f - irrigatedFracOfArea) * rain_and_melt_to_soil_mm) * (1 - gwNonIrrigated) + irrigatedFracOfArea * rain_and_melt_to_soil_mm * (1 - gwIrrigated) + rain_and_melt_to_soil_mm*((gwIrrigated*irrigatedFracOfArea) + (gwNonIrrigated*(1-irrigatedFracOfArea))) - q0 - q2
      // = starting_water + snowThrufall + rainThrufall - q0 - q2
      //       - rain_and_melt_to_soil + ((1.f - irrigatedFracOfArea) * rain_and_melt_to_soil_mm) * (1 - gwNonIrrigated) + irrigatedFracOfArea * rain_and_melt_to_soil_mm * (1 - gwIrrigated) + rain_and_melt_to_soil_mm*((gwIrrigated*irrigatedFracOfArea) + (gwNonIrrigated*(1-irrigatedFracOfArea)))
      // = starting_water + snowThrufall + rainThrufall - q0 - q2
      //       - rain_and_melt_to_soil + rain_and_melt_to_soil_mm * (1.f - irrigatedFracOfArea) * (1 - gwNonIrrigated) +  rain_and_melt_to_soil_mm * irrigatedFracOfArea * (1 - gwIrrigated) + rain_and_melt_to_soil_mm*((gwIrrigated*irrigatedFracOfArea) + (gwNonIrrigated*(1-irrigatedFracOfArea)))
      // = starting_water + snowThrufall + rainThrufall - q0 - q2
      //       + rain_and_melt_to_soil * (-1 + (1.f - irrigatedFracOfArea) * (1 - gwNonIrrigated) +  irrigatedFracOfArea * (1 - gwIrrigated) + (gwIrrigated*irrigatedFracOfArea) + gwNonIrrigated*(1-irrigatedFracOfArea))
      // = starting_water + snowThrufall + rainThrufall - q0 - q2
      //       + rain_and_melt_to_soil * ((1.f - irrigatedFracOfArea) * (1 - gwNonIrrigated) +  irrigatedFracOfArea * (1 - gwIrrigated) + (gwIrrigated*irrigatedFracOfArea) + gwNonIrrigated*(1-irrigatedFracOfArea) - 1)
      // = starting_water + snowThrufall + rainThrufall - q0 - q2
      //       + rain_and_melt_to_soil * ((1.f - irrigatedFracOfArea)  +  irrigatedFracOfArea - 1)
      // = starting_water + snowThrufall + rainThrufall - q0 - q2

      //    float tot_water_m3 = starting_water_m3 + hruRainThrufall_liters / 1000.f + hruSnowThrufall_liters / 1000.f;
      float tot_water_m3 = (float)(starting_water_m3 + hru_sm2soil_m3); // hruRainThrufall_liters / 1000.f + hruSnowThrufall_liters / 1000.f;
      float mass_balance_discrep = (float)((ending_water_m3 + pHRU->m_currentRunoff*natural_area_m2 / 1000.f) - tot_water_m3);
      // if (h == hru_of_interest) // bottom end of HJA
      if (abs(mass_balance_discrep / tot_water_m3) > 1.e-2)
      {
/*
      CString msg; msg.Format("*** HBVdailyProcess()10: h = %d, mass balance discrepancy fraction = %f, tot_water_m3 = %f, ending_water_m3 = %f, pHRU->m_currentRunoff*natural_area_m2/1000 = %f, "
      "q0_m3 = %f, q2_m3 = %f, mass_balance_discrep = %f, hruRainThrufall_liters/1000 = %f, hruSnowThrufall_liters/1000 = %f, natural_area_m2 = %f",
      h, mass_balance_discrep / tot_water_m3, tot_water_m3, ending_water_m3, pHRU->m_currentRunoff*natural_area_m2 / 1000.f, q0_m3, q2_m3, mass_balance_discrep,
      hruRainThrufall_liters / 1000.f, hruSnowThrufall_liters / 1000.f, natural_area_m2);
      Report::LogMsg(msg);
/*
         for (int l = 0; l < hruLayerCount; l++)
         {
            CString msg;
            HRULayer *pHRULayer = pHRU->GetLayer(l);
            if (pHRULayer->m_volumeWater < 0)
            {
               msg.Format("HBVdailyProcess()11 pHRU->hru_id = %d, l = %d, pHRULayer->m_layer = %d, pHRULayer->m_volumeWater = %f, pHRULayer->GetFluxValue() = %f,",
                  pHRU->m_id, l, pHRULayer->m_layer, pHRULayer->m_volumeWater, pHRULayer->GetFluxValue());
               if (pHRULayer->m_volumeWater < -1.) Report::WarningMsg(msg); else Report::LogMsg(msg);
               msg.Format("HBVdailyProcess()12 tot_water_m3 = %f, mass_balance_discrep = %f", tot_water_m3, mass_balance_discrep);
               if (pHRULayer->m_volumeWater < -1.) Report::WarningMsg(msg); else Report::LogMsg(msg);
            }
         }
      } // end of if (abs(mass_balance_discrep / tot_water_m3) > 1.e-2)
   } // end of loop thru HRUs

   if (hbvcalib_error_flag && pFlowContext->dayOfYear == 0)
   {
      CString msg; msg.Format("*** HBVdailyProcess()13 For some HRU, hbvcalib<0"); Report::LogMsg(msg);
   }

   return 0.0f;
} // end of HBVdailyProcess()
x*/


float HBV::HBV_IrrigatedSoil(FlowContext *pFlowContext)
   {
   int hru_of_interest = 1816;  

   if (pFlowContext->timing & 1) // Init()
      return HBV::InitHBV_Global(pFlowContext, NULL);
   // check other timings!!!!

   MapLayer *pIDULayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;
   bool readOnlyFlag = pIDULayer->m_readOnly;
   pIDULayer->m_readOnly = false;
   pIDULayer->SetColData(m_colSNOWTHRU_D, VData(0), true);
   pIDULayer->SetColData(m_colSNOWEVAP_D, VData(0), true);
   pIDULayer->SetColData(m_colSFCF, VData(0), true);
   if (pFlowContext->dayOfYear == 0)
      {
      pIDULayer->SetColData(m_colSNOWTHRU_Y, VData(0), true);
      pIDULayer->SetColData(m_colRAINTHRU_Y, VData(0), true);
      pIDULayer->SetColData(m_colSNOWEVAP_Y, VData(0), true);      //pIDULayer->SetColData(m_colRAINEVAP_Y, VData(0), true);
      }
   pIDULayer->m_readOnly = readOnlyFlag;

   int hruCount = pFlowContext->pFlowModel->GetHRUCount();

   ParamTable *pLULC_Table = pFlowContext->pFlowModel->GetTable("HBV");   // store this pointer (and check to make sure it's not NULL)

   // omp_set_num_threads(8);

   float lh_fus = 334.774f;  // latent heat of fusion for water (kJ/kg), value from Wikipedia

   // Set model parameters which are constant across ecoregions.
   float snowThreshold = -2.0f; // all precip below this is snow, deg C, value from Anne Nolin
   float rainThreshold = 6.0f; // all precip above this is rain, deg C, value from Anne Nolin
   float cwh = 0.1f; // snowpack retains melt until this fraction is exceeded, value from Anne Nolin
   float max_lai_for_interception = 8.f; // LAI at which sfcf and rfcf top out, value from Dave Turner
   float max_sfcf = 0.20f; // max fraction of falling snow which sublimates before becoming part of the snowpack, value from Dave Turner
   float max_rfcf = 0.24f; // max fraction of falling rain which evaporates in the canopy before reaching the ground, value from Dave Turner
   float max_rain_canopy_evap_mm = 0.f; // turn off rain evap per Anne Nolin 6/6/16, was 5.f; // cap on canopy rainfall evap, value from Dave Turner in email to Conklin on 11/18/15 2:22 pm RE: new WW2100 HistoricRef output data
   float k = 0.5f;     // canopy light extinction coefficient
   float swalb = 0.15f; // albedo of canopy, used in snow melt calculation, value from Anne Nolin in email to Conklin on 1/7/16 at 8:54 am Re: "SWALB" parameter in the snow melt calculation in WW2100

   // iterate through hrus/hrulayers 
   bool hbvcalib_error_flag = false;

   // #pragma omp parallel for firstprivate( pFlowContext )
   for (int h = 0; h < hruCount; h++)
      {
      HRU *pHRU = pFlowContext->pFlowModel->GetHRU(h);

      double hru_snowpack_m3 = 0.;
      double hru_sm2soil_m3 = 0.;
      double hru_sm2atm_m3 = 0.;
      double hru_area = 0.;
      for (int i = 0; i < pHRU->m_polyIndexArray.GetSize(); i++)
      { // Aggregate SnowMod data from the IDU level to the HRU level
         int idu = pHRU->m_polyIndexArray[i];
         float idu_area; m_pIDUlayer->GetData(idu, m_colAREA, idu_area);
         float idu_snowpack; m_pIDUlayer->GetData(idu, m_colSNOWPACK, idu_snowpack);
         float idu_snowcanopy; m_pIDUlayer->GetData(idu, m_colSNOWCANOPY, idu_snowcanopy);
         float idu_sm2soil; m_pIDUlayer->GetData(idu, m_colSM2SOIL, idu_sm2soil);
         float idu_sm2atm; m_pIDUlayer->GetData(idu, m_colSM2ATM, idu_sm2atm);

         hru_area += idu_area;
         hru_snowpack_m3 += ((idu_snowpack + idu_snowcanopy) / 1000.) * idu_area;
         hru_sm2soil_m3 += (idu_sm2soil / 1000.) * idu_area;
         hru_sm2atm_m3 += (idu_sm2atm / 1000.) * idu_area;
      }
      float hru_snowpack_mmH2O = (float)((hru_snowpack_m3 / hru_area) * 1000.);
      float hru_sm2soil_mmH2O = (float)((hru_sm2soil_m3  / hru_area) * 1000.);
      float hru_sm2atm_mmH2O = (float)((hru_sm2atm_m3 / hru_area) * 1000.);

      m_pHRUlayer->SetDataU(h, m_colHruSNOW_BOX, hru_snowpack_mmH2O);
      m_pHRUlayer->SetDataU(h, m_colHruSM2SOIL, hru_sm2soil_mmH2O);
      m_pHRUlayer->SetDataU(h, m_colHruSM2ATM, hru_sm2atm_mmH2O);
      int hruLayerCount = pHRU->GetLayerCount();
      double natural_area_m2 = pHRU->m_HRUeffArea_m2;
      float CFMAX = 0.f, CFR = 0.f, Beta = 0.f, kPerc = 0.0f;
      float k0 = 0.f, k1 = 0.f, UZL = 0.f, k2 = 0.0f;
      float fc = 0.0f;
      float tt = 0.f; // snow melt starts at this temperature, deg C

      VData hbvcalib;
      int ecoregion;
      int colECOREGION = pIDULayer->GetFieldCol("ECOREGION");

      // Get model parameters which may vary by HBV calibration region.
      pFlowContext->pFlowModel->GetHRUData(pHRU, pLULC_Table->m_iduCol, hbvcalib, DAM_FIRST);  // get HRU info
      pFlowContext->pFlowModel->GetHRUData(pHRU, colECOREGION, ecoregion, DAM_FIRST);  // get HRU info
	   //if (ecoregion == 8 || ecoregion == 9 || ecoregion == 10) hbvcalib = 28; // Use calibrated HBV parameter values for ClackamasAboveRiverMill for snowy areas
      if (hbvcalib < 0) 
         {
         hbvcalib = 0;
         hbvcalib_error_flag = true;
         }
      int hbvcalib_int; hbvcalib.GetAsInt(hbvcalib_int);
      if (pLULC_Table->m_type == DOT_FLOAT) hbvcalib.ChangeType(TYPE_FLOAT);
      bool ok = pLULC_Table->Lookup(hbvcalib, m_col_cfmax, CFMAX) && CFMAX>0;                // degree-day factor (mm oC-1 day-1)
      float melt_mm_per_degC = CFMAX; // 6.5f; // snow melt rate as a function of temperature, value from Anne Nolin
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_tt, tt);                         // refreezing coefficient
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_cfr, CFR) && CFR>0;                         // refreezing coefficient
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_fc, fc) && fc>0;                           // maximum soil moisture storage (mm)
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_beta, Beta) && Beta>0;                       // parameter that determines the relative contribution to runoff from rain or snowmelt (-)
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_kperc, kPerc) && kPerc>0;                     // Percolation constant (mm)
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_uzl, UZL) && UZL>0;                         // threshold parameter (mm)
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_k0, k0) && k0>0;                           // Recession coefficient (day-1)
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_k1, k1) && k1>0;                           // Recession coefficient (day-1)
      ok &= pLULC_Table->Lookup(hbvcalib, m_col_k2, k2) && k2>0;                           // Recession coefficient (day-1)
      if (!ok)
         {
         CString msg; msg.Format("*** HBV_IrrigatedSoil()1 ok = %d, h = %d, hbvcalib = %f Error getting parameter value", ok, h, hbvcalib);
         Report::ErrorMsg(msg);
         }
      // Get the current Day/HRU weather data and calculate terms which depend only on weather data
      float airTemp = -999.f;  pFlowContext->pFlowModel->GetTodaysHRUclimate(CDT_TMEAN, pHRU, airTemp);  // C
      float convective_melt_mm = (airTemp > tt) ? (CFMAX*(airTemp - tt)) : 0.f;
      float snow_frac_of_total_precip = 0.f;
      if (airTemp >= rainThreshold) snow_frac_of_total_precip = 0.f; // raining
      else if ((airTemp < rainThreshold) && (airTemp >= snowThreshold))
         snow_frac_of_total_precip = (rainThreshold - airTemp) / (rainThreshold - snowThreshold); // snow mixed with rain
      else snow_frac_of_total_precip = 1.f; // just snowing
      float precip = 0.f; pFlowContext->pFlowModel->GetTodaysHRUclimate(CDT_PRECIP, pHRU, precip);  // mm
      float snow_total_mm = precip * snow_frac_of_total_precip;
      float rain_total_mm = precip - snow_total_mm;

      float sw = 0.f; pFlowContext->pFlowModel->GetTodaysHRUclimate(CDT_SOLARRAD, pHRU, sw);    // average daily downward shortwave radiation at the top of the canopy, W/m2
      float rn = sw * 3.6f * 24.f; // (1 day)*W/m2 = (1 day)*(24hrs/day)*(3600s/hr)*(1KJ/1000J)*J/s/m2 = (24*3600/1000)kJ/m2

      // Get state variables from layers, these will be used to calculate some of the rates.
      // Here total snowpack SWE = water_in_snowpack_mm + snow_in_snowpack_mmSWE
      HRULayer *pHRULayer1 = pHRU->GetLayer(BOX_MELT);
      float water_in_snowpack_m3 = (float)pHRULayer1->m_volumeWater;
      float water_in_snowpack_mm = (float)(((water_in_snowpack_m3 / natural_area_m2)*1000.f > 0.0f) ? (water_in_snowpack_m3 / natural_area_m2)*1000.f : 0.0f); // mm
      HRULayer *pHRULayer0 = pHRU->GetLayer(BOX_SNOWPACK);
      float snow_in_snowpack_m3SWE = (float)pHRULayer0->m_volumeWater;
      float snow_in_snowpack_mmSWE = (float)(((snow_in_snowpack_m3SWE/natural_area_m2)*1000.f > 0.0f) ? (snow_in_snowpack_m3SWE / natural_area_m2)*1000.f : 0.0f); 

      float hruIrrigatedArea = 0.0f;
      double hru_wetland_area_m2 = 0.;
      for (int idu = 0; idu < pHRU->m_polyIndexArray.GetSize(); idu++)
      {
         float area = 0.0f;
         pFlowContext->pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[idu], pFlowContext->pFlowModel->m_colCatchmentArea, area);

         int irrigated = 0;
         pFlowContext->pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[idu], pFlowContext->pFlowModel->m_colIrrigation, irrigated);
         if (irrigated != 0) hruIrrigatedArea += area;
         else
         {
            int lulc_a = 0;
            pFlowContext->pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[idu], pFlowContext->pFlowModel->m_colLulcA, lulc_a);
            if (lulc_a == LULCA_WETLAND) hru_wetland_area_m2 += area;
         }
      }
      double hru_non_irrigated_area_m2 = (double)pHRU->m_HRUtotArea_m2 - (double)hruIrrigatedArea;

      hruIrrigatedArea = (float)(hruIrrigatedArea * pHRU->m_frc_naturl);
      hru_non_irrigated_area_m2 *= pHRU->m_frc_naturl;
      hru_wetland_area_m2 *= pHRU->m_frc_naturl;

      float irrigatedFracOfArea = 0.0f;
      if ( natural_area_m2 > 0.0f ) irrigatedFracOfArea = (float)(hruIrrigatedArea / natural_area_m2);

      float nonIrrigatedSoilWater_mm =  0.0f;
      if ( natural_area_m2 - hruIrrigatedArea > 0.0f ) nonIrrigatedSoilWater_mm = float(pHRU->GetLayer(BOX_NAT_SOIL)->m_volumeWater / (natural_area_m2 - hruIrrigatedArea)*1000.0f); // convert from m3 to mm
        
      float irrigatedSoilWater_mm = 0.0f;
      if (hruIrrigatedArea > 0.0f) irrigatedSoilWater_mm = float(pHRU->GetLayer(BOX_IRRIG_SOIL)->m_volumeWater / hruIrrigatedArea*1000.0f); // convert from m3 to mm
        
      float upperGroundWater_mm = 0.0f;
      if (natural_area_m2 > 0.0f) upperGroundWater_mm = float(pHRU->GetLayer(BOX_FAST_GW)->m_volumeWater / natural_area_m2*1000.0f);//convert from m to mm

      //Calculate rates
      double infiltration_from_standing_H2O_m3 = 0.;
      double hru_wetl2q_m3 = 0.;
      if (pHRU->m_standingH2Oflag)
      {
         double hru_standing_H2O_m3 = pHRU->GetLayer(BOX_STANDING_H2O)->m_volumeWater;
         int idus_in_hru = (int)pHRU->m_polyIndexArray.GetSize();
         double wetness_check_m3 = 0.;

         // How much room is there in the topsoil for more water to infiltrate from standing water?
         double room_mm = (double)fc - (double)nonIrrigatedSoilWater_mm; ASSERT(room_mm > 0.);

         for (int i = 0; i < idus_in_hru; i++)
         {
            int idu_ndx = pHRU->m_polyIndexArray[i];
            double wetness_mm = gpFlowModel->Att(idu_ndx, WETNESS);
            if (wetness_mm <= 0.) continue;

            float idu_area_m2 = gpFlowModel->AttFloat(idu_ndx, AREA);
            double idu_standing_H2O_m3 = (wetness_mm / 1000.) * idu_area_m2;
            wetness_check_m3 += idu_standing_H2O_m3;

            double wetl2q_m3 = 0.;

            if (room_mm > wetness_mm)
            { // Drain all the standing water out of this IDU into the soil.
               infiltration_from_standing_H2O_m3 += idu_standing_H2O_m3;
               wetness_mm = 0.;
            } // end of if (room_mm > wetness_mm)
            else
            { // Drain enough standing water out of this IDU to saturate the soil.
               double idu_room_m3 = (room_mm / 1000.) * idu_area_m2;
               infiltration_from_standing_H2O_m3 += idu_room_m3;
               double wetness_m3 = (wetness_mm / 1000.) * idu_area_m2;
               wetness_m3 -= idu_room_m3;
               wetness_mm = (wetness_m3 / idu_area_m2) * 1000.;
            } // end of if (room_mm > wetness_mm) ... else

            // Is there enough standing water in the IDU to overflow back into the reach?
            double wetl_cap_mm = gpFlowModel->Att(idu_ndx, WETL_CAP);
            if (wetness_mm > wetl_cap_mm)
            { // Yes.
               double wetl2q_mm = wetness_mm - wetl_cap_mm;
               wetl2q_m3 = (wetl2q_mm / 1000.) * idu_area_m2;
               wetness_mm -= wetl2q_mm;
            } // end of if (wetness_mm > wetl_cap_mm)

            hru_wetl2q_m3 += wetl2q_m3;
            pIDULayer->SetDataU(idu_ndx, WETNESS, wetness_mm);
         } // end of loop through IDUs

         ASSERT(close_enough(wetness_check_m3, hru_standing_H2O_m3, 1e-5));
      } // end of if (pHRU->m_standingH2Oflag)

      // gwIrrigated is the proportion of rain/snowmelt that bypasses the irrigated soil bucket, and is added directly to GW
      float gwIrrigated = GroundWaterRechargeFraction(precip, irrigatedSoilWater_mm, fc, Beta); 

      // gwNonIrrigated is the proportion of rain/snowmelt/infiltration_from_standing_H2O that bypasses the non-irrigated soil bucket, and is added directly to GW
      double infiltration_from_standing_H2O_mm = hru_non_irrigated_area_m2 > 0 ?
         ((infiltration_from_standing_H2O_m3 / hru_non_irrigated_area_m2) * 1000.) : 0;
      float gwNonIrrigated = GroundWaterRechargeFraction((float)(precip + infiltration_from_standing_H2O_mm), nonIrrigatedSoilWater_mm, fc, Beta);
      if (h == hru_of_interest) 
         if (gwNonIrrigated >= 1.f)
         {
         CString msg; msg.Format("*** HBV_IrrigatedSoil()2: gwNonIrrigated = %f, nonIrrigatedSoilWater_mm = %f, fc = %f, Beta = %f", gwNonIrrigated, nonIrrigatedSoilWater_mm, fc, Beta);
         Report::LogMsg(msg);
         }
      float potentialPercolation_mm = Percolation(upperGroundWater_mm, kPerc); // filling the deepest reservoir
      float potentialPercolation_m3 = (float)(potentialPercolation_mm * natural_area_m2 / 1000.f);

      // Partition precipitation into snow/rain and melt/refreeze.
      float hruRainThrufall_liters = 0.0f; // Accumulates IDU rain volumes to get the value for the HRU.
      float hruSnowThrufall_liters = 0.0f; // Accumulates IDU snow volumes to get the value for the HRU.
      float hruMelt_liters = 0.f; // Accumulates IDU snow melt volumes to get the value for the HRU.
      float hruRainEvap_liters = 0.f; 
      float hruSnowEvap_liters = 0.f; 
      for (int idu = 0; idu <pHRU->m_polyIndexArray.GetSize(); idu++)
         { // Here precip_total = rain_thrufall + rain_evap + snow_thrufall + snow_evap.
         float rain_thrufall_mm = 0.0f;
         float snow_thrufall_mm = 0.0f; // snow water equivalent

         float iduArea = 0.0f; pFlowContext->pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[idu], pFlowContext->pFlowModel->m_colCatchmentArea, iduArea);
         double idu_natural_area = iduArea * pHRU->m_frc_naturl;

         float lai = 0.0f; 
         int lulc_a = 0; pFlowContext->pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[idu], pFlowContext->pFlowModel->m_colLulcA, lulc_a);
         if (lulc_a == 4)
            {
            int pvt = 0; pFlowContext->pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[idu], pFlowContext->pFlowModel->m_colPVT, pvt);
            if (pvt > 0) pFlowContext->pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[idu], pFlowContext->pFlowModel->m_colLai, lai);
            else lai = 1.f;
            }
         else lai = 1.f;

         float frac_of_max_lai = lai / max_lai_for_interception; 
         if (frac_of_max_lai > 1.f) frac_of_max_lai = 1.f; else if (frac_of_max_lai < 0.f) frac_of_max_lai = 0.f;

         float sfcf = 0.f; // snowfall correction factor, the fraction of snow which sublimates instead of forming part of the snowpack
         float snow_evap_mm = 0.f;
         if (snow_total_mm > 0.f)
            {
            sfcf = frac_of_max_lai * max_sfcf; 
            snow_evap_mm = snow_total_mm * sfcf; 
            }
         snow_thrufall_mm = snow_total_mm - snow_evap_mm;  // The part of the total precip which becomes part of the snowpack.

         float rfcf = 0.f; // rainfall correction factor, the fraction of rain intercepted by the canopy and evaporated before reaching the ground
         float rain_evap_mm = 0.f;
         if (rain_total_mm > 0.f)
            {
            rfcf = frac_of_max_lai * max_rfcf; 
            rain_evap_mm = rain_total_mm * rfcf;
            if (rain_evap_mm > max_rain_canopy_evap_mm) rain_evap_mm = max_rain_canopy_evap_mm;
            }
         rain_thrufall_mm = rain_total_mm - rain_evap_mm;

         pIDULayer->m_readOnly = false;
         pIDULayer->SetData(pHRU->m_polyIndexArray[idu], m_colSNOWTHRU_D, snow_thrufall_mm);
         float snow_thrufall_yr_mm = 0.f; pIDULayer->GetData(pHRU->m_polyIndexArray[idu], m_colSNOWTHRU_Y, snow_thrufall_yr_mm);
         pIDULayer->SetData(pHRU->m_polyIndexArray[idu], m_colSNOWTHRU_Y, snow_thrufall_yr_mm + snow_thrufall_mm);
         pIDULayer->SetData(pHRU->m_polyIndexArray[idu], m_colSNOWEVAP_D, snow_evap_mm);
         float snow_evap_yr_mm = 0.f; pIDULayer->GetData(pHRU->m_polyIndexArray[idu], m_colSNOWEVAP_Y, snow_evap_yr_mm);
         pIDULayer->SetData(pHRU->m_polyIndexArray[idu], m_colSNOWEVAP_Y, snow_evap_yr_mm + snow_evap_mm);
         pIDULayer->SetData(pHRU->m_polyIndexArray[idu], m_colSFCF, sfcf);

         pIDULayer->SetData(pHRU->m_polyIndexArray[idu], m_colRAINTHRU_D, rain_thrufall_mm);
         float rain_thrufall_yr_mm = 0.f; pIDULayer->GetData(pHRU->m_polyIndexArray[idu], m_colRAINTHRU_Y, rain_thrufall_yr_mm);
         float rain_evap_yr_mm = 0.f; //pIDULayer->GetData(pHRU->m_polyIndexArray[idu], m_colRAINEVAP_Y, rain_evap_yr_mm);

         float swtrans = (1 - swalb)*rn*exp(lai*-k); //shorwave transmitted through canopy (kJ/m2/d)
         float radiative_melt_mm = (airTemp > tt) ? swtrans / lh_fus : 0.f; // units = (kJ/m2/d) / (kJ/kgH2O) = kgH2O/(m2*d) = mmH2O/d
         float potentialMelt_mm = convective_melt_mm + radiative_melt_mm; 
         float melt_mm = min(potentialMelt_mm, snow_in_snowpack_mmSWE);
         hruMelt_liters = (float)(hruMelt_liters + melt_mm*idu_natural_area);

         hruSnowThrufall_liters = (float)(hruSnowThrufall_liters + snow_thrufall_mm * idu_natural_area);
         hruRainThrufall_liters = (float)(hruRainThrufall_liters + rain_thrufall_mm * idu_natural_area);
         hruSnowEvap_liters = (float)(hruSnowEvap_liters + snow_evap_mm * idu_natural_area);
         hruRainEvap_liters = (float)(hruRainEvap_liters + rain_evap_mm * idu_natural_area);
         } // end polyIndexArray

      // Mass balance check
      float precip_liters = (float)(precip * natural_area_m2);
      float massBalDiscrepancyFrac = precip != 0.f ? (hruSnowThrufall_liters + hruSnowEvap_liters + hruRainThrufall_liters + hruRainEvap_liters - precip_liters)/precip_liters : 1.e-10f;
      // if (h == hru_of_interest) 
         if (abs(massBalDiscrepancyFrac) > 1.e-5f)
         {
         CString msg; 
         msg.Format("HBV_IrrigatedSoil()3 h = %d, precip = %f, natural_area_m2 = %f, pHRU->m_HRUtotArea_m2 = %f, pHRU->m_frc_naturl = %f",
               h, precip, natural_area_m2, pHRU->m_HRUtotArea_m2, pHRU->m_frc_naturl);
         Report::LogMsg(msg);
         msg.Format("*** HBV_IrrigatedSoil()4 h = %d, massBalDiscrepancyFrac = %f, precip_liters = %f, hruSnowThrufall_liters = %f, hruSnowEvap_liters = %f, hruRainThrufall_liters = %f,"
            " hruRainEvap_liters = %f", h, massBalDiscrepancyFrac, precip_liters, hruSnowThrufall_liters, hruSnowEvap_liters, hruRainThrufall_liters, hruRainEvap_liters);
         Report::LogMsg(msg);
         }
      float hruMelt_mm = (float)(hruMelt_liters / natural_area_m2);
      float hruSnowThrufall_mm = (float)(hruSnowThrufall_liters / natural_area_m2);
      float hruRainThrufall_mm = (float)(hruRainThrufall_liters / natural_area_m2);

      // Snowpack at the beginning of the timestep is water_in_snowpack_mm + snow_in_snowpack_mmSWE.
      // Precip is added during the timestep.
      // Some of the precip goes right back to the atmosphere as snow_evap and rain_evap.
      // The rest of the precip is thrufall_snow_mm and thrufall_rain_mm.
      // Convective heat and radiant energy is sufficient to melt a certain amount of snow.  Radiant energy is a function of LAI.
      // The actual melt amount is constrained IDU-by-IDU to the amount of snow available to melt.
      // The actual melt is accumulated over all the IDUs and then however much can't be held in the snowpack itself is deducted from the hru snow amount all at once
      // and routed into the ground, as rain_and_melt_to_soil_mm.
      // This mixing of IDU-level processes dependent on IDU-specific LAI values with HRU-level processes will necessarily result in inaccurate
      // representation of IDU-level snowpack.

      float rain_and_melt_to_soil_mm = 0.f; 
      float rain_and_melt_to_soil_m3 = 0.f;
      float refreezing_mm = 0.f;
      float refreezing_m3 = 0.f;
      float rechargeToIrrigatedSoil_m3 = 0.f;
      float rechargeToNonIrrigatedSoil_m3 = 0.f;
      float rechargeToUpperGW_m3 = 0.f;

      if (airTemp > tt)
         { // Some melt from snowpack.
         float available_to_melt_mm = snow_in_snowpack_mmSWE; 
         if (h == hru_of_interest) 
            if (hruMelt_mm > (available_to_melt_mm + 5.e-4))
            {
            CString msg; msg.Format("*** HBV_IrrigatedSoil()5: h = %d, hruMelt_mm = %f is greater than available_to_melt_mm = %f", h, hruMelt_mm, available_to_melt_mm);
            Report::LogMsg(msg);
            }
         if (hruMelt_mm > available_to_melt_mm) hruMelt_mm = available_to_melt_mm;
         } // end of if (airTemp > tt)
      else hruMelt_mm = hruMelt_liters = 0.f;

      if (airTemp > snowThreshold)
         { // Rain can be mixed with snow at air temps above snowThreshold
         // meltwater and rainwater transfers to soil, when beyond a threshold (CWH, usually 0.1) of the snowpack
         float updated_snow_in_snowpack_mm = hruSnowThrufall_mm + snow_in_snowpack_mmSWE;
         float max_water_in_updated_snowpack_mm = cwh*updated_snow_in_snowpack_mm;
         float updated_water_in_snowpack_mm = hruRainThrufall_mm + water_in_snowpack_mm;
         if (updated_water_in_snowpack_mm > max_water_in_updated_snowpack_mm)
            {
            rain_and_melt_to_soil_mm = updated_water_in_snowpack_mm - max_water_in_updated_snowpack_mm;
            rain_and_melt_to_soil_m3 = (float)(rain_and_melt_to_soil_mm * natural_area_m2 / 1000.f);
            }
         rechargeToIrrigatedSoil_m3 = (irrigatedFracOfArea * rain_and_melt_to_soil_m3) * (1 - gwIrrigated); // Recharge into the irrigated soil bucket, from rain/melt
         rechargeToNonIrrigatedSoil_m3 = ((1.f - irrigatedFracOfArea) * rain_and_melt_to_soil_m3) * (1 - gwNonIrrigated); // Recharge into the nonirrigated soil bucket, from rain/melt
         rechargeToUpperGW_m3 = rain_and_melt_to_soil_m3 - rechargeToIrrigatedSoil_m3 - rechargeToNonIrrigatedSoil_m3;
         if (rechargeToUpperGW_m3 < 0.f) rechargeToUpperGW_m3 = 0.f;
         } // end of if (airTemp >= snowThreshold)
      // else if (airTemp == snowThreshold) no melt, no rain, no recharge, no refreezing
      else if (airTemp < tt)
         { // refreezing of water in the snowpack
         float potentialRefreezing_mm = CFR*CFMAX*(tt - airTemp);
         refreezing_mm = min(potentialRefreezing_mm, water_in_snowpack_mm);
         if (refreezing_mm < 0.f) refreezing_mm = 0.f;
         refreezing_m3 = (float)(refreezing_mm * natural_area_m2 / 1000.f);
         }

      rain_and_melt_to_soil_m3 = (float)(rain_and_melt_to_soil_mm * natural_area_m2 / 1000.f);
      double total_infiltration_m3 = rain_and_melt_to_soil_m3 + infiltration_from_standing_H2O_m3;
      rechargeToIrrigatedSoil_m3 = (irrigatedFracOfArea * rain_and_melt_to_soil_m3) * (1 - gwIrrigated); // Recharge into the irrigated soil bucket, from rain/melt
      rechargeToNonIrrigatedSoil_m3 = (float)(((1.f - irrigatedFracOfArea) * total_infiltration_m3) * (1 - gwNonIrrigated)); // Recharge into the nonirrigated soil bucket, from rain/melt/standing water
      rechargeToUpperGW_m3 = (float)(total_infiltration_m3 - rechargeToIrrigatedSoil_m3 - rechargeToNonIrrigatedSoil_m3);
      if (rechargeToUpperGW_m3 < 0.f) rechargeToUpperGW_m3 = 0.f;

      pHRU->m_rainThrufall_mm = hruRainThrufall_mm; //mm/d
      pHRU->m_rainEvap_mm = (float)(hruRainEvap_liters/natural_area_m2); // mm
      pHRU->m_snowThrufall_mm = hruSnowThrufall_mm; //mm/d
      pHRU->m_snowEvap_mm = (float)(hruSnowEvap_liters/natural_area_m2); // mm SWE
      pHRU->m_melt_mm = hruMelt_mm;
      pHRU->m_refreezing_mm =  refreezing_mm;
      pHRU->m_infiltration_mm = (float)((total_infiltration_m3 / natural_area_m2) * 1000.);
      pHRU->m_infiltrationFromStandingH2O_m3 = infiltration_from_standing_H2O_m3;
      pHRU->m_rechargeToIrrigatedSoil_mm = (float)((rechargeToIrrigatedSoil_m3 / natural_area_m2) * 1000.f);
      pHRU->m_rechargeToNonIrrigatedSoil_mm = (float)((rechargeToNonIrrigatedSoil_m3 / natural_area_m2) * 1000.f);
      pHRU->m_rechargeTopSoil_mm = (float)((((double)rechargeToIrrigatedSoil_m3 + (double)rechargeToNonIrrigatedSoil_m3) / natural_area_m2) * 1000.f);
      pHRU->m_rechargeToUpperGW_mm = (float)((rechargeToUpperGW_m3 / natural_area_m2) * 1000.f);

      //Calculate the source/sink term for each HRULayer
      float q0_mm = 0.0f; float q0_m3 = 0.f; 
      float q2_mm = 0.0f; float q2_m3 = 0.f;
      float starting_water_m3 =  0.f;
      float starting_total_flux_m3 = 0.f;
      float ending_water_m3 = 0.f;
      float adjusted_percolation_m3 = 0.f;
      // #pragma omp parallel for 
      for (int l = 0; l < hruLayerCount; l++)
         {
         HRULayer *pHRULayer = pHRU->GetLayer(l);
         starting_water_m3 += (float)pHRULayer->m_volumeWater; 
         float waterDepth = float((pHRULayer->m_volumeWater / natural_area_m2)*1000.0f);//mm
         float ss = 0.0f;
         float starting_layer_flux_m3 = pHRULayer->GetFluxValue();
         starting_total_flux_m3 += starting_layer_flux_m3;
         pHRULayer->m_wc = waterDepth;//mm water

         switch (pHRULayer->m_layer)
            {
            case 0: // Snow
               //ss = hruSnowThrufall_mm-hruMelt_mm+refreezing;//mm
               pHRULayer->AddFluxFromGlobalHandler(hruSnowThrufall_liters / 1000.0f, FL_TOP_SOURCE);     //m3/d
               pHRULayer->AddFluxFromGlobalHandler(hruMelt_liters / 1000.0f, FL_BOTTOM_SINK);     //m3/d
               pHRULayer->AddFluxFromGlobalHandler(refreezing_m3, FL_BOTTOM_SOURCE);     //m3/d
               break;

            case 1: // Melt or wetland standing water
               pHRULayer->AddFluxFromGlobalHandler(hruRainThrufall_liters / 1000.0f, FL_TOP_SOURCE);     //m3/d
               if (pHRU->m_standingH2Oflag)
               { // This compartment represents wetland standing water.
                  pHRULayer->AddFluxFromGlobalHandler((float)infiltration_from_standing_H2O_m3, FL_BOTTOM_SINK);     //m3/d
                  if (hru_wetl2q_m3 > 0.)
                     pHRULayer->AddFluxFromGlobalHandler((float)hru_wetl2q_m3, FL_STREAM_SINK);
               }
               else
               { // This compartment represents water in the snowpack.
                  pHRULayer->AddFluxFromGlobalHandler(hruMelt_liters / 1000.0f, FL_TOP_SOURCE);     //m3/d
                  pHRULayer->AddFluxFromGlobalHandler(refreezing_m3, FL_TOP_SINK);     //m3/d
               }
               pHRULayer->AddFluxFromGlobalHandler(rain_and_melt_to_soil_m3, FL_BOTTOM_SINK);     //m3/d
               break;

            case 2: // UnirrigatedSoil
               pHRULayer->m_wc = nonIrrigatedSoilWater_mm; // This stmt is needed because non-irrigated area is not necessarily the same as the area of HRU.
               pHRULayer->AddFluxFromGlobalHandler(rechargeToNonIrrigatedSoil_m3, FL_TOP_SOURCE);     //m3/d
               // EvapTrans will add a transpiration flux out of this layer later in this timestep.
               break;

            case 3: // IrrigatedSoil
               pHRULayer->m_wc = irrigatedSoilWater_mm; // This stmt is needed because irrigated area is not necessarily the same as the area of HRU.
               pHRULayer->AddFluxFromGlobalHandler(rechargeToIrrigatedSoil_m3, FL_TOP_SOURCE);     //m3/d
               // EvapTrans will add a transpiration flux out of this layer later in this timestep.
               break;

            case 4: // Upper Groundwater: ss = meltToSoil*(gw) - percolation - q0
               //                              + outdoor use of urban water + rural residential water - evapotranspiration from outdoor use of municipal and rural residential water
               pHRULayer->AddFluxFromGlobalHandler(-rechargeToUpperGW_m3, FL_SINK); //m3/d   
               q0_mm = Q0(waterDepth, k0, k1, UZL);
               q0_m3 = (float)(q0_mm*natural_area_m2 / 1000.0f);
               adjusted_percolation_m3 = min(potentialPercolation_m3, (float)pHRULayer->m_volumeWater + rechargeToUpperGW_m3 - q0_m3);
               if (adjusted_percolation_m3 > 0.f) pHRULayer->AddFluxFromGlobalHandler(adjusted_percolation_m3, FL_BOTTOM_SINK);     //m3/d 
               pHRULayer->AddFluxFromGlobalHandler(q0_m3, FL_STREAM_SINK);     //m3/d
               if (h == hru_of_interest)
               {
                  CString msg; msg.Format("HBV_IrrigatedSoil()6 waterDepth = %f, k0 = %f, k1 = %f, UZL = %f, q0_mm = %f, q0_m3 = %f, adjusted_percolation_m3 = %f", 
                        waterDepth, k0, k1, UZL, q0_mm, q0_m3, adjusted_percolation_m3);
                  Report::LogMsg(msg);
               }
               // AltWaterMaster::FateOfUGA_UrbanWater() and FateOfIDU_RuralResidentialWater() will add fluxes in and out of this layer later in this timestep.
               break;

            case 5: // Lower Groundwater
               q2_mm = Q2(waterDepth, k2);
               q2_m3 = (float)(q2_mm*natural_area_m2 / 1000.f);
               //ss = percolation - q2; //filling the deepest reservoir
               pHRULayer->AddFluxFromGlobalHandler(adjusted_percolation_m3, FL_TOP_SOURCE);     //m3/d 
               pHRULayer->AddFluxFromGlobalHandler(q2_m3, FL_STREAM_SINK);     //m3/d
               if (h == hru_of_interest)
                  {
                  CString msg; msg.Format("HBV_IrrigatedSoil()7 waterDepth = %f, k2 = %f, q2_mm = %f, q2_m3 = %f", waterDepth, k2, q2_mm, q2_m3);
                  Report::LogMsg(msg);
                  }
               if (ecoregion == HC_ECOREGION) pHRU->m_aquifer_recharge_mm = q2_mm;
               break;

            default: break;
            } // end of switch on soil layer

         float trial_ending_layer_m3 = (float)(pHRULayer->m_volumeWater + pHRULayer->GetFluxValue());
         if (trial_ending_layer_m3 < 0.f)
            { // Deal with a negative water volume in the layer.
            if (h == hru_of_interest) // bottom end of HJA
               if (trial_ending_layer_m3 < -1.f)
               {
               CString msg; msg.Format("*** HBV_IrrigatedSoil()8: h = %d, l = %d, pHRULayer->m_layer = %d, trial_ending_layer_m3 = %f, pHRULayer->m_volumeWater = %f, pHRULayer->GetFluxValue() = %f"
                  ", hruSnowThrufall_mm*natural_area_m2 / 1000.0f = %f, hruMelt_mm*natural_area_m2 / 1000.0f = %f, refreezing_mm*natural_area_m2 / 1000.0f = %f, hruRainThrufall_mm*natural_area_m2 / 1000.0f = %f, "
                  "rain_and_melt_to_soil_m3 = %f", 
                  h, l, pHRULayer->m_layer, trial_ending_layer_m3, pHRULayer->m_volumeWater, pHRULayer->GetFluxValue(),
                  hruSnowThrufall_mm*natural_area_m2 / 1000.0f, hruMelt_mm*natural_area_m2 / 1000.0f, refreezing_mm*natural_area_m2 / 1000.0f,
                  hruRainThrufall_mm*natural_area_m2 / 1000.0f, rain_and_melt_to_soil_m3);
               Report::LogMsg(msg);
               if (pHRULayer->m_layer == 4)
                  {
                  msg.Format("rechargeToUpperGW_m3 = %f, potentialPercolation_m3 = %f, adjusted_percolation_m3 = %f, q0_m3 = %f", rechargeToUpperGW_m3, potentialPercolation_m3, adjusted_percolation_m3, q0_m3);
                  Report::LogMsg(msg);
                  }
               }
            // pHRULayer->AddFluxFromGlobalHandler(-trial_ending_layer_m3, FL_USER_DEFINED); // This contributes to a mass balance discrepancy.
            }
         float ending_layer_m3 = (float)(pHRULayer->m_volumeWater + pHRULayer->GetFluxValue());
         ending_water_m3 += ending_layer_m3;
         } // end of loop thru the soil layers

      pHRU->m_snowpackFlag = (snow_in_snowpack_m3SWE + pHRULayer0->GetFluxValue()) > 0.;

      if (h == hru_of_interest) // bottom end of HJA
         if ((snow_in_snowpack_m3SWE + water_in_snowpack_m3 + pHRULayer0->GetFluxValue() + pHRULayer1->GetFluxValue()) < -0.1f)
         {
         CString msg; msg.Format("*** HBV_IrrigatedSoil()9: SWE < -0.1 m3SWE snow_in_snowpack_m3SWE = %f, water_in_snowpack_m3 = %f, pHRULayer0->GetFluxValue() = %f, pHRULayer1->GetFluxValue() = %f"
            ", starting_total_flux_m3 = %f",
            snow_in_snowpack_m3SWE, water_in_snowpack_m3, pHRULayer0->GetFluxValue(), pHRULayer1->GetFluxValue(), starting_total_flux_m3);
         Report::LogMsg(msg);
         }

      pHRU->m_currentRunoff = q0_mm + q2_mm;
      pHRU->DistributeToReaches(q0_m3 + q2_m3);
      if (pHRU->m_HRUeffArea_m2 > 0)
         { 
         pHRU->m_Q0_mm = (q0_m3 / pHRU->m_HRUeffArea_m2) * 1000.f;
         pHRU->m_Q2_mm = (q2_m3 / pHRU->m_HRUeffArea_m2) * 1000.f;
         pHRU->m_percolation_mm = (adjusted_percolation_m3 / pHRU->m_HRUeffArea_m2) * 1000.f;
         }
      else pHRU->m_Q0_mm = pHRU->m_Q2_mm = pHRU->m_percolation_mm = 0.f;
      // water = starting_water 
      // + snowThrufall + refreezing - hruMelt      layer 0
      //                - refreezing + hruMelt + rainThrufall - rain_and_melt_to_soil     layer 1
      //                                                                              + rechargeToNonIrrigatedSoil    layer 2
      //                                                                                                           + rechargeToIrrigatedSoil  layer 3
      //                                                                                                                                     + rain_and_melt_to_soil_mm*((gwIrrigated*irrigatedFracOfArea) + (gwNonIrrigated*(1-irrigatedFracOfArea))) - percolation - q0    layer 4
      //                                                                                                                                                                                                                                     + percolation      - q2   layer 5
      // = starting_water + snowThrufall + rainThrufall - rain_and_melt_to_soil + rechargeToNonIrrigatedSoil + rechargeToIrrigatedSoil + rain_and_melt_to_soil_mm*((gwIrrigated*irrigatedFracOfArea) + (gwNonIrrigated*(1-irrigatedFracOfArea))) - q0 - q2
      // = starting_water + snowThrufall + rainThrufall - rain_and_melt_to_soil + ((1.f - irrigatedFracOfArea) * rain_and_melt_to_soil_mm) * (1 - gwNonIrrigated) + irrigatedFracOfArea * rain_and_melt_to_soil_mm * (1 - gwIrrigated) + rain_and_melt_to_soil_mm*((gwIrrigated*irrigatedFracOfArea) + (gwNonIrrigated*(1-irrigatedFracOfArea))) - q0 - q2
      // = starting_water + snowThrufall + rainThrufall - q0 - q2
      //       - rain_and_melt_to_soil + ((1.f - irrigatedFracOfArea) * rain_and_melt_to_soil_mm) * (1 - gwNonIrrigated) + irrigatedFracOfArea * rain_and_melt_to_soil_mm * (1 - gwIrrigated) + rain_and_melt_to_soil_mm*((gwIrrigated*irrigatedFracOfArea) + (gwNonIrrigated*(1-irrigatedFracOfArea)))
      // = starting_water + snowThrufall + rainThrufall - q0 - q2
      //       - rain_and_melt_to_soil + rain_and_melt_to_soil_mm * (1.f - irrigatedFracOfArea) * (1 - gwNonIrrigated) +  rain_and_melt_to_soil_mm * irrigatedFracOfArea * (1 - gwIrrigated) + rain_and_melt_to_soil_mm*((gwIrrigated*irrigatedFracOfArea) + (gwNonIrrigated*(1-irrigatedFracOfArea)))
      // = starting_water + snowThrufall + rainThrufall - q0 - q2
      //       + rain_and_melt_to_soil * (-1 + (1.f - irrigatedFracOfArea) * (1 - gwNonIrrigated) +  irrigatedFracOfArea * (1 - gwIrrigated) + (gwIrrigated*irrigatedFracOfArea) + gwNonIrrigated*(1-irrigatedFracOfArea))
      // = starting_water + snowThrufall + rainThrufall - q0 - q2
      //       + rain_and_melt_to_soil * ((1.f - irrigatedFracOfArea) * (1 - gwNonIrrigated) +  irrigatedFracOfArea * (1 - gwIrrigated) + (gwIrrigated*irrigatedFracOfArea) + gwNonIrrigated*(1-irrigatedFracOfArea) - 1)
      // = starting_water + snowThrufall + rainThrufall - q0 - q2
      //       + rain_and_melt_to_soil * ((1.f - irrigatedFracOfArea)  +  irrigatedFracOfArea - 1)
      // = starting_water + snowThrufall + rainThrufall - q0 - q2

      float tot_water_m3 = starting_water_m3 + hruRainThrufall_liters / 1000.f + hruSnowThrufall_liters / 1000.f;
      float mass_balance_discrep = (float)((ending_water_m3 + pHRU->m_currentRunoff*natural_area_m2/1000.f) - tot_water_m3);
      if (h == hru_of_interest) // bottom end of HJA
         if (abs(mass_balance_discrep / tot_water_m3) > 1.e-2)
         {
         CString msg; msg.Format("*** HBV_IrrigatedSoil()10: h = %d, mass balance discrepancy fraction = %f, tot_water_m3 = %f, ending_water_m3 = %f, pHRU->m_currentRunoff*natural_area_m2/1000 = %f, "
            "q0_m3 = %f, q2_m3 = %f, mass_balance_discrep = %f, hruRainThrufall_liters/1000 = %f, hruSnowThrufall_liters/1000 = %f, natural_area_m2 = %f", 
            h, mass_balance_discrep / tot_water_m3, tot_water_m3, ending_water_m3, pHRU->m_currentRunoff*natural_area_m2 / 1000.f, q0_m3, q2_m3, mass_balance_discrep,
            hruRainThrufall_liters / 1000.f, hruSnowThrufall_liters / 1000.f, natural_area_m2);
         Report::LogMsg(msg);
         for (int l = 0; l < hruLayerCount; l++)
            {
            HRULayer *pHRULayer = pHRU->GetLayer(l);
            msg.Format("l = %d, pHRULayer->m_layer = %d, pHRULayer->m_volumeWater = %f, pHRULayer->GetFluxValue() = %f, ", l, pHRULayer->m_layer, pHRULayer->m_volumeWater, pHRULayer->GetFluxValue());
            Report::LogMsg(msg);
            }
         msg.Format("hruSnowThrufall_liters / 1000.0f = %f, hruMelt_liters / 1000.0f = %f, refreezing_mm*natural_area_m2 / 1000.0f = %f, hruRainThrufall_liters / 1000.0f = %f, "
            "rain_and_melt_to_soil_m3 = %f",
            hruSnowThrufall_liters / 1000.f, hruMelt_liters / 1000.f, refreezing_mm*natural_area_m2 / 1000.f,
            hruRainThrufall_liters / 1000.f, rain_and_melt_to_soil_m3); 
         Report::LogMsg(msg);
         }
      } // end of loop thru HRUs

   if (hbvcalib_error_flag && pFlowContext->dayOfYear==0)
      {
      CString msg; msg.Format("*** HBV_IrrigatedSoil()11 For some HRU, hbvcalib<0"); Report::LogMsg(msg);
      }

   return 0.0f;
   } // end of HBV_IrrigatedSoil()


float HBV::Snow( float waterDepth, float precip, float temp,float CFMAX, float CFR, float TT  )
   {
   float melt=0.0f;
   if (temp >= TT) //it is melting
      {
      melt=CFMAX*(temp-TT);//mm/d
      if (waterDepth < melt)
         melt=waterDepth;
      }
   else//snowing
      melt=0.0f;
   
   if (melt<0.0f)
      melt=0.0f;

   return melt;
   }


float HBV::GroundWaterRecharge(float precip, float waterDepth, float FC,  float Beta )
   {
   float value=0.0f;
   float lossFraction = (pow((waterDepth/FC),Beta));
   if (lossFraction > 1.0f)
      lossFraction=1.0f;

   if (waterDepth>0)
      value=precip*lossFraction;

   return value;

   }

float HBV::GroundWaterRechargeFraction(float precip, float waterDepth, float FC,  float Beta )
   {
   float value=0.0f;
   float lossFraction = (pow((waterDepth/FC),Beta));
   if (lossFraction > 1.0f)
      lossFraction=1.0f;
   return lossFraction;
   }

float HBV::Percolation( float waterDepth, float kPerc )
   {   
   float value=0.0f;
   if (waterDepth>5.0f)
      value = (waterDepth - 5.f)*kPerc; 
   return(min(value, waterDepth));

   }

float HBV::Q0( float waterDepth, float k0, float k1, float UZL )
   {
   float value=0.0f;
   if (waterDepth>0 && waterDepth > UZL)
      value = (k0*(waterDepth-UZL))+(k1*waterDepth);
   else if (waterDepth>0 && waterDepth <= UZL)
      value=k1*waterDepth;
   return(min(value, waterDepth));
   } // end of Q0()


float HBV::Q2( float waterDepth, float k2 )
   {
   float value=0.0f;
   if (waterDepth>0)
      value=k2*waterDepth;
   return(min(value, waterDepth));
   } // end of Q2


BOOL HBV::CalcDailyUrbanWater(FlowContext *pFlowContext)
	{
	if (pFlowContext->timing & GMT_INIT)
		return HBV::CalcDailyUrbanWaterInit(pFlowContext);

	if (pFlowContext->timing & GMT_REACH)
		HBV::CalcDailyUrbanWaterRun(pFlowContext);
	return TRUE;
	}

BOOL HBV::CalcDailyUrbanWaterInit(FlowContext *pFlowContext)
	{
	MapLayer *pIDULayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

	pIDULayer->CheckCol( m_colH2OResidnt, "H2ORESIDNT", TYPE_FLOAT, CC_MUST_EXIST );
	pIDULayer->CheckCol( m_colH2OIndComm, "H2OINDCOMM", TYPE_FLOAT, CC_MUST_EXIST );
	pIDULayer->CheckCol( m_colDailyUrbanDemand, "URBDMAND_DY", TYPE_FLOAT, CC_AUTOADD );
	pIDULayer->CheckCol(m_colIDUArea, "AREA", TYPE_FLOAT, CC_MUST_EXIST);
	pIDULayer->CheckCol(m_colUGB, "UGB", TYPE_INT, CC_MUST_EXIST);   // urban water 
	
	bool readOnlyFlag = pIDULayer->m_readOnly;
	pIDULayer->m_readOnly = false;
   pIDULayer->SetColData( m_colDailyUrbanDemand, VData(0), true );
	pIDULayer->m_readOnly = readOnlyFlag;

	return TRUE;
	}

BOOL HBV::CalcDailyUrbanWaterRun(FlowContext *pFlowContext)
   {
	MapLayer *pIDULayer = (MapLayer*)pFlowContext->pEnvContext->pMapLayer;

	float ETtG = 1.f; // average summer ET (april-October) for urban lawn grass in current time step
	float ET0G = 1.f; // average summer ET (april-October) for urban lawn grass in time step 0
	int   doy = pFlowContext->dayOfYear;

	for (MapLayer::Iterator idu = pIDULayer->Begin(); idu != pIDULayer->End(); idu++)
		{
		float annualResidentialDemand = 0.f; // ccf/day/acre
		float annualIndCommDemand = 0.f;     // ccf/day/acre
		float iduArea = 0.f; // m2
		int UGBid = 0;

		pIDULayer->GetData(idu, m_colUGB, UGBid);

		if ( UGBid < 1 ) continue; // skip IDU id not UGB

		pIDULayer->GetData(idu, m_colH2OResidnt, annualResidentialDemand);
		pIDULayer->GetData(idu, m_colH2OIndComm, annualIndCommDemand);
		pIDULayer->GetData(idu, m_colIDUArea, iduArea);
		
		// ccf/day/acre
      float dailyResidentialWaterDemand = (float)(1 - 0.2 * (ETtG / ET0G) * (cos(2 * PI * doy / pFlowContext->pEnvContext->daysInCurrentYear))) * annualResidentialDemand;
      float dailyIndCommWaterDemand = annualIndCommDemand / pFlowContext->pEnvContext->daysInCurrentYear;

		// ccf/day/acre
		float dailyUrbanWaterDemand = dailyResidentialWaterDemand + dailyIndCommWaterDemand;
		
		// 1 cf/day  = 3.2774128e-07 m3/sec
		// 1 ccf/day = 3.2774128e-05 m3/sec
		// 1 acre = 4046.86 m2
		// m3/sec = ccf/day/acre * 3.2774128e-05  / 4046.86 * iduArea
		dailyUrbanWaterDemand = dailyUrbanWaterDemand * 3.2774128e-05f / M2_PER_ACRE * iduArea;
		
		bool readOnlyFlag = pIDULayer->m_readOnly;
		pIDULayer->m_readOnly = false;
		pIDULayer->SetData( idu, m_colDailyUrbanDemand, dailyUrbanWaterDemand );
		pIDULayer->m_readOnly = readOnlyFlag;
		}

	return TRUE;
   }
