#include "stdafx.h"

#pragma hdrstop

#include "GlobalMethods.h"
#include "Flow.h"
#include <dataobj.h>
//x #include <idataobj.H>
//x #include <fdataobj.h>

#include <UNITCONV.H>
#include <omp.h>

extern FlowProcess *gpFlow;
extern FlowModel* gpModel;
extern FlowModel* gpFlowModel;

bool ReachRouting::Init(FlowContext* pFlowContext)
{
   m_pHBVtable = gpModel->GetTable("HBV");
   if (m_pHBVtable != NULL)
   {
      gpModel->m_colHbvW2A_SLP = m_pHBVtable->GetFieldCol("W2A_SLP");
      gpModel->m_colHbvW2A_INT = m_pHBVtable->GetFieldCol("W2A_INT");
   }
   else
   {
      CString msg;
      msg.Format("ReachRouting::Init() m_pHBVtable is NULL. This can happen when the HBV.csv file is open in Excel.");
      Report::ErrorMsg(msg);
   }

   return(m_pHBVtable != NULL && gpModel->m_colHbvW2A_SLP > 0 && gpModel->m_colHbvW2A_INT > 0);
} // end of ReachRouting::Init()

bool ReachRouting::Step( FlowContext *pFlowContext )
{    
   bool rtnflag = false;

   // handle NONE, EXTERNAL cases if defined
   rtnflag = GlobalMethod::Step(pFlowContext);    // true means GlobalMethod handled it.
   if (!rtnflag) switch( m_method )
   {
      case GM_EULER:
      case GM_RK4:
      case GM_RKF:
         rtnflag = gpModel->m_reachBlock.Integrate( gpModel->m_currentTime, gpModel->m_currentTime+this->m_reachTimeStep, GetReachDerivatives, pFlowContext );  // NOTE: GetReachDerivatives does not work
         break;
      case GM_KINEMATIC:
         rtnflag = SolveReachKinematicWave( pFlowContext );
         break;
      case GM_NONE:
         rtnflag =  true;
         break;
      default:
         ASSERT( 0 );
         rtnflag = false;
   } // end of switch(m_method)

   // Ensure that every discharge is > 0 and every volume is > 0.
   for (int reach_ndx = 0; reach_ndx < gpModel->m_reachArray.GetSize(); reach_ndx++)
   {
      Reach * pReach = gpModel->m_reachArray[reach_ndx];
      int subnode_count = pReach->GetSubnodeCount();
      for (int subnode_ndx = 0; subnode_ndx < subnode_count; subnode_ndx++)
      {
         ReachSubnode *pNode = pReach->GetReachSubnode(subnode_ndx);
         if (isnan(pNode->m_discharge)) 
         { 
            pNode->m_nanOccurred = true; 
            pNode->m_discharge = 0; 
            pNode->m_dischargeWP = WaterParcel(0, 0);
            pNode->m_dischargeDOY = pFlowContext->dayOfYear;
         }
         if (pNode->m_discharge <= 0)
         {
            double discharge_m3 = pNode->m_discharge * SEC_PER_DAY;
            ASSERT(close_enough(discharge_m3, pNode->m_dischargeWP.m_volume_m3, 0.1, 100.));

            double H2O_temp_degC = DEFAULT_REACH_H2O_TEMP_DEGC;
            if (pNode->m_discharge < 0) H2O_temp_degC = fabs(pNode->m_dischargeWP.WaterTemperature());
            double node_current_added_discharge_cms = NOMINAL_LOW_FLOW_CMS - pNode->m_discharge;
            pNode->m_addedDischarge_cms += node_current_added_discharge_cms;
            pNode->m_discharge = NOMINAL_LOW_FLOW_CMS;
            pNode->m_dischargeWP = WaterParcel(pNode->m_discharge * SEC_PER_DAY, H2O_temp_degC);
            pNode->m_dischargeDOY = pFlowContext->dayOfYear;
         }

         if (isnan(pNode->m_waterParcel.m_volume_m3))
         { 
            pNode->m_nanOccurred = true; 
            pNode->m_waterParcel = WaterParcel(0, 0);
         }
         if (pNode->m_waterParcel.m_volume_m3 <= 0)
         {
            double H2O_temp_degC = DEFAULT_REACH_H2O_TEMP_DEGC;
            if (pNode->m_waterParcel.m_volume_m3 < 0) H2O_temp_degC = fabs(pNode->m_waterParcel.WaterTemperature());
            float depth = GetManningDepthFromQ(pReach, pNode->m_discharge, pReach->m_wdRatio);
            float width = pReach->m_wdRatio * depth;
            float volume = (width * depth * pReach->m_length / subnode_count);

            float node_current_added_volume_m3 = (float)(volume - pNode->m_waterParcel.m_volume_m3);
            WaterParcel node_current_added_volumeWP = WaterParcel(node_current_added_volume_m3, H2O_temp_degC);
            pNode->m_addedVolumeWP.MixIn(node_current_added_volumeWP);
            pNode->m_waterParcel = WaterParcel(volume, H2O_temp_degC);
         }
      } // end of loop thru subnodes
   } // end of logic to ensure that every discharge is > 0 and every volume is > 0

   return(rtnflag);
} // end of ReachRouting::Step()


float max_sw_W_m2[366] = // Represents an estimate of clear sky shortwave in the McKenzie basin.  See DataCW3M/CW3MdigitalHandbook/cloudiness.xlsx
{82.1f,
82.6f,
83.0f,
83.6f,
84.1f,
84.7f,
85.4f,
86.0f,
86.8f,
87.5f,
88.3f,
89.2f,
90.0f,
91.0f,
91.9f,
92.9f,
93.9f,
95.0f,
96.1f,
97.2f,
98.4f,
99.6f,
100.9f,
102.2f,
103.5f,
104.8f,
106.2f,
107.6f,
109.1f,
110.6f,
112.1f,
113.7f,
115.2f,
116.9f,
118.5f,
120.2f,
121.9f,
123.6f,
125.4f,
127.2f,
129.0f,
130.8f,
132.7f,
134.6f,
136.5f,
138.5f,
140.4f,
142.4f,
144.5f,
146.5f,
148.6f,
150.6f,
152.7f,
154.9f,
157.0f,
159.2f,
161.3f,
163.5f,
165.8f,
168.0f,
170.2f,
172.5f,
174.8f,
177.1f,
179.4f,
181.7f,
184.0f,
186.3f,
188.7f,
191.0f,
193.4f,
195.8f,
198.1f,
200.5f,
202.9f,
205.3f,
207.7f,
210.1f,
212.5f,
214.9f,
217.3f,
219.7f,
222.1f,
224.6f,
227.0f,
229.4f,
231.8f,
234.2f,
236.6f,
239.0f,
241.3f,
243.7f,
246.1f,
248.5f,
250.8f,
253.2f,
255.5f,
257.8f,
260.1f,
262.4f,
264.7f,
267.0f,
269.3f,
271.5f,
273.7f,
276.0f,
278.2f,
280.3f,
282.5f,
284.7f,
286.8f,
288.9f,
291.0f,
293.0f,
295.1f,
297.1f,
299.1f,
301.1f,
303.0f,
305.0f,
306.9f,
308.8f,
310.6f,
312.4f,
314.2f,
316.0f,
317.7f,
319.4f,
321.1f,
322.8f,
324.4f,
326.0f,
327.5f,
329.1f,
330.6f,
332.0f,
333.5f,
334.9f,
336.2f,
337.5f,
338.8f,
340.1f,
341.3f,
342.5f,
343.6f,
344.8f,
345.8f,
346.9f,
347.9f,
348.8f,
349.8f,
350.6f,
351.5f,
352.3f,
353.1f,
353.8f,
354.5f,
355.1f,
355.7f,
356.3f,
356.8f,
357.3f,
357.8f,
358.2f,
358.6f,
358.9f,
359.2f,
359.4f,
359.6f,
359.8f,
359.9f,
360.0f,
360.0f,
360.0f,
359.9f,
359.9f,
359.7f,
359.6f,
359.3f,
359.1f,
358.8f,
358.5f,
358.1f,
357.7f,
357.2f,
356.7f,
356.2f,
355.6f,
355.0f,
354.3f,
353.6f,
352.9f,
352.1f,
351.3f,
350.4f,
349.5f,
348.6f,
347.6f,
346.6f,
345.5f,
344.5f,
343.3f,
342.2f,
341.0f,
339.7f,
338.5f,
337.2f,
335.8f,
334.5f,
333.1f,
331.6f,
330.2f,
328.7f,
327.1f,
325.5f,
324.0f,
322.3f,
320.7f,
319.0f,
317.3f,
315.5f,
313.7f,
311.9f,
310.1f,
308.2f,
306.4f,
304.4f,
302.5f,
300.5f,
298.6f,
296.6f,
294.5f,
292.5f,
290.4f,
288.3f,
286.2f,
284.1f,
281.9f,
279.7f,
277.6f,
275.3f,
273.1f,
270.9f,
268.6f,
266.4f,
264.1f,
261.8f,
259.5f,
257.2f,
254.8f,
252.5f,
250.2f,
247.8f,
245.4f,
243.1f,
240.7f,
238.3f,
235.9f,
233.5f,
231.1f,
228.7f,
226.3f,
223.9f,
221.5f,
219.1f,
216.7f,
214.2f,
211.8f,
209.4f,
207.0f,
204.6f,
202.2f,
199.9f,
197.5f,
195.1f,
192.7f,
190.4f,
188.0f,
185.7f,
183.3f,
181.0f,
178.7f,
176.4f,
174.1f,
171.9f,
169.6f,
167.4f,
165.1f,
162.9f,
160.7f,
158.6f,
156.4f,
154.3f,
152.2f,
150.1f,
148.0f,
145.9f,
143.9f,
141.9f,
139.9f,
137.9f,
136.0f,
134.1f,
132.2f,
130.3f,
128.5f,
126.7f,
124.9f,
123.1f,
121.4f,
119.7f,
118.0f,
116.4f,
114.8f,
113.2f,
111.7f,
110.2f,
108.7f,
107.3f,
105.8f,
104.5f,
103.1f,
101.8f,
100.5f,
99.3f,
98.1f,
96.9f,
95.8f,
94.7f,
93.6f,
92.6f,
91.6f,
90.7f,
89.8f,
88.9f,
88.1f,
87.3f,
86.6f,
85.9f,
85.2f,
84.6f,
84.0f,
83.4f,
82.9f,
82.4f,
82.0f,
81.6f,
81.3f,
81.0f,
80.7f,
80.5f,
80.3f,
80.2f,
80.1f,
80.0f,
80.0f,
80.0f,
80.1f,
80.2f,
80.4f,
80.5f,
80.8f,
81.1f,
81.4f,
81.7f,
81.7f
};

double ReachRouting::Cloudiness(double SWunshaded_W_m2, int dayOfYear)
// ??? we need a better estimate of cloudiness than this
{
   double frac_of_max_sw = SWunshaded_W_m2 / max_sw_W_m2[dayOfYear];
   if (frac_of_max_sw > 1.) frac_of_max_sw = 1.;
   if (frac_of_max_sw < 0.) frac_of_max_sw = 0.;
   double cloudiness_tuning_knob = 1.0;
   double cloudiness_frac = cloudiness_tuning_knob * (1. - frac_of_max_sw);
   return(cloudiness_frac);
} // end of Cloudiness()


WaterParcel ReachRouting::ApplyEnergyFluxes(WaterParcel origWP, double H2Oarea_m2, double netSW_W_m2, 
   double H2Otemp_degC, double airTemp_degC, double VTSfrac, double cloudinessFrac, double windspeed_m_sec, double spHumidity, double RHpct, 
   double & rEvap_m3, double & rEvap_kJ, double & rSW_kJ, double & rLW_kj)
{
   WaterParcel rtnWP = origWP;

   double rad_sw_net_W_m2 = netSW_W_m2; 
   double net_lw_out_W_m2 = NetLWout_W_m2(airTemp_degC, cloudinessFrac, H2Otemp_degC, RHpct, VTSfrac);

   // The evap rate is a function of shortwave flux, longwave flux, aerodynamic evaporation, the air temperature,
   // the water temperature, and the relative humidity.  The equation also uses the latent heat of vaporization,
   // the psychrometric constant, and the slope of the saturation vapor v. air temperature curve.  The
   // aerodynamic evaporation is a function of the windspeed.
   double evap_m_s = Reach::Evap_m_s(rtnWP.WaterTemperature(), rad_sw_net_W_m2, net_lw_out_W_m2, airTemp_degC, windspeed_m_sec, spHumidity);
   double evap_m3 = evap_m_s * H2Oarea_m2 * SEC_PER_DAY;
   if (evap_m3 > origWP.m_volume_m3) evap_m3 = origWP.m_volume_m3;
   double evap_kJ = evap_m3 * DENSITY_H2O * Reach::LatentHeatOfVaporization_MJ_kg(H2Otemp_degC) * 1000.;

   double sw_kJ = rad_sw_net_W_m2 * H2Oarea_m2 * SEC_PER_DAY / 1000;
   double lw_kJ = net_lw_out_W_m2 * H2Oarea_m2 * SEC_PER_DAY / 1000;

   // Lose water via evaporation.
   // The amount of water lost is proportional to the surface area of the water.
//       double subreach_precip_vol_m3 = pSubreach->m_subreach_surf_area_m2 * reach_precip_mm / 1000;
//       WaterParcel subreach_precipWP(subreach_precip_vol_m3, temp_air_degC);
//       pSubreach->m_waterParcel.MixIn(subreach_precipWP); // ??? This violates conservation of mass because IDU surface areas overlap stream surface areas.
   if (!rtnWP.Evaporate(evap_m3, evap_kJ))
   { // Evaporation was unsuccessful because it would have resulted in negative thermal energy or negative volume.
      Report::LogMsg("ReachRouting::ApplyEnergyFluxes() Evaporation unsuccessful due to low temperature or low volume.");
   }

   // Gain thermal energy from incoming shortwave radiation and lose it to outgoing longwave radiation.
   double net_rad_kJ = sw_kJ - lw_kJ;
   if (net_rad_kJ < 0.)
   { // Theoretically, the water is losing heat to the atmosphere by radiation.
      double min_skin_temp_degC = 1.;
      double skin_temp_degC = max(airTemp_degC, min_skin_temp_degC);
      if (rtnWP.WaterTemperature() > skin_temp_degC)
      { // The water is warmer than the air, so it is plausible that the water might cool down
         // but not to a temperature less than the skin temperature (the temperature of the air in contact with the liquid water surface).
         double temp_diff = rtnWP.WaterTemperature() - skin_temp_degC;
         double max_heat_loss_kJ = WaterParcel::ThermalEnergy(rtnWP.m_volume_m3, temp_diff);
         double heat_loss_kJ = min(max_heat_loss_kJ, -net_rad_kJ);
         double thermal_energy_kJ = rtnWP.ThermalEnergy() - heat_loss_kJ;
         rtnWP.m_temp_degC = WaterParcel::WaterTemperature(rtnWP.m_volume_m3, thermal_energy_kJ);
         ASSERT(rtnWP.m_temp_degC >= 0);
      }
   }
   else
   { // The water is gaining energy from the atmosphere.
      double thermal_energy_kJ = rtnWP.ThermalEnergy() + net_rad_kJ;
      rtnWP.m_temp_degC = WaterParcel::WaterTemperature(rtnWP.m_volume_m3, thermal_energy_kJ);
      ASSERT(rtnWP.m_temp_degC < 50.);
   }

   ASSERT(rtnWP.m_temp_degC >= 0);
   
   rEvap_m3 = evap_m3;
   rEvap_kJ = evap_kJ;
   rSW_kJ = sw_kJ;
   rLW_kj = lw_kJ;
   return(rtnWP);
} // end of ApplyEnergyFluxes()


bool ReachRouting::SolveReachKinematicWave(FlowContext* pFlowContext)
{
   ASSERT(pFlowContext->svCount <= 0);
   pFlowContext->Reset();
   int reachCount = gpModel->GetReachCount();
   clock_t start = clock();

   int prev_hbvcalib = -1;
   for (int i = 0; i < reachCount; i++)
   {
      Reach* pReach = gpModel->GetReach(i);     // Note: these are guaranteed to be non-phantom

      int hbvcalib; gpModel->m_pStreamLayer->GetData(pReach->m_polyIndex, gpModel->m_colReachHBVCALIB, hbvcalib);
      VData hbvcalibV = hbvcalib;
      hbvcalibV.ChangeType(TYPE_FLOAT);
      float w2a_slp = 0; float& r_w2a_slp = w2a_slp;
      float w2a_int = 0; float& r_w2a_int = w2a_int;
      bool ok = m_pHBVtable->Lookup(hbvcalibV, gpModel->m_colHbvW2A_SLP, r_w2a_slp);
      ok &= m_pHBVtable->Lookup(hbvcalibV, gpModel->m_colHbvW2A_INT, r_w2a_int);
      if (!ok || (w2a_slp == 0 && w2a_int == 0))
      {
         if (pFlowContext->dayOfYear == 0 && pFlowContext->pEnvContext->yearOfRun == 0 && hbvcalib != prev_hbvcalib)
         {
            CString msg;
            msg.Format("SolveReachKinematicWave() For hbvcalib = %d, w2a_slp = %f and w2a_int = %f.  "
               "Will use DEFAULT_SOIL_H2O_TEMP_DEGC = %f for the temperature of runoff water going into the streams.",
               hbvcalib, w2a_slp, w2a_int, DEFAULT_SOIL_H2O_TEMP_DEGC);
            Report::LogMsg(msg);
         }
         w2a_int = DEFAULT_SOIL_H2O_TEMP_DEGC;
         w2a_slp = 0;
      }
      prev_hbvcalib = hbvcalib;

      WaterParcel upstream_inflowWP = GetReachInflowWP(pReach, 0);
      Reach* pUpstreamLeftReach = gpModel->GetReachFromNode(pReach->m_pLeft);
      if (pUpstreamLeftReach != NULL)
      {
         int final_subnode = pUpstreamLeftReach->GetSubnodeCount() - 1;
         ReachSubnode* pNode = (ReachSubnode*)pUpstreamLeftReach->m_subnodeArray[final_subnode];
         int upstream_discharge_doy = pNode->m_dischargeDOY;
         ASSERT(upstream_discharge_doy == pFlowContext->dayOfYear);
         Reach* pUpstreamRightReach = gpModel->GetReachFromNode(pReach->m_pRight);
         if (pUpstreamRightReach != NULL)
         {
            int final_subnode = pUpstreamRightReach->GetSubnodeCount() - 1;
            ReachSubnode* pNode = (ReachSubnode*)pUpstreamRightReach->m_subnodeArray[final_subnode];
            int upstream_discharge_doy = pNode->m_dischargeDOY;
            ASSERT(upstream_discharge_doy == pFlowContext->dayOfYear);
         }
      }

      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachQ_UPSTREAM, upstream_inflowWP.m_volume_m3 / SEC_PER_DAY);

      int num_subreaches = pReach->GetSubnodeCount();
      float temp_air_degC = gpModel->GetTodaysReachTEMP_AIR(pReach);
      float tmax_air_degC = gpModel->GetTodaysReachTMAX_AIR(pReach);
      float reach_precip_mm = gpModel->GetTodaysReachPRECIP(pReach);
      float reach_ws_m_sec = gpModel->GetTodaysReachWINDSPEED(pReach);
      float rad_sw_unshaded_W_m2 = gpModel->GetTodaysReachRAD_SW(pReach); // Shortwave from the climate data takes into account cloudiness but not shading.

      // ??? we need a better estimate of cloudiness than this
      double frac_of_max_sw = rad_sw_unshaded_W_m2 / max_sw_W_m2[pFlowContext->dayOfYear];
      if (frac_of_max_sw > 1.) frac_of_max_sw = 1.;
      if (frac_of_max_sw < 0.) frac_of_max_sw = 0.;
      double cloudiness_tuning_knob = 1.0;
      double cloudiness_frac = cloudiness_tuning_knob * (1. - frac_of_max_sw);

      float sphumidity = gpModel->GetTodaysReachSPHUMIDITY(pReach);
      double z_mean_m; gpModel->m_pStreamLayer->GetData(pReach->m_polyIndex, gpModel->m_colReachZ_MEAN, z_mean_m);
      double ea, vpd;
      double rh_pct = 100 * ETEquation::CalculateRelHumidity(sphumidity, temp_air_degC, tmax_air_degC, (float)z_mean_m, ea, vpd);
      double reach_net_lw_kJ = 0;
      double total_surface_in_subreaches_m2 = 0;
      double lw_x_surface_accum_W = 0;
      double sw_x_surface_accum_W = 0;
      pReach->m_reach_evap_m3 = 0.;
      pReach->m_reach_evap_kJ = 0.;
      double width_x_length_accum = 0.;
      double depth_x_length_accum = 0.;
      double manning_depth_x_length_accum = 0;
      double volume_accum_m3 = 0.;
      double temp_h2o_air_slope = 0;
      double temp_h2o_air_intercept = 0;
      double rad_sw_net_W_m2 = pFlowContext->pFlowModel->GetReachShade_a_lator_W_m2(pReach, rad_sw_unshaded_W_m2);
      for (int l = 0; l < pReach->GetSubnodeCount(); l++)
      {
         pFlowContext->pReach = pReach;
         ReachSubnode* pSubreach = pReach->GetReachSubnode(l);
         ASSERT(pSubreach->m_dischargeDOY == (pFlowContext->dayOfYear - 1));

         double orig_m_volume_m3 = pSubreach->m_waterParcel.m_volume_m3;
         double vts_frac = pReach->GetSubreachViewToSky_frac(l);

         double lateralInflow_m3 = GetLateralInflow(pReach);
         ASSERT(!isnan(lateralInflow_m3));
         WaterParcel lateralInflowWP(0, 0);
         if (lateralInflow_m3 > 0)
         { // The net lateral flow is into the reach. Assign a temperature to it.
            lateralInflowWP.m_volume_m3 = lateralInflow_m3;
            double temp_h2o_degC = (w2a_slp == 0) ? DEFAULT_SOIL_H2O_TEMP_DEGC
               : w2a_slp * max(0, temp_air_degC) + w2a_int;
            temp_h2o_degC = max(temp_h2o_degC, DEFAULT_MIN_SKIN_TEMP_DEGC);
            lateralInflowWP.m_temp_degC = temp_h2o_degC;
            PutLateralWP(pReach, l, lateralInflowWP, 0);
         }
         else PutLateralWP(pReach, l, lateralInflow_m3); // The net lateral inflow is <= 0 representing water leaving the reach.

         double evap_m3, evap_kJ, sw_kJ, lw_kJ;
         WaterParcel adjustedWP = ApplyEnergyFluxes(pSubreach->m_waterParcel, pSubreach->m_subreach_surf_area_m2, rad_sw_net_W_m2, 
            pSubreach->m_waterParcel.WaterTemperature(), temp_air_degC, vts_frac, cloudiness_frac, reach_ws_m_sec, sphumidity, rh_pct,
            evap_m3, evap_kJ, sw_kJ, lw_kJ);
         ASSERT(adjustedWP.WaterTemperature() < 50.);
         pSubreach->m_waterParcel = adjustedWP;
         pSubreach->m_evap_m3 = evap_m3;
         pSubreach->m_evap_kJ = evap_kJ;
         pSubreach->m_sw_kJ = sw_kJ;
         pSubreach->m_lw_kJ = lw_kJ;

         sw_x_surface_accum_W += (pSubreach->m_sw_kJ * 1000.) / SEC_PER_DAY;
         lw_x_surface_accum_W += (pSubreach->m_lw_kJ * 1000.) / SEC_PER_DAY;
         reach_net_lw_kJ += pSubreach->m_lw_kJ;

         total_surface_in_subreaches_m2 += pSubreach->m_subreach_surf_area_m2;
         pReach->m_reach_evap_m3 += pSubreach->m_evap_m3;
         pReach->m_reach_evap_kJ += pSubreach->m_evap_kJ;

         WaterParcel outflowWP(0, 0);
         outflowWP = ApplyReachOutflowWP(pReach, l, pFlowContext->timeStep); // s/b DailySubreachFlow(pReach, l, subreach_evapWP, subreach_precipWP);
         ASSERT(pSubreach->m_dischargeDOY == (pFlowContext->dayOfYear - 1));
         pSubreach->m_dischargeDOY = pFlowContext->dayOfYear;

         ASSERT(pSubreach->m_waterParcel.m_volume_m3 > 0);

         pSubreach->m_discharge = outflowWP.m_volume_m3 / SEC_PER_DAY;  // convert units to m3/s
         pSubreach->m_manning_depth_m = GetManningDepthFromQ(pReach, pSubreach->m_discharge, pReach->m_wdRatio);
         if (pFlowContext->m_SALmode)
         {
            bool sal_reach; gpModel->m_pStreamLayer->GetData(pReach->m_polyIndex, gpModel->m_colReachSAL_REACH, sal_reach);
            if (!sal_reach) pSubreach->SetSubreachGeometry(pSubreach->m_waterParcel.m_volume_m3, pReach->m_wdRatio);
            else
            {
               double width_given_m = 0.; gpModel->m_pStreamLayer->GetData(pReach->m_polyIndex, gpModel->m_colReachWIDTHGIVEN, width_given_m);
               double dummy = 0;
               pSubreach->SetSubreachGeometry(pSubreach->m_waterParcel.m_volume_m3, dummy, width_given_m);
            }
         }
         else pSubreach->SetSubreachGeometry(pSubreach->m_waterParcel.m_volume_m3, pReach->m_wdRatio);
         ASSERT(pSubreach->m_subreach_width_m > 0.);
         width_x_length_accum += pSubreach->m_subreach_width_m * pSubreach->m_subreach_length_m;
         depth_x_length_accum += pSubreach->m_subreach_depth_m * pSubreach->m_subreach_length_m;
         manning_depth_x_length_accum += pSubreach->m_manning_depth_m * pSubreach->m_subreach_length_m;
         volume_accum_m3 += pSubreach->m_waterParcel.m_volume_m3;

         if (pSubreach->m_discharge < 0 || pSubreach->m_discharge > 1.e10f || pSubreach->m_discharge != pSubreach->m_discharge)
         {
            CString msg;
            msg.Format("*** SolveReachKinematicWave(): COMID = %d, i = %d, l = %d, m_discharge = %f",
               pReach->m_reachID, i, l, pSubreach->m_discharge);
            Report::LogMsg(msg);
            msg.Format("Setting pSubreach->m_discharge to %f cms and continuing.", NOMINAL_LOW_FLOW_CMS); Report::LogMsg(msg);
            pSubreach->m_discharge = NOMINAL_LOW_FLOW_CMS;
         }
      } // end of loop through subreaches
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachAREA_H2O, total_surface_in_subreaches_m2);
      pReach->m_reach_volume_m3 = volume_accum_m3;
      pReach->m_rad_lw_kJ = reach_net_lw_kJ;
      double reach_lw_W_m2 = lw_x_surface_accum_W / total_surface_in_subreaches_m2;
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachRAD_LW_OUT, reach_lw_W_m2);
      double reach_sw_W_m2 = sw_x_surface_accum_W / total_surface_in_subreaches_m2;
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachRAD_SW_IN, reach_sw_W_m2);
      double reach_kJ = reach_sw_W_m2 * SEC_PER_DAY * total_surface_in_subreaches_m2 / 1000.;
      double reach_kcal = 0.2390057361 * reach_kJ;
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachKCAL_REACH, reach_kcal);

      double reach_width_calc_m = width_x_length_accum / pReach->m_length;
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachWIDTH_CALC, reach_width_calc_m);
      double reach_depth_m = depth_x_length_accum / pReach->m_length;
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachDEPTH, reach_depth_m);
      double reach_manning_depth_m = manning_depth_x_length_accum / pReach->m_length;
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachDEPTHMANNG, reach_manning_depth_m);
      double turnover = (pReach->GetReachDischargeWP()).m_volume_m3 / volume_accum_m3;
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachTURNOVER, turnover);

      // Wetlands stuff
      // Has the flow gone over the stream banks?
      double q_cap_cms = pReach->Att(Q_CAP);
      double q_cms = (pReach->GetReachDischargeWP().m_volume_m3 / SEC_PER_DAY);
      if (q_cap_cms > 0 && q_cms > q_cap_cms)
      { // Flow has gone over the banks.
         double qspill_frc = 0.; gpModel->m_pStreamLayer->GetData(pReach->m_polyIndex, gpModel->m_colReachQSPILL_FRC, qspill_frc);
         double q2wetl_cms = q_cms * qspill_frc;
         gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachQ2WETL, q2wetl_cms);
      }


   } // end of loop through reaches

   clock_t finish = clock();
   double duration = (float)(finish - start) / CLOCKS_PER_SEC;
   gpModel->m_reachFluxFnRunTime += (float)duration;

   return true;
} // end of SolveReachKinematicWave()


double ReachRouting::NetLWout_W_m2(double tempAir_degC, double cL, double tempH2O_degC, double RH_pct, double theta_vts)
// Eq. numbers are from sec 2.2 (p.51) of Boyd & Kasper HeatSource 7.0 document
// In Boyd & Kasper, positive fluxes are into the water, and negative fluxes are out of the water.
// On return from this function (and elsewhere in CW3M), positive longwave is out of the water, 
// while positive shortwave is into the water.
   {
   // 2-74 atmospheric longwave radiation flux attenuated in water column, W/m2
   double s_b = 0.567e-7; // W/(m2*K4) Stefan-Boltzmann constant
   double t_a_degK = tempAir_degC + 273.2;
   double bb_atm_W_m2 = 0.96 * s_b * pow(t_a_degK, 4); // black body radiation from the atmosphere
   double e_s_mbar = 6.1275 * exp((17.27 * tempAir_degC) / (237.3 + tempAir_degC)); // 2-79 saturation vapor pressure
   double e_a_mbar = (RH_pct / 100.) * e_s_mbar; // 2-78 vapor pressure
   double exponent = 1. /  7.;
   double power_term = pow(((0.1 * e_a_mbar) / t_a_degK), exponent);
   double eps_atm = 1.72 * power_term * (1 + 0.22 * cL * cL); // Boyd & Kasper have 0.22 + cL^2, but Dingman p. 196 eq. 5-39 has 0.22 * cL^2, which makes more sense
   double phi_a_lw = 0.96 * eps_atm * bb_atm_W_m2; // This is what is in Boyd & Kasper.
//x   phi_a_lw = 213. + 5.5 * tempAir_degC; // This is from fig. 4.10 p. 52 of Monteith & Unsworth Principles of Environmental Physics 2nd Ed.

   // 2-75 land cover longwave radiation flux attenuated in water column, W/m2
   double phi_lc_lw = 0.96 * (1 - theta_vts) * bb_atm_W_m2; 

   // 2-76 longwave radiation flux emitted from water column - back radiation, W/m2
   double t_w_degK = tempH2O_degC + 273.2;
   double bb_h2o_W_m2 = 0.96 * s_b * pow(t_w_degK, 4);
   double phi_s_lw = -bb_h2o_W_m2; 

   // 2-73 longwave radiation flux attenuated in water
   double phi_longwave = phi_a_lw + phi_lc_lw + phi_s_lw; // 2-73

   return(-phi_longwave); 
   } // end of NetLWout_W_m2()


double ReachRouting::GetLateralInflow( Reach *pReach )
   {
   float inflow = 0;
   inflow = -pReach->GetFluxValue();
   inflow /= pReach->GetSubnodeCount();  // m3/day

   if (inflow != inflow)
      {
      CString msg; msg.Format("*** GetLateralInflow(): inflow = %f, pReach->GetFluxValue() = %f, pReach->GetSubnodeCount() = %d",
         inflow, pReach->GetFluxValue(), pReach->GetSubnodeCount());
      Report::LogMsg(msg);
      msg.Format("Setting inflow to 0.0 cms and continuing"); Report::LogMsg(msg);
      inflow = 0.f;
      }
   for (int i = 0; i < pReach->GetSubnodeCount(); i++)
   {
      ReachSubnode *pSubnode = pReach->GetReachSubnode(i);
      pSubnode->m_lateralInflow = inflow;
   }

   return inflow;
   } // end of GetLateralInflow()


void ReachRouting::PutLateralWP(Reach * pReach, int subreachNdx, double daily_net_subreach_lateral_flow_m3) // Allocates runoff and withdrawals to a subreach
{
   ASSERT(!isnan(daily_net_subreach_lateral_flow_m3));

   ReachSubnode* pNode = pReach->GetReachSubnode(subreachNdx);

   bool flow_is_into_reach = daily_net_subreach_lateral_flow_m3 >= 0;
   double subreach_volume_in_m3 = flow_is_into_reach ? daily_net_subreach_lateral_flow_m3 : 0;
   double subreach_volume_out_m3 = flow_is_into_reach ? 0 : fabs(daily_net_subreach_lateral_flow_m3);

   pNode->m_runoffWP = WaterParcel(subreach_volume_in_m3, DEFAULT_SOIL_H2O_TEMP_DEGC); // water parcel running off the soil into the subreach
   pNode->m_withdrawalWP = WaterParcel(subreach_volume_out_m3, pNode->m_waterParcel.WaterTemperature()); // water parcel being withdrawn from the subreach (e.g. for irrigation)
} // end of PutLateralWP(pReach, subreachNdx, daily_net_subreach_lateral_flow_m3)


void ReachRouting::PutLateralWP(Reach* pReach, int subreachNdx, WaterParcel runoffWP, double withdrawal_m3)
{
   ReachSubnode* pNode = pReach->GetReachSubnode(subreachNdx);

   pNode->m_runoffWP = runoffWP;

   WaterParcel mixWP = pNode->m_waterParcel;
   mixWP.MixIn(runoffWP);
   pNode->m_withdrawalWP = WaterParcel(withdrawal_m3, mixWP.WaterTemperature());
} // end of PutLateralWP(pReach, subreachNdx, runoffWP, withdrawal_m3)


void ReachRouting::MoveWP(double volume_m3, WaterParcel* pFromWP, WaterParcel* pToWP)
{
   ASSERT(volume_m3 > 0 && volume_m3 < pFromWP->m_volume_m3);

   WaterParcel to_be_movedWP(volume_m3, pFromWP->WaterTemperature());
   pFromWP->Discharge(to_be_movedWP);
   pToWP->MixIn(to_be_movedWP);
} // end of MoveWP()


double ReachRouting::GetReachFluxes( FlowContext *pFlowContext, Reach *pReach )
   {
   float totalFlux = 0;

   pFlowContext->pReach = pReach;

   for ( int i=0; i < pReach->GetFluxCount(); i++ )
      {
      Flux *pFlux = pReach->GetFlux( i );
      ASSERT( pFlux != NULL );

      pFlowContext->pFlux = pFlux;
      pFlowContext->pFluxInfo = pFlux->m_pFluxInfo;
      
      float flux = pFlux->Evaluate( pFlowContext );
      
      // Note: this is a rate, applied over a time step

      // figure out which state var this flux is associated with
      //int sv = pFlux->m_pStateVar->m_index;

      // source or sink?  If sink, flip sign  
      if ( pFlux->IsSource() )
         totalFlux += flux;
      else
         totalFlux -= flux;
      }

   return totalFlux;
   }


double ReachRouting::KinematicWave(double oldQ_cms, double upstreamInflow_cms, double lateralInflow_cms, 
   double manningDepth_m, double wdRatio, double manningN, double slope, double length_m)
{
   double beta = 3.0 / 5.0;
   double manning_width_m = wdRatio * manningDepth_m;
   double wp = manning_width_m + manningDepth_m + manningDepth_m; // = (2 + wdRatio) * manningDepth_m = cross-section + manningDepth_m
   double alph = manningN * pow((long double)wp, (long double)(2 / 3.)) / sqrt(slope);
   double alpha = pow((long double)alph, (long double) 0.6);
      
   double Qstar_cms = (oldQ_cms + upstreamInflow_cms) / 2.0f; ASSERT(Qstar_cms > 0.);  // ? which book ? from Chow, eqn. 9.6.4 (m3/sec)
   double z = alpha * beta * pow(Qstar_cms, beta - 1.0); // s/m
   float dx_m = (float)length_m;

   double Qin_m2 = (upstreamInflow_cms * SEC_PER_DAY) / dx_m; // inflow term
   double Qcurrent_m2 = oldQ_cms * z; // current flow rate
   double divisor_s_per_m = z + SEC_PER_DAY / dx_m; // divisor        
   double qsurface_m2 = (lateralInflow_cms * SEC_PER_DAY) / dx_m;  //m2
   double newQ_cms = (qsurface_m2 + Qin_m2 + Qcurrent_m2) / divisor_s_per_m; 

   return(newQ_cms);
} // end of KinematicWave()


WaterParcel ReachRouting::ApplyReachOutflowWP(Reach* pReach, int subnode, double timeStep)
// WaterParcel version of EstimateReachOutflow()
// solves the KW equations for one subreach
{
   ReachSubnode* pSubnode = pReach->GetReachSubnode(subnode); ASSERT(pSubnode != NULL);
   pSubnode->m_previousWP = pSubnode->m_waterParcel;

   ASSERT(pSubnode->m_waterParcel.m_volume_m3 > 0);

   double original_temp_degC = pSubnode->m_waterParcel.WaterTemperature();
   double original_volume_m3 = pSubnode->m_waterParcel.m_volume_m3;
   WaterParcel upstream_inflowWP = GetReachInflowWP(pReach, subnode);
   double original_upstream_inflow_volume_m3 = upstream_inflowWP.m_volume_m3;

   // Use yesterday's outflow rate to determine the geometry of this subreach.
   double old_Q_cms = pSubnode->m_dischargeWP.m_volume_m3 / SEC_PER_DAY; ASSERT(old_Q_cms > 0); // old_Q_cms is yesterday's outflow rate from this subreach.

   // Make sure there is enough water available to handle any withdrawals from the reach for irrigation or municipal use.
   double net_lateral_inflow_m3 = pSubnode->m_runoffWP.m_volume_m3 - pSubnode->m_withdrawalWP.m_volume_m3;
   if (net_lateral_inflow_m3 < 0)
   { // net lateral inflow is negative, so there must be withdrawals of some sort
      if ((original_volume_m3 + upstream_inflowWP.m_volume_m3 + net_lateral_inflow_m3) < pSubnode->m_min_volume_m3)
      { // There is not enough water to handle the withdrawals.  
         // Add some magic water. This violates conservation of mass.
         double magic_H2O_to_add_m3 = pSubnode->m_min_volume_m3 - (original_volume_m3 + upstream_inflowWP.m_volume_m3 + net_lateral_inflow_m3);
         WaterParcel magic_H2O_to_addWP(magic_H2O_to_add_m3, original_temp_degC);
         pSubnode->m_addedVolumeWP.MixIn(magic_H2O_to_addWP);
         pSubnode->m_waterParcel.MixIn(WaterParcel(magic_H2O_to_addWP));
     } // end of block for adding magic water to the reach
   } // end of block for net lateral flow out of the reach

   // If there is more water already in this reach than the sum of what is flowing in from upstream plus the lateral inflow plus the minimum volume,
   // then treat some of what is already in the reach as if it had flowed in from upstream.
   if (net_lateral_inflow_m3 > 0 && pSubnode->m_waterParcel.m_volume_m3 > (net_lateral_inflow_m3 + upstream_inflowWP.m_volume_m3 + pSubnode->m_min_volume_m3))
   { // Treat the excess volume in the subreach as if it were flowing in from upstream.
      // This is a gimmick to get the KinematicWave() method to work better, since it doesn't take into account how much water is already in the reach.
      double excess_volume_m3 = pSubnode->m_waterParcel.m_volume_m3 - (net_lateral_inflow_m3 + upstream_inflowWP.m_volume_m3 + pSubnode->m_min_volume_m3);
      MoveWP(excess_volume_m3, &(pSubnode->m_waterParcel), &upstream_inflowWP);
   } // end of logic to move excess water so that it is treated as if it had flowed in from upstream

   // Now determine the discharge.
   double Qnew_cms = 0.;
   // How much water is available?
   double available_for_discharge_m3 = pSubnode->m_waterParcel.m_volume_m3 + upstream_inflowWP.m_volume_m3 + pSubnode->m_runoffWP.m_volume_m3
      - pSubnode->m_withdrawalWP.m_volume_m3 - pSubnode->m_min_volume_m3;
   double available_for_discharge_cms = available_for_discharge_m3 / SEC_PER_DAY;
   // Is there enough available to provide the minimum flow?
   double min_Q_cms = pReach->NominalLowFlow_cms();
   double magic_H2O_to_add_m3 = 0.;
   if (available_for_discharge_cms < min_Q_cms)
   { // No, not enough available to provide the minimum flow. Add some magic water.
      magic_H2O_to_add_m3 += (min_Q_cms - available_for_discharge_cms) * SEC_PER_DAY;
      Qnew_cms = min_Q_cms; 
   }
   else
   { // Yes, available water is sufficient for the minimum discharge. Try to use the kinematic wave solution.
      double KW_solution_cms = KinematicWave(old_Q_cms, upstream_inflowWP.m_volume_m3 / SEC_PER_DAY, net_lateral_inflow_m3 / SEC_PER_DAY,
         pSubnode->m_manning_depth_m, pReach->m_wdRatio, pReach->m_n, pReach->m_slope, pReach->m_deltaX);
      // Is the kinematic wave solution at least as large as the minimum flow?
      if (KW_solution_cms >= min_Q_cms)
      { // The kinematic wave solution is usable, provided there is enough water available.
         // How does available water compare to the Kinematic Wave solution?
         if (KW_solution_cms <= available_for_discharge_cms) Qnew_cms = KW_solution_cms; // Use the KW solution.
         else Qnew_cms = available_for_discharge_cms; // Maintain the minimum volume using a discharge less than the KW solution.
      }
      else
      { // The kinematic wave solution is not usable, because it is less than the minimum flow. It can even be negative, if the lateral
         // flow is out of the reach and large (e.g. the irrigation pumps suck all the water out of the reach and run dry).
         Qnew_cms = min_Q_cms;
      }
   }
   ASSERT(Qnew_cms > 0 && Qnew_cms < 1.e10);

   pSubnode->m_waterParcel.MixIn(upstream_inflowWP); 
   pSubnode->m_waterParcel.MixIn(pSubnode->m_runoffWP); 
   if (magic_H2O_to_add_m3 > 0.)
   {
      double magic_H2O_temp_degC = original_temp_degC;
      WaterParcel magicWP = WaterParcel(magic_H2O_to_add_m3, magic_H2O_temp_degC);
      pSubnode->m_waterParcel.MixIn(magicWP);
      pSubnode->m_addedVolumeWP.MixIn(magicWP);
   }

   if (pSubnode->m_withdrawalWP.m_volume_m3 > 0)
   { // Deal with lateral withdrawals, as for irrigation and drinking water
      // Adjust the temperature of any water being withdrawn laterally to match the temperature of the water in the reach,
      // now that runoff into the reach, upstream inflow, and magic water have been mixed in.
      pSubnode->m_withdrawalWP.m_temp_degC = pSubnode->m_waterParcel.m_temp_degC;
      pSubnode->m_waterParcel.Discharge(pSubnode->m_withdrawalWP);
   }

   pSubnode->m_discharge = Qnew_cms;
   pSubnode->m_dischargeWP = WaterParcel(pSubnode->m_discharge * SEC_PER_DAY, pSubnode->m_waterParcel.WaterTemperature());
   pSubnode->m_waterParcel.Discharge(pSubnode->m_dischargeWP);

   return(pSubnode->m_dischargeWP);
} // end of ApplyReachOutflowWP()


WaterParcel ReachRouting::GetReachInflowWP(Reach* pReach, int subNode)
{
   WaterParcel inflowWP(0,0);
   if (subNode == 0)  // look upstream?
   {
      Reservoir* pRes = pReach->m_pReservoir;
      if (pRes != NULL) inflowWP = pRes->m_outflowWP;
      else
      {
         if (pReach->m_pLeft != NULL) inflowWP = (gpModel->GetReachFromNode(pReach->m_pLeft))->GetReachDischargeWP();
         if (pReach->m_pRight != NULL) inflowWP.MixIn((gpModel->GetReachFromNode(pReach->m_pRight))->GetReachDischargeWP());
      }
   }
   else
   {
      ReachSubnode* pNode = (ReachSubnode*)pReach->m_subnodeArray[(long int)subNode - 1];  ///->GetReachSubnode( subNode-1 );
      ASSERT(pNode != NULL);
      inflowWP = pNode->m_dischargeWP;
   }

   return(inflowWP);
} // end of GetReachInflowWP()


double ReachRouting::GetLateralSVInflow( Reach *pReach, int sv )
   {
   double inflow = 0;
   ReachSubnode *pNode = pReach->GetReachSubnode( 0 );
   inflow=pNode->GetExtraSvFluxValue(sv);
   inflow /= pReach->GetSubnodeCount();  // kg/d

   return inflow;
   }


double ReachRouting::GetReachSVOutflow( ReachNode *pReachNode, int sv )   // recursive!!! for pahntom nodes
   {
   if ( pReachNode == NULL )
      return 0;

   if ( pReachNode->IsPhantomNode() )
      {
      double flux = GetReachSVOutflow( pReachNode->m_pLeft, sv );
      flux += GetReachSVOutflow( pReachNode->m_pRight, sv );

      return flux;
      }
   else
      {
      Reach *pReach = gpModel->GetReachFromNode( pReachNode );
      ReachSubnode *pNode = (ReachSubnode*) pReach->m_subnodeArray[ 0 ]; 

      if ( pReach != NULL && pNode->m_waterParcel.m_volume_m3 > 0.0f)
         return float( pNode->GetStateVar( sv ) / pNode->m_waterParcel.m_volume_m3 * pNode->m_discharge * 86400.0f );//kg/d 
      else
         {
         //ASSERT( 0 );
         return 0;
         }
      }
   }

/*x
double ReachRouting::GetReachSVInflow( Reach *pReach, int subNode, int sv )
   {
    double flux = 0.0;
    double Q = 0.0;

   if ( subNode == 0 )  // look upstream?
      {
      Reservoir *pRes = pReach->m_pReservoir;

      if ( pRes == NULL )
         {
         flux = GetReachSVOutflow( pReach->m_pLeft, sv );
         flux += GetReachSVOutflow( pReach->m_pRight, sv );
         }
      else
         flux = pRes->m_outflow/SEC_PER_DAY;  //m3/day to m3/s
      }
   else
      {
      ReachSubnode *pNode = (ReachSubnode*) pReach->m_subnodeArray[ subNode-1 ];  ///->GetReachSubnode( subNode-1 );
      ASSERT( pNode != NULL );
     
      if ( pNode->m_svArray != NULL)
         flux  = float( pNode->m_svArray[sv]/pNode->m_waterParcel.m_volume_m3 * pNode->m_discharge*86400.0f );//kg/d 
      }

   return flux; //kg/d;
   }
x*/

void ReachRouting::GetReachDerivatives( double time, double timeStep, int svCount, double *derivatives /*out*/, void *extra )
   {
   FlowContext *pFlowContext = (FlowContext*) extra;

   FlowContext flowContext( *pFlowContext );   // make a copy for openMP

   // NOTE: NEED TO INITIALIZE TIME IN FlowContext before calling here...
   // NOTE:  Should probably do a breadth-first search of the tree(s) rather than what is here.  This could be accomplished by
   //  sorting the m_reachArray in InitReaches()

   FlowModel *pModel = pFlowContext->pFlowModel;
   flowContext.timeStep = (float) timeStep;
   
   // compute derivative values for reachs based on any associated subnode fluxes
   int reachCount = (int) pModel->m_reachArray.GetSize();

   omp_set_num_threads( gpFlow->m_processorsUsed );
   #pragma omp parallel for firstprivate( flowContext )
   for ( int i=0; i < reachCount; i++ )
      {
      Reach *pReach = pModel->m_reachArray[ i ];
      flowContext.pReach = pReach;

      for ( int l=0; l < pReach->GetSubnodeCount(); l++ )
         {
         ReachSubnode *pNode = pReach->GetReachSubnode( l );
         int svIndex = pNode->m_svIndex;

         flowContext.reachSubnodeIndex = l;
         derivatives[svIndex]=0;

         // add all fluxes for this reach
         for ( int k=0; k < pReach->GetFluxCount(); k++ )
            {
            Flux *pFlux = pReach->GetFlux( k );
            flowContext.pFlux = pFlux;

            float flux = pFlux->Evaluate( pFlowContext );

            // figure out which state var this flux is associated with
            int sv = pFlux->m_pStateVar->m_index;

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
   }


ReachRouting *ReachRouting::LoadXml( TiXmlElement *pXmlReachRouting, LPCTSTR filename )
   {
   ReachRouting *pRouting = new ReachRouting( "Reach Routing" );  // defaults to GM_KINEMATIC

   LPTSTR method = NULL;
   LPTSTR query = NULL;   // ignored for now
   LPTSTR fn = NULL;
   LPTSTR db = NULL;

   XML_ATTR attrs[] = {
      // attr                 type          address                     isReq  checkCol
      { "name",               TYPE_CSTRING,   &(pRouting->m_name),      false,   0 },
      { "method",             TYPE_STRING,    &method,                  false,   0 },
      { "query",              TYPE_STRING,    &query,                   false,   0 },
      { NULL,                 TYPE_NULL,     NULL,                      false,  0 } };

   bool ok = TiXmlGetAttributes( pXmlReachRouting, attrs, filename );
   if ( ! ok )
      {
      CString msg; 
      msg.Format( _T("Flow: Misformed element reading <reach_routing> attributes in input file %s"), filename );
      Report::ErrorMsg( msg );
      }

   if ( method )
      {
      if ( lstrcmpi( method, "kinematic" ) == 0 )
         pRouting->SetMethod(  GM_KINEMATIC );
      else if ( lstrcmpi( method, "euler" ) == 0 )
         pRouting->SetMethod(  GM_EULER );
      else if ( lstrcmpi( method, "rkf" ) == 0 )
         pRouting->SetMethod(  GM_RKF );
      else if ( lstrcmpi( method, "rk4" ) == 0 )
         pRouting->SetMethod(  GM_RK4 );
      else if ( lstrcmpi( method, "none" ) == 0 )
         pRouting->SetMethod(  GM_NONE );
      else if ( strncmp( method, "fn", 2 ) == 0 )
         {
         pRouting->SetMethod(  GM_EXTERNAL );
         // source string syntax= fn:<dllpath:function> for functional, db:<datasourcepath:columnname> for datasets
         pRouting->m_extSource = method;
         FLUXSOURCE sourceLocation = ParseSource( pRouting->m_extSource, pRouting->m_extPath, pRouting->m_extFnName,
               pRouting->m_extFnDLL, pRouting->m_extFn );
         
         if ( sourceLocation != FS_FUNCTION )
            {
            Report::ErrorMsg( "Fatal Error on direct reach solution method - no solution will be performed" );
            pRouting->SetMethod( GM_NONE );
            }
         }
      }

   return pRouting;
   }
   

