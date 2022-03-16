// WaterParcel.cpp

#include "libs.h"
#include "ScienceFcns.h"

#include "WaterParcel.h"


WaterParcel::WaterParcel()
{
   m_volume_m3 = 0;
   m_temp_degC = 0;
} // end of default constructor for WaterParcel objects


WaterParcel::WaterParcel(double volume_m3, double temperature_degC)
{
   this->m_volume_m3 = volume_m3;
   this->m_temp_degC = temperature_degC;
} // end of WaterParcel constructor


void WaterParcel::Discharge(WaterParcel dischargeWP)
// This routine removes water and energy from a WaterParcel object, but it doesn't put it anywhere.
// So for conservation of mass and energy, add the WaterParcel in somewhere else (e.g. call MixIn()) before calling Discharge(). 
{
   double departing_volume_m3 = dischargeWP.m_volume_m3;
   ASSERT(departing_volume_m3 >= 0);
   double departing_energy_kJ = dischargeWP.ThermalEnergy();

   double new_energy_kJ = this->ThermalEnergy() - dischargeWP.ThermalEnergy(); ASSERT(new_energy_kJ >= 0);
   this->m_volume_m3 -= dischargeWP.m_volume_m3; ASSERT(this->m_volume_m3 >= 0);
   this->m_temp_degC = WaterTemperature(new_energy_kJ); ASSERT(this->m_temp_degC >= 0);
} // end of void WaterParcel::Discharge(WaterParcel)


WaterParcel WaterParcel::Discharge(double outflowVolume_m3)
{
   ASSERT(outflowVolume_m3 >= 0 && outflowVolume_m3 <= this->m_volume_m3);

   WaterParcel departingWP(outflowVolume_m3, this->m_temp_degC);
   this->m_volume_m3 -= outflowVolume_m3;
   return(departingWP);
} // end of WaterParcel WaterParcel::Discharge(double)


int WaterParcel::Evaporate(double evap_volume_m3, double evap_energy_kJ) // Rtns 0 normally; < 0 for exceptional conditions.
// Rtns -1 and WaterParcel is diminished by evaporation when the temperature of the diminished WaterParcel is > NOMINAL_MAX_WATER_TEMP_DEGC.
// Rtns -2 and WaterParcel is unchanged when the diminished energy is <= 0.
// Rtns -3 and WaterParcel is unchanged when the diminished volume is <= 0.
{
   double WP_energy_kJ = this->ThermalEnergy() - evap_energy_kJ;
   if (WP_energy_kJ <= 0.) 
      return(-2);

   double WP_volume_m3 = m_volume_m3 - evap_volume_m3;
   if (WP_volume_m3 <= 0.) 
      return(-3);

   m_volume_m3 = WP_volume_m3;
   m_temp_degC = WaterParcel::WaterTemperature(m_volume_m3, WP_energy_kJ);

   if (m_temp_degC > NOMINAL_MAX_WATER_TEMP_DEGC)
   { 
      CString msg;
      msg.Format("Evaporate() m_temp_degC = %f is > NOMINAL_MAX_WATER_TEMP_DEGC.", m_temp_degC);
      Report::LogWarning(msg);
      return(-1);
   }
   return(0);
} // end of Evaporate()


void WaterParcel::MixIn(WaterParcel inflowWP)
{ // Note that this method tolerates negative values for volume and energy.
   if (inflowWP.m_volume_m3 == 0) return;

   double thermal_energy_kJ = this->ThermalEnergy() + inflowWP.ThermalEnergy();
   m_volume_m3 += inflowWP.m_volume_m3;
   m_temp_degC = WaterTemperature(m_volume_m3, thermal_energy_kJ);
} // end of MixIn()


double WaterParcel::WaterTemperature(double thermalEnergy_kJ)
{
   double temp_degC = WaterTemperature(this->m_volume_m3, thermalEnergy_kJ);
   return(temp_degC);
} // end of WaterTemperature(thermalEnergy_kJ)


double WaterParcel::WaterTemperature(double volume_m3, double thermalEnergy_kJ)
{
   if (volume_m3 == 0) return(0);

   double temperature_degC = thermalEnergy_kJ / (volume_m3 * DENSITY_H2O * SPECIFIC_HEAT_H2O);
   ASSERT(temperature_degC < 300. && temperature_degC >= 0.);
   return(temperature_degC);
} // end of static double WaterTemperature(double volume_m3, double thermalEnergy_kJ);


double WaterParcel::WaterTemperature()
{
   ASSERT(0 <= m_temp_degC && m_temp_degC < 300.); // ??? < 50.
   return(m_temp_degC);
} // end of WaterTemperature()


double WaterParcel::ThermalEnergy()
{
   double thermalEnergy_kJ = ThermalEnergy(m_volume_m3, m_temp_degC);
   return(thermalEnergy_kJ);
} // end of ThermalEnergy()


double WaterParcel::ThermalEnergy(double temperature_degC)
{
   double thermalEnergy_kJ = ThermalEnergy(m_volume_m3, temperature_degC);
   return(thermalEnergy_kJ);
} // end of ThermalEnergy(temperature_degC))


double WaterParcel::ThermalEnergy(double volume_m3, double temperature_degC)
{
   double thermalEnergy_kJ = temperature_degC * volume_m3 * DENSITY_H2O * SPECIFIC_HEAT_H2O;
   return(thermalEnergy_kJ);
} // end of static double ThermalEnergy(double volume_m3, double temperature_degC);


double WaterParcel::SatVP_mbar(double tempAir_degC)
{
   double sat_vp_kPa = 0.611 * exp(17.3 * tempAir_degC / (tempAir_degC + 237.3)); // Eq. D-7, p. 586 in Dingman 2002
   double sat_vp_mbar = sat_vp_kPa * PA_PER_MBAR / 1000.;
   return(sat_vp_mbar);
} // end of SatVP_mbar(tempAir_degC)
