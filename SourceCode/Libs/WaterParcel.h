#pragma once
#include "libs.h"

class LIBSAPI WaterParcel
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

/*x
// class LIBSAPI WaterParcel
class WaterParcel
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
