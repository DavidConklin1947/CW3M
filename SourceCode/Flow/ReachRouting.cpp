#include "stdafx.h"

#pragma hdrstop

#include "GlobalMethods.h"
#include "Flow.h"

#include <UNITCONV.H>
#include <omp.h>

extern FlowProcess *gpFlow;
extern FlowModel *gpModel;



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
            pNode->m_addedVolume_m3 += node_current_added_volume_m3;
            pNode->m_waterParcel = WaterParcel(volume, H2O_temp_degC);
         }
      } // end of loop thru subnodes
   } // end of logic to ensure that every discharge is > 0 and every volume is > 0

   return(rtnflag);
} // end of ReachRouting::Step()


bool ReachRouting::SolveReachKinematicWave( FlowContext *pFlowContext )
   {
   ASSERT(pFlowContext->svCount <= 0);

   pFlowContext->Reset();

   int reachCount = gpModel->GetReachCount();

   clock_t start = clock();

   // basic idea - for each Reach, estimate its outflow 
   for ( int i=0; i < reachCount; i++ )
      {
      Reach *pReach = gpModel->GetReach( i );     // Note: these are guaranteed to be non-phantom
      int num_subreaches = pReach->GetSubnodeCount();
      float temp_air_degC = gpModel->GetTodaysReachTEMP_AIR(pReach);
      float tmax_air_degC = gpModel->GetTodaysReachTMAX_AIR(pReach);
      float reach_precip_mm = gpModel->GetTodaysReachPRECIP(pReach);
      float reach_ws_m_sec = gpModel->GetTodaysReachWINDSPEED(pReach);
      float rad_sw_unshaded_W_m2 = gpModel->GetTodaysReachRAD_SW(pReach); // Shortwave from the climate data takes into account cloudiness but not shading.
      double cloudiness_frac = reach_precip_mm > 0 ? 0.5 : 0; // ??? we need a better estimate of cloudiness than this
      float sphumidity = gpModel->GetTodaysReachSPHUMIDITY(pReach);
      double z_mean_m; gpModel->m_pStreamLayer->GetData(pReach->m_polyIndex, gpModel->m_colReachZ_MEAN, z_mean_m);
      double ea, vpd;
      double rh_pct = 100 * ETEquation::CalculateRelHumidity(sphumidity, temp_air_degC, tmax_air_degC, (float)z_mean_m, ea, vpd);
      double reach_net_lw_kJ = 0;

      // Put this here temporarily
      for (int subreachNdx = 0; subreachNdx < num_subreaches; subreachNdx++)
      {
         ReachSubnode * pReachSubnode = pReach->GetReachSubnode(subreachNdx);
         SetSubreachGeometry(pReach, subreachNdx, pReachSubnode->m_discharge);
      }

      double total_surface_in_subreaches_m2 = 0;
      double lw_x_surface_accum = 0;
      double sw_x_surface_accum = 0;
      pReach->m_reach_evap_m3 = 0.;
      pReach->m_reach_evap_kJ = 0.;
      double subreach_length_m = pReach->m_length / pReach->GetSubnodeCount();
      double width_x_length_accum = 0.;
      double manning_depth_x_length_accum = 0;
      double volume_accum_m3 = 0.;
      for (int l = 0; l < pReach->GetSubnodeCount(); l++)
         {
         pFlowContext->pReach = pReach;
         ReachSubnode *pNode = pReach->GetReachSubnode(l);
         double orig_m_volume_m3 = pNode->m_waterParcel.m_volume_m3;

         double rad_sw_net_W_m2 = rad_sw_unshaded_W_m2; // ??? s/b pReach->GetSegmentShade_a_lator(segment, rad_sw_unshaded_W_m2);
         double temp_H2O_degC = pNode->m_waterParcel.m_temp_degC;
         double vts_frac = pReach->GetSubreachViewToSky_frac(l);
         double net_lw_out_W_m2 = NetLWout_W_m2(temp_air_degC, cloudiness_frac, temp_H2O_degC, rh_pct, vts_frac);

         // The evap rate is a function of shortwave flux, longwave flux, aerodynamic evaporation, the air temperature,
         // the water temperature, and the relative humidity.  The equation also uses the latent heat of vaporization,
         // the psychrometric constant, and the slope of the saturation vapor v. air temperature curve.  The
         // aerodynamic evaporation is a function of the windspeed.
         double evap_m_s = pReach->Evap_m_s(l, rad_sw_net_W_m2, net_lw_out_W_m2, temp_air_degC, reach_ws_m_sec, sphumidity);
         double evap_m3 = evap_m_s * pNode->m_subreach_surf_area_m2 * SEC_PER_DAY;
         pNode->m_evap_m3 = evap_m3;
         pNode->m_evap_kJ = evap_m3 * DENSITY_H2O * pReach->LatentHeatOfVaporization_MJ_kg(temp_H2O_degC) * 1000.;

         pNode->m_sw_kJ = rad_sw_net_W_m2 * pNode->m_subreach_surf_area_m2 * SEC_PER_DAY / 1000;
         pNode->m_lw_kJ = net_lw_out_W_m2 * pNode->m_subreach_surf_area_m2 * SEC_PER_DAY / 1000;
         reach_net_lw_kJ += pNode->m_lw_kJ;
         total_surface_in_subreaches_m2 += pNode->m_subreach_surf_area_m2;
         lw_x_surface_accum += net_lw_out_W_m2 * pNode->m_subreach_surf_area_m2;
         sw_x_surface_accum += (pNode->m_sw_kJ / SEC_PER_DAY) * 1000;



         double lateralInflow = GetLateralInflow(pReach);
         double externalFluxes = GetReachFluxes(pFlowContext, pReach);
         double new_lateralInflow = lateralInflow + externalFluxes;

         if (new_lateralInflow != new_lateralInflow)
            {
            CString msg;
            msg.Format("*** SolveReachKinematicWave(): i = %d, l = %d, lateralInflow = %f, externalFluxes = %f", i, l, lateralInflow, externalFluxes);
            Report::LogMsg(msg);
            msg.Format("Setting new_lateralInflow to %f cms and continuing.", NOMINAL_LOW_FLOW_CMS); Report::LogMsg(msg);
            new_lateralInflow = NOMINAL_LOW_FLOW_CMS;
            }

         PutLateralWP(pReach, l, new_lateralInflow);

         // Gain water from precip and lose water via evaporation.
         // In both cases the amount of water gained or lost is proportional to the surface area of the water.
//       double subreach_precip_vol_m3 = pNode->m_subreach_surf_area_m2 * reach_precip_mm / 1000;
//       WaterParcel subreach_precipWP(subreach_precip_vol_m3, temp_air_degC);
//       pNode->m_waterParcel.MixIn(subreach_precipWP); // ??? This violates conservation of mass because IDU surface areas overlap stream surface areas.
         pNode->m_waterParcel.Evaporate(pNode->m_evap_m3, pNode->m_evap_kJ);
         pReach->m_reach_evap_m3 += pNode->m_evap_m3;
         pReach->m_reach_evap_kJ += pNode->m_evap_kJ;

         // Gain thermal energy from incoming shortwave radiation and lose it to outgoing longwave radiation.
// not yet         double subreach_tempC = pNode->m_waterParcel.WaterTemperature();
// not yet         double subreach_kJ = pNode->m_waterParcel.ThermalEnergy(subreach_tempC) + pReach->SubReachNetRad_kJ(l);
         double subreach_net_rad_kJ = pReach->SubreachNetRad_kJ(l);
         if (subreach_net_rad_kJ < 0.)
         { // Theoretically, the water is losing heat to the atmosphere by radiation.
            double min_skin_temp_degC = 1.;
            double skin_temp_degC = max(temp_air_degC, min_skin_temp_degC);
            if (pNode->m_waterParcel.WaterTemperature() > skin_temp_degC)
            { // The water is warmer than the air, so it is plausible that the water might cool down
               // but not to a temperature less than the skin temperature (the temperature of the air in contact with the liquid water surface).
               double temp_diff = pNode->m_waterParcel.WaterTemperature() - skin_temp_degC;
               double max_heat_loss_kJ = WaterParcel::ThermalEnergy(pNode->m_waterParcel.m_volume_m3, temp_diff);
               double heat_loss_kJ = min(max_heat_loss_kJ, -subreach_net_rad_kJ);
               double thermal_energy_kJ = pNode->m_waterParcel.ThermalEnergy() - heat_loss_kJ;
               pNode->m_waterParcel.m_temp_degC = WaterParcel::WaterTemperature(pNode->m_waterParcel.m_volume_m3, thermal_energy_kJ);               
               ASSERT(pNode->m_waterParcel.m_temp_degC >= 0);
            }
         }
         else
         {
            double thermal_energy_kJ = pNode->m_waterParcel.ThermalEnergy() + subreach_net_rad_kJ;
            pNode->m_waterParcel.m_temp_degC = WaterParcel::WaterTemperature(pNode->m_waterParcel.m_volume_m3, thermal_energy_kJ);

            if (l == 0 && pNode->m_waterParcel.m_temp_degC >= 150.)
            {
               CString msg;
               msg.Format("SolveReachKinematicWave() temperature >= 50 degC in reach %d subreach %d: orig kJ = %f,\n"
                  "pNode->m_sw_kJ = %f, pNode->m_lw_kJ = %f\n"
                  "pNode->m_subreach_surf_area_m2 = %f, pNode->m_volume_m3 = %f, pNode->m_subreach_manning_depth_m = %f, pNode->m_subreach_width_m = %f, m_temp_degC = %f",
                  pReach->m_reachID, l, pNode->m_waterParcel.ThermalEnergy(), pNode->m_sw_kJ, pNode->m_lw_kJ,
                  pNode->m_subreach_surf_area_m2, pNode->m_waterParcel.m_volume_m3, pNode->m_subreach_manning_depth_m, pNode->m_subreach_width_m, pNode->m_waterParcel.m_temp_degC);
               Report::LogMsg(msg);
            }
         }
         ASSERT(pNode->m_waterParcel.m_temp_degC >= 0);

         WaterParcel outflowWP(0,0);
         outflowWP = ApplyReachOutflowWP(pReach, l, pFlowContext->timeStep); // s/b DailySubreachFlow(pReach, l, subreach_evapWP, subreach_precipWP);
         
         ASSERT(pNode->m_waterParcel.m_volume_m3 > 0);

         pNode->m_discharge = outflowWP.m_volume_m3 / SEC_PER_DAY;  // convert units to m3/s
         width_x_length_accum += pNode->m_subreach_width_m * subreach_length_m;
         manning_depth_x_length_accum += pNode->m_subreach_manning_depth_m * subreach_length_m;
         volume_accum_m3 += pNode->m_waterParcel.m_volume_m3;

         if (pNode->m_discharge < 0 || pNode->m_discharge > 1.e10f || pNode->m_discharge != pNode->m_discharge)
            {
            CString msg;
            msg.Format("*** SolveReachKinematicWave(): COMID = %d, i = %d, l = %d, m_discharge = %f",
               pReach->m_reachID, i, l, pNode->m_discharge);
            Report::LogMsg(msg);
            msg.Format("Setting pNode->m_discharge to %f cms and continuing.", NOMINAL_LOW_FLOW_CMS); Report::LogMsg(msg);
            pNode->m_discharge = NOMINAL_LOW_FLOW_CMS;
            }
         } // end of loop through subreaches
      pReach->m_rad_lw_kJ = reach_net_lw_kJ;
      double reach_lw_W_m2 = lw_x_surface_accum / total_surface_in_subreaches_m2;
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachRAD_LW_OUT, reach_lw_W_m2);
      double reach_sw_W_m2 = sw_x_surface_accum / total_surface_in_subreaches_m2;
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachRAD_SW_IN, reach_sw_W_m2);
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachAREA_H2O, total_surface_in_subreaches_m2);

      double reach_width_m = width_x_length_accum / pReach->m_length;
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachWIDTH, reach_width_m);
      double reach_manning_depth_m = manning_depth_x_length_accum / pReach->m_length;
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachDEPTHMANNG, reach_manning_depth_m);
      double turnover = (pReach->GetDischarge() * SEC_PER_DAY) / volume_accum_m3;
      gpModel->m_pStreamLayer->SetDataU(pReach->m_polyIndex, gpModel->m_colReachTURNOVER, turnover);

      } // end of loop through reaches

   clock_t finish = clock();
   double duration = (float)(finish - start) / CLOCKS_PER_SEC;   
   gpModel->m_reachFluxFnRunTime += (float) duration;   

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
   inflow = pReach->GetFluxValue();
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
   } // end of GetLateralFlow()


void ReachRouting::PutLateralWP(Reach * pReach, int subreachNdx, double daily_net_subreach_lateral_flow_m3) // Allocates runoff and withdrawals to a subreach
{
   ASSERT(!isnan(daily_net_subreach_lateral_flow_m3));

   ReachSubnode* pNode = pReach->GetReachSubnode(subreachNdx);

   bool flow_is_into_reach = daily_net_subreach_lateral_flow_m3 >= 0;
   double subreach_volume_in_m3 = flow_is_into_reach ? daily_net_subreach_lateral_flow_m3 : 0;
   double subreach_volume_out_m3 = flow_is_into_reach ? 0 : fabs(daily_net_subreach_lateral_flow_m3);

   pNode->m_runoffWP = WaterParcel(subreach_volume_in_m3, DEFAULT_SOIL_H2O_TEMP_DEGC); // water parcel running off the soil into the subreach
   pNode->m_withdrawalWP = WaterParcel(subreach_volume_out_m3, pNode->m_waterParcel.WaterTemperature()); // water parcel being withdrawn from the subreach (e.g. for irrigation)
} // end of PutLateralWP()


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


double ReachRouting::KinematicWave(double oldQ_cms /* Q */, double upstreamInflow_cms /* Qin */, double lateralInflow_cms, Reach * pReach)
{
   double beta = 3.0 / 5.0;
   double wp = (double)(pReach->m_width + pReach->m_depth + pReach->m_depth);
   double alph = pReach->m_n * pow((long double)wp, (long double)(2 / 3.)) / sqrt(pReach->m_slope);
   double alpha = pow((long double)alph, (long double) 0.6);
      
   double Qstar_cms = (oldQ_cms + upstreamInflow_cms) / 2.0f; ASSERT(Qstar_cms > 0.);  // from Chow, eqn. 9.6.4 (m3/sec)
   double z = alpha * beta * pow(Qstar_cms, beta - 1.0); // s/m
   float dx_m = pReach->m_deltaX;

   double Qin_m2 = (upstreamInflow_cms * SEC_PER_DAY) / dx_m; // inflow term
   double Qcurrent_m2 = oldQ_cms * z; // current flow rate
   double divisor_s_per_m = z + SEC_PER_DAY / dx_m; // divisor        
   double qsurface_m2 = (lateralInflow_cms * SEC_PER_DAY) / dx_m;  //m2
   double newQ_cms = (qsurface_m2 + Qin_m2 + Qcurrent_m2) / divisor_s_per_m; ASSERT(newQ_cms > 0.);

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
//x   double lateral_inflow_m3 = lateralInflowWP.m_volume_m3;
   WaterParcel upstream_inflowWP = GetReachInflowWP(pReach, subnode);
   double original_upstream_inflow_volume_m3 = upstream_inflowWP.m_volume_m3;

   // Use yesterday's outflow rate to determine the geometry of this subreach.
   double old_Q_cms = pSubnode->m_dischargeWP.m_volume_m3 / SEC_PER_DAY; ASSERT(old_Q_cms > 0); // old_Q_cms is yesterday's outflow rate from this subreach.
   SetGeometry(pReach, (float)old_Q_cms);

   double Qin_m3 = GetReachInflow(pReach, subnode) * SEC_PER_DAY;
   ASSERT(close_enough(original_upstream_inflow_volume_m3, Qin_m3, 0.1, 100.));

   // Make sure there is enough water available to handle any withdrawals from the reach for irrigation or municipal use.
   double net_lateral_inflow_m3 = pSubnode->m_runoffWP.m_volume_m3 - pSubnode->m_withdrawalWP.m_volume_m3;
   if (net_lateral_inflow_m3 < 0)
   { // net lateral inflow is negative, so there must be withdrawals of some sort
      if ((original_volume_m3 + upstream_inflowWP.m_volume_m3 + net_lateral_inflow_m3) < 0)
      { // There is not enough water to handle the withdrawals.  
         // Add some magic water. This violates conservation of mass.
         double shortage_volume_m3 = fabs(original_volume_m3 + upstream_inflowWP.m_volume_m3 + net_lateral_inflow_m3);
         double min_volume_m3 = (NOMINAL_LOW_WATER_LITERS_PER_METER * pReach->m_length / pReach->GetSubnodeCount()) / LITERS_PER_M3;
         double magic_H2O_to_add_m3 = min_volume_m3 + shortage_volume_m3;
         pSubnode->m_waterParcel.MixIn(WaterParcel(magic_H2O_to_add_m3, original_temp_degC));
         pSubnode->m_addedVolume_m3 += magic_H2O_to_add_m3;
      } // end of block for adding magic water to the reach
   } // end of block for net lateral flow out of the reach

   // If there is more water already in this reach than the sum of what is flowing in from upstream plus the lateral inflow,
   // then treat some of what is already in the reach as if it had flowed in from upstream.
   if (net_lateral_inflow_m3 > 0 && pSubnode->m_waterParcel.m_volume_m3 > (net_lateral_inflow_m3 + upstream_inflowWP.m_volume_m3))
   { // Treat the excess volume in the subreach as if it were flowing in from upstream.
      // This is a gimmick to get the KinematicWave() method to work better, since it doesn't take into account how much water is already in the reach.
      double excess_volume_m3 = pSubnode->m_waterParcel.m_volume_m3 - (net_lateral_inflow_m3 + upstream_inflowWP.m_volume_m3);
      MoveWP(excess_volume_m3, &(pSubnode->m_waterParcel), &upstream_inflowWP);
   } // end of special logic for headwater reaches

   double Qnew_cms = KinematicWave(old_Q_cms, upstream_inflowWP.m_volume_m3 / SEC_PER_DAY, net_lateral_inflow_m3 / SEC_PER_DAY, pReach); ASSERT(Qnew_cms > 0);

   // Constrain the amount of water leaving the reach to no more than what is already there plus
   // what is coming in, less at least a small amount to keep the volume positive and greater than zero.
   float min_volume_m3 = (NOMINAL_LOW_WATER_LITERS_PER_METER * pReach->m_length / pReach->GetSubnodeCount()) / LITERS_PER_M3;
   double incoming_volume_m3 = upstream_inflowWP.m_volume_m3 + net_lateral_inflow_m3;
   double max_outgoing_volume_m3 = (pSubnode->m_waterParcel.m_volume_m3 + incoming_volume_m3) - min_volume_m3;
   double max_Qnew_cms = max_outgoing_volume_m3 / SEC_PER_DAY;
   if (max_Qnew_cms < NOMINAL_LOW_FLOW_CMS) max_Qnew_cms = NOMINAL_LOW_FLOW_CMS;
   if (Qnew_cms > max_Qnew_cms) Qnew_cms = max_Qnew_cms;
   ASSERT(Qnew_cms > 0 && Qnew_cms < 1.e10);

   SetGeometry(pReach, (float)Qnew_cms);
   SetSubreachGeometry(pReach, subnode, Qnew_cms);

   pSubnode->m_waterParcel.MixIn(upstream_inflowWP); 
   pSubnode->m_waterParcel.MixIn(pSubnode->m_runoffWP); 
   pSubnode->m_waterParcel.Discharge(pSubnode->m_withdrawalWP);

   double outflow_today_m3 = Qnew_cms * (double)SEC_PER_DAY;
   pSubnode->m_dischargeWP = WaterParcel(outflow_today_m3, pSubnode->m_waterParcel.WaterTemperature());
   pSubnode->m_waterParcel.Discharge(pSubnode->m_dischargeWP);

   return(pSubnode->m_dischargeWP);
} // end of ApplyReachOutflowWP()


double ReachRouting::GetReachInflow( Reach *pReach, int subNode )
   {
   double Q = 0;

   ///////
   if ( subNode == 0 )  // look upstream?
      {
      Reservoir *pRes = pReach->m_pReservoir;

      if ( pRes == NULL )
         {
         Q = GetReachOutflow( pReach->m_pLeft );
         Q += GetReachOutflow( pReach->m_pRight );
         }
      else
         //Q = pRes->GetResOutflow( pRes, m_flowContext.dayOfYear );
       Q = pRes->m_outflow/SEC_PER_DAY;  //m3/day to m3/s
      }
   else
      {
      ReachSubnode *pNode = (ReachSubnode*) pReach->m_subnodeArray[ subNode-1 ];  ///->GetReachSubnode( subNode-1 );
      ASSERT( pNode != NULL );
      Q = pNode->m_discharge;
      }

   return Q;
   } // end of GetReachInflow()


WaterParcel ReachRouting::GetReachInflowWP(Reach* pReach, int subNode)
{
   WaterParcel inflowWP(0,0);
   if (subNode == 0)  // look upstream?
   {
      Reservoir* pRes = pReach->m_pReservoir;
      if (pRes != NULL)
      {
         inflowWP = pRes->m_outflowWP;
         ASSERT(pReach->m_pRight == NULL);

         double reach_inflow_m3 = SEC_PER_DAY * GetReachInflow(pReach, subNode);
         ASSERT(close_enough(inflowWP.m_volume_m3, reach_inflow_m3, 0.1, 100.));
      }
      else
      {
         inflowWP = GetReachOutflowWP(pReach->m_pLeft);
         inflowWP.MixIn(GetReachOutflowWP(pReach->m_pRight));

         double reach_inflow_m3 = SEC_PER_DAY * GetReachInflow(pReach, subNode);
         ASSERT(close_enough(inflowWP.m_volume_m3, reach_inflow_m3, 0.1, 100.));
      }
   }
   else
   {
      ReachSubnode* pNode = (ReachSubnode*)pReach->m_subnodeArray[subNode - 1];  ///->GetReachSubnode( subNode-1 );
      ASSERT(pNode != NULL);
      inflowWP = pNode->m_dischargeWP;
   }

   double reach_inflow_m3 = SEC_PER_DAY * GetReachInflow(pReach, subNode);
   ASSERT(close_enough(inflowWP.m_volume_m3, reach_inflow_m3, 0.1, 100.));

   return(inflowWP);
} // end of GetReachInflowWP()

double ReachRouting::GetReachOutflow( ReachNode *pReachNode )   // recursive!!! for pahntom nodes
   {
   if ( pReachNode == NULL )
      return 0;

   if ( pReachNode->IsPhantomNode() )
      {
      double q = GetReachOutflow( pReachNode->m_pLeft );
      q += GetReachOutflow( pReachNode->m_pRight );

      return q;
      }
   else
      {
      Reach *pReach = gpModel->GetReachFromNode( pReachNode );

      if ( pReach != NULL )
         return pReach->GetDischarge();
      else
         {
         ASSERT( 0 );
         return 0;
         }
      }
   }


WaterParcel ReachRouting::GetReachOutflowWP(ReachNode* pReachNode)
{
   WaterParcel outflowWP(0, 0);
   if (pReachNode == NULL) return(outflowWP);

   ASSERT(!pReachNode->IsPhantomNode());

   Reach* pReach = gpModel->GetReachFromNode(pReachNode);
   ASSERT(pReach != NULL);
   outflowWP = pReach->GetDischargeWP();
   return(outflowWP);
} // end of GetReachOutflowWP()


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
   