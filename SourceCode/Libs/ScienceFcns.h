/*
 *  ScienceFcns.h
 *
 */

#ifndef SCIENCEFCNS_H
#define SCIENCEFCNS_H

#include "assert.h"
#include <UNITCONV.H>

#ifdef UNIX_MAPSS
  #ifndef Boolean
    #define Boolean int
  #endif
#endif

#ifndef FALSE
   #define FALSE false
#endif

#ifndef TRUE
   #define TRUE true
#endif

# ifndef MIN
#  define MIN(x, y)	(((x) < (y)) ? (x) : (y))
# endif				/* MIN */

# ifndef MAX
#  define MAX(x, y)	(((x) > (y)) ? (x) : (y))
# endif				/* MAX */

#define PI 3.141592654
 
#define SEA_LEVEL 1.013246e5 /* standard sea level pressure (Pa) */
#define GRAV 9.80665 /* gravitational acceleration at reference latitude 45d 32m 33s */
#define MOL_AIR 28.9644 /* molecular weight of air (g mole-1) */
#define MOL_H2O         18.0153 /* molecular weight of water vapor (kg / kmole) */
#define	TB 288.0
#define	STDLR	-0.0065
#define RGAS            8.31432e3 /* gas constant (J / kmole / deg) */
#define AH		1.0	/* ratio sensible/momentum phi func	*/
#define AV		1.0	/* ratio latent/momentum phi func	*/
#define CP_AIR 1.005e3 /* specific heat of air at constant pressure (J / kg / deg) */
#define DALR ( GRAV / CP_AIR ) /*  dry adiabatic lapse rate (deg / m) */
#define FREEZE 2.7316e2 /* freezing point of water at standard pressure (deg K) */
#define BOIL   3.7315e2 /* boiling point of water at standard pressure (deg K) */
#define VON_KARMAN 3.5e-1 /* Von Karman constant */
#define PAESCHKE	7.35	/* Paeschke's const	*/
#define THRESH		1.e-5	/* convergence threshold		*/
#define ITMAX		50	/* max # iterations allowed		*/
#define SECONDS_PER_DAY		(60.0f * 60.0f * 24.0f)	/* SEC * MIN * HRS */
#define BETA_S		5.2
#define BETA_U		16
#define DAYS_PER_YEAR 365
#define DAYS_PER_MONTH {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}

# define JAN			0
# define FEB			1
# define MAR			2
# define APR			3
# define MAY			4
# define JUN			5
# define JUL			6
# define AUG			7
# define SEP			8
# define OCT			9
# define NOV			10
# define DEC			11
#define MONTHS 12
#define MONTH_BEGIN     	0
#define MONTH_END		1

# define NEXT_MO(_mo)			((_mo + 1) % MONTHS)
# define PREV_MO(_mo)			((_mo + DEC) % MONTHS)

#define SM		0 /* psi function code: momentum */
#define SH		1 /* psi function code: sensible heat flux */
#define SV		2 /* psi function code: latent heat flux */

# define mapssBOREAL			0
# define mapssTEMPERATE		1
# define mapssSUBTROPICAL	        2
# define mapssTROPICAL		3


/*
 *  integral of hydrostatic equation over layer with linear temperature
 *  variation
 *
 *      pb = base level pressure, tb = base level temp (K),
 *      L = lapse rate (deg/km), h = layer thickness (km),
 *      g = grav accel (m/s^2), m = molec wt (kg/kmole),
 *
 *      (the factors 1.e-3 and 1.e3 are for units conversion)
 */
#define HYSTAT(pb,tb,L,h,g,m)           ((pb) * (((L)==0.) ?\
                exp(-(g)*(m)*(h)*1.e3/(RGAS*(tb))) :\
                pow((tb)/((tb)+(L)*(h)),(g)*(m)/(RGAS*(L)*1.e-3))))
/*
 *  specific humidity, as function of e & P
 *      e = vapor pressure, P = pressure (same units)
 */
#define SPEC_HUM(e,P)           ((e)*MOL_H2O/(MOL_AIR*(P)+(e)*(MOL_H2O-MOL_AIR)))

/*
 *  give density of a gas (kg/m^3) as a function of mol. wt, pressure, and temperature
 *
 *      p = pressure (Pa), m = molecular weight (kg/kmole),
 *      t = temperature (K), rho = density (kg/m^3)
 */
#define GAS_DEN(p,m,t)          ((p)*(m)/(RGAS*(t)))

/*
 *  virtual temperature, i.e. the fictitious temperature that air must have at
 *  pressure p to have the same density, as a water vapor-air mixture at
 *  pressure P, temperature t, and vapor pressure e
 *
 *      t = temperature (K),
 *      e = vapor pressure, P = pressure (e and P in same units),
 */
#define VIRT(t,e,P)             ((t)/(1.-(1.-MOL_H2O/MOL_AIR)*((e)/(P))))

/*
 *  latent heat of vaporization at temperature t (deg K)
 */
#define LH_VAP(t)               (2.5e6 - 2.95573e3 *((t) - FREEZE))

/*
 *  latent heat of fusion at temperature t (deg K)
 */
#define LH_FUS(t)               (3.336e5 + 1.6667e2 * (FREEZE - (t)))


typedef enum {UNKNOWNzone = 0, ARCTICzone, BOREALzone, TEMPERATEzone, SUBTROPICALzone, TROPICALzone} ClimateZone;


class ScienceFcns
{ // functions which communicate only thru the calling sequence
   int days_per_mo[MONTHS];
   int month_days[MONTHS][2];

public:
    ScienceFcns();
    ~ScienceFcns() {};
    
    float efold(float tau, float raw, float previous);
    void FindSlopeYIntercept(float X1, float Y1, float X2, float Y2, float * slopeP, float * y_interceptP); // Find the slope and y-intercept 
        // of a line passing between 2 points.
    float C3prodPct(float meantmp[]); // Estimate C3 production as a % of total production, from monthly air temps, 
        // without weighting by month-to-month LAIs    
    float normalizedC3prod(float soil_tmp);
    float normalizedC4prod(float soil_tmp);
    float estimate_max_soil_tmp(float meantmp[MONTHS]);
    float DewpointTemperature(float satvp);
    float FindRatio(float x, float slope, float y_intercept);
    float GrowingDegreeDays(float tmp[], float base_temp);
    float GrowingDegreeDays(float tmp[], float base_temp, bool SouthernHemisphereFlag);
    float annual_max(float monthly_vals[]);
    float annual_min(float monthly_vals[]);
    float annual_average(float monthly_vals[], bool shift_flag);
    float annual_sum(float monthly_vals[], bool shift_flag);
    bool close_enough(double a, double b, double tolerance);
    bool close_enough(double a, double b, double tol_ratio, double tol_diff);
    void MixIndex(float ppt[], float tmp[], float p_hi_mult, float * tmp_indexP, float * ppt_indexP, float * mix_indexP);
    float FrostIndex(float tmp[]);
    int ClimateZone4MAPSS(float min_tmp, float n_decid_bound, float s_decid_bound, float frost);
    ClimateZone ClimateZone4Biogeography(float gdd_zero, float min_tmp, float az_thres, float bz_thres, float tz_thres, float stz_thres);
    void PhytomorphologicalIndices(float tmp_index, float ppt_index, float * evergP, float * needlP);
    void allometry_DavidKing(float abovegr_live_wood, bool db_flag, float * tree_htP, float * dbhP);
    void live_wood(float dbh, bool deciduous, float lwood, float * lbranchP, float * lstemP);
    int doy_of_dom0(int tgt_month, bool southernHemiFlag);
    float clip(float val, float minval, float maxval);
    float crown_kill(float ht, float cl, float hk);


   // unit conversions
   static float CtoF(float degC) { return(1.8f*degC + 32.0f); }
   static float FtoC(float degF) { return((degF - 32.f)*5.f/9.f); }
   static float mm_to_in(float mm) { return(mm*0.0393700787f); }
   static float in_to_mm(float inches) { return(inches/0.0393700787f); }
   static float ft_to_m(float ft) { return(ft*0.3048f); }
   static float ft3_to_m3(float ft3) { return(ft3 * 0.02831685f); }
   static float m_to_ft(float m) { return(m*3.28084f); }
   static float btu_per_sec_per_ft_to_kW_per_m(float btu_per_sec_per_ft);
   static float g_per_m2_to_tons_per_acre(float g_per_m2) { return(g_per_m2*.004460897f); }
   static float lbs_per_ac_to_g_per_m2(float lbs_per_ac) { return(lbs_per_ac*0.112f); }
   static float cm_to_in(float cm) { return(cm*0.393700787f); }
   static float kJ_per_day_to_W(float kJ_per_day) { return(kJ_per_day/86.4f); }
   static float m_per_sec_to_mph(float m_per_sec) { return(2.23694f*m_per_sec); }
   static float degC_to_degK(float degC) { return(degC + 273.15f); }
   static float degK_to_degC(float degK) { return(degK - 273.15f); }
   static float g_to_lbs(float gram) { return(gram * 0.00220462f); }

   static float ScienceFcns::bed_depth(float tot_fuel_bed_bio_gDM_per_m2, float depth_ratio)
      // tot_fuel_bed_bio is in g DM m-2
      // returns fuel_depth in meters
   {
      float fuel_depth_ft, fuel_depth_m;

      if (tot_fuel_bed_bio_gDM_per_m2 > 10500.) tot_fuel_bed_bio_gDM_per_m2 = 10500.;
      float tot_fuel_bed_bio_T_per_ac = g_per_m2_to_tons_per_acre(tot_fuel_bed_bio_gDM_per_m2); // .0044409f; // convert to tons/acre 

      fuel_depth_ft = tot_fuel_bed_bio_T_per_ac * depth_ratio;
      if (fuel_depth_ft > 2.0) fuel_depth_ft = 2.0;

      fuel_depth_m = ft_to_m(fuel_depth_ft); // convert depth from ft to meters
      return(fuel_depth_m);

   } // end of ScienceFcns::bed_depth()

  /*
   *
   * ----------------------------  sati
   *
   */
   static double ScienceFcns::sati(double tk)
   {
      double  l10;
      double  x;
      /*
      if (tk <= 0.) {
      fprintf(stderr,"SATI: temp = %g\n", tk);
      return(0.);
      }
      */
      assert(tk>0.);

      if (tk > FREEZE) {
         x = satw(tk);
         return(x);
      }

      errno = 0;
      l10 = log(1.e1);

      x = pow(1.e1, -9.09718*((FREEZE / tk) - 1.) - 3.56654*log(FREEZE / tk) / l10 +
         8.76793e-1*(1. - (tk / FREEZE)) + log(6.1071) / l10);

      if (errno) {
         /* syserr(); */
         fprintf(stderr, "SATI: bad return from log or pow\n");
      }

      return(x*1.e2);
   }
   
   
   /*
   ********************************************************************************
   **
   **  NAME
   **      satw, sati - saturation vapor pressure over water and ice
   **
   **  SYNOPSIS
   **      double  satw(tk)
   **      double  tk;
   **
   **      double  sati(tk)
   **      double  tk;
   **
   **  DESCRIPTION
   **      Satw returns saturation vapor pressure over water (in Pa)
   **      as a function of temperature (degrees K).
   **
   **      Sati returns saturation vapor pressure over ice (in Pa)
   **      as a function of temperature (degrees K).
   **
   **  DIAGNOSTICS
   **      Calls syserr() and/or usrerr()
   **
   **  HISTORY
   **      July 1982: written by J. Dozier, Department of Geography, UCSB
   **
   **
   */

   /*
   *
   * ----------------------------  satw
   *
   */
   static double ScienceFcns::satw(double tk)
   {
      double  x, l10;

      ASSERT(tk>0.);
      errno = 0;

      l10 = log(1.e1);

      x = -7.90298*(BOIL / tk - 1.) + 5.02808*log(BOIL / tk) / l10 -
         1.3816e-7*(pow(1.e1, 1.1344e1*(1. - tk / BOIL)) - 1.) +
         8.1328e-3*(pow(1.e1, -3.49149*(BOIL / tk - 1.)) - 1.) +
         log(SEA_LEVEL) / l10;

      x = pow(1.e1, x);

      ASSERT(!errno);
      return(x);
   }

   
   /*
    *
    * ----------------------------  satvp
    *
    */
    static double ScienceFcns::satvp(double d_point)
    {
       double	sat_vpres;

       d_point += FREEZE;

       /*	calculate saturation vapor pressure	*/
       sat_vpres = sati(d_point);

       return(sat_vpres);
    }



    /*
    * psi-functions
    *	code =	SM	momentum
    *		SH	sensible heat flux
    *		SV	latent heat flux
    */

    /*
    *
    * ----------------------------  psi
    *
    */
    static double psi(
       double zeta, /* z/lo				*/
       int code) /* which psi function? */
    {
       double	x;		/* height function variable	*/
       double	result;
       static int	already;
       static double	pid2;

       if (zeta > 0) {		/* stable */
          if (zeta > 1)
             zeta = 1;
          result = -BETA_S * zeta;
       }

       else if (zeta < 0) {	/* unstable */

                              /* find pi/2 on first pass */
          if (!already) {
             already = 1;
             pid2 = asin(1.0);
          }

          x = sqrt(sqrt(1 - BETA_U * zeta));

          switch (code) {
             case SM:
                result = 2 * log((1 + x) / 2) + log((1 + x * x) / 2) -
                   2 * atan(x) + pid2;
                break;

             case SH:
             case SV:
                result = 2 * log((1 + x * x) / 2);
                break;

             default: /* shouldn't reach */
                result = 0;
          }
       }

       else {			/* neutral */
          result = 0;
       }

       return (result);
    } // end of psi()


    /*
    *
    * ----------------------------  hle1
    *
    */
    static int ScienceFcns::hle1(
       double	press,	/* in: air pressure (Pa)			*/
       double	ta,	/* in: air temperature (K) at height za	*/
       double	ts,	/* in: surface temperature (K)		*/
       double	za,	/* in: height of air temp measurement (m)	*/
       double	ea,	/* in: vapor pressure (Pa) at height zq	*/
       double	es,	/* in: vapor pressure (Pa) at surface	*/
       double	zq,	/* in: height of spec hum measurement (m)	*/
       double	u,	/* in: wind speed (m/s) at height zu	*/
       double	zu,	/* in: height of wind speed measurement (m)	*/
       double	z0,	/* in: roughness length (m)			*/
       double *h,	/* out: sens heat flux (+ to surf) (W/m^2)	*/
       double *le,	/* out: latent heat flux (+ to surf) (W/m^2)	*/
       double *e)	/* out: mass flux (+ to surf) (kg/m^2/s)	*/
    {
       double	ah = AH;
       double	av = AV;
       double	cp = CP_AIR;
       double	d0;	/* displacement height (eq. 5.3)	*/
       double	dens;	/* air density				*/
       double	diff;	/* difference between guesses		*/
       double	factor;
       double	g = GRAV;
       double	k = VON_KARMAN;
       double	last;	/* last guess at lo			*/
       double	lo;	/* Obukhov stability length (eq. 4.25)	*/
       double	ltsh;	/* log ((za-d0)/z0)			*/
       double	ltsm;	/* log ((zu-d0)/z0)			*/
       double	ltsv;	/* log ((zq-d0)/z0)			*/
       double	qa;	/* specific humidity at height zq	*/
       double	qs;	/* specific humidity at surface		*/
       double	ustar;	/* friction velocity (eq. 4.34')	*/
       double	xlh;	/* latent heat of vap/subl		*/
       int	ier;	/* return error code			*/
       int	iter = 0;	/* iteration counter			*/

       if (z0 <= 0 || zq <= z0 || zu <= z0 || za <= z0) return(-2); /* heights must be positive */
       if (ta <= 0 || ts <= 0) return(-2); /* temperatures are Kelvin */
       if (ea <= 0 || es <= 0 || press <= 0 || ea >= press || es >= press) return(-2); // pressures must be positive

                                                                                       /* vapor pressures can't exceed saturation */
       if (es > sati(ts)) es = sati(ts);
       if (ea > satw(ta)) ea = satw(ta);

       d0 = 2 * PAESCHKE * z0 / 3; // displacement plane height, eq. 5.3 & 5.4

                                   // constant log expressions
       ltsm = log((zu - d0) / z0);
       ltsh = log((za - d0) / z0);
       ltsv = log((zq - d0) / z0);

       // convert vapor pressures to specific humidities
       qa = SPEC_HUM(ea, press);
       qs = SPEC_HUM(es, press);

       ta += DALR * za; // convert temperature to potential temperature

                        // air density at press, virtual temp of geometric mean of air and surface
       dens = GAS_DEN(press, MOL_AIR, VIRT(sqrt(ta*ts), sqrt(ea*es), press));

       /*
       * starting value, assume neutral stability, so psi-functions
       * are all zero
       */
       ustar = k * u / ltsm;
       factor = k * ustar * dens;
       *e = (qa - qs) * factor * av / ltsv;
       *h = (ta - ts) * factor * cp * ah / ltsh;

       /*
       * if not neutral stability, iterate on Obukhov stability
       * length to find solution
       */
       if (ta != ts) {

          lo = HUGE_VAL;
          iter = 0;

          do {
             last = lo;

             /*
             * Eq 4.25, but no minus sign as we define
             * positive H as toward surface
             */
             lo = ustar * ustar * ustar * dens
                / (k * g * (*h / (ta*cp) + 0.61 * *e));

             /*
             * friction velocity, eq. 4.34'
             */
             ustar = k * u / (ltsm - psi(zu / lo, SM));

             /*
             * evaporative flux, eq. 4.33'
             */
             factor = k * ustar * dens;
             *e = (qa - qs) * factor * av / (ltsv - psi(zq / lo, SV));
             if (*e != *e)
                ASSERT(1);

             /*
             * sensible heat flus, eq. 4.35'
             * with sign reversed
             */
             *h = (ta - ts) * factor * ah * cp / (ltsh - psi(za / lo, SH));

             diff = last - lo;

          } while (fabs(diff) > THRESH &&
             fabs(diff / lo) > THRESH &&
             ++iter < ITMAX);
       }

       ier = (iter >= ITMAX) ? -1 : 0;

       xlh = LH_VAP(ts);
       if (ts <= FREEZE)
          xlh += LH_FUS(ts);

       /*
       * latent heat flux (- away from surf)
       */
       *le = xlh * *e;

       return (ier);
    } // end of hle1(press, ta, ts, za, ea, es, zq, u, zu, z0, h, le, e)



      /*
    **	trbxfr - calculates H & LE using Brutsaert's method
    **
    **  synopsis
    **	trbxfr	elev= [ K ] [ mm ] [ dif ] [ <infile ] [ >outfile ]
    **
    **  description
    **	Calculates turbulent transfer using Brutsaert's description
    **	of the Businger-Dyer approach, using the Obukhov length for
    **	stability determination: (Refs in hle1)
    **	Air pressure is set from site elev.
    **	If temp/vapor pressure are measured at a different height
    **	from wind speed, "dif" is set, and z0 is assumed to be the
    **	roughness length.  Reads input file from stdin:
    **
    **		z, z0, t, t0, e, e0, u, u0
    **
    **	or, if "dif" is set:
    **
    **		zu, zt, z0, t, t0, e, e0, u
    **
    **		z  = upper height (m)
    **		z0 = lower height (m)
    **		zu = wind speed height (m)
    **		zt = temp/humidity height (m)
    **		t  = upper temperature (C)
    **		t0 = lower temperature (C)
    **		e  = upper vapro press. (Pa)
    **		e0 = lower vapor press. (Pa)
    **		u  = upper wind speed (m/sec)
    **		u0 = lower wind speed (m/sec)
    **
    **	(if z0 = roughness length, t0 = surface temp., e0 = surface
    **	 vapor press., and u0 = 0.0)
    **
    **	Outputs water gain/loss (+/-mm m^-2 s^-1), if mm is set.
    **	Output is to stdout.
    **
    **  diagnostics
    **	terminates with error message;
    **
    **  history
    **	July, 1984:  written by D. Marks, (GSFC) CSL, UCSB;
    **	June, 1987:  updated to use Brutsaert's method by
    **		     J. Dozier, CRSEO, UCSB;
    **
    **  bugs
    **	currently only allows 2 measurement heights, though could
    **	support three (for wind speed, air temp, and humidity);
    **
    */


    /*
    *
    * ----------------------------  trbxfr
    *
    */
    static double trbxfr(double z, double z0, double t, double t0, double e, double e0, double u, double elev)
    {
       double	pa;
       double	h;
       double	le;
       double	ev_air;

       /*	set pa from site elev	*/
       pa = HYSTAT(SEA_LEVEL, TB, (STDLR * 1000.0), (elev / 1000.0),
          GRAV, MOL_AIR);

       t += FREEZE;
       t0 += FREEZE;

       if (hle1(pa, t, t0, z, e, e0, z, u, z, z0, &h, &le, &ev_air) != 0)
       {
          /******
          This is what happend when you do not
          listen to your mother.  I am going to
          return a positive number.  Normally this
          would be a negative number and all would
          be ok.  I will look for this on the other
          side and note the error.
          *******/
          return((float) 1.0);
       }
       if (ev_air>0.f || ev_air != ev_air)
          ASSERT(1);
       return((float)ev_air);
    }
    static float calcPET(float tmp, float vpr, float vp_sat, float wnd, float elevation, float z, float z0)
    {
       float t, pet;
       t = (vp_sat>vpr) ? (float)trbxfr(z, z0, tmp, tmp, vpr, vp_sat, wnd, elevation) : 0.f;

       // trbxfr() returns a positive number when an error occurs.
       if (t >= 0.0 || vp_sat<vpr)
       {
          // if ((vpr - vp_sat)>50.0f) printf("*** ProcessModel.cpp/calcPET(): vp_sat, vpr t = %f, %f, %f, %f\n", vp_sat, vpr, vpr - vp_sat, t);
          t = 0.;
          ASSERT(0);
       }

       pet = t * SECONDS_PER_DAY * -1.0f;

       ASSERT(pet >= 0.0);

       return(pet);
    } // end of calcPET()


    static double CalcRelHumidityKP(float specHumid, float tMin, float tMax, float elevation, double &ea)
    { // This code is from WW2100 ETEquation.cpp, probably originally written by Cynthia Schwartz.
       // Atmospheric Pressure; kPa
       double P = 101.3 * pow(((288.0 - 0.0065 * elevation) / 288.0), 5.257);

       // P is barometric pressure at elevation
       // ea is actual water vapor pressure
       // es is saturated water vapor pressure
       // relative humidity is ratio of ea/es

       ea = P / ((0.622 / specHumid) + 1.0 - 0.622);

       double esTmax = 3.38639 * (pow(0.00738 * tMax + 0.8072, 8.0) - 0.000019 * fabs(1.8 * tMax + 48.0) + 0.001316);
       double esTmin = 3.38639 * (pow(0.00738 * tMin + 0.8072, 8.0) - 0.000019 * fabs(1.8 * tMin + 48.0) + 0.001316);

       double es = (esTmax + esTmin) / 2.0;
       double rh = ea / es;

       if (ea >= es) return 1.0;

       return rh;
    }


    static void ScienceFcns::tree_dim(float tree_lai, float ltree, bool deciduous, float * tree_htP, float * dbhP)
    {

       float k, dbh_max, ht_max, c, d, /* lleaf, */ stems;
       float stnd_clos, la;
       float b2, b3;
       float dbh, tree_ht;

       /* assign thinning law constant as function of veg type */
       /* thinning constant (k) from D.E. Weller (1989) */
       if (deciduous)
       {

          /* parameterized for QUKE */

          k = 8696.;
          dbh_max = 349.;
          ht_max = 3960;
          c = 0.451f;
          d = 1.60f;

          // lleaf = tree_lai / .012;
       }

       else
       {

          /* parameterized for PSME */

          k = 5073.;
          dbh_max = 220.;
          ht_max = 8000;
          c = 0.116f;
          d = 1.89f;

          // lleaf = tree_lai / .003;
       }

       /* compute n stems per m2 using
       self-thinning law (Weller 1989) */

       stems = (float)(pow((k / ltree), 2.)); // Can produce a nan when ltree is very small.

                                              // Use 1000 as a practical upper bound on the number of tree stems m-2
                                              // based on the data for trees in fig. 2 of Weller 1989.
       if (stems>1000. || isnan(stems)) stems = 1000.;

       /* compute relative stand closure */
       stnd_clos = tree_lai / 3.75f;
       if (stnd_clos >= 1.0) stnd_clos = 1.0;
       else stems -= (stems * (1.f - stnd_clos)); /* reduce stems in open stand */

                                                  /* compute leaf area per tree in sq m */

       la = tree_lai / stems;

       /* estimate tree dbh (cm) from la using SILVA func */

       dbh = (float)pow((la / c), 1. / d);

       /* lower and upper constraints on current dbh */

       if (dbh <= 0.0)
          dbh = .001f;

       if (dbh > dbh_max)
          dbh = dbh_max;


       /* estimate tree height (cm) from dbh using JABOWA funcs */

       b2 = (float)(2. * ((ht_max - 137.) / dbh_max));
       b3 = (float)((ht_max - 137.) / pow(dbh_max, 2.f));

       tree_ht = (float)(137. + (b2 * dbh) - (b3 * pow(dbh, 2.)));

       /* convert cm -> m */

       tree_ht *= 0.01f;

       *dbhP = dbh;
       *tree_htP = tree_ht;

    } // end of ScienceFcns::tree_dim()

}; // end of class ScienceFcns

#endif /* SCIENCEFCNS_H */
