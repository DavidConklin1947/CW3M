#include "StdAfx.h"
#pragma hdrstop

#include "ETEquation.h"
#include "GlobalMethods.h"
#include <Report.h>
#include <cmath>
#include <DATE.HPP>
#include <UNITCONV.h>
#include "Flow.h"
#include "AlgLib\AlgLib.h"

#define MJPERHR_PER_W 0.0036              // Watts -> MegaJoules per hour
#define MJPERD_PER_W  0.0864              // Watts/m^2 -> MegaJoules/m^2*day
#define KMPERDAY_PER_MPERSEC 86.4         // m/sec -> km/day
#define KMD_PER_MMHR 0.000024;            // mm/h -> km/day


double ETEquation::NOVAL;

ETEquation::ETEquation( EvapTrans *pEvapTrans )
: m_pEvapTrans( pEvapTrans )
   {
   NOVAL = -10000000.;

   m_mode = NO_MODE;
   m_useTallRefCrop = true;

//   m_airTemperature = NOVAL;
 
   m_dailyMeanTemperature = NOVAL;

   m_relativeHumidity = NOVAL;
   m_windSpeed = NOVAL;
   m_solarRadiation = NOVAL;
   m_twilightSolarRadiation = NOVAL;
   m_stationElevation = NOVAL;
   m_stationLatitude = NOVAL;
   m_stationLongitude = NOVAL;
   m_timeZoneLongitude = NOVAL;

   m_dailyMaxTemperature = NOVAL;
   m_dailyMinTemperature = NOVAL;
   m_recentMeanTemperature = NOVAL;

   m_agrimetStationCoeffs;
 /*  m_month=0;
   m_day=0;
   m_year=0;
   m_hoursInDay = (UINT) NOVAL;*/
   }


ETEquation::~ETEquation( void )
   {
   }

float ETEquation::Run()
   {
   return Run( m_mode, NULL );
   }

float ETEquation::Run( unsigned short mode, HRU *pHRU )
   {
   float result = -1.0f;

   switch( mode )
   {
   case ASCE:
      result = Asce();
      break;

   case FAO56:
      result = Fao56();
      break;
   
   case PENN_MONT:
      result = PennMont( pHRU );
      break;

   case KIMB_PENN:
      result = KimbPenn();
      break;

   case HARGREAVES:
      result = Hargreaves();
      break;

   default:
      CString msg; 
      msg.Format( _T("ET Equation: no equation mode defined"));
      Report::ErrorMsg( msg );
   };

   return result;
   }

////////////////////////////////////////////
//                                        //
//                Getters                 //
//                                        //
////////////////////////////////////////////
unsigned short ETEquation::GetMode()
   {
   return m_mode;
   }

bool ETEquation::GetUseTallRefCrop()
   {
   return m_useTallRefCrop;
   }

double ETEquation::GetDailyMeanTemperature()
   {
   return m_dailyMeanTemperature;
   }

double ETEquation::GetRelativeHumidity()
   {
   return m_relativeHumidity;
   }

double ETEquation::GetWindSpeed()
   {
   return m_windSpeed;
   }

double ETEquation::GetLAI()
   {
   return m_lai;
   }

double ETEquation::GetTreeHeight()
   {
   return m_treeHeight;
   }

double ETEquation::GetSolarRadiation()
   {
   return m_solarRadiation;
   }

double ETEquation::GetTwilightSolarRadiation()
   {
   return m_twilightSolarRadiation;
   }

double ETEquation::GetStationElevation()
   {
   return m_stationElevation;
   }

double ETEquation::GetStationLatitude()
   {
   return m_stationLatitude;
   }

double ETEquation::GetStationLongitude()
   {
   return m_stationLongitude;
   }

double ETEquation::GetTimeZoneLongitude()
   {
   return m_timeZoneLongitude;
   }

double ETEquation::GetDailyMaxTemperature()
   {
   return m_dailyMaxTemperature;
   }

double ETEquation::GetDailyMinTemperature()
   {
   return m_dailyMinTemperature;
   }

double ETEquation::GetRecentMeanTemperature()
   {
   return m_recentMeanTemperature;
   }

//double ETEquation::GetAgrimentStationCoeff(const unsigned int coeff)
//   {
//      double ret=NOVAL;
//      if(coeff < MAX_AGRIMET_COEFF)
//         ret=m_agrimetStationCoeffs[coeff];
//      else
//         {
//         //remove this block to have a bad index fail with NOVAL return only
//         CString msg; 
//         msg.Format( _T("ET Equation: Agrimet station coefficient out of bounds (%u passed, %u max)"),coeff,MAX_AGRIMET_COEFF-1);
//         Report::ErrorMsg( msg );
//         ASSERT( 0 );
//         }
//      return ret;
//   }

//unsigned int ETEquation::GetYear()
//   {
//   return m_year;
//   }
//
unsigned int ETEquation::GetMonth()
   {
   return m_month;
   }
//
//unsigned int ETEquation::GetDay()
//   {
//   return m_day;
//   }

unsigned int ETEquation::GetDoy()
   {
   return m_doy;
   }

//unsigned int ETEquation::GetHoursInDay()
//   {
//   return m_hoursInDay;
//   }

////////////////////////////////////////////
//                                        //
//                Setters                 //
//                                        //
////////////////////////////////////////////
void ETEquation::SetMode( unsigned short mode )
   {
   m_mode = mode;
   }

void ETEquation::SetUseTallRefCrop(bool utc)
   {
   m_useTallRefCrop = utc;
   }

//void ETEquation::SetAirTemperature( double at )
//   {
//   m_airTemperature = at;
//   }

void ETEquation::SetDailyMeanTemperature( double tMean )
   {
   m_dailyMeanTemperature = tMean;
   }

void ETEquation::SetRelativeHumidity( double rh )
   {
   m_relativeHumidity = rh;
   }

void ETEquation::SetSpecificHumidity( double sph )
   {
   m_specificHumidity = sph;
   }

void ETEquation::SetTreeHeight( double height )
   {
   m_treeHeight = height;
   }

void ETEquation::SetLAI( double lai )
   {
   m_lai = lai;
   }

void ETEquation::SetWindSpeed( double ws )
   {
   m_windSpeed = ws;
   }

void ETEquation::SetSolarRadiation(double sr)
{
   m_solarRadiation = sr;
}


void ETEquation::SetSWcoeff(double sw_coeff) { m_sw_coeff = (float)sw_coeff; }
void ETEquation::SetLWcoeff(double lw_coeff) { m_lw_coeff = (float)lw_coeff; }


void ETEquation::SetTwilightSolarRadiation( double tsr )
   {
   m_twilightSolarRadiation = tsr;
   }

void ETEquation::SetStationElevation( double se )
   {
   m_stationElevation = se;
   }

void ETEquation::SetStationLatitude( double sl )
   {
   m_stationLatitude = sl;
   }

void ETEquation::SetStationLongitude( double sl )
   {
   m_stationLongitude = sl;
   }

void ETEquation::SetTimeZoneLongitude( double tzl )
   {
   m_timeZoneLongitude = tzl;
   }

void ETEquation::SetDailyMaxTemperature(double tmp)
   {
   m_dailyMaxTemperature=tmp;
   }

void ETEquation::SetDailyMinTemperature(double tmp)
   {
   m_dailyMinTemperature=tmp;
   }

void ETEquation::SetRecentMeanTemperature(double tmp)
   {
   m_recentMeanTemperature=tmp;
   }

 void ETEquation::SetAgrimentStationCoeff( vector<double> agrimetStationCoeffs )
    {
    m_agrimetStationCoeffs = agrimetStationCoeffs;
    }

//void ETEquation::SetYear( unsigned int y )
//   {
//   m_year = y;
//   }
void ETEquation::SetDoy( unsigned int doy )
   {
   m_doy = doy <= 364 ? doy : 364; // Constrain the length of the year to 365 days.
   }


void ETEquation::SetMonth( unsigned int m )
   {
   m_month = m;
   }

//void ETEquation::SetDay( unsigned int d )
//   {
//   m_day = d;
//   }

//void ETEquation::SetDate( unsigned int d, unsigned int m, unsigned int y )
//   {
//   m_year = y;
//   m_month = m;
//   m_day = d;
//   }

//void ETEquation::SetHoursInDay( unsigned int hid )
//   {
//   m_hoursInDay = hid;
//   }

////////////////////////////////////////////
//                                        //
//       Equations for calculation        //
//                                        //
////////////////////////////////////////////


// FAO56  Based on "Evapotranspiration and Consumptive Irrigation Water Requirements for Idaho"
// Richard Allen and Clarence W. Robison 2007,  Appendix 1, pg 62
float ETEquation::Fao56()
   {  
   float result = -1.0;
   
   //run integ_check here
   if( m_doy != NOVAL &&
       m_dailyMinTemperature != NOVAL &&
       m_dailyMaxTemperature != NOVAL &&
       m_dailyMeanTemperature != NOVAL &&  
       m_specificHumidity != NOVAL &&
       m_windSpeed != NOVAL && 
       m_solarRadiation != NOVAL &&
       m_stationElevation != NOVAL &&
       m_stationLatitude != NOVAL &&
       m_stationLongitude != NOVAL )
  //     m_timeZoneLongitude != NOVAL ) 
      {

      // needed for hourly calculations
      /*const static double Cn_daytime_shortReference = 37;
      const static double Cn_nighttime_shortReference = 37;
      const static double Cn_daytime_tallReference = 66;
      const static double Cn_nighttime_tallReference = 66;
      const static double Cd_daytime_shortReference = 0.24;
      const static double Cd_nighttime_shortReference = 0.96;
      const static double Cd_daytime_tallReference = 0.25;
      const static double Cd_nighttime_tallReference = 1.7;*/


      
      const static double sigma = 4.901E-9;                                               // Stefan_Boltzmann : MJ K^-4 m^-2 d^-1     
      const static double lambda = 2.45;                                                  // Latent Heat of Vaporization : MJ/kg     
      const static double Gsc = 4.92;                                                     // Solar Constant : MJ/(h m^2)   
      const static double albedo = 0.23;                                                  // Albedo (canopy reflection coefficient) : dimensionless  

      double P = 101.3 * pow(((293.0 - 0.0065 * m_stationElevation) / 293.0), 5.26);     // Atmospheric Pressure, Eq. 1.2 : kPa, m
      double gamma = 0.000665 * P;                                                       // Psychrometic Constant : kPa/deg C

      // Slope of Saturation VaporPressure-Temperature Curve, Eq. 1.4 : kPa/deg C
      double slopeDelta = ( 2503.0 * exp ( ( 17.27 * m_dailyMeanTemperature ) / ( m_dailyMeanTemperature + 237.3 ) ) ) / pow( m_dailyMeanTemperature + 237.3, 2.0 );

      // Saturation Vapor Pressure : kPa
   /*   double esTmin = .6108 * exp( ( 17.27 * (double)m_dailyMinTemperature ) / ( 237.3 + (double)m_dailyMinTemperature ) );
      double esTmax = .6108 * exp( ( 17.27 * (double)m_dailyMaxTemperature ) / ( 237.3 + (double)m_dailyMaxTemperature ) );
      double es = ( esTmax + esTmin )/2.0;*/

      // Actual Water Vapor Pressure : kPa
      double ea = 0.0;
	  double vpd = 0.0;

      // Relative Humidity
   //   CalcRelHumidity( (float)m_specificHumidity, (float)m_dailyMinTemperature, (float)m_dailyMaxTemperature, (float)m_stationElevation, ea ); 
	  CalculateRelHumidity((float)m_specificHumidity, (float)m_dailyMeanTemperature, (float)m_dailyMaxTemperature, (float)m_stationElevation, ea, vpd);


      // Net Short-Wave Radiation, Eq 1.10 : MJ/(m^2 d)    
      double solarRadiation = (double)( m_solarRadiation*MJPERD_PER_W );
      double netShortWaveRad = ( 1.0 - albedo ) * solarRadiation ;
  
      // Set up calculation for Net Longwave Radiation
      // Station latitude : radians
      double phi = ( PI / 180.0 ) * (double)m_stationLatitude;

      // Julian doy of the year :Janaury 1 = 1; December 31 = 365
      int J =  m_doy + 1;

      // Seasonal correction for solar time (Sc)
      double b = 2.0 * PI * ( m_doy - 1.39 ) / 365.0;             // radians
      double Sc = 0.1645 * sin( 2.0 * b ) - 0.1255 * cos( b ) - 0.025 * sin( b );
   
      // Solar Declination : radians
      double radDelta = 0.409 * sin( ( 2.0 * PI) / 365.0 ) * (J - 1.39 );
   
      ////Solar time angle at midpoint of time period
      ////p40, eq55
      //double omega = (PI/12.0) * ( (m_hoursInDay * 0.06667*(m_stationLongitude - m_timeZoneLongitude) + Sc) - 12.0);
   
      ////Solar time angles at beginning and end of time period
      ////p40, eqs 53, 54
      ////Length of calculation period is assumed = 1 hour
      //double omega1 = omega - PI / 24.0;
      //double omega2 = omega + PI / 24.0;

      // Sunset Hour Angle
      //p42, eq59
      double omegas = acos( -1.0 * tan( phi ) * tan( radDelta ) );

      ////apply limits to hour angles
      ////p41, eq56
      //if(omega1 < -omegas)
      //   omega1 = -omegas;
      //else if(omega1 > omegas)
      //   omega1 = omegas;
   
      //if (omega2 < -omegas)
      //   omega2 = -omegas;
      //else if (omega2 > omegas)
      //   omega2 = omegas;

      //if (omega1 > omega2)
      //   omega1 = omega2;
   
      ////angle of the sun above the horizon at midpoint of calculation period; radians
      ////p43, eq62
      //double beta = asin(sin(phi) * sin(radDelta) + cos(phi) * cos(radDelta) * cos(omega));

       // Inverse relative distance factor for earth-sun : dimensionless
      double dr = 1.0 + 0.033 * cos( J * (2.0 * PI) / 365.0 );   
      
      // Extraterrestrial radiation, Eq. 1.14 : MJ/(m^2 d)
      double Ra = ( 24.0 / PI ) * Gsc * dr * ( omegas * sin( phi ) * sin( radDelta ) + cos( phi ) * cos( radDelta ) * sin( omegas ) );


      

      //   double relativeSolarRad;
      //if (abs(beta) < 0.3)
      //   {
      //   relativeSolarRad = m_twilightSolarRadiation / ((0.75 + 2e-5 * m_stationElevation) * Ra);
      //   }
      //else
      //   {
      //         //using modified version of Clear-sky solar radiation; MJ/(h m^2)
      //         //p37, eq47

      // Compute relative Solar Radiation based on angle of sun relative to horizon, Eq. 1.12 : dimensionless ??
      double relativeSolarRad = solarRadiation / ( ( 0.75 + 2.0E-5 * m_stationElevation ) * Ra );
   //      }

      // Enforce limits on ration of Rs/Rso
      if ( relativeSolarRad < 0.3 )
         relativeSolarRad = 0.3;
      else if ( relativeSolarRad > 1.0 )
         relativeSolarRad = 1.0;
      else if( isNan<double>( relativeSolarRad ))
         relativeSolarRad = 1.0;

      // Cloudiness function, Eq. 1.12 : dimensionless
      double fcd = 1.35 * relativeSolarRad - 0.35;

      // Net Long-Wave Radiation, Eq. 1.11 : MJ/(m^2 d)
      double tMinKelvin = m_dailyMinTemperature + 273.0;
      double tMaxKelvin = m_dailyMaxTemperature + 273.0;

      double netLongWaveRad = sigma * fcd * ( 0.34 - 0.14 * sqrt( ea ) ) * ( ( pow( tMinKelvin, 4.0 ) + pow( tMaxKelvin, 4.0 )  )/ 2.0 );

      // Net Radiation, Eq 1.9 : MJ/(m^2 d)
      double netRadiation = m_sw_coeff * netShortWaveRad - m_lw_coeff * netLongWaveRad;
      

      //Soil Heat Flux Density, Eq. 1.22 : MJ/(d m^2)                          //  For a daily timestep G = 0 (Zero net flux across a day)
      double G = 0.0;

      //if (netRadiation < 0.0) //nighttime
      //   {
      //   G = Rn * ((m_useTallRefCrop) ? 0.2 : 0.5);
      //   }
      //else //daytime
      //   {
      //   G = Rn * ((m_useTallRefCrop) ? 0.04 : 0.1);
      //   }

      //Wind Profile Relationship
      // Eq. 1.23
      //i'm going to assume that the anemometer is 2 meters above the ground
      //and not do eq67
      //select numberator and denominator constants 
      //based on what reference crop is being used
      //and night/day state
     
      double Cn = 1600;
      double Cd = 0.38;

      //if (netRadiation < 0.0) //nighttime
      //   {
      //   if (m_useTallRefCrop)
      //      {
      //      Cn = Cn_nighttime_tallReference;
      //      Cd = Cd_nighttime_tallReference;
      //      }
      //   else
      //      {
      //      Cn = Cn_nighttime_shortReference;
      //      Cd = Cd_nighttime_shortReference;
      //      }
      //   }
      //else //daytime
      //   {
      //   if (m_useTallRefCrop)
      //      {
      //      Cn = Cn_daytime_tallReference;
      //      Cd = Cd_daytime_tallReference;
      //      }
      //   else
      //      {
      //      Cn = Cn_daytime_shortReference;
      //      Cd = Cd_daytime_shortReference;
      //      }
      //   }
   
      // Standardized Reference Evapotranspiration : mm/day
      double ETsz = ( 0.408 * slopeDelta * ( netRadiation - G ) + gamma * ( Cn / ( m_dailyMeanTemperature + 273.0 ) ) * m_windSpeed * vpd )
                    / ( slopeDelta + gamma * ( 1.0 + Cd * m_windSpeed));
      result = (float)ETsz;
      
      }
   else ASSERT(false);

   return result;
   }


   float ETEquation::Hargreaves()
      {

     // int _month = 0;int _day;int _year;
      //BOOL ok = ::GetCalDate( m_doy, &_year, &_month, &_day, TRUE );
      int month = GetMonth();
      float etrc = -1.0f;
      float temp = (float)GetDailyMeanTemperature();
      //if(m_solarRadiation!=NOVAL && m_airTemperature != NOVAL)

      //float latitude = 17.0f/57.2957795f;//convertToRadians
      float latitude = 45.0f / 57.2957795f; //convertToRadians
      float solarDeclination= 0.4903f * (float)sin( 2.0 * PI / 365.0 * m_doy - 1.405 );

      float sd_degrees = solarDeclination * 57.2957795f ;
      float sunsetHourAngle = (float)acos( -1.0 * tan( latitude ) * tan( solarDeclination ) );
      float sHA_degress = sunsetHourAngle * 57.2957795f ;
      float N = 24.0f / PI * sunsetHourAngle;
      float dr = 1 + 0.033f * cos( 2 * PI / 365.f * m_doy );
      float So_mm_d = 15.392f * dr *( sunsetHourAngle * sin( latitude ) * sin( solarDeclination ) + cos( latitude ) * cos( solarDeclination ) * sin( sunsetHourAngle ) ); 
   
      int *monthlyTempDiff = new int[ 12 ];

      //mean monthly diuranal temp differences from SALEM OREGON
      monthlyTempDiff[0]=7;
      monthlyTempDiff[1]=9;
      monthlyTempDiff[2]=9;
      monthlyTempDiff[3]=12;
      monthlyTempDiff[4]=14;
      monthlyTempDiff[5]=14;
      monthlyTempDiff[6]=17;
      monthlyTempDiff[7]=17;
      monthlyTempDiff[8]=14;
      monthlyTempDiff[9]=13;
      monthlyTempDiff[10]=8;
      monthlyTempDiff[11]=7;

      //float etrc = 0.0023f * So_mm_d * sqrt((float)monthlyTempDiff[month-1]) * ( temp + 17.8f ) * 7.5f ; //gives reference crop et in mm / d*/
   
      etrc = 0.0023f * So_mm_d * sqrt( (float)monthlyTempDiff[ month-1 ] ) * ( temp + 17.8f ) * 0.750f; //gives reference crop et in mm / d*/
 
      delete [] monthlyTempDiff;

      //   etrc = 0.0023f * (m_solarRadiation*W_TO_MJ) * 4.0f * ( m_airTemperature + 17.8f ) ; //gives reference crop et in mm / d*/
      return etrc / 24.0f;
      }


   float ETEquation::PennMont( HRU *pHRU )
   {
   float result = -1.0;
   
   //run integ_check here
   if( m_dailyMeanTemperature != NOVAL &&
       m_dailyMinTemperature != NOVAL &&
       m_dailyMaxTemperature != NOVAL &&
       m_specificHumidity != NOVAL &&
       m_windSpeed != NOVAL && 
       m_solarRadiation != NOVAL &&
       m_stationElevation != NOVAL &&
       m_stationLatitude != NOVAL &&
       m_stationLongitude != NOVAL &&
  //     m_timeZoneLongitude != NOVAL &&
       m_doy != NOVAL )
   //    m_month != NOVAL )
   //    m_day != NOVAL &&
   //    m_year != NOVAL &&
  //     m_hoursInDay != NOVAL)
      {
     
      const static double sigma = 4.901E-9;                                               // Stefan_Boltzmann : MJ K^-4 m^-2 d^-1     
  //    const static double lambda = 2.45;                                                  // Latent Heat of Vaporization : MJ/kg     
      const static double Gsc = 4.92;                                                     // Solar Constant : MJ/(h m^2)   
      const static double albedo = 0.15;                                                  // Albedo (canopy reflection coefficient) : dimensionless
      //const static double albedo = 0.23;

     /* const static double Cn_daytime_shortReference = 37;
      const static double Cn_nighttime_shortReference = 37;
      const static double Cn_daytime_tallReference = 66;
      const static double Cn_nighttime_tallReference = 66;
      const static double Cd_daytime_shortReference = 0.24;
      const static double Cd_nighttime_shortReference = 0.96;
      const static double Cd_daytime_tallReference = 0.25;
      const static double Cd_nighttime_tallReference = 1.7;*/
     
      
      double P = 101.3 * pow( ( ( 293.0 - 0.0065 * m_stationElevation) / 293.0 ), 5.26 ); // Atmospheric Pressure : kPa

      double gamma = 0.000665 * P;                                                        // Psychrometic Constant : kPa/deg C
     
	

 //     double cp = 1.0131E-3;                                                            // specific heat of moist air : MJ/kg C
	  double cp = 1.006E-3;                                                              // specific heat of dry air : MJ/kg C


  //    double rho = (float) 3.486*P/(275.0 + m_airTemperature ) ;                        // density of air  C (kg/m3) Shuttleworth 4.2.4. 
    
      // rho = pressure/R*T   R = 287.058 J/kgK

      //  double cp = 1.0131; // specific heat of moist air ( kJ/kg degree C)

      // Latent Heat of Vaporization; MJ/kg
      double lambda = 2.501 - 2.361E-3 * m_dailyMeanTemperature;
      // lambda needed in kJ/kg instead of MJ/kg
   //   double gamma = cp*P/(0.622*lambda)/1000.; // kPa/C


      // Slope of Saturation VaporPressure-Temperature Curve: kPa/deg C
      // m_airTemperature = daily mean temperature
      double slopeDelta = 4098.0 * ( 0.6108 * exp( ( 17.27 * m_dailyMeanTemperature ) / (m_dailyMeanTemperature + 237.3) ) ) / pow( m_dailyMeanTemperature + 237.3, 2.0 );    

      // Saturation Vapor Pressure : kPa
   //   double esTmin = .6108 +  exp( ( 17.27 * (double)m_dailyMinTemperature ) / ( 237.3 + (double)m_dailyMinTemperature ) );
   //   double esTmax = .6108 +  exp( ( 17.27 * (double)m_dailyMaxTemperature ) / ( 237.3 + (double)m_dailyMaxTemperature ) );
   //   double es = ( esTmax + esTmin ) / 2.0;
       
      // Actual Water Vapor Pressure : kPa
      double ea = 0.0;
	  double vpd = 0.0;
	  CalculateRelHumidity((float)m_specificHumidity, (float)m_dailyMeanTemperature, (float)m_dailyMaxTemperature, (float)m_stationElevation, ea, vpd);

	  double virtualT = (273.0 + m_dailyMeanTemperature) / (1.0 - 0.378 * ea / P);
		    double rho = (float)1000.0 * P / (287.058 * (273.0 + m_dailyMeanTemperature));      // density of air : C (kg/m3) Shuttleworth 4.2.4. 
	//  double rho = (float) (1000.0 * P) / (287.058  * virtualT);      // density of air : C (kg/m3) Shuttleworth 4.2.4. 

      
      double solarRadiation = (double)(m_solarRadiation * MJPERD_PER_W);			// incoming radiation MJ/(m^2 d)	
	  // Net Short-Wave Radiation : MJ/(m^2 d)
      double netShortWaveRad = ( 1.0 - albedo ) * solarRadiation ;
  
      // latitude : radians
      double phi = (PI / 180.0 ) * (double)m_stationLatitude;

      double J = m_doy + 1;

      // seasonal correction for Solar Time (Sc)
  //    double b = 2.0 * PI * ( J - 1.39 ) / 365.0;
  //    double Sc = 0.1645 * sin( 2.0 * b ) - 0.1255 * cos( b ) - 0.025 * sin( b );
   
      // Solar Declination : radians
      double radDelta = 0.409 * sin(  ( 2.0 * PI  * J) / 365.0 - 1.39 );
	  double declination = 180 / PI * radDelta;
 
      //solar time angle at midpoint of time period
      //p40, eq55
   //   double omega = (PI/12.0) * ( (m_hoursInDay * 0.06667*(m_stationLongitude - m_timeZoneLongitude) + Sc) - 12.0);
   
      //solar time angles at beginning and end of time period
      //p40, eqs 53, 54
      //length of calculation period is assumed = 1 hour
  //    double omega1 = omega - PI / 24.0;
   //   double omega2 = omega + PI / 24.0;


      // Sunset Hour angle
      double omegas = acos( -1.0 * tan( phi ) * tan( radDelta ) );

      //apply limits to hour angles
      //p41, eq56
     /* if(omega1 < -omegas)
         omega1 = -omegas;
      else if(omega1 > omegas)
         omega1 = omegas;
   
      if (omega2 < -omegas)
         omega2 = -omegas;
      else if (omega2 > omegas)
         omega2 = omegas;

      if (omega1 > omega2)
         omega1 = omega2;*/

      //angle of the sun above the horizon at midpoint of calculation period; radians
      //p43, eq62
  //    double beta = asin(sin(phi) * sin(delta) + cos(phi) * cos(delta) * cos(omega));

      
      // inverse relative distance factor for earth-sun : dimensionless
      double dr = 1.0 + 0.033 * cos( J * ( 2.0 * PI ) / 365.0 );

      // Extraterrestrial radiation : MJ(d m^2)
      //p39, eq48
  //    double Ra = (12.0 / PI) * Gsc * dr * ((omega2 - omega1) * sin(phi) * sin(delta) + cos(phi) * cos(delta) * (sin(omega2) - sin(omega1)));
      double Ra = ( 24.0 / PI ) * Gsc * dr * ( omegas * sin( phi ) * sin( radDelta ) + cos( phi ) * cos( radDelta ) * sin( omegas ) );

      //compute relative solar radiation based on angle of sun relative to horizon
      //see p35
      //double relativeSolarRad;

      //if (abs(beta) < 0.3)
      //   {
      //   relativeSolarRad = m_twilightSolarRadiation / ((0.75 + 2e-5 * m_stationElevation) * Ra);
      //   }
      //else
      //   {
      //   //using modified version of Clear-sky solar radiation; MJ/(h m^2)
      //   //p37, eq47
         double relativeSolarRad = solarRadiation / ( ( 0.75 + 2.0E-5 * m_stationElevation ) * Ra );
    //     }

      //enforce limits on ration of Rs/Rso
      //p35
      if ( relativeSolarRad < 0.3 )
         relativeSolarRad = 0.3;
      else if ( relativeSolarRad > 1.0 )
         relativeSolarRad = 1.0;
      else if( isNan<double>( relativeSolarRad ) )
         relativeSolarRad = 1.0;

      //cloudiness function
      //p34, eq45
      //this must be adjusted
      double fcd = 1.35 * relativeSolarRad - 0.35;


 
      //Net Long-Wave Radiation; MJ/(m^2 d)
      //p34, eq44

      ////double fcd=1.f;
      //double Rnl = sigma * fcd * (0.34 - 0.14 * sqrt(ea)) * pow(m_airTemperature, 4);
    

      ////Net Radiation; MJ/(m^2 d)
      ////p32, eq42
      //double Rn = Rns - Rnl;

       //Net Long-Wave Radiation, Eq. 1.11 : MJ/(m^2 d)

      double tMinKelvin = m_dailyMinTemperature + 273;
      double tMaxKelvin = m_dailyMaxTemperature + 273;

      double netLongWaveRad = sigma * fcd * ( 0.34 - 0.14 * sqrt( ea ) ) * ( ( pow( tMinKelvin, 4.0 ) + pow( tMaxKelvin, 4.0 ) ) / 2.0 );

      //Net Radiation; MJ/(m^2 d)
      // Eq 1.9

      double netRadiation = netShortWaveRad - netLongWaveRad;


      //Soil Heat Flux Density : MJ/(d m^2)
      //p44, eqs65-66
      double G = 0.0;

      //if (Rn < 0.0) //nighttime
      //   {
      //   G = Rn * ((m_useTallRefCrop) ? 0.2 : 0.5);
      //   }
      //else //daytime
      //   {
      //   G = Rn * ((m_useTallRefCrop) ? 0.04 : 0.1);
      //   }

      int count = (int)pHRU->m_polyIndexArray.GetSize();

      for ( int i = 0; i < count; i++ )
         {
         int idu = pHRU->m_polyIndexArray[ i ];

         bool processIDU = true;
         if ( m_pEvapTrans->m_pQuery != NULL )
            {
            m_pEvapTrans->m_pQuery->Run( idu, processIDU );
            if ( processIDU )
               {
               int lulc_a = 0;  m_pEvapTrans->m_flowContext->pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[i], m_pEvapTrans->m_colLULC_A, lulc_a);

               float lai = 1.0f; m_pEvapTrans->m_flowContext->pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[i], m_pEvapTrans->m_colLAI, lai);
               float height = 10.f;
               if (lulc_a == 4)
                  { // LULC_A == {forest}
                  int pvt = 0;  m_pEvapTrans->m_flowContext->pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[i], m_pEvapTrans->m_colPVT, pvt);
                  if (pvt > 0)
                     {
                     float age = 1.0f; m_pEvapTrans->m_flowContext->pEnvContext->pMapLayer->GetData(pHRU->m_polyIndexArray[i], m_pEvapTrans->m_colAgeClass, age);
                     height = (float)(46.6 * (1.0 - exp(-0.0144 * age)));
                     }
                  else height = 10.f;
                  }

               // Surface Resistance rs
               lai /= 2;     // Active leaf area
               if (lai < 0.1f) lai = 0.1f;
               float rs = m_pEvapTrans->m_effBulkStomatalResistance / lai;  // : s/m.  This is a standard leaf resistance for forest canopies (Shuttleworth and Wallace, 1985).

               //Aerodynamic Resistance
               if ( alglib::fp_eq( height, 0.0 ) )
                  height = 0.1f;

               float d = 2.0f / 3.0f * height;                    // 0 plane displacement height
               float zom = 0.123f * height;                       // roughness length governing momentum transfer
               float zoh = 0.0123f * height;                      // roughness length governing transfer of head and vapor
               float zm = height + 2;                             // height of wind measurement
               float zh = height + 2;                             // height of humidity measurement
               float ra = 1.0f;

               if ( m_windSpeed > 0 && zm > d )
                  ra = (float)( log( ( zm - d ) / zom ) * log( ( zh-d ) / zoh ) / 0.41 * 0.41 * m_windSpeed );
               else
                  ra = 1.0f;
   
               //P-M Evapotranspiration; mm/day
               double numerator = slopeDelta * ( netRadiation - G ) + ( rho * cp * vpd / ra ) * SEC_PER_DAY;
               double denominator = 2453.0 * ( slopeDelta + gamma * ( 1.0 + rs / ra ) );
               double ETsz =  ( numerator / denominator ) * MM_PER_M;
			//   double denominator = lambda * (slopeDelta + gamma * (1.0 + rs / ra));
			//   double ETsz = (numerator / denominator);

               result = (float)ETsz;
               
               if (result < 0.0f) 
				   result = 0.0f;

               GlobalMethod::m_iduIrrRequestArray[ pHRU->m_polyIndexArray[ i ] ] = result;
               GlobalMethod::m_iduVPDarray[ pHRU->m_polyIndexArray[ i ] ] = (float)vpd;

  //             ASSERT(result>=0.0f);
               }
            }
         }
      }

   return result;
   }

float ETEquation::KimbPenn()
   {
   // Source: http://www.usbr.gov/pn/agrimet/aginfo/AgriMet%20Kimberly%20Penman%20Equation.pdf

   float result = -1.0;

   //number of coefficients expected from an agriment station (for calculating clear-day shortwave radiation)
   const unsigned int STATION_COEFF_COUNT = 5;
   bool allCoeffs = true;
  /* for(unsigned int i=0; i<STATION_COEFF_COUNT && allCoeffs;i++)
      allCoeffs=m_agrimetStationCoeffs[i]==NOVAL;*/

   if(!m_agrimetStationCoeffs.empty() &&
      m_stationElevation!=NOVAL &&
      m_solarRadiation!=NOVAL &&
      m_dailyMaxTemperature!=NOVAL &&
      m_dailyMinTemperature!=NOVAL &&
      m_recentMeanTemperature!=NOVAL &&
      m_specificHumidity != NOVAL &&
      m_relativeHumidity!=NOVAL &&
      m_dailyMeanTemperature != NOVAL &&
      m_windSpeed!=NOVAL)
      {
      //Stefan_Boltzmann constant
      const static double sigma = 4.901E-9; 

      //specific heat of air at constant pressure
      const double C_P = 0.001005;

      //mean air temperature (C)
  //    double meanTemperature=(m_dailyMaxTemperature+m_dailyMinTemperature)/2.0;

      //slope of saturation vapor pressure - temperature curve
  //    double delta=0.200*pow(0.00738*meanTemperature+0.8072,7.0)-0.000116;;
      double slopeDelta = 0.200 * pow( 0.00738 * (double)m_dailyMeanTemperature + 0.8072, 7.0 ) - 0.000116;

      //Psychochromatic constant (gamma)
      // -- Latent head of vaporization of water
      double lambda = 2.501 - 0.002361 * m_dailyMeanTemperature;
      // -- Estimated mean atmospheric pressure
      double P = 101.3 * pow( ( ( 288.0 - 0.0065 * (double)m_stationElevation ) / 288.0 ), 5.257 );

      double gamma = ( C_P * P ) / ( 0.622 * lambda );

      double factor1 = slopeDelta / ( slopeDelta + gamma );
      double factor2 = 1.0 - factor1;

      //Net radiation energy (Rn)
      // -- albedo
      double alpha = 0.29 + 0.06 * sin( m_doy + 97.92 );

      // -- clear day shortwave radiation
      double Rs0 = 0.0;
      int stationCount = (int)m_agrimetStationCoeffs.size();
      for ( int i = 0; i < stationCount; i++ )
         Rs0 += m_agrimetStationCoeffs.at( i ) * pow( (double)m_doy, (double)i );
      // -- Rs/Rso

      Rs0 = Rs0 * 0.041868; // convert to MJ/m^2d

      double solarRadiation = ( m_solarRadiation*MJPERD_PER_W );
      double radRatio=solarRadiation / Rs0;
      // -- imperical constants (a,b,a1,b1)
      double a = 1.0;
      double b = -.01;

      if( radRatio > 0.70 )
         {
         a = 1.126;
         b = -0.07;
         }
      else if ( radRatio <= 0.70 )
         {
         a = 1.017;
         b = -0.06;
         }

      double a1 = 0.26 + 0.1 * exp( -1.0 * pow( 0.0154 * (double)( m_doy-177 ), 2.0 ) );
      const double b1 = -0.139;

	//	double dewTemperature=CalcDewTemp(m_relativeHumidity, m_dailyMeanTemperature);
      double ea = 0.0;
      CalcRelHumidityKP( (float)m_specificHumidity, (float)m_dailyMinTemperature, (float)m_dailyMaxTemperature, (float)m_stationElevation, ea );
      // -- saturation vapor pressure at mean daily dewpoint temperature
    //  double ea=3.38639*(pow(0.00738*dewTemperature+0.8072,8)-0.000019*(1.8*dewTemperature+48.0)+0.001316);
      
      // -- Theoretical outgoing longwave radiation
      double Rb0 = ( a1 + b1 * sqrt( ABS( ea ) ) ) * sigma * ( ( pow( m_dailyMaxTemperature + 273.15, 4.0 ) + pow( m_dailyMinTemperature + 273.15, 4.0 ) ) / 2.0 );
      // -- outgoing longwave radiation
      double Rb = Rb0 * ( a * radRatio + b );
      // netRadiation
      double Rn = ( 1.0 - alpha ) * solarRadiation - Rb;

      //Soil Heat flux (G)
      double G = 0.377 * ( (double)m_dailyMeanTemperature - (double)m_recentMeanTemperature );
   //   G=0.0;

      //Advective Energy Transfer (Wf)
      // -- empirical constants (aw,bw)
      double aw = 0.4 + 1.4 * exp( -1.0 * pow( ( m_doy-173 ) / 58.0, 2.0 ) );
      double bw = 0.007 + 0.004 * exp( -1.0 * pow( ( m_doy - 243 ) / 80.0, 2.0 ) );
      double Wf = aw + bw * (double)m_windSpeed * KMPERDAY_PER_MPERSEC; // windspeed needs to be converted from m/sec to km/d for Kimberly-Pennman 

      //Vapor Pressure Deficit (es)
      // -- saturation vapor pressure at max temperature
      double eTmax = 3.38639 * ( pow( 0.00738 * m_dailyMaxTemperature + 0.8072, 8.0 ) - 0.000019 * ABS( 1.8 * m_dailyMaxTemperature + 48.0 ) + 0.001316 );
      // -- saturation vapor pressure at min temperature
      double eTmin = 3.38639 * ( pow( 0.00738 * m_dailyMinTemperature + 0.8072, 8.0 ) - 0.000019 * ABS( 1.8 * m_dailyMinTemperature + 48.0 ) + 0.001316 );
      double es = ( eTmax + eTmin ) / 2.0;

		if (ea > es) es = ea;

      //ET equation
      result = (float) ( ( factor1 * ( Rn-G ) + factor2 * 6.43 * Wf * ( es-ea ) ) / lambda); //mm/d    
      }
   return result;
   }

   float ETEquation::Asce()
   { 
	   float result = -1.0;

	   if (m_doy != NOVAL &&
		   m_dailyMinTemperature != NOVAL &&
		   m_dailyMaxTemperature != NOVAL &&
		   m_dailyMeanTemperature != NOVAL &&
		   m_specificHumidity != NOVAL &&
		   m_windSpeed != NOVAL &&
		   m_solarRadiation != NOVAL &&
		   m_stationElevation != NOVAL &&
		   m_stationLatitude != NOVAL &&
		   m_stationLongitude != NOVAL)
	   {
		   // Stefan_Boltzmann : MJ K^-4 m^-2 d^-1
		   const static double sigma = 2.042E-10;

		   // solar constant; MJ/(h m^2)
		   const static double Gsc = 4.92;

		   // -- Latent head of vaporization of water
		   const double lambda = 2.45;	

		   // Albedo (canopy reflection coefficient); dimensionless
		   const static double albedo = 0.23;

		   // -- Estimated mean atmospheric pressure
		   double P = 101.3 * pow( ( ( 293.0 - 0.0065 * m_stationElevation ) / 293.0 ), 5.26 );

		   //Psychrometic Constant: kPa/deg C
		   double gamma = 0.000665 * P;

		   //Slope of Saturation VaporPressure-Temperature Curve: kPa/deg C
		   //Eq. 1.4
		   // m_airTemperature = daily mean temperature
		   double slopeDelta = ( 2503.0 * exp( ( 17.27 * m_dailyMeanTemperature ) / ( m_dailyMeanTemperature + 237.3 ) ) ) / pow( m_dailyMeanTemperature + 237.3, 2.0 );

		   //Saturation Vapor Pressure; kPa
		   double esTmin = .6108 * exp( ( 17.27 * m_dailyMinTemperature ) / ( 237.3 + m_dailyMinTemperature ) );
		   double esTmax = .6108 * exp( ( 17.27 * m_dailyMaxTemperature ) / ( 237.3 + m_dailyMaxTemperature ) );
		   double es = ( esTmax + esTmin ) / 2.0;

		   //Actual Water Vapor Pressure; kPa
		   double ea = 0.0;
		   CalcRelHumidity( (float)m_specificHumidity, (float)m_dailyMinTemperature, (float)m_dailyMaxTemperature, (float)m_stationElevation, ea );

		   //Net Short-Wave Radiation; MJ/(m^2 d)
		   // Eq 1.10

		   double solarRadiation = (double)( m_solarRadiation * MJPERD_PER_W );
		   double netShortWaveRad = ( 1.0 - albedo ) * solarRadiation;

		   // Set up calculation for Net LongWave Radiation
		   //latitude; radians
		   double phi = ( PI / 180.0 ) * (double)m_stationLatitude;

		   int J = m_doy + 1;

		   //seasonal correction for solar time (Sc)
		   //p42, eqs 57, 58 respectively
		   double b = 2.0 * PI * ( J - 1.39 ) / 365.0;
		   double Sc = 0.1645 * sin( 2 * b ) - 0.1255 * cos( b ) - 0.025 * sin( b );

		   //solar declination; radians
		   //p39, eq51
		   double radDelta = 0.409 * sin( ( ( 2 * PI ) / 365.0 ) * J - 1.39 );

		   //sunset hour angle
		   //p42, eq59
		   double omegas = acos( -1 * tan( phi ) * tan( radDelta ) );

		   //inverse relative distance factor for earth-sun; unitless
		   double dr = 1.0 + 0.033 * cos( J * ( 2.0 * PI ) / 365.0 );

		   //Extraterrestrial radiation; MJ/(m^2 d)
		   // Eq. 1.14
		   double Ra = ( 24.0 / PI ) * Gsc * dr * ( omegas * sin( phi ) * sin( radDelta ) + cos( phi ) * cos( radDelta ) * sin( omegas ) );

		   //compute relative solar radiation based on angle of sun relative to horizon
		   //Eq. 1.12

		   //   double relativeSolarRad;

		   //if (abs(beta) < 0.3)
		   //   {
		   //   relativeSolarRad = m_twilightSolarRadiation / ((0.75 + 2e-5 * m_stationElevation) * Ra);
		   //   }
		   //else
		   //   {
		   //         //using modified version of Clear-sky solar radiation; MJ/(h m^2)
		   //         //p37, eq47
		   double relativeSolarRad = solarRadiation / ( ( 0.75 + 2.0E-5 * m_stationElevation ) * Ra );
		   //      }

		   //enforce limits on ration of Rs/Rso
		   // 
		   if ( relativeSolarRad < 0.3 )
			   relativeSolarRad = 0.3;
		   else if ( relativeSolarRad > 1.0 )
			   relativeSolarRad = 1.0;
		   else if ( isNan<double>( relativeSolarRad ) )
			   relativeSolarRad = 1.0;

		   //cloudiness function
		   //Eq. 1.12
		   //this must be adjusted
		   double fcd = 1.35 * relativeSolarRad - 0.35;

		   //Net Long-Wave Radiation; MJ/(m^2 d)
		   // Eq. 1.11
		   //Net Long-Wave Radiation; MJ/(m^2 d)
		   // Eq. 1.11

		   double tMeanKelvin = m_dailyMeanTemperature+ 273.16;

		   double netLongWaveRad = sigma * fcd * ( 0.34 - 0.14 * sqrt( ea ) ) * pow( tMeanKelvin, 4.0 );

		   //Net Radiation; MJ/(m^2 d)
		   // Eq 1.9

		   double netRadiation = netShortWaveRad - netLongWaveRad;

		   double u2 = m_windSpeed * 4.87 / ( log( 67.8 * 2.0 - 5.42 ) );

		   //Soil Heat Flux Density; MJ/(d m^2)
		   // Eq. 1.22 
		   // daily timestep G = 0
		   double G = 0.0;

		   //select numerator and denominator constants 
		   //based on what reference crop is being used
		   //and night/day state
		   double Cn = 1600;
		   double Cd = 0.38;

		   //ET equation
		   result  = float( ( 0.408 * slopeDelta * ( netRadiation - G ) + gamma * ( Cn / ( m_dailyMeanTemperature + 273.0 ) ) * m_windSpeed * ( es - ea ) )
			   / (slopeDelta + gamma * ( 1.0 + Cd * u2 ) ) );   //mm/day   
	   }
	   return result;
   }

/** Method for calculating the Dew Point temperature based on the Clausius-Clapeyron equation 
   \param relHum The relative humidity of the site, represented as a value in range [0,100].
   \param temp The temperature of the region in degrees Celsius.
   \return The approximate dew point temperature in degrees Celsius.
*/
double ETEquation::CalcDewTemp(const double & relHum, double temp/* C */)
   {
   //-- calculate dew temp using Clausius-Clapeyron equation: http://iridl.ldeo.columbia.edu/dochelp/QA/Basic/dewpoint.html

   const double E0 = 0.611; //kPa
   const double LRV = 5423.0; //K
   const double T0 = 273.0; //K
   
   //from C to K
   temp += 273.15;

   const double Es = E0 * exp( LRV*( ( 1.0 / T0 ) - ( 1/temp ) ) );
   const double E = relHum * Es;
   
   const double Td = -1.0 / ( ( log( E/E0 ) * ( 1.0 / LRV ) ) - ( 1.0 / T0 ) );

   //K to C
   return ( Td - 273.15 );
   }

double ETEquation::CalcRelHumidity(float specHumid, float tMin, float tMax, float elevation, double &ea)
   {
   // Atmospheric Pressure; kPa
   double P = 101.3 * pow( ( ( 293.0 - 0.0065 * elevation) / 293.0 ), 5.26 );

   // P is barometric pressure at elevation
   // ea is actual water vapor pressure
   // es is saturated water vapor pressure
   // relative humidity is ratio of ea/es

  // ea = P / ( ( 0.622 / specHumid ) + 1.0 - 0.622 );

   ea = specHumid * 101.3 / (0.378 * specHumid + 0.622);


   double esTmin = .6108 * exp( ( 17.27 *  tMin ) / ( 237.3 +  tMin ) );
   double esTmax = .6108 * exp( ( 17.27 *  tMax ) / ( 237.3 +  tMax ) );

   double es = ( esTmax + esTmin ) / 2.0;
   double rh = ea / es;

	if ( ea >= es ) return 1.0;

   return rh;
   }


double ETEquation::CalculateRelHumidity(float specHumid, float meanTemp, float tMax, float elevation, double &ea, double &vpd)
{
	// Atmospheric Pressure; kPa
	double P = 101.3 * pow(((293.0 - 0.0065 * elevation) / 293.0), 5.26);

	// P is barometric pressure at elevation
	// ea is actual water vapor pressure
	// es is saturated water vapor pressure
	// relative humidity is ratio of ea/es

	ea = P / ((0.622 / specHumid) + 1.0 - 0.622);

	ea = specHumid * P / (0.378 * specHumid + 0.622);

	//  double es = .6112 * exp( ( 17.67 * meanTemp ) / ( meanTemp + 243.5 ) );
	//      e <-qair * press / (0.378 * qair + 0.622)

	double es = .61275 * exp((17.37 * meanTemp) / (meanTemp + 237.3));

	//double esTmin = .6108 * exp( ( 17.27 *  tMin ) / ( 237.3 + tMin ) );
	//double esTmax = .6108 * exp( ( 17.27 *  tMax ) / ( 237.3 + tMax ) );

	//double es = ( esTmax + esTmin ) / 2.0;

	vpd =  ( es > ea ) ? es - ea : 0.;

	double rh = ea / es;

	if (rh > 1.0) rh = 1.0;
	if (rh < 0.0) rh = 0.0;
	//     if ( ea >= es ) return 1.0;

	return rh;
}


double ETEquation::CalcRelHumidityKP(float specHumid, float tMin, float tMax, float elevation, double &ea)
   {
   // Atmospheric Pressure; kPa
   double P = 101.3 * pow( ( ( 288.0 - 0.0065 * elevation) / 288.0 ), 5.257 );

   // P is barometric pressure at elevation
   // ea is actual water vapor pressure
   // es is saturated water vapor pressure
   // relative humidity is ratio of ea/es

   ea = P / ( ( 0.622 / specHumid ) + 1.0 - 0.622 );

   double esTmax = 3.38639 * ( pow( 0.00738 * tMax + 0.8072, 8.0 ) - 0.000019 * ABS( 1.8 * tMax + 48.0 ) + 0.001316 );
   double esTmin = 3.38639 * ( pow( 0.00738 * tMin + 0.8072, 8.0 ) - 0.000019 * ABS( 1.8 * tMin + 48.0 ) + 0.001316 );

   double es = ( esTmax + esTmin ) / 2.0;
   double rh = ea / es;
 
	if (ea >= es) return 1.0;

   return rh;
   }